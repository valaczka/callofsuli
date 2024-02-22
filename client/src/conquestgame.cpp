/*
 * ---- Call of Suli ----
 *
 * conquestgame.cpp
 *
 * Created on: 2024. 01. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ConquestGame
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

#include "conquestgame.h"
#include "application.h"
#include "client.h"
#include "question.h"
#include "utils_.h"
#include "gamequestion.h"
#include "actiongame.h"

#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif


/**
 * @brief ConquestGame::ConquestGame
 * @param client
 */


ConquestGame::ConquestGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(GameMap::Conquest, missionLevel, client)
	, m_landDataList(std::make_unique<ConquestLandDataList>())
	, m_engineModel(std::make_unique<QSListModel>())
	, m_playersModel(std::make_unique<QSListModel>())
{
	LOG_CTRACE("game") << "ConquestGame created" << this;

	m_engineModel->setRoleNames(QStringList{
									QStringLiteral("engineId"),
									QStringLiteral("owner")
								});

	m_playersModel->setRoleNames(Utils::getRolesFromObject(ConquestPlayer().metaObject()));

	if (m_client) {
		connect(m_client->httpConnection()->webSocket(), &WebSocket::messageReceived, this, &ConquestGame::onJsonReceived);
		connect(m_client->httpConnection()->webSocket(), &WebSocket::activeChanged, this, &ConquestGame::onWebSocketActiveChanged);

		m_client->httpConnection()->webSocket()->observerAdd(QStringLiteral("conquest"));

		if (m_client->httpConnection()->webSocket()->active())
			onWebSocketActiveChanged();
		else
			m_client->httpConnection()->webSocket()->connect();
	}


	if (m_missionLevel && m_missionLevel->map()) {
		m_config.mapUuid = m_missionLevel->map()->uuid();
		m_config.missionUuid = m_missionLevel->mission()->uuid();
		m_config.missionLevel = m_missionLevel->level();
	}


	connect(&m_timeSyncTimer, &QTimer::timeout, this, &ConquestGame::onTimeSyncTimerTimeout);

	m_timeSyncTimer.setInterval(15000);
	m_timeSyncTimer.start();
}



/**
 * @brief ConquestGame::~ConquestGame
 */

ConquestGame::~ConquestGame()
{
	m_tickTimer.stop();

	if (m_client && m_client->httpConnection() && m_client->httpConnection()->webSocket()) {
		m_client->httpConnection()->webSocket()->observerRemove(QStringLiteral("conquest"));
	}

	LOG_CTRACE("game") << "ConquestGame destroyed" << this;
}


/**
 * @brief ConquestGame::sendWebSocketMessage
 * @param data
 */

void ConquestGame::sendWebSocketMessage(const QJsonValue &data)
{
	if (!m_client || !m_client->httpConnection()->webSocket()->active()) {
		LOG_CWARNING("game") << "WebSocket inactive";
		return;
	}

	LOG_CTRACE("game") << "SEND:" << data;

	m_client->httpConnection()->webSocket()->send(QStringLiteral("conquest"), data);
}




/**
 * @brief ConquestGame::getEngineList
 */

void ConquestGame::getEngineList()
{
	LOG_CTRACE("game") << "Get engine list";

	QJsonObject o = m_config.toBaseJson();
	o[QStringLiteral("cmd")] = QStringLiteral("list");

	sendWebSocketMessage(o);
}



/**
 * @brief ConquestGame::gameCreate
 */

void ConquestGame::gameCreate(const QString &character)
{
	LOG_CDEBUG("game") << "Create ConquestGame" << qPrintable(m_config.mapUuid)
					   << qPrintable(m_config.missionUuid) << m_config.missionLevel
					   << character;

	if (!ActionGame::availableCharacters().contains(character)) {
		m_client->messageWarning(tr("Válassz érvényes karaktert"));
		return;
	}

	ConquestWordListHelper helper;

	getWorldList(&helper);
	getCharacterList(&helper);

	QJsonObject o = m_config.toBaseJson();
	o[QStringLiteral("cmd")] = QStringLiteral("create");
	o[QStringLiteral("worldList")] = helper.toJson().value(QStringLiteral("worldList")).toArray();
	o[QStringLiteral("characterList")] = helper.toJson().value(QStringLiteral("characterList")).toArray();
	o[QStringLiteral("character")] = character;

	sendWebSocketMessage(o);
}


/**
 * @brief ConquestGame::getPlayerColor
 * @param id
 * @return
 */

QColor ConquestGame::getPlayerColor(const int &id) const
{
	for (const QVariant &v : m_playersModel->storage()) {
		const QVariantMap &m = v.toMap();

		if (m.value(QStringLiteral("playerId")).toInt() == id) {
			const QString &ch = m.value(QStringLiteral("character")).toString();
			return QColor::fromString(m_client->availableCharacters().value(ch).toMap().value(QStringLiteral("color")).toString());
		}
	}

	return Qt::black;
}




