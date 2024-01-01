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
#include "litegame.h"
#include "mapimage.h"
#include "testgame.h"
#include "utils_.h"
#include "application.h"
#include "gamemap.h"
#include "gameterrain.h"
#include "actiongame.h"
#include <QScopedPointer>
#include <Logger.h>

MapPlay::MapPlay(Client *client, QObject *parent)
	: QObject{parent}
	, m_client(client)
	, m_missionList(new MapPlayMissionList())
{
	Q_ASSERT(m_client);

	LOG_CTRACE("game") << "Map play created:" << this;
}




/**
 * @brief MapPlay::~MapPlay
 */

MapPlay::~MapPlay()
{
	unloadGameMap();

	LOG_CTRACE("game") << "Map play destroyed:" << this;
}


/**
 * @brief MapPlay::loadFromBinaryData
 * @param data
 * @return
 */

bool MapPlay::loadFromBinaryData(const QByteArray &data)
{
	std::unique_ptr<GameMap> map(GameMap::fromBinaryData(data));

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

	loadGameMap(map);

	return true;
}


/**
 * @brief MapPlay::loadFromFile
 * @param filename
 * @return
 */

bool MapPlay::loadFromFile(const QString &filename)
{
	const auto &c = Utils::fileContent(filename);
	return c ? loadFromBinaryData(*c) : false;
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


/**
 * @brief MapPlay::uuid
 * @return
 */

QString MapPlay::uuid() const
{
	return m_gameMap ? m_gameMap->uuid() : QStringLiteral("");
}


GameMap *MapPlay::gameMap() const
{
	return m_gameMap.get();
}

void MapPlay::setGameMap(std::unique_ptr<GameMap> &newGameMap)
{
	if (m_gameMap == newGameMap)
		return;
	m_gameMap = std::move(newGameMap);
	emit gameMapChanged();
}

void MapPlay::clearGameMap()
{
	m_gameMap.reset();
	emit gameMapChanged();
}


/**
 * @brief MapPlay::loadGameMap
 * @param map
 */

void MapPlay::loadGameMap(std::unique_ptr<GameMap> &map)
{
	unloadGameMap();

	LOG_CDEBUG("game") << "Load gamemap:" << map.get();

	setGameMap(map);

	if (!m_gameMap)
		return;

	reloadMissionList();

	LOG_CDEBUG("game") << "Add mapimage provider for map:" << m_gameMap->uuid();
	MapImage *mapImage = new MapImage(m_gameMap.get());
	Application::instance()->engine()->addImageProvider(QStringLiteral("mapimage"), std::move(mapImage));

	AbstractMapPlaySolver::clear(this);

	updateSolver();

	setGameState(StateSelect);

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
		clearGameMap();

		emit gameMapUnloaded();
	}

	AbstractMapPlaySolver::clear(this);

	setGameState(StateInvalid);
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
		LOG_CTRACE("game") << "Load mission" << mission << mission->name() << mission->modes();

		if (mission->modes().testFlag(GameMap::Exam)) {
			LOG_CTRACE("game") << "Mission (exam) skipped:" << mission << mission->name();
			continue;
		}

		MapPlayMission *pMission = new MapPlayMission(mission);
		m_missionList->append(pMission);

		for (int i=1; i<=mission->levels().size(); i++) {
			GameMapMissionLevel *mLevel = mission->level(i);

			if (!mLevel)
				continue;

			MapPlayMissionLevel *pLevel = new MapPlayMissionLevel(pMission, mLevel, false);
			pMission->missionLevelList()->append(pLevel);

			if (mLevel->canDeathmatch() && mission->modes().testFlag(GameMap::Action)) {
				MapPlayMissionLevel *pLevel = new MapPlayMissionLevel(pMission, mLevel, true);
				pMission->missionLevelList()->append(pLevel);
			}
		}
	}
}



/**
 * @brief MapPlay::createLevelGame
 * @param level
 * @param mode
 * @return
 */

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

	case GameMap::Lite:
		g = new LiteGame(level->missionLevel(), m_client);
		break;

	case GameMap::Test:
		g = new TestGame(level->missionLevel(), m_client);
		break;

	case GameMap::Practice:
		g = new LiteGame(level->missionLevel(), m_client, true);
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
 * @brief MapPlay::onGamePrepared
 */

