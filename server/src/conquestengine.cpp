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

#define LAND_XP				50
#define LAND_XP_ONCE		100

#define MSEC_SELECT			10000
#define MSEC_ANSWER			30000
#define MSEC_FINISH			10000
#define MSEC_FINISH_WAIT	2000
#define MSEC_GAME_TIMEOUT	10*60*1000


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

	LOG_CINFO("engine") << "HANDLE" << cmd << message << id << engine;

	using fnDef = QJsonObject (ConquestEngine::*)(WebSocketStream *, const QJsonObject &);

	static QHash<std::string, fnDef> fMap = {
		{ "state", &ConquestEngine::cmdState },
		{ "start", &ConquestEngine::gameStart },
		{ "prepare", &ConquestEngine::gamePrepare },
		{ "enroll", &ConquestEngine::cmdEnroll },
		{ "play", &ConquestEngine::gamePlay },
		{ "questionRequest", &ConquestEngine::cmdQuestionRequest },
		{ "finish", &ConquestEngine::gameFinish },
		{ "pick", &ConquestEngine::cmdPick },
		{ "answer", &ConquestEngine::cmdAnswer },
		{ "test", &ConquestEngine::cmdTest },	///
	};

	auto fn = fMap.value(cmd.toStdString());


	QJsonObject ret;

	if (cmd == QStringLiteral("create")) {
		if (engine ){
			ret = {
				{ QStringLiteral("error"), QStringLiteral("engine already connected") }
			};
		} else {
			ret = cmdCreate(stream, obj, handler);
		}
	} else if (cmd == QStringLiteral("list")) {
		ret = cmdList(obj, handler);
	} else if (cmd == QStringLiteral("connect")) {
		if (engine ){
			ret = {
				{ QStringLiteral("error"), QStringLiteral("engine already connected") }
			};
		} else {
			ret = cmdConnect(stream, handler, id);
		}
	} else if (fn) {
		if (engine)
			ret = std::invoke(fn, engine, stream, obj);
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
		ptr->m_config.gameState = ConquestConfig::StateConnect;
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

QJsonObject ConquestEngine::gameFinish(WebSocketStream *, const QJsonObject &)
{
	LOG_CDEBUG("engine") << "Finish conquest game:" << m_id;

	if (m_elapsedTimer.isValid())
		m_elapsedTimer.invalidate();

	m_config.gameState = ConquestConfig::StateFinished;

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

QJsonObject ConquestEngine::gameStart(WebSocketStream *stream, const QJsonObject &)
{
	if (m_config.gameState != ConquestConfig::StateConnect)
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

	m_config.gameState = ConquestConfig::StatePrepare;

	prepareWorld();
	preparePlayerOrder();
	pickLands();

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

QJsonObject ConquestEngine::gamePrepare(WebSocketStream *stream, const QJsonObject &message)
{
	if (m_config.gameState != ConquestConfig::StatePrepare)
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

QJsonObject ConquestEngine::gamePlay(WebSocketStream *stream, const QJsonObject &)
{
	if (m_config.gameState == ConquestConfig::StatePlay) {
		streamTriggerEvent(stream);
		return {};
	}

	if (m_config.gameState != ConquestConfig::StatePrepare)
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

	if (!m_question->hasQuestion()) {
		LOG_CTRACE("engine") << "Wait for questions";
		return {
			{ QStringLiteral("wait"), 1 }
		};
	}

	LOG_CINFO("engine") << "Play ConquestGame" << m_id;

	m_startedAt = QDateTime::currentMSecsSinceEpoch();
	m_elapsedTimer.start();
	m_config.currentTurn = -1;
	m_config.gameState = ConquestConfig::StatePlay;
	m_config.currentStage = ConquestTurn::StagePick;

	if (!nextPick(false)) {
		LOG_CERROR("engine") << "FINISH GAME ERROR";
		gameFinish(nullptr, {});

		return {
			{ QStringLiteral("error"), QStringLiteral("play error") }
		};
	}

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
	return (useCount == 1 && m_config.gameState == ConquestConfig::StateFinished);
}



/**
 * @brief ConquestEngine::timerTick
 */

void ConquestEngine::timerTick()
{
	LOG_CTRACE("engine") << "Timer tick" << currentTick() << this;

	if (m_config.gameState != ConquestConfig::StatePrepare && m_config.gameState != ConquestConfig::StatePlay)
		return;

	if (currentTick() >= MSEC_GAME_TIMEOUT) {
		LOG_CINFO("engine") << "Game timer timeout" << this;
		gameFinish(nullptr, {});
		return;
	}

	m_question->check();
	checkTurn();

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

	QMutexLocker locker(&m_engineMutex);

	QJsonObject ret = m_config.toJson();

	ret.insert(QStringLiteral("cmd"), QStringLiteral("state"));
	ret.insert(QStringLiteral("engine"), m_id);
	ret.insert(QStringLiteral("interval"), m_service->mainTimerInterval());
	ret.insert(QStringLiteral("host"), (m_hostStream == stream ? true : false));
	ret.insert(QStringLiteral("playerLimit"), (int) m_playerLimit);

	if (m_startedAt != -1) {
		ret.insert(QStringLiteral("startedAt"), m_startedAt);
	}

	if (m_elapsedTimer.isValid()) {
		ret.insert(QStringLiteral("tick"), currentTick());
	}


	const auto &player = playerGet(stream);

	if (player) {
		ret.insert(QStringLiteral("playerId"), player->playerId);
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

		// playerConnectStream
	}

	if (m_playerLimit > 0 && m_players.size() >= m_playerLimit) {
		LOG_CWARNING("engine") << "Players number full";
		return -1;
	}

	int playerId = 1;

	for (const auto &p : m_players) {
		playerId = std::max(p.playerId+1, playerId);
	}

	const QString &username = stream->credential().username();

	LOG_CDEBUG("engine") << "Enroll player" << playerId << stream << qPrintable(username);

	ConquestEnginePlayer player(playerId, username, stream);

	switch (m_players.size()) {
		case 0:
			player.theme = "red";
			break;
		case 1:
			player.theme = "green";
			break;
		case 2:
			player.theme = "blue";
			break;
		default:
			player.theme = "purple";
			break;
	}

	m_players.push_back(player);

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
	for (ConquestEnginePlayer &p : m_players) {
		if (p.playerId == playerId) {
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
		if ((playerId == -1 && p.stream == stream) || (p.playerId == playerId))
			p.stream = nullptr;
	}
}




/**
 * @brief ConquestEngine::playerGet
 * @param stream
 * @return
 */

std::optional<ConquestEnginePlayer> ConquestEngine::playerGet(WebSocketStream *stream) const
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
			return p.playerId;
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
			gameFinish(stream, {});
		} else {
			LOG_CINFO("engine") << "Next host stream:" << next;
			setHostStream(next);
		}

		m_handler->engineTriggerEngine(this);
	}
}



/**
 * @brief ConquestEngine::cmdList
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdList(const QJsonObject &message, EngineHandler *handler)
{
	LOG_CTRACE("engine") << "List ConquestEngines";

	ConquestConfig config;
	config.fromJson(message);

	if (config.mapUuid.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing mapuuid") }
		};

	if (config.missionUuid.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing missionuuid") }
		};

	if (config.missionLevel <= 0)
		return {
			{ QStringLiteral("error"), QStringLiteral("missing level") }
		};


	const auto &list = handler->engineGet<ConquestEngine>(AbstractEngine::EngineConquest);

	QJsonArray ret;

	for (ConquestEngine *e : list) {
		if (e->config().mapUuid != config.mapUuid ||
				e->config().missionUuid != config.missionUuid ||
				e->config().missionLevel != config.missionLevel
				)
			continue;

		ret.append(QJsonObject{
					   { QStringLiteral("engineId"), e->id() },
					   { QStringLiteral("owner"), e->owner() },
				   });

	}

	return {
		{ QStringLiteral("list"), ret }
	};
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
	ConquestConfig config;
	config.fromJson(message);

	if (config.mapUuid.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing mapuuid") }
		};

	if (config.missionUuid.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing missionuuid") }
		};

	if (config.missionLevel <= 0)
		return {
			{ QStringLiteral("error"), QStringLiteral("missing level") }
		};



	ConquestWordListHelper worldHelper;
	worldHelper.fromJson(message);

	if (worldHelper.worldList.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing worldList") }
		};


	auto ptr = createEngine(stream, handler);

	if (ptr.expired()) {
		return {
			{ QStringLiteral("error"), QStringLiteral("internal error") }
		};
	}

	auto engine = ptr.lock().get();

	engine->setOwner(stream->credential().username());
	engine->m_config.mapUuid = config.mapUuid;
	engine->m_config.missionUuid = config.missionUuid;
	engine->m_config.missionLevel = config.missionLevel;
	engine->m_worldListHelper = worldHelper.worldList;

	engine->updatePlayerLimit();


	LOG_CINFO("engine") << "CREATED" << engine->id();

	return {
		{ QStringLiteral("created"), true },
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

QJsonObject ConquestEngine::cmdConnect(WebSocketStream *stream, EngineHandler *handler, const int &id)
{
	LOG_CTRACE("engine") << "Connect to ConquestEngine:" << id;

	const auto &e = connectToEngine(id, stream, handler);

	if (e.expired()) {
		return {
			{ QStringLiteral("error"), QStringLiteral("internal error") }
		};
	}

	ConquestEngine *engine = qobject_cast<ConquestEngine*>(e.lock().get());

	LOG_CINFO("engine") << "CONNECTED" << id  << engine;

	const int &pid = engine->playerGetId(stream->credential().username());

	if (pid != -1) {
		LOG_CWARNING("engine") << "Player already enrolled, REENROLL:" << qPrintable(stream->credential().username());

		// playerConnectStream
	}

	return {
		{ QStringLiteral("engine"), id }
	};
}




/**
 * @brief ConquestEngine::cmdState
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdState(WebSocketStream *, const QJsonObject &)
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

QJsonObject ConquestEngine::cmdEnroll(WebSocketStream *stream, const QJsonObject &)
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

QJsonObject ConquestEngine::cmdQuestionRequest(WebSocketStream *stream, const QJsonObject &message)
{
	LOG_CTRACE("engine") << "Upload questions";

	if (m_config.gameState != ConquestConfig::StatePrepare && m_config.gameState != ConquestConfig::StatePlay)
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
 * @brief ConquestEngine::cmdPick
 * @param stream
 * @param message
 * @return
 */

QJsonObject ConquestEngine::cmdPick(WebSocketStream *stream, const QJsonObject &message)
{
	if (!stream)
		return {};

	QMutexLocker locker(&m_engineMutex);

	LOG_CINFO("engine") << "Pick" << stream->credential().username() << message;

	const auto &player = playerGet(stream);

	if (!player)
		return {};

	LOG_CDEBUG("engine") << "Picked player" << player->playerId << m_config.currentTurn;

	if (m_config.currentTurn < 0 || m_config.currentTurn >= m_config.turnList.size())
		return {};

	ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

	LOG_CDEBUG("engine") << "Picked turn" << turn.subStage << turn.player;

	if (turn.subStage != ConquestTurn::SubStageUserSelect || turn.player != player->playerId)
		return {};

	const QString &id = message.value(QStringLiteral("id")).toString();

	if (m_config.currentStage == ConquestTurn::StagePick) {
		const int &idx = m_config.world.landFind(id);

		LOG_CDEBUG("engine") << "Picked IDX" << idx;

		if (idx == -1) {
			return {
				{ QStringLiteral("error"), QStringLiteral("invalid land") }
			};
		}

		ConquestWorldData &land = m_config.world.landList[idx];
		turn.pickedId = id;

		if (land.proprietor != -1) {
			return {
				{ QStringLiteral("error"), QStringLiteral("land unavailable") }
			};
		}

		land.proprietor = player->playerId;
	}

	nextSubStage();

	//m_handler->engineTriggerEngine(this);
	return {};
}


/**
 * @brief ConquestEngine::cmdAnswer
 * @param stream
 * @param message
 * @return
 */

QJsonObject ConquestEngine::cmdAnswer(WebSocketStream *stream, const QJsonObject &message)
{
	if (!stream)
		return {};

	QMutexLocker locker(&m_engineMutex);

	LOG_CINFO("engine") << "Answer" << stream->credential().username() << message;

	const auto &player = playerGet(stream);

	if (!player)
		return {};

	LOG_CDEBUG("engine") << "Answer player" << player->playerId << m_config.currentTurn;

	if (m_config.currentTurn < 0 || m_config.currentTurn >= m_config.turnList.size())
		return {};

	ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

	LOG_CDEBUG("engine") << "Answer turn" << turn.subStage;

	if (turn.subStage != ConquestTurn::SubStageUserAnswer)
		return {};

	if (turn.answerGet(player->playerId))
		return {};

	ConquestAnswer answer;
	answer.player = player->playerId;
	answer.answer = message;

	turn.answerList.append(answer);

	QList<int> tmp;

	for (const ConquestPlayer &p : m_players)
		tmp.append(p.playerId);

	for (const ConquestAnswer &a : turn.answerList)
		tmp.removeAll(a.player);

	LOG_CERROR("engine") << "Answer result ------";

	for (const auto &a : turn.answerList) {
		LOG_CDEBUG("engine") << a.player << a.answer;
	}

	LOG_CERROR("engine") << "------ Answer result";


	if (tmp.isEmpty())
		nextSubStage();
	else
		m_handler->engineTriggerEngine(this);

	return {};
}



/**
 * @brief ConquestEngine::cmdTest
 * @param stream
 * @param message
 * @return
 */

QJsonObject ConquestEngine::cmdTest(WebSocketStream *stream, const QJsonObject &message)
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

	gamePlay(stream, {});
}



/**
 * @brief ConquestEngine::updatePlayerLimit
 */

void ConquestEngine::updatePlayerLimit()
{
	m_playerLimit = 0;

	for (const auto &w : m_worldListHelper) {
		for (const auto &p : w.infoList) {
			if (p.playerCount < 0)
				continue;

			uint c = (uint) p.playerCount;

			if (c > m_playerLimit)
				m_playerLimit = c;
		}
	}

	// Max. 4 players [-> prepareTurns()]

	if (m_playerLimit <= 0 || m_playerLimit > 4)
		m_playerLimit = 4;
}




/**
 * @brief ConquestEngine::prepareWorld
 */

void ConquestEngine::prepareWorld()
{
	LOG_CTRACE("engine") << "Prepare world";

	if (m_players.empty())
		return;

	const int &s = m_players.size();

	QList<ConquestWorldHelper> list;

	for (const auto &w : m_worldListHelper) {
		if (auto it = std::find_if(w.infoList.constBegin(), w.infoList.constEnd(),
								   [s](const ConquestWorldHelperInfo &i){
								   return i.playerCount == s;
	}); it != w.infoList.constEnd()) {
			list.append(w);
		}
	}

	if (list.isEmpty()) {
		LOG_CERROR("engine") << "World not found for" << s << "players";
		m_config.world.name = QStringLiteral("");
		m_config.world.playerCount = 0;
		m_config.world.landList.clear();
		return;
	}

	const ConquestWorldHelper &world = list.at(QRandomGenerator::global()->bounded(list.size()));

	m_config.world.name = world.name;
	m_config.world.playerCount = s;


	// Create lands

	m_config.world.landList.clear();

	const auto infoIt = std::find_if(world.infoList.constBegin(), world.infoList.constEnd(),
									 [s](const ConquestWorldHelperInfo &i){
		return i.playerCount == s;
	});

	Q_ASSERT(infoIt != world.infoList.constEnd());

	for (const QString &id : infoIt->landIdList) {
		ConquestWorldData d;
		d.id = id;
		d.proprietor = -1;
		d.xp = LAND_XP;
		d.xpOnce = LAND_XP_ONCE;
		d.fortress = -1;

		m_config.world.landList.append(d);
	}

	LOG_CTRACE("engine") << "World prepared:" << m_config.world.name << m_config.world.playerCount;
}



/**
 * @brief ConquestEngine::pickLands
 */

void ConquestEngine::pickLands()
{
	LOG_CTRACE("engine") << "Pick lands";

	if (m_players.empty())
		return;

	QList<int> idxList;

	for (int i=0; i<m_config.world.landList.size(); ++i) {
		const auto &d = m_config.world.landList.at(i);
		if (d.proprietor == -1)
			idxList.append(i);
	}

	Q_ASSERT(!idxList.isEmpty());
	Q_ASSERT(idxList.size() > (qsizetype) m_players.size());

	for (auto &p : m_players) {
		const auto &idx = idxList.takeAt(QRandomGenerator::global()->bounded(idxList.size()));
		m_config.world.landList[idx].proprietor = p.playerId;
	}
}


/**
 * @brief ConquestEngine::prepareTurns
 */

void ConquestEngine::prepareTurns(const ConquestTurn::Stage &stage)
{
	static const std::map<int, std::vector<std::vector<int> > > turns = {
		// 2 players (4 x 4)

		{ 2, {
			  { 0, 1 },
			  { 1, 0 }
		  }
		},


		// 3 players (9 x 2)

		{ 3, {
			  { 0, 1, 2 },
			  { 2, 0, 1 },
			  { 1, 2, 0 },
		  }
		},


		// 4 players (16 x 1)

		{ 4, {
			  { 0, 1, 2, 3 },
			  { 3, 0, 1, 2 },
			  { 2, 3, 0, 1 },
			  { 1, 2, 3, 0 },
		  }
		},
	};

	LOG_CTRACE("engine") << "Prepare turns" << stage;
	m_config.turnList.clear();
	m_config.currentTurn = -1;

	const auto &pSize = m_config.order.size();

	if (pSize < 2 || pSize > 4)
		return;

	const auto &list = turns.at(pSize);


	if (stage == ConquestTurn::StagePick) {
		for (const auto &turnList : list) {
			for (auto it = turnList.cbegin(); it != turnList.cend(); ++it) {
				ConquestTurn turn;
				turn.player = m_config.order.at(*it);
				if (it == turnList.cbegin())
					turn.subStage = ConquestTurn::SubStageUserAnswer;
				else
					turn.subStage = ConquestTurn::SubStageUserSelect;
				m_config.turnList.append(turn);
			}
		}
	} else if (stage == ConquestTurn::StageBattle) {
		int count = 0;

		switch (pSize) {
			case 2: count = 2; break;			//4
			case 3: count = 2; break;
			case 4: count = 1; break;
			default:
				count = 1;
				break;
		}

		for (int i=0; i<count; ++i) {
			for (const auto &turnList : list) {
				for (const int &id : turnList) {
					ConquestTurn turn;
					turn.player = m_config.order.at(id);
					turn.subStage = ConquestTurn::SubStageUserSelect;
					m_config.turnList.append(turn);
				}
			}
		}
	} else if (stage == ConquestTurn::StageLastRound) {
		// GET XP ORDERED LIST

		for (const int &id : m_config.order) {
			ConquestTurn turn;
			turn.player = id;
			turn.subStage = ConquestTurn::SubStageUserSelect;
			m_config.turnList.append(turn);
		}
	}
}



/**
 * @brief ConquestEngine::preparePlayerOrder
 */

void ConquestEngine::preparePlayerOrder()
{
	LOG_CTRACE("engine") << "Prepare player order";

	m_config.order.clear();

	if (m_players.empty())
		return;

	QList<int> tmp;

	for (const ConquestPlayer &p : m_players)
		tmp.append(p.playerId);

	while (!tmp.isEmpty())
		m_config.order.append(tmp.takeAt(QRandomGenerator::global()->bounded(tmp.size())));

	LOG_CTRACE("engine") << "Player order:" << m_config.order;
}



/**
 * @brief ConquestEngine::checkTurn
 */

void ConquestEngine::checkTurn()
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay)
		return;

	LOG_CTRACE("engine") << "Check turn" << m_config.currentTurn << m_config.currentStage;

	if (m_config.currentTurn < 0 || m_config.currentTurn >= m_config.turnList.size()) {
		LOG_CWARNING("engine") << "Invalid turn:" << m_config.currentTurn;
		return;
	}

	const ConquestTurn &turn = m_config.turnList.at(m_config.currentTurn);

	if (const auto &t = currentTick(); t == 0 || turn.subStageEnd == 0 || turn.subStageEnd > t)
		return;

	LOG_CERROR("engine") << "NEXT" << currentTick() << turn.subStageEnd;

	nextSubStage();
}





