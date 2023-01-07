/*
 * ---- Call of Suli ----
 *
 * gameterrain.cpp
 *
 * Created on: 2020. 11. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameTerrain
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gameterrain.h"
#include "Logger.h"
#include "client.h"
#include "gameterrainmap.h"
#include "qdiriterator.h"


QVector<GameTerrain> GameTerrain::m_availableTerrains;



/**
 * @brief GameTerrain::GameTerrain
 * @param filename
 */


GameTerrain::GameTerrain()
{

}


/**
 * @brief GameTerrain::~GameTerrain
 */

GameTerrain::~GameTerrain()
{

}



/**
 * @brief GameTerrain::isValid
 * @return
 */

bool GameTerrain::isValid() const
{
	return m_isValid;
}









/**
 * @brief GameTerrain::pickables
 * @return
 */

const QVector<GameTerrain::PickableData> &GameTerrain::pickables() const
{
	return m_pickables;
}



/**
 * @brief GameTerrain::playerPositions
 * @return
 */

const QVector<GameTerrain::PlayerPositionData> &GameTerrain::playerPositions() const
{
	return m_playerPositions;
}


/**
 * @brief GameTerrain::defaultPlayerPosition
 * @return
 */

GameTerrain::PlayerPositionData GameTerrain::defaultPlayerPosition() const
{
	foreach (const PlayerPositionData &data, m_playerPositions) {
		if (data.start)
			return data;
	}

	if (!m_playerPositions.isEmpty())
		return m_playerPositions.first();

	return PlayerPositionData();
}



/**
 * @brief GameTerrain::reloadAvailableTerrains
 */

void GameTerrain::reloadAvailableTerrains()
{
	LOG_CINFO("client") << "Reload terrains...";

	const QString cache = Utils::standardPath("terrain_cache.json");

	LOG_CINFO("client") << "Terrain cache:" << cache;

	QJsonObject terrainCache = Utils::fileToJsonObject(cache);

	m_availableTerrains.clear();

	QDirIterator it(QStringLiteral(":/terrain"), {QStringLiteral("level1.tmx")}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		const QString &realname = it.next();

		const QString &terrainName = realname.section('/',-2,-2);

		const QString &terrainDir = QStringLiteral(":/terrain/")+terrainName;
		const QString &dataFile = terrainDir+QStringLiteral("/data.json");

		QString displayName = terrainName;
		QString bgImage;
		QString bgMusic;

		if (QFile::exists(dataFile)) {
			const QJsonObject &data = Utils::fileToJsonObject(dataFile);

			if (data.contains(QStringLiteral("name")))
				displayName = data.value(QStringLiteral("name")).toString();

			if (data.contains(QStringLiteral("backgroundMusic"))) {
				bgMusic = data.value(QStringLiteral("backgroundMusic")).toString();
				if (!bgMusic.startsWith(QStringLiteral(":/")) && !bgMusic.startsWith(QStringLiteral("/"))) {
					bgMusic.prepend(terrainDir+QStringLiteral("/"));
				}
			}

			if (data.contains(QStringLiteral("backgroundImage"))) {
				bgImage = data.value(QStringLiteral("backgroundImage")).toString();
				if (!bgImage.startsWith(QStringLiteral(":/")) && !bgImage.startsWith(QStringLiteral("/"))) {
					bgImage.prepend(terrainDir+QStringLiteral("/"));
				}
			}
		}


		for (int level=1; level<=3; level++) {
			const QString tmxFile = QStringLiteral("%1/level%2.tmx").arg(terrainDir).arg(level);

			QFileInfo tmxI(tmxFile);

			if (!tmxI.exists())
				continue;

			LOG_CINFO("client") << "Load terrain" << terrainName << "level" << level;

			const qint64 &tmxModified = tmxI.lastModified().toMSecsSinceEpoch();

			const QJsonObject &cacheObject = terrainCache.value(tmxFile).toObject();
			double cacheModified = cacheObject.value(QLatin1String("lastModified")).toDouble();

			if (cacheModified > tmxModified) {
				GameTerrain t = GameTerrain::fromJsonObject(cacheObject);
				t.m_name = terrainName;
				t.m_level = level;
				t.m_displayName = displayName;
				t.m_backgroundImage = bgImage;
				t.m_backgroundMusic = bgMusic;
				m_availableTerrains.append(t);
			} else {
				GameTerrainMap t;
				if (!t.loadMapFromFile(tmxFile)) {
					LOG_CWARNING("debug") << "Can't load terrain:" << qPrintable(tmxFile);
					continue;
				}
				QJsonObject obj = t.toJsonObject();
				obj[QStringLiteral("lastModified")] = QDateTime::currentDateTime().toMSecsSinceEpoch();

				t.m_name = terrainName;
				t.m_level = level;
				t.m_displayName = displayName;
				t.m_backgroundImage = bgImage;
				t.m_backgroundMusic = bgMusic;

				m_availableTerrains.append(t);

				terrainCache[tmxFile] = obj;
			}
		}

	}

	LOG_CINFO("client") << "...loaded" << m_availableTerrains.size() << "terrains";

	LOG_CDEBUG("client") << "Save terrain cache:" << cache;

	Utils::jsonObjectToFile(terrainCache, cache);
}


/**
 * @brief GameTerrain::availableTerrains
 * @return
 */

const QVector<GameTerrain> &GameTerrain::availableTerrains()
{
	return m_availableTerrains;
}


/**
 * @brief GameTerrain::terrainAvailable
 * @param name
 * @param level
 * @return
 */

