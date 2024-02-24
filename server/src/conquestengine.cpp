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
#include "userapi.h"
#include "websocketstream.h"
#include "enginehandler.h"
#include "generalapi.h"
#include "webserver.h"

#define GAME_PREPARE_INTERVAL_MSEC		2500
#define STATE_SEND_INTERVAL_MSEC		3000

int ConquestEngine::m_nextId = 0;
QList<ConquestWorldHelper> ConquestEngine::m_helper = {};



/**
 * @brief ConquestEngine::ConquestEngine
 * @param handler
 * @param parent
 */

ConquestEngine::ConquestEngine(EngineHandler *handler, QObject *parent)
	: AbstractEngine(EngineConquest, handler, parent)
{
	LOG_CTRACE("engine") << "ConquestEngine created" << this;

	m_playerLimit = MAX_PLAYERS_COUNT;
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

	LOG_CTRACE("engine") << "ConquestEngine handle message" << cmd << message << id << engine;

	using fnDef = QJsonObject (ConquestEngine::*)(WebSocketStream *, const QJsonObject &);

	static QHash<std::string, fnDef> fMap = {
		{ "state", &ConquestEngine::cmdState },
		{ "start", &ConquestEngine::cmdStart },
		{ "prepare", &ConquestEngine::cmdPrepare },
		{ "enroll", &ConquestEngine::cmdEnroll },
		{ "leave", &ConquestEngine::cmdLeave },
		{ "play", &ConquestEngine::cmdPlay },
		{ "questionRequest", &ConquestEngine::cmdQuestionRequest },
		{ "pick", &ConquestEngine::cmdPick },
		{ "answer", &ConquestEngine::cmdAnswer },
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
		ret = cmdList(stream, obj, handler);
	} else if (cmd == QStringLiteral("connect")) {
		if (engine ){
			ret = {
				{ QStringLiteral("error"), QStringLiteral("engine already connected") }
			};
		} else {
			const bool &forced = obj.value(QStringLiteral("forced")).toBool(false);
			ret = cmdConnect(stream, handler, id, forced);
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

std::shared_ptr<ConquestEngine> ConquestEngine::createEngine(WebSocketStream *stream, EngineHandler *handler)
{
	if (!handler)
		return {};

	increaseNextId();

	LOG_CDEBUG("engine") << "Create ConquestEngine" << m_nextId << stream;

	auto ptr = std::make_shared<ConquestEngine>(handler);

	ptr->setId(m_nextId);

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

		if (ConquestEngine *e = qobject_cast<ConquestEngine*>(ptr.lock().get()); e) {
			if (e->config().gameState != ConquestConfig::StateConnect &&
					e->playerFind(stream->credential().username()) == e->m_players.cend()) {
				LOG_CWARNING("engine") << "Engine state conflict:" << id << e->config().gameState << stream->credential().username();
				return std::weak_ptr<AbstractEngine>();
			}
		}

		handler->websocketEngineLink(stream, ptr.lock());
	}

	return ptr;
}



/**
 * @brief ConquestEngine::restoreEngines
 * @return
 */

int ConquestEngine::restoreEngines(ServerService *service)
{
	Q_ASSERT(service);

	LOG_CDEBUG("engine") << "Restore ConquestEngines";
	int count = 0;

	QDir dir = service->settings()->dataDir();
	static const QString subdir = QStringLiteral("engineData");

	QDirIterator it(dir.absoluteFilePath(subdir), {QStringLiteral("conquest.*.dat")}, QDir::Files|QDir::Readable);

	int nextId = 0;

	static const QRegularExpression exp(QStringLiteral(R"(/conquest\.(\d+)\.dat$)"));

	while (it.hasNext()) {
		const QString &f = it.next();

		const QRegularExpressionMatch &match = exp.match(f);

		if (!match.hasMatch())
			continue;

		const int &id = match.captured(1).toInt();

		const auto &data = Utils::fileToJsonObject(f);

		if (!data) {
			LOG_CWARNING("engine") << "Invalid content:" << qPrintable(f);
			continue;
		}

		if (id > nextId)
			nextId = id;

		auto ptr = std::make_shared<ConquestEngine>(service->engineHandler());

		ptr->setId(id);
		ptr->engineRestore(*data);
		service->engineHandler()->engineAdd(ptr);

		++count;
		LOG_CINFO("engine") << "ConquestEngine" << id << "restored";
	}

	setNextId(nextId);

	return count;
}


/**
 * @brief ConquestEngine::loadWorldDataFromResource
 * @param prefix
 * @return
 */

void ConquestEngine::loadWorldDataFromResource(const QString &path)
{
	QDirIterator it2(path, { QStringLiteral("data.json") }, QDir::Files, QDirIterator::Subdirectories);

	while (it2.hasNext()) {
		const QString &jsonFile = it2.next();

		const QString &name = jsonFile.section('/',-2,-2);

		const auto &data = Utils::fileToJsonObject(jsonFile);

		if (!data) {
			LOG_CERROR("engine") << "Invalid JSON content:" << qPrintable(jsonFile);
			continue;
		}

		const QJsonObject &orig = data->value(QStringLiteral("orig")).toObject();

		ConquestWorldHelper world;

		world.fromJson(orig);

		world.name = name;
		world.landIdList = data->value(QStringLiteral("lands")).toObject().keys();
		world.adjacency = orig.value(QStringLiteral("adjacency")).toObject();

		m_helper.append(world);

		LOG_CTRACE("engine") << "ConquestWorld registered:" << name << world.description << qPrintable(jsonFile);
	}
}



/**
 * @brief ConquestEngine::gameFinish
 * @param stream
 * @param message
 * @param handler
 * @return
 */

void ConquestEngine::gameFinish(const bool &hasError)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay)
		return;

	if (hasError)
		LOG_CWARNING("engine") << "Finish conquest game:" << m_id << "with error";
	else
		LOG_CDEBUG("engine") << "Finish conquest game:" << m_id;

	const qint64 elapsed = currentTick();

	if (m_elapsedTimer.isValid())
		m_elapsedTimer.invalidate();

	m_config.gameState = hasError ? ConquestConfig::StateError : ConquestConfig::StateFinished;

	const auto api = qobject_cast<UserAPI*>(m_service->webServer().lock().get()->handler()->api("user"));

	if (!api) {
		LOG_CERROR("engine") << "API not found: user";
		m_config.gameState = ConquestConfig::StateError;
	}


	UserAPI::UserGame g;

	g.map = m_config.mapUuid;
	g.mission = m_config.missionUuid;
	g.level = m_config.missionLevel;
	g.deathmatch = false;
	g.mode = GameMap::Conquest;


	static const QJsonObject inventory;
	static const QJsonArray statistics;
	///const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();

	for (auto &p : m_players) {
		//LOG_CTRACE("engine") << "PLAYER" << p.username << p.xp << p.hp << "GameId" << p.gameId;

		if (p.gameId == -1)
			continue;

		p.success = m_config.playerResult(p);

		if (api) {
			bool okPtr = false;
			api->gameFinish(p.username, p.gameId, g, inventory, statistics, p.success, p.xp, elapsed, &okPtr);
			if (!okPtr) {
				LOG_CERROR("engine") << "Game finish error" << p.gameId << p.username;
			} else {
				p.gameId = -1;
			}
		}
	}



	if (const QString &filename = engineBackupFile(); QFile::exists(filename)) {
		LOG_CTRACE("engine") << "Remove backup engine file:" << m_id << qPrintable(filename);
		QFile::remove(filename);
	}

	m_handler->engineTriggerEngine(this);
}