/**
 * @brief ConquestGame::gameAbort
 */

void ConquestGame::gameAbort()
{
	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	emit gameFinished(Fail);
}


/**
 * @brief ConquestGame::loadPage
 * @return
 */

QQuickItem *ConquestGame::loadPage()
{
	return m_client->stackPushPage(QStringLiteral("PageConquestGame.qml"), QVariantMap({
																						   { QStringLiteral("game"), QVariant::fromValue(this) }
																					   }));

}


/**
 * @brief ConquestGame::timerEvent
 */

void ConquestGame::timerEvent(QTimerEvent *)
{
	if (m_config.gameState != ConquestConfig::StatePlay)
		return;

	///LOG_CTRACE("game") << "TIMER EVENT" << m_tickTimer.currentTick();

	/*ObjectStateSnapshot snap;

	foreach (GameObject *o, m_scene->m_gameObjects) {
		const qint64 id = getEntityId(o);

		if (o && id != -1) {
			o->getStateSnapshot(&snap, id);
		}
	}

	m_client->httpConnection()->webSocket()->send(qCompress(snap.toByteArray()));*/
}


/**
 * @brief ConquestGame::connectGameQuestion
 */

void ConquestGame::connectGameQuestion()
{
	connect(m_gameQuestion, &GameQuestion::success, this, &ConquestGame::onGameQuestionSuccess);
	connect(m_gameQuestion, &GameQuestion::failed, this, &ConquestGame::onGameQuestionFailed);
	connect(m_gameQuestion, &GameQuestion::finished, this, &ConquestGame::onGameQuestionFinished);
}


/**
 * @brief ConquestGame::gameStartEvent
 * @return
 */

bool ConquestGame::gameStartEvent()
{
	return true;
}


/**
 * @brief ConquestGame::gameFinishEvent
 * @return
 */

bool ConquestGame::gameFinishEvent()
{
	return true;
}




/**
 * @brief ConquestGame::onTimeSyncTimerTimeout
 */

void ConquestGame::onTimeSyncTimerTimeout()
{
	if (!m_client || !m_client->httpConnection() || !m_client->httpConnection()->webSocket())
		return;

	if (!m_client->httpConnection()->webSocket()->active()) {
#ifndef QT_NO_DEBUG
		LOG_CWARNING("game") << "WebSocket inactive";
#endif
		return;
	}

	LOG_CTRACE("game") << "Sync server time" << QDateTime::currentDateTime();

	m_client->httpConnection()->webSocket()->send(QStringLiteral("timeSync"), QJsonObject{
													  { QStringLiteral("clientTime"), QDateTime::currentMSecsSinceEpoch() }
												  });
}



/**
 * @brief ConquestGame::onWebSocketActiveChanged
 */

void ConquestGame::onWebSocketActiveChanged()
{
	auto ws = m_client->httpConnection()->webSocket();

	const bool active = ws->active();
	LOG_CTRACE("game") << "MultiPlayerGame WebSocket active changed" << active;

	if (active && !m_binarySignalConnected) {
		connect(ws->socket(), &QWebSocket::binaryMessageReceived, this, &ConquestGame::onBinaryMessageReceived);
		m_binarySignalConnected = true;
	}

	if (m_engineId == -1) {
		getEngineList();
	} else {
		LOG_CDEBUG("game") << "Auto connect to engine" << m_engineId;
		sendWebSocketMessage(QJsonObject({
											 { QStringLiteral("cmd"), QStringLiteral("connect") },
											 { QStringLiteral("engine"), m_engineId }
										 }));
	}

}



/**
 * @brief ConquestGame::onJsonReceived
 * @param operation
 * @param data
 */

void ConquestGame::onJsonReceived(const QString &operation, const QJsonValue &data)
{
	if (operation == QStringLiteral("timeSync")) {
		const QJsonObject &obj = data.toObject();
		qint64 clientTime = QDateTime::currentMSecsSinceEpoch() - JSON_TO_INTEGER(obj.value(QStringLiteral("clientTime")));

		m_tickTimer.setLatency(clientTime/2);

	} else if (operation == QStringLiteral("conquest")) {
		const QJsonObject &obj = data.toObject();
		const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

		using fnDef = void (ConquestGame::*)(const QJsonObject &);

		static const QHash<std::string, fnDef> fMap = {
			{ "state", &ConquestGame::cmdState },
			{ "start", &ConquestGame::cmdStart },
			{ "list", &ConquestGame::cmdList },
			{ "create", &ConquestGame::cmdCreate },
			{ "connect", &ConquestGame::cmdConnect },
			{ "prepare", &ConquestGame::cmdPrepare },
			{ "leave", &ConquestGame::cmdLeave },
			{ "questionRequest", &ConquestGame::cmdQuestionRequest },
		};

		auto fn = fMap.value(cmd.toStdString());

		if (obj.contains(QStringLiteral("error"))) {
			m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
		} else if (fn) {
			std::invoke(fn, this, obj);
		} else {
#ifndef QT_NO_DEBUG
			LOG_CWARNING("game") << "Invalid command?" << cmd;
#endif
		}
	}
}



