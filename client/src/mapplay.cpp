/*
 * ---- Call of Suli ----
 *
 * mapplay.cpp
 *
 * Created on: 2022. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlay
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

#include "mapplay.h"
#include "abstractgame.h"
#include "mapimage.h"
#include "utils.h"
#include "application.h"
#include "gamemap.h"
#include "gameterrain.h"
#include "actiongame.h"
#include <QScopedPointer>
#include <Logger.h>

MapPlay::MapPlay(Client *client, QObject *parent)
	: QObject{(parent ? parent : client)}
	, m_client(client)
	, m_missionList(new MapPlayMissionList(this))
{
	Q_ASSERT(m_client);

	LOG_CDEBUG("game") << "Map play object created:" << this;
}




/**
 * @brief MapPlay::~MapPlay
 */

MapPlay::~MapPlay()
{
	unloadGameMap();

	if (m_solver) {
		delete m_solver;
		m_solver = nullptr;
	}

	delete m_missionList;

	LOG_CDEBUG("game") << "Map play object destroyed:" << this;
}


/**
 * @brief MapPlay::loadFromBinaryData
 * @param data
 * @return
 */

bool MapPlay::loadFromBinaryData(const QByteArray &data)
{
	QScopedPointer<GameMap> map(GameMap::fromBinaryData(data));

	if (!map) {
		Application::instance()->messageError(tr("Nem lehet megnyitni a pályát!"), tr("Belső hiba"));
		return false;
	}

	GameMapMissionIface *merror = map->checkLockTree();

	if (merror) {
		Application::instance()->messageError(tr("Nem lehet megnyitni a pályát!"), tr("Hibás zárolás"));
		return false;
	}

	if (!checkTerrains(map.get())) {
		Application::instance()->messageError(tr("Nem lehet megnyitni a pályát!"), tr("Hibás harcmező"));
		return false;
	}

	if (map->appVersion() > 0 && map->appVersion() > Utils::versionCode()) {
		Application::instance()->messageWarning(tr("A pálya az alkalmazásnál magasabb verziószámmal készült.\nFrissítsd az alkalmazást a legújabb verzióra!"),
												tr("Frissítés szükséges"));
		return false;
	}

	loadGameMap(map.take());

	return true;
}


/**
 * @brief MapPlay::loadFromFile
 * @param filename
 * @return
 */

bool MapPlay::loadFromFile(const QString &filename)
{
	return loadFromBinaryData(Utils::fileContent(filename));
}




/**
 * @brief MapPlay::checkTerrains
 * @param map
 * @return
 */

bool MapPlay::checkTerrains(GameMap *map)
{
	if (!map)
		return false;

	foreach (GameMapMission *m, map->missions()) {
		foreach (GameMapMissionLevel *ml, m->levels()) {
			const QString &terrain = ml->terrain();

			if (!GameTerrain::terrainAvailable(terrain)) {
				LOG_CWARNING("game") << "Missing terrain:" << terrain;
				return false;
			}
		}
	}

	return true;

}


/**
 * @brief MapPlay::client
 * @return
 */

Client *MapPlay::client() const
{
	return m_client;
}


GameMap *MapPlay::gameMap() const
{
	return m_gameMap;
}

void MapPlay::setGameMap(GameMap *newGameMap)
{
	if (m_gameMap == newGameMap)
		return;
	m_gameMap = newGameMap;
	emit gameMapChanged();
}


/**
 * @brief MapPlay::loadGameMap
 * @param map
 */

void MapPlay::loadGameMap(GameMap *map)
{
	unloadGameMap();

	LOG_CDEBUG("game") << "Load gamemap:" << map;

	setGameMap(map);

	if (!m_gameMap)
		return;

	reloadMissionList();

	LOG_CDEBUG("game") << "Add mapimage provider for map:" << m_gameMap->uuid();
	MapImage *mapImage = new MapImage(m_gameMap);
	Application::instance()->engine()->addImageProvider(QStringLiteral("mapimage"), mapImage);

	AbstractMapPlaySolver::clear(this);

	updateSolver();

	emit gameMapLoaded();
}




/**
 * @brief MapPlay::unloadGameMap
 */

