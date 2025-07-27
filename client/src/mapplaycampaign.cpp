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

	m_finishTimer.setInterval(5000);
	connect(&m_finishTimer, &QTimer::timeout, this, &MapPlayCampaign::onFinishTimerTimeout);

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

	const auto &data = m_handler->read(map);

	if (!data) {
		m_client->messageWarning(tr("A pálya még nincs letöltve!"), tr("Hiba"));
		return false;
	}

	if (!loadFromBinaryData(*data)) {
		m_client->messageWarning(tr("Nem lehet betölteni a pályát!"), tr("Belső hiba"));
		return false;
	}

	m_campaign = campaign;

	auto solver = new MapPlaySolverDefault(this);
	setSolver(solver);

	updateSolver();

	if (m_campaign && (m_campaign->finished() || !m_campaign->started()))
		setReadOnly(true);

	return true;
}



/**
 * @brief MapPlayCampaign::updateSolver
 */

void MapPlayCampaign::updateSolver()
{
	if (!m_gameMap || !m_solver || !m_client)
		return;

	m_client->send(HttpConnection::ApiUser, QStringLiteral("map/%1/solver").arg(m_gameMap->uuid()))
			->done(this, [this](const QJsonObject &data){
		for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
			GameMapMission *mission = m_gameMap->mission(it.key());

			if (mission) {
				if (it.value().isObject()) {
					GameMap::SolverInfo s(it.value().toObject());
					m_solver->loadSolverInfo(mission, s);
				} else {
					GameMap::SolverInfo s(it.value().toArray());
					m_solver->loadSolverInfo(mission, s);
				}
			}
		}

		QList<MapPlayMissionLevel*> list = m_solver->updateLock();
		m_solver->updateXP();
		emit missionLevelUnlocked(list);
	});
}


/**
 * @brief MapPlayCampaign::getShortTimeHelper
 * @param missionLevel
 * @return
 */

int MapPlayCampaign::getShortTimeHelper(MapPlayMissionLevel *missionLevel) const
{
	if (!missionLevel)
		return -1;
	return m_shortTimeHelper.value(missionLevel->missionLevel(), 0);
}



/**
 * @brief MapPlayCampaign::playMultiPlayer
 * @param level
 * @return
 */

bool MapPlayCampaign::playMultiPlayer(MapPlayMissionLevel *level)
{
	return play(level, GameMap::Rpg, QJsonObject(), true);
}



/**
 * @brief MapPlayCampaign::onCurrentGamePrepared
 */

void MapPlayCampaign::onCurrentGamePrepared()
{
	if (!m_client || !m_gameMap || !m_client->currentGame())
		return;


	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());
	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_client->currentGame());

	if (!levelGame || !game) {
		LOG_CERROR("client") << "Object cast error" << m_client->currentGame();
		return;
	}


	if (ActionRpgMultiplayerGame *agame = qobject_cast<ActionRpgMultiplayerGame*>(m_client->currentGame())) {
		RpgConfigBase gameData;
		gameData.mapUuid = m_gameMap->uuid();
		gameData.missionUuid = levelGame->uuid();
		gameData.missionLevel = levelGame->level();
		gameData.campaign = m_campaign ? m_campaign->campaignid() : 0;

		m_client->send(HttpConnection::ApiUser, QStringLiteral("campaign/%1/game/token").arg(
						   m_campaign ? m_campaign->campaignid() : 0
										), gameData.toJson())
				->error(this, [this](const QNetworkReply::NetworkError &){
			m_client->messageError(tr("Hálózati hiba"), tr("Játék indítása sikertelen"));
			destroyCurrentGame();
		})
				->fail(this, [this](const QString &err){
			m_client->messageError(err, tr("Játék indítása sikertelen"));
			destroyCurrentGame();
		})
				->done(this, [this, agame](const QJsonObject &data){

			const QByteArray &token = data.value(QStringLiteral("token")).toString().toLatin1();
			if (token.isEmpty()) {
				m_client->messageError(tr("Érvénytelen játékazonosító érekezett"), tr("Játék indítása sikertelen"));
				destroyCurrentGame();
				return;
			}

			LOG_CDEBUG("client") << "Game play (multiplayer)";

			agame->setConnectionToken(token);
			agame->load();
			setGameState(StatePlay);
		});

		return;
	}



	setFinishedData({});

	if (levelGame->mode() == GameMap::Practice) {
		levelGame->load();
		setGameState(StatePlay);
	} else {
		if (LiteGame *lg = qobject_cast<LiteGame*>(m_client->currentGame()); lg && levelGame->mode() == GameMap::Lite) {
			LOG_CDEBUG("client") << "Add extra time to GameLite" << m_extraTimeFactor;

			if (m_extraTimeFactor > 0.0)
				lg->setAddExtraTime(m_extraTimeFactor);
		}

		m_client->send(HttpConnection::ApiUser, QStringLiteral("campaign/%1/game/create").arg(
						   m_campaign ? m_campaign->campaignid() : 0
										), {
						   { QStringLiteral("map"), m_gameMap->uuid() },
						   { QStringLiteral("mission"), levelGame->uuid() },
						   { QStringLiteral("level"), levelGame->level() },
						   { QStringLiteral("mode"), levelGame->mode() },
						   { QStringLiteral("extended"), m_extendedData }
					   })
				->error(this, [this](const QNetworkReply::NetworkError &){
			m_client->messageError(tr("Hálózati hiba"), tr("Játék indítása sikertelen"));
			destroyCurrentGame();
		})
				->fail(this, [this](const QString &err){
			m_client->messageError(err, tr("Játék indítása sikertelen"));
			destroyCurrentGame();
		})
				->done(this, [this, game, levelGame](const QJsonObject &data){
			const int &gameId = data.value(QStringLiteral("id")).toInt(-1);
			if (gameId < 0) {
				m_client->messageError(tr("Érvénytelen játékazonosító érekezett"), tr("Játék indítása sikertelen"));
				destroyCurrentGame();
				return;
			}

			LOG_CDEBUG("client") << "Game play (campaign)" << gameId;

			game->setGameId(gameId);
			game->setServerExtended(data.value(QStringLiteral("extended")).toObject());
			levelGame->load();

			setGameState(StatePlay);

			m_updateTimer.start();
		});
	}
}


