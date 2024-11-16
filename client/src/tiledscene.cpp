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
#include "tiledobject.h"
#include "tiledvisualitem.h"
#include "tilelayeritem.h"
#include "tiledgame.h"
#include "tilesetmanager.h"
#include "isometricobjectiface.h"
#include "application.h"
#include "libtcod/path.hpp"

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>
#include <libtiled/grouplayer.h>
#include <libtiled/mapreader.h>
#include <libtiled/imagelayer.h>
#include <libtiled/maprenderer.h>

TiledScene::TiledScene(QQuickItem *parent)
	: TiledQuick::MapItem(parent)
	, m_world(new Box2DWorld)
{
	LOG_CTRACE("scene") << "Scene created" << this;

	m_world->setGravity(QPointF{0,0});
	m_world->setTimeStep(1./60.);
	connect(m_world.get(), &Box2DWorld::stepped, this, &TiledScene::onWorldStepped, Qt::DirectConnection);
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


	qDeleteAll(m_visualItems);
	m_visualItems.clear();
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

	Tiled::MapReader mapReader;

	m_map = mapReader.readMap(Tiled::urlToLocalFileOrQrc(url));

	if (m_map) {
		setMap(m_map.get());
		return true;
	}

	return false;
}


/**
 * @brief TiledScene::findShortestPath
 * @param from
 * @param to
 * @return
 */

std::optional<QPolygonF> TiledScene::findShortestPath(const QPointF &from, const QPointF &to) const
{
	return findShortestPath(from.x(), from.y(), to.x(), to.y());
}



/**
 * @brief TiledScene::findShortestPath
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @return
 */

std::optional<QPolygonF> TiledScene::findShortestPath(const qreal &x1, const qreal &y1, const qreal &x2, const qreal &y2) const
{
	if (qFuzzyCompare(x1, x2) && qFuzzyCompare(y1, y2))
		return std::nullopt;

	if (!m_tcodMap.map)
		return std::nullopt;

	//TCODPath path(m_tcodMap.map.get());
	TCODDijkstra path(m_tcodMap.map.get());

	const int chX1 = std::floor(x1/m_tcodMap.chunkWidth);
	const int chX2 = std::floor(x2/m_tcodMap.chunkWidth);
	const int chY1 = std::floor(y1/m_tcodMap.chunkHeight);
	const int chY2 = std::floor(y2/m_tcodMap.chunkHeight);

	if (!m_tcodMap.map->isWalkable(chX1, chY1))
		return std::nullopt;

	// Ha közel vagyunk (same chunk), nem is számolunk

	if (chX1 == chX2 && chY1 == chY2)
		return QPolygonF() << QPointF(x2, y2);

	path.compute(chX1, chY1);

	if (!path.setPath(chX2, chY2))
		return std::nullopt;

	QPolygonF polygon;


	for (int i=0; i<path.size()-1; ++i) {
		int x, y;
		path.get(i, &x, &y);

		// Ha az első chunk ugyanaz, mint amiben vagyunk (elvileg mindig)
		if (i == 0 && x == chX1 && y == chY1)
			polygon << QPointF(x1, y1);
		else
			polygon << QPointF(
						   (x+0.5) * m_tcodMap.chunkWidth,
						   (y+0.5) * m_tcodMap.chunkHeight
						   );
	}

	// Az utolsó chunk közepe helyett a végleges pozíciót adjuk meg

	polygon << QPointF(x2, y2);

	/*
	struct TmpPolygon {
		int x = 0;
		int y = 0;
		QPointF point;
		bool add = false;
	};

	QList<TmpPolygon> tmpPolygon;

	static const auto fnCheck = [](const QRectF &area, const QLineF &line) -> bool {
		return line.intersects(QLineF{area.topLeft(), area.topRight()}) == QLineF::BoundedIntersection ||
				line.intersects(QLineF{area.topRight(), area.bottomRight()}) == QLineF::BoundedIntersection ||
				line.intersects(QLineF{area.bottomLeft(), area.bottomRight()}) == QLineF::BoundedIntersection ||
				line.intersects(QLineF{area.topLeft(), area.bottomLeft()}) == QLineF::BoundedIntersection
				;
	};

	tmpPolygon.emplace_back(-1, -1, QPointF{x1, y1}, true);
	int lastIndex = 1;

	for (int i=0; i<=path.size(); ++i) {
		QPointF currentPoint;
		int x, y;

		if (i < path.size()) {
			path.get(i, &x, &y);

			currentPoint.setX((x+0.5) * m_tcodMap.chunkWidth);
			currentPoint.setY((y+0.5) * m_tcodMap.chunkHeight);
		} else {
			x = -1;
			y = -1;
			currentPoint.setX(x2);
			currentPoint.setY(y2);
		}

		if (lastIndex < tmpPolygon.size()) {
			QList<TmpPolygon>::iterator lastPtr = tmpPolygon.begin() + lastIndex;

			const QLineF line(lastPtr->point, currentPoint);

			int idx = lastIndex;

			for (auto it = lastPtr; it != tmpPolygon.end(); ++it) {
				const QRectF chunk(QPointF(m_tcodMap.chunkWidth * it->x, m_tcodMap.chunkHeight * it->y),
								   QSizeF(m_tcodMap.chunkWidth, m_tcodMap.chunkHeight));

				if (!fnCheck(chunk, line)) {
					tmpPolygon.last().add = true;
					lastIndex = idx+1;
					break;
				}

				++idx;
			}
		}

		tmpPolygon.emplace_back(x, y, currentPoint, i == path.size());
	}

	QPolygonF polygon;

	for (const TmpPolygon &p : tmpPolygon) {
		if (p.add)
			polygon.append(p.point);
	}
	*/

	return polygon;
}

