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


/**
 * @brief ConquestGame::ConquestGame
 * @param client
 */


ConquestGame::ConquestGame(Client *client)
	: AbstractGame(GameMap::Conquest, client)
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

	if (m_client) {
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

	QJsonObject o = m_config.toJson();
	o[QStringLiteral("cmd")] = QStringLiteral("list");

	sendWebSocketMessage(o);
}



/**
 * @brief ConquestGame::gameCreate
 */

void ConquestGame::gameCreate()
{
	LOG_CDEBUG("game") << "Create ConquestGame" << qPrintable(m_config.mapUuid)
					   << qPrintable(m_config.missionUuid) << m_config.missionLevel;

	const ConquestWordListHelper &helper = getWorldList();

	QJsonObject o = m_config.toJson();
	o[QStringLiteral("cmd")] = QStringLiteral("create");
	o[QStringLiteral("worldList")] = helper.toJson().value(QStringLiteral("worldList")).toArray();

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

		if (m.value(QStringLiteral("playerId")).toInt() == id)
			return QColor::fromString(m.value(QStringLiteral("theme")).toString());;
	}

	return Qt::black;
}




/**
 * @brief ConquestGame::gameAbort
 */

void ConquestGame::gameAbort()
{
	setFinishState(Neutral);

	LOG_CINFO("game") << "Game aborted:" << this;

	emit gameFinished(Neutral);
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

	LOG_CTRACE("game") << "TIMER EVENT" << m_tickTimer.currentTick();

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
	if (!m_client)
		return;

	if (!m_client->httpConnection()->webSocket()->active()) {
		LOG_CWARNING("game") << "WebSocket inactive";
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

	getEngineList();
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
			{ "questionRequest", &ConquestGame::cmdQuestionRequest },

			{ "test", &ConquestGame::cmdTest },	////
		};

		auto fn = fMap.value(cmd.toStdString());

		if (obj.contains(QStringLiteral("error"))) {
			m_client->messageError(obj.value(QStringLiteral("error")).toString(), tr("Belső hiba"));
		} else if (fn) {
			std::invoke(fn, this, obj);
		} else {
			LOG_CWARNING("game") << "Invalid command?" << cmd;
		}
	}
}



/**
 * @brief ConquestGame::onConfigChanged
 */

void ConquestGame::onConfigChanged()
{
	LOG_CINFO("game") << "ConquestGame state:" << m_config.gameState;

	if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size())
		setCurrentTurn(m_config.turnList.at(m_config.currentTurn));
	else
		setCurrentTurn({});

	setCurrentStage(m_config.currentStage);


	if ((m_config.gameState == ConquestConfig::StatePrepare || m_config.gameState == ConquestConfig::StatePlay) &&
			m_config.world.name != m_loadedWorld) {
		reloadLandList();
	}

	for (ConquestLandData *land : *m_landDataList) {
		const int &idx = m_config.world.landFind(land->landId());
		if (idx != -1)
			land->loadFromConfig(m_config.world.landList[idx]);
	}

	/*sendWebSocketMessage(QJsonObject{
		{ QStringLiteral("cmd"), QStringLiteral("prepare") },
		{ QStringLiteral("engine"), m_engineId },
		{ QStringLiteral("ready"), true }
	});*/
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
	//setEngineId(data.value(QStringLiteral("engine")).toInt(-1));

	if (data.value(QStringLiteral("engine")).toInt(-1) != m_engineId) {
		LOG_CWARNING("game") << "Game engine mismatch";
	}

	setHostMode(data.value(QStringLiteral("host")).toVariant().toBool() ? HostMode::ModeHost : HostMode::ModeGuest);
	ConquestConfig c;
	c.fromJson(data);
	setConfig(c);

	///LOG_CERROR("game") << "THIS" << QJsonDocument(c.toJson()).toJson(QJsonDocument::Indented).constData();

	Utils::patchSListModel(m_playersModel.get(), data.value(QStringLiteral("users")).toArray().toVariantList(),
						   QStringLiteral("playerId"));

	if (data.contains(QStringLiteral("playerId")))
		setPlayerId(data.value(QStringLiteral("playerId")).toInt());


	if (qint64 tick = JSON_TO_INTEGER_Y(data.value(QStringLiteral("tick")), -1); tick != -1) {
		m_tickTimer.start(this, tick);
	}

	/*if (data.contains(QStringLiteral("interval"))) {
		setServerInterval(data.value(QStringLiteral("interval")).toInt());
	}*/
}


/**
 * @brief ConquestGame::cmdCreate
 * @param data
 */

void ConquestGame::cmdCreate(const QJsonObject &data)
{
	setEngineId(data.value(QStringLiteral("engine")).toInt(-1));
	m_client->snack(tr("Engine %1 created").arg(m_engineId));
}


/**
 * @brief ConquestGame::cmdConnect
 * @param data
 */

