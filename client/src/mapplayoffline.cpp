/*
 * ---- Call of Suli ----
 *
 * mapplayoffline.cpp
 *
 * Created on: 2026. 02. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayOffline
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

#include "mapplayoffline.h"
#include "litegame.h"
#include "offlineclientengine.h"



/**
 * @brief MapPlayOffline::MapPlayOffline
 * @param handler
 * @param parent
 */

MapPlayOffline::MapPlayOffline(StudentMapHandler *handler, OfflineClientEngine *engine, QObject *parent)
	: MapPlay{handler ? handler->client() : nullptr, parent}
	, m_handler(handler)
	, m_engine(engine)
{

}


/**
 * @brief MapPlayOffline::~MapPlayOffline
 */

MapPlayOffline::~MapPlayOffline()
{

}



/**
 * @brief MapPlayOffline::load
 * @param campaign
 * @param map
 * @return
 */

bool MapPlayOffline::load(Campaign *campaign, StudentMap *map)
{
	if (!m_client || !m_engine || !m_handler)
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

	if (!m_engine->checkCampaignValid(m_campaign))
		setReadOnly(true);

	const GameMap::GameModes enabledModes = m_engine->getCampaignGameModes(m_campaign);

	if (enabledModes == GameMap::Invalid) {
		LOG_CWARNING("client") << "All game mode disabled";
		setReadOnly(true);
	}

	for (GameMapMission *m : m_gameMap->missions()) {
		GameMap::GameModes mode = m->modes();
		if (mode == GameMap::Invalid)
			mode = GameMap::Lite;

		m->setModes(mode & enabledModes);

		for (GameMapMissionLevel *ml : m->levels()) {
			GameMap::GameModes mode = ml->modes();
			if (mode == GameMap::Invalid)
				continue;

			ml->setModes(mode & enabledModes);
		}
	}

	return true;
}




/**
 * @brief MapPlayOffline::updateSolver
 */

void MapPlayOffline::updateSolver()
{
	if (!m_gameMap || !m_solver || !m_client || !m_engine || !m_handler)
		return;

	m_solver->clear();

	OfflineMap *map = m_engine->findMap(m_gameMap->uuid());

	if (!map)
		return;

	for (auto it = map->map().solver.constBegin();
		 it != map->map().solver.constEnd();
		 ++it) {
		GameMapMission *mission = m_gameMap->mission(it.key());

		if (mission) {
			if (it->isObject()) {
				GameMap::SolverInfo s(it->toObject());
				m_solver->loadSolverInfo(mission, s);
			} else {
				GameMap::SolverInfo s(it->toArray());
				m_solver->loadSolverInfo(mission, s);
			}
		}
	}

	for (const OfflinePermit &p : m_engine->db().permitList()) {
		for (const OfflineReceipt &r : p.receiptList()) {
			if (r.map != m_gameMap->uuid().toUtf8())
				continue;

			if (!r.success)
				continue;

			GameMapMission *mission = m_gameMap->mission(QString::fromUtf8(r.mission));

			if (!mission)
				continue;

			MapPlayMissionLevel *ml = getMissionLevel(mission->level(r.level));

			if (!ml)
				continue;

			MapPlaySolverData solver = ml->solverData();
			solver.setSolved(std::max(1, solver.solved()+1));				// Van, amikor -1
			ml->setSolverData(solver);
		}
	}



	QList<MapPlayMissionLevel*> list = m_solver->updateLock();
	m_solver->updateXP();
	emit missionLevelUnlocked(list);
}



/**
 * @brief MapPlayOffline::getShortTimeHelper
 * @param missionLevel
 * @return
 */

int MapPlayOffline::getShortTimeHelper(MapPlayMissionLevel *missionLevel) const
{
	if (!missionLevel)
		return -1;

	return m_shortTimeHelper.value(missionLevel->missionLevel(), 0);
}



/**
 * @brief MapPlayOffline::extraTimeFactor
 * @return
 */