/**
 * @brief ConquestGame::onBinaryMessageReceived
 * @param message
 */

void ConquestGame::onBinaryMessageReceived(const QByteArray &message)
{
	LOG_CTRACE("game") << "Binary message received" << message.size();

	QJsonDocument doc = QJsonDocument::fromJson(qUncompress(message));

	if (doc.isNull()) {
		LOG_CWARNING("game") << "Invalid binary message received";
		return;
	}

	cmdState(doc.object());
}



/**
 * @brief ConquestGame::onConfigChanged
 */

void ConquestGame::onConfigChanged()
{
	LOG_CDEBUG("game") << "ConquestGame state:" << m_config.gameState << m_config.currentStage << m_config.currentTurn;

	if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size())
		setCurrentTurn(m_config.turnList.at(m_config.currentTurn));
	else
		setCurrentTurn({});

	setCurrentStage(m_config.currentStage);

	if (m_config.gameState == ConquestConfig::StateInvalid)
		return;

	if (m_config.gameState == ConquestConfig::StatePlay && m_oldGameState != ConquestConfig::StatePlay) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/prepare_yourself.mp3"), Sound::VoiceoverChannel);
		m_client->sound()->playSound(backgroundMusic(), Sound::MusicChannel);
	}

	if (m_config.gameState == ConquestConfig::StateFinished && m_oldGameState != ConquestConfig::StateFinished) {
		m_client->sound()->stopMusic();
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/win.mp3"), Sound::VoiceoverChannel);
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
		if (m_gameSuccess)
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);
		else
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
	}


	if ((m_config.gameState == ConquestConfig::StatePrepare || m_config.gameState == ConquestConfig::StatePlay) &&
			m_config.world.name != m_loadedWorld) {
		reloadLandList();
	}

	for (ConquestLandData *land : *m_landDataList) {
		const int &idx = m_config.world.landFind(land->landId());
		if (idx != -1)
			land->loadFromConfig(m_config.world.landList[idx]);
	}

	if (m_config.gameState == ConquestConfig::StatePrepare && m_oldGameState != ConquestConfig::StatePrepare) {
		sendWebSocketMessage(QJsonObject{
								 { QStringLiteral("cmd"), QStringLiteral("prepare") },
								 { QStringLiteral("engine"), m_engineId },
								 { QStringLiteral("ready"), true }
							 });
	}

	m_oldGameState = m_config.gameState;

}


/**
 * @brief ConquestGame::updatePlayer
 */

void ConquestGame::updatePlayer()
{
	for (const QVariant &v : m_playersModel->storage()) {
		const QVariantMap &m = v.toMap();

		if (m.value(QStringLiteral("playerId")).toInt() == m_playerId) {
			setXp(m.value(QStringLiteral("xp")).toInt());
			setHp(m.value(QStringLiteral("hp")).toInt());
			setGameSuccess(m.value(QStringLiteral("success")).toBool());

			return;
		}
	}
}



/**
 * @brief ConquestGame::cmdList
 * @param data
 */

void ConquestGame::cmdList(const QJsonObject &data)
{
	Utils::patchSListModel(m_engineModel.get(), data.value(QStringLiteral("list")).toArray().toVariantList(),
						   QStringLiteral("engineId"));
}



/**
 * @brief ConquestGame::cmdState
 * @param data
 */

void ConquestGame::cmdState(const QJsonObject &data)
{
	if (data.value(QStringLiteral("engine")).toInt(-1) != m_engineId) {
		LOG_CWARNING("game") << "Game engine mismatch";
	}

	Utils::patchSListModel(m_playersModel.get(), data.value(QStringLiteral("users")).toArray().toVariantList(),
						   QStringLiteral("playerId"));

	emit playersModelChanged();

	if (data.contains(QStringLiteral("playerId")))
		setPlayerId(data.value(QStringLiteral("playerId")).toInt());


	updatePlayer();

	setHostMode(data.value(QStringLiteral("host")).toVariant().toBool() ? HostMode::ModeHost : HostMode::ModeGuest);
	ConquestConfig c;
	c.fromJson(data);
	setConfig(c);


	if (qint64 tick = JSON_TO_INTEGER_Y(data.value(QStringLiteral("tick")), -1); tick != -1) {
		m_tickTimer.start(this, tick);
	}
}


/**
 * @brief ConquestGame::cmdCreate
 * @param data
 */

void ConquestGame::cmdCreate(const QJsonObject &data)
{
	setEngineId(data.value(QStringLiteral("engine")).toInt(-1));
	//m_client->snack(tr("Engine %1 created").arg(m_engineId));
}


