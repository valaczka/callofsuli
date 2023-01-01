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
#include "actiongame.h"
#include "mapimage.h"
#include "utils.h"
#include "application.h"
#include "gamemap.h"
#include "websocketmessage.h"
#include "gameterrain.h"
#include <QScopedPointer>

MapPlay::MapPlay(Client *client, QObject *parent)
	: QObject{(parent ? parent : client)}
	, m_client(client)
	, m_missionList(new MapPlayMissionList(this))
{
	Q_ASSERT(m_client);

	qCDebug(lcGame).noquote() << tr("Map play object created:") << this;
}

/**
 * @brief MapPlay::~MapPlay
 */
MapPlay::~MapPlay()
{
	unloadGameMap();

	delete m_missionList;

	qCDebug(lcGame).noquote() << tr("Map play object destroyed:") << this;
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

	if (map->appVersion() > 0 && map->appVersion() > WebSocketMessage::versionCode()) {
		Application::instance()->messageWarning(tr("A pálya az alkalmazásnál magasabb verziószámmal készült.\nFrissítsd az alkalmazást a legújabb verzióra!"),
												tr("Frissítés szükséges"));
		return false;
	}

	loadGameMap(map.take());

	reloadMissionList();

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
 * @brief MapPlay::reloadMissionList
 */

void MapPlay::reloadMissionList()
{
	if (!m_gameMap) {
		m_missionList->clear();
		return;
	}

	foreach (GameMapMission *m, m_gameMap->missions()) {
		MapPlayMissionLevelList *mllist = new MapPlayMissionLevelList(this);
		//QVariantList levelList;

		foreach (GameMapMissionLevel *ml, m->levels()) {
			MapPlayMissionLevel *mll = new MapPlayMissionLevel(this);
			mll->setLevel(ml->level());
			mll->setDeathmatch(false);
			mll->setMapLevel(ml);
			mllist->append(mll);

			if (ml->canDeathmatch()) {
				MapPlayMissionLevel *mll = new MapPlayMissionLevel(this);
				mll->setLevel(ml->level());
				mll->setDeathmatch(true);
				mll->setMapLevel(ml);
				mllist->append(mll);
			}
		}

		MapPlayMission *mm = new MapPlayMission(this);
		mm->setName(m->name());
		mm->setLevels(mllist);
		m_missionList->append(mm);
	}


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
			const QString &terrainDir = terrain.section("/", 0, -2);
			const int &terrainLevel = terrain.section("/", -1, -1).toInt();

			if (!GameTerrain::terrainAvailable(terrainDir, terrainLevel)) {
				qCWarning(lcGame).noquote() << tr("Missing terrain:") << terrain;
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

const AbstractGame::Mode &MapPlay::gameMode() const
{
	return m_gameMode;
}

void MapPlay::setGameMode(const AbstractGame::Mode &newGameMode)
{
	if (m_gameMode == newGameMode)
		return;
	m_gameMode = newGameMode;
	emit gameModeChanged();
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

	qCDebug(lcGame).noquote() << tr("Load gamemap:") << map;

	setGameMap(map);

	if (!map)
		return;

	qCDebug(lcGame).noquote() << tr("Add mapimage provider for map:") << map->uuid();
	MapImage *mapImage = new MapImage(map);
	Application::instance()->engine()->addImageProvider("mapimage", mapImage);

	emit gameMapLoaded();
}




/**
 * @brief MapPlay::unloadGameMap
 */

void MapPlay::unloadGameMap()
{
	if (Application::instance() && Application::instance()->engine() && Application::instance()->engine()->imageProvider("mapimage")) {
		qCDebug(lcGame).noquote() << tr("Remove image provider mapimage");
		Application::instance()->engine()->removeImageProvider("mapimage");
	}

	if (m_gameMap) {
		qCDebug(lcGame).noquote() << tr("Delete gamemap:") << m_gameMap;
		delete m_gameMap;

		setGameMap(nullptr);

		emit gameMapUnloaded();
	}
}


MapPlayMissionList *MapPlay::missionList() const
{
	return m_missionList;
}

void MapPlay::setMissionList(MapPlayMissionList *newMissionList)
{
	if (m_missionList == newMissionList)
		return;
	m_missionList = newMissionList;
	emit missionListChanged();
}


/**
 * @brief MapPlay::play
 * @param level
 */

void MapPlay::play(MapPlayMissionLevel *level)
{
	qDebug() << "PLAY" << level << level->level() << level->mapLevel();

	ActionGame *game = new ActionGame(level->mapLevel(), m_client);
	game->setDeathmatch(level->deathmatch());

	m_client->setCurrentGame(game);

	game->load();
}
