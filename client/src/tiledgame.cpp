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
#include "rpgenemyiface.h"
#include "tiledspritehandler.h"
#include "utils_.h"
#include <libtiled/objectgroup.h>
#include <libtiled/mapreader.h>
#include <libtiled/map.h>
#include <libtiled/imagecache.h>

std::unordered_map<QString, std::unique_ptr<QSGTexture>> TiledGame::m_sharedTextures;



/**
 * @brief TiledGame::TiledGame
 * @param parent
 */

TiledGame::TiledGame(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CTRACE("scene") << "TiledGame created" << this;

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
	const bool defValue = false;
#else
	const bool defValue = true;
#endif

	setMouseNavigation(Utils::settingsGet(QStringLiteral("game/mouseNavigation"), defValue).toBool());
	setMouseAttack(Utils::settingsGet(QStringLiteral("game/mouseAttack"), false).toBool());
	setFlickableInteractive(Utils::settingsGet(QStringLiteral("game/flickableInteractive"), true).toBool());

	connect(this, &TiledGame::activeFocusChanged, this, [this](const bool &focus){
		if (!focus) {
			m_keyboardJoystickState.clear();
			updateKeyboardJoystick();
		}
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
	LOG_CTRACE("game") << "Load game with base path:" << def.basePath;

	for (const TiledSceneDefinition &s : std::as_const(def.scenes)) {
		if (loadScene(s, def.basePath))
			LOG_CINFO("game") << "Scene loaded:" << s.id << qPrintable(s.file);
		else {
			emit gameLoadFailed(tr("Hibás pálya"));
			return false;
		}

		Tiled::ImageCache::clear();
	}

	TiledScene *firstScene = findScene(def.firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Invalid scene id:" << def.firstScene;
		emit gameLoadFailed(tr("Kezdőpont nincs beállítva"));
		return false;
	}

	setCurrentScene(firstScene);

	return true;
}


/**
 * @brief TiledGame::getDynamicTilesets
 * @param def
 * @param basePath
 * @return
 */

std::optional<QStringList> TiledGame::getDynamicTilesets(const TiledGameDefinition &def)
{
	QStringList list;

	for (const TiledSceneDefinition &s : def.scenes) {
		LOG_CTRACE("game") << "Get dynamic tilesets from file:" << s.file;

		const QString filename = Tiled::urlToLocalFileOrQrc(QUrl(def.basePath+'/'+s.file));

		QDir path;
		path.setPath(QFileInfo(filename).absolutePath());

		QFile f(filename);

		if (!f.exists()) {
			LOG_CERROR("game") << "Read error:" << filename;
			return std::nullopt;
		}

		if (!f.open(QFile::ReadOnly | QFile::Text)) {
			LOG_CERROR("game") << "Read error:" << filename;
			return std::nullopt;
		}

		QXmlStreamReader xml;

		xml.setDevice(&f);

		if (xml.readNextStartElement() && xml.name() == QStringLiteral("map")) {
			while (xml.readNextStartElement()) {
				if (xml.name() == QStringLiteral("tileset")) {
					const QXmlStreamAttributes atts = xml.attributes();
					const QString source = atts.value(QStringLiteral("source")).toString();

					if (!source.isEmpty())
						list.append(QFileInfo(QDir::cleanPath(path.filePath(source))).canonicalFilePath());

					xml.skipCurrentElement();

				} else if (xml.name() == QStringLiteral("objectgroup") &&
						   xml.attributes().value(QStringLiteral("name")).toString() == QStringLiteral("enemy")) {

					while (xml.readNextStartElement()) {
						if (xml.name() == QStringLiteral("object")) {
							const QXmlStreamAttributes atts = xml.attributes();
							const QString type = atts.value(QStringLiteral("type")).toString();
							const QString name = atts.value(QStringLiteral("name")).toString();

							list.append(RpgEnemyIface::directoryBaseName(RpgEnemyIface::typeFromString(type),
																		 name) + QStringLiteral(".dres"));
						}

						xml.skipCurrentElement();
					}
				} else {
					xml.skipCurrentElement();
				}

			}
		} else {
			LOG_CERROR("game") << "Invalid file:" << filename;
			return std::nullopt;
		}
	}

	list.removeDuplicates();
	return list;
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
	} else if (Tiled::ImageLayer *image= layer->asImageLayer()) {
		loadImageLayer(scene, image, renderer);
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
	auto it = m_sharedTextures.find(path);

	if (it != m_sharedTextures.end())
		return it->second.get();

	if (!window) {
		LOG_CERROR("scene") << "Can't create texture:" << path;
		return nullptr;
	}

	QSGTexture *texture = window->createTextureFromImage(QImage(path));

	LOG_CTRACE("scene") << "Create texture from image:" << path << texture;

	std::unique_ptr<QSGTexture> s(texture);

	const auto &ptr = m_sharedTextures.insert({path, std::move(s)});

	Q_ASSERT(ptr.second);

	return ptr.first->second.get();
}


/**
 * @brief TiledGame::sceneList
 * @return
 */

QVector<TiledScene *> TiledGame::sceneList() const
{
	QVector<TiledScene *> list;

	for (const Scene &s : m_sceneList) {
		if (s.scene)
			list.append(s.scene);
	}

	return list;
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

	if (m_currentScene) {
		if (!newCurrentScene || newCurrentScene->m_ambientSound != m_currentScene->m_ambientSound)
			m_currentScene->stopMusic();
	}

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

bool TiledGame::loadScene(const TiledSceneDefinition &def, const QString &basePath)
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

	if (!item.scene->load(QUrl(basePath+'/'+def.file))) {
		LOG_CERROR("game") << "Scene load error" << def.file;
		item.container->deleteLater();
		return false;
	}

	item.scene->setAmbientSound(def.ambient);
	item.scene->setBackgroundMusic(def.music);
	item.scene->setSceneEffect(def.effect);

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

	QString className = group->className();

	if (className.isEmpty())
		className = group->name();

	QRectF tmpViewport;

	for (Tiled::MapObject *object : std::as_const(group->objects())) {
		int idx = findLoadedObject(object->id(), scene->sceneId());

		if (idx != -1) {
			LOG_CERROR("game") << "Object already created" << object->id() << scene->sceneId();
			continue;
		}

		if (className == QStringLiteral("ground")) {
			loadGround(scene, object, renderer);
		} else if (className == QStringLiteral("dynamicZ")) {
			loadDynamicZ(scene, object, renderer);
		} else if (className == QStringLiteral("player")) {
			addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
			addLoadedObject(object->id(), scene->sceneId());
		} else if (className == QStringLiteral("viewport")) {
			if (object->name() == QStringLiteral("topLeft"))
				tmpViewport.setTopLeft(renderer->pixelToScreenCoords(object->position()));
			else if (object->name() == QStringLiteral("bottomRight"))
				tmpViewport.setBottomRight(renderer->pixelToScreenCoords(object->position()));
		} else if (className == QStringLiteral("transport")) {
			if (TiledTransport::typeFromString(object->className()) != TiledTransport::TransportInvalid) {
				loadTransport(scene, object, renderer);
			} else {
				LOG_CWARNING("game") << "Invalid transport object:" << object->id() << object->name();
			}
		} else if (group->className().isEmpty()) {
			if (object->className() == QStringLiteral("ground")) {
				loadGround(scene, object, renderer);
			} else if (object->className() == QStringLiteral("player")) {
				addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
				addLoadedObject(object->id(), scene->sceneId());
			} else if (object->className() == QStringLiteral("viewport")) {
				if (object->name() == QStringLiteral("topLeft"))
					tmpViewport.setTopLeft(renderer->pixelToScreenCoords(object->position()));
				else if (object->name() == QStringLiteral("bottomRight"))
					tmpViewport.setBottomRight(renderer->pixelToScreenCoords(object->position()));
			} else if (TiledTransport::typeFromString(object->className()) != TiledTransport::TransportInvalid) {
				loadTransport(scene, object, renderer);
			} else {
				loadObjectLayer(scene, object, className, renderer);
			}
		} else {
			loadObjectLayer(scene, object, className, renderer);
		}
	}

	if (!tmpViewport.isEmpty()) {
		LOG_CDEBUG("scene") << "Set viewport on scene" << scene->sceneId() << tmpViewport;
		scene->setViewport(tmpViewport);
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

TiledObjectBasePolygon *TiledGame::loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer, const QPointF &translate)
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
	mapObject->setGame(this);
	mapObject->fixture()->setDensity(1);
	mapObject->fixture()->setFriction(1);
	mapObject->fixture()->setRestitution(0);
	mapObject->fixture()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround));


	QPointF delta;

	if (!translate.isNull()) {
		mapObject->body()->emplace(mapObject->body()->bodyPosition() + translate);
		delta = renderer->screenToPixelCoords(renderer->tileToScreenCoords(0,0) + translate);
	}

	scene->m_groundObjects.append(QPointer(mapObject));

	if (!object->name().isEmpty()) {
		switch (object->shape()) {
			case Tiled::MapObject::Rectangle:
				scene->appendDynamicZ(object->name(), object->bounds().translated(delta));
				break;
			case Tiled::MapObject::Polygon:
				scene->appendDynamicZ(object->name(), object->polygon().translated(object->position()+delta).boundingRect());
				break;
			default:
				LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();
				break;
		}
	}

	if (object->hasProperty(QStringLiteral("z"))) {
		mapObject->setZ(object->property(QStringLiteral("z")).toInt());
	} else {
		mapObject->setZ(0);
	}

	if (object->hasProperty(QStringLiteral("transparent")))
		mapObject->body()->setOpaque(!object->property(QStringLiteral("transparent")).toBool());
	else if (object->hasProperty(QStringLiteral("opaque")))
		mapObject->body()->setOpaque(object->property(QStringLiteral("opaque")).toBool());

	addLoadedObject(object->id(), scene->sceneId());

	return mapObject;
}