/**
 * @brief ConquestGame::cmdConnect
 * @param data
 */

void ConquestGame::cmdConnect(const QJsonObject &data)
{
	setEngineId(data.value(QStringLiteral("engine")).toInt(-1));
	//m_client->snack(tr("Engine %1 connected").arg(m_engineId));

	if (data.contains(QStringLiteral("users"))) {
		Utils::patchSListModel(m_playersModel.get(), data.value(QStringLiteral("users")).toArray().toVariantList(),
							   QStringLiteral("playerId"));
		emit playersModelChanged();
	}
}


/**
 * @brief ConquestGame::cmdStart
 * @param data
 */

void ConquestGame::cmdStart(const QJsonObject &data)
{
	const QJsonArray &list = data.value(QStringLiteral("worldList")).toArray();

	QStringList l;

	for (const QJsonValue &v : list)
		l.append(v.toString());

	LOG_CTRACE("game") << "Select world list:" << l;

	setWorldListSelect(l);
}




/**
 * @brief ConquestGame::cmdPrepare
 * @param data
 */

void ConquestGame::cmdPrepare(const QJsonObject &)
{

}


/**
 * @brief ConquestGame::cmdLeave
 * @param data
 */

void ConquestGame::cmdLeave(const QJsonObject &)
{
	LOG_CDEBUG("game") << "Leave engine";
	//m_client->snack(tr("Engine %1 leaved").arg(m_engineId));
	setEngineId(-1);
	setPlayerId(-1);
	setHostMode(ModeGuest);

	m_config.reset();
	emit configChanged();
	onConfigChanged();

	m_playersModel->clear();
	m_engineModel->clear();
	getEngineList();
	updatePlayer();
}



/**
 * @brief ConquestGame::cmdQuestionRequest
 * @param data
 */

void ConquestGame::cmdQuestionRequest(const QJsonObject &)
{
	LOG_CDEBUG("game") << "Generate questions";

	if (!m_map) {
		LOG_CERROR("game") << "Missing map";
		return;
	}

	GameMapMissionLevel *ml = m_map->missionLevel(m_config.missionUuid, m_config.missionLevel);

	if (!ml) {
		LOG_CERROR("game") << "Invalid MissionLevel";
		return;
	}

	QVector<Question> list;

	foreach (GameMapChapter *chapter, ml->chapters()) {
		foreach (GameMapObjective *objective, chapter->objectives()) {
			int n = (objective->storageId() > 0 ? objective->storageCount() : 1);

			for (int i=0; i<n; ++i)
				list.append(Question(objective));

		}
	}

	LOG_CDEBUG("game") << "Created " << list.size() << " questions";


	QJsonArray qArray;

	while (!list.isEmpty()) {
		const Question &q = list.takeAt(QRandomGenerator::global()->bounded(list.size()));

		ModuleInterface *iface = Application::instance()->objectiveModules().value(q.module());

		if (!iface)
			continue;

		const qreal factor = iface->xpFactor() * 1.1;
		const qreal msec = factor * MSEC_ANSWER;

		QJsonObject obj = QJsonObject::fromVariantMap(q.generate());
		obj[QStringLiteral("module")] = q.module();
		obj[QStringLiteral("uuid")] = q.uuid();
		obj[QStringLiteral("duration")] = qFloor(msec);
		qArray.append(obj);
	}

	sendWebSocketMessage(QJsonObject{
							 { QStringLiteral("cmd"), QStringLiteral("questionRequest") },
							 { QStringLiteral("engine"), m_engineId },
							 { QStringLiteral("list"), qArray }
						 });
}





/**
 * @brief ConquestGame::reloadLandList
 */

