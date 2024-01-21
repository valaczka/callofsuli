/*
 * ---- Call of Suli ----
 *
 * conquestengine.cpp
 *
 * Created on: 2024. 01. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestEngine
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "conquestengine.h"
#include "Logger.h"
#include "qjsonobject.h"
#include "serverservice.h"
#include "websocketstream.h"
#include "enginehandler.h"

int ConquestEngine::m_nextId = 0;


/**
 * @brief ConquestEngine::ConquestEngine
 * @param handler
 * @param parent
 */

ConquestEngine::ConquestEngine(EngineHandler *handler, QObject *parent)
	: AbstractEngine(EngineConquest, handler, parent)
{
	LOG_CTRACE("engine") << "ConquestEngine created" << this;

	m_question = std::make_unique<ConquestQuestion>(this);
}


/**
 * @brief ConquestEngine::~ConquestEngine
 */

ConquestEngine::~ConquestEngine()
{
	m_question.reset();
	LOG_CTRACE("engine") << "ConquestEngine destroyed" << this;
}






/**
 * @brief ConquestEngine::handleWebSocketMessage
 * @param stream
 * @param message
 * @param handler
 */

void ConquestEngine::handleWebSocketMessage(WebSocketStream *stream, const QJsonValue &message, EngineHandler *handler)
{
	if (!handler || !stream)
		return;

	const QJsonObject &obj = message.toObject();
	const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

	const auto &id = obj.value(QStringLiteral("engine")).toInt(-1);
	ConquestEngine *engine = stream->engineGet<ConquestEngine>(AbstractEngine::EngineConquest, id);

	LOG_CINFO("engine") << "HANDLE" << cmd << message;

	using fnDef = QJsonObject (ConquestEngine::*)(WebSocketStream *, const QJsonObject &, EngineHandler *);

	static QHash<std::string, fnDef> fMap = {
		{ "state", &ConquestEngine::cmdState },
		{ "start", &ConquestEngine::gameStart },
		{ "prepare", &ConquestEngine::gamePrepare },
		{ "enroll", &ConquestEngine::cmdEnroll },
		{ "play", &ConquestEngine::gamePlay },
		{ "questionRequest", &ConquestEngine::cmdQuestionRequest },
		{ "finish", &ConquestEngine::gameFinish },
		{ "test", &ConquestEngine::cmdTest },
	};

	auto fn = fMap.value(cmd.toStdString());


	QJsonObject ret;

	if (cmd == QStringLiteral("create")) {
		ret = cmdCreate(stream, obj, handler);
	} else if (cmd == QStringLiteral("connect")) {
		ret = cmdConnect(stream, obj, handler, engine);
	} else if (fn) {
		if (engine)
			ret = std::invoke(fn, engine, stream, obj, handler);
		else
			ret = {
				{ QStringLiteral("error"), QStringLiteral("invalid engine") }
			};
	} else {
		ret = {
			{ QStringLiteral("error"), QStringLiteral("invalid command") }
		};
	}

	if (!ret.isEmpty()) {
		ret.insert(QStringLiteral("cmd"), cmd);
		sendStreamJson(stream, ret);
	}
}



/**
 * @brief ConquestEngine::createEngine
 * @param stream
 * @param handler
 * @return
 */

std::weak_ptr<ConquestEngine> ConquestEngine::createEngine(WebSocketStream *stream, EngineHandler *handler)
{
	if (!handler)
		return {};

	increaseNextId();

	LOG_CDEBUG("engine") << "Create ConquestEngine" << m_nextId << stream;

	auto ptr = std::make_shared<ConquestEngine>(handler);

	ptr->setId(m_nextId);

	if (stream) {
		ptr->setHostStream(stream);
		LOG_CINFO("engine") << "SET HOST" << ptr.get() << stream << ptr->m_id;
		ptr->m_config.state = ConquestConfig::StateConnect;
		handler->websocketEngineLink(stream, ptr);
		ptr->playerEnroll(stream);
	}

	handler->engineAdd(ptr);

	return ptr;
}