void MapPlay::onCurrentGamePrepared()
{
	LOG_CWARNING("game") << "Missing game prepared implementation!";
}






/**
 * @brief MapPlay::onGameFinished
 */

void MapPlay::onCurrentGameFinished()
{
	LOG_CWARNING("game") << "Missing game finished implementation!";

	if (m_client->currentGame())
		m_client->currentGame()->setReadyToDestroy(true);
}



/**
 * @brief MapPlay::readOnly
 * @return
 */

bool MapPlay::readOnly() const
{
	return m_readOnly;
}

void MapPlay::setReadOnly(bool newReadOnly)
{
	if (m_readOnly == newReadOnly)
		return;
	m_readOnly = newReadOnly;
	emit readOnlyChanged();
}


/**
 * @brief MapPlay::gameState
 * @return
 */

MapPlay::GameState MapPlay::gameState() const
{
	return m_gameState;
}

void MapPlay::setGameState(GameState newGameState)
{
	if (m_gameState == newGameState)
		return;
	m_gameState = newGameState;
	emit gameStateChanged();
}


/**
 * @brief MapPlay::online
 * @return
 */

bool MapPlay::online() const
{
	return m_online;
}

void MapPlay::setOnline(bool newOnline)
{
	if (m_online == newOnline)
		return;
	m_online = newOnline;
	emit onlineChanged();
}







/**
 * @brief MapPlay::solver
 * @return
 */

AbstractMapPlaySolver *MapPlay::solver() const
{
	return m_solver.get();
}