void ConquestGame::cmdConnect(const QJsonObject &data)
{
	setEngineId(data.value(QStringLiteral("engine")).toInt(-1));
	m_client->snack(tr("Engine %1 connected").arg(m_engineId));
}


/**
 * @brief ConquestGame::cmdStart
 * @param data
 */

void ConquestGame::cmdStart(const QJsonObject &data)
{

}


/**
 * @brief ConquestGame::cmdPrepare
 * @param data
 */

void ConquestGame::cmdPrepare(const QJsonObject &data)
{

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
		QJsonObject obj = QJsonObject::fromVariantMap(q.generate());
		obj[QStringLiteral("module")] = q.module();
		obj[QStringLiteral("uuid")] = q.uuid();
		qArray.append(obj);
	}

	sendWebSocketMessage(QJsonObject{
							 { QStringLiteral("cmd"), QStringLiteral("questionRequest") },
							 { QStringLiteral("engine"), m_engineId },
							 { QStringLiteral("list"), qArray }
						 });
}





/**
 * @brief ConquestGame::cmdTest
 * @param data
 */

void ConquestGame::cmdTest(const QJsonObject &data)
{
	if (data.contains(QStringLiteral("stateId"))) {
		//emit testImage(data.value(QStringLiteral("stateId")).toInt(), data.value(QStringLiteral("value")).toBool());
	}
}


/**
 * @brief ConquestGame::reloadLandList
 */

void ConquestGame::reloadLandList()
{
	LOG_CDEBUG("game") << "Reload land list:" << qPrintable(m_config.world.name) << m_config.world.playerCount;
	m_landDataList->clear();

	m_loadedWorld.clear();

	if (m_config.world.name.isEmpty() || m_config.world.playerCount < 2)
		return;

	const auto &data = Utils::fileToJsonObject(QStringLiteral(":/conquest/%1/data.json").arg(m_config.world.name));

	if (!data) {
		m_client->messageError(tr("Hibás térkép"), tr("Belső hiba"));
		return;
	}

	const QJsonObject &pData = data->value(QStringLiteral("player%1").arg(m_config.world.playerCount)).toObject();
	const QJsonObject &lands = pData.value(QStringLiteral("lands")).toObject();

	if (pData.isEmpty() || lands.isEmpty()) {
		m_client->messageError(tr("Üres térkép"), tr("Belső hiba"));
		return;
	}

	const QJsonObject &orig = data->value(QStringLiteral("orig")).toObject();

	setWorldSize(QSize(
					 orig.value(QStringLiteral("width")).toInt(),
					 orig.value(QStringLiteral("height")).toInt()
					 ));

	QList<ConquestLandData*> landList;

	landList.reserve(lands.keys().size());

	for (const QString &id : lands.keys()) {
		const QJsonObject &obj = lands.value(id).toObject();
		ConquestLandData *land = new ConquestLandData;
		land->setGame(this);
		land->setLandId(id);
		land->setBaseX(obj.value(QStringLiteral("x")).toDouble());
		land->setBaseY(obj.value(QStringLiteral("y")).toDouble());

		land->setImgMap(QStringLiteral("qrc:/conquest/%1/player%2-land-%3.svg")
						.arg(m_config.world.name).arg(m_config.world.playerCount).arg(id));
		land->setImgBorder(QStringLiteral("qrc:/conquest/%1/player%2-land-%3-border.svg")
						   .arg(m_config.world.name).arg(m_config.world.playerCount).arg(id));

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

ConquestWordListHelper ConquestGame::getWorldList() const
{
	QDirIterator it(QStringLiteral(":/conquest"), { QStringLiteral("data.json") }, QDir::Files, QDirIterator::Subdirectories);

	static const QRegularExpression exp(R"(^player)");

	ConquestWordListHelper wList;

	while (it.hasNext()) {
		const QString &jsonFile = it.next();
		const QString &name = jsonFile.section('/',-2,-2);

		const auto &data = Utils::fileToJsonObject(jsonFile);

		if (!data) {
			LOG_CERROR("game") << "Invalid JSON content:" << qPrintable(jsonFile);
			continue;
		}

		ConquestWorldHelper world;

		world.name = name;

		for (const QString &key : data->keys()) {
			if (key == QStringLiteral("orig"))
				continue;

			QString k = key;

			ConquestWorldHelperInfo info;
			info.playerCount = k.remove(exp).toInt();

			if (info.playerCount < 1) {
				LOG_CERROR("game") << "Invalid JSON content:" << key << qPrintable(jsonFile);
				continue;
			}

			info.landIdList = data->value(key).toObject().value(QStringLiteral("lands")).toObject().keys();
			world.infoList.append(info);
		}

		wList.worldList.append(world);
	}

	return wList;
}



/**
 * @brief ConquestGame::loadQuestion
 */

void ConquestGame::loadQuestion()
{
	if (m_config.currentQuestion.isEmpty())
		return;

	LOG_CERROR("game") << QJsonDocument(m_config.currentQuestion).toJson().constData();

	ModuleInterface *iface = Application::instance()->objectiveModules().value(m_config.currentQuestion.value(QStringLiteral("module")).toString());

	if (!iface) {
		m_client->messageError(tr("Érvénytelen feladattípus"), tr("Belső hiba"));
		return;
	}

	if (m_gameQuestion) {
		if (m_currentStage == ConquestTurn::StageBattle) {
			m_gameQuestion->setPermanentDisabled(m_playerId != m_currentTurn.player && !m_isAttacked);
		}


		m_gameQuestion->loadQuestion(iface->name(), iface->qmlQuestion(),
									 m_config.currentQuestion.toVariantMap(),
									 m_config.currentQuestion.value(QStringLiteral("uuid")).toString());
	}

}



/**
 * @brief ConquestGame::revealQuestion
 */

void ConquestGame::revealQuestion()
{
	LOG_CERROR("game") << "REVEAL";

	for (const auto &a : m_currentTurn.answerList) {
		LOG_CERROR("game") << a.player << a.success << a.elapsed << a.answer;
	}

	int pId = -1;

	if (m_currentStage == ConquestTurn::StageBattle) {
		pId = m_config.getPickedLandProprietor(m_config.currentTurn);
	}

	QVariantMap answer;

	if (m_currentTurn.player == m_playerId || (pId != -1 && m_playerId == pId))
		answer = m_currentTurn.answerGet(m_playerId).value_or(QJsonObject{})
				 .value(QStringLiteral("answer")).toObject()
				 .toVariantMap();
	else
		answer = m_currentTurn.answerGet(m_currentTurn.player).value_or(QJsonObject{})
				 .value(QStringLiteral("answer")).toObject()
				 .toVariantMap();

	m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(1250);

	m_gameQuestion->finish();
}



/**
 * @brief ConquestGame::onGameQuestionSuccess
 * @param answer
 */

void ConquestGame::onGameQuestionSuccess(const QVariantMap &answer)
{
	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), true, m_gameQuestion->elapsedMsec());

	/*int xp = m_gameQuestion->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * (qreal) LITE_GAME_BASE_XP;
	setXp(m_xp+xp);

	if (!m_indices.isEmpty())
		m_indices.takeFirst();
	emit questionsChanged(); */

	/*m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(0);
	m_gameQuestion->finish();*/

	const QJsonObject &answerData = QJsonObject::fromVariantMap(answer);
	QJsonObject obj({
						{ QStringLiteral("cmd"), QStringLiteral("answer") },
						{ QStringLiteral("engine"), m_engineId },
						{ QStringLiteral("answer"), answerData }
					});

	ConquestAnswer::addDetails(&obj, true, m_gameQuestion->elapsedMsec());

	sendWebSocketMessage(obj);
}