void MapPlay::unloadGameMap()
{
	if (Application::instance() && Application::instance()->engine() && Application::instance()->engine()->imageProvider(QStringLiteral("mapimage"))) {
		LOG_CDEBUG("game") << "Remove image provider mapimage";
		Application::instance()->engine()->removeImageProvider(QStringLiteral("mapimage"));
	}

	m_missionList->clear();

	if (m_gameMap) {
		LOG_CDEBUG("game") << "Delete gamemap:" << m_gameMap;
		delete m_gameMap;

		setGameMap(nullptr);

		emit gameMapUnloaded();
	}

	AbstractMapPlaySolver::clear(this);
}



/**
 * @brief MapPlay::reloadMissionList
 */

void MapPlay::reloadMissionList()
{
	if (!m_gameMap)
		return;

	LOG_CTRACE("game") << "Reload mission list";

	m_missionList->clear();

	foreach (GameMapMission *mission, m_gameMap->missions()) {
		if (mission->modes() && !mission->modes().testFlag(GameMap::Action) && !mission->modes().testFlag(GameMap::Lite)) {
			LOG_CERROR("game") << "Missing implementation";
			continue;
		}

		MapPlayMission *pMission = new MapPlayMission(mission);
		m_missionList->append(pMission);

		for (int i=1; i<=mission->levels().size(); i++) {
			GameMapMissionLevel *mLevel = mission->level(i);

			if (!mLevel)
				continue;

			MapPlayMissionLevel *pLevel = new MapPlayMissionLevel(mLevel, false);
			pMission->missionLevelList()->append(pLevel);

			if (mLevel->canDeathmatch()) {
				MapPlayMissionLevel *pLevel = new MapPlayMissionLevel(mLevel, true);
				pMission->missionLevelList()->append(pLevel);
			}
		}
	}
}




AbstractLevelGame *MapPlay::createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode)
{
	Q_ASSERT(level);
	Q_ASSERT(level->missionLevel());
	Q_ASSERT(m_client);


	AbstractLevelGame *g = nullptr;

	switch (mode) {
	case GameMap::Action:
		g = new ActionGame(level->missionLevel(), m_client);
		break;

	default:
		m_client->messageError(tr("A játékmód nem indítható"), tr("Belső hiba"));
		return nullptr;
		break;
	}

	g->setDeathmatch(level->deathmatch());



	return g;
}


/**
 * @brief MapPlay::onGamePrepared
 */

void MapPlay::onCurrentGamePrepared()
{
	LOG_CDEBUG("game") << "Current game prepared" << m_currentGame;
}






/**
 * @brief MapPlay::onGameFinished
 */

void MapPlay::onCurrentGameFinished()
{
	LOG_CDEBUG("game") << "Missing game finished implementation!";

	m_currentGame->setReadyToDestroy(true);
}




/**
 * @brief MapPlay::currentGame
 * @return
 */

AbstractLevelGame *MapPlay::currentGame() const
{
	return m_currentGame;
}

void MapPlay::setCurrentGame(AbstractLevelGame *newCurrentGame)
{
	if (m_currentGame == newCurrentGame)
		return;
	m_currentGame = newCurrentGame;
	emit currentGameChanged();
}






/**
 * @brief MapPlay::solver
 * @return
 */

AbstractMapPlaySolver *MapPlay::solver() const
{
	return m_solver;
}

void MapPlay::setSolver(AbstractMapPlaySolver *newSolver)
{
	if (m_solver)
		delete m_solver;

	m_solver = newSolver;

	updateSolver();
}




/**
 * @brief MapPlay::getMission
 * @param mission
 * @return
 */

MapPlayMission *MapPlay::getMission(GameMapMissionIface *mission) const
{
	for (MapPlayMission *m : *m_missionList) {
		if (m->mission() == mission)
			return m;
	}

	return nullptr;
}



/**
 * @brief MapPlay::getMissionLevel
 * @param missionLevel
 * @return
 */

MapPlayMissionLevel *MapPlay::getMissionLevel(GameMapMissionLevelIface *missionLevel, const bool &deathmatch) const
{
	for (MapPlayMission *mission : *m_missionList) {
		for (MapPlayMissionLevel *level : *mission->missionLevelList()) {
			if (level->missionLevel() == missionLevel && level->deathmatch() == deathmatch)
				return level;
		}
	}

	return nullptr;
}


/**
 * @brief MapPlay::play
 * @param level
 * @return
 */

