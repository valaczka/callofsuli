/*
 * ---- Call of Suli ----
 *
 * gamesceneprivate.cpp
 *
 * Created on: 2020. 10. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameScenePrivate
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

#include "gamescene.h"

#include "mapreader.h"
#include "map.h"
#include "tmxlayer.h"
#include "scene.h"
#include "cosgame.h"
#include "gameobject.h"
#include "gameladder.h"
#include "mapobject.h"

GameScene::GameScene(QQuickItem *parent)
	: QQuickItem(parent)
	, m_map(nullptr)
	, m_tiledLayers()
	, m_game(nullptr)
{

}


GameScene::~GameScene()
{
	if (m_map)
		delete m_map;

	m_map = nullptr;

	qDeleteAll(m_tiledLayers.begin(), m_tiledLayers.end());
	m_tiledLayers.clear();

}


/**
 * @brief GameScenePrivate::setSource
 * @param source
 */

void GameScene::setSource(QUrl source)
{
	if (m_source == source)
		return;

	m_source = source;

	QString sourceAsString;
	if (m_source.url().startsWith("qrc:/"))
		sourceAsString = m_source.url().replace(QString("qrc:/"), QString(":/"));
	else
		sourceAsString = m_source.toLocalFile();

	if (!loadMap(sourceAsString))
		return;

	setImplicitWidth(m_map->width() * m_map->tileWidth());
	setImplicitHeight(m_map->height() * m_map->tileHeight());

	loadLayers();

	emit sourceChanged(m_source);
}

/**
 * @brief GameScenePrivate::setTiledLayers
 * @param tiledLayers
 */


void GameScene::setTiledLayers(QList<TiledPaintedLayer *> tiledLayers)
{
	if (m_tiledLayers == tiledLayers)
		return;

	m_tiledLayers = tiledLayers;
	emit tiledLayersChanged(m_tiledLayers);
}


/**
 * @brief GameScenePrivate::setGame
 * @param game
 */

void GameScene::setGame(CosGame *game)
{
	if (m_game == game)
		return;

	m_game = game;
	emit gameChanged(m_game);
}





/**
 * @brief GameScenePrivate::loadMap
 * @param source
 */

bool GameScene::loadMap(const QString &source)
{
	Tiled::MapReader reader;

	if (m_map)
		delete m_map;

	m_map = nullptr;

	if(!QFile::exists(source))
		qWarning() << source << " does not exist.";

	m_map = reader.readMap(source);
	if (!m_map) {
		qCritical("Failed to read map: %s", qPrintable(source));
		return false;
	}

	emit mapChanged(m_map);

	return true;
}


/**
 * @brief GameScenePrivate::loadLayers
 */

void GameScene::loadLayers()
{
	foreach(Tiled::Layer *layer, m_map->layers())
	{
		qDebug() << "Load layer" << layer->name();

		if (layer->isTileLayer()) {
			TiledPaintedLayer *paintedLayer = new TiledPaintedLayer(this->parentItem());
			paintedLayer->setMap(m_map);
			paintedLayer->setLayer(layer->asTileLayer());
			paintedLayer->setZ(-1);
			m_tiledLayers.append(paintedLayer);
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

	emit layersLoaded();
}


/**
 * @brief GameScenePrivate::loadGroundLayer
 * @param layer
 */

void GameScene::loadGroundLayer(Tiled::Layer *layer)
{
	qDebug() << "Load ground layer" << layer;

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		GameObject *item = new GameObject(this->parentItem());
		item->setX(object->x());
		item->setY(object->y());
		item->setZ(0);
		item->setWidth(object->width());
		item->setHeight(object->height());
		item->setRotation(object->rotation());
		item->setVisible(object->isVisible());
		item->setId(object->id());
		item->setProperties(object->properties());
		item->setDensity(1);
		item->setRestitution(0);
		item->setFriction(1);
		item->setCategories(Box2DFixture::Category1);
		item->setCollidesWith(Box2DFixture::Category1|Box2DFixture::Category2|Box2DFixture::Category5);
		item->createFixture(object);
	}

}




/**
 * @brief GameScenePrivate::loadEnemyLayer
 */

void GameScene::loadEnemyLayer(Tiled::Layer *layer)
{
	qDebug() << "Load enemy layer" << layer;

	if (!m_game) {
		qWarning() << "Missing game";
		return;
	}

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		if (!object->isPolyShape())
			continue;

		QPolygonF polygon = object->polygon();
		QRectF rect = polygon.boundingRect();
		rect.moveTo(object->x(), object->y());


		int block = object->property("block").toInt();

		if (block > 0) {
			GameEnemyData *enemy = new GameEnemyData(m_game);

			enemy->setBoundRect(rect.toRect());

			GameBlock *b = m_game->getBlock(block);
			enemy->setBlock(b);
			b->addEnemy(enemy);

			m_game->addEnemy(enemy);
		}

	}

}


/**
 * @brief GameScenePrivate::loadPlayerLayer
 * @param layer
 */

void GameScene::loadPlayerLayer(Tiled::Layer *layer)
{
	qDebug() << "Load player layer" << layer;

	if (!m_game) {
		qWarning() << "Missing game";
		return;
	}

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		int x = object->x();
		int y = object->y();
		int block = object->property("block").toInt();

		if (block > 0) {
			GameBlock *b = m_game->getBlock(block);
			Box2DFixture *fixture = b->addPlayerPosition(QPoint(x,y), this->parentItem());

			if (fixture) {
				connect(fixture, &Box2DFixture::beginContact, m_game, &CosGame::setLastPosition);
			}
		} else
			qWarning() << "Invalid block" << block;
	}

}



/**
 * @brief GameScenePrivate::loadLadderLayer
 * @param layer
 */

void GameScene::loadLadderLayer(Tiled::Layer *layer)
{
	qDebug() << "Load ladders layer" << layer;

	if (!m_game) {
		qWarning() << "Missing game";
		return;
	}

	if (!layer)
		return;

	Tiled::ObjectGroup *og = layer->asObjectGroup();

	QList<Tiled::MapObject*> objects = og->objects();

	foreach (Tiled::MapObject *object, objects) {
		if (object->shape() != Tiled::MapObject::Rectangle)
			continue;

		QRect r;
		r.setX(object->x());
		r.setY(object->y());
		r.setWidth(object->width());
		r.setHeight(object->height());

		GameLadder *ladder = new GameLadder(m_game);
		ladder->setBoundRect(r);
		ladder->setActive(true);

		int blockTop = object->property("blockTop").toInt();
		int blockBottom = object->property("blockBottom").toInt();

		if (blockTop > 0 && blockBottom > 0) {
			GameBlock *bTop = m_game->getBlock(blockTop);
			GameBlock *bBottom = m_game->getBlock(blockBottom);
			ladder->setBlockTop(bTop);
			ladder->setBlockBottom(bBottom);
			bTop->addLadder(ladder);
			bBottom->addLadder(ladder);
			ladder->setActive(false);
		}

		m_game->addLadder(ladder);
	}
}