/**
 * @brief TiledGame::loadDynamicZ
 * @param scene
 * @param object
 * @param renderer
 * @return
 */

bool TiledGame::loadDynamicZ(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(object);
	Q_ASSERT(renderer);

	if (object->name().isEmpty()) {
		LOG_CERROR("scene") << "Invalid Tiled::MapObject name" << object->id();
		return false;
	}

	if (object->className() == QStringLiteral("ground"))
		return loadGround(scene, object, renderer);

	QPolygonF polygon;


	switch (object->shape()) {
		case Tiled::MapObject::Rectangle:
			scene->appendDynamicZ(object->name(), object->bounds());
			break;
		case Tiled::MapObject::Polygon:
			scene->appendDynamicZ(object->name(), object->polygon().translated(object->position()).boundingRect());
			break;
		case Tiled::MapObject::Point:
			scene->appendDynamicZ(object->name(), QRectF{object->position(), object->position()});
			break;
		default:
			LOG_CERROR("scene") << "Invalid Tiled::MapObject shape" << object->shape();
			break;
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
	TiledObject::createFromMapObject<TiledObjectBasePolygon>(&mapObject, object, renderer, scene);

	if (!mapObject)
		return false;

	mapObject->setParent(scene);
	mapObject->setScene(scene);
	mapObject->fixture()->setSensor(true);
	mapObject->fixture()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport));

	if (!m_transportList.add(TiledTransport::typeFromString(object->className()), object->name(),
							 object->property(QStringLiteral("lock")).toString(),
							 object->hasProperty(QStringLiteral("direction")) ?
							 object->property(QStringLiteral("direction")).toInt() :
							 -1,
							 scene, mapObject))
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