/**
 * @brief TiledScene::appendToObjects
 * @param object
 */

void TiledScene::appendToObjects(TiledObject *object)
{
	m_tiledObjectsToAppend.append(object);
}


/**
 * @brief TiledScene::removeFromObjects
 * @param object
 */

void TiledScene::removeFromObjects(TiledObject *object)
{
	m_tiledObjectsToRemove.append(object);
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
		if (o->body()->isActive() && o->fixture()->containsPoint(point))
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

		m_stepFactors.push_back(f);
	} else {
		m_worldStepTimer.start();
	}

	if (m_stepFactors.size() > 6) {
		m_stepFactors.erase(m_stepFactors.begin());
		const qreal f = std::accumulate(m_stepFactors.cbegin(), m_stepFactors.cend(), 0.f) /
						m_stepFactors.size();
		if (f > 1.1f)
			factor = f;
	}

	for (TiledObject *obj : m_tiledObjects) {
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

	if (m_game)
		m_game->onSceneWorldStepped(this);

	emit worldStepped();
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
	for (auto *item : m_visualItems) {
		if (TiledQuick::TileLayerItem *mapItem = qobject_cast<TiledQuick::TileLayerItem *>(item)) {
			if (mapItem->tileLayer()->usedTilesets().contains(tileset->sharedPointer())) {
				mapItem->update();
			}
		}
	}
}


/**
 * @brief TiledScene::reloadTcodMap
 */

