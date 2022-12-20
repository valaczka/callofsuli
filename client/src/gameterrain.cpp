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
#include "client.h"
#include "libtiled/mapreader.h"


GameTerrain::GameTerrain(const QString &filename)
{
	if (!filename.isEmpty())
		loadMapFromFile(filename);
}


/**
 * @brief GameTerrain::~GameTerrain
 */

GameTerrain::~GameTerrain()
{
	m_map.reset(nullptr);
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
 * @brief GameTerrain::loadMap
 * @return
 */

bool GameTerrain::loadMapFromFile(QString filename)
{
	m_isValid = false;
	m_enemies.clear();
	m_blocks.clear();
	m_fireCount = 0;
	m_fenceCount = 0;
	m_teleportCount = 0;

	Tiled::MapReader reader;

	if (filename.startsWith("qrc:"))
		filename.replace("qrc:", ":");

	if(!QFile::exists(filename)) {
		qCWarning(lcClient).noquote() << QObject::tr("A terepfájl nem létezik:") << filename;
		return false;
	}

	qCDebug(lcClient).noquote() << QObject::tr("Load map from file:") << filename;

	m_map.reset(nullptr);
	m_map = reader.readMap(filename);

	if (!m_map) {
		qCWarning(lcClient).noquote() << QObject::tr("Failed to read %1 (%2)").arg(filename, reader.errorString());
		return false;
	}

	if (loadObjectLayers()) {
		m_isValid = true;

		qCDebug(lcClient).noquote() << QObject::tr("Loaded enemies: %1").arg(enemies().size());
		qCDebug(lcClient).noquote() << QObject::tr("Loaded blocks: %1").arg(blocks().size());
		qCDebug(lcClient).noquote() << QObject::tr("Loaded fires: %1").arg(fires().size());
		qCDebug(lcClient).noquote() << QObject::tr("Loaded fences: %1").arg(fences().size());
		qCDebug(lcClient).noquote() << QObject::tr("Loaded teleports: %1").arg(teleports().size());

		return true;
	}

	qCWarning(lcClient) << QObject::tr("Failed to read layers:") << filename;



	return false;
}



/**
 * @brief GameTerrain::loadMap
 * @param terrain
 * @param level
 * @return
 */

bool GameTerrain::loadMap(const QString &terrain, const int &level)
{
	return loadMapFromFile(QString(":/terrain/%1/level%2.tmx").arg(terrain).arg(level));
}




/**
 * @brief GameTerrain::loadObjectLayers
 * @return
 */

bool GameTerrain::loadObjectLayers()
{
	for (auto layer = m_map->objectGroups().begin(); layer != m_map->objectGroups().end(); ++layer) {
		const QString &name = layer->name();
		qCDebug(lcClient).noquote() << QObject::tr("Load layer:") << name;

		Tiled::ObjectGroup *gLayer = static_cast<Tiled::ObjectGroup*>(*layer);


		if (name == "Enemies")
			readEnemyLayer(gLayer);
		else if (name == "Objects")
			readObjectLayer(gLayer);
		else if (name == "Player")
			readPlayerLayer(gLayer);

		/*if (layer->isObjectGroup() && layer->name() == "Enemies")
		loadEnemyLayer(layer);
	} else if (layer->isObjectGroup() && layer->name() == "Ground") {
		loadGroundLayer(layer);
	} else if (layer->isObjectGroup() && layer->name() == "Player") {
		loadPlayerLayer(layer);
	} else if (layer->isObjectGroup() && layer->name() == "Ladders") {
		loadLadderLayer(layer);
	} else if (layer->isObjectGroup() && layer->name() == "Objects") {
		loadObjectLayer(layer);
	} else if (layer->isObjectGroup() && layer->name() == "Items") {
		loadItemLayer(layer);
	} else if (layer->isObjectGroup() && layer->name() == "Preview") {
		loadPreviewLayer(layer);
	}*/
	}

	return true;
}



/**
 * @brief GameTerrain::width
 * @return
 */

int GameTerrain::width() const
{
	if (m_map)
		return m_map->width() * m_map->tileWidth();
	else
		return 0;
}





/**
 * @brief GameTerrain::height
 * @return
 */

int GameTerrain::height() const
{
	if (m_map)
		return m_map->height() * m_map->tileHeight();
	else
		return 0;
}


/**
 * @brief GameTerrain::map
 * @return
 */

Tiled::Map* GameTerrain::map() const
{
	return m_map.get();
}


/**
 * @brief GameTerrain::readEnemyLayer
 */

void GameTerrain::readEnemyLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		qCCritical(lcClient).noquote() << QObject::tr("Invalid object layer");
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


		if (object->type() == "sniper")
			enemy.type = EnemySniper;
		else
			enemy.type = EnemySoldier;

		if (block > 0)
			enemy.block = block;

		qCDebug(lcClient).noquote() << QObject::tr("Add enemy") << enemy.type << enemy.rect << enemy.block;

		m_enemies.append(enemy);
	}
}



