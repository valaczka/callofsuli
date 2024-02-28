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
#include "maprenderer.h"
#include "tiledmapobject.h"
#include "tilelayeritem.h"
#include "tilesetmanager.h"

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>

TiledScene::TiledScene(QQuickItem *parent)
	: TiledQuick::MapItem(parent)
	, m_mapLoader(new TiledQuick::MapLoader)
{
	setImplicitHeight(100);
	setImplicitWidth(100);

	//setAcceptHoverEvents(true);
	//setAcceptedMouseButtons(Qt::LeftButton);
	//setAcceptTouchEvents(true);

	connect(m_mapLoader.get(), &TiledQuick::MapLoader::statusChanged, this, &TiledScene::onSceneStatusChanged);
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
	return m_running;
}

void TiledScene::setRunning(bool newRunning)
{
	if (m_running == newRunning)
		return;
	m_running = newRunning;
	emit runningChanged();
}


/**
 * @brief TiledScene::world
 * @return
 */

Box2DWorld *TiledScene::world() const
{
	return m_world;
}

void TiledScene::setWorld(Box2DWorld *newWorld)
{
	if (m_world == newWorld)
		return;
	m_world = newWorld;
	emit worldChanged();

	if (m_world) {
		fpsSet();
		connect(m_world, &Box2DWorld::stepped, this, &TiledScene::onWorldStepped);
	}
}



/**
 * @brief TiledScene::onWorldStepped
 */

void TiledScene::onWorldStepped()
{
	/*qreal factor = 1.0;

	if (!m_elapsedTimer.isValid())
		m_elapsedTimer.start();
	else {
		const qreal &msec = m_elapsedTimer.restart();
		const qreal &interval = m_timingTimerTimeoutMsec;

		factor = msec/interval;

		const qreal &fpsFactor = (msec/1000.) * m_fpsList.at(m_fpsIndex);


		if (fpsFactor >= 1.5) {
			if (!m_performanceTimer.isValid())
				m_performanceTimer.start();
			else if (m_performanceTimer.elapsed() >= 2000) {
				fpsDecrease();
				m_performanceTimer.invalidate();
			}
		} else if (fpsFactor <= 0.8) {
			if (!m_performanceTimer.isValid())
				m_performanceTimer.start();
			else if (m_performanceTimer.elapsed() >= 1000) {
				fpsIncrease();
				m_performanceTimer.invalidate();
			}
		} else if (m_performanceTimer.isValid())
			m_performanceTimer.invalidate();
	}

	if (m_game)
		m_game->sceneTimerTimeout(m_timingTimerTimeoutMsec, factor);*/


}



/**
 * @brief TiledScene::fpsSet
 */

void TiledScene::fpsSet()
{
	/*double fps = m_fpsList.at(m_fpsIndex);
	LOG_CINFO("scene") << "Set fps:" << fps;

#ifndef QT_NO_DEBUG
	if (m_game)
		m_game->message(QString("FPS SET %1").arg(fps));
#endif

	if (m_world)
		m_world->setTimeStep(1./fps);	*/

	m_world->setTimeStep(1./60.);
}



/**
 * @brief TiledScene::joystickConnect
 */

void TiledScene::joystickConnect(const bool &connect)
{
	if (!m_joystick)
		return;

	const int methodIndex = this->metaObject()->indexOfMethod("updateJoystick()");

	LOG_CWARNING("scene") << "Joystick connect" << connect << methodIndex;

	Q_ASSERT(methodIndex != -1);

	const QMetaObject *mo = m_joystick->metaObject();

	static const QList<const char*> propList = {
		"currentX",
		"currentY",
		"currentAngle",
		"currentDistance",
		"hasTouch",
	};

	for (const char *prop : propList) {
		auto p = mo->property(mo->indexOfProperty(prop));

		if (p.hasNotifySignal()) {
			if (connect)
				QObject::connect(m_joystick, p.notifySignal(), this, this->metaObject()->method(methodIndex));
			else
				QObject::disconnect(m_joystick, p.notifySignal(), this, this->metaObject()->method(methodIndex));
		}
	}

	if (connect)
		LOG_CTRACE("scene") << "Joystick connected";
	else
		LOG_CTRACE("scene") << "Joystick disconnected";
}



/**
 * @brief TiledScene::updateJoystick
 */

void TiledScene::updateJoystick()
{
	JoystickState state;

	if (m_joystick) {
		state.dx = m_joystick->property("currentX").toReal();
		state.dy = m_joystick->property("currentY").toReal();
		state.distance = m_joystick->property("currentDistance").toReal();
		state.angle = m_joystick->property("currentAngle").toReal();
		state.hasTouch = m_joystick->property("hasTouch").toBool();
		state.hasKeyboard = m_joystickState.hasKeyboard;
	}

	setJoystickState(state);
}