void ConquestGame::reloadLandList()
{
	LOG_CDEBUG("game") << "Reload land list:" << qPrintable(m_config.world.name);
	m_landDataList->clear();

	m_loadedWorld.clear();

	if (m_config.world.name.isEmpty())
		return;

	const auto &data = Utils::fileToJsonObject(QStringLiteral(":/conquest/%1/data.json").arg(m_config.world.name));

	if (!data) {
		m_client->messageError(tr("Hibás térkép"), tr("Belső hiba"));
		return;
	}

	const QJsonObject &lands = data->value(QStringLiteral("lands")).toObject();

	if (lands.isEmpty()) {
		m_client->messageError(tr("Üres térkép"), tr("Belső hiba"));
		return;
	}

	const QJsonObject &orig = data->value(QStringLiteral("orig")).toObject();

	setWorldSize(QSize(
					 orig.value(QStringLiteral("width")).toInt(),
					 orig.value(QStringLiteral("height")).toInt()
					 ));

	jsonOrigDataCheck(orig);

	if (QString img = orig.value(QStringLiteral("bg")).toString(); !img.isEmpty()) {
		img.prepend(QStringLiteral("qrc:/conquest/")+m_config.world.name+QStringLiteral("/"));
		setWorldBgImage(img);
	} else {
		setWorldBgImage(QStringLiteral("qrc:/conquest/")+m_config.world.name+QStringLiteral("/bg.png"));
	}

	if (QString img = orig.value(QStringLiteral("over")).toString(); !img.isEmpty()) {
		img.prepend(QStringLiteral("qrc:/conquest/")+m_config.world.name+QStringLiteral("/"));
		setWorldOverImage(img);
	} else {
		setWorldOverImage(QStringLiteral("qrc:/conquest/")+m_config.world.name+QStringLiteral("/over.png"));
	}


	QList<ConquestLandData*> landList;

	landList.reserve(lands.keys().size());

	for (const QString &id : lands.keys()) {
		const QJsonObject &obj = lands.value(id).toObject();
		ConquestLandData *land = new ConquestLandData;
		land->setGame(this);
		land->setLandId(id);

		const qreal baseX = obj.value(QStringLiteral("x")).toDouble();
		const qreal baseY = obj.value(QStringLiteral("y")).toDouble();

		land->setBaseX(baseX);
		land->setBaseY(baseY);
		land->setOverX(baseX + obj.value(QStringLiteral("textX")).toDouble());
		land->setOverY(baseY + obj.value(QStringLiteral("textY")).toDouble());

		land->setImgMap(QStringLiteral("qrc:/conquest/%1/land-%2.svg")
						.arg(m_config.world.name).arg(id));
		land->setImgBorder(QStringLiteral("qrc:/conquest/%1/land-%2-border.svg")
						   .arg(m_config.world.name).arg(id));

		landList.append(land);
	}

	LOG_CDEBUG("game") << "Loaded" << landList.size() << "lands";

	m_landDataList->append(landList);

	m_loadedWorld = m_config.world.name;

}




/**
 * @brief ConquestGame::getWorldList
 * @return
 */

void ConquestGame::getWorldList(ConquestWordListHelper *helper) const
{
	Q_ASSERT(helper);

	helper->worldList.clear();

	QDirIterator it(QStringLiteral(":/conquest"), { QStringLiteral("data.json") }, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		const QString &jsonFile = it.next();
		const QString &name = jsonFile.section('/',-2,-2);

		const auto &data = Utils::fileToJsonObject(jsonFile);

		if (!data) {
			LOG_CERROR("game") << "Invalid JSON content:" << qPrintable(jsonFile);
			continue;
		}

		const QJsonObject &orig = data->value(QStringLiteral("orig")).toObject();

		ConquestWorldHelper world;

		world.fromJson(orig);

		world.name = name;
		world.landIdList = data->value(QStringLiteral("lands")).toObject().keys();
		world.adjacency = orig.value(QStringLiteral("adjacency")).toObject();

		helper->worldList.append(world);
	}
}


/**
 * @brief ConquestGame::getCharacterList
 * @param helper
 */

void ConquestGame::getCharacterList(ConquestWordListHelper *helper) const
{
	Q_ASSERT(helper);

	helper->characterList = ActionGame::availableCharacters();
}



/**
 * @brief ConquestGame::loadQuestion
 */

void ConquestGame::loadQuestion()
{
	if (m_config.currentQuestion.isEmpty())
		return;

	if (m_config.currentQuestion == m_loadedQuestion) {
		LOG_CDEBUG("game") << "Question already loaded, skip";
		return;
	}

	m_loadedQuestion = {};

	ModuleInterface *iface = Application::instance()->objectiveModules().value(m_config.currentQuestion.value(QStringLiteral("module")).toString());

	if (!iface) {
		m_client->messageError(tr("Érvénytelen feladattípus"), tr("Belső hiba"));
		return;
	}

	/*if (m_currentStage == ConquestTurn::StageBattle && (m_playerId == m_currentTurn.player || m_isAttacked))
		emit mapDownRequest();*/

	if (m_gameQuestion) {
		if (m_currentStage == ConquestTurn::StageBattle || m_currentStage == ConquestTurn::StageLastRound) {
			m_gameQuestion->setPermanentDisabled(m_playerId != m_currentTurn.player && !m_isAttacked);

			if (m_isAttacked)
				m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/fight.mp3"), Sound::VoiceoverChannel);
		}

		m_loadedQuestion = m_config.currentQuestion;

		m_gameQuestion->loadQuestion(iface->name(), iface->qmlQuestion(),
									 m_config.currentQuestion.toVariantMap(),
									 m_config.currentQuestion.value(QStringLiteral("uuid")).toString());
	} else {
		m_loadedQuestion = {};
	}
}



/**
 * @brief ConquestGame::revealQuestion
 */

