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
#include "box2dfixture.h"
#include "isometricenemy.h"
#include "maprenderer.h"
#include "tiledobject.h"
#include "tilelayeritem.h"
#include "tiledgame.h"

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>

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

	connect(m_mapLoader.get(), &TiledQuick::MapLoader::statusChanged, this, &TiledScene::onSceneStatusChanged);
}


/**
 * @brief TiledScene::~TiledScene
 */

TiledScene::~TiledScene()
{
	m_mapLoader->setSource(QUrl{});

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
	/*if (!url.isLocalFile()) {
		LOG_CERROR("scene") << "Invalid URL:" << url;
		return false;
	}*/

	LOG_CDEBUG("scene") << "Load TMX:" << qPrintable(url.toLocalFile());

	m_mapLoader->setSource(url);

	return true;
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
 * @brief TiledScene::onSceneStatusChanged
 * @param status
 */
void TiledScene::onSceneStatusChanged(const TiledQuick::MapLoader::Status &status)
{
	LOG_CDEBUG("scene") << "Status changed" << status;

	if (m_mapLoader->map()) {
		setMap(m_mapLoader->map());
	}
}


/**
 * @brief TiledScene::onWorldStepped
 */

void TiledScene::onWorldStepped()
{
	for (TiledObject *obj : m_tiledObjects) {
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
 * @brief TiledScene::active
 * @return
 */

bool TiledScene::active() const
{
	return m_active;
}

void TiledScene::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}




QVariantList TiledScene::testPoints() const
{
	return m_testPoints;
}

void TiledScene::setTestPoints(const QVariantList &newTestPoints)
{
	if (m_testPoints == newTestPoints)
		return;
	m_testPoints = newTestPoints;
	emit testPointsChanged();
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
 * @brief TiledScene::loadObjectLayer
 * @param group
 */

void TiledScene::loadObjectLayer(Tiled::ObjectGroup *group)
{
	Q_ASSERT(group);

	for (Tiled::MapObject *object : group->objects()) {
		if (object->className() == "enemy") {
			QPolygonF p = TiledObjectBase::toPolygon(object, mRenderer.get());
			LOG_CINFO("scene") << "ENEMY" << object->name();

			IsometricEnemy *character = IsometricEnemy::createEnemy(this);

			Q_ASSERT(character);

			character->setScene(this);
			character->setGame(m_game);
			character->loadPathMotor(p);


			m_tiledObjects.append(character);

			/*QVariantList list;
			for (const auto &point : p)
				list.append(point);
			setTestPoints(list);*/

			continue;
		} else if (object->className() == "player") {

			LOG_CINFO("scene") << "PLAYER" << object->position();

			if (m_game)
				m_game->loadPlayer(this, mRenderer->pixelToScreenCoords(object->position()));

			continue;
		} else if (object->className() == "gate") {

			LOG_CINFO("scene") << "GATE" << object->position();

			loadGate(object);

			continue;
		}

		loadGround(object);
	}
}



/**
 * @brief TiledScene::loadGround
 * @param object
 */

void TiledScene::loadGround(Tiled::MapObject *object)
{
	TiledObjectBasePolygon *mapObject = nullptr;
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, mRenderer.get());

	if (!mapObject)
		return;

	mapObject->setScene(this);
	mapObject->fixture()->setDensity(1);
	mapObject->fixture()->setFriction(1);
	mapObject->fixture()->setRestitution(0);
	mapObject->fixture()->setCategories(Box2DFixture::Category1);

	if (object->hasProperty(QStringLiteral("dynamicZ"))) {
		if (const QPolygonF &p = mapObject->screenPolygon(); !p.isEmpty()) {

			DynamicZ dz;
			dz.polygon = p.translated(mapObject->position());

			LOG_CINFO("scene") << "ADD POLYGON" << dz.polygon;

			if (object->hasProperty(QStringLiteral("dynamicVertical")))
				dz.vertical = object->property(QStringLiteral("dynamicVertical")).toBool();

			if (object->hasProperty(QStringLiteral("dynamicHorizontal")))
				dz.horizontal = object->property(QStringLiteral("dynamicHorizontal")).toBool();

			m_dynamicZList[object->property(QStringLiteral("dynamicZ")).toInt()] = dz;
		}
	}

	if (object->hasProperty(QStringLiteral("z"))) {
		mapObject->setZ(object->property(QStringLiteral("z")).toInt());
	} else {
		mapObject->setZ(0);
	}
}


/**
 * @brief TiledScene::loadGate
 * @param object
 */

void TiledScene::loadGate(Tiled::MapObject *object)
{
	TiledObjectBasePolygon *mapObject = nullptr;
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, mRenderer.get(), this);

	if (!mapObject)
		return;

	mapObject->setScene(this);
	mapObject->fixture()->setSensor(true);
	mapObject->fixture()->setCategories(Box2DFixture::Category4);

	bool r = m_game->addGate(object->name(), this, mapObject);
	LOG_CINFO("scene") << "Add gate" << object->name() << this << mapObject << r;
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
 * @brief TiledScene::tiledObjects
 * @return
 */

QList<TiledObject *> TiledScene::tiledObjects() const
{
	return m_tiledObjects;
}

void TiledScene::setTiledObjects(const QList<TiledObject *> &newTiledObjects)
{
	if (m_tiledObjects == newTiledObjects)
		return;
	m_tiledObjects = newTiledObjects;
	emit tiledObjectsChanged();
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

	/*Tiled::TilesetManager *manager = Tiled::TilesetManager::instance();
	manager->setAnimateTiles(true);
	connect(manager, &Tiled::TilesetManager::repaintTileset, layerItem, [this, layerItem](Tiled::Tileset *tileset) {
		layerItem->update();
	});*/

	mRenderer = Tiled::MapRenderer::create(mMap);

	for (Tiled::Layer *layer : mMap->layers()) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			TiledQuick::TileLayerItem *layerItem = new TiledQuick::TileLayerItem(tl, mRenderer.get(), this);
			mTileLayerItems.append(layerItem);
			if (tl->hasProperty(QStringLiteral("z"))) {
				layerItem->setZ(tl->property(QStringLiteral("z")).toInt());
			} else {
				layerItem->setZ(0);
			}

		} else if (Tiled::ObjectGroup *group = layer->asObjectGroup()) {
			loadObjectLayer(group);
		}
	}


	const QRect rect = mRenderer->mapBoundingRect();
	setWidth(rect.width());
	setHeight(rect.height());
}
