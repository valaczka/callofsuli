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
#include "application.h"
#include "isometricobject.h"

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>
#include <libtiled/grouplayer.h>
#include <libtiled/mapreader.h>
#include <libtiled/imagelayer.h>
#include <libtiled/maprenderer.h>

TiledScene::TiledScene(QQuickItem *parent)
	: TiledQuick::MapItem(parent)
{
	LOG_CTRACE("scene") << "Scene created" << this;

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
	unsetMap();

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
 * @brief TiledScene::reorderObjectsZ
 */

void TiledScene::reorderObjectsZ(const std::vector<TiledObject*> list)
{
	QHash<qreal, QMultiMap<qreal, TiledObject*>> map;

	for (TiledObject *obj : list) {
		IsometricObject *iso = dynamic_cast<IsometricObject*>(obj);

		qreal z = 0;
		const qreal y = obj->bodyPositionF().y();

		if (iso)
			z = getDynamicZ(obj->bodyPositionF(), iso->defaultZ()) + iso->subZ();
		else if (obj->m_visualItem)
			z = obj->m_visualItem->z();

		map[z].insert(y, obj);
	}

	for (const auto &[subZ, subMap] : map.asKeyValueRange()) {
		qreal subsubZ = 0.0001;

		for (const auto &[key, o] : subMap.asKeyValueRange()) {
			if (o->m_visualItem)
				o->m_visualItem->setZ(subZ+subsubZ);
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
		TiledQuick::TileLayerItem *mapItem = nullptr;

		if (TiledQuick::TileLayerItem *m = qobject_cast<TiledQuick::TileLayerItem *>(item))
			mapItem = m;
		else if (TiledVisualItem *m = qobject_cast<TiledVisualItem*>(item))
			mapItem = m->layerItem();

		if (mapItem && mapItem->tileLayer()->usedTilesets().contains(tileset->sharedPointer()))
			mapItem->update();
	}
}




/**
 * @brief TiledScene::debugDrawEvent
 * @param debugDraw
 */

void TiledScene::debugDrawEvent(TiledDebugDraw *debugDraw)
{
	if (m_debugDraw != debugDraw) {
		LOG_CERROR("scene") << "TiledDebugDraw mismatch" << m_debugDraw << debugDraw;
		return;
	}

	if (m_game)
		m_game->sceneDebugDrawEvent(debugDraw, this);
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

	return layerItem;
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

	return img;
}



/**
 * @brief TiledScene::addVisualItem
 * @param layer
 * @param renderer
 * @return
 */

TiledVisualItem *TiledScene::addVisualItem(Tiled::TileLayer *layer, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(layer);
	Q_ASSERT(renderer);

	TiledQuick::TileLayerItem *layerItem = new TiledQuick::TileLayerItem(layer, renderer, this);
	//m_visualItems.append(layerItem);


	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledImageFromLayer.qml"), this);

	auto img = qobject_cast<TiledVisualItem*>(
				   component.createWithInitialProperties({
															 { QStringLiteral("tiledLayer"), QVariant::fromValue(layerItem) }
														 }));

	if (!img) {
		LOG_CERROR("scene") << "TiledVisualItem create error" << component.errorString();
		return nullptr;
	}

	//layerItem->setParentItem(img);

	img->setLayerItem(layerItem);
	img->setParentItem(this);
	img->setParent(this);
	img->setScene(this);

	m_visualItems.append(img);

	return img;
}



/**
 * @brief TiledScene::addLightObject
 * @param object
 */

void TiledScene::addLightObject(Tiled::MapObject *object)
{
	m_lightObjects.append(object);
}







/**
 * @brief TiledScene::addLight
 * @param object
 * @param renderer
 * @return
 */

QQuickItem *TiledScene::addLight(Tiled::MapObject *object, Tiled::MapRenderer *renderer, const qreal &opacity)
{
	if (!object) {
		LOG_CERROR("scene") << "Missing map object";
		return nullptr;
	}

	if (object->shape() != Tiled::MapObject::Ellipse) {
		LOG_CERROR("scene") << "Invalid map object" << sceneId() << object->id();
		return nullptr;
	}

	QPointF offset;

	if (Tiled::ObjectGroup *gLayer = object->objectGroup())
		offset = gLayer->totalOffset();

	const QPointF &pos = renderer ? renderer->pixelToScreenCoords(offset + object->position()) :
									offset+object->position();

	qreal w = object->width() * sqrt(1.5);
	qreal h = object->height() * sqrt(0.5);


	QQuickItem *layerItem = nullptr;

	// Find last layer with the specified name

	const auto it = std::find_if(m_visualItems.crbegin(),
								 m_visualItems.crend(), [&object](QQuickItem *i){
					if (TiledQuick::TileLayerItem *layerItem = qobject_cast<TiledQuick::TileLayerItem *>(i)) {
					if (layerItem->tileLayer()->name() == object->name()) {
					return true;
}
}
					return false;

});


	if (it != m_visualItems.crend())
		layerItem = *it;


	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledVisualLight.qml"), this);

	QQuickItem *item = qobject_cast<QQuickItem*>(
						   component.createWithInitialProperties({
																	 { QStringLiteral("parent"), QVariant::fromValue(this) },
																	 { QStringLiteral("targetItem"), QVariant::fromValue(layerItem) },
																	 { QStringLiteral("opacity"), opacity },
																 }));

	if (!item) {
		LOG_CERROR("scene") << "TiledGame loadLights error" << component.errorString();
		return nullptr;
	}

	if (const QString &c = object->propertyAsString(QStringLiteral("color")); !c.isEmpty()) {
		item->setProperty("color", QColor::fromString(c));
	}

	item->setX(pos.x() - w/2);
	item->setY(pos.y() + h/2 * sin(M_PI/6));
	item->setWidth(w);
	item->setHeight(h);

	return item;
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

	m_lightObjects.clear();

	for (Tiled::Layer *layer : mMap->layers()) {
		m_game->loadSceneLayer(this, layer, mRenderer.get());
	}

	setTileLayersZ();

	m_game->loadLights(this, m_lightObjects, mRenderer.get());
	m_lightObjects.clear();


	const QRect rect = mRenderer->mapBoundingRect();
	setWidth(rect.width());
	setHeight(rect.height());

	if (m_viewport.isEmpty())
		setViewport(rect);
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
		m_dynamicZList.emplace_back(name, QVector<QRectF>{area}, 0);
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
	int minZ = 0;
	bool findZ = true;

	for (QQuickItem *item : m_visualItems) {
		QString name;

		if (TiledQuick::TileLayerItem *layerItem = qobject_cast<TiledQuick::TileLayerItem *>(item)) {
			if (Tiled::TileLayer *layer = layerItem->tileLayer()) {
				name = layer->name();
			}
		} else if (TiledVisualItem *vItem = qobject_cast<TiledVisualItem *>(item)) {
			name = vItem->name();
		}


		if (!name.isEmpty()) {
			const auto it = std::find_if(m_dynamicZList.begin(), m_dynamicZList.end(), [&name](const DynamicZ &d) {
				return (d.name == name);
			});

			if (it != m_dynamicZList.end()) {
				if (it->z == 0)
					it->z = i;
				else
					i = it->z;

				findZ = false;
			} else {
				for (QQuickItem *vi : m_visualItems) {
					if (vi == item)
						break;

					if (TiledQuick::TileLayerItem *layerItem = qobject_cast<TiledQuick::TileLayerItem *>(vi)) {
						if (layerItem->tileLayer()->name() == name) {
							LOG_CTRACE("scene") << "Scene" << m_sceneId << "link layer" << layerItem->tileLayer()->id()
												<< "to" << name;
							i = vi->z();
							break;
						}
					}

				}
			}
		}

		item->setZ(i);


		if (findZ)
			minZ = i;

		++i;
	}

	LOG_CTRACE("scene") << "Layers in scene" << m_sceneId;

	for (QQuickItem *item : m_visualItems) {
		QString name;
		if (TiledQuick::TileLayerItem *layerItem = qobject_cast<TiledQuick::TileLayerItem *>(item)) {
			name = layerItem->tileLayer()->name();
		} else if (TiledVisualItem *vItem = qobject_cast<TiledVisualItem *>(item)) {
			name = vItem->name();
		}

		LOG_CTRACE("scene") << "  -" << item << item->z() << name;
	}


	m_dynamicZList.emplace_back(QStringLiteral("__default__"), QList<QRectF>{QRectF(0,0,1,1)}, minZ);
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
