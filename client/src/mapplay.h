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

	Q_PROPERTY(GameMap *gameMap READ gameMap NOTIFY gameMapChanged)
	Q_PROPERTY(QString uuid READ uuid CONSTANT)
	Q_PROPERTY(MapPlayMissionList *missionList READ missionList CONSTANT)
	Q_PROPERTY(bool online READ online WRITE setOnline NOTIFY onlineChanged)
	Q_PROPERTY(GameState gameState READ gameState WRITE setGameState NOTIFY gameStateChanged)
	Q_PROPERTY(QJsonObject finishedData READ finishedData WRITE setFinishedData NOTIFY finishedDataChanged)
	Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly NOTIFY readOnlyChanged)

public:
	explicit MapPlay(Client *client, QObject *parent = nullptr);
	virtual ~MapPlay();

	/**
	 * @brief The GameState enum
	 */

	enum GameState {
		StateInvalid = 0,
		StateSelect,
		StateLoading,
		StatePlay,
		StateFinished
	};

	Q_ENUM(GameState)

	Q_INVOKABLE bool loadFromBinaryData(const QByteArray &data);
	Q_INVOKABLE bool loadFromFile(const QString &filename);

	static bool checkTerrains(GameMap *map);

	Client *client() const;
	QString uuid() const;

	GameMap *gameMap() const;
	void setGameMap(std::unique_ptr<GameMap> &newGameMap);
	void clearGameMap();

	AbstractMapPlaySolver *solver() const;
	void setSolver(AbstractMapPlaySolver *newSolver);

	MapPlayMissionList *missionList() const { return m_missionList.get(); }

	MapPlayMission *getMission(GameMapMissionIface *mission) const;
	MapPlayMissionLevel *getMissionLevel(GameMapMissionLevelIface *missionLevel, const bool &deathmatch) const;

	Q_INVOKABLE bool play(MapPlayMissionLevel *level, const GameMap::GameMode &mode, const QJsonObject &extended = {});
	Q_INVOKABLE virtual void updateSolver();

	Q_INVOKABLE int calculateXP(MapPlayMissionLevel *level, const GameMap::GameMode &mode) const;

	Q_INVOKABLE MapPlayMissionLevel* getNextLevel(MapPlayMissionLevel *currentLevel = nullptr, const GameMap::GameMode &mode = GameMap::Invalid) const;

	Q_INVOKABLE QVariantMap inventoryInfo(const QString &module) const;

	bool online() const;
	void setOnline(bool newOnline);

	GameState gameState() const;
	void setGameState(GameState newGameState);

	const QJsonObject &finishedData() const;
	void setFinishedData(const QJsonObject &newFinishedData);

	bool readOnly() const;
	void setReadOnly(bool newReadOnly);

	Q_INVOKABLE virtual int getShortTimeHelper(MapPlayMissionLevel */*missionLevel*/) const { return -1; }

protected:
	void loadGameMap(std::unique_ptr<GameMap> &map);
	void unloadGameMap();
	void reloadMissionList();

	virtual AbstractLevelGame *createLevelGame(MapPlayMissionLevel *level, const GameMap::GameMode &mode);
	virtual void onCurrentGamePrepared();
	virtual void onCurrentGameFinished();

signals:
	void missionLevelUnlocked(QList<MapPlayMissionLevel*> list);
	void currentGameFailed();
	void gameMapLoaded();
	void gameMapUnloaded();
	void gameMapChanged();
	void onlineChanged();
	void gameStateChanged();
	void finishedDataChanged();
	void readOnlyChanged();
	void shortTimeHelperUpdated();

protected:
	Client *const m_client ;
	std::unique_ptr<GameMap> m_gameMap;
	std::unique_ptr<AbstractMapPlaySolver> m_solver;
	std::unique_ptr<MapPlayMissionList> m_missionList;
	bool m_online = true;
	GameState m_gameState = StateInvalid;
	QJsonObject m_finishedData;
	bool m_readOnly = false;
	QJsonObject m_extendedData;
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

	virtual int calculateXP(MapPlayMissionLevel *level, const GameMap::GameMode &mode) const = 0;
	virtual QList<MapPlayMissionLevel*> updateLock() = 0;
	virtual void updateXP();

	const QStringList &forceUnlockMissionList() const;
	void setForceUnlockMissionList(const QStringList &newForceUnlockMissionList);

protected:
	MapPlay *m_mapPlay = nullptr;
	QStringList m_forceUnlockMissionList;

};




/**
 * @brief The MapPlaySolverAction class
 */


class MapPlaySolverDefault : public AbstractMapPlaySolver
{
public:
	MapPlaySolverDefault(MapPlay *mapPlay) : AbstractMapPlaySolver(mapPlay) {}

	virtual int calculateXP(MapPlayMissionLevel *level, const GameMap::GameMode &mode) const override;
	virtual QList<MapPlayMissionLevel*> updateLock() override;

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
	Q_PROPERTY(QString description READ description CONSTANT)
	Q_PROPERTY(QString uuid READ uuid CONSTANT)
	Q_PROPERTY(int lockDepth READ lockDepth WRITE setLockDepth NOTIFY lockDepthChanged)
	Q_PROPERTY(MapPlayMissionLevelList *missionLevelList READ missionLevelList CONSTANT)
	Q_PROPERTY(QString medalImage READ medalImage CONSTANT)

public:
	explicit MapPlayMission(GameMapMission *mission, QObject *parent = nullptr);
	virtual ~MapPlayMission();

	GameMapMission *mission() const;
	MapPlayMissionLevelList *missionLevelList() const { return m_missionLevelList; }

	QString name() const;
	QString uuid() const;
	QString description() const;

	GameMap::SolverInfo toSolverInfo() const;

	QString medalImage() const;

	static bool modeEnabled(GameMapMission *mission, const GameMap::GameMode &mode);
	Q_INVOKABLE bool modeEnabled(const GameMap::GameMode &mode) const { return modeEnabled(m_mission, mode); }

	int lockDepth() const;
	void setLockDepth(int newLockDepth);

signals:
	void lockDepthChanged();

private:
	GameMapMission * m_mission = nullptr;
	MapPlayMissionLevelList *m_missionLevelList = nullptr;
	int m_lockDepth = -1;
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
	Q_PROPERTY(qreal passed READ passed CONSTANT)
	Q_PROPERTY(int solved READ solved NOTIFY solvedChanged)
	Q_PROPERTY(QString medalImage READ medalImage CONSTANT)
	Q_PROPERTY(MapPlayMission *mission READ mission CONSTANT)

public:
	explicit MapPlayMissionLevel(MapPlayMission *mission, GameMapMissionLevel *missionLevel, const bool &deathmatch, QObject *parent = nullptr);
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
	qreal passed() const;
	QString medalImage() const;

	MapPlayMission *mission() const;

	static bool modeEnabled(GameMapMissionLevel *level, const GameMap::GameMode &mode);
	Q_INVOKABLE bool modeEnabled(const GameMap::GameMode &mode) const { return modeEnabled(m_missionLevel, mode); }

signals:
	void lockDepthChanged();
	void xpChanged();
	void solvedChanged();

private:
	MapPlayMission *const m_mission;
	GameMapMissionLevel *const m_missionLevel;
	MapPlaySolverData m_solverData;
	bool m_deathmatch = false;
	int m_lockDepth = 1;
	int m_xp = 0;
};

Q_DECLARE_METATYPE(MapPlayMissionLevelList*)

#endif // MAPPLAY_H