void ConquestGame::revealQuestion()
{
	int answerId = -1;

	if (m_currentStage == ConquestTurn::StageBattle) {
		if (m_fighter1.playerId == m_playerId || m_fighter2.playerId == m_playerId)
			answerId = m_playerId;
		else
			answerId = m_currentTurn.player;
	} else {
		answerId = m_playerId;
	}

	const auto &answer = m_currentTurn.answerGet(answerId);


	if (answer) {
		if (m_gameQuestion) m_gameQuestion->answerReveal(answer->answer.toVariantMap());

		if (answerId == m_playerId) {
			if (answer->success) {
				if (m_gameQuestion) m_gameQuestion->setMsecBeforeHide(0);

				if (m_currentStage == ConquestTurn::StageBattle && m_currentTurn.answerState == ConquestTurn::AnswerPlayerWin && m_fighter1.playerId == m_playerId) {
					m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/winner.mp3"), Sound::VoiceoverChannel);
				} else {
					m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/correct.mp3"), Sound::SfxChannel);
				}
			} else {
				if (m_gameQuestion) m_gameQuestion->setMsecBeforeHide(1250);
				emit answerFailed();

				m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/loser.mp3"), Sound::VoiceoverChannel);

#ifndef Q_OS_WASM
				StandaloneClient *client = qobject_cast<StandaloneClient*>(m_client);
				if (client)
					client->performVibrate();
#endif
			}
		} else {
			if (m_gameQuestion) m_gameQuestion->setMsecBeforeHide(1250);
		}
	} else {
		if (m_gameQuestion) m_gameQuestion->setMsecBeforeHide(0);
	}

	if (m_gameQuestion) m_gameQuestion->finish();
}



/**
 * @brief ConquestGame::onGameQuestionSuccess
 * @param answer
 */

void ConquestGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), true, m_gameQuestion->elapsedMsec());

	ConquestAnswer a;
	a.answer = QJsonObject::fromVariantMap(answer);
	a.elapsed = m_gameQuestion->elapsedMsec();
	a.success = true;

	QJsonObject obj = a.toJson();
	obj.insert(QStringLiteral("cmd"), QStringLiteral("answer"));
	obj.insert(QStringLiteral("engine"), m_engineId);

	sendWebSocketMessage(obj);
}


/**
 * @brief ConquestGame::onGameQuestionFailed
 * @param answer
 */

void ConquestGame::onGameQuestionFailed(const QVariantMap &answer)
{
	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), false, m_gameQuestion->elapsedMsec());

	ConquestAnswer a;
	a.answer = QJsonObject::fromVariantMap(answer);
	a.elapsed = m_gameQuestion->elapsedMsec();
	a.success = false;

	QJsonObject obj = a.toJson();
	obj.insert(QStringLiteral("cmd"), QStringLiteral("answer"));
	obj.insert(QStringLiteral("engine"), m_engineId);

	sendWebSocketMessage(obj);
}


/**
 * @brief ConquestGame::onGameQuestionFinished
 */

void ConquestGame::onGameQuestionFinished()
{
	m_loadedQuestion = {};
}


/**
 * @brief ConquestGame::worldListSelect
 * @return
 */

QStringList ConquestGame::worldListSelect() const
{
	return m_worldListSelect;
}

void ConquestGame::setWorldListSelect(const QStringList &newWorldListSelect)
{
	if (m_worldListSelect == newWorldListSelect)
		return;
	m_worldListSelect = newWorldListSelect;
	emit worldListSelectChanged();
}


/**
 * @brief ConquestGame::missionLevel
 * @return
 */

int ConquestGame::missionLevel() const
{
	auto m = m_map ? m_map->missionLevel(m_config.missionUuid, m_config.missionLevel) : nullptr;
	return m ? m->level() : -1;
}


/**
 * @brief ConquestGame::mission
 * @return
 */

QString ConquestGame::missionName() const
{
	auto m = m_map ? m_map->mission(m_config.missionUuid) : nullptr;
	return m ? m->name() : QStringLiteral("");
}



/**
 * @brief ConquestGame::fighter2
 * @return
 */

ConquestPlayer ConquestGame::fighter2() const
{
	return m_fighter2;
}

void ConquestGame::setFighter2(const ConquestPlayer &newFighter2)
{
	if (m_fighter2 == newFighter2)
		return;
	m_fighter2 = newFighter2;
	emit fighter2Changed();
}

ConquestPlayer ConquestGame::fighter1() const
{
	return m_fighter1;
}

void ConquestGame::setFighter1(const ConquestPlayer &newFighter1)
{
	if (m_fighter1 == newFighter1)
		return;
	m_fighter1 = newFighter1;
	emit fighter1Changed();
}



bool ConquestGame::isAttacked() const
{
	return m_isAttacked;
}

void ConquestGame::setIsAttacked(bool newIsAttacked)
{
	if (m_isAttacked == newIsAttacked)
		return;
	m_isAttacked = newIsAttacked;
	emit isAttackedChanged();
}






