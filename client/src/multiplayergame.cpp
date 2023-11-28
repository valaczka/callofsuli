#include "multiplayergame.h"
#include "Logger.h"
#include "client.h"
#include "gameenemysoldier.h"
#include <QRandomGenerator>


/**
 * @brief MultiPlayerGame::MultiPlayerGame
 * @param missionLevel
 * @param client
 */

MultiPlayerGame::MultiPlayerGame(GameMapMissionLevel *missionLevel, Client *client)
	: ActionGame(missionLevel, client, GameMap::MultiPlayer)
{
	if (m_client) {
		connect(m_client->httpConnection()->webSocket(), &WebSocket::messageReceived, this, &MultiPlayerGame::onJsonReceived);
		connect(m_client->httpConnection()->webSocket(), &WebSocket::activeChanged, this, &MultiPlayerGame::onActiveChanged);

		m_client->httpConnection()->webSocket()->observerAdd(QStringLiteral("multiplayer"));

		if (m_client->httpConnection()->webSocket()->active())
			onActiveChanged();
		else
			m_client->httpConnection()->webSocket()->connect();
	}

	connect(&m_timeSyncTimer, &QTimer::timeout, this, &MultiPlayerGame::onTimeSyncTimerTimeout);

	m_timeSyncTimer.setInterval(8000);
	m_timeSyncTimer.start();
}


/**
 * @brief MultiPlayerGame::~MultiPlayerGame
 */

MultiPlayerGame::~MultiPlayerGame()
{
	if (m_client) {
		m_client->httpConnection()->webSocket()->observerRemove(QStringLiteral("multiplayer"));
	}
}



/**
 * @brief MultiPlayerGame::start
 */

void MultiPlayerGame::start()
{
	loadGamePage();
}


/**
 * @brief MultiPlayerGame::gameAbort
 */

void MultiPlayerGame::gameAbort()
{
	setFinishState(Neutral);

	LOG_CINFO("game") << "Game aborted:" << this;

	emit gameFinished(Neutral);
}




/**
 * @brief MultiPlayerGame::loadPage
 * @return
 */

QQuickItem *MultiPlayerGame::loadPage()
{
	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMultiPlayerGame.qml"), QVariantMap({
																										  { QStringLiteral("game"), QVariant::fromValue(this) }
																									  }));

	return page;
}



/**
 * @brief MultiPlayerGame::onSceneAboutToStart
 */

void MultiPlayerGame::onSceneAboutToStart()
{
	startWithRemainingTime(m_missionLevel->duration()*1000);
	m_tickTimer.start(this, 1);

	if (m_deathmatch) {
		message(tr("LEVEL %1").arg(level()));
		message(tr("SUDDEN DEATH"));
		message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
		m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"));
	} else {
		message(tr("LEVEL %1").arg(level()));
		message(m_multiPlayerMode == MultiPlayerHost ? tr("MULTIPLAYER HOST") : tr("MULTIPLAYER CLIENT"));
		m_scene->playSoundVoiceOver(QStringLiteral("qrc:/sound/voiceover/begin.mp3"));
	}
}



/**
 * @brief MultiPlayerGame::timerEvent
 * @param event
 */

void MultiPlayerGame::timerEvent(QTimerEvent *)
{
	if (!m_scene)
		return;

	LOG_CTRACE("game") << "TIMER EVENT" << m_tickTimer.currentTick();

	ObjectStateSnapshot snap;
	int id = 1780;

	foreach (GameObject *o, m_scene->m_gameObjects)
		if (o) {
			o->getStateSnapshot(&snap, ++id);
		}

	LOG_CINFO("game") << snap.toReadable().constData();

	m_client->httpConnection()->webSocket()->send(qCompress(snap.toByteArray()));
}




/**
 * @brief MultiPlayerGame::sendWebSocketMessage
 * @param cmd
 * @param data
 */

void MultiPlayerGame::sendWebSocketMessage(const QJsonValue &data)
{
	if (!m_client || !m_client->httpConnection()->webSocket()->active()) {
		LOG_CWARNING("game") << "WebSocket inactive";
		return;
	}

	m_client->httpConnection()->webSocket()->send(QStringLiteral("multiplayer"), data);
}



/**
 * @brief MultiPlayerGame::getServerState
 */

void MultiPlayerGame::getServerState()
{
	m_client->httpConnection()->webSocket()->send(QStringLiteral("multiplayer"), QJsonObject{
													  { QStringLiteral("cmd"), QStringLiteral("state") }
												  });

}



/**
 * @brief MultiPlayerGame::onTimeSyncTimerTimeout
 */

void MultiPlayerGame::onTimeSyncTimerTimeout()
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
 * @brief MultiPlayerGame::onActiveChanged
 */

void MultiPlayerGame::onActiveChanged()
{
	auto ws = m_client->httpConnection()->webSocket();

	connect(ws->socket(), &QWebSocket::binaryMessageReceived, this, &MultiPlayerGame::onBinaryDataReceived);

	const bool active = ws->active();
	LOG_CTRACE("game") << "MultiPlayerGame WebSocket active changed" << active;

	sendWebSocketMessage(QJsonObject{
							 { QStringLiteral("cmd"), QStringLiteral("connect") }
						 });


}



