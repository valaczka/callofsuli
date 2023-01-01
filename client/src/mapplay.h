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
#include "abstractgame.h"
#include "mapplaymission.h"

class Client;
class GameMap;

class MapPlay : public QObject
{
	Q_OBJECT

	Q_PROPERTY(AbstractGame::Mode gameMode READ gameMode WRITE setGameMode NOTIFY gameModeChanged)
	Q_PROPERTY(GameMap *gameMap READ gameMap WRITE setGameMap NOTIFY gameMapChanged)
	Q_PROPERTY(MapPlayMissionList* missionList READ missionList WRITE setMissionList NOTIFY missionListChanged)

public:
	explicit MapPlay(Client *client, QObject *parent = nullptr);
	virtual ~MapPlay();

	Q_INVOKABLE bool loadFromBinaryData(const QByteArray &data);
	Q_INVOKABLE bool loadFromFile(const QString &filename);

	Q_INVOKABLE void reloadMissionList();

	static bool checkTerrains(GameMap *map);

	Client *client() const;

	const AbstractGame::Mode &gameMode() const;
	void setGameMode(const AbstractGame::Mode &newGameMode);

	GameMap *gameMap() const;
	void setGameMap(GameMap *newGameMap);

	MapPlayMissionList *missionList() const;
	void setMissionList(MapPlayMissionList *newMissionList);

	Q_INVOKABLE void play(MapPlayMissionLevel *level);

protected:
	void loadGameMap(GameMap *map);
	void unloadGameMap();

signals:
	void gameMapLoaded();
	void gameMapUnloaded();
	void gameModeChanged();
	void gameMapChanged();

	void missionListChanged();

protected:
	Client *const m_client = nullptr;
	AbstractGame::Mode m_gameMode = AbstractGame::Action;
	GameMap *m_gameMap = nullptr;

private:
	MapPlayMissionList *m_missionList = nullptr;
};


#endif // MAPPLAY_H