/**
 * @brief ConquestEngine::connectToEngine
 * @param id
 * @param stream
 * @param handler
 * @return
 */

std::weak_ptr<AbstractEngine> ConquestEngine::connectToEngine(const int &id, WebSocketStream *stream, EngineHandler *handler)
{
	if (!handler || !stream)
		return std::weak_ptr<AbstractEngine>();

	LOG_CDEBUG("engine") << "Connect to ConquestEngine" << id;

	const auto &ptr = handler->engineGet(AbstractEngine::EngineConquest, id);

	if (!ptr.expired()) {
		LOG_CTRACE("engine") << "Engine exists" << id;
		handler->websocketEngineLink(stream, ptr.lock());
	}

	return ptr;
}



/**
 * @brief ConquestEngine::gameFinish
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::gameFinish(WebSocketStream *, const QJsonObject &, EngineHandler *)
{
	LOG_CDEBUG("engine") << "Finish conquest game:" << m_id;

	m_config.state = ConquestConfig::StateFinished;

	m_handler->engineTriggerEngine(this);

	return {};
}




/**
 * @brief ConquestEngine::gameStart
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::gameStart(WebSocketStream *stream, const QJsonObject &, EngineHandler *)
{
	if (m_config.state != ConquestConfig::StateConnect)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};


	if (stream != m_hostStream)
		return {
			{ QStringLiteral("error"), QStringLiteral("permission denied") }
		};


	if (m_players.size() < 2)
		return {
			{ QStringLiteral("error"), QStringLiteral("insufficient players") }
		};

	m_config.state = ConquestConfig::StatePrepare;

	// TODO: getWorld by players number

	m_config.world = "sample";

	m_handler->engineTriggerEngine(this);

	return {};
}



/**
 * @brief ConquestEngine::gamePrepare
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::gamePrepare(WebSocketStream *stream, const QJsonObject &message, EngineHandler *)
{
	if (m_config.state != ConquestConfig::StatePrepare)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};

	if (message.value(QStringLiteral("ready")).toBool()) {
		onPlayerPrepared(stream);
		return {};
	}

	return {};
}



/**
 * @brief ConquestEngine::gamePlay
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::gamePlay(WebSocketStream *stream, const QJsonObject &, EngineHandler *)
{
	if (m_config.state != ConquestConfig::StatePrepare)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};


	int w = 0;

	for (auto &p : m_players) {
		if (p.stream == stream)
			p.prepared = true;
		else if (!p.prepared)
			++w;
	}

	if (w) {
		LOG_CTRACE("engine") << "Wait for" << w << "players to be prepared";
		return {
			{ QStringLiteral("wait"), w }
		};
	}

	LOG_CDEBUG("engine") << "Play ConquestGame" << m_id << this;

	m_startedAt = QDateTime::currentMSecsSinceEpoch();
	m_elapsedTimer.start();
	m_config.state = ConquestConfig::StatePlay;

	m_handler->engineTriggerEngine(this);

	return {
		{ QStringLiteral("play"), true }
	};
}



/**
 * @brief ConquestEngine::canDelete
 * @param useCount
 * @return
 */

bool ConquestEngine::canDelete(const int &useCount)
{
	return (useCount == 1 && m_config.state == ConquestConfig::StateFinished);
}



/**
 * @brief ConquestEngine::timerTick
 */

void ConquestEngine::timerTick()
{
	LOG_CTRACE("engine") << "Timer tick" << this;

	if (m_config.state != ConquestConfig::StatePrepare && m_config.state != ConquestConfig::StatePlay)
		return;

	m_question->check();

	/*if (m_gameState != StatePlaying)
		return;

	QByteArray content;

	for (const auto & [entityId, entity] : m_entities) {
		content.append("=========================================\n");
		content.append(QString("%1 - %2\n").arg(entityId).arg(entity.type).toUtf8());
		content.append(entity.owner.toUtf8()).append("\n");
		content.append("=========================================\n");
		for (const auto &rs : entity.renderedStates) {
			content.append(rs.toReadable());
			content.append("---------------------------------\n");
		}
	}

	QFile f("/tmp/_state.txt");
	f.open(QIODevice::WriteOnly);
	f.write(content);
	f.close();

	sendStates();

	LOG_CTRACE("engine") << "States saved";*/
}