bool MapPlay::play(MapPlayMissionLevel *level, const GameMap::GameMode &mode)
{
	if (!m_client) {
		LOG_CERROR("game") << "Missing client";
		return false;
	}

	if (!level || !level->missionLevel()) {
		LOG_CERROR("game") << "Missing level";
		return false;
	}

	if (m_client->currentGame()) {
		m_client->messageError(tr("Még folyamatban van egy másik játék"), tr("Játék nem indítható"));
		return false;
	}

	AbstractLevelGame *g = createLevelGame(level, mode);

	if (!g)
		return false;

	connect(g, &AbstractGame::gameFinished, this, &MapPlay::onCurrentGameFinished);

	setCurrentGame(g);
	m_client->setCurrentGame(g);

	onCurrentGamePrepared();

	return true;

}



/**
 * @brief MapPlay::updateSolver
 */

void MapPlay::updateSolver()
{
	if (m_solver) {
		m_solver->updateLock();
		m_solver->updateXP();
	}
}












/**
 * @brief MapPlayMissionLevel::MapPlayMissionLevel
 * @param missionLevel
 * @param parent
 */

MapPlayMissionLevel::MapPlayMissionLevel(GameMapMissionLevel *missionLevel, const bool &deathmatch, QObject *parent)
	: QObject(parent)
	, m_missionLevel(missionLevel)
	, m_deathmatch(deathmatch)
{
	LOG_CDEBUG("game") << "Map play mission level created:" << this;
}


/**
 * @brief MapPlayMissionLevel::~MapPlayMissionLevel
 */

MapPlayMissionLevel::~MapPlayMissionLevel()
{
	LOG_CDEBUG("game") << "Map play mission destroyed:" << this;
}


/**
 * @brief MapPlayMissionLevel::solverData
 * @return
 */

const MapPlaySolverData &MapPlayMissionLevel::solverData() const
{
	return m_solverData;
}


/**
 * @brief MapPlayMissionLevel::setSolverData
 * @param newSolverData
 */

void MapPlayMissionLevel::setSolverData(const MapPlaySolverData &newSolverData)
{
	m_solverData = newSolverData;
	emit solvedChanged();
}



void MapPlayMissionLevel::solverDataIncrement()
{
	++m_solverData;
	emit solvedChanged();
}



/**
 * @brief MapPlayMissionLevel::lockDepth
 * @return
 */

int MapPlayMissionLevel::lockDepth() const
{
	return m_lockDepth;
}

void MapPlayMissionLevel::setLockDepth(int newLockDepth)
{
	if (m_lockDepth == newLockDepth)
		return;
	m_lockDepth = newLockDepth;
	emit lockDepthChanged();
}

int MapPlayMissionLevel::xp() const
{
	return m_xp;
}

void MapPlayMissionLevel::setXp(int newXp)
{
	if (m_xp == newXp)
		return;
	m_xp = newXp;
	emit xpChanged();
}


/**
 * @brief MapPlayMissionLevel::level
 * @return
 */

int MapPlayMissionLevel::level() const
{
	return m_missionLevel ? m_missionLevel->level() : 0;
}

int MapPlayMissionLevel::solved() const
{
	return m_solverData.solved();
}

QString MapPlayMissionLevel::medalImage() const
{
	return AbstractLevelGame::medalImagePath(m_missionLevel);
}




/**
 * @brief MapPlayMission::MapPlayMission
 * @param mission
 * @param parent
 */

MapPlayMission::MapPlayMission(GameMapMission *mission, QObject *parent)
	: QObject(parent)
	, m_mission(mission)
	, m_missionLevelList(new MapPlayMissionLevelList(this))
{
	LOG_CDEBUG("game") << "Map play mission created:" << this;
}


/**
 * @brief MapPlayMission::~MapPlayMission
 */

MapPlayMission::~MapPlayMission()
{
	delete m_missionLevelList;

	LOG_CDEBUG("game") << "Map play mission destroyed:" << this;
}




/**
 * @brief MapPlayMission::name
 * @return
 */

QString MapPlayMission::name() const
{
	return m_mission ? m_mission->name() : QLatin1String("");
}


/**
 * @brief MapPlayMission::toSolverInfo
 * @return
 */

GameMap::SolverInfo MapPlayMission::toSolverInfo() const
{
	QJsonObject object;

	for (MapPlayMissionLevel *level : *m_missionLevelList) {
		const QString &key = level->deathmatch() ? QStringLiteral("d%1").arg(level->level()) : QStringLiteral("t%1").arg(level->level());
		object[key] = level->solverData().solved();
	}

	return GameMap::SolverInfo(object);
}



/**
 * @brief AbstractMapPlaySolver::clear
 */