/**
 * @brief ConquestEngine::cmdStart
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdStart(WebSocketStream *stream, const QJsonObject &)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StateConnect && m_config.gameState != ConquestConfig::StateFinished)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};


	if (stream != m_hostStream)
		return {
			{ QStringLiteral("error"), QStringLiteral("permission denied") }
		};


	const int pSize = m_players.size();

	if (pSize < 2)
		return {
			{ QStringLiteral("error"), QStringLiteral("insufficient players") }
		};

	if (m_config.gameState == ConquestConfig::StateFinished) {
		int w = 0;

		for (auto &p : m_players) {
			if (p.prepared)
				++w;
		}

		if (w) {
			LOG_CTRACE("engine") << "Wait for" << w << "players to be unprepared";

			return {
				{ QStringLiteral("wait"), w }
			};
		}

		m_config.reset();
	}

	for (auto &p : m_players) {
		p.reset();
	}

	QJsonArray wList;

	for (const ConquestWorldHelper &w : m_helper) {
		if (w.playerCount == 0 || w.playerCount == pSize) {
			QJsonObject o;
			o[QStringLiteral("name")] = w.name;
			o[QStringLiteral("description")] = w.description.isEmpty() ? w.name : w.description;
			wList.append(o);
		}
	}

	if (wList.isEmpty()) {
		LOG_CERROR("engine") << "No available world";
		m_config.world.name = QStringLiteral("");
		m_config.world.landList.clear();
		m_config.gameState = ConquestConfig::StateError;
		m_handler->engineTriggerEngine(this);

		return {
			{ QStringLiteral("error"), QStringLiteral("no world") }
		};
	}

	m_config.gameState = ConquestConfig::StateWorldSelect;

	m_handler->engineTriggerEngine(this);

	return {
		{ QStringLiteral("worldList"), wList }
	};
}



/**
 * @brief ConquestEngine::cmdPrepare
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdPrepare(WebSocketStream *stream, const QJsonObject &message)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState == ConquestConfig::StateFinished && message.value(QStringLiteral("unprepare")).toBool()) {
		bool modified = false;

		for (auto &p : m_players) {
			if (p.stream == stream) {
				p.prepared = false;
				modified = true;
			}
		}

		if (modified)
			m_handler->engineTriggerEngine(this);

		return {};
	}

	if (m_config.gameState != ConquestConfig::StatePrepare && m_config.gameState != ConquestConfig::StatePlay)
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
 * @brief ConquestEngine::cmdPlay
 * @param stream
 * @param message
 * @param handler
 * @return
 */

QJsonObject ConquestEngine::cmdPlay(WebSocketStream *stream, const QJsonObject &message)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState == ConquestConfig::StatePlay) {
		triggerEvent();
		return {};
	}

	if (m_config.gameState == ConquestConfig::StateWorldSelect && m_hostStream == stream) {
		const QString &world = message.value(QStringLiteral("world")).toString();

		if (world.isEmpty())
			return {
				{ QStringLiteral("error"), QStringLiteral("world missing") }
			};

		if (!prepareWorld(world)) {
			return {
				{ QStringLiteral("error"), QStringLiteral("invalid world") }
			};
		};

		preparePlayerOrder();
		pickLands();

		m_config.gameState = ConquestConfig::StatePrepare;
	}

	if (m_config.gameState != ConquestConfig::StatePrepare)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};


	int w = 0;
	bool modified = false;

	for (auto &p : m_players) {
		if (p.stream == stream) {
			p.prepared = true;
			modified = true;
		} else if (!p.prepared)
			++w;
	}

	if (w) {
		LOG_CTRACE("engine") << "Wait for" << w << "players to be prepared";

		if (modified)
			m_handler->engineTriggerEngine(this);

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

	LOG_CINFO("engine") << "Play ConquestEngine" << m_id;

	const auto api = qobject_cast<UserAPI*>(m_service->webServer().lock().get()->handler()->api("user"));

	if (!api) {
		LOG_CERROR("engine") << "API not found: user";
		gameFinish(true);
		return {
			{ QStringLiteral("error"), QStringLiteral("internal error") }
		};
	}


	QMutexLocker pLocker(&m_playerMutex);

	if (m_config.gameState == ConquestConfig::StatePlay)
		return {};

	UserAPI::UserGame g;

	g.map = m_config.mapUuid;
	g.mission = m_config.missionUuid;
	g.level = m_config.missionLevel;
	g.deathmatch = false;
	g.mode = GameMap::Conquest;


	static const QJsonObject inventory;
	///const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();


	m_startedAt = QDateTime::currentMSecsSinceEpoch();
	m_elapsedTimer.start();
	m_config.currentTurn = -1;
	m_config.gameState = ConquestConfig::StatePlay;
	m_config.currentStage = ConquestTurn::StagePrepare;

	for (auto &p : m_players) {
		p.hp = m_config.startHp;

		int gameId = -1;
		api->gameCreate(p.username, m_config.campaign, g, inventory, &gameId);

		if (gameId == -1) {
			LOG_CERROR("engine") << "ConquestEngine play error" << m_id;
			gameFinish(true);
			return {
				{ QStringLiteral("error"), QStringLiteral("internal error") }
			};
		}

		p.gameId = gameId;
	}

	m_handler->engineTriggerEngine(this);

	return {
		{ QStringLiteral("play"), true }
	};
}


/**
 * @brief ConquestEngine::userCanConnect
 * @param username
 * @return
 */

bool ConquestEngine::userCanConnect(const QString &username) const
{
	if (m_config.gameState == ConquestConfig::StateInvalid || m_config.gameState == ConquestConfig::StateConnect)
		return true;

	if (playerExists(username))
		return true;

	return false;
}



/**
 * @brief ConquestEngine::canDelete
 * @param useCount
 * @return
 */

bool ConquestEngine::canDelete(const int &useCount)
{
	return (useCount == 1 && (m_id == -1 ||
							  m_config.gameState == ConquestConfig::StateFinished ||
							  m_config.gameState == ConquestConfig::StateInvalid ||
							  m_config.gameState == ConquestConfig::StateConnect ||
							  m_config.gameState == ConquestConfig::StateError
							  ));
}



/**
 * @brief ConquestEngine::timerTick
 */

