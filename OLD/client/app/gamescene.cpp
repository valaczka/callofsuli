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
#include "maplistobject.h"

GameScene::GameScene(QQuickItem *parent)
	: QQuickItem(parent)
	, m_tiledLayers()
	, m_game(nullptr)
	, m_teleports()
{

}

/**
 * @brief GameScene::~GameScene
 */

GameScene::~GameScene()
{
	qDeleteAll(m_tiledLayers.begin(), m_tiledLayers.end());
	m_tiledLayers.clear();
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
 * @brief GameScene::loadScene
 * @param tmxFileName
 */

void GameScene::loadGroundObjects(GameTerrain *terrainData)
{
	foreach (QRectF rect, terrainData->groundObjects()) {
		GameObject *item = new GameObject(this->parentItem());
		item->setX(rect.x());
		item->setY(rect.y());
		item->setZ(0);
		item->setWidth(rect.width());
		item->setHeight(rect.height());
		item->setVisible(true);
		item->setDensity(1);
		item->setRestitution(0);
		item->setFriction(1);
		item->setCategories(Box2DFixture::Category1);
		item->setCollidesWith(Box2DFixture::Category1|Box2DFixture::Category2|Box2DFixture::Category5);
		item->createRectangularFixture();
	}
}



/**
 * @brief GameScene::loadScene
 * @return
 */


void GameScene::loadPlayerPositions(GameTerrain *terrainData)
{
	QMapIterator<int, GameBlock *> it(terrainData->blocks());

	while (it.hasNext()) {
		it.next();

		GameBlock *block = it.value();
		int blockNum = it.key();

		foreach (QPointF point, block->playerPositions()) {
			GameObject *item = new GameObject(this->parentItem());

			qreal w = 20;
			qreal h = 80;
			qreal x = point.x()-w/2;
			qreal y = point.y()-h/2;

			item->setX(x);
			item->setY(y);
			item->setZ(0);
			item->setWidth(w);
			item->setHeight(h);
			item->setVisible(true);
			item->setDensity(1);
			item->setRestitution(0);
			item->setFriction(1);
			item->setSensor(true);
			item->setCategories(Box2DFixture::Category6);
			item->setCollidesWith(Box2DFixture::Category3);
			item->setExtra({{"block", blockNum}});

			Box2DFixture *fixture = item->createRectangularFixture();

			if (fixture) {
				connect(fixture, &Box2DFixture::beginContact, m_game, &CosGame::setLastPosition);
			}
		}
	}


	foreach (QPointF point, terrainData->playerPositions()) {
		GameObject *item = new GameObject(this->parentItem());

		qreal w = 20;
		qreal h = 80;
		qreal x = point.x()-w/2;
		qreal y = point.y()-h/2;

		item->setX(x);
		item->setY(y);
		item->setZ(0);
		item->setWidth(w);
		item->setHeight(h);
		item->setVisible(true);
		item->setDensity(1);
		item->setRestitution(0);
		item->setFriction(1);
		item->setSensor(true);
		item->setCategories(Box2DFixture::Category6);
		item->setCollidesWith(Box2DFixture::Category3);

		Box2DFixture *fixture = item->createRectangularFixture();

		if (fixture) {
			connect(fixture, &Box2DFixture::beginContact, m_game, &CosGame::setLastPosition);
		}
	}
}


/**
 * @brief GameScene::loadFires
 * @param terrainData
 */

void GameScene::loadFires(GameTerrain *terrainData)
{
	foreach (QPointF point, terrainData->fires()) {
		QQuickItem *item = nullptr;

		QMetaObject::invokeMethod(m_game->gameScene(), "createFire", Qt::DirectConnection,
								  Q_RETURN_ARG(QQuickItem*, item)
								  );

		if (!item) {
			qWarning() << "Can't create fire at" << point;
			continue;
		}

		item->setX(point.x()-(item->width()/2));
		item->setY(point.y()-item->height()+10);			// +10: az animáció korrekciója miatt lejjebb kell tenni
	}
}




/**
 * @brief GameScene::loadItems
 * @param terrainData
 */

void GameScene::loadItems(GameTerrain *terrainData)
{
	foreach (GameTerrainItem t, terrainData->items()) {
		QVariantMap m = t.second.data;
		m["bottomPoint"] = t.first;

		QMetaObject::invokeMethod(m_game->gameScene(), "createPickable", Qt::QueuedConnection,
								  Q_ARG(int, t.second.type),
								  Q_ARG(QVariant, m)
								  );
	}
}


/**
 * @brief GameScene::loadFences
 * @param terrainData
 */

void GameScene::loadFences(GameTerrain *terrainData)
{
	foreach (QPointF point, terrainData->fences()) {
		QQuickItem *item = nullptr;

		QMetaObject::invokeMethod(m_game->gameScene(), "createFence", Qt::DirectConnection,
								  Q_RETURN_ARG(QQuickItem*, item)
								  );

		if (!item) {
			qWarning() << "Can't create fence at" << point;
			continue;
		}

		item->setX(point.x()-(item->width()/2));
		item->setY(point.y()-item->height());
	}
}


/**
 * @brief GameScene::loadTeleports
 * @param terrainData
 */

void GameScene::loadTeleports(GameTerrain *terrainData)
{
	m_teleports.clear();

	foreach (QPointF point, terrainData->teleports()) {
		QQuickItem *item = nullptr;

		QMetaObject::invokeMethod(m_game->gameScene(), "createTeleport", Qt::DirectConnection,
								  Q_RETURN_ARG(QQuickItem*, item)
								  );

		if (!item) {
			qWarning() << "Can't create teleport at" << point;
			continue;
		}

		item->setX(point.x()-(item->width()/2));
		item->setY(point.y()-item->height());

		m_teleports.append(item);
	}
}


/**
 * @brief GameScene::teleports
 * @return
 */

const QList<QQuickItem *> &GameScene::teleports() const
{
	return m_teleports;
}



/**
 * @brief GameScene::loadScene
 * @return
 */

bool GameScene::loadScene()
{
	if (!m_game)
		return false;

	if (m_game->terrainData()) {
		qWarning() << this << "Scene already loaded";
		return false;
	}

	emit sceneLoadStarted();

	m_game->addTerrainData(&m_tiledLayers, parentItem());

	if (!m_game->loadTerrainData()) {
		emit sceneLoadFailed();
		return false;
	}

	GameTerrain *terrainData = m_game->terrainData();
	auto map = terrainData->map();

	setImplicitWidth(map->width() * map->tileWidth());
	setImplicitHeight(map->height() * map->tileHeight());


	loadGroundObjects(terrainData);
	loadPlayerPositions(terrainData);
	loadFires(terrainData);
	loadFences(terrainData);
	loadTeleports(terrainData);
	loadItems(terrainData);

	emit sceneLoaded();

	return true;
}