AbstractMapPlaySolver::AbstractMapPlaySolver(MapPlay *mapPlay)
	: m_mapPlay(mapPlay)
{
	Q_ASSERT(m_mapPlay);
}


/**
 * @brief AbstractMapPlaySolver::clear
 */

void AbstractMapPlaySolver::clear()
{
	clear(m_mapPlay);
}


/**
 * @brief AbstractMapPlaySolver::clear
 * @param mapPlay
 */

void AbstractMapPlaySolver::clear(MapPlay *mapPlay)
{
	Q_ASSERT(mapPlay);

	LOG_CDEBUG("game") << "Clear solver info";

	for (MapPlayMission *mission : *mapPlay->missionList()) {
		for (MapPlayMissionLevel *level : *mission->missionLevelList()) {
			level->setSolverData(MapPlaySolverData(0));
		}
	}
}


/**
 * @brief AbstractMapPlaySolver::loadSolverInfo
 * @param missionLevel
 * @param info
 * @return
 */

bool AbstractMapPlaySolver::loadSolverInfo(GameMapMission *mission, const GameMap::SolverInfo &info)
{
	if (!mission) {
		LOG_CWARNING("game") << "Invalid mission";
		return false;
	}

	return loadSolverInfo(m_mapPlay, mission, info);
}




/**
 * @brief AbstractMapPlaySolver::loadSolverInfo
 * @param mapPlay
 * @param missionLevel
 * @param info
 * @return
 */

bool AbstractMapPlaySolver::loadSolverInfo(MapPlay *mapPlay, GameMapMission *mission, const GameMap::SolverInfo &info)
{
	Q_ASSERT(mapPlay);
	Q_ASSERT(mission);

	bool found = false;

	for (MapPlayMission *m : *mapPlay->missionList()) {
		if (m->mission() != mission)
			continue;

		for (MapPlayMissionLevel *level : *m->missionLevelList()) {
			MapPlaySolverData data = level->solverData();
			data.setSolved(info.solved(level->missionLevel()->level(), level->deathmatch()));
			level->setSolverData(data);
			found = true;
		}
	}

	return found;
}



/**
 * @brief MapPlaySolverAction::updateLock
 */

void MapPlaySolverAction::updateLock()
{
	LOG_CDEBUG("game") << "MapPlaySolverAction update locks";

	GameMap *map = m_mapPlay->gameMap();

	if (!map)
		return;


	for (MapPlayMission *mission : *m_mapPlay->missionList()) {
		GameMapMission *gMission = mission->mission();

		const QVector<GameMapMissionLevelIface *> &locks = map->missionLockTree(gMission);

		int lockDepth = 0;

		foreach (GameMapMissionLevelIface *ml, locks) {
			MapPlayMissionLevel *l = m_mapPlay->getMissionLevel(ml, false);
			if (l && (l->lockDepth()>0 || !l->solverData().solved())) {
				lockDepth = qMax(l->lockDepth(), 1);
				break;
			}
		}

		if (lockDepth) {
			for (MapPlayMissionLevel *level : *mission->missionLevelList())
				level->setLockDepth(lockDepth);

			continue;
		}

		for (int i=1; i<mission->missionLevelList()->size(); ++i) {
			MapPlayMissionLevel *level = m_mapPlay->getMissionLevel(gMission->level(i), false);
			MapPlayMissionLevel *dmLevel = m_mapPlay->getMissionLevel(gMission->level(i), true);

			if (!level)
				continue;

			if (lockDepth) {
				level->setLockDepth(lockDepth);
				if (dmLevel)
					dmLevel->setLockDepth(lockDepth);
				continue;
			}

			level->setLockDepth(0);

			if (dmLevel) {
				dmLevel->setLockDepth(level->solverData().solved() ? 0 : 1);
			}

			if (level->solverData().solved() < 1)
				lockDepth = 1;
		}


	}

}


/**
 * @brief MapPlaySolverAction::updateXP
 */

void MapPlaySolverAction::updateXP()
{
	LOG_CDEBUG("game") << "MapPlaySolverAction update xp";

	for (MapPlayMission *mission : *m_mapPlay->missionList()) {
		for (MapPlayMissionLevel *level : *mission->missionLevelList()) {
			level->setXp(m_base * GameMap::computeSolvedXpFactor(level->level(),
																 level->deathmatch(),
																 level->solverData().solved(),
																 GameMap::Action));
		}
	}
}