/**
 * @brief ConquestEngine::nextSubStage
 */

void ConquestEngine::nextSubStage()
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay)
		return;

	LOG_CTRACE("engine") << "Next substage" << m_config.currentTurn << m_config.currentStage;

	bool run = false;

	switch (m_config.currentStage) {
		case ConquestTurn::StageInvalid:
			LOG_CERROR("engine") << "Invalid stage";
			run = false;
			break;

		case ConquestTurn::StagePick:
			run = nextPick(true);
			break;

		case ConquestTurn::StageBattle:
			run = nextBattle(true);
			break;

		case ConquestTurn::StageLastRound:
			break;

	}

	if (!run) {
		LOG_CERROR("engine") << "FINISH GAME";
		gameFinish(nullptr, {});
		return;
	}

	m_handler->engineTriggerEngine(this);
}



/**
 * @brief ConquestEngine::questionNext
 * @return
 */

bool ConquestEngine::questionNext()
{
	QMutexLocker locker(&m_engineMutex);

	LOG_CWARNING("engine") << "Question next";

	const QJsonValue &v = m_question->next();

	if (v.isNull()) {
		LOG_CERROR("engine") << "Empty question";
		return false;
	}

	m_config.currentQuestion = v.toObject();

	return true;
}


/**
 * @brief ConquestEngine::questionClear
 */

