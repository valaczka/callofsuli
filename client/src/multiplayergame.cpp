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

		m_client->httpConnection()->webSocket()->observerAdd(QStringLiteral("multiplayer"));
		m_client->httpConnection()->webSocket()->connect();
	}

	connect(&m_timeSyncTimer, &QTimer::timeout, this, &MultiPlayerGame::onTimeSyncTimerTimeout);

	m_timeSyncTimer.setInterval(8000);
	m_timeSyncTimer.start();

	m_world = librg_world_create();
}


/**
 * @brief MultiPlayerGame::~MultiPlayerGame
 */

MultiPlayerGame::~MultiPlayerGame()
{
	if (m_client) {
		m_client->httpConnection()->webSocket()->observerRemove(QStringLiteral("multiplayer"));
	}

	librg_world_destroy(m_world);
	m_world = nullptr;
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
 * @brief MultiPlayerGame::recreateEnemies
 */

void MultiPlayerGame::recreateEnemies()
{
	if (m_multiPlayerMode == MultiPlayerHost) {
		ActionGame::recreateEnemies();
	} else {
		if (!m_scene) {
			LOG_CWARNING("game") << "Missing scene";
			return;
		}

		LOG_CDEBUG("game") << "Recreate enemies";

		QList<GameEnemy *> soldiers;

		for (auto &el : m_enemies) {
			const GameTerrain::EnemyData &e = el->enemyData();
			const GameTerrain::EnemyType &type = el->enemyData().type;

			if (el->enemy())
				continue;

			if (type != GameTerrain::EnemySoldier || m_closedBlocks.contains(e.block))
				continue;

			GameEnemySoldier *soldier = GameEnemySoldier::create(m_scene, e);

			soldier->setFacingLeft(QRandomGenerator::global()->generate() % 2);

			soldier->setX(e.rect.left() + e.rect.width()/2);
			soldier->setY(e.rect.bottom()-soldier->height());

			soldier->startMovingAfter(2500);

			soldier->setIsRemote(true);

			el->setEnemy(soldier);

			connect(soldier, &GameEntity::killed, this, &ActionGame::onEnemyDied);

			soldiers.append(soldier);
		}

		LOG_CDEBUG("game") << soldiers.size() << " new enemies created";

		linkQuestionToEnemies(soldiers);
		linkPickablesToEnemies(soldiers);

		emit activeEnemiesChanged();
	}
}