void ConquestEngine::timerTick()
{
	if (m_config.gameState == ConquestConfig::StateFinished || m_config.gameState == ConquestConfig::StateError ||
			m_config.gameState == ConquestConfig::StateWorldSelect)
		return;

	LOG_CTRACE("engine") << "Timer tick" << currentTick() << this;

	if (m_config.gameState == ConquestConfig::StatePrepare || m_config.gameState == ConquestConfig::StatePlay) {
		if (m_config.gameState == ConquestConfig::StatePlay && m_config.currentStage <= ConquestTurn::StagePick &&
				currentTick() >= MSEC_GAME_TIMEOUT) {
			LOG_CINFO("engine") << "ConquestEngine timer timeout" << m_id;
			gameFinish(true);
			return;
		}

		m_question->check();
		if (checkTurn())
			return;
	}

	const qint64 msec = QDateTime::currentMSecsSinceEpoch();

	if (msec - m_lastStateSent > STATE_SEND_INTERVAL_MSEC) {
		m_handler->engineTriggerEngine(this);
		m_lastStateSent = msec;
	}
}



/**
 * @brief ConquestEngine::triggerEvent
 * @param stream
 */

void ConquestEngine::triggerEvent()
{
	LOG_CTRACE("engine") << "Stream trigger" << this << currentTick();

	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState == ConquestConfig::StatePlay) {
		engineBackup();
	}

	QJsonObject ret = m_config.toJson();

	ret.insert(QStringLiteral("cmd"), QStringLiteral("state"));
	ret.insert(QStringLiteral("engine"), m_id);
	ret.insert(QStringLiteral("interval"), m_service->mainTimerInterval());
	ret.insert(QStringLiteral("playerLimit"), (int) m_playerLimit);
	ret.insert(QStringLiteral("users"), playersToArray());

	if (m_startedAt != -1) {
		ret.insert(QStringLiteral("startedAt"), m_startedAt);
	}

	for (const auto &stream : m_streams) {

		if (m_elapsedTimer.isValid()) {
			ret.insert(QStringLiteral("tick"), currentTick());
		} else {
			ret.remove(QStringLiteral("tick"));
		}

		ret.insert(QStringLiteral("host"), (m_hostStream == stream ? true : false));
		const auto &player = playerFind(stream);

		if (player != m_players.end()) {
			ret.insert(QStringLiteral("playerId"), player->playerId);
		} else {
			ret.remove(QStringLiteral("playerId"));
		}

		const QByteArray &data = QJsonDocument(ret).toJson(QJsonDocument::Compact);
		const QByteArray &compressed = qCompress(data);

		stream->sendBinaryMessage(compressed);
	}
}


/**
 * @brief ConquestEngine::currentTick
 * @return
 */

qint64 ConquestEngine::currentTick() const
{
	if (m_elapsedTimer.isValid())
		return m_elapsedTimer.elapsed() + m_tickBegin;
	else
		return 0;
}


/**
 * @brief ConquestEngine::playerEnroll
 * @param stream
 * @return
 */

int ConquestEngine::playerEnroll(WebSocketStream *stream, const QString &character)
{
	QMutexLocker locker(&m_engineMutex);

	if (!stream)
		return -1;

	if (m_config.gameState != ConquestConfig::StateConnect && m_config.gameState != ConquestConfig::StateFinished)
		return -1;

	if (character.isEmpty())
		return -1;

	if (const int &id = playerGetId(stream->credential().username()); id != -1) {
		LOG_CWARNING("engine") << "Player already enrolled:" << m_id << qPrintable(stream->credential().username());
		return id;
	}

	if (m_playerLimit > 0 && (m_players.size() >= m_playerLimit || m_players.size() >= MAX_PLAYERS_COUNT)) {
		LOG_CWARNING("engine") << "Players number full" << m_id;
		return -1;
	}

	if (!m_characterListHelper.characterList.contains(character)) {
		LOG_CWARNING("engine") << "Invalid character" << character;
		return -1;
	}

	int playerId = 1;
	bool characterFree = true;

	for (const auto &p : m_players) {
		playerId = std::max(p.playerId+1, playerId);
		if (p.character == character)
			characterFree = false;
	}

	if (!characterFree) {
		LOG_CWARNING("engine") << "Character already used:" << character << qPrintable(stream->credential().username());
		return -1;
	}

	const QString &username = stream->credential().username();

	LOG_CDEBUG("engine") << "Enroll player" << playerId << "to engine" << m_id << qPrintable(username);

	const QString &fullNickName = userGetFullNickName(username);

	ConquestEnginePlayer player(playerId, username, fullNickName, stream);
	player.character = character;

	m_players.push_back(player);

	m_handler->engineTriggerEngine(this);

	return playerId;
}



/**
 * @brief ConquestEngine::playerLeave
 * @param stream
 * @return
 */

bool ConquestEngine::playerLeave(WebSocketStream *stream, const bool &forced)
{
	QMutexLocker locker(&m_engineMutex);

	if (!stream)
		return false;

	if (auto it = playerFind(stream); it != m_players.end()) {
		LOG_CDEBUG("engine") << "Leave player" << it->playerId << stream << qPrintable(it->username);

		if ((m_config.gameState == ConquestConfig::StatePlay ||
			 m_config.gameState == ConquestConfig::StatePrepare ||
			 m_config.gameState == ConquestConfig::StateWorldSelect
			 )
				&& !forced) {
			LOG_CWARNING("engine") << "Player leave must be forced in game state" << m_config.gameState;
			return false;
		}

		//id = it->playerId;
		m_players.erase(it);
	} else {
		LOG_CDEBUG("engine") << "Leave engine" << m_id << stream;
	}

	m_handler->websocketEngineUnlink(stream, this);

	m_handler->engineTriggerEngine(this);

	return true;
}




/**
 * @brief ConquestEngine::playerConnectStream
 * @param playerId
 * @param stream
 * @return
 */

