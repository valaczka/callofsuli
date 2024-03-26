/*
 * ---- Call of Suli ----
 *
 * tiledgame.cpp
 *
 * Created on: 2024. 03. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledGame
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

#include "tiledgame.h"
#include "Logger.h"
#include "application.h"
#include "isometricentity.h"
#include <libtiled/objectgroup.h>

std::unordered_map<QString, std::unique_ptr<QSGTexture>> TiledGame::m_sharedTextures;

TiledGame::TiledGame(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CTRACE("scene") << "TiledGame created" << this;

	connect(this, &TiledGame::activeFocusChanged, this, [this](const bool &focus){
		if (!focus)
			updateKeyboardJoystick(KeyboardJoystickState{.5, .5});
	});
}


/**
 * @brief TiledGame::~TiledGame
 */

TiledGame::~TiledGame()
{
	for (const auto &s : std::as_const(m_sceneList)) {
		s.scene->stopMusic();
		s.scene->world()->setRunning(false);
		if (s.scene->game() == this)
			s.scene->setGame(nullptr);
	}

	m_currentScene = nullptr;
	m_sceneList.clear();
	m_loadedObjectList.clear();
	m_playerPositionList.clear();

	LOG_CTRACE("scene") << "TiledGame destroy" << this;
}





/**
 * @brief TiledGame::load
 * @param def
 * @return
 */

bool TiledGame::load(const TiledGameDefinition &def)
{
	LOG_CTRACE("game") << "Load game";

	for (const TiledSceneDefinition &s : std::as_const(def.scenes)) {
		if (loadScene(s))
			LOG_CINFO("game") << "Scene loaded:" << s.id << qPrintable(s.file);
		else {
			emit gameLoadFailed();
			return false;
		}
	}

	TiledScene *firstScene = findScene(def.firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Invalid scene id:" << def.firstScene;
		emit gameLoadFailed();
		return false;
	}

	setCurrentScene(firstScene);

	return true;
}





/**
 * @brief TiledGame::loadSceneLayer
 * @param scene
 * @param layer
 * @return
 */

Tiled::TileLayer *TiledGame::loadSceneLayer(TiledScene *scene, Tiled::Layer *layer, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(layer);
	Q_ASSERT(renderer);

	LOG_CTRACE("game") << "Load layer" << layer->id() << layer->name() << "to scene" << scene->sceneId();

	if (Tiled::TileLayer *tl = layer->asTileLayer()) {
		scene->addTileLayer(tl, renderer);
	} else if (Tiled::ObjectGroup *group = layer->asObjectGroup()) {
		loadObjectLayer(scene, group, renderer);
	} else if (Tiled::GroupLayer *group = layer->asGroupLayer()) {
		loadGroupLayer(scene, group, renderer);
	}

	return nullptr;
}



/**
 * @brief TiledGame::getTexture
 * @param path
 * @return
 */

QSGTexture *TiledGame::getTexture(const QString &path, QQuickWindow *window)
{
	Q_ASSERT(window);

	auto it = m_sharedTextures.find(path);

	if (it != m_sharedTextures.end())
		return it->second.get();

	QSGTexture *texture = window->createTextureFromImage(QImage(path));

	LOG_CTRACE("scene") << "Create texture from image:" << path << texture;

	std::unique_ptr<QSGTexture> s(texture);

	const auto &ptr = m_sharedTextures.insert({path, std::move(s)});

	Q_ASSERT(ptr.second);

	return ptr.first->second.get();
}





/**
 * @brief TiledGame::findScene
 * @param id
 * @return
 */

TiledScene *TiledGame::findScene(const int &id) const
{
	auto it = std::find_if(m_sceneList.constBegin(), m_sceneList.constEnd(),
						   [&id](const Scene &s){
		return s.scene && s.scene->sceneId() == id;
	});

	if (it == m_sceneList.constEnd())
		return nullptr;

	return it->scene;
}


/**
 * @brief TiledGame::currentScene
 * @return
 */

TiledScene *TiledGame::currentScene() const
{
	return m_currentScene;
}