/**
 * @brief ConquestEngine::streamTriggerEvent
 * @param stream
 */

void ConquestEngine::streamTriggerEvent(WebSocketStream *stream)
{
	LOG_CINFO("engine") << "Stream trigger" << this << stream << currentTick();

	QJsonObject ret = m_config.toJson();

	ret.insert(QStringLiteral("cmd"), QStringLiteral("state"));
	ret.insert(QStringLiteral("engine"), m_id);
	ret.insert(QStringLiteral("interval"), m_service->mainTimerInterval());
	ret.insert(QStringLiteral("host"), (m_hostStream == stream ? true : false));

	if (m_startedAt != -1) {
		ret.insert(QStringLiteral("startedAt"), m_startedAt);
	}

	if (m_elapsedTimer.isValid()) {
		ret.insert(QStringLiteral("tick"), currentTick());
	}


	const auto &player = playerGet(stream);

	if (player) {
		ret.insert(QStringLiteral("playerId"), player->id);
		//ret.insert(QStringLiteral("playerEntityId"), player->entityId);
	}


	QJsonArray players;

	for (const auto &p : m_players)
		players.append(p.toJson());

	ret.insert(QStringLiteral("users"), players);

	sendStreamJson(stream, ret);
}


/**
 * @brief ConquestEngine::currentTick
 * @return
 */

qint64 ConquestEngine::currentTick() const
{
	if (m_elapsedTimer.isValid())
		return m_elapsedTimer.elapsed();
	else
		return 0;
}


/**
 * @brief ConquestEngine::playerEnroll
 * @param stream
 * @return
 */

int ConquestEngine::playerEnroll(WebSocketStream *stream)
{
	if (!stream)
		return -1;

	const int &id = playerGetId(stream->credential().username());

	if (id != -1) {
		LOG_CWARNING("engine") << "Player already enrolled:" << qPrintable(stream->credential().username());
	}

	if (m_playerLimit > 0 && m_players.size() >= m_playerLimit) {
		LOG_CWARNING("engine") << "Players number full";
		return -1;
	}

	int playerId = 1;

	for (const auto &p : m_players) {
		playerId = std::max(p.id+1, playerId);
	}

	const QString &username = stream->credential().username();

	LOG_CDEBUG("engine") << "Enroll player" << playerId << stream << qPrintable(username);

	m_players.push_back({playerId, username, stream});

	m_handler->engineTriggerEngine(this);

	return playerId;
}




/**
 * @brief ConquestEngine::playerConnectStream
 * @param playerId
 * @param stream
 * @return
 */

bool ConquestEngine::playerConnectStream(const int &playerId, WebSocketStream *stream)
{
	for (Player &p : m_players) {
		if (p.id == playerId) {
			if (p.stream == nullptr) {
				p.stream = stream;
				return true;
			} else {
				LOG_CWARNING("engine") << "Can't connect player stream" << stream << "to id" << playerId;
				return false;
			}
		}
	}

	return false;
}


/**
 * @brief ConquestEngine::playerDisconnectStream
 * @param playerId
 * @param stream
 */

void ConquestEngine::playerDisconnectStream(const int &playerId, WebSocketStream *stream)
{
	for (auto &p : m_players) {
		if ((playerId == -1 && p.stream == stream) || (p.id == playerId))
			p.stream = nullptr;
	}
}




/**
 * @brief ConquestEngine::playerGet
 * @param stream
 * @return
 */

std::optional<ConquestEngine::Player> ConquestEngine::playerGet(WebSocketStream *stream) const
{
	if (!stream)
		return std::nullopt;

	for (const auto &p : m_players) {
		if (p.stream == stream)
			return p;
	}

	return std::nullopt;
}


