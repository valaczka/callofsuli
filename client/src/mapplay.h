/*
 * ---- Call of Suli ----
 *
 * mapplay.h
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

#ifndef MAPPLAY_H
#define MAPPLAY_H

#include <QObject>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "abstractlevelgame.h"
#include "gamemap.h"

class Client;
class AbstractMapPlaySolver;
class MapPlayMission;
using MapPlayMissionList = qolm::QOlm<MapPlayMission>;
class MapPlayMissionLevel;
using MapPlayMissionLevelList = qolm::QOlm<MapPlayMissionLevel>;


/**
 * @brief The MapPlay class
 */

class MapPlay : public QObject
{
	Q_OBJECT

	Q_PROPERTY(GameMap *gameMap READ gameMap WRITE setGameMap NOTIFY gameMapChanged)
	Q_PROPERTY(MapPlayMissionList *missionList READ missionList CONSTANT)
	Q_PROPERTY(AbstractLevelGame *currentGame READ currentGame WRITE setCurrentGame NOTIFY currentGameChanged)

public:
	explicit MapPlay(Client *client, QObject *parent = nullptr);
	virtual ~MapPlay();

	Q_INVOKABLE bool loadFromBinaryData(const QByteArray &data);
	Q_INVOKABLE bool loadFromFile(const QString &filename);

	static bool checkTerrains(GameMap *map);

	Client *client() const;

	GameMap *gameMap() const;
	void setGameMap(GameMap *newGameMap);

	AbstractMapPlaySolver *solver() const;
	void setSolver(AbstractMapPlaySolver *newSolver);

	MapPlayMissionList *missionList() const { return m_missionList; }

	MapPlayMission *getMission(GameMapMissionIface *mission) const;
	MapPlayMissionLevel *getMissionLevel(GameMapMissionLevelIface *missionLevel, const bool &deathmatch) const;

	Q_INVOKABLE bool play(MapPlayMissionLevel *level, const GameMap::GameMode &mode);

	void updateSolver();

	AbstractLevelGame *currentGame() const;
	void setCurrentGame(AbstractLevelGame *newCurrentGame);

protected:
	void loadGameMap(GameMap *map);
	void unloadGameMap();
	void reloadMissionList();

	virtual AbstractLevelGame *createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode);
	virtual void onCurrentGamePrepared();
	virtual void onCurrentGameFinished();

signals:
	void gameMapLoaded();
	void gameMapUnloaded();
	void gameMapChanged();
	void currentGameChanged();

protected:
	Client *const m_client = nullptr;
	GameMap *m_gameMap = nullptr;
	AbstractMapPlaySolver *m_solver = nullptr;
	MapPlayMissionList *const m_missionList;
	AbstractLevelGame *m_currentGame = nullptr;

};




/**
 * @brief The MapPlaySolverData class
 */

class MapPlaySolverData
{
public:
	explicit MapPlaySolverData(const int &solved = 0) : m_solved(solved) {}
	virtual ~MapPlaySolverData() {}

	int solved() const { return m_solved; }
	void setSolved(int newSolved) { m_solved = newSolved; }

	int solvedIncrement() { return ++m_solved; }

	MapPlaySolverData& operator++() { ++m_solved; return *this; }
	MapPlaySolverData operator++(int) { MapPlaySolverData old = *this; ++m_solved; return old; }

private:
	int m_solved = 0;
};



/**
 * @brief The MapPlaySolver class
 */

class AbstractMapPlaySolver
{
public:
	AbstractMapPlaySolver(MapPlay *mapPlay);
	virtual ~AbstractMapPlaySolver() {}

	void clear();
	static void clear(MapPlay *mapPlay);

	bool loadSolverInfo(GameMapMission *mission, const GameMap::SolverInfo &info);
	static bool loadSolverInfo(MapPlay *mapPlay, GameMapMission *mission, const GameMap::SolverInfo &info);

	virtual void updateLock() = 0;
	virtual void updateXP() = 0;

protected:
	MapPlay *m_mapPlay = nullptr;

};




/**
 * @brief The MapPlaySolverAction class
 */


class MapPlaySolverAction : public AbstractMapPlaySolver
{
public:
	MapPlaySolverAction(MapPlay *mapPlay) : AbstractMapPlaySolver(mapPlay) {}

	virtual void updateLock() override;
	virtual void updateXP() override;

	int base() const { return m_base; }
	void setBase(int newBase) { m_base = newBase; }

private:
	int m_base = 100;
};




/**
 * @brief The MapPlayMission class
 */

class MapPlayMission : public QObject
{
	Q_OBJECT

	Q_PROPERTY(GameMapMission *mission READ mission CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(MapPlayMissionLevelList *missionLevelList READ missionLevelList CONSTANT)

public:
	explicit MapPlayMission(GameMapMission *mission, QObject *parent = nullptr);
	virtual ~MapPlayMission();

	GameMapMission *mission() const { return m_mission; }
	MapPlayMissionLevelList *missionLevelList() const { return m_missionLevelList; }

	QString name() const;

	GameMap::SolverInfo toSolverInfo() const;

private:
	GameMapMission *const m_mission;
	MapPlayMissionLevelList *const m_missionLevelList;
};


Q_DECLARE_METATYPE(MapPlayMissionList*)

/**
 * @brief The MapPlayMission class
 */

class MapPlayMissionLevel : public QObject
{
	Q_OBJECT

	Q_PROPERTY(GameMapMissionLevel *missionLevel READ missionLevel CONSTANT)
	Q_PROPERTY(bool deathmatch READ deathmatch CONSTANT)
	Q_PROPERTY(int lockDepth READ lockDepth WRITE setLockDepth NOTIFY lockDepthChanged)
	Q_PROPERTY(int xp READ xp WRITE setXp NOTIFY xpChanged)
	Q_PROPERTY(int level READ level CONSTANT)
	Q_PROPERTY(int solved READ solved NOTIFY solvedChanged)
	Q_PROPERTY(QString medalImage READ medalImage CONSTANT)

public:
	explicit MapPlayMissionLevel(GameMapMissionLevel *missionLevel, const bool &deathmatch, QObject *parent = nullptr);
	virtual ~MapPlayMissionLevel();

	const MapPlaySolverData &solverData() const;
	void setSolverData(const MapPlaySolverData &newSolverData);
	void solverDataIncrement();

	GameMapMissionLevel *missionLevel() const { return m_missionLevel; }

	int lockDepth() const;
	void setLockDepth(int newLockDepth);

	int xp() const;
	void setXp(int newXp);

	bool deathmatch() const { return m_deathmatch; }

	int level() const;
	int solved() const;
	QString medalImage() const;

signals:
	void lockDepthChanged();
	void xpChanged();
	void solvedChanged();

private:
	GameMapMissionLevel *const m_missionLevel = nullptr;
	MapPlaySolverData m_solverData;
	bool m_deathmatch = false;
	int m_lockDepth = 1;
	int m_xp = 0;
};

Q_DECLARE_METATYPE(MapPlayMissionLevelList*)

#endif // MAPPLAY_H