void TiledGame::loadObjectLayer(TiledScene *, Tiled::MapObject *, const QString &, Tiled::MapRenderer *)
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
 * @brief TiledGame::loadImageLayer
 * @param scene
 * @param image
 * @param renderer
 */

void TiledGame::loadImageLayer(TiledScene *, Tiled::ImageLayer *, Tiled::MapRenderer *)
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

	switch (key) {
		case Qt::Key_Shift:
			m_keyboardJoystickState.shift = true;
			break;

		case Qt::Key_Left:
		case Qt::Key_4:
		case Qt::Key_A:
			m_keyboardJoystickState.left = true;
			break;

		case Qt::Key_Right:
		case Qt::Key_6:
		case Qt::Key_D:
			m_keyboardJoystickState.right = true;
			break;

		case Qt::Key_Up:
		case Qt::Key_8:
		case Qt::Key_W:
			m_keyboardJoystickState.up = true;
			break;

		case Qt::Key_Down:
		case Qt::Key_2:
		case Qt::Key_S:
			m_keyboardJoystickState.down = true;
			break;

		case Qt::Key_Home:
		case Qt::Key_7:
			m_keyboardJoystickState.upLeft = true;
			break;

		case Qt::Key_End:
		case Qt::Key_1:
			m_keyboardJoystickState.downLeft = true;
			break;

		case Qt::Key_PageUp:
		case Qt::Key_9:
			m_keyboardJoystickState.upRight = true;
			break;

		case Qt::Key_PageDown:
		case Qt::Key_3:
			m_keyboardJoystickState.downRight = true;
			break;

#ifndef QT_NO_DEBUG
		case Qt::Key_P:
			setDebugView(!m_debugView);
			break;
#endif
	}

	updateKeyboardJoystick();
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

	switch (key) {
		case Qt::Key_Shift:
			m_keyboardJoystickState.shift = false;
			break;

		case Qt::Key_Left:
		case Qt::Key_4:
		case Qt::Key_A:
			m_keyboardJoystickState.left = false;
			break;

		case Qt::Key_Right:
		case Qt::Key_6:
		case Qt::Key_D:
			m_keyboardJoystickState.right = false;
			break;

		case Qt::Key_Up:
		case Qt::Key_8:
		case Qt::Key_W:
			m_keyboardJoystickState.up = false;
			break;

		case Qt::Key_Down:
		case Qt::Key_2:
		case Qt::Key_S:
			m_keyboardJoystickState.down = false;
			break;

		case Qt::Key_Home:
		case Qt::Key_7:
			m_keyboardJoystickState.upLeft = false;
			break;

		case Qt::Key_End:
		case Qt::Key_1:
			m_keyboardJoystickState.downLeft = false;
			break;

		case Qt::Key_PageUp:
		case Qt::Key_9:
			m_keyboardJoystickState.upRight = false;
			break;

		case Qt::Key_PageDown:
		case Qt::Key_3:
			m_keyboardJoystickState.downRight = false;
			break;
	}

	updateKeyboardJoystick();
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
 * @brief TiledGame::transportMarket
 * @return
 */