void ConquestEngine::questionClear()
{
	QMutexLocker locker(&m_engineMutex);

	LOG_CTRACE("engine") << "Question clear";

	m_config.currentQuestion = {};
}



/**
 * @brief ConquestEngine::nextPick
 * @return
 */

bool ConquestEngine::nextPick(const bool &subStage)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay || m_config.currentStage != ConquestTurn::StagePick)
		return false;

	LOG_CWARNING("engine") << "Next pick" << m_config.currentTurn << subStage;


	int nextTurn = m_config.currentTurn+1;

	if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size()) {
		ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

		if (turn.subStage == ConquestTurn::SubStageUserAnswer) {

			LOG_CTRACE("enginge") << "Change" << m_config.currentTurn << "ANSWER -> SELECT";

			turn.subStage = ConquestTurn::SubStageUserSelect;

			nextTurn = m_config.currentTurn;

			for (int i=m_config.currentTurn; i < m_config.turnList.size(); ++i) {
				ConquestTurn &t = m_config.turnList[i];

				if (t.subStage != ConquestTurn::SubStageUserSelect)
					break;

				LOG_CWARNING("engine") << "FIND Answer result ------" << t.player;

				for (const auto &a : turn.answerList) {
					LOG_CDEBUG("engine") << a.player << a.answer;
				}

				LOG_CWARNING("engine") << "------ FIND Answer result";

				const auto &answer = turn.answerGetSuccess(t.player);			// !!! turn -> mert oda mentjük mindet!

				if (!answer) {

					LOG_CTRACE("enginge") << "Change" << i << "PLAYER -> -1" << t.player;

					t.player = -1;
					t.subStage = ConquestTurn::SubStageFinished;
					t.subStageEnd = 0;
				} else {
					LOG_CTRACE("enginge") << "Answer" << i << "PLAYER" << t.player << answer.value();
				}
			}
		} else {
			turn.subStage = ConquestTurn::SubStageFinished;
			turn.subStageEnd = 0;
		}

	}


	LOG_CDEBUG("enginge") << "NEXT TURN" << nextTurn << " - size:" << m_config.turnList.size();

	// Next turn

	if (subStage && nextTurn >= 0 && nextTurn < m_config.turnList.size()) {
		for (; nextTurn < m_config.turnList.size(); ++nextTurn) {
			ConquestTurn &t = m_config.turnList[nextTurn];

			if (t.subStage == ConquestTurn::SubStageUserAnswer) {
				questionClear();

				if (!questionNext())
					return false;

				t.subStageEnd = currentTick() + MSEC_ANSWER;
				m_config.currentTurn = nextTurn;

				LOG_CTRACE("enginge") << "QUESTION TURN" << m_config.currentTurn << t.subStage << "PLAYER" << t.player;

				return true;
			}

			if (t.player == -1)
				continue;

			const QStringList &lands = getPickableLands(t.player);

			if (lands.isEmpty()) {
				LOG_CTRACE("engine") << "No more pickable lands for player" << t.player;
				break;
			}

			t.canPick = lands;
			t.subStageEnd = currentTick() + MSEC_SELECT;
			m_config.currentTurn = nextTurn;

			LOG_CTRACE("enginge") << "OTHER TURN" << m_config.currentTurn << t.subStage << "PLAYER" << t.player;

			return true;
		}
	}


	if (auto it = std::find_if(m_config.world.landList.cbegin(), m_config.world.landList.cend(),
							   [](const ConquestWorldData &d) {
							   return d.proprietor == -1;
}); it == m_config.world.landList.cend()) {
		LOG_CWARNING("engine") << "No more lands";
		m_config.currentStage = ConquestTurn::StageBattle;
		m_config.currentTurn = -1;

		return nextBattle(false);
	}

	prepareTurns(ConquestTurn::StagePick);

	if (m_config.turnList.empty()) {
		LOG_CERROR("engine") << "Invalid turnList";
		return false;
	}

	return nextPick(true);

}