/**
 * @brief TiledScene::updateKeyboardJoystick
 * @param state
 */

void TiledScene::updateKeyboardJoystick(const KeyboardJoystickState &state)
{
	if (m_joystickState.hasTouch)
		return;

	if (state.dx == m_keyboardJoystickState.dx && state.dy == m_keyboardJoystickState.dy)
		return;

	m_keyboardJoystickState = state;

	JoystickState jState = m_joystickState;
	jState.hasKeyboard = state.dx != 0.5 || state.dy != 0.5;
	setJoystickState(jState);

	if (!m_joystick)
		return;

	QMetaObject::invokeMethod(m_joystick, "moveThumbRelative",
							  Q_ARG(QVariant, state.dx),
							  Q_ARG(QVariant, state.dy)
							  );
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

bool TiledScene::debugView() const
{
	return m_debugView;
}

void TiledScene::setDebugView(bool newDebugView)
{
	if (m_debugView == newDebugView)
		return;
	m_debugView = newDebugView;
	emit debugViewChanged();
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
 * @brief TiledScene::joystickState
 * @return
 */

TiledScene::JoystickState TiledScene::joystickState() const
{
	return m_joystickState;
}

void TiledScene::setJoystickState(const JoystickState &newJoystickState)
{
	if (m_joystickState == newJoystickState)
		return;
	m_joystickState = newJoystickState;
	emit joystickStateChanged();
}



/**
 * @brief TiledScene::keyPressEvent
 * @param event
 */

void TiledScene::keyPressEvent(QKeyEvent *event)
{
	const int &key = event->key();

	qreal dx = m_keyboardJoystickState.dx;
	qreal dy = m_keyboardJoystickState.dy;

	switch (key) {
		case Qt::Key_Left:
			dx = 0.;
			break;

		case Qt::Key_Right:
			dx = 1.;
			break;

		case Qt::Key_Up:
			dy = 0.;
			break;

		case Qt::Key_Down:
			dy = 1.;
			break;

		case Qt::Key_Home:
			dx = 0.;
			dy = 0.;
			break;

		case Qt::Key_End:
			dx = 0.;
			dy = 1.;
			break;

		case Qt::Key_PageUp:
			dx = 1.;
			dy = 0.;
			break;

		case Qt::Key_PageDown:
			dx = 1.;
			dy = 1.;
			break;
	}

	updateKeyboardJoystick(KeyboardJoystickState{dx, dy});
}


/**
 * @brief TiledScene::keyReleaseEvent
 * @param event
 */

void TiledScene::keyReleaseEvent(QKeyEvent *event)
{
	const int &key = event->key();

	if (event->isAutoRepeat())
		return;

	qreal dx = m_keyboardJoystickState.dx;
	qreal dy = m_keyboardJoystickState.dy;

	switch (key) {
		case Qt::Key_Left:
		case Qt::Key_Right:
			dx = 0.5;
			break;

		case Qt::Key_Up:
		case Qt::Key_Down:
			dy = 0.5;
			break;

		case Qt::Key_Home:
		case Qt::Key_End:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
			dx = 0.5;
			dy = 0.5;
			break;
	}

	updateKeyboardJoystick(KeyboardJoystickState{dx, dy});
}



/**
 * @brief TiledScene::joystick
 * @return
 */

QQuickItem *TiledScene::joystick() const
{
	return m_joystick;
}

void TiledScene::setJoystick(QQuickItem *newJoystick)
{
	if (m_joystick == newJoystick)
		return;

	if (m_joystick)
		joystickConnect(false);

	m_joystick = newJoystick;
	emit joystickChanged();

	if (m_joystick)
		joystickConnect(true);
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
			for (Tiled::MapObject *object : group->objects()) {
				TiledMapObject *mapObject = TiledMapObject::createFromMapObject(object, mRenderer.get(), this);
				if (!mapObject)
					continue;

				mapObject->defaultFixture()->setDensity(1);
				mapObject->defaultFixture()->setFriction(1);
				mapObject->defaultFixture()->setRestitution(0);
				mapObject->defaultFixture()->setCategories(Box2DFixture::Category1);

				if (object->hasProperty(QStringLiteral("dynamicZ"))) {
					if (const QPolygonF &p = mapObject->screenPolygon(); !p.isEmpty()) {

						DynamicZ dz;
						dz.polygon = p;

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
		}
	}



	const QRect rect = mRenderer->mapBoundingRect();
	setWidth(rect.width());
	setHeight(rect.height());
}
