/*
 * ---- Call of Suli ----
 *
 * gameterrainmap.cpp
 *
 * Created on: 2022. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameTerrainMap
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

#include "gameterrainmap.h"
#include "Logger.h"
#include "client.h"
#include "libtiled/mapreader.h"
#include "libtiled/objectgroup.h"


GameTerrainMap::GameTerrainMap(const QString &filename)
{
	if (!filename.isEmpty())
		loadMapFromFile(filename);
}


/**
 * @brief GameTerrainMap::~GameTerrainMap
 */

GameTerrainMap::~GameTerrainMap()
{
	m_map.reset(nullptr);
}


/**
 * @brief GameTerrainMap::loadMapFromFile
 * @param filename
 * @return
 */

bool GameTerrainMap::loadMapFromFile(QString filename)
{
	m_isValid = false;
	m_enemies.clear();
	m_blocks.clear();
	m_objects.clear();
	m_playerPositions.clear();
	m_pickables.clear();

	Tiled::MapReader reader;

	if (filename.startsWith(QStringLiteral("qrc:")))
		filename.replace(QStringLiteral("qrc:"), QStringLiteral(":"));

	if(!QFile::exists(filename)) {
		LOG_CWARNING("client") << "A terepfájl nem létezik:" << qPrintable(filename);
		return false;
	}

	LOG_CDEBUG("client") << "Load map from file:" << qPrintable(filename);

	m_map.reset(nullptr);

	m_map = reader.readMap(filename);

	if (!m_map) {
		LOG_CWARNING("client") << "Failed to read" << qPrintable(filename) << "error:" << qPrintable(reader.errorString());
		return false;
	}

	if (loadObjectLayers()) {
		m_isValid = true;

		LOG_CDEBUG("client") << "Loaded enemies:" << enemies().size();
		LOG_CDEBUG("client") << "Loaded blocks:" << blocks().size();
		LOG_CDEBUG("client") << "Loaded fires:" << fires().size();
		LOG_CDEBUG("client") << "Loaded fences:" << fences().size();
		LOG_CDEBUG("client") << "Loaded teleports:" << teleports().size();
		LOG_CDEBUG("client") << "Loaded pickables:" << m_pickables.size();

		return true;
	}

	LOG_CWARNING("client") << "Failed to read layers:" << qPrintable(filename);



	return false;
}


/**
 * @brief GameTerrainMap::loadMap
 * @param terrain
 * @param level
 * @return
 */

bool GameTerrainMap::loadMap(const QString &terrain, const int &level)
{
	return loadMapFromFile(QStringLiteral(":/terrain/%1/level%2.tmx").arg(terrain).arg(level));
}



/**
 * @brief GameTerrainMap::loadObjectLayers
 * @return
 */

bool GameTerrainMap::loadObjectLayers()
{
	for (auto layer = m_map->objectGroups().begin(); layer != m_map->objectGroups().end(); ++layer) {
		const QString &name = layer->name();
		LOG_CDEBUG("client") << "Load layer:" << name;

		Tiled::ObjectGroup *gLayer = static_cast<Tiled::ObjectGroup*>(*layer);

		if (name == QStringLiteral("Enemies"))
			readEnemyLayer(gLayer);
		else if (name == QStringLiteral("Objects"))
			readObjectLayer(gLayer);
		else if (name == QStringLiteral("Player"))
			readPlayerLayer(gLayer);
		else if (name == QStringLiteral("Items") || name == QStringLiteral("Pickables"))
			readPickableLayer(gLayer);

	}

	return true;
}



/**
 * @brief GameTerrainMap::width
 * @return
 */

int GameTerrainMap::width() const
{
	if (m_map)
		return m_map->width() * m_map->tileWidth();
	else
		return 0;
}



/**
 * @brief GameTerrainMap::height
 * @return
 */

int GameTerrainMap::height() const
{
	if (m_map)
		return m_map->height() * m_map->tileHeight();
	else
		return 0;
}



/**
 * @brief GameTerrainMap::readEnemyLayer
 * @param layer
 */

void GameTerrainMap::readEnemyLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		LOG_CERROR("client") << "Invalid object layer";
		return;
	}

	foreach (Tiled::MapObject *object, layer->objects()) {
		const int &block = object->property("block").toInt();

		if (block > 0 && !m_blocks.contains(block))
			m_blocks.append(block);

		const qreal &ox = object->x();
		const qreal &oy = object->y();

		EnemyData enemy;

		if (object->shape() == Tiled::MapObject::Polyline || object->shape() == Tiled::MapObject::Polygon) {
			QPolygonF polygon = object->polygon();
			enemy.rect = polygon.boundingRect();
			enemy.rect.adjust(ox, oy, ox, oy);
		} else {
			enemy.rect = QRectF(ox, oy, 1, 1);
		}


		if (object->type() == QStringLiteral("sniper"))
			enemy.type = EnemySniper;
		else
			enemy.type = EnemySoldier;

		if (block > 0)
			enemy.block = block;

		LOG_CDEBUG("client") << "Add enemy" << enemy.type << enemy.rect << enemy.block;

		m_enemies.append(enemy);
	}
}



/**
 * @brief GameTerrainMap::readObjectLayer
 * @param layer
 */

void GameTerrainMap::readObjectLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		LOG_CERROR("client") << "Invalid object layer";
		return;
	}

	foreach (Tiled::MapObject *object, layer->objects()) {
		const QString type = object->type();
		ObjectType o = Invalid;

		if (type == QStringLiteral("fire"))
			o = Fire;
		else if (type == QStringLiteral("fence"))
			o = Fence;
		else if (type == QStringLiteral("teleport"))
			o = Teleport;

		if (o != Invalid) {
			ObjectData d;
			d.point = object->position();
			d.type = o;

			m_objects.append(d);

		} else {
			LOG_CWARNING("client") << "Invalid map object type:" << type;
		}
	}
}



/**
 * @brief GameTerrainMap::readPlayerLayer
 * @param layer
 */

void GameTerrainMap::readPlayerLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		LOG_CERROR("client") << "Invalid object layer";
		return;
	}

	foreach (Tiled::MapObject *object, layer->objects()) {
		const int &block = object->property("block").toInt();

		if (block <= 0) {
			LOG_CWARNING("client") << "Invalid player position block:" << object->position();
			continue;
		}

		PlayerPositionData d;
		d.point = object->position();
		d.block = block;
		d.start = object->property("start").toBool();

		m_playerPositions.append(d);
	}
}




/**
 * @brief GameTerrainMap::readPickableLayer
 * @param layer
 */

void GameTerrainMap::readPickableLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		LOG_CERROR("client") << "Invalid object layer";
		return;
	}

	foreach (Tiled::MapObject *object, layer->objects()) {
		const QString type = object->type();
		GamePickable::GamePickableData o = GamePickable::pickableDataHash().value(type);

		if (o.type != GamePickable::PickableInvalid) {
			PickableData d;
			d.point = object->position();
			d.data = o;

			m_pickables.append(d);

		} else {
			LOG_CWARNING("client") << "Invalid map object type:" << type;
		}
	}
}


/**
 * @brief GameTerrainMap::map
 * @return
 */

Tiled::Map*GameTerrainMap::map() const
{
	return m_map.get();
}