bool ConquestEngine::playerConnectStream(const int &playerId, WebSocketStream *stream)
{
	QMutexLocker locker(&m_engineMutex);

	for (ConquestEnginePlayer &p : m_players) {
		if (p.playerId == playerId) {
			if (p.stream == nullptr) {
				p.stream = stream;
				p.online = stream ? true : false;
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
	QMutexLocker locker(&m_engineMutex);

	for (auto &p : m_players) {
		if ((playerId == -1 && p.stream == stream) || (p.playerId == playerId)) {
			p.stream = nullptr;
			p.online = false;
		}
	}
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

	QMutexLocker locker(&m_engineMutex);

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
			LOG_CINFO("engine") << "ConquestEngine all stream dismissed" << m_id;
			setHostStream(nullptr);
			gameFinish(true);
			///m_handler->engineRemoveUnused();
			setId(-1);
			return;
		} else {
			LOG_CDEBUG("engine") << "Next host stream:" << next;
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

QJsonObject ConquestEngine::cmdList(WebSocketStream *stream, const QJsonObject &message, EngineHandler *handler)
{
	Q_ASSERT(stream);

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
		if (e->id() == -1 ||
				e->config().mapUuid != config.mapUuid ||
				e->config().missionUuid != config.missionUuid ||
				e->config().missionLevel != config.missionLevel ||
				!e->userCanConnect(stream->credential().username())
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
	Q_ASSERT(stream);

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

	if (config.campaign <= 0)
		return {
			{ QStringLiteral("error"), QStringLiteral("missing campaign") }
		};


	ConquestWorldListHelper chHelper;
	chHelper.fromJson(message);

	if (chHelper.characterList.isEmpty())
		return {
			{ QStringLiteral("error"), QStringLiteral("missing characterList") }
		};


	const std::shared_ptr<ConquestEngine> &ptr = createEngine(stream, handler);
	auto engine = ptr.get();

	if (!engine) {
		return {
			{ QStringLiteral("error"), QStringLiteral("internal error") }
		};
	}

	const QString &fullname = engine->userGetFullNickName(stream->credential().username());

	engine->setOwner(fullname);
	engine->m_config.mapUuid = config.mapUuid;
	engine->m_config.missionUuid = config.missionUuid;
	engine->m_config.missionLevel = config.missionLevel;
	engine->m_config.campaign = config.campaign;

	if (config.startHp <= 0) {
		if (config.missionLevel >= 3)
			config.startHp = 4;
		else if (config.missionLevel == 2)
			config.startHp = 3;
		else
			config.startHp = 2;
	}

	engine->m_config.startHp = config.startHp;
	engine->m_characterListHelper = chHelper;

	if (stream) {
		engine->setHostStream(stream);
		engine->m_config.gameState = ConquestConfig::StateConnect;
		handler->websocketEngineLink(stream, ptr);
		if (engine->playerEnroll(stream, message.value(QStringLiteral("character")).toString()) == -1) {
			return {
				{ QStringLiteral("error"), QStringLiteral("enroll failed") }
			};
		}
	}

	handler->engineAdd(ptr);


	LOG_CINFO("engine") << "ConquestEngine created:" << engine->id() << qPrintable(stream->credential().username());

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

QJsonObject ConquestEngine::cmdConnect(WebSocketStream *stream, EngineHandler *handler, const int &id, const bool &forced)
{
	Q_ASSERT(stream);

	LOG_CTRACE("engine") << "Connect to ConquestEngine:" << id << "forced:" << forced;

	const auto &ptr = handler->engineGet(AbstractEngine::EngineConquest, id);

	if (ptr.expired()) {
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid engine") }
		};
	}

	ConquestEngine *engine = qobject_cast<ConquestEngine*>(ptr.lock().get());
	QMutexLocker locker(&engine->m_engineMutex);

	if (!engine->userCanConnect(stream->credential().username())) {
		return {
			{ QStringLiteral("error"), QStringLiteral("connection disabled") }
		};
	}

	handler->websocketEngineLink(stream, ptr.lock());

	auto it = engine->playerFind(stream->credential().username());

	if (it != engine->m_players.end()) {
		LOG_CDEBUG("engine") << "Player already enrolled, connect streams:" << engine->id() << qPrintable(stream->credential().username());

		if (it->stream) {
			if (forced) {
				LOG_CINFO("engine") << "Player stream replace:" << engine->id() << qPrintable(it->username) << it->stream;
				it->stream = nullptr;
				it->online = false;
			} else {
				LOG_CWARNING("engine") << "Player stream already connected:" << engine->id() << qPrintable(it->username) << it->stream;
				return {
					{ QStringLiteral("error"), QStringLiteral("already connected") }
				};
			}
		}

		it->stream = stream;
		it->online = stream ? true : false;

		if (engine->m_elapsedTimerStartRequired) {
			LOG_CINFO("engine") << "Continue ConquestEngine timer" << id;
			engine->m_elapsedTimerStartRequired = false;
			engine->m_elapsedTimer.start();
		}
	}

	if (!engine->hostStream())
		engine->setHostStream(stream);

	return {
		{ QStringLiteral("engine"), id },
		{ QStringLiteral("users"), engine->playersToArray() }
	};
}



/**
 * @brief ConquestEngine::engineBackup
 */

void ConquestEngine::engineBackup()
{
	const QString &filename = engineBackupFile(true);

	LOG_CTRACE("engine") << "Backup engine" << m_id << qPrintable(filename);

	QMutexLocker locker(&m_engineMutex);

	QJsonObject ret = m_config.toJson();

	ret.insert(QStringLiteral("owner"), m_owner);
	ret.insert(QStringLiteral("playerLimit"), (int) m_playerLimit);
	ret.insert(QStringLiteral("startedAt"), m_startedAt);
	ret.insert(QStringLiteral("tick"), currentTick());

	QJsonArray players;

	for (const auto &p : m_players)
		players.append(p.toJson());

	ret.insert(QStringLiteral("users"), players);

	ret.insert(QStringLiteral("questionArray"), m_question->array());
	ret.insert(QStringLiteral("adjacency"), ConquestWorldHelper::adjacencyToJson(m_config.world.adjacencyMatrix));

#ifdef QT_NO_DEBUG
	const QByteArray &content = QJsonDocument(ret).toJson(QJsonDocument::Compact);
#else
	const QByteArray &content = QJsonDocument(ret).toJson(QJsonDocument::Indented);
#endif

	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) {
		LOG_CERROR("engine") << "Engine backup error:" << qPrintable(filename);
		return;
	}

	f.write(content);
	f.flush();
	f.close();
}


/**
 * @brief ConquestEngine::engineRestore
 * @param data
 * @return
 */

bool ConquestEngine::engineRestore(const QJsonObject &data)
{
	LOG_CINFO("engine") << "Restore engine from data" << m_id;

	ConquestConfig c;
	c.fromJson(data);

	ConquestWorldHelper::adjacencyToMatrix(data.value(QStringLiteral("adjacency")).toObject(),
										   &c.world.adjacencyMatrix);

	setConfig(c);

	setOwner(data.value(QStringLiteral("owner")).toString());
	setPlayerLimit(data.value(QStringLiteral("playerLimit")).toInt());
	m_startedAt = data.value(QStringLiteral("startedAt")).toInteger();
	m_tickBegin = data.value(QStringLiteral("tick")).toInteger();

	const auto &list = data.value(QStringLiteral("users")).toArray();

	for (const QJsonValue &v : list) {
		ConquestEnginePlayer player;
		player.fromJson(v.toObject());
		m_players.push_back(player);
	}

	m_question->upload(data.value(QStringLiteral("questionArray")).toArray());

	m_elapsedTimerStartRequired = true;

	return true;
}



/**
 * @brief ConquestEngine::engineBackupFile
 * @param createDir
 * @return
 */

QString ConquestEngine::engineBackupFile(const ServerService *service, const int &id, const bool &createDir)
{
	Q_ASSERT(service);

	QDir dir = service->settings()->dataDir();
	static const QString subdir = QStringLiteral("engineData");

	if (createDir && !QFile::exists(dir.absoluteFilePath(subdir))) {
		LOG_CINFO("engine") << "Create path:" << qPrintable(dir.absoluteFilePath(subdir));
		dir.mkdir(subdir);
	}

	return dir.absoluteFilePath(subdir+QStringLiteral("/conquest.%1.dat").arg(id));
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

QJsonObject ConquestEngine::cmdEnroll(WebSocketStream *stream, const QJsonObject &message)
{
	QMutexLocker locker(&m_engineMutex);

	const int &id = playerEnroll(stream, message.value(QStringLiteral("character")).toString());

	if (id == -1) {
		return {
			{ QStringLiteral("error"), QStringLiteral("enroll failed") }
		};
	}

	///m_handler->engineTriggerEngine(this);

	return {
		{ QStringLiteral("playerId"), id }
	};
}



/**
 * @brief ConquestEngine::cmdLeave
 * @param stream
 * @param message
 * @return
 */

QJsonObject ConquestEngine::cmdLeave(WebSocketStream *stream, const QJsonObject &)
{
	QMutexLocker locker(&m_engineMutex);

	if (!playerLeave(stream, false)) {
		return {
			{ QStringLiteral("error"), QStringLiteral("leave failed") }
		};
	}

	m_handler->engineRemoveUnused();

	///m_handler->engineTriggerEngine(this);

	return {
		{ QStringLiteral("leaved"), true }
	};
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

	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePrepare && m_config.gameState != ConquestConfig::StatePlay)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};


	if (stream != m_hostStream)
		return {
			{ QStringLiteral("error"), QStringLiteral("permission denied") }
		};

	m_question->upload(message.value(QStringLiteral("list")).toArray());

	cmdPlay(stream, {});

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
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid stream") }
		};

	if (m_config.gameState != ConquestConfig::StatePlay)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};

	LOG_CTRACE("engine") << "Pick" << stream->credential().username() << message;

	QMutexLocker locker(&m_engineMutex);

	const auto &ptr = playerFind(stream);

	if (ptr == m_players.end())
		return {};

	if (const QString &landId = message.value(QStringLiteral("id")).toString();
			!m_config.landPick(landId, ptr->playerId, &*ptr)) {
		LOG_CERROR("engine") << "Land pick failed:" << m_id << landId << ptr->playerId;
		return {};
	}

	nextSubStage();

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
	if (m_config.gameState != ConquestConfig::StatePlay)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid state") }
		};

	if (!stream)
		return {
			{ QStringLiteral("error"), QStringLiteral("invalid stream") }
		};

	LOG_CTRACE("engine") << "Answer" << stream->credential().username() << message;

	QMutexLocker locker(&m_engineMutex);


	const auto &player = playerFind(stream);

	if (player == m_players.end())
		return {};


	if (m_config.currentStage == ConquestTurn::StageBattle) {
		if (m_config.currentTurn < 0 || m_config.currentTurn >= m_config.turnList.size())
			return {};

		const ConquestTurn &turn = m_config.turnList.at(m_config.currentTurn);

		if (const int &propPlayer = m_config.getPickedLandProprietor(m_config.currentTurn); propPlayer != -1) {
			if (player->playerId != propPlayer && player->playerId != turn.player) {
				return {
					{ QStringLiteral("error"), QStringLiteral("access denied") }
				};
			}
		} else {
			if (player->playerId != turn.player) {
				return {
					{ QStringLiteral("error"), QStringLiteral("access denied") }
				};
			}
		}
	}


	ConquestAnswer answer;
	answer.fromJson(message);
	answer.player = player->playerId;

	if (!m_config.playerAnswer(answer)) {
		LOG_CERROR("engine") << "Answer error:" << m_id << answer.player << answer.answer;
		return {};
	}

	const ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

	QList<int> tmp;

	if (m_config.currentStage == ConquestTurn::StagePick) {
		for (const ConquestPlayer &p : m_players)
			tmp.append(p.playerId);
	} else {
		tmp.append(turn.player);

		if (const int &propPlayer = m_config.getPickedLandProprietor(m_config.currentTurn); propPlayer != -1)
			tmp.append(propPlayer);
	}

	for (const ConquestAnswer &a : turn.answerList)
		tmp.removeAll(a.player);

	if (tmp.isEmpty())
		nextSubStage();

	return {
		{ QStringLiteral("status"), QStringLiteral("ok") }
	};
}