bool TiledGame::transportMarket()
{
	emit marketRequest();
	return true;
}


/**
 * @brief TiledGame::transportGate
 * @param object
 * @param transport
 * @return
 */

bool TiledGame::transportGate(TiledObject *object, TiledTransport *transport, TiledObjectBase *transportBase)
{
	Q_ASSERT(object);
	Q_ASSERT(transport);
	Q_ASSERT(transportBase);

	TiledScene *oldScene = object->scene();
	TiledScene *newScene = transportBase ? transport->otherScene(transportBase) : transport->otherScene(oldScene);
	TiledObjectBase *newObject = transportBase ? transport->otherObject(transportBase) : transport->otherObject(oldScene);
	const int newDirection = transportBase ? transport->otherDirection(transportBase) : -1;

	if (!newScene || !newObject) {
		LOG_CERROR("game") << "Broken transport object";
		return false;
	}

	if (!transport->isOpen())
		return false;

	changeScene(object, oldScene, newScene, newObject->body()->bodyPosition());

	if (newDirection != -1)
		object->setCurrentDirection(object->nearestDirectionFromRadian(TiledObject::toRadian(newDirection)));

	if (!transportAfterEvent(object, newScene, newObject))
		return false;

	setCurrentScene(newScene);

	return true;
}


/**
 * @brief TiledGame::transportDoor
 * @param object
 * @param transport
 * @return
 */