/**
 * @brief GameTerrain::readObjectLayer
 */

void GameTerrain::readObjectLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		qCCritical(lcClient).noquote() << QObject::tr("Invalid object layer");
		return;
	}

	foreach (Tiled::MapObject *object, layer->objects()) {
		const QString type = object->type();
		ObjectType o = Invalid;

		if (type == "fire")
			o = Fire;
		else if (type == "fence")
			o = Fence;
		else if (type == "teleport")
			o = Teleport;

		if (o != Invalid) {
			ObjectData d;
			d.point = object->position();
			d.type = o;

			m_objects.append(d);

		} else {
			qCWarning(lcClient).noquote() << QObject::tr("Invalid map object type:") << type;
		}
	}

}


/**
 * @brief GameTerrain::readPlayerLayer
 * @param layer
 */

void GameTerrain::readPlayerLayer(Tiled::ObjectGroup *layer)
{
	if (!layer) {
		qCCritical(lcClient).noquote() << QObject::tr("Invalid object layer");
		return;
	}

	foreach (Tiled::MapObject *object, layer->objects()) {
		const int &block = object->property("block").toInt();

		if (block <= 0) {
			qCWarning(lcClient).noquote() << QObject::tr("Invalid player position block:") << object->position();
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
 * @brief GameTerrain::playerPositions
 * @return
 */

const QList<GameTerrain::PlayerPositionData> &GameTerrain::playerPositions() const
{
	return m_playerPositions;
}


/**
 * @brief GameTerrain::defaultPlayerPosition
 * @return
 */

GameTerrain::PlayerPositionData GameTerrain::defaultPlayerPosition() const
{
	PlayerPositionData pos;

	pos.block = -1;

	foreach (const PlayerPositionData &data, m_playerPositions) {
		if (data.start && (pos.block == -1 || data.block < pos.block))
			pos = data;
	}

	return pos;
}




/**
 * @brief GameTerrain::objects
 * @return
 */

const QList<GameTerrain::ObjectData> &GameTerrain::objects() const
{
	return m_objects;
}


/**
 * @brief GameTerrain::blocks
 * @return
 */

const QList<int> &GameTerrain::blocks() const
{
	return m_blocks;
}






/**
 * @brief GameTerrain::objects
 * @param type
 * @return
 */

QList<GameTerrain::ObjectData> GameTerrain::objects(const ObjectType &type) const
{
	QList<ObjectData> list;
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

const QList<GameTerrain::EnemyData> &GameTerrain::enemies() const
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

	QList<Tiled::MapObject*> objects = og->objects();

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

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects)
		m_preview.append(PreviewData(object->position(), object->property("text").toString()));

	emit previewChanged();
}
*/




/**
 * @brief GameTerrain::setTiledLayers
 * @param tiledLayers
 */

/*
void GameTerrain::setTiledLayers(QList<TiledPaintedLayer *> *tiledLayers)
{
	m_tiledLayers = tiledLayers;
}
*/








/*

const QList<GameTerrain::PreviewData> &GameTerrain::preview() const
{
	return m_preview;
}

void GameTerrain::setPreview(const QList<PreviewData> &newPreview)
{
	if (m_preview == newPreview)
		return;
	m_preview = newPreview;
	emit previewChanged();
}
*/