void MapPlay::setSolver(AbstractMapPlaySolver *newSolver)
{
	m_solver.reset(std::move(newSolver));
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
	for (const MapPlayMission *mission : *m_missionList) {
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

bool MapPlay::play(MapPlayMissionLevel *level, const GameMap::GameMode &mode, const QJsonObject &extended)
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

	m_client->setCurrentGame(g);

	m_extendedData = extended;

	onCurrentGamePrepared();

	return true;

}



/**
 * @brief MapPlay::updateSolver
 */

void MapPlay::updateSolver()
{
	if (m_solver) {
		QList<MapPlayMissionLevel*> list = m_solver->updateLock();
		m_solver->updateXP();
		emit missionLevelUnlocked(list);
	}
}






/**
 * @brief MapPlay::calculateXP
 * @param level
 * @param mode
 * @return
 */

int MapPlay::calculateXP(MapPlayMissionLevel *level, const GameMap::GameMode &mode) const
{
	if (m_solver)
		return m_solver->calculateXP(level, mode);
	else
		return 0;
}



/**
 * @brief MapPlay::getNextLevel
 * @param currentLevel
 * @return
 */

MapPlayMissionLevel *MapPlay::getNextLevel(MapPlayMissionLevel *currentLevel, const GameMap::GameMode &mode) const
{
	MapPlayMission *mission = currentLevel ? currentLevel->mission() : nullptr;

	if (mission && (mode == GameMap::Invalid || mission->modeEnabled(mode))) {
		MapPlayMissionLevel *nextLevel = nullptr;

		for (MapPlayMissionLevel *ml : *mission->missionLevelList())
		{
			if (ml->solved() > 0)
				continue;

			if (ml->level() == currentLevel->level() && !currentLevel->deathmatch() && ml->deathmatch())
				return ml;

			if (!nextLevel && ml->level() > currentLevel->level())
				nextLevel = ml;
			else if (nextLevel && nextLevel->deathmatch() && ml->level() == nextLevel->level() && !ml->deathmatch())
				nextLevel = ml;
			else if (nextLevel && ml->level() < nextLevel->level())
				nextLevel = ml;
		}

		if (nextLevel)
			return nextLevel;
	}

	for (const MapPlayMission *m : *m_missionList) {
		if (mode != GameMap::Invalid && !m->modeEnabled(mode))
			continue;

		MapPlayMissionLevel *nextLevel = nullptr;

		for (MapPlayMissionLevel *ml : *m->missionLevelList()) {
			if (ml->solved() > 0)
				continue;

			if (!nextLevel)
				nextLevel = ml;
			else if (nextLevel && nextLevel->deathmatch() && ml->level() == nextLevel->level() && !ml->deathmatch())
				nextLevel = ml;
			else if (nextLevel && ml->level() < nextLevel->level())
				nextLevel = ml;
		}

		if (nextLevel)
			return nextLevel;
	}

	return nullptr;
}



/**
 * @brief MapPlay::inventoryInfo
 * @param module
 * @return
 */

QVariantMap MapPlay::inventoryInfo(const QString &module) const
{
	GamePickable::GamePickableData data;

	if (module == QStringLiteral("hp"))
		data = GamePickable::pickableDataDetails(GamePickable::PickableHealth);
	else if (module == QStringLiteral("shield"))
		data = GamePickable::pickableDataDetails(GamePickable::PickableShield1);
	else
		data = GamePickable::pickableDataHash().value(module);


	if (data.type == GamePickable::PickableInvalid)
		return QVariantMap {
			{ QStringLiteral("name"), tr("Érvénytelen modul: %1").arg(module) },
			{ QStringLiteral("icon"), QStringLiteral("") }
		};
	else
		return QVariantMap {
			{ QStringLiteral("name"), data.name },
			{ QStringLiteral("icon"), data.image }
		};
}












/**
 * @brief MapPlayMissionLevel::MapPlayMissionLevel
 * @param missionLevel
 * @param parent
 */

MapPlayMissionLevel::MapPlayMissionLevel(MapPlayMission *mission, GameMapMissionLevel *missionLevel, const bool &deathmatch, QObject *parent)
	: QObject(parent)
	, m_mission(mission)
	, m_missionLevel(missionLevel)
	, m_deathmatch(deathmatch)
{
	Q_ASSERT(mission);
	Q_ASSERT(missionLevel);

	LOG_CTRACE("game") << "Map play mission level created:" << this;
}


/**
 * @brief MapPlayMissionLevel::~MapPlayMissionLevel
 */

MapPlayMissionLevel::~MapPlayMissionLevel()
{
	LOG_CTRACE("game") << "Map play mission destroyed:" << this;
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

qreal MapPlayMissionLevel::passed() const
{
	return m_missionLevel ? m_missionLevel->passed() : 0;
}

QString MapPlayMissionLevel::medalImage() const
{
	return AbstractLevelGame::medalImagePath(m_missionLevel);
}


MapPlayMission *MapPlayMissionLevel::mission() const
{
	return m_mission;
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
	LOG_CTRACE("game") << "Map play mission created:" << this;
}


/**
 * @brief MapPlayMission::~MapPlayMission
 */

MapPlayMission::~MapPlayMission()
{
	if (m_missionLevelList) {
		delete m_missionLevelList;
		m_missionLevelList = nullptr;
	}

	LOG_CTRACE("game") << "Map play mission destroyed:" << this;
}


/**
 * @brief MapPlayMission::mission
 * @return
 */

GameMapMission *MapPlayMission::mission() const
{
	return m_mission;
}




/**
 * @brief MapPlayMission::name
 * @return
 */

QString MapPlayMission::name() const
{
	return m_mission ? m_mission->name() : QStringLiteral("");
}


/**
 * @brief MapPlayMission::uuid
 * @return
 */

QString MapPlayMission::uuid() const
{
	return m_mission ? m_mission->uuid() : QStringLiteral("");
}


/**
 * @brief MapPlayMission::description
 * @return
 */

QString MapPlayMission::description() const
{
	if (!m_mission)
		return QStringLiteral("");

	QString txt = m_mission->description();

	if (m_lockDepth > 0) {
		txt += tr("\n\nA küldetés zárolva van, előbb teljesítened kell a következőket:");

		foreach (const GameMapMissionLevel *ml, m_mission->locks()) {
			const QString &name = ml->mission() ? ml->mission()->name() : QStringLiteral("???");

			txt += tr("\n%1 (level %2)").arg(name).arg(ml->level());
		}
	}

	return txt;
}


/**
 * @brief MapPlayMission::toSolverInfo
 * @return
 */

GameMap::SolverInfo MapPlayMission::toSolverInfo() const
{
	QJsonObject object;

	for (const MapPlayMissionLevel *level : *m_missionLevelList) {
		const QString &key = level->deathmatch() ? QStringLiteral("d%1").arg(level->level()) : QStringLiteral("t%1").arg(level->level());
		object[key] = level->solverData().solved();
	}

	return GameMap::SolverInfo(object);
}




/**
 * @brief MapPlayMission::medalImage
 * @return
 */

QString MapPlayMission::medalImage() const
{
	return AbstractLevelGame::medalImagePath(m_mission);
}



/**
 * @brief MapPlayMission::modeEnabled
 * @param mission
 * @param mode
 * @return
 */

bool MapPlayMission::modeEnabled(GameMapMission *mission, const GameMap::GameMode &mode)
{
	if (!mission)
		return false;
	else if (mission->modes().testFlag(GameMap::Invalid))
		return mode == GameMap::Action || mode == GameMap::Lite;
	else
		return mission->modes().testFlag(mode);
}



/**
 * @brief MapPlayMission::lockDepth
 * @return
 */

int MapPlayMission::lockDepth() const
{
	return m_lockDepth;
}

void MapPlayMission::setLockDepth(int newLockDepth)
{
	if (m_lockDepth == newLockDepth)
		return;
	m_lockDepth = newLockDepth;
	emit lockDepthChanged();
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

	for (const MapPlayMission *mission : *mapPlay->missionList()) {
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

	for (const MapPlayMission *m : *mapPlay->missionList()) {
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
 * @brief AbstractMapPlaySolver::updateXP
 */

void AbstractMapPlaySolver::updateXP()
{
	LOG_CDEBUG("game") << "AbstractMapPlaySolver update xp";

	for (const MapPlayMission *mission : *m_mapPlay->missionList()) {
		for (MapPlayMissionLevel *level : *mission->missionLevelList()) {
			const GameMap::GameModes &modes = mission->mission()->modes();
			int xp = 0;

			const QVector<GameMap::GameMode> &list = {
				GameMap::Action, GameMap::Lite, GameMap::Test, GameMap::Quiz, GameMap::Exam
			};

			if (modes.testFlag(GameMap::Invalid))
				xp = calculateXP(level, GameMap::Action);

			foreach (const GameMap::GameMode &mode, list) {
				if (modes.testFlag(mode))
					xp = qMax(xp, calculateXP(level, mode));
			}

			level->setXp(xp);
		}
	}
}





/**
 * @brief MapPlaySolverAction::updateLock
 */

int MapPlaySolverDefault::calculateXP(MapPlayMissionLevel *level, const GameMap::GameMode &mode) const
{
	if (!level)
		return -1;

	return m_base * GameMap::computeSolvedXpFactor(level->level(),
												   level->deathmatch(),
												   level->solverData().solved(),
												   mode);
}





/**
 * @brief MapPlaySolverAction::updateLock
 */

QList<MapPlayMissionLevel *> MapPlaySolverDefault::updateLock()
{
	LOG_CDEBUG("game") << "MapPlaySolverDefault update locks";

	QList<MapPlayMissionLevel*> ret;

	GameMap *map = m_mapPlay->gameMap();

	if (!map)
		return ret;


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

			mission->setLockDepth(lockDepth);

			continue;
		}

		mission->setLockDepth(0);

		for (int i=1; i<=mission->missionLevelList()->size(); ++i) {
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

			const int &oldDepth = level->lockDepth();
			level->setLockDepth(0);
			if (oldDepth > 0 && !ret.contains(level))
				ret.append(level);

			if (dmLevel) {
				const int &oldDepth = dmLevel->lockDepth();
				dmLevel->setLockDepth(level->solverData().solved() ? 0 : 1);
				if (oldDepth > 0 && dmLevel->lockDepth() == 0 && !ret.contains(dmLevel))
					ret.append(dmLevel);
			}

			if (level->solverData().solved() < 1)
				lockDepth = 1;
		}


	}

	return ret;
}


/**
 * @brief MapPlay::finishedData
 * @return
 */

const QJsonObject &MapPlay::finishedData() const
{
	return m_finishedData;
}

void MapPlay::setFinishedData(const QJsonObject &newFinishedData)
{
	if (m_finishedData == newFinishedData)
		return;
	m_finishedData = newFinishedData;
	emit finishedDataChanged();
}
