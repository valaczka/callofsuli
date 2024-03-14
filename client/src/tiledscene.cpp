/*
 * ---- Call of Suli ----
 *
 * tiledscene.cpp
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledScene
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

#include "tiledscene.h"
#include "Logger.h"
#include "maprenderer.h"
#include "tiledobject.h"
#include "tilelayeritem.h"
#include "tiledgame.h"
#include "tilesetmanager.h"
#include "isometricobjectiface.h"

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>
#include <libtiled/grouplayer.h>

TiledScene::TiledScene(QQuickItem *parent)
	: TiledQuick::MapItem(parent)
	, m_mapLoader(new TiledQuick::MapLoader)
	, m_world(new Box2DWorld)
{
	LOG_CTRACE("scene") << "Scene created" << this;

	m_world->setGravity(QPointF{0,0});
	m_world->setTimeStep(1./60.);
	connect(m_world.get(), &Box2DWorld::stepped, this, &TiledScene::onWorldStepped);
	m_world->componentComplete();

	setImplicitHeight(100);
	setImplicitWidth(100);

	//setAcceptHoverEvents(true);
	//setAcceptedMouseButtons(Qt::LeftButton);
	//setAcceptTouchEvents(true);

	Tiled::TilesetManager *manager = Tiled::TilesetManager::instance();
	/// ENABLE: manager->setAnimateTiles(true);
	connect(manager, &Tiled::TilesetManager::repaintTileset, this, &TiledScene::repaintTilesets);

}


/**
 * @brief TiledScene::~TiledScene
 */

TiledScene::~TiledScene()
{
	unsetMap();
	m_world->setRunning(false);

	LOG_CTRACE("scene") << "Scene destroyed" << this;
}



/**
 * @brief TiledScene::getDynamicZ
 * @param point
 * @param defaultValue
 * @return
 */

int TiledScene::getDynamicZ(const QPointF &point, const int &defaultValue) const
{
	return getDynamicZ(point.x(), point.y(), defaultValue);
}



/**
 * @brief TiledScene::getDynamicZ
 * @param item
 * @param defaultValue
 * @return
 */

int TiledScene::getDynamicZ(QQuickItem *item, const int &defaultValue) const
{
	if (!item)
		return defaultValue;

	return getDynamicZ(item->position(), defaultValue);
}


/**
 * @brief TiledScene::load
 * @param url
 * @return
 */

bool TiledScene::load(const QUrl &url)
{
	LOG_CTRACE("scene") << "Load scene from:" << qPrintable(url.toDisplayString());

	m_mapLoader->setSource(url);

	if (m_mapLoader->status() == TiledQuick::MapLoader::Ready && m_mapLoader->map()) {
		setMap(m_mapLoader->map());
		return true;
	}

	return false;
}



/**
 * @brief TiledScene::appendToObjects
 * @param object
 */

void TiledScene::appendToObjects(TiledObject *object)
{
	m_tiledObjects.append(object);
}


/**
 * @brief TiledScene::removeFromObjects
 * @param object
 */

void TiledScene::removeFromObjects(TiledObject *object)
{
	m_tiledObjects.removeAll(object);
}


/**
 * @brief TiledScene::getDynamicZ
 * @param x
 * @param y
 * @param defaultValue
 * @return
 */

int TiledScene::getDynamicZ(const qreal &x, const qreal &y, const int &defaultValue) const
{
	int z = defaultValue;

	for (const auto &[dZ, p] : m_dynamicZList) {
		if (p.polygon.isEmpty())
			continue;

		const QPointF &center = p.polygon.boundingRect().center();

		if (p.vertical && y <= center.y())
			continue;

		if (p.horizontal && x <= center.x())
			continue;

		z = std::max(z, dZ+1);
	}

	return z;
}




/**
 * @brief TiledScene::running
 * @return
 */

bool TiledScene::running() const
{
	return m_world->isRunning();
}

void TiledScene::setRunning(bool newRunning)
{
	if (m_world->isRunning() == newRunning)
		return;
	m_world->setRunning(newRunning);
	emit runningChanged();
}








/**
 * @brief TiledScene::onWorldStepped
 */

void TiledScene::onWorldStepped()
{
	for (TiledObject *obj : m_tiledObjects) {
		if (!obj)
			continue;

		obj->worldStep();
	}

	reorderObjectsZ();

}


/**
 * @brief TiledScene::reorderObjectsZ
 */

void TiledScene::reorderObjectsZ()
{
	QHash<qreal, QMultiMap<qreal, TiledObject*>> map;

	for (TiledObject *obj : m_tiledObjects) {
		if (!obj || !obj->body())
			continue;

		IsometricObjectIface *iso = dynamic_cast<IsometricObjectIface*>(obj);

		qreal z = 0;
		const qreal y = obj->body()->bodyPosition().y();

		if (iso)
			z = getDynamicZ(obj->body()->bodyPosition(), iso->defaultZ()) + iso->subZ();
		else
			z = obj->z();

		map[z].insert(y, obj);
	}

	for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
		qreal subsubZ = 0.00001;

		const qreal subZ = it.key();
		const auto &subMap = it.value();

		for (auto it2 = subMap.constBegin(); it2 != subMap.constEnd(); ++it2) {
			it2.value()->setZ(subZ+subsubZ);
			subsubZ += 0.00001;
		}
	}
}



/**
 * @brief TiledScene::repaintTilesets
 */

void TiledScene::repaintTilesets(Tiled::Tileset *tileset)
{
	for (auto *mapItem : std::as_const(mTileLayerItems)) {
		if (mapItem->tileLayer()->usedTilesets().contains(tileset->sharedPointer())) {
			mapItem->update();
		}
	}
}



/**
 * @brief TiledScene::sceneId
 * @return
 */

int TiledScene::sceneId() const
{
	return m_sceneId;
}

void TiledScene::setSceneId(int newSceneId)
{
	if (m_sceneId == newSceneId)
		return;
	m_sceneId = newSceneId;
	emit sceneIdChanged();
}



/**
 * @brief TiledScene::game
 * @return
 */

TiledGame *TiledScene::game() const
{
	return m_game;
}

void TiledScene::setGame(TiledGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();
}







/**
 * @brief TiledScene::mapLoader
 * @return
 */

TiledQuick::MapLoader*TiledScene::mapLoader() const
{
	return m_mapLoader.get();
}



/**
 * @brief TiledScene::world
 * @return
 */

Box2DWorld *TiledScene::world() const
{
	return m_world.get();
}




/**
 * @brief TiledScene::refresh
 */

void TiledScene::refresh()
{
	if (!isComponentComplete())
		return;

	qDeleteAll(mTileLayerItems);
	mTileLayerItems.clear();
	m_dynamicZList.clear();

	mRenderer = nullptr;

	if (!mMap)
		return;

	mRenderer = Tiled::MapRenderer::create(mMap);

	for (Tiled::Layer *layer : mMap->layers()) {
		Tiled::TileLayer *tl = m_game->loadSceneLayer(this, layer, mRenderer.get());

		if (!tl)
			continue;

		TiledQuick::TileLayerItem *layerItem = new TiledQuick::TileLayerItem(tl, mRenderer.get(), this);
		mTileLayerItems.append(layerItem);

		if (tl->hasProperty(QStringLiteral("z"))) {
			layerItem->setZ(tl->property(QStringLiteral("z")).toInt());
		} else {
			layerItem->setZ(0);
		}

		/// TODO: add controlled layer
	}


	const QRect rect = mRenderer->mapBoundingRect();
	setWidth(rect.width());
	setHeight(rect.height());
}