qreal MapPlayOffline::extraTimeFactor() const
{
	return m_extraTimeFactor;
}

void MapPlayOffline::setExtraTimeFactor(qreal newExtraTimeFactor)
{
	if (qFuzzyCompare(m_extraTimeFactor, newExtraTimeFactor))
		return;
	m_extraTimeFactor = newExtraTimeFactor;
	emit extraTimeFactorChanged();
}




/**
 * @brief MapPlayOffline::onCurrentGamePrepared
 */

void MapPlayOffline::onCurrentGamePrepared()
{
	if (!m_client || !m_gameMap || !m_client->currentGame() || !m_engine)
		return;


	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());

	if (!levelGame) {
		LOG_CERROR("client") << "Object cast error" << m_client->currentGame();
		return;
	}


	if (const auto &ptr = m_engine->requireReceipt(m_campaign)) {
		m_receipt = *ptr;

		/*
		QS_BYTEARRAY(map)
		QS_BYTEARRAY(mission)
		QS_FIELD(quint32, level)
		QS_FIELD(qint64, clock)
		QS_FIELD(GameMap::GameMode, mode)
		QS_FIELD(quint32, duration)
		QS_FIELD(bool, success)
		QS_FIELD(quint32, xp)
		QS_FIELD(quint32, currency)
		*/

		m_receipt.map = levelGame->map()->uuid().toUtf8();
		m_receipt.mission = levelGame->missionLevel()->mission()->uuid().toUtf8();
		m_receipt.level = levelGame->level();
		m_receipt.mode = levelGame->mode();

	} else {
		LOG_CERROR("client") << "Receipt request error";
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


		LOG_CDEBUG("client") << "Game play (offline)";

		levelGame->load();
		setGameState(StatePlay);


		/*m_client->send(HttpConnection::ApiUser, QStringLiteral("campaign/%1/game/create").arg(
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
		});*/
	}
}



/**
 * @brief MapPlayOffline::onCurrentGameFinished
 */

void MapPlayOffline::onCurrentGameFinished()
{
	if (!m_client || !m_client->currentGame() || !m_engine)
		return;


	AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());

	if (!levelGame) {
		LOG_CERROR("client") << "Object cast error" << m_client->currentGame();
		return;
	}


	if (levelGame->finishState() == AbstractGame::Fail)
		emit currentGameFailed();


	if (levelGame->mode() == GameMap::Practice /*|| levelGame->mode() == GameMap::MultiPlayer*/) {
		if (m_client->currentGame()) {
			m_client->currentGame()->setReadyToDestroy(true);
		} else {
			LOG_CERROR("client") << "Missing current game";
		}

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

		/*
		QS_BYTEARRAY(map)
		QS_BYTEARRAY(mission)
		QS_FIELD(quint32, level)
		QS_FIELD(qint64, clock)
		QS_FIELD(GameMap::GameMode, mode)
		QS_FIELD(quint32, duration)
		QS_FIELD(bool, success)
		QS_FIELD(quint32, xp)
		QS_FIELD(quint32, currency)
		*/

		m_receipt.success = levelGame->finishState() == AbstractGame::Success;
		m_receipt.duration = levelGame->elapsedMsec();
		m_receipt.xp = levelGame->xp();
		m_receipt.stat = stat;
		m_receipt.extended = extended;

		m_engine->appendReceipt(m_receipt);

		m_receipt = {};


		/*m_finishObject = QJsonObject({
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
		m_finishTimer.start(); */

		LOG_CDEBUG("client") << "Try finishing game";


		///AbstractLevelGame *levelGame = qobject_cast<AbstractLevelGame*>(m_client->currentGame());

		///setFinishedData(data);

		if (m_client->currentGame()) {
			m_client->currentGame()->setReadyToDestroy(true);
		} else {
			LOG_CERROR("client") << "Missing current game";
		}

		setGameState(StateFinished);
	}

	updateSolver();
}



OfflineClientEngine *MapPlayOffline::engine() const
{
	return m_engine.get();
}
