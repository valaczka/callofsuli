/*
 * ---- Call of Suli ----
 *
 * mapplaycampaign.cpp
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayCampaign
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

#include "mapplaycampaign.h"


/**
 * @brief MapPlayCampaign::MapPlayCampaign
 * @param handler
 * @param parent
 */

MapPlayCampaign::MapPlayCampaign(StudentMapHandler *handler, QObject *parent)
	: MapPlay{handler ? handler->client() : nullptr, parent}
	, m_handler(handler)
{
	Q_ASSERT(m_handler);

	m_updateTimer.setInterval(5000);
	connect(&m_updateTimer, &QTimer::timeout, this, &MapPlayCampaign::onUpdateTimerTimeout);

	LOG_CTRACE("client") << "MapPlayCampaign created" << this;
}


/**
 * @brief MapPlayCampaign::~MapPlayCampaign
 */

MapPlayCampaign::~MapPlayCampaign()
{
	LOG_CTRACE("client") << "MapPlayCampaign destroyed" << this;
}



/**
 * @brief MapPlayCampaign::load
 * @param campaign
 * @param map
 * @return
 */

bool MapPlayCampaign::load(Campaign *campaign, StudentMap *map)
{
	if (!m_client)
		return false;

	const QByteArray &data = m_handler->read(map);

	if (data.isEmpty()) {
		m_client->messageWarning(tr("A pálya még nincs letöltve!"), tr("Hiba"));
		return false;
	}

	if (!loadFromBinaryData(data)) {
		m_client->messageWarning(tr("Nem lehet betölteni a pályát!"), tr("Belső hiba"));
		return false;
	}

	m_campaign = campaign;

	MapPlaySolverDefault *solver = new MapPlaySolverDefault(this);
	setSolver(solver);

	updateSolver();

	return true;
}



/**
 * @brief MapPlayCampaign::updateSolver
 */

void MapPlayCampaign::updateSolver()
{
	if (!m_gameMap || !m_solver || !m_client)
		return;

	m_client->send(WebSocket::ApiUser, QStringLiteral("map/%1/solver").arg(m_gameMap->uuid()))
			->done([this](const QJsonObject &data){
		for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
			GameMapMission *mission = m_gameMap->mission(it.key());
			GameMap::SolverInfo s(it.value().toObject());

			if (mission)
				m_solver->loadSolverInfo(mission, s);
		}

		QList<MapPlayMissionLevel*> list = m_solver->updateLock();
		m_solver->updateXP();
		emit missionLevelUnlocked(list);
	});
}



/**
 * @brief MapPlayCampaign::onCurrentGamePrepared
 */

void MapPlayCampaign::onCurrentGamePrepared()
{
	if (!m_currentGame || !m_client || !m_gameMap)
		return;

	if (!m_campaign) {
		LOG_CERROR("client") << "Missing campaign";
		return;
	}

	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_currentGame);
	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_currentGame);

	if (!levelGame || !game) {
		LOG_CERROR("client") << "Object cast error" << m_currentGame;
		return;
	}

	setFinishedData({});

	m_client->send(WebSocket::ApiUser, QStringLiteral("campaign/%1/game/create").arg(m_campaign->campaignid()), {
					   { QStringLiteral("map"), m_gameMap->uuid() },
					   { QStringLiteral("mission"), levelGame->uuid() },
					   { QStringLiteral("level"), levelGame->level() },
					   { QStringLiteral("deathmatch"), levelGame->deathmatch() },
					   { QStringLiteral("mode"), levelGame->mode() }
				   })
			->error([this](const QNetworkReply::NetworkError &){
		m_client->messageError(tr("Hálózati hiba"), tr("Játék indítása sikertelen"));
		destroyCurrentGame();
	})
			->fail([this](const QString &err){
		m_client->messageError(err, tr("Játék indítása sikertelen"));
		destroyCurrentGame();
	})
			->done([this, game, levelGame](const QJsonObject &data){
		const int &gameId = data.value(QStringLiteral("id")).toInt(-1);
		if (gameId < 0) {
			m_client->messageError(tr("Érvénytelen játékazonosító érekezett"), tr("Játék indítása sikertelen"));
			destroyCurrentGame();
			return;
		}

		LOG_CDEBUG("client") << "Game play (campaign)" << gameId;

		game->setGameId(gameId);
		levelGame->load();

		setGameState(StatePlay);

		m_updateTimer.start();
	});
}