bool GameTerrain::terrainAvailable(const QString &name, const int &level)
{
	foreach (const GameTerrain &t, m_availableTerrains) {
		if (t.name() == name && t.level() == level)
			return true;
	}

	return false;
}


/**
 * @brief GameTerrain::terrainAvailable
 * @param missionLevelName
 * @return
 */

bool GameTerrain::terrainAvailable(const QString &missionLevelName)
{
	const QString &terrainDir = missionLevelName.section("/", 0, -2);
	const int &terrainLevel = missionLevelName.section("/", -1, -1).toInt();

	return terrainAvailable(terrainDir, terrainLevel);
}


/**
 * @brief GameTerrain::terrain
 * @param missionLevelName
 * @return
 */

GameTerrain GameTerrain::terrain(const QString &missionLevelName)
{
	const QString &terrainDir = missionLevelName.section("/", 0, -2);
	const int &terrainLevel = missionLevelName.section("/", -1, -1).toInt();

	foreach (const GameTerrain &t, m_availableTerrains) {
		if (t.name() == terrainDir && t.level() == terrainLevel)
			return t;
	}

	return GameTerrain();
}


/**
 * @brief GameTerrain::toJsonObject
 * @return
 */

QJsonObject GameTerrain::toJsonObject() const
{
	QJsonObject o;

	o[QStringLiteral("enemies")] = m_enemies.count();
	o[QStringLiteral("fires")] = fires().count();
	o[QStringLiteral("fences")] = fences().count();
	o[QStringLiteral("teleports")] = teleports().count();
	o[QStringLiteral("blocks")] = blocks().count();
	o[QStringLiteral("pickables")] = pickables().count();

	return o;
}


/**
 * @brief GameTerrain::fromJsonObject
 * @param object
 * @return
 */

GameTerrain GameTerrain::fromJsonObject(const QJsonObject &object)
{
	GameTerrain t;

	for (int i=0; i<object.value("enemies").toInt(); ++i)
		t.m_enemies.append(EnemyData());

	for (int i=0; i<object.value("fires").toInt(); ++i) {
		ObjectData o;
		o.type = Fire;
		t.m_objects.append(o);
	}

	for (int i=0; i<object.value("fences").toInt(); ++i) {
		ObjectData o;
		o.type = Fence;
		t.m_objects.append(o);
	}

	for (int i=0; i<object.value("teleports").toInt(); ++i) {
		ObjectData o;
		o.type = Teleport;
		t.m_objects.append(o);
	}

	for (int i=0; i<object.value("blocks").toInt(); ++i)
		t.m_blocks.append(-1);

	for (int i=0; i<object.value("pickables").toInt(); ++i)
		t.m_pickables.append(PickableData());

	return t;
}

const QString &GameTerrain::name() const
{
	return m_name;
}

void GameTerrain::setName(const QString &newName)
{
	m_name = newName;
}

int GameTerrain::level() const
{
	return m_level;
}

void GameTerrain::setLevel(int newLevel)
{
	m_level = newLevel;
}

const QString &GameTerrain::displayName() const
{
	return m_displayName;
}

void GameTerrain::setDisplayName(const QString &newDisplayName)
{
	m_displayName = newDisplayName;
}

const QString &GameTerrain::backgroundImage() const
{
	return m_backgroundImage;
}

void GameTerrain::setBackgroundImage(const QString &newBackgroundImage)
{
	m_backgroundImage = newBackgroundImage;
}

const QString &GameTerrain::backgroundMusic() const
{
	return m_backgroundMusic;
}

void GameTerrain::setBackgroundMusic(const QString &newBackgroundMusic)
{
	m_backgroundMusic = newBackgroundMusic;
}




/**
 * @brief GameTerrain::objects
 * @return
 */

const QVector<GameTerrain::ObjectData> &GameTerrain::objects() const
{
	return m_objects;
}


/**
 * @brief GameTerrain::blocks
 * @return
 */

const QVector<int> &GameTerrain::blocks() const
{
	return m_blocks;
}






/**
 * @brief GameTerrain::objects
 * @param type
 * @return
 */

QVector<GameTerrain::ObjectData> GameTerrain::objects(const ObjectType &type) const
{
	QVector<ObjectData> list;
	foreach (const ObjectData &data, m_objects) {
		if (data.type == type)
			list.append(data);
	}
	return list;
}



/**
 * @brief GameTerrain::enemies
 * @return
 */

const QVector<GameTerrain::EnemyData> &GameTerrain::enemies() const
{
	return m_enemies;
}














/**
 * @brief GameTerrain::loadItemLayer
 * @param layer
 */

/*
void GameTerrain::loadItemLayer(Tiled::Layer *layer)
{
	qDebug() << "Load item layer" << layer;

	if (!layer)
		return;


	QHash<QString, GameEnemyData::InventoryType> inventoryTypes = GameEnemyData::inventoryTypes();



	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QVector<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		QString type = object->type();

		if (!inventoryTypes.contains(type)) {
			qWarning() << "Invalid item" << type;
			continue;
		}

		m_items.append(GameTerrainItem(object->position(), inventoryTypes.value(type)));
	}
}
*/

/**
 * @brief GameTerrain::loadPreviewLayer
 * @param layer
 */

/*
void GameTerrain::loadPreviewLayer(Tiled::Layer *layer)
{
	qDebug() << "Load preview layer" << layer;

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QVector<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects)
		m_preview.append(PreviewData(object->position(), object->property("text").toString()));

	emit previewChanged();
}
*/