/**
 * @brief MapPlayCampaign::onCurrentGameFinished
 */

void MapPlayCampaign::onCurrentGameFinished()
{
	if (!m_client || !m_client->currentGame())
		return;


	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());


	if (qobject_cast<ActionRpgMultiplayerGame*>(m_client->currentGame())) {
		/*destroyCurrentGame();
		setGameState(StateFinished);
		updateSolver();
		m_client->reloadUser();*/

		m_finishObject = QJsonObject({
										 { QStringLiteral("statistics"), levelGame->getStatistics() },
									 });

		m_finishTries = 0;
		onFinishTimerTimeout();
		m_finishTimer.start();

		return;
	}


	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_client->currentGame());

	if (!levelGame || !game) {
		LOG_CERROR("client") << "Object cast error" << m_client->currentGame();
		return;
	}
	m_updateTimer.stop();

	if (levelGame->finishState() == AbstractGame::Fail)
		emit currentGameFailed();


	if (levelGame->mode() == GameMap::Practice /*|| levelGame->mode() == GameMap::MultiPlayer*/) {
		destroyCurrentGame();
		//updateSolver();
		setGameState(StateFinished);
	} else {
		const QJsonArray &stat = levelGame->getStatistics();
		const QJsonObject &extended = levelGame->getExtendedData();

		if (LiteGame *lg = qobject_cast<LiteGame*>(m_client->currentGame()); lg && levelGame->mode() == GameMap::Lite) {
			if (GameMapMissionLevel *ml = lg->missionLevel(); lg->shortTimeHelper() && ml) {
				int num = m_shortTimeHelper.value(ml, 0);
				m_shortTimeHelper[ml] = ++num;
				emit shortTimeHelperUpdated();
			}
		}

		m_finishObject = QJsonObject({
										 { QStringLiteral("success"), levelGame->finishState() == AbstractGame::Success },
										 { QStringLiteral("xp"), levelGame->xp() },
										 { QStringLiteral("duration"), levelGame->elapsedMsec() },
										 { QStringLiteral("statistics"), stat },
										 { QStringLiteral("extended"), extended },
									 });

		if (ActionRpgGame *rpgGame = qobject_cast<ActionRpgGame*>(m_client->currentGame())) {
			if (RpgGame *g = rpgGame->rpgGame()) {
				m_finishObject.insert(QStringLiteral("wallet"), g->usedWalletAsArray());
				m_finishObject.insert(QStringLiteral("currency"), g->currency());
			}
		}

		levelGame->clearStatistics(stat);

		m_finishTries = 0;
		onFinishTimerTimeout();
		m_finishTimer.start();
	}

}



/**
 * @brief MapPlayCampaign::onUpdateTimerTimeout
 */

