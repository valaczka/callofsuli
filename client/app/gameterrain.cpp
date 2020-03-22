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

#include "mapreader.h"
#include "map.h"
#include "tmxlayer.h"
#include "maplistobject.h"

GameTerrain::GameTerrain(QList<TiledPaintedLayer *> *tiledLayers, QQuickItem *tiledLayersParent, QObject *parent)
	: QObject(parent)
	, m_enemies()
	, m_ladders()
	, m_blocks()
	, m_groundObjects()
	, m_playerPositions()
	, m_startPosition()
	, m_tmxFile()
	, m_map(nullptr)
	, m_tiledLayers(tiledLayers)
	, m_tiledLayersParent(tiledLayersParent)
{

}


/**
 * @brief GameTerrain::~GameTerrain
 */

GameTerrain::~GameTerrain()
{
	qDeleteAll(m_enemies.begin(), m_enemies.end());
	m_enemies.clear();

	qDeleteAll(m_blocks.begin(), m_blocks.end());
	m_blocks.clear();

	qDeleteAll(m_ladders.begin(), m_ladders.end());
	m_ladders.clear();

	m_groundObjects.clear();

	if (m_map)
		delete m_map;
}


/**
 * @brief GameTerrain::loadTmxFile
 * @param filename
 * @return
 */

bool GameTerrain::loadTmxFile(const QString &filename)
{
	m_tmxFile = filename;

	qDeleteAll(m_enemies.begin(), m_enemies.end());
	m_enemies.clear();

	qDeleteAll(m_blocks.begin(), m_blocks.end());
	m_blocks.clear();

	qDeleteAll(m_ladders.begin(), m_ladders.end());
	m_ladders.clear();

	m_groundObjects.clear();

	if (m_tmxFile.isEmpty())
		return false;


	if (!loadMap())
		return false;

	loadLayers();

	return true;
}


/**
 * @brief GameTerrain::loadMap
 * @return
 */

bool GameTerrain::loadMap()
{
	Tiled::MapReader reader;

	if (m_map)
		delete m_map;

	m_map = nullptr;

	if(!QFile::exists(m_tmxFile)) {
		qWarning() << m_tmxFile << " does not exist";
		return false;
	}

	m_map = reader.readMap(m_tmxFile);
	if (!m_map) {
		qWarning() << tr("Failed to read:") << m_tmxFile;
		return false;
	}

	return true;
}



/**
 * @brief CosGame::getBlock
 * @param num
 * @param create
 * @return
 */


GameBlock *GameTerrain::getBlock(const int &num)
{
	if (m_blocks.contains(num))
		return m_blocks.value(num);

	qDebug() << "Create block" << num;

	GameBlock *block = new GameBlock(this);
	m_blocks.insert(num, block);

	return block;
}




/**
 * @brief GameTerrain::loadLayers
 */

void GameTerrain::loadLayers()
{
	foreach(Tiled::Layer *layer, m_map->layers())
	{
		qDebug() << "Load layer" << layer->name();

		if (layer->isTileLayer() && m_tiledLayers) {
			TiledPaintedLayer *paintedLayer = new TiledPaintedLayer(m_tiledLayersParent);
			paintedLayer->setMap(m_map);
			paintedLayer->setLayer(layer->asTileLayer());
			QVariant layerZ = layer->property("z");
			paintedLayer->setX(layer->offset().x());
			paintedLayer->setY(layer->offset().y());
			paintedLayer->setZ(layerZ.isValid() ? layerZ.toInt() : -1);
			m_tiledLayers->append(paintedLayer);
		} else if (layer->isObjectGroup() && layer->name() == "Enemies") {
			loadEnemyLayer(layer);
		} else if (layer->isObjectGroup() && layer->name() == "Ground") {
			loadGroundLayer(layer);
		} else if (layer->isObjectGroup() && layer->name() == "Player") {
			loadPlayerLayer(layer);
		} else if (layer->isObjectGroup() && layer->name() == "Ladders") {
			loadLadderLayer(layer);
		}
	}
}










/**
 * @brief GameTerrain::loadEnemyLayer
 * @param layer
 */

void GameTerrain::loadEnemyLayer(Tiled::Layer *layer)
{
	qDebug() << "Load enemy layer" << layer;

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		if (!object->isPolyShape())
			continue;

		QPolygonF polygon = object->polygon();
		QRectF rect = polygon.boundingRect();

		qreal ox = object->x();
		qreal oy = object->y();

		rect.adjust(ox, oy, ox, oy);

		int block = object->property("block").toInt();

		if (block > 0) {
			GameEnemyData *enemy = new GameEnemyData(this);

			enemy->setBoundRect(rect);

			GameBlock *b = getBlock(block);
			enemy->setBlock(b);
			b->addEnemy(enemy);

			m_enemies.append(enemy);
		}
	}
}



/**
 * @brief GameScene::loadGroundLayer
 * @param layer
 */


void GameTerrain::loadGroundLayer(Tiled::Layer *layer)
{
	qDebug() << "Load ground layer" << layer;

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects)
		m_groundObjects.append(QRectF(object->x(), object->y(), object->width(), object->height()));
}



/**
 * @brief GameTerrain::loadPlayerLayer
 * @param layer
 */


void GameTerrain::loadPlayerLayer(Tiled::Layer *layer)
{
	qDebug() << "Load player layer" << layer;

	if (!layer)
		return;

	QPointF defaultStartPosition, firstStartPosition;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		qreal x = object->x();
		qreal y = object->y();
		int block = object->property("block").toInt();

		if (firstStartPosition.isNull())
			firstStartPosition = QPointF(x,y);

		if (object->property("start").toBool())
			defaultStartPosition = QPointF(x,y);

		if (block > 0) {
			GameBlock *b = getBlock(block);
			b->addPlayerPosition(QPointF(x, y));
		} else
			m_playerPositions.append(QPointF(x, y));
	}

	if (!defaultStartPosition.isNull())
		m_startPosition = defaultStartPosition;
	else
		m_startPosition = firstStartPosition;

}






/**
 * @brief GameScene::loadLadderLayer
 * @param layer
 */

void GameTerrain::loadLadderLayer(Tiled::Layer *layer)
{
	qDebug() << "Load ladders layer" << layer;

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		if (object->shape() != Tiled::MapObject::Rectangle)
			continue;

		QRectF r;
		r.setX(object->x());
		r.setY(object->y());
		r.setWidth(object->width());
		r.setHeight(object->height());

		GameLadder *ladder = new GameLadder(this);
		ladder->setBoundRect(r);
		ladder->setActive(true);

		int blockTop = object->property("blockTop").toInt();
		int blockBottom = object->property("blockBottom").toInt();

		if (blockTop > 0 && blockBottom > 0) {
			GameBlock *bTop = getBlock(blockTop);
			GameBlock *bBottom = getBlock(blockBottom);
			ladder->setBlockTop(bTop);
			ladder->setBlockBottom(bBottom);
			bTop->addLadder(ladder);
			bBottom->addLadder(ladder);
			ladder->setActive(false);
		}

		m_ladders.append(ladder);
	}
}










/**
 * @brief GameTerrain::setTiledLayersParent
 * @param tiledLayersParent
 */

void GameTerrain::setTiledLayersParent(QQuickItem *tiledLayersParent)
{
	m_tiledLayersParent = tiledLayersParent;
}

/**
 * @brief GameTerrain::setTiledLayers
 * @param tiledLayers
 */

void GameTerrain::setTiledLayers(QList<TiledPaintedLayer *> *tiledLayers)
{
	m_tiledLayers = tiledLayers;
}