/**
 * @brief ConquestEngine::playerGetId
 * @param username
 * @return
 */

int ConquestEngine::playerGetId(const QString &username) const
{
	if (username.isEmpty())
		return -1;

	for (const auto &p : m_players) {
		if (p.username == username)
			return p.id;
	}

	return -1;
}




/**
 * @brief ConquestEngine::playerGetStream
 * @param username
 * @return
 */

WebSocketStream *ConquestEngine::playerGetStream(const QString &username) const
{
	if (username.isEmpty())
		return nullptr;

	for (const auto &p : m_players) {
		if (p.username == username)
			return p.stream;
	}

	return nullptr;
}





/**
 * @brief ConquestEngine::sendStreamJson
 * @param stream
 * @param value
 */

void ConquestEngine::sendStreamJson(WebSocketStream *stream, const QJsonValue &value)
{
	if (stream)
		stream->sendJson("conquest", value);
}



/**
 * @brief ConquestEngine::streamUnlinkedEvent
 * @param stream
 */

void ConquestEngine::streamUnlinkedEvent(WebSocketStream *stream)
{
	LOG_CTRACE("engine") << "ConquestEngine stream disconnected:" << stream << (stream ? stream->credential().username() : "");

	playerDisconnectStream(stream);

	if (m_hostStream == stream) {
		LOG_CTRACE("engine") << "Host stream disconnected";

		WebSocketStream *next = nullptr;

		for (auto &s : m_streams) {
			if (s == stream)
				continue;

			next = s;
			break;
		}

		if (!next) {
			LOG_CWARNING("engine") << "All stream dismissed";
			setHostStream(nullptr);
			gameFinish(stream, {}, m_handler);
		} else {
			LOG_CINFO("engine") << "Next host stream:" << next;
			setHostStream(next);
		}

		m_handler->engineTriggerEngine(this);
	}
}





/**
 * @brief ConquestEngine::cmdCreate
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdCreate(WebSocketStream *stream, const QJsonObject &message, EngineHandler *handler)
{
	const QString &map = message.value(QStringLiteral("map")).toString();
	const QString &mission = message.value(QStringLiteral("mission")).toString();
	const int &level = message.value(QStringLiteral("level")).toInt(0);

	if (map.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing mapuuid") }
		};

	if (mission.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing missionuuid") }
		};

	if (level <= 0)
		return {
			{ QStringLiteral("error"), QStringLiteral("missing level") }
		};

	auto ptr = createEngine(stream, handler);

	if (ptr.expired()) {
		return {
			{ QStringLiteral("error"), QStringLiteral("internal error") }
		};
	}

	auto engine = ptr.lock().get();

	engine->m_config.mapUuid = map;
	engine->m_config.missionUuid = mission;
	engine->m_config.missionLevel = level;

	LOG_CINFO("engine") << "CREATED" << engine->id();

	return {
		{ QStringLiteral("engine"), engine->id() }
	};
}


/**
 * @brief ConquestEngine::cmdConnect
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdConnect(WebSocketStream *stream, const QJsonObject &message,
									   EngineHandler *handler, ConquestEngine *engine)
{
	LOG_CTRACE("engine") << "Connect to ConquestEngine";

	if (!engine) {
		LOG_CERROR("engine") << "TEST CASE" << __PRETTY_FUNCTION__;

		const auto &list = handler->engineGet<ConquestEngine>(EngineConquest);

		LOG_CDEBUG("engine") << "Engines" << list;

		if (list.isEmpty()) {
			return cmdCreate(stream, message, handler);
		}

		/*return {
			{ QStringLiteral("error"), QStringLiteral("invalid engine") }
		};*/

		engine = list.at(0);
	}

	if (const auto &e = connectToEngine(engine->id(), stream, handler); e.expired()) {
		return {
			{ QStringLiteral("error"), QStringLiteral("internal error") }
		};
	}

	LOG_CINFO("engine") << "CONNECTED" << engine->id();

	return {
		{ QStringLiteral("engine"), engine->id() }
	};
}