/**
 * @brief ConquestGame::defaultMessageColor
 * @return
 */

QColor ConquestGame::defaultMessageColor() const
{
	return m_defaultMessageColor;
}

void ConquestGame::setDefaultMessageColor(const QColor &newDefaultMessageColor)
{
	if (m_defaultMessageColor == newDefaultMessageColor)
		return;
	m_defaultMessageColor = newDefaultMessageColor;
	emit defaultMessageColorChanged();
}

ConquestTurn::Stage ConquestGame::currentStage() const
{
	return m_currentStage;
}



/**
 * @brief ConquestGame::setCurrentStage
 * @param newCurrentStage
 */

void ConquestGame::setCurrentStage(const ConquestTurn::Stage &newCurrentStage)
{
	if (m_currentStage == newCurrentStage)
		return;
	m_currentStage = newCurrentStage;

	emit currentStageChanged();

	switch (m_currentStage) {
		case ConquestTurn::StagePick:
			message(tr("Területválasztás"));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/ready.mp3"), Sound::VoiceoverChannel);
			break;
		case ConquestTurn::StageBattle:
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);
			message(tr("Küzdelem"));
			break;
		case ConquestTurn::StageLastRound:
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/final_round.mp3"), Sound::VoiceoverChannel);
			message(tr("Final round"));
			break;
		default:
			break;
	}
}


/**
 * @brief ConquestGame::playersModel
 * @return
 */

QSListModel*ConquestGame::playersModel() const
{
	return m_playersModel.get();
}



/**
 * @brief ConquestGame::currentTurn
 * @return
 */

ConquestTurn ConquestGame::currentTurn() const
{
	return m_currentTurn;
}

void ConquestGame::setCurrentTurn(const ConquestTurn &newCurrentTurn)
{
	if (m_currentTurn == newCurrentTurn)
		return;
	m_currentTurn = newCurrentTurn;
	emit currentTurnChanged();

	const int idx = (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size()) ?
						m_config.world.landFind(m_config.turnList.at(m_config.currentTurn).pickedId) :
						-1;


	if (m_currentTurn.subStage == ConquestTurn::SubStageWait) {
		LOG_CDEBUG("game") << "Reveal question";
		revealQuestion();

		if (m_fighter2Fortress > 0) {
			if (idx != -1)
				setFighter2Fortress(m_config.world.landList.at(idx).fortress);
			else if (m_currentTurn.answerState == ConquestTurn::AnswerPlayerWin)
				setFighter2Fortress(m_fighter2Fortress-1);
		}

		return;
	}


	const int pId = (idx == -1) ? -1 : m_config.world.landList.at(idx).proprietor;

	if (pId == -1)
		setIsAttacked(false);
	else
		setIsAttacked(m_playerId == pId);

	if (m_gameQuestion && m_config.currentQuestion.isEmpty())
		m_gameQuestion->forceDestroy();


	if (m_currentTurn.subStage == ConquestTurn::SubStagePrepareBattle ||
			m_currentTurn.subStage == ConquestTurn::SubStageUserAnswer ||
			m_currentTurn.subStage == ConquestTurn::SubStageWait) {

		auto f1 = m_fighter1;
		auto f2 = m_fighter2;

		f1.playerId = m_currentTurn.player;
		f1.character = playerCharacter(f1.playerId);
		f2.playerId = pId;
		f2.character = playerCharacter(f2.playerId);

		setFighter1(f1);
		setFighter2(f2);

		if (idx != -1 && m_config.world.landList.at(idx).fortress > 0)
			setFighter2Fortress(m_config.world.landList.at(idx).fortress);
		else
			setFighter2Fortress(-1);

	} else {
		setFighter1({});
		setFighter2({});
		setFighter2Fortress(-1);
	}

	if (m_currentTurn.subStage == ConquestTurn::SubStageUserSelect && m_currentTurn.player == m_playerId) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"), Sound::SfxChannel);
		emit mapUpRequest();
		message(tr("Válassz területet"));
	} else if (m_currentTurn.subStage == ConquestTurn::SubStageUserAnswer) {
		loadQuestion();
	}
}



/**
 * @brief ConquestGame::engineModel
 * @return
 */

QSListModel*ConquestGame::engineModel() const
{
	return m_engineModel.get();
}



/**
 * @brief ConquestGame::worldSize
 * @return
 */

QSize ConquestGame::worldSize() const
{
	return m_worldSize;
}

void ConquestGame::setWorldSize(const QSize &newWorldSize)
{
	if (m_worldSize == newWorldSize)
		return;
	m_worldSize = newWorldSize;
	emit worldSizeChanged();
}



/**
 * @brief ConquestGame::landDataList
 * @return
 */

ConquestLandDataList*ConquestGame::landDataList() const
{
	return m_landDataList.get();
}




/**
 * @brief ConquestGame::playerId
 * @return
 */

