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
#include "application.h"

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
	manager->setAnimateTiles(true);
	connect(manager, &Tiled::TilesetManager::repaintTileset, this, &TiledScene::repaintTilesets);

}


/**
 * @brief TiledScene::~TiledScene
 */

TiledScene::~TiledScene()
{
	m_world->setRunning(false);
	unsetMap();

	for (const auto &o : std::as_const(m_tiledObjects)) {
		if (o && o->scene() == this)
			o->setScene(nullptr);
	}


	qDeleteAll(mTileLayerItems);
	mTileLayerItems.clear();
	m_dynamicZList.clear();

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
	//m_tiledObjects.append(QPointer(object));
	m_tiledObjectsToAppend.append(QPointer(object));
}


/**
 * @brief TiledScene::removeFromObjects
 * @param object
 */

void TiledScene::removeFromObjects(TiledObject *object)
{
	//m_tiledObjects.removeAll(QPointer(object));
	m_tiledObjectsToRemove.append(QPointer(object));
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

	QPointF pos(x, y);

	if (Tiled::MapRenderer *renderer = mRenderer.get()) {
		pos = renderer->screenToPixelCoords(x, y);
	}

	for (const DynamicZ &p : std::as_const(m_dynamicZList)) {
		if (!p.isOver(pos))
			continue;

		z = std::max(z, p.z);
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
 * @brief TiledScene::startMusic
 */

void TiledScene::startMusic()
{
	if (!m_ambientSound.isEmpty())
		Application::instance()->client()->sound()->playSound(m_ambientSound, Sound::Music2Channel);

	if (!m_backgroundMusic.isEmpty())
		Application::instance()->client()->sound()->playSound(m_backgroundMusic, Sound::MusicChannel);
}



/**
 * @brief TiledScene::stopMusic
 */

void TiledScene::stopMusic()
{
	if (!m_ambientSound.isEmpty())
		Application::instance()->client()->sound()->stopSound(m_ambientSound, Sound::Music2Channel);

	if (!m_backgroundMusic.isEmpty())
		Application::instance()->client()->sound()->stopSound(m_backgroundMusic, Sound::MusicChannel);
}


/**
 * @brief TiledScene::isGroundContainsPoint
 * @param ground
 * @return
 */

bool TiledScene::isGroundContainsPoint(const QPointF &point) const
{
	for (TiledObjectBasePolygon *o : std::as_const(m_groundObjects)) {
		if (o->fixture()->containsPoint(point))
			return true;
	}

	return false;
}








/**
 * @brief TiledScene::onWorldStepped
 */

void TiledScene::onWorldStepped()
{
	qreal factor = 1.f;

	if (m_worldStepTimer.isValid()) {
		const qint64 &msec = m_worldStepTimer.restart();

		qreal f = msec/(1000.f/60.f);

		if (f > 1.1f)
			factor = f;
	} else {
		m_worldStepTimer.start();
	}

	for (TiledObject *obj : std::as_const(m_tiledObjects)) {
		if (!obj)
			continue;

		obj->worldStep(factor);
	}

	for (const QPointer<TiledObject> &ptr : m_tiledObjectsToAppend) {
		if (ptr)
			m_tiledObjects.append(ptr);
	}
	m_tiledObjectsToAppend.clear();

	for (const QPointer<TiledObject> &ptr : m_tiledObjectsToRemove) {
		if (ptr)
			m_tiledObjects.removeAll(ptr);
	}
	m_tiledObjectsToRemove.clear();

	reorderObjectsZ();

}


/**
 * @brief TiledScene::reorderObjectsZ
 */

void TiledScene::reorderObjectsZ()
{
	QHash<qreal, QMultiMap<qreal, TiledObject*>> map;

	for (TiledObject *obj : std::as_const(m_tiledObjects)) {
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
		qreal subsubZ = 0.0001;

		const qreal subZ = it.key();
		const auto &subMap = it.value();

		for (auto it2 = subMap.constBegin(); it2 != subMap.constEnd(); ++it2) {
			it2.value()->setZ(subZ+subsubZ);
			subsubZ += 0.0001;
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
 * @brief TiledScene::addTileLayer
 * @param layer
 * @param renderer
 */

TiledQuick::TileLayerItem *TiledScene::addTileLayer(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(layer);
	Q_ASSERT(renderer);

	TiledQuick::TileLayerItem *layerItem = new TiledQuick::TileLayerItem(layer, renderer, this);
	mTileLayerItems.append(layerItem);

	if (layer->hasProperty(QStringLiteral("z"))) {
		layerItem->setZ(layer->property(QStringLiteral("z")).toInt());
	} else {
		layerItem->setZ(0);
	}

	return layerItem;
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

	for (Tiled::Layer *layer : std::as_const(mMap->layers())) {
		m_game->loadSceneLayer(this, layer, mRenderer.get());
	}

	setTileLayersZ();


	const QRect rect = mRenderer->mapBoundingRect();
	setWidth(rect.width());
	setHeight(rect.height());
}


/**
 * @brief TiledScene::appendDynamicZ
 * @param name
 * @param src
 */

void TiledScene::appendDynamicZ(const QString &name, const QRectF &area)
{
	auto it = std::find_if(m_dynamicZList.begin(), m_dynamicZList.end(), [&name](const DynamicZ &d) {
		return (d.name == name);
	});

	if (it == m_dynamicZList.end()) {
		m_dynamicZList.emplace_back(name, QVector<QRectF>{area}, 1);
	} else {
		it->areas.append(area);
	}

}




/**
 * @brief TiledScene::setTileLayersZ
 */

void TiledScene::setTileLayersZ()
{
	int i=2;

	for (TiledQuick::TileLayerItem *layerItem : mTileLayerItems) {
		if (Tiled::TileLayer *layer = layerItem->tileLayer()) {
			const QString &name = layer->name();

			auto it = std::find_if(m_dynamicZList.begin(), m_dynamicZList.end(), [&name](const DynamicZ &d) {
				return (d.name == name);
			});

			if (it != m_dynamicZList.end()) {
				layerItem->setZ(i);
				it->z = i;
				++i;
			} else {
				LOG_CTRACE("scene") << "Skip from dynamicZ" << name << layerItem->z();
			}

		}
	}
}


/**
 * @brief TiledScene::backgroundMusic
 * @return
 */

QString TiledScene::backgroundMusic() const
{
	return m_backgroundMusic;
}

void TiledScene::setBackgroundMusic(const QString &newBackgroundMusic)
{
	if (m_backgroundMusic == newBackgroundMusic)
		return;
	m_backgroundMusic = newBackgroundMusic;
	emit backgroundMusicChanged();
}

QString TiledScene::ambientSound() const
{
	return m_ambientSound;
}

void TiledScene::setAmbientSound(const QString &newAmbientSound)
{
	if (m_ambientSound == newAmbientSound)
		return;
	m_ambientSound = newAmbientSound;
	emit ambientSoundChanged();
}


/**
 * @brief TiledScene::DynamicZ::getMaxBottomRight
 * @return
 */

QPointF TiledScene::DynamicZ::getMaxBottomRight() const
{
	QPointF pos;

	for (auto it = areas.constBegin(); it != areas.constEnd(); ++it) {
		if (it == areas.constBegin())
			pos = it->bottomRight();
		else {
			if (it->bottom() > pos.y())
				pos.setY(it->bottom());
			if (it->right() > pos.x())
				pos.setX(it->right());
		}
	}

	return pos;
}


/**
 * @brief TiledScene::DynamicZ::getMinTopLeft
 * @return
 */

QPointF TiledScene::DynamicZ::getMinTopLeft() const
{
	QPointF pos;

	for (auto it = areas.constBegin(); it != areas.constEnd(); ++it) {
		if (it == areas.constBegin())
			pos = it->topLeft();
		else {
			if (it->top() < pos.y())
				pos.setY(it->top());
			if (it->left() < pos.x())
				pos.setX(it->left());
		}
	}

	return pos;
}


/**
 * @brief TiledScene::DynamicZ::isOver
 * @param x
 * @return
 */

bool TiledScene::DynamicZ::isOver(const qreal &x, const qreal &y) const
{
	const QPointF &topLeft = getMinTopLeft();

	for (const QRectF &a : areas) {
		if (y < topLeft.y() || x < topLeft.x())
			return false;

		if (y <= a.bottom() && x <= a.right())
			return false;
	}

	return true;
}