bool TiledGame::transportDoor(TiledObject */*object*/, TiledTransport */*transport*/)
{
	return false;
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

void TiledGame::updateKeyboardJoystick()
{
	if (m_joystickState.hasTouch)
		return;

	JoystickState jState = m_joystickState;
	jState.hasKeyboard = m_keyboardJoystickState.up ||
						 m_keyboardJoystickState.down ||
						 m_keyboardJoystickState.left ||
						 m_keyboardJoystickState.right ||
						 m_keyboardJoystickState.upLeft ||
						 m_keyboardJoystickState.upRight ||
						 m_keyboardJoystickState.downLeft ||
						 m_keyboardJoystickState.downRight;

	qreal dx = 0.;
	qreal dy = 0.;

	if ((m_keyboardJoystickState.up && m_keyboardJoystickState.left) || m_keyboardJoystickState.upLeft) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::NorthWest);
		dx = -0.5;
		dy = -0.5;
	} else if ((m_keyboardJoystickState.up && m_keyboardJoystickState.right) || m_keyboardJoystickState.upRight) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::NorthEast);
		dx = 0.5;
		dy = -0.5;
	} else if ((m_keyboardJoystickState.down && m_keyboardJoystickState.left) || m_keyboardJoystickState.downLeft) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::SouthWest);
		dx = -0.5;
		dy = 0.5;
	} else if ((m_keyboardJoystickState.down && m_keyboardJoystickState.right) || m_keyboardJoystickState.downRight) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::SouthEast);
		dx = 0.5;
		dy = 0.5;
	} else if (m_keyboardJoystickState.up) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::North);
		dy = -0.5;
	} else if (m_keyboardJoystickState.down) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::South);
		dy = 0.5;
	} else if (m_keyboardJoystickState.left) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::West);
		dx = -0.5;
	} else if (m_keyboardJoystickState.right) {
		jState.angle = TiledObject::directionToIsometricRaidan(TiledObject::East);
		dx = 0.5;
	}


	if (m_keyboardJoystickState.shift && jState.hasKeyboard) {
		jState.distance = 0.7;
		dx *= 0.4;
		dy *= 0.4;
	} else if (jState.hasKeyboard) {
		jState.distance = 1.0;
	} else {
		jState.distance = 0.;
		dx = 0.;
		dy = 0.;
	}


	dx += 0.5;
	dy += 0.5;

	setJoystickState(jState);

	if (!m_joystick)
		return;

	QMetaObject::invokeMethod(m_joystick, "moveThumbRelative",
							  Q_ARG(QVariant, dx),
							  Q_ARG(QVariant, dy)
							  );
}

bool TiledGame::flickableInteractive() const
{
	return m_flickableInteractive;
}

void TiledGame::setFlickableInteractive(bool newFlickableInteractive)
{
	if (m_flickableInteractive == newFlickableInteractive)
		return;
	m_flickableInteractive = newFlickableInteractive;
	emit flickableInteractiveChanged();
}


/**
 * @brief TiledGame::mouseAttack
 * @return
 */

bool TiledGame::mouseAttack() const
{
	return m_mouseAttack;
}

void TiledGame::setMouseAttack(bool newMouseAttack)
{
	if (m_mouseAttack == newMouseAttack)
		return;
	m_mouseAttack = newMouseAttack;
	emit mouseAttackChanged();

	if (m_mouseAttack)
		setMouseNavigation(false);
}


/**
 * @brief TiledGame::mouseNavigation
 * @return
 */

bool TiledGame::mouseNavigation() const
{
	return m_mouseNavigation;
}