/**
 * @brief ConquestEngine::cmdState
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdState(WebSocketStream *, const QJsonObject &, EngineHandler *)
{
	m_handler->engineTriggerEngine(this);
	return {};
}



/**
 * @brief ConquestEngine::cmdEnroll
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdEnroll(WebSocketStream *stream, const QJsonObject &, EngineHandler *)
{
	playerEnroll(stream);
	m_handler->engineTriggerEngine(this);
	return {};
}



/**
 * @brief ConquestEngine::cmdQuestionRequest
 * @param stream
 * @param message
 * @return
 */

QJsonObject ConquestEngine::cmdQuestionRequest(WebSocketStream *stream, const QJsonObject &message, EngineHandler *)
{
	LOG_CTRACE("engine") << "Upload questions";

	if (m_config.state != ConquestConfig::StatePrepare && m_config.state != ConquestConfig::StatePlay)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};


	if (stream != m_hostStream)
		return {
			{ QStringLiteral("error"), QStringLiteral("permission denied") }
		};

	m_question->upload(message.value(QStringLiteral("list")).toArray());

	return {};
}



/**
 * @brief ConquestEngine::cmdTest
 * @param stream
 * @param message
 * @return
 */

QJsonObject ConquestEngine::cmdTest(WebSocketStream *stream, const QJsonObject &message, EngineHandler *)
{
	for (const auto &p : m_players) {
		sendStreamJson(p.stream, QJsonObject{
						  { QStringLiteral("cmd"), QStringLiteral("test") },
						   { QStringLiteral("stateId"), message.value("stateId") },
						   { QStringLiteral("value"), message.value("value") },
					   });
	}

	return {};
}



/**
 * @brief ConquestEngine::onPlayerPrepared
 */

void ConquestEngine::onPlayerPrepared(WebSocketStream *stream)
{
	if (!stream)
		return;

	LOG_CDEBUG("engine") << "Player prepared" << stream << qPrintable(stream->credential().username());

	gamePlay(stream, {}, m_handler);
}


/**
 * @brief ConquestEngine::config
 * @return
 */

ConquestConfig ConquestEngine::config() const
{
	return m_config;
}

void ConquestEngine::setConfig(const ConquestConfig &newConfig)
{
	m_config = newConfig;
}



/**
 * @brief ConquestEngine::hostStream
 * @return
 */

WebSocketStream *ConquestEngine::hostStream() const
{
	return m_hostStream;
}

void ConquestEngine::setHostStream(WebSocketStream *newHostStream)
{
	m_hostStream = newHostStream;
}



/**
 * @brief ConquestQuestion::check
 */

void ConquestQuestion::check()
{
	m_worker.execInThread([this]{
		QMutexLocker locker(&m_mutex);
		LOG_CTRACE("engine") << "Check questions" << m_questionArray.size();

		if (m_questionArray.size() >= 4)
			return;

		auto *ws = m_engine->m_hostStream;

		if (!ws) {
			LOG_CINFO("engine") << "NO WS" << ws;
			return;
		}

		const qint64 &now = QDateTime::currentSecsSinceEpoch();

		LOG_CDEBUG("engine") << "CHECK" << now << m_lastRequest << (now-m_lastRequest) ;

		if (m_lastRequest == 0 || now-m_lastRequest > 4) {
			LOG_CINFO("engine") << "REQUEST" << ws;
			m_engine->sendStreamJson(ws, QJsonObject{
										{ QStringLiteral("cmd"), QStringLiteral("questionRequest") }
									 });
			m_lastRequest = now;
		}

	});
}



/**
 * @brief ConquestQuestion::upload
 * @param list
 */

void ConquestQuestion::upload(const QJsonArray &list)
{
	m_worker.execInThread([this, list]{
		QMutexLocker locker(&m_mutex);
		LOG_CTRACE("engine") << "Upload questions" << list;

		for (const QJsonValue &v : list)
			m_questionArray.append(v);
	});
}