void TiledGame::setCurrentScene(TiledScene *newCurrentScene)
{
	if (m_currentScene == newCurrentScene)
		return;

	if (m_currentScene)
		m_currentScene->stopMusic();

	m_currentScene = newCurrentScene;
	emit currentSceneChanged();

	if (m_currentScene)
		m_currentScene->startMusic();
}



/**
 * @brief TiledGame::loadScene
 * @param file
 * @return
 */

bool TiledGame::loadScene(const TiledSceneDefinition &def)
{
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledFlickableScene.qml"), this);

	LOG_CDEBUG("game") << "Create scene flickable item:" << def.id;

	Scene item;

	item.container = qobject_cast<QQuickItem*>(component.create());

	if (!item.container) {
		LOG_CERROR("game") << "Scene create error" << component.errorString();
		return false;
	}

	item.scene = qvariant_cast<TiledScene*>(item.container->property("scene"));
	Q_ASSERT(item.scene);

	item.scene->setSceneId(def.id);
	item.scene->setGame(this);
	item.container->setParentItem(this);
	item.container->setParent(this);

	if (!item.scene->load(QUrl(def.file))) {
		LOG_CERROR("game") << "Scene load error" << def.file;
		item.container->deleteLater();
		return false;
	}

	item.scene->setAmbientSound(def.ambient);
	item.scene->setBackgroundMusic(def.music);

	m_sceneList.append(std::move(item));

	return true;
}



/**
 * @brief TiledGame::loadObjectLayer
 * @param group
 * @return
 */

bool TiledGame::loadObjectLayer(TiledScene *scene, Tiled::ObjectGroup *group, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(group);
	Q_ASSERT(renderer);

	for (Tiled::MapObject *object : std::as_const(group->objects())) {
		int idx = findLoadedObject(object->id(), scene->sceneId());

		if (idx != -1) {
			LOG_CERROR("game") << "Object already created" << object->id() << scene->sceneId();
			continue;
		}

		if (object->className() == QStringLiteral("player")) {
			addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
			addLoadedObject(object->id(), scene->sceneId());
		} else if (TiledTransport::typeFromString(object->className()) != TiledTransport::TransportInvalid) {
			loadTransport(scene, object, renderer);
		} else if (object->className() == QStringLiteral("ground") ||
				   (object->className().isEmpty() && group->className() == QStringLiteral("ground"))) {
			loadGround(scene, object, renderer);
		} else {
			loadObjectLayer(scene, object, renderer);
		}
	}

	return true;
}



/**
 * @brief TiledGame::loadGround
 * @param scene
 * @param object
 * @param renderer
 * @return
 */

TiledObjectBasePolygon *TiledGame::loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(object);
	Q_ASSERT(renderer);

	TiledObjectBasePolygon *mapObject = nullptr;
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, renderer, scene);

	if (!mapObject)
		return nullptr;

	mapObject->setParent(scene);
	mapObject->setScene(scene);
	mapObject->fixture()->setDensity(1);
	mapObject->fixture()->setFriction(1);
	mapObject->fixture()->setRestitution(0);
	mapObject->fixture()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround));

	scene->m_groundObjects.append(QPointer(mapObject));

	if (object->hasProperty(QStringLiteral("dynamicZ"))) {
		if (const QPolygonF &p = mapObject->screenPolygon(); !p.isEmpty()) {

			TiledScene::DynamicZ dz;
			dz.polygon = p.translated(mapObject->position());

			if (object->hasProperty(QStringLiteral("dynamicVertical")))
				dz.vertical = object->property(QStringLiteral("dynamicVertical")).toBool();

			if (object->hasProperty(QStringLiteral("dynamicHorizontal")))
				dz.horizontal = object->property(QStringLiteral("dynamicHorizontal")).toBool();

			scene->m_dynamicZList[object->property(QStringLiteral("dynamicZ")).toInt()] = dz;
		}
	}

	if (object->hasProperty(QStringLiteral("z"))) {
		mapObject->setZ(object->property(QStringLiteral("z")).toInt());
	} else {
		mapObject->setZ(0);
	}

	addLoadedObject(object->id(), scene->sceneId());

	return mapObject;
}