int ConquestGame::playerId() const
{
	return m_playerId;
}

void ConquestGame::setPlayerId(int newPlayerId)
{
	if (m_playerId == newPlayerId)
		return;
	m_playerId = newPlayerId;
	emit playerIdChanged();
}



/**
 * @brief ConquestGame::engineId
 * @return
 */

int ConquestGame::engineId() const
{
	return m_engineId;
}

void ConquestGame::setEngineId(int newEngineId)
{
	if (m_engineId == newEngineId)
		return;
	m_engineId = newEngineId;
	emit engineIdChanged();
}


/**
 * @brief ConquestGame::hostMode
 * @return
 */

ConquestGame::HostMode ConquestGame::hostMode() const
{
	return m_hostMode;
}

void ConquestGame::setHostMode(const HostMode &newHostMode)
{
	if (m_hostMode == newHostMode)
		return;
	m_hostMode = newHostMode;
	emit hostModeChanged();
}



/**
 * @brief ConquestGame::config
 * @return
 */

ConquestConfig ConquestGame::config() const
{
	return m_config;
}

void ConquestGame::setConfig(const ConquestConfig &newConfig)
{
	if (m_config == newConfig)
		return;
	m_config = newConfig;
	emit configChanged();
	onConfigChanged();
}




/**
 * @brief ConquestGame::handler
 * @return
 */

StudentMapHandler *ConquestGame::handler() const
{
	return m_handler;
}

void ConquestGame::setHandler(StudentMapHandler *newHandler)
{
	m_handler = newHandler;
}



/**
 * @brief ConquestGame::message
 * @param text
 * @param color
 */

void ConquestGame::messageColor(const QString &text, const QColor &color)
{
	if (!m_messageList) {
		LOG_CINFO("game") << text;
		return;
	}

	LOG_CDEBUG("game") << text;

	QMetaObject::invokeMethod(m_messageList, "message", Qt::DirectConnection,
							  Q_ARG(QVariant, text),
							  Q_ARG(QVariant, color.name()));
}



/**
 * @brief ConquestGame::playerCharacter
 * @param id
 * @return
 */

QString ConquestGame::playerCharacter(const int &id) const
{
	const int idx = m_playersModel->indexOf(QStringLiteral("playerId"), id);
	if (idx == -1)
		return QStringLiteral("");
	else
		return m_playersModel->storage().at(idx).toMap().value(QStringLiteral("character")).toString();
}


/**
 * @brief ConquestGame::playMenuBgMusic
 */

void ConquestGame::playMenuBgMusic()
{
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/menu/bg.mp3"), Sound::MusicChannel);
}


/**
 * @brief ConquestGame::stopMenuBgMusic
 */

void ConquestGame::stopMenuBgMusic()
{
	m_client->sound()->stopMusic();
}


/**
 * @brief ConquestGame::messageList
 * @return
 */

QQuickItem *ConquestGame::messageList() const
{
	return m_messageList;
}

void ConquestGame::setMessageList(QQuickItem *newMessageList)
{
	if (m_messageList == newMessageList)
		return;
	m_messageList = newMessageList;
	emit messageListChanged();
}


/**
 * @brief ConquestGame::maxPlayersCount
 * @return
 */

int ConquestGame::maxPlayersCount()
{
	return MAX_PLAYERS_COUNT;
}

int ConquestGame::hp() const
{
	return m_hp;
}

void ConquestGame::setHp(int newHp)
{
	if (m_hp == newHp)
		return;

	m_hp = newHp;
	emit hpChanged();
}



QString ConquestGame::worldBgImage() const
{
	return m_worldBgImage;
}

void ConquestGame::setWorldBgImage(const QString &newWorldBgImage)
{
	if (m_worldBgImage == newWorldBgImage)
		return;
	m_worldBgImage = newWorldBgImage;
	emit worldBgImageChanged();
}

QString ConquestGame::worldOverImage() const
{
	return m_worldOverImage;
}

void ConquestGame::setWorldOverImage(const QString &newWorldOverImage)
{
	if (m_worldOverImage == newWorldOverImage)
		return;
	m_worldOverImage = newWorldOverImage;
	emit worldOverImageChanged();
}

int ConquestGame::fighter2Fortress() const
{
	return m_fighter2Fortress;
}

void ConquestGame::setFighter2Fortress(int newFighter2Fortress)
{
	if (m_fighter2Fortress == newFighter2Fortress)
		return;
	m_fighter2Fortress = newFighter2Fortress;
	emit fighter2FortressChanged();
}

bool ConquestGame::gameSuccess() const
{
	return m_gameSuccess;
}

void ConquestGame::setGameSuccess(bool newGameSuccess)
{
	if (m_gameSuccess == newGameSuccess)
		return;
	m_gameSuccess = newGameSuccess;
	emit gameSuccessChanged();
}
