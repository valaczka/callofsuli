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
#include "isometricenemy.h"
#include "isometricplayer.h"
#include <libtiled/objectgroup.h>

TiledGame::TiledGame(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CTRACE("scene") << "TiledGame created" << this;
}


/**
 * @brief TiledGame::~TiledGame
 */

TiledGame::~TiledGame()
{
	LOG_CTRACE("scene") << "TiledGame destroy" << this;
}


/**
 * @brief TiledGame::load
 * @return
 */

bool TiledGame::load()
{

	QString test = R"({
	"firstScene": 1,
	"scenes": [
		{
			"id": 1,
			"file": "qrc:/teszt.tmx"
		},

		{
			"id": 2,
			"file": "qrc:/teszt2.tmx"
		}
	]
	})";


	TiledGameDefinition def;
	def.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	return load(def);
}



/**
 * @brief TiledGame::load
 * @param def
 * @return
 */

bool TiledGame::load(const TiledGameDefinition &def)
{
	LOG_CTRACE("game") << "Load game";

	for (const TiledSceneDefinition &s : def.scenes) {
		if (loadScene(s.id, s.file))
			LOG_CINFO("game") << "Scene loaded:" << s.id << qPrintable(s.file);
		else
			return false;
	}

	TiledScene *firstScene = findScene(def.firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Invalid scene id:" << def.firstScene;
		return false;
	}

	setCurrentScene(firstScene);
	firstScene->forceActiveFocus();

	return true;
}



/**
 * @brief TiledGame::addGate
 * @param name
 * @param scene
 * @param object
 * @return
 */

bool TiledGame::addGate(const QString &name, TiledScene *scene, TiledObjectBase *object)
{
	return m_transportList.add(name, scene, object);
}


/**
 * @brief TiledGame::switchScene
 * @param character
 */

void TiledGame::switchScene()
{
	LOG_CDEBUG("scene") << "SWITCH";

	TiledTransport *transport = m_player->currentTransport();

	if (!transport) {
		LOG_CWARNING("scene") << "No transport";
		return;
	}

	TiledScene *oldScene = m_player->scene();
	TiledScene *newScene = transport->otherScene(oldScene);
	TiledObjectBase *newObject = transport->otherObject(oldScene);

	if (!newScene || !newObject) {
		LOG_CERROR("scene") << "Wrong transport object";
		return;
	}

	oldScene->removeFromObjects(m_player.get());

	m_player->setScene(newScene);
	m_player->emplace(newObject->body()->bodyPosition());

	newScene->appendToObjects(m_player.get());

	setCurrentScene(newScene);
	newScene->forceActiveFocus();
}



/**
 * @brief TiledGame::loadPlayer
 * @param scene
 * @param pos
 */

void TiledGame::loadPlayer(TiledScene *scene, const QPointF &pos)
{
	m_player.reset(IsometricPlayer::createPlayer(scene));

	Q_ASSERT(m_player);

	m_player->emplace(pos);
	m_player->setCurrentDirection(TiledObject::South);

	scene->appendToObjects(m_player.get());
	setFollowedItem(m_player.get());
	setControlledPlayer(m_player.get());
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

	LOG_CTRACE("game") << "Load layer" << layer->typeId() << layer->id() << layer->name() << "to scene" << scene->sceneId();

	if (Tiled::TileLayer *tl = layer->asTileLayer()) {
		return tl;
	} else if (Tiled::ObjectGroup *group = layer->asObjectGroup()) {
		loadObjectLayer(scene, group, renderer);
	} /*else if (Tiled::GroupLayer *group = layer->asGroupLayer()) {
		loadGroupLayer(group);
	}*/

	return nullptr;
}




/**
 * @brief TiledGame::getTexture
 * @param path
 * @return
 */

const std::shared_ptr<QSGTexture> &TiledGame::getTexture(const QString &path)
{
	auto it = m_sharedTextures.find(path);

	if (it != m_sharedTextures.end())
		return *it;

	QSGTexture *texture = window()->createTextureFromImage(QImage(path));

	LOG_CTRACE("scene") << "Create texture from image:" << path << texture;

	std::shared_ptr<QSGTexture> s(texture);

	return *(m_sharedTextures.insert(path, std::move(s)));
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
	m_currentScene = newCurrentScene;
	emit currentSceneChanged();
}