/**
 * @brief MultiPlayerGame::onJsonReceived
 * @param operation
 * @param data
 */

void MultiPlayerGame::onJsonReceived(const QString &operation, const QJsonValue &data)
{
	if (operation == QStringLiteral("timeSync")) {
		const QJsonObject &obj = data.toObject();
		qint64 clientTime = QDateTime::currentMSecsSinceEpoch() - obj.value(QStringLiteral("clientTime")).toInteger();

		m_tickTimer.setLatency(clientTime/2);

		LOG_CTRACE("game") << "ACTUAL SERVER TIME" << m_tickTimer.latency();
	} else if (operation == QStringLiteral("multiplayer")) {
		const QJsonObject &obj = data.toObject();
		const QString &cmd = obj.value(QStringLiteral("cmd")).toString();

		if (cmd == QStringLiteral("connect")) {
			setEngineId(obj.value(QStringLiteral("engine")).toInt(-1));
			getServerState();
		} else if (cmd == QStringLiteral("state")) {
			setEngineId(obj.value(QStringLiteral("engine")).toInt(-1));
			setMultiPlayerMode(obj.value(QStringLiteral("host")).toVariant().toBool() ? MultiPlayerHost : MultiPlayerClient);
		}
	}
}



/**
 * @brief MultiPlayerGame::onBinaryDataReceived
 * @param data
 */

void MultiPlayerGame::onBinaryDataReceived(const QByteArray &data)
{
	LOG_CINFO("game") << "Binary message received" << data;

	const std::optional<ObjectStateSnapshot> &snap = ObjectStateSnapshot::fromByteArray(qUncompress(data));

	if (!snap) {
		LOG_CWARNING("game") << "Invalid binary message received";
		return;
	}

	if (m_multiPlayerMode == MultiPlayerClient) {
		for (const auto &s : snap->list) {
			GameObject *ptr = m_test_enemies.value(s->id);

			if (ptr)
				ptr->setStateFromSnapshot(s.get());
			else if (s->type == ObjectStateBase::TypeEnemySoldier) {
				GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, {});
				soldier->setStateFromSnapshot(s.get());
				m_test_enemies.insert(s->id, soldier);
			}

		}
	}
}



/**
 * @brief MultiPlayerGame::loadGamePage
 */

void MultiPlayerGame::loadGamePage()
{
	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageActionGame.qml"), QVariantMap({
																									 { QStringLiteral("game"), QVariant::fromValue(this) }
																								 }));

	connect(page, &QQuickItem::destroyed, this, [this](){
		LOG_CINFO("game") << "Multiplayer page destroyed";
		setScene(nullptr);
		setPageItem(nullptr);
	}, Qt::DirectConnection);

	const QVariant &scene = page->property("scene");

	setScene(qvariant_cast<GameScene*>(scene));
	setPageItem(page);
}




int MultiPlayerGame::engineId() const
{
	return m_engineId;
}

void MultiPlayerGame::setEngineId(int newEngineId)
{
	if (m_engineId == newEngineId)
		return;
	m_engineId = newEngineId;
	emit engineIdChanged();
}



/**
 * @brief MultiPlayerGame::multiPlayerMode
 * @return
 */

const MultiPlayerGame::Mode &MultiPlayerGame::multiPlayerMode() const
{
	return m_multiPlayerMode;
}

void MultiPlayerGame::setMultiPlayerMode(const Mode &newMultiPlayerMode)
{
	if (m_multiPlayerMode == newMultiPlayerMode)
		return;
	m_multiPlayerMode = newMultiPlayerMode;
	emit multiPlayerModeChanged();
}



/**
 * @brief MultiPlayerGame::sceneTimerTimeout
 * @param msec
 * @param delayFactor
 */

void MultiPlayerGame::sceneTimerTimeout(const int &msec, const qreal &delayFactor)
{
	if (!m_scene) {
		LOG_CWARNING("game") << "Missing scene";
		return;
	}

	if (m_multiPlayerMode == MultiPlayerHost) {
		foreach (GameObject *o, m_scene->m_gameObjects) {
			if (o) {
				o->onTimingTimerTimeout(msec, delayFactor);
				o->cacheCurrentState();
			}
		}
	} else {

	}


	foreach (GameObject *o, m_scene->m_gameObjects) {
		GameEntity *e = qobject_cast<GameEntity*>(o);
		if (e)
			e->performRayCast();
	}
}



/**
 * @brief MultiPlayerGame::onSceneReady
 */

void MultiPlayerGame::onSceneReady()
{
	LOG_CTRACE("game") << "Multiplayer scene ready" << this;

	createQuestions();
	createEnemyLocations();
	if (m_multiPlayerMode == MultiPlayerHost) {
		createFixEnemies();
		createInventory();
	}

	pageItem()->setState(QStringLiteral("run"));

	m_scene->playSoundMusic(backgroundMusic());
}



/**
 * @brief MultiPlayerGame::onSceneAnimationFinished
 */

void MultiPlayerGame::onSceneAnimationFinished()
{
	LOG_CTRACE("game") << "Multiplayer scene amimation finsihed" << this;

	if (m_multiPlayerMode == MultiPlayerHost) {
		recreateEnemies();
		createPlayer();
	}
}