/**
 * @brief ConquestEngine::nextBattle
 * @param subStage
 * @return
 */

bool ConquestEngine::nextBattle(const bool &subStage)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay || m_config.currentStage != ConquestTurn::StageBattle)
		return false;

	LOG_CTRACE("engine") << "Next battle" << subStage;


	if (subStage) {
		if (m_config.currentTurn < 0 || m_config.currentTurn >= m_config.turnList.size()) {
			LOG_CERROR("engine") << "Invalid turn" << m_config.currentTurn << m_config.currentStage;
			return false;
		}

		ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

		if (turn.subStage == ConquestTurn::SubStageUserSelect) {
			if (turn.pickedId.isEmpty()) {
				turn.subStage = ConquestTurn::SubStageFinished;
				turn.subStageEnd = currentTick() + MSEC_FINISH_WAIT;
				return true;
			}

			if (!questionNext())
				return false;

			turn.subStage = ConquestTurn::SubStageUserAnswer;
			turn.subStageEnd = currentTick() + MSEC_ANSWER;

			return true;
		}

		if (turn.subStage == ConquestTurn::SubStageUserAnswer) {
			turn.subStage = ConquestTurn::SubStageFinished;
			turn.subStageEnd = currentTick() + MSEC_FINISH;

			return true;
		}

		if (turn.subStage == ConquestTurn::SubStageFinished) {
			turn.subStage = ConquestTurn::SubStageInvalid;
			turn.subStageEnd = 0;

			return nextBattle(false);
		}

		LOG_CERROR("engine") << "Invalid substage" << m_config.currentTurn << m_config.currentStage << turn.subStage;
		return false;
	}



	int nextIdx = m_config.currentTurn+1;

	if (nextIdx < 0 || nextIdx >= m_config.turnList.size()) {
		LOG_CINFO("engine") << "Game finished";
		return false;
	}

	questionClear();

	ConquestTurn &turn = m_config.turnList[nextIdx];
	turn.subStage = ConquestTurn::SubStageUserSelect;
	turn.subStageEnd = currentTick() + MSEC_SELECT;
	turn.pickedId.clear();
	m_config.currentTurn = nextIdx;

	return true;
}