/**
 * @brief ConquestEngine::playerExists
 * @param username
 * @return
 */

bool ConquestEngine::playerExists(const QString &username) const
{
	return playerFind(username) != m_players.cend();
}





/**
 * @brief ConquestEngine::onPlayerPrepared
 */

void ConquestEngine::onPlayerPrepared(WebSocketStream *stream)
{
	if (!stream)
		return;

	LOG_CDEBUG("engine") << "Player prepared" << m_id << stream << qPrintable(stream->credential().username());

	cmdPlay(stream, {});
}



/**
 * @brief ConquestEngine::playersToArray
 * @return
 */

QJsonArray ConquestEngine::playersToArray() const
{
	QJsonArray players;

	for (const auto &p : m_players)
		players.append(p.toJson());

	return players;
}



/**
 * @brief ConquestEngine::userGetFullNickName
 * @param username
 * @return
 */

QString ConquestEngine::userGetFullNickName(const QString &username)
{
	QJsonObject userInfo;

	QDefer ret;
	m_service->databaseMainWorker()->execInThread([ret, this, &userInfo, username]() mutable {
		const auto api = m_service->webServer().lock().get()->handler()->api("general");
		if (!api) {
			LOG_CERROR("engine") << "API not found: general";
			ret.reject();
			return;
		}
		const auto &list = GeneralAPI::_user(api, username);

		if (!list || list->isEmpty()) {
			LOG_CERROR("engine") << "User not found:" << qPrintable(username);
			ret.reject();
			return;
		}

		userInfo = list->at(0).toObject();
		ret.resolve();
	});
	QDefer::await(ret);

	QString fullNickName;

	if (const QString &s = userInfo.value(QStringLiteral("nickname")).toString(); !s.isEmpty())
		fullNickName = s;
	else
		fullNickName = userInfo.value(QStringLiteral("familyName")).toString().append(QStringLiteral(" "))
					   .append(userInfo.value(QStringLiteral("givenName")).toString());

	return fullNickName;
}





/**
 * @brief ConquestEngine::prepareWorld
 */