/**
 * @brief ConquestGame::onGameQuestionFailed
 * @param answer
 */

void ConquestGame::onGameQuestionFailed(const QVariantMap &answer)
{
	/*#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(m_client);
	if (client)
		client->performVibrate();
#endif*/

	addStatistics(m_gameQuestion->module(), m_gameQuestion->objectiveUuid(), false, m_gameQuestion->elapsedMsec());

	/*m_gameQuestion->answerReveal(answer);
	m_gameQuestion->setMsecBeforeHide(1250);

	m_gameQuestion->finish();*/

	const QJsonObject &answerData = QJsonObject::fromVariantMap(answer);
	QJsonObject obj({
						{ QStringLiteral("cmd"), QStringLiteral("answer") },
						{ QStringLiteral("engine"), m_engineId },
						{ QStringLiteral("answer"), answerData }
					});

	ConquestAnswer::addDetails(&obj, false, m_gameQuestion->elapsedMsec());

	sendWebSocketMessage(obj);
}


/**
 * @brief ConquestGame::onGameQuestionFinished
 */

void ConquestGame::onGameQuestionFinished()
{
	//LOG_CWARNING("game") << "Implementation empty";
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
			break;
		case ConquestTurn::StageBattle:
			message(tr("Küzdelem"));
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

	if (m_currentTurn.subStage == ConquestTurn::SubStageFinished) {
		revealQuestion();
		return;
	}

	const int &pId = m_config.getPickedLandProprietor(m_config.currentTurn);
	if (pId == -1)
		setIsAttacked(false);
	else
		setIsAttacked(m_playerId == pId);

	if (m_gameQuestion)
		m_gameQuestion->forceDestroy();

	if (m_currentTurn.subStage == ConquestTurn::SubStageUserSelect && m_currentTurn.player == m_playerId)
		message(tr("Válassz területet"));
	else if (m_currentTurn.subStage == ConquestTurn::SubStageUserAnswer) {
		if (m_currentTurn.player == m_playerId)
			message(tr("Kérdés"));
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