/**
 * @brief TiledGame::loadTransport
 * @param scene
 * @param object
 * @param renderer
 * @return
 */

bool TiledGame::loadTransport(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	TiledObjectBasePolygon *mapObject = nullptr;
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, renderer, scene);

	if (!mapObject)
		return false;

	mapObject->setParent(scene);
	mapObject->setScene(scene);
	mapObject->fixture()->setSensor(true);
	mapObject->fixture()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport));

	if (!m_transportList.add(TiledTransport::typeFromString(object->className()), object->name(), scene, mapObject))
		return false;

	addLoadedObject(object->id(), scene->sceneId());

	return true;
}



/**
 * @brief TiledGame::loadObjectLayer
 * @param scene
 * @param object
 * @param renderer
 */

void TiledGame::loadObjectLayer(TiledScene *, Tiled::MapObject *, Tiled::MapRenderer *)
{
	LOG_CERROR("game") << "Missing implementation:" << __PRETTY_FUNCTION__;
}


/**
 * @brief TiledGame::loadGroupLayer
 * @param scene
 * @param group
 * @param renderer
 */

void TiledGame::loadGroupLayer(TiledScene *, Tiled::GroupLayer *, Tiled::MapRenderer *)
{
LOG_CERROR("game") << "Missing implementation:" << __PRETTY_FUNCTION__;
}




/**
 * @brief TiledGame::keyPressEvent
 * @param event
 */

void TiledGame::keyPressEvent(QKeyEvent *event)
{
	const int &key = event->key();

	qreal dx = m_keyboardJoystickState.dx;
	qreal dy = m_keyboardJoystickState.dy;

	const bool isSlow = event->modifiers().testAnyFlag(Qt::ShiftModifier);

	switch (key) {
		case Qt::Key_Left:
			dx = isSlow ? 0.2 : 0.;
			break;

		case Qt::Key_Right:
			dx = isSlow ? 0.8 : 1.;
			break;

		case Qt::Key_Up:
			dy = isSlow ? 0.2 : 0.;
			break;

		case Qt::Key_Down:
			dy = isSlow ? 0.8 : 1.;
			break;

		case Qt::Key_Home:
			dx = isSlow ? 0.2 : 0.;
			dy = isSlow ? 0.2 : 0.;
			break;

		case Qt::Key_End:
			dx = isSlow ? 0.2 : 0.;
			dy = isSlow ? 0.8 : 1.;
			break;

		case Qt::Key_PageUp:
			dx = isSlow ? 0.8 : 1.;
			dy = isSlow ? 0.2 : 0.;
			break;

		case Qt::Key_PageDown:
			dx = isSlow ? 0.8 : 1.;
			dy = isSlow ? 0.8 : 1.;
			break;

#ifndef QT_NO_DEBUG
		case Qt::Key_D:
			setDebugView(!m_debugView);
			break;
#endif
	}

	updateKeyboardJoystick(KeyboardJoystickState{dx, dy});
}


/**
 * @brief TiledGame::keyReleaseEvent
 * @param event
 */

void TiledGame::keyReleaseEvent(QKeyEvent *event)
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
 * @brief TiledGame::transportBeforeEvent
 * @param object
 * @param transport
 * @return
 */

bool TiledGame::transportBeforeEvent(TiledObject */*object*/, TiledTransport */*transport*/)
{
	return true;
}



/**
 * @brief TiledGame::transportAfterEvent
 * @return
 */

bool TiledGame::transportAfterEvent(TiledObject */*object*/, TiledScene */*newScene*/, TiledObjectBase */*newObject*/)
{
	return true;
}






/**
 * @brief TiledGame::joystickConnect
 * @param connect
 */