/**
 * @brief TiledGame::loadScene
 * @param file
 * @return
 */

bool TiledGame::loadScene(const int sceneId, const QString &file)
{
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledFlickableScene.qml"), this);

	LOG_CDEBUG("game") << "Create scene flickable item:" << component.isReady();

	Scene item;

	item.container = qobject_cast<QQuickItem*>(component.create());

	if (!item.container) {
		LOG_CERROR("game") << "Scene create error" << component.errorString();
		return false;
	}

	item.scene = qvariant_cast<TiledScene*>(item.container->property("scene"));
	Q_ASSERT(item.scene);

	item.scene->setSceneId(sceneId);
	item.scene->setGame(this);
	item.container->setParentItem(this);
	item.container->setParent(this);

	if (!item.scene->load(QUrl(file))) {
		LOG_CERROR("game") << "Scene load error" << file;
		item.container->deleteLater();
		return false;
	}

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

	for (Tiled::MapObject *object : group->objects()) {
		int idx = findObject(object->id(), scene->sceneId());

		if (idx != -1) {
			LOG_CERROR("game") << "Object already created" << object->id() << scene->sceneId();
			continue;
		}

		if (object->className() == "enemy") {
			QPolygonF p = TiledObjectBase::toPolygon(object, renderer);
			LOG_CINFO("scene") << "ENEMY" << object->id();

			IsometricEnemy *character = IsometricEnemy::createEnemy(scene);
			Q_ASSERT(character);

			character->setObjectId(object->id());

			m_objectList.append(Object{
									EnemyWerebear,
									object->id(),
									scene->sceneId(),
									scene,
									character
								});

			character->loadPathMotor(p);
			scene->appendToObjects(character);
		} else if (object->className() == "player") {
			LOG_CINFO("scene") << "PLAYER" << object->id();
			loadPlayer(scene, renderer->pixelToScreenCoords(object->position()));
		} else if (object->className() == "gate") {

			LOG_CINFO("scene") << "GATE" << object->id();

			loadTransport(scene, object, renderer);
		} else if (object->className() == "ground" || (object->className().isEmpty() && group->className() == "ground")) {
			loadGround(scene, object, renderer);
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

bool TiledGame::loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(object);
	Q_ASSERT(renderer);

	TiledObjectBasePolygon *mapObject = nullptr;
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, renderer);

	if (!mapObject)
		return false;

	mapObject->setScene(scene);
	mapObject->fixture()->setDensity(1);
	mapObject->fixture()->setFriction(1);
	mapObject->fixture()->setRestitution(0);
	mapObject->fixture()->setCategories(Box2DFixture::Category1);

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

	return true;
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
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, renderer, this);

	if (!mapObject)
		return false;

	mapObject->setScene(scene);
	mapObject->fixture()->setSensor(true);
	mapObject->fixture()->setCategories(Box2DFixture::Category4);

	return addGate(object->name(), scene, mapObject);
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


		case Qt::Key_S:
			switchScene();
			break;

		case Qt::Key_D:
			setDebugView(!m_debugView);
			break;

		case Qt::Key_Space:
			if (m_controlledPlayer)
				m_controlledPlayer->hit();
			break;
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




/**
 * @brief TiledGame::findObject
 * @param id
 * @param sceneId
 * @return
 */

int TiledGame::findObject(const int &id, const int &sceneId) const
{
	auto it = std::find_if(m_objectList.cbegin(), m_objectList.cend(),
						   [&id, &sceneId](const Object &o){
		return o.sceneId == sceneId && o.id == id;
	});

	if (it == m_objectList.cend())
		return -1;
	else
		return it-m_objectList.cbegin();
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
 * @brief TiledGame::controlledPlayer
 * @return
 */

IsometricPlayer *TiledGame::controlledPlayer() const
{
	return m_controlledPlayer;
}

void TiledGame::setControlledPlayer(IsometricPlayer *newControlledItem)
{
	if (m_controlledPlayer == newControlledItem)
		return;
	m_controlledPlayer = newControlledItem;
	emit controlledPlayerChanged();
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

	if (m_controlledPlayer)
		m_controlledPlayer->onJoystickStateChanged();
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