void TiledScene::reloadTcodMap()
{
	m_tcodMap.map.reset();

	static const qreal chunkSize = 30.;

	const int wSize = std::ceil(m_viewport.width() / chunkSize);
	const int hSize = std::ceil(m_viewport.height() / chunkSize);

	LOG_CDEBUG("scene") << "Create tcod map" << wSize << hSize;

	m_tcodMap.chunkWidth = m_viewport.width() / wSize;
	m_tcodMap.chunkHeight = m_viewport.height() / hSize;

	m_tcodMap.map.reset(new TCODMap(wSize, hSize));

	Q_ASSERT(m_tcodMap.map.get());

	m_tcodMap.map->clear(true, true);

	for (int i=0; i<wSize; ++i) {
		for (int j=0; j<hSize; ++j) {
			QPolygonF chunk(QRectF(QPointF(m_tcodMap.chunkWidth*i, m_tcodMap.chunkHeight*j),
								   QSizeF(m_tcodMap.chunkWidth, m_tcodMap.chunkHeight)));

			for (TiledObjectBasePolygon *o : std::as_const(m_groundObjects)) {
				if (o->body()->isActive() && chunk.intersects(o->screenPolygon().translated(o->body()->bodyPosition()))) {
					m_tcodMap.map->setProperties(i, j, true, false);
					break;
				}
			}
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
	m_visualItems.append(layerItem);

	if (layer->hasProperty(QStringLiteral("z"))) {
		layerItem->setZ(layer->property(QStringLiteral("z")).toInt());
	} else {
		layerItem->setZ(0);
	}

	return layerItem;
}



/**
 * @brief TiledScene::addVisualTileLayer
 * @param layer
 * @param renderer
 * @return
 */

QQuickItem *TiledScene::addVisualTileLayer(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(layer);
	Q_ASSERT(renderer);

	LOG_CERROR("scene") << "DEPRECATED" << __PRETTY_FUNCTION__;

	TiledQuick::TileLayerItem *layerItem = addTileLayer(layer, renderer);

	if (!layerItem) {
		LOG_CERROR("scene") << "TileLayer createVisual layerItem error";
		return nullptr;
	}

	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledLayerVisual.qml"), this);

	QQuickItem *item = qobject_cast<QQuickItem*>(component.create());

	if (!item) {
		LOG_CERROR("scene") << "TileLayer createVisual error" << component.errorString();
		return nullptr;
	}

	item->setParentItem(this);
	item->setParent(this);
	item->setProperty("baseItem", QVariant::fromValue(layerItem));

	return item;
}


/**
 * @brief TiledScene::addVisualItem
 * @return
 */

TiledVisualItem *TiledScene::addVisualItem()
{
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledImage.qml"), this);

	auto img = qobject_cast<TiledVisualItem*>(component.create());

	if (!img) {
		LOG_CERROR("scene") << "TiledVisualItem create error" << component.errorString();
		return nullptr;
	}

	img->setParentItem(this);
	img->setParent(this);
	img->setScene(this);

	m_visualItems.append(img);

	return img;
}




/**
 * @brief TiledScene::addVisualItem
 * @param layer
 * @return
 */

TiledVisualItem *TiledScene::addVisualItem(Tiled::ImageLayer *layer)
{
	TiledVisualItem *img = addVisualItem();

	if (!img)
		return nullptr;

	if (!layer)
		return img;

	img->setPosition(layer->position() + layer->offset());
	img->setSource(layer->imageSource());
	img->setName(layer->name());

	if (layer->hasProperty(QStringLiteral("z"))) {
		img->setZ(layer->property(QStringLiteral("z")).toInt());
	} else {
		img->setZ(0);
	}

	return img;
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

	qDeleteAll(m_visualItems);
	m_visualItems.clear();
	m_dynamicZList.clear();

	mRenderer = nullptr;

	if (!mMap)
		return;

	mRenderer = Tiled::MapRenderer::create(mMap);

	for (Tiled::Layer *layer : mMap->layers()) {
		m_game->loadSceneLayer(this, layer, mRenderer.get());
	}

	setTileLayersZ();


	const QRect rect = mRenderer->mapBoundingRect();
	setWidth(rect.width());
	setHeight(rect.height());

	if (m_viewport.isEmpty())
		setViewport(rect);

	reloadTcodMap();
}


TiledSceneDefinition::SceneEffect TiledScene::sceneEffect() const
{
	return m_sceneEffect;
}

void TiledScene::setSceneEffect(TiledSceneDefinition::SceneEffect newSceneEffect)
{
	if (m_sceneEffect == newSceneEffect)
		return;
	m_sceneEffect = newSceneEffect;
	emit sceneEffectChanged();
}

QRectF TiledScene::onScreenArea() const
{
	return m_onScreenArea;
}

void TiledScene::setOnScreenArea(const QRectF &newOnScreenArea)
{
	if (m_onScreenArea == newOnScreenArea)
		return;
	m_onScreenArea = newOnScreenArea;
	emit onScreenAreaChanged();
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

	for (QQuickItem *item : m_visualItems) {
		QString name;
		int dynamicZ = -1;

		if (TiledQuick::TileLayerItem *layerItem = qobject_cast<TiledQuick::TileLayerItem *>(item)) {
			if (Tiled::TileLayer *layer = layerItem->tileLayer()) {
				name = layer->name();
				if (layer->hasProperty(QStringLiteral("dynamicZ"))) {
					dynamicZ = layer->property(QStringLiteral("dynamicZ")).toInt();
				}
			}
		} else if (TiledVisualItem *visualItem = qobject_cast<TiledVisualItem *>(item)) {
			name = visualItem->name();
			if (visualItem->dynamicZ() != -1) {
				dynamicZ = visualItem->dynamicZ();
			}
		}

		auto it = std::find_if(m_dynamicZList.begin(), m_dynamicZList.end(), [&name](const DynamicZ &d) {
			return (d.name == name);
		});

		if (it != m_dynamicZList.end()) {
			if (dynamicZ != -1) {
				i = std::max(i, dynamicZ);
			}
			item->setZ(i);
			it->z = i;
			++i;
		} else {
			LOG_CTRACE("scene") << "Skip from dynamicZ" << name << item->z();
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


/**
 * @brief TiledScene::viewport
 * @return
 */

QRectF TiledScene::viewport() const
{
	return m_viewport;
}

void TiledScene::setViewport(const QRectF &newViewport)
{
	if (m_viewport == newViewport)
		return;
	m_viewport = newViewport;
	emit viewportChanged();
}