/**
 * @brief MapPlayCampaign::onCurrentGameFinished
 */

void MapPlayCampaign::onCurrentGameFinished()
{
	if (!m_currentGame || !m_client)
		return;

	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_currentGame);
	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_currentGame);

	if (!levelGame || !game) {
		LOG_CERROR("client") << "Object cast error" << m_currentGame;
		return;
	}

	if (levelGame->finishState() == AbstractGame::Fail)
		emit currentGameFailed();


	const QJsonArray &stat = m_currentGame->getStatistics();

	m_client->send(WebSocket::ApiUser, QStringLiteral("game/%1/finish").arg(game->gameId()), {
					   { QStringLiteral("success"), levelGame->finishState() == AbstractGame::Success },
					   { QStringLiteral("xp"), levelGame->xp() },
					   { QStringLiteral("duration"), levelGame->elapsedMsec() },
					   { QStringLiteral("statistics"), stat }
				   })
			->fail([this, stat](const QString &err){
		m_client->messageError(err, tr("Játék mentése sikertelen"));
		m_currentGame->clearStatistics(stat);
		destroyCurrentGame();
	})
			->done([this](const QJsonObject &data){

		setFinishedData(data);

		destroyCurrentGame();

		updateSolver();

		setGameState(StateFinished);
	});

}



/**
 * @brief MapPlayCampaign::onUpdateTimerTimeout
 */

void MapPlayCampaign::onUpdateTimerTimeout()
{
	LOG_CTRACE("client") << "GAME UPDATE";

	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_currentGame);

	if (m_gameState != StatePlay || !m_currentGame || !game)
		return;

	const QJsonArray &stat = m_currentGame->getStatistics();
	int xp = m_currentGame->xp();

	if (stat.isEmpty() && m_lastXP == xp)
		return;

	m_client->send(WebSocket::ApiUser, QStringLiteral("game/%1/update").arg(game->gameId()), {
					   { QStringLiteral("xp"), xp },
					   { QStringLiteral("statistics"), stat }
				   })
			->fail([stat, this](const QString &err){
		LOG_CWARNING("client") << "Game update error:" << qPrintable(err);

		if (m_currentGame)
			m_currentGame->clearStatistics(stat, true);
	})
			->done([this, stat, xp](const QJsonObject &){
		if (m_currentGame)
			m_currentGame->clearStatistics(stat, false);

		m_lastXP = xp;

		LOG_CTRACE("client") << "GAME UPDATED SUCCESSFULY";
	});
}





/**
 * @brief MapPlayCampaign::createLevelGame
 * @param level
 * @param mode
 * @return
 */


AbstractLevelGame *MapPlayCampaign::createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode)
{
	Q_ASSERT(level);
	Q_ASSERT(level->missionLevel());
	Q_ASSERT(m_client);


	AbstractLevelGame *g = nullptr;

	switch (mode) {
	case GameMap::Action:
		g = new CampaignActionGame(level->missionLevel(), m_client);
		break;

	case GameMap::Lite:
		g = new CampaignLiteGame(level->missionLevel(), m_client);
		break;

	case GameMap::Test:
		g = new CampaignTestGame(level->missionLevel(), m_client);
		break;

	default:
		m_client->messageError(tr("A játékmód nem indítható"), tr("Belső hiba"));
		return nullptr;
		break;
	}

	g->setDeathmatch(level->deathmatch());

	setGameState(StateLoading);

	return g;
}


/**
 * @brief MapPlayCampaign::destroyCurrentGame
 */

void MapPlayCampaign::destroyCurrentGame()
{
	AbstractLevelGame *g = m_currentGame;

	setCurrentGame(nullptr);
	if (m_client) m_client->setCurrentGame(nullptr);
	if (g) g->setReadyToDestroy(true);
}