/**
 * @brief ConquestEngine::nextLastRound
 * @param subStage
 * @return
 */

bool ConquestEngine::nextLastRound(const bool &subStage)
{
	return nextBattle(subStage);
}



/**
 * @brief ConquestEngine::getPickableLands
 * @return
 */

QStringList ConquestEngine::getPickableLands(const int &playerId)
{
	QMutexLocker locker(&m_engineMutex);

	QStringList list;

	for (const auto &l : m_config.world.landList) {
		if (l.proprietor == playerId)
			continue;

		if (m_config.currentStage == ConquestTurn::StagePick && l.proprietor != -1)
			continue;

		// ADJACENCY CHECK

		// if (not adjacent)
		//		continue

		list.append(l.id);
	}


	// Without adjacency check

	if (list.isEmpty()) {
		for (const auto &l : m_config.world.landList) {
			if (l.proprietor == playerId)
				continue;

			if (m_config.currentStage == ConquestTurn::StagePick && l.proprietor != -1)
				continue;

			list.append(l.id);
		}
	}

	return list;
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
	const qint64 &now = QDateTime::currentSecsSinceEpoch();

	if (m_lastRequest > 0 && now-m_lastRequest < 5)
		return;

	m_worker.execInThread([this, now]{
		QMutexLocker locker(&m_mutex);
		LOG_CTRACE("engine") << "Check questions" << m_questionArray;

		if (m_questionArray.size() >= 6)
			return;

		auto *ws = m_engine->m_hostStream;

		if (!ws) {
			LOG_CINFO("engine") << "NO WS" << ws;
			return;
		}

		LOG_CINFO("engine") << "REQUEST" << ws;
		m_engine->sendStreamJson(ws, QJsonObject{
									 { QStringLiteral("cmd"), QStringLiteral("questionRequest") }
								 });
		m_lastRequest = now;

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

		for (const QJsonValue &v : list)
			m_questionArray.append(v);

		LOG_CINFO("engine") << "Upload questions" << list << "=>" << m_questionArray;
	});
}




/**
 * @brief ConquestQuestion::next
 * @return
 */

QJsonValue ConquestQuestion::next()
{
	QDefer ret;

	QJsonValue value = QJsonValue::Null;

	m_worker.execInThread([this, &value, ret]() mutable {
		QMutexLocker locker(&m_mutex);

		if (!m_questionArray.isEmpty())
			value = m_questionArray.takeAt(0);

		LOG_CINFO("engine") << "Get next question" << value << m_questionArray;

		ret.resolve();
	});

	QDefer::await(ret);

	return value;
}



/**
 * @brief ConquestQuestion::hasQuestion
 * @return
 */

bool ConquestQuestion::hasQuestion()
{
	QDefer ret;

	m_worker.execInThread([this, ret]() mutable {
		QMutexLocker locker(&m_mutex);
		LOG_CTRACE("engine") << "Check existing questions" ;

		if (!m_questionArray.isEmpty())
			ret.resolve();
		else
			ret.reject();
	});

	QDefer::await(ret);

	return ret.state() == RESOLVED;
}