void TiledGame::joystickConnect(const bool &connect)
{
	if (!m_joystick)
		return;

	const int methodIndex = this->metaObject()->indexOfMethod("updateJoystick()");

	Q_ASSERT(methodIndex != -1);

	const QMetaObject *mo = m_joystick->metaObject();

	static const QList<const char*> propList = {
		"currentX",
		"currentY",
		"currentAngle",
		"currentDistance",
		"hasTouch",
	};

	for (const char *prop : std::as_const(propList)) {
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
 * @brief TiledGame::updateJoystick
 */

void TiledGame::updateJoystick()
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
 * @brief TiledGame::updateKeyboardJoystick
 * @param state
 */

void TiledGame::updateKeyboardJoystick(const KeyboardJoystickState &state)
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

qreal TiledGame::baseScale() const
{
	return m_baseScale;
}

void TiledGame::setBaseScale(qreal newBaseScale)
{
	if (qFuzzyCompare(m_baseScale, newBaseScale))
		return;
	m_baseScale = newBaseScale;
	emit baseScaleChanged();
}

QColor TiledGame::defaultMessageColor() const
{
	return m_defaultMessageColor;
}

void TiledGame::setDefaultMessageColor(const QColor &newDefaultMessageColor)
{
	if (m_defaultMessageColor == newDefaultMessageColor)
		return;
	m_defaultMessageColor = newDefaultMessageColor;
	emit defaultMessageColorChanged();
}

QQuickItem *TiledGame::messageList() const
{
	return m_messageList;
}

void TiledGame::setMessageList(QQuickItem *newMessageList)
{
	if (m_messageList == newMessageList)
		return;
	m_messageList = newMessageList;
	emit messageListChanged();
}




/**
 * @brief TiledGame::findLoadedObject
 * @param id
 * @param sceneId
 * @return
 */

int TiledGame::findLoadedObject(const int &id, const int &sceneId) const
{
	auto it = std::find_if(m_loadedObjectList.cbegin(), m_loadedObjectList.cend(),
						   [&id, &sceneId](const TiledObjectBase::ObjectId &o){
		return o.sceneId == sceneId && o.id == id;
	});

	if (it == m_loadedObjectList.cend())
		return -1;
	else
		return it-m_loadedObjectList.cbegin();
}




/**
 * @brief TiledGame::addLoadedObject
 * @param id
 * @param sceneId
 * @return
 */

bool TiledGame::addLoadedObject(const int &id, const int &sceneId)
{
	auto it = std::find_if(m_loadedObjectList.cbegin(), m_loadedObjectList.cend(),
						   [&id, &sceneId](const TiledObjectBase::ObjectId &o){
		return o.sceneId == sceneId && o.id == id;
	});

	if (it != m_loadedObjectList.cend())
		return false;

	m_loadedObjectList.append(TiledObjectBase::ObjectId{ id, sceneId });

	return true;
}



/**
 * @brief TiledGame::changeScene
 * @param object
 * @param from
 * @param to
 */

void TiledGame::changeScene(TiledObject *object, TiledScene *from, TiledScene *to, const QPointF &toPoint)
{
	Q_ASSERT(object);
	Q_ASSERT(from);
	Q_ASSERT(to);

	from->removeFromObjects(object);

	object->setScene(to);
	object->body()->emplace(toPoint);

	to->appendToObjects(object);

	IsometricEntityIface *entity = dynamic_cast<IsometricEntityIface*>(object);

	if (entity)
		entity->updateSprite();
}



/**
 * @brief TiledGame::addPlayerPosition
 * @param scene
 * @param position
 */

void TiledGame::addPlayerPosition(TiledScene *scene, const QPointF &position)
{
	auto it = std::find_if(m_playerPositionList.constBegin(), m_playerPositionList.constEnd(),
						   [scene, &position](const PlayerPosition &p){
		return p.scene == scene && p.position == position;
	});

	if (it != m_playerPositionList.constEnd()) {
		LOG_CDEBUG("game") << "Player position already loaded:" << it->sceneId << it->position;
		return;
	}

	m_playerPositionList.append(PlayerPosition{
									scene ? scene->sceneId() : -1,
									scene,
									position
								});
}



/**
 * @brief TiledGame::followedItem
 * @return
 */

TiledObject *TiledGame::followedItem() const
{
	return m_followedItem;
}

void TiledGame::setFollowedItem(TiledObject *newFollowedItem)
{
	if (m_followedItem == newFollowedItem)
		return;
	m_followedItem = newFollowedItem;
	emit followedItemChanged();
}


/**
 * @brief TiledGame::transportList
 * @return
 */

const TiledTransportList &TiledGame::transportList() const
{
	return m_transportList;
}




/**
 * @brief TiledGame::transport
 * @param object
 * @param transport
 * @return
 */

bool TiledGame::transport(TiledObject *object, TiledTransport *transport)
{
	Q_ASSERT(object);

	if (!transport)
		return false;


	TiledScene *oldScene = object->scene();
	TiledScene *newScene = transport->otherScene(oldScene);
	TiledObjectBase *newObject = transport->otherObject(oldScene);

	if (!newScene || !newObject) {
		LOG_CERROR("game") << "Broken transport object";
		return false;
	}

	if (!transportBeforeEvent(object, transport))
		return false;

	changeScene(object, oldScene, newScene, newObject->body()->bodyPosition());

	if (!transportAfterEvent(object, newScene, newObject))
		return false;

	setCurrentScene(newScene);

	return true;
}




/**
 * @brief TiledGame::playSfx
 * @param source
 */

void TiledGame::playSfx(const QString &source, TiledScene *scene, const float &baseVolume) const
{
	if (!scene || !m_currentScene || m_currentScene != scene)
		return;

	const qreal &scale = m_currentScene->scale() + (1.-m_baseScale);
	const qreal &factor = scale < 1.0 ? std::max(0.1, -1.+2.*scale)*baseVolume : baseVolume;

	Application::instance()->client()->sound()->playSound(source, Sound::SfxChannel, factor);
}


/**
 * @brief TiledGame::playSfx
 * @param source
 * @param position
 */

void TiledGame::playSfx(const QString &source, TiledScene *scene, const QPointF &position, const float &baseVolume) const
{
	if (!scene || !m_currentScene || m_currentScene != scene)
		return;

	static const std::map<qreal, float> distanceMap = {
		{ 0, 1.0 },
		{ 20, 0.8 },
		{ 50, 0.7 },
		{ 75, 0.5 },
		{ 100, 0.4 },
		{ 120, 0.3 },
		{ 150, 0.2 }
	};

	const QRectF &rect = m_currentScene->visibleArea();
	const qreal &scale = m_currentScene->scale() + (1.-m_baseScale);
	const qreal &factor = scale < 1.0 ? std::max(0.1, -1.+2.*scale)*baseVolume : baseVolume;

	for (const auto &[margin, volume] : std::as_const(distanceMap)) {
		if (rect.marginsAdded(QMarginsF{margin, margin, margin, margin}).contains(position)) {
			Application::instance()->client()->sound()->playSound(source, Sound::SfxChannel, volume*factor);
			return;
		}
	}

}



/**
 * @brief TiledGame::messageColor
 * @param text
 * @param color
 */

void TiledGame::messageColor(const QString &text, const QColor &color)
{
	if (!m_messageList) {
		LOG_CINFO("game") << text;
		return;
	}

	LOG_CDEBUG("game") << text;

	QMetaObject::invokeMethod(m_messageList, "message", Qt::DirectConnection,
							  Q_ARG(QVariant, text),
							  Q_ARG(QVariant, color.name()));
}



/**
 * @brief TiledGame::joystick
 * @return
 */

QQuickItem *TiledGame::joystick() const
{
	return m_joystick;
}

void TiledGame::setJoystick(QQuickItem *newJoystick)
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
 * @brief TiledGame::joystickState
 * @return
 */

TiledGame::JoystickState TiledGame::joystickState() const
{
	return m_joystickState;
}

void TiledGame::setJoystickState(const JoystickState &newJoystickState)
{
	if (m_joystickState == newJoystickState)
		return;
	m_joystickState = newJoystickState;
	emit joystickStateChanged();

	joystickStateEvent(newJoystickState);
}



/**
 * @brief TiledGame::debugView
 * @return
 */

bool TiledGame::debugView() const
{
	return m_debugView;
}

void TiledGame::setDebugView(bool newDebugView)
{
	if (m_debugView == newDebugView)
		return;
	m_debugView = newDebugView;
	emit debugViewChanged();
}