bool ConquestEngine::prepareWorld(const QString &world)
{
	LOG_CTRACE("engine") << "Prepare world";

	QMutexLocker locker(&m_engineMutex);

	if (m_players.empty())
		return false;

	const int pSize = m_players.size();

	auto it = std::find_if(m_helper.constBegin(),
						   m_helper.constEnd(),
						   [world, pSize](const ConquestWorldHelper &w){
		return (w.name == world && (w.playerCount == 0 || w.playerCount == pSize));
	});

	if (it == m_helper.constEnd())  {
		LOG_CWARNING("engine") << "Invalid world:" << world;
		return false;
	}

	m_config.world.name = it->name;
	m_config.world.adjacencyMatrix = it->adjacencyToMatrix();


	// Create lands

	m_config.world.landList.clear();

	for (const QString &id : it->landIdList) {
		ConquestWorldData d;
		d.id = id;
		d.proprietor = -1;
		d.xp = LAND_XP;
		d.xpOnce = LAND_XP_ONCE;
		d.fortress = -1;

		m_config.world.landList.append(d);
	}

	LOG_CTRACE("engine") << "World prepared:" << m_config.world.name;

	return true;
}



/**
 * @brief ConquestEngine::pickLands
 */

void ConquestEngine::pickLands()
{
	LOG_CTRACE("engine") << "Pick lands";

	QMutexLocker locker(&m_engineMutex);

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
		int _index = 0;

		// Avoid fortress adjacency

		for (int i=0; i<10; ++i) {
			_index = QRandomGenerator::global()->bounded(idxList.size());
			const auto &idx = idxList.at(_index);
			const auto &land = m_config.world.landList.at(idx);
			const QStringList &list = m_config.world.adjacencyMatrix.value(land.id);
			bool hasFortressAdjacency = false;
			for (const QString &s : list) {
				const int &idx = m_config.world.landFind(s);
				if (idx == -1)
					continue;

				if (m_config.world.landList.at(idx).fortress != -1) {
					hasFortressAdjacency = true;
					break;
				}
			}
			if (!hasFortressAdjacency)
				break;

			if (i==9)
				LOG_CERROR("engine") << "Fortress adjacency check overrun";
		};

		const auto &idx = idxList.takeAt(_index);
		auto &land = m_config.world.landList[idx];
		land.proprietor = p.playerId;
		land.fortress = INITIAL_FORTRESS_COUNT;
		land.xp = 0;
		land.xpOnce = 0;
	}
}


/**
 * @brief ConquestEngine::prepareTurns
 */