void MapPlayCampaign::onUpdateTimerTimeout()
{
	LOG_CTRACE("client") << "Update game";

	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());
	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_client->currentGame());

	if (m_gameState != StatePlay)
		return;

	if (!levelGame || !game) {
		LOG_CERROR("client") << "Object cast error" << m_client->currentGame();
		return;
	}

	if (levelGame->mode() == GameMap::Practice)
		return;

	if (qobject_cast<ActionRpgMultiplayerGame*>(m_client->currentGame())) {
		/// ----- statistics!!!!
		return;
	}


	const QJsonArray &stat = levelGame->getStatistics();
	int xp = levelGame->xp();

	QJsonObject data {
		{ QStringLiteral("xp"), xp },
		{ QStringLiteral("statistics"), stat }
	};

	if (ActionRpgGame *rpgGame = qobject_cast<ActionRpgGame*>(m_client->currentGame())) {
		if (RpgGame *g = rpgGame->rpgGame()) {
			data.insert(QStringLiteral("wallet"), g->usedWalletAsArray());
			data.insert(QStringLiteral("currency"), g->currency());
		}
	} else if (stat.isEmpty() && m_lastXP == xp) {
		return;
	}

	m_client->send(HttpConnection::ApiUser, QStringLiteral("game/%1/update").arg(game->gameId()), data)
			->fail(this, [stat, this](const QString &err){
		LOG_CWARNING("client") << "Game update error:" << qPrintable(err);

		if (m_client->currentGame())
			m_client->currentGame()->clearStatistics(stat, true);
	})
			->done(this, [this, stat, xp](const QJsonObject &){
		if (m_client->currentGame())
			m_client->currentGame()->clearStatistics(stat, false);

		m_lastXP = xp;
	});
}



/**
 * @brief MapPlayCampaign::onFinishTimerTimeout
 */

void MapPlayCampaign::onFinishTimerTimeout()
{
	if (m_finishObject.isEmpty()) {
		m_finishTimer.stop();
		return;
	}

	LOG_CDEBUG("client") << "Try finishing game" << m_finishTries;


	if (ActionRpgMultiplayerGame *game = qobject_cast<ActionRpgMultiplayerGame*>(m_client->currentGame())) {
		if (const QJsonObject &data = game->finalData(); !data.isEmpty()) {
			m_finishTimer.stop();
			m_finishObject = QJsonObject();

			setFinishedData(data);

			destroyCurrentGame();

			updateSolver();

			setGameState(StateFinished);

			m_client->reloadUser();

			return;
		}

		if (++m_finishTries > 4) {
			m_client->messageError(tr("Játék mentése sikertelen"));
			m_finishTimer.stop();
			setGameState(StateFinished);
			destroyCurrentGame();
			return;
		}
	}

	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());
	CampaignGameIface *game = dynamic_cast<CampaignGameIface*>(m_client->currentGame());

	if (!levelGame || !game || ++m_finishTries > 4) {
		m_client->messageError(tr("Játék mentése sikertelen"));
		m_finishTimer.stop();
		setGameState(StateFinished);
		destroyCurrentGame();
		return;
	}

	m_client->send(HttpConnection::ApiUser, QStringLiteral("game/%1/finish").arg(game->gameId()), m_finishObject)
			->fail(this, [](const QString &err){
		LOG_CERROR("client") << "Game finish error:" << qPrintable(err);
	})
			->done(this, [this](const QJsonObject &data){
		m_finishTimer.stop();
		m_finishObject = QJsonObject();

		setFinishedData(data);

		destroyCurrentGame();

		updateSolver();

		setGameState(StateFinished);

		m_client->reloadUser();
	});
}





/**
 * @brief MapPlayCampaign::createLevelGame
 * @param level
 * @param mode
 * @return
 */


AbstractLevelGame *MapPlayCampaign::createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode, const bool &multi)
{
	Q_ASSERT(level);
	Q_ASSERT(level->missionLevel());
	Q_ASSERT(m_client);


	AbstractLevelGame *g = nullptr;

	switch (mode) {
		case GameMap::Rpg:
			if (multi)
				g = new CampaignActionRpgMultiplayerGame(level->missionLevel(), m_client);
			else
				g = new CampaignActionRpgGame(level->missionLevel(), m_client);
			break;

		case GameMap::Lite:
			g = new CampaignLiteGame(level->missionLevel(), m_client);
			break;

		case GameMap::Test:
			g = new CampaignTestGame(level->missionLevel(), m_client);
			break;

		case GameMap::Practice:
			g = new CampaignLiteGame(level->missionLevel(), m_client, true);
			break;

		default:
			m_client->messageError(tr("A játékmód nem indítható"), tr("Belső hiba"));
			return nullptr;
			break;
	}

	setGameState(StateLoading);

	return g;
}


/**
 * @brief MapPlayCampaign::destroyCurrentGame
 */

void MapPlayCampaign::destroyCurrentGame()
{
	if (m_client->currentGame()) {
		m_client->currentGame()->setReadyToDestroy(true);
	} else {
		LOG_CERROR("client") << "Missing current game";
	}
}




/**
 * @brief MapPlayCampaign::extraTimeFactor
 * @return
 */

qreal MapPlayCampaign::extraTimeFactor() const
{
	return m_extraTimeFactor;
}

void MapPlayCampaign::setExtraTimeFactor(qreal newExtraTimeFactor)
{
	if (qFuzzyCompare(m_extraTimeFactor, newExtraTimeFactor))
		return;
	m_extraTimeFactor = newExtraTimeFactor;
	emit extraTimeFactorChanged();
}