void TiledGame::setMouseNavigation(bool newMouseNavigation)
{
	if (m_mouseNavigation == newMouseNavigation)
		return;
	m_mouseNavigation = newMouseNavigation;
	emit mouseNavigationChanged();

	if (m_mouseNavigation)
		setMouseAttack(false);
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

	if (from == to) {
		object->body()->emplace(toPoint);
	} else {
		from->removeFromObjects(object);

		object->setScene(to);
		object->body()->emplace(toPoint);

		to->appendToObjects(object);
	}

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

TiledObjectBase *TiledGame::followedItem() const
{
	return m_followedItem;
}

void TiledGame::setFollowedItem(TiledObjectBase *newFollowedItem)
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
 * @brief TiledGame::transportList
 * @return
 */

TiledTransportList &TiledGame::transportList()
{
	return m_transportList;
}




/**
 * @brief TiledGame::transport
 * @param object
 * @param transport
 * @return
 */

bool TiledGame::transport(TiledObject *object, TiledTransport *transport, TiledObjectBase *transportBase)
{
	Q_ASSERT(object);

	if (!transport)
		return false;

	if (transport->type() == TiledTransport::TransportMarket) {
		return transportMarket();
	}

	if (!transportBeforeEvent(object, transport))
		return false;

	switch (transport->type()) {
		case TiledTransport::TransportGate:
			return transportGate(object, transport, transportBase);

		case TiledTransport::TransportDoor:
			return transportDoor(object, transport);

		case TiledTransport::TransportInvalid:
		case TiledTransport::TransportMarket:
			return false;
	}

	return true;
}



/**
 * @brief TiledGame::onMouseClick
 * @param x
 * @param y
 * @param modifiers
 */

void TiledGame::onMouseClick(const qreal &x, const qreal &y, const Qt::MouseButtons &buttons, const int &modifiers)
{
	Q_UNUSED(x);
	Q_UNUSED(y);
	Q_UNUSED(modifiers);
	Q_UNUSED(buttons);
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

	/*static const std::map<qreal, float> distanceMap = {
		{ 0, 1.0 },
		{ 20, 0.8 },
		{ 50, 0.7 },
		{ 75, 0.5 },
		{ 100, 0.4 },
		{ 120, 0.3 },
		{ 150, 0.2 }
	};

	const QRectF &rect = m_currentScene->onScreenArea();
	const qreal &scale = m_currentScene->scale() + (1.-m_baseScale);
	const qreal &factor = scale < 1.0 ? std::max(0.1, -1.+2.*scale)*baseVolume : baseVolume;

	for (const auto &[margin, volume] : std::as_const(distanceMap)) {
		if (rect.marginsAdded(QMarginsF{margin, margin, margin, margin}).contains(position)) {
			Application::instance()->client()->sound()->playSound(source, Sound::SfxChannel, volume*factor);
			return;
		}
	}*/

	if (const auto &ptr = getSfxVolume(scene, position, baseVolume, m_baseScale))
		Application::instance()->client()->sound()->playSound(source, Sound::SfxChannel, *ptr);

}



/**
 * @brief TiledGame::getSfxVolume
 * @param scene
 * @param position
 * @param baseVolume
 * @return
 */

std::optional<qreal> TiledGame::getSfxVolume(TiledScene *scene, const QPointF &position, const float &/*baseVolume*/, const qreal &/*baseScale*/)
{
	if (!scene)
		return std::nullopt;

	static const std::map<qreal, float> distanceMap = {
		{ 0, 1.0 },
		{ 20, 0.8 },
		{ 50, 0.7 },
		{ 75, 0.5 },
		{ 100, 0.4 },
		{ 120, 0.3 },
		{ 150, 0.2 }
	};

	const QRectF &rect = scene->onScreenArea();

	// DISABLE VOLUME DECREASE BY SCENE SCALE

	/*const qreal &scale = scene->scale() + (1.-baseScale);
	const qreal &factor = scale < 1.0 ? std::max(0.1, -1.+2.*scale)*baseVolume : baseVolume;*/
	static const qreal factor = 1.0;

	for (const auto &[margin, volume] : std::as_const(distanceMap)) {
		if (rect.marginsAdded(QMarginsF{margin, margin, margin, margin}).contains(position)) {
			return volume*factor;
		}
	}

	return std::nullopt;
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
 * @brief TiledGame::playerPositionsCount
 * @param sceneId
 * @return
 */

int TiledGame::playerPositionsCount(const int &sceneId) const
{
	int num = 0;

	for (const PlayerPosition &p : m_playerPositionList) {
		if (p.sceneId == sceneId)
			++num;
	}

	return num;
}


/**
 * @brief TiledGame::playerPositionsCount
 * @param scene
 * @return
 */

int TiledGame::playerPositionsCount(TiledScene *scene) const
{
	int num = 0;

	for (const PlayerPosition &p : m_playerPositionList) {
		if (p.scene == scene)
			++num;
	}

	return num;
}



/**
 * @brief TiledGame::playerPosition
 * @param sceneId
 * @param num
 * @return
 */

std::optional<QPointF> TiledGame::playerPosition(const int &sceneId, const int &num) const
{
	int i = 0;

	for (const PlayerPosition &p : m_playerPositionList) {
		if (p.sceneId == sceneId) {
			if (i==num)
				return p.position;
			else
				++i;
		}
	}

	return std::nullopt;
}




/**
 * @brief TiledGame::playerPosition
 * @param scene
 * @param num
 * @return
 */

std::optional<QPointF> TiledGame::playerPosition(TiledScene *scene, const int &num) const
{
	int i = 0;

	for (const PlayerPosition &p : m_playerPositionList) {
		if (p.scene == scene) {
			if (i==num)
				return p.position;
			else
				++i;
		}
	}

	return std::nullopt;
}


/**
 * @brief TiledGame::onSceneWorldStepped
 * @param scene
 */

void TiledGame::onSceneWorldStepped(TiledScene *scene)
{
	Q_UNUSED(scene);
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




/**
 * @brief TiledGame::loadTextureSprites
 * @param handler
 * @param mapper
 * @param path
 * @param layer
 * @return
 */

bool TiledGame::loadTextureSprites(TiledSpriteHandler *handler, const QVector<TextureSpriteMapper> &mapper,
								   const QString &path, const QString &layer)
{
	Q_ASSERT(handler);

	LOG_CTRACE("game") << "Load sprite texture" << path << layer;

	const auto &ptr = Utils::fileToJsonObject(
						  path.endsWith('/') ?
							  path+QStringLiteral("/texture.json") :
							  path+QStringLiteral(".json")
							  );

	if (!ptr)
		return false;

	TextureSpriteDef def;
	def.fromJson(*ptr);

	const auto &sprites = spritesFromMapper(mapper, def);

	return appendToSpriteHandler(handler, sprites,
								 path.endsWith('/') ?
									 path+QStringLiteral("/texture.png") :
									 path+QStringLiteral(".png"),
								 layer);
}



/**
 * @brief TiledGame::spriteFromMapper
 * @param mapper
 * @param def
 * @param name
 * @param direction
 * @param maxCount
 * @return
 */

TextureSprite TiledGame::spriteFromMapper(const QVector<TextureSpriteMapper> &mapper,
										  const TextureSpriteDef &def,
										  const QString &name,
										  const TiledObject::Direction &direction,
										  const int &maxCount)
{
	TextureSprite sprite;
	sprite.name = name;

	for (auto it = mapper.cbegin(); it != mapper.cend() && (maxCount == 0 || sprite.frames.size() < maxCount); ++it) {
		if (it->name != name || it->direction != direction)
			continue;

		const int frameNum = it - mapper.cbegin();

		const auto frameIt = def.frames.find(frameNum+1);				// Azért +1, mert a json frames számozása 1-től indul

		if (frameIt == def.frames.constEnd()) {
			LOG_CERROR("game") << "Invalid frame" << frameNum << it->name << it->direction;
			continue;
		}

		sprite.frames.append(*frameIt);
		sprite.duration = it->duration;
		sprite.loops = it->loops;
		sprite.size.w = it->width;
		sprite.size.h = it->height;
	}

	return sprite;
}


/**
 * @brief TiledGame::spritesFromMapper
 * @param mapper
 * @param def
 * @return
 */

QVector<TiledGame::TextureSpriteDirection> TiledGame::spritesFromMapper(const QVector<TextureSpriteMapper> &mapper, const TextureSpriteDef &def)
{
	QVector<TextureSpriteDirection> list;

	const QStringList sprites = spriteNamesFromMapper(mapper);

	for (const QString &s : sprites) {
		const QVector<TiledObject::Direction> directions = directionsFromMapper(mapper, s);

		for (const auto &d : directions) {
			TextureSpriteDirection data;
			data.sprite = spriteFromMapper(mapper, def, s, d);
			data.direction = d;
			list.append(data);
		}
	}

	return list;
}




/**
 * @brief TiledGame::spriteNamesFromMapper
 * @param mapper
 * @return
 */

QStringList TiledGame::spriteNamesFromMapper(const QVector<TextureSpriteMapper> &mapper)
{
	QStringList list;

	for (const auto &m : mapper) {
		if (!list.contains(m.name))
			list.append(m.name);
	}

	return list;
}



/**
 * @brief TiledGame::directionsFromMapper
 * @param mapper
 * @param name
 * @return
 */


QVector<TiledObject::Direction> TiledGame::directionsFromMapper(const QVector<TextureSpriteMapper> &mapper, const QString &name)
{
	QVector<TiledObject::Direction> list;

	for (const auto &m : mapper) {
		if (m.name == name && !list.contains(m.direction))
			list.append(m.direction);
	}

	return list;
}



/**
 * @brief TiledGame::appendToSpriteHandler
 * @param handler
 * @param sprites
 * @param source
 * @param layer
 * @return
 */

bool TiledGame::appendToSpriteHandler(TiledSpriteHandler *handler, const QVector<TextureSpriteDirection> &sprites, const QString &source,
									  const QString &layer)
{
	Q_ASSERT(handler);

	for (const auto &d : sprites) {
		if (!handler->addSprite(d.sprite, layer, d.direction, source))
			return false;
	}

	return true;
}



/**
 * @brief TiledGame::appendToSpriteHandler
 * @param handler
 * @param sprites
 * @param source
 * @param layer
 * @return
 */

bool TiledGame::appendToSpriteHandler(TiledSpriteHandler *handler, const QVector<TextureSprite> &sprites, const QString &source,
									  const QString &layer)
{
	Q_ASSERT(handler);

	for (const auto &s : sprites) {
		if (!handler->addSprite(s, layer, source))
			return false;
	}

	return true;
}