void ConquestEngine::prepareTurns(const ConquestTurn::Stage &stage)
{
	static const std::map<int, std::vector<std::vector<int> > > turns = {
		// 2 players (8 x 2) -> land: 2 + 4x2

		{ 2, {
			  { 0, 1 },
			  { 1, 0 },
			  { 1, 0 },
			  { 0, 1 },
		  }
		},


		// 3 players (9 x 2) -> land: 3 + 3x3

		{ 3, {
			  { 0, 1, 2 },
			  { 2, 0, 1 },
			  { 1, 2, 0 },
		  }
		},


		// 4 players (16 x 1) -> land: 4 + 2x4

		{ 4, {
			  { 0, 1, 2, 3 },
			  { 3, 0, 1, 2 },
			  { 2, 3, 0, 1 },
			  { 1, 2, 3, 0 },
		  }
		},
	};

	LOG_CTRACE("engine") << "Prepare turns" << stage;

	QMutexLocker locker(&m_engineMutex);

	m_config.turnList.clear();
	m_config.currentTurn = -1;

	const auto &pSize = m_config.order.size();

	if (pSize < 2 || pSize > 4)
		return;

	const auto &list = turns.at(pSize);


	if (stage == ConquestTurn::StagePick) {
		int count = 0;

		switch (pSize) {
			case 2: count = 4; break;
			case 3: count = 3; break;
			case 4: count = 2; break;
			default:
				count = 1;
				break;
		}

		for (int i=0; i<(int)list.size() && i<count; ++i) {
			const auto &turnList = list.at(i);

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
			case 2: count = 2; break;
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
		std::vector<ConquestEnginePlayer> pList = m_players;

		std::sort(pList.begin(), pList.end(), [](const ConquestEnginePlayer &p1, const ConquestEnginePlayer &p2){
			return p1.xp > p2.xp;
		});

		for (const ConquestEnginePlayer &p : pList) {
			ConquestTurn turn;
			turn.player = p.playerId;
			turn.subStage = ConquestTurn::SubStageUserSelect;

			if (!m_config.checkPlayerLands(p.playerId))
				turn.player = -1;

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

	QMutexLocker locker(&m_engineMutex);

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

bool ConquestEngine::checkTurn()
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay)
		return false;

	if (m_config.currentStage == ConquestTurn::StagePrepare) {
		if (currentTick() < GAME_PREPARE_INTERVAL_MSEC)
			return false;
		else {
			playBegin();
			return false;
		}
	}

	LOG_CTRACE("engine") << "Check turn" << m_config.currentTurn << m_config.currentStage;

	if (m_config.currentTurn < 0 || m_config.currentTurn >= m_config.turnList.size()) {
		LOG_CWARNING("engine") << "Invalid turn:" << m_config.currentTurn;
		return false;
	}

	const ConquestTurn &turn = m_config.turnList.at(m_config.currentTurn);

	if (const auto &t = currentTick(); t == 0 || turn.subStageEnd == 0 || turn.subStageEnd > t)
		return false;

	nextSubStage();

	return true;
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
			run = nextBattle(true, false);
			break;

		case ConquestTurn::StageLastRound:
			run = nextBattle(true, true);
			break;

		case ConquestTurn::StagePrepare:
			break;

	}

	if (!run) {
		LOG_CERROR("engine") << "ConquestEngine next substage error" << m_id;
		gameFinish(true);
		return;
	}

	m_handler->engineTriggerEngine(this);
}



/**
 * @brief ConquestEngine::playBegin
 */

void ConquestEngine::playBegin()
{
	LOG_CDEBUG("engine") << "Game play begin";

	m_config.currentStage = ConquestTurn::StagePick;

	if (!nextPick(false)) {
		LOG_CERROR("engine") << "ConquestEngine play error" << m_id;
		gameFinish(true);
	}


	/*
	////////

	m_config.currentStage = ConquestTurn::StageBattle;

	for (auto &w : m_config.world.landList) {
		const int &pId = m_players.at(QRandomGenerator::global()->bounded((int) m_players.size())).playerId;
		LOG_CINFO("engine") << "ADD" << w.id << pId;
		w.proprietor = pId;
	}

	if (!nextBattle(false)) {
		LOG_CERROR("engine") << "FINISH GAME ERROR";
		gameFinish(nullptr, {});

		return {
			{ QStringLiteral("error"), QStringLiteral("play error") }
		};
	}

	/////////
*/
}



/**
 * @brief ConquestEngine::questionNext
 * @return
 */

bool ConquestEngine::questionNext()
{
	QMutexLocker locker(&m_engineMutex);

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

	LOG_CTRACE("engine") << "Next pick" << m_config.currentTurn << subStage;

	int nextTurn = m_config.currentTurn+1;

	if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size()) {
		ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

		if (turn.subStage == ConquestTurn::SubStageUserAnswer) {
			turn.subStage = ConquestTurn::SubStageWait;
			turn.subStageStart = currentTick();
			turn.subStageEnd = currentTick() + MSEC_WAIT;

			for (int i=m_config.currentTurn; i < m_config.turnList.size(); ++i) {
				ConquestTurn &t = m_config.turnList[i];

				if (i > m_config.currentTurn && t.subStage != ConquestTurn::SubStageUserSelect)
					break;

				const bool &answer = turn.answerIsSuccess(t.player, false);			// !!! turn -> mert oda mentjük mindet!

				const auto &ptr= playerFind(t.player);
				const auto &answerPlayer = turn.answerGet(t.player);

				if (ptr != m_players.end() && (!answerPlayer || !answerPlayer->success)) {
					ptr->hp = std::max(ptr->hp-1, 0);
				}

				if (!answer) {
					t.player = -1;
					if (i > m_config.currentTurn) {
						t.subStage = ConquestTurn::SubStageFinished;
						t.subStageEnd = 0;
						t.subStageStart = 0;
					}
				}
			}

			questionClear();

			return true;
		} else if (turn.subStage == ConquestTurn::SubStageWait) {
			nextTurn = m_config.currentTurn;

			turn.subStage = ConquestTurn::SubStageUserSelect;
			turn.subStageEnd = 0;
			turn.subStageStart = 0;
		} else {
			turn.subStage = ConquestTurn::SubStageFinished;
			turn.subStageEnd = 0;
			turn.subStageStart = 0;
		}

		turn.clear();
	}


	LOG_CTRACE("engine") << "Next turn:" << nextTurn;

	// Next turn

	if (subStage && nextTurn >= 0 && nextTurn < m_config.turnList.size()) {
		for (; nextTurn < m_config.turnList.size(); ++nextTurn) {
			ConquestTurn &t = m_config.turnList[nextTurn];

			if (t.subStage == ConquestTurn::SubStageUserAnswer) {
				questionClear();

				if (!questionNext())
					return false;

				t.subStageStart = currentTick();
				t.subStageEnd = currentTick() + std::max(MSEC_ANSWER,
														 m_config.currentQuestion.value(QStringLiteral("duration")).toInt());
				m_config.currentTurn = nextTurn;

				return true;
			}

			if (t.player == -1)
				continue;

			const QStringList &lands = getPickableLands(t.player);

			if (lands.isEmpty()) {
				LOG_CTRACE("engine") << "No more pickable lands for player" << t.player;
				break;
			} else {
				t.canPick = lands;
				t.subStageStart = currentTick();
				t.subStageEnd = currentTick() + MSEC_SELECT;
				m_config.currentTurn = nextTurn;

				return true;
			}
		}
	}


	if (auto it = std::find_if(m_config.world.landList.cbegin(), m_config.world.landList.cend(),
							   [](const ConquestWorldData &d) {
							   return d.proprietor == -1;
}); it == m_config.world.landList.cend()) {
		m_config.currentStage = ConquestTurn::StageBattle;
		m_config.currentTurn = -1;
		m_config.turnList.clear();

		return nextBattle(false, false);
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

bool ConquestEngine::nextBattle(const bool &subStage, const bool &isLastRound)
{
	QMutexLocker locker(&m_engineMutex);

	if (m_config.gameState != ConquestConfig::StatePlay ||
			(m_config.currentStage != ConquestTurn::StageBattle &&
			 m_config.currentStage != ConquestTurn::StageLastRound))
		return false;

	LOG_CTRACE("engine") << "Next battle" << m_config.currentTurn << subStage;


	if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size()) {
		ConquestTurn &turn = m_config.turnList[m_config.currentTurn];

		if (turn.subStage == ConquestTurn::SubStageUserSelect) {
			if (turn.pickedId.isEmpty()) {
				LOG_CDEBUG("engine") << "ConquestGame" << m_id << "player pick missing" << turn.player;

				const auto &ptrNew = playerFind(turn.player);

				if (ptrNew != m_players.end()) {
					ptrNew->hp = std::max(ptrNew->hp-1, 0);
					ptrNew->streak = 0;
				}

				turn.subStage = ConquestTurn::SubStageFinished;
				turn.subStageEnd = 0;
				turn.subStageStart = 0;
			} else {
				turn.canPick.clear();
				turn.subStage = ConquestTurn::SubStagePrepareBattle;
				turn.subStageStart = currentTick();
				turn.subStageEnd = currentTick() + MSEC_PREPARE;
				return true;
			}
		} else if (turn.subStage == ConquestTurn::SubStagePrepareBattle) {
			questionClear();

			if (!questionNext())
				return false;

			turn.fortressQuestion = INITIAL_FORTRESS_QUESTION-1;
			turn.subStage = ConquestTurn::SubStageUserAnswer;
			turn.subStageStart = currentTick();
			turn.subStageEnd = currentTick() + std::max(MSEC_ANSWER,
														m_config.currentQuestion.value(QStringLiteral("duration")).toInt());

			return true;
		} else if (turn.subStage == ConquestTurn::SubStageUserAnswer) {
			const int &idx = m_config.world.landFind(turn.pickedId);

			bool clearPick = true;

			if (idx == -1) {
				LOG_CERROR("engine") << "Invalid land id" << turn.pickedId;
				return false;
			}

			ConquestWorldData &land = m_config.world.landList[idx];

			const auto &ptrNew = playerFind(turn.player);
			const auto &ptrOld = playerFind(land.proprietor);

			const bool &answer = turn.answerIsSuccess(turn.player, land.fortress <= 0 || turn.fortressQuestion <= 0);

			const auto &answerPlayer = turn.answerGet(turn.player);
			const auto &answerOpposite = turn.answerGet(land.proprietor);

			static const int streakSize = 4;

			if (ptrNew != m_players.end()) {
				if (answerPlayer && answerPlayer->success) {
					ptrNew->streak++;
					if (ptrNew->streak % streakSize == 0)
						ptrNew->hp++;
				} else {
					ptrNew->hp = std::max(ptrNew->hp-1, 0);
					ptrNew->streak = 0;
				}
			}

			if (ptrOld != m_players.end()) {
				if (answerOpposite && answerOpposite->success) {
					ptrOld->streak++;
					if (ptrOld->streak % streakSize == 0)
						ptrOld->hp++;
				} else {
					ptrOld->hp = std::max(ptrOld->hp-1, 0);
					ptrOld->streak = 0;
				}
			}


			if (answer) {
				turn.answerState = ConquestTurn::AnswerPlayerWin;

				if (ptrNew == m_players.end()) {
					LOG_CERROR("engine") << "Invalid player id" << turn.player;
					return false;
				}

				if (land.fortress <= 0) {
					m_config.landSwapPlayer(turn.pickedId, &*ptrNew,
											ptrOld == m_players.end() ? nullptr : &*ptrOld,
											answerOpposite && answerOpposite->success);

					checkAndRemovePlayersTurns();
				} else {
					if (answerPlayer && answerPlayer->success &&
							answerOpposite && answerOpposite->success &&
							turn.fortressQuestion > 0) {
						--turn.fortressQuestion;
						clearPick = false;
					} else {
						--land.fortress;
						if (land.fortress <= 0) {
							land.xpOnce = LAND_XP_ONCE;
							land.xp = LAND_XP;
							land.fortress = 0;
							m_config.allLandSwapPlayer(&*ptrNew, ptrOld == m_players.end() ? nullptr : &*ptrOld);
							checkAndRemovePlayersTurns();
						} else
							clearPick = false;
					}
				}
			} else {
				turn.answerState = ConquestTurn::AnswerPlayerLost;

				if (ptrOld != m_players.end() && answerOpposite && answerOpposite->success)
					m_config.landDefended(turn.pickedId, &*ptrOld);
			}


			if (clearPick)
				turn.pickedId.clear();
			turn.subStage = ConquestTurn::SubStageWait;
			turn.subStageStart = currentTick();
			turn.subStageEnd = currentTick() + MSEC_WAIT;

			return true;
		} else if (turn.subStage == ConquestTurn::SubStageWait) {
			const int &idx = m_config.world.landFind(turn.pickedId);

			if (idx != -1) {
				ConquestWorldData &land = m_config.world.landList[idx];

				if (land.fortress > 0 && turn.answerState == ConquestTurn::AnswerPlayerWin) {
					questionClear();

					if (!questionNext())
						return false;

					turn.answerList.clear();
					turn.answerState = ConquestTurn::AnswerPending;
					turn.subStage = ConquestTurn::SubStageUserAnswer;
					turn.subStageStart = currentTick();
					turn.subStageEnd = currentTick() + std::max(MSEC_ANSWER,
																m_config.currentQuestion.value(QStringLiteral("duration")).toInt());

					return true;
				}
			}
		}

		turn.clear();
	}

	int nextTurn = m_config.currentTurn+1;

	LOG_CTRACE("engine") << "Next turn:" << nextTurn;

	// Next turn

	if (subStage) {
		if (!checkOtherPlayerAvailable()) {
			LOG_CTRACE("engine") << "ConquestGame finished: no more lands" << m_id;
			gameFinish(false);
			return true;
		}

		while (nextTurn >= 0 && nextTurn < m_config.turnList.size()) {
			ConquestTurn &t = m_config.turnList[nextTurn];
			if (t.player != -1)
				break;

			++nextTurn;
			LOG_CTRACE("engine") << "Skip to turn:" << nextTurn;
		}

		if (nextTurn >= 0 && nextTurn < m_config.turnList.size()) {
			ConquestTurn &t = m_config.turnList[nextTurn];

			const QStringList &lands = getPickableLands(t.player);

			if (lands.isEmpty()) {
				LOG_CTRACE("engine") << "No more pickable lands for player" << t.player;
				return false;
			}

			t.canPick = lands;
			t.subStageStart = currentTick();
			t.subStageEnd = currentTick() + MSEC_SELECT;
			m_config.currentTurn = nextTurn;

			return true;
		} else if (isLastRound) {
			LOG_CTRACE("engine") << "ConquestGame finished successful" << m_id;
			gameFinish(false);
			return true;
		} else {
			LOG_CTRACE("engine") << "ConquestGame last round" << m_id;
			m_config.currentStage = ConquestTurn::StageLastRound;
			m_config.currentTurn = -1;
			m_config.turnList.clear();
			return nextBattle(false, true);
		}
	}

	if (isLastRound)
		prepareTurns(ConquestTurn::StageLastRound);
	else
		prepareTurns(ConquestTurn::StageBattle);

	if (m_config.turnList.empty()) {
		LOG_CERROR("engine") << "Invalid turnList";
		return false;
	}

	return nextBattle(true, isLastRound);
}



/**
 * @brief ConquestEngine::getPickableLands
 * @return
 */

QStringList ConquestEngine::getPickableLands(const int &playerId)
{
	QMutexLocker locker(&m_engineMutex);

	const bool isFirstRound = m_config.currentStage == ConquestTurn::StageBattle &&
							  m_config.currentTurn < (int) m_players.size();

	QStringList list;

	QStringList tmp;

	for (const auto &l : m_config.world.landList) {
		if (l.proprietor == playerId)
			tmp.append(m_config.world.adjacencyMatrix.value(l.id));
	}

	tmp.removeDuplicates();

	for (const auto &l : m_config.world.landList) {
		if (m_config.currentStage == ConquestTurn::StagePick && l.proprietor != -1)
			continue;

		if (tmp.contains(l.id) && l.proprietor != playerId) {
			if (!isFirstRound || l.fortress == -1)
				list.append(l.id);
		}
	}


	// Without adjacency check

	if (list.isEmpty()) {
		for (const auto &l : m_config.world.landList) {
			if (l.proprietor == playerId)
				continue;

			if (m_config.currentStage == ConquestTurn::StagePick && l.proprietor != -1)
				continue;

			if (!isFirstRound || l.fortress == -1)
				list.append(l.id);
		}
	}

	return list;
}



/**
 * @brief ConquestEngine::checkAndRemovePlayersTurns
 */

void ConquestEngine::checkAndRemovePlayersTurns()
{
	QMutexLocker locker(&m_engineMutex);

	for (const ConquestEnginePlayer &p : m_players) {
		if (!m_config.checkPlayerLands(p.playerId)) {
			LOG_CTRACE("engine") << "Remove player" << p.playerId << "from next turns";
			m_config.removePlayerFromNextTurns(p.playerId);
		}
	}
}


/**
 * @brief ConquestEngine::checkOtherPlayerAvailable
 * @return
 */

bool ConquestEngine::checkOtherPlayerAvailable()
{
	QMutexLocker locker(&m_engineMutex);

	int pId = -1;

	for (const auto &l : m_config.world.landList) {
		if (pId == -1) {
			pId = l.proprietor;
			continue;
		}

		if (l.proprietor != -1 && l.proprietor != pId)
			return true;
	}

	return false;
}



/**
 * @brief ConquestEngine::resetPlayers
 */

void ConquestEngine::resetPlayers()
{

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
	m_config.userHost = newHostStream ? newHostStream->credential().username() : QStringLiteral("");
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
		if (m_questionArray.size() >= 10)
			return;

		m_lastRequest = now;

		auto *ws = m_engine->m_hostStream;

		if (!ws) {
			LOG_CWARNING("engine") << "ConquestEngine" << m_engine->id() << "host stream missing";
			return;
		}

		m_engine->sendStreamJson(ws, QJsonObject{
									 { QStringLiteral("cmd"), QStringLiteral("questionRequest") }
								 });

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

		LOG_CDEBUG("engine") << "Upload questions";
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

		if (!m_questionArray.isEmpty())
			ret.resolve();
		else
			ret.reject();
	});

	QDefer::await(ret);

	return ret.state() == RESOLVED;
}
