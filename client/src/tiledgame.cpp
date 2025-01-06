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
#include <libtiled/tilesetmanager.h>
#include <enet/enet.h>


/**
 * @brief The TiledGamePrivate class
 */

class TiledGamePrivate
{
private:
	TiledGamePrivate(TiledGame *game)
		: q(game)
	{ }

	~TiledGamePrivate()
	{
		m_stepTimerRunning = false;

		if (m_stepTimerId != Qt::TimerId::Invalid)
			q->killTimer(m_stepTimerId);
	}

	struct Scene {
		Scene()
		{}

		Scene(Scene &&o) noexcept
			: container(o.container)
			, scene(o.scene)
			, world(std::move(o.world))
		{}

		QQuickItem *container = nullptr;
		TiledScene *scene = nullptr;
		std::unique_ptr<b2::World> world;
	};


	struct KeyboardJoystickState {
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;

		bool upLeft = false;
		bool upRight = false;
		bool downLeft = false;
		bool downRight = false;

		bool shift = false;


		void clear() {
			left = false;
			right = false;
			up = false;
			down = false;

			upLeft = false;
			upRight = false;
			downLeft = false;
			downRight = false;

			shift = false;
		}
	};


	template <typename T, typename = std::enable_if<std::is_base_of<TiledObjectBody, T>::value>::type>
	std::vector<T*> getObjects();

	template <typename T, typename = std::enable_if<std::is_base_of<TiledObjectBody, T>::value>::type>
	std::vector<T*> getObjects(TiledScene *scene);

	template <typename T, typename = std::enable_if<std::is_base_of<TiledObjectBody, T>::value>::type>
	std::vector<T*> getObjects(b2::World *world);

	TiledObjectBody *findObject(const TiledObjectBody::ObjectId &id);
	TiledObjectBody *findObject(const TiledObjectBody::ObjectId &id, const int &ownerId);
	TiledObjectBody *findObject(const int &sceneId, const int &id) {
		return findObject(TiledObjectBody::ObjectId{.sceneId = sceneId, .id = id});
	}
	TiledObjectBody *findObject(const int &sceneId, const int &id, const int &ownerId) {
		return findObject(TiledObjectBody::ObjectId{.sceneId = sceneId, .id = id}, ownerId);
	}

	TiledObjectBody *addObject(std::unique_ptr<TiledObjectBody> &body, const int &owner = -1);
	bool removeObject(TiledObjectBody *body);

	void updateObjects();
	void dumpObjects();



	void startStepTimer();
	void stepWorlds();


	TiledGame *const q;

	Qt::TimerId m_stepTimerId = Qt::TimerId::Invalid;
	bool m_stepTimerRunning = false;
	QElapsedTimer m_stepElapsedTimer;
	qint64 m_stepLag = 0;

	QLambdaThreadWorker m_stepTimerThread;
	QRecursiveMutex m_stepMutex;
	QMutex m_updateMutex;

	std::vector<Scene> m_sceneList;
	std::vector<TiledGame::Body> m_bodyList;

	std::vector<TiledGame::Body> m_addBodyList;
	std::vector<TiledObjectBody*> m_removeBodyList;

	int m_nextBodyId = 0;
	QVector<TiledGame::PlayerPosition> m_playerPositionList;
	TiledTransportList m_transportList;
	KeyboardJoystickState m_keyboardJoystickState;
	static std::unordered_map<QString, std::unique_ptr<QSGTexture>> m_sharedTextures;

	friend class TiledGame;
};


std::unordered_map<QString, std::unique_ptr<QSGTexture>> TiledGamePrivate::m_sharedTextures;



/**
 * @brief TiledGame::TiledGame
 * @param parent
 */

TiledGame::TiledGame(QQuickItem *parent)
	: QQuickItem(parent)
	, d(new TiledGamePrivate(this))
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

	m_tickTimer.reset(new AbstractGame::TickTimer);

	connect(this, &TiledGame::activeFocusChanged, this, [this](const bool &focus){
		if (!focus) {
			d->m_keyboardJoystickState.clear();
			updateKeyboardJoystick();
		}
	});
}


/**
 * @brief TiledGame::~TiledGame
 */

TiledGame::~TiledGame()
{
	for (const auto &s : std::as_const(d->m_sceneList)) {
		s.scene->stopMusic();
		if (s.scene->game() == this)
			s.scene->setGame(nullptr);
	}

	m_currentScene = nullptr;

	d->m_bodyList.clear();
	d->m_sceneList.clear();
	d->m_playerPositionList.clear();

	delete d;
	d = nullptr;

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

	d->startStepTimer();

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
	auto it = TiledGamePrivate::m_sharedTextures.find(path);

	if (it != TiledGamePrivate::m_sharedTextures.end())
		return it->second.get();

	if (!window) {
		LOG_CERROR("scene") << "Can't create texture:" << path;
		return nullptr;
	}

	QSGTexture *texture = window->createTextureFromImage(QImage(path));

	LOG_CTRACE("scene") << "Create texture from image:" << path << texture;

	std::unique_ptr<QSGTexture> s(texture);

	const auto &ptr = TiledGamePrivate::m_sharedTextures.insert({path, std::move(s)});

	Q_ASSERT(ptr.second);

	return ptr.first->second.get();
}


/**
 * @brief TiledGame::clearSharedTextures
 */

void TiledGame::clearSharedTextures()
{
	TiledGamePrivate::m_sharedTextures.clear();
}


/**
 * @brief TiledGame::sceneList
 * @return
 */

QVector<TiledScene *> TiledGame::sceneList() const
{
	QVector<TiledScene *> list;
	list.reserve(d->m_sceneList.size());

	for (const TiledGamePrivate::Scene &s : std::as_const(d->m_sceneList)) {
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
	auto it = std::find_if(d->m_sceneList.cbegin(), d->m_sceneList.cend(),
						   [&id](const TiledGamePrivate::Scene &s){
		return s.scene && s.scene->sceneId() == id;
	});

	if (it == d->m_sceneList.cend())
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

	TiledGamePrivate::Scene item;

	item.container = qobject_cast<QQuickItem*>(component.create());

	if (!item.container) {
		LOG_CERROR("game") << "Scene create error" << component.errorString();
		return false;
	}


	b2::World::Params params;
	params.gravity.x = 0.0f;
	params.gravity.y = 0.0f;

	item.world.reset(new b2::World(params));

	item.scene = qvariant_cast<TiledScene*>(item.container->property("scene"));
	Q_ASSERT(item.scene);

	item.world->SetUserData(item.scene);

	item.scene->setSceneId(def.id);
	item.scene->setGame(this);
	item.scene->m_world = item.world.get();

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


	d->m_sceneList.push_back(std::move(item));

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
		if (className == QStringLiteral("ground")) {
			loadGround(scene, object, renderer);
		} else if (className == QStringLiteral("dynamicZ")) {
			loadDynamicZ(scene, object, renderer);
		} else if (className == QStringLiteral("player")) {
			addPlayerPosition(scene, renderer->pixelToScreenCoords(object->position()));
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

TiledObjectBody *TiledGame::loadGround(TiledScene *scene, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
{
	Q_ASSERT(scene);
	Q_ASSERT(object);
	Q_ASSERT(renderer);

	b2::Shape::Params params;
	params.friction = 1.f;
	params.density = 1.f;
	params.restitution = 0.f;
	params.isSensor = false;
	params.enableSensorEvents = false;
	params.enableContactEvents = true;
	params.filter = TiledObjectBody::getFilter(TiledObjectBody::FixtureGround);

	TiledObjectBody *mapObject = createFromMapObject<TiledObjectBody>(scene, object, renderer, params);

	if (!mapObject)
		return nullptr;

	QPointF delta;

	if (Tiled::ObjectGroup *gLayer = object->objectGroup()) {
		delta = renderer->screenToPixelCoords(renderer->tileToScreenCoords(0,0) + gLayer->totalOffset());
	}


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

	/*if (object->hasProperty(QStringLiteral("z"))) {
		mapObject->setZ(object->property(QStringLiteral("z")).toInt());
	} else {
		mapObject->setZ(0);
	}*/

	if (object->hasProperty(QStringLiteral("transparent")))
		mapObject->setOpaque(!object->property(QStringLiteral("transparent")).toBool());
	else if (object->hasProperty(QStringLiteral("opaque")))
		mapObject->setOpaque(object->property(QStringLiteral("opaque")).toBool());

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
	/*TiledObjectBasePolygon *mapObject = nullptr;
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

	return true;*/

	return false;
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
 * @brief TiledGame::timerEvent
 * @param event
 */

void TiledGame::timerEvent(QTimerEvent */*event*/)
{
	updateStepTimer();
}


/**
 * @brief TiledGame::timeSteppedEvent
 */

void TiledGame::timeSteppedEvent()
{
	QMutexLocker locker(&d->m_stepMutex);

	for (const Body &ptr : std::as_const(d->m_bodyList)) {
		if (ptr.body->scene() == m_currentScene)
			ptr.body->synchronize();
	}

	for (const TiledGamePrivate::Scene &ptr : std::as_const(d->m_sceneList)) {
		ptr.scene->reorderObjectsZ(d->getObjects<TiledObject>(ptr.scene));
		emit ptr.scene->worldStepped();
	}
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
			d->m_keyboardJoystickState.shift = true;
			break;

		case Qt::Key_Left:
		case Qt::Key_4:
		case Qt::Key_A:
			d->m_keyboardJoystickState.left = true;
			break;

		case Qt::Key_Right:
		case Qt::Key_6:
		case Qt::Key_D:
			d->m_keyboardJoystickState.right = true;
			break;

		case Qt::Key_Up:
		case Qt::Key_8:
		case Qt::Key_W:
			d->m_keyboardJoystickState.up = true;
			break;

		case Qt::Key_Down:
		case Qt::Key_2:
		case Qt::Key_S:
			d->m_keyboardJoystickState.down = true;
			break;

		case Qt::Key_Home:
		case Qt::Key_7:
			d->m_keyboardJoystickState.upLeft = true;
			break;

		case Qt::Key_End:
		case Qt::Key_1:
			d->m_keyboardJoystickState.downLeft = true;
			break;

		case Qt::Key_PageUp:
		case Qt::Key_9:
			d->m_keyboardJoystickState.upRight = true;
			break;

		case Qt::Key_PageDown:
		case Qt::Key_3:
			d->m_keyboardJoystickState.downRight = true;
			break;

#ifndef QT_NO_DEBUG
		case Qt::Key_P:
			setDebugView(!m_debugView);
			break;

		case Qt::Key_F11:
			d->dumpObjects();
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
			d->m_keyboardJoystickState.shift = false;
			break;

		case Qt::Key_Left:
		case Qt::Key_4:
		case Qt::Key_A:
			d->m_keyboardJoystickState.left = false;
			break;

		case Qt::Key_Right:
		case Qt::Key_6:
		case Qt::Key_D:
			d->m_keyboardJoystickState.right = false;
			break;

		case Qt::Key_Up:
		case Qt::Key_8:
		case Qt::Key_W:
			d->m_keyboardJoystickState.up = false;
			break;

		case Qt::Key_Down:
		case Qt::Key_2:
		case Qt::Key_S:
			d->m_keyboardJoystickState.down = false;
			break;

		case Qt::Key_Home:
		case Qt::Key_7:
			d->m_keyboardJoystickState.upLeft = false;
			break;

		case Qt::Key_End:
		case Qt::Key_1:
			d->m_keyboardJoystickState.downLeft = false;
			break;

		case Qt::Key_PageUp:
		case Qt::Key_9:
			d->m_keyboardJoystickState.upRight = false;
			break;

		case Qt::Key_PageDown:
		case Qt::Key_3:
			d->m_keyboardJoystickState.downRight = false;
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

bool TiledGame::transportAfterEvent(TiledObject */*object*/, TiledScene */*newScene*/, TiledObject */*newObject*/)
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

bool TiledGame::transportGate(TiledObject *object, TiledTransport *transport, TiledObject *transportBase)
{
	Q_ASSERT(object);
	Q_ASSERT(transport);
	Q_ASSERT(transportBase);

	TiledScene *oldScene = object->scene();
	TiledScene *newScene = transportBase ? transport->otherScene(transportBase) : transport->otherScene(oldScene);
	TiledObject *newObject = transportBase ? transport->otherObject(transportBase) : transport->otherObject(oldScene);
	const int newDirection = transportBase ? transport->otherDirection(transportBase) : -1;

	if (!newScene || !newObject) {
		LOG_CERROR("game") << "Broken transport object";
		return false;
	}

	if (!transport->isOpen())
		return false;

	changeScene(object, newScene, newObject->bodyPosition());

	if (newDirection != -1)
		object->setFacingDirection(object->nearestDirectionFromRadian(TiledObject::toRadian(newDirection)));

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
 * @brief TiledGame::sceneDebugDrawEvent
 */

void TiledGame::sceneDebugDrawEvent(TiledDebugDraw *debugDraw, TiledScene *scene)
{
	if (!scene || !debugDraw)
		return;

	QMutexLocker locker(&d->m_stepMutex);

	for (const auto &ptr : d->m_bodyList) {
		TiledObjectBody *b = ptr.body.get();
		if (b->scene() == scene)
			b->debugDraw(debugDraw);
	}
}


/**
 * @brief TiledGame::bodyList
 * @return
 */

const std::vector<TiledGame::Body> &TiledGame::bodyList() const
{
	return d->m_bodyList;
}


/**
 * @brief TiledGame::bodyList
 * @return
 */

std::vector<TiledGame::Body> &TiledGame::bodyList()
{
	return d->m_bodyList;
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
	jState.hasKeyboard = d->m_keyboardJoystickState.up ||
						 d->m_keyboardJoystickState.down ||
						 d->m_keyboardJoystickState.left ||
						 d->m_keyboardJoystickState.right ||
						 d->m_keyboardJoystickState.upLeft ||
						 d->m_keyboardJoystickState.upRight ||
						 d->m_keyboardJoystickState.downLeft ||
						 d->m_keyboardJoystickState.downRight;

	qreal dx = 0.;
	qreal dy = 0.;

	if ((d->m_keyboardJoystickState.up && d->m_keyboardJoystickState.left) || d->m_keyboardJoystickState.upLeft) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::NorthWest);
		dx = -0.5;
		dy = -0.5;
	} else if ((d->m_keyboardJoystickState.up && d->m_keyboardJoystickState.right) || d->m_keyboardJoystickState.upRight) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::NorthEast);
		dx = 0.5;
		dy = -0.5;
	} else if ((d->m_keyboardJoystickState.down && d->m_keyboardJoystickState.left) || d->m_keyboardJoystickState.downLeft) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::SouthWest);
		dx = -0.5;
		dy = 0.5;
	} else if ((d->m_keyboardJoystickState.down && d->m_keyboardJoystickState.right) || d->m_keyboardJoystickState.downRight) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::SouthEast);
		dx = 0.5;
		dy = 0.5;
	} else if (d->m_keyboardJoystickState.up) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::North);
		dy = -0.5;
	} else if (d->m_keyboardJoystickState.down) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::South);
		dy = 0.5;
	} else if (d->m_keyboardJoystickState.left) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::West);
		dx = -0.5;
	} else if (d->m_keyboardJoystickState.right) {
		jState.angle = TiledObject::directionToIsometricRadian(TiledObject::East);
		dx = 0.5;
	}


	if (d->m_keyboardJoystickState.shift && jState.hasKeyboard) {
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



/**
 * @brief TiledGame::updateStepTimer
 */

void TiledGame::updateStepTimer()
{
	if (m_paused) {
		d->m_stepElapsedTimer.invalidate();
		d->m_stepLag = 0;
		return;
	}

	if (d->m_stepElapsedTimer.isValid()) {
		d->m_stepLag += d->m_stepElapsedTimer.restart();
	} else {
		d->m_stepElapsedTimer.start();
	}

	int frame = 0;

	if (d->m_stepLag < 1000/60)
		return;

	while (d->m_stepLag >= 1000/60) {
		d->m_stepLag -= 1000/60;

		if (m_funcBeforeWorldStep)
			m_funcBeforeWorldStep();

		d->stepWorlds();

		if (m_funcAfterWorldStep)
			m_funcAfterWorldStep();

		++frame;
	}

	if (frame > 1)
		LOG_CTRACE("scene") << "Render lag:" << frame << "frames";

	timeSteppedEvent();
}


/**
 * @brief TiledGame::addObject
 * @param bodyPtr
 */

TiledObjectBody *TiledGame::addObject(std::unique_ptr<TiledObjectBody> &body, const int &id, const int &owner)
{
	Q_ASSERT(body);

	if (id < 0 || (id == 0 && owner != 0)) {
		LOG_CERROR("scene") << "Invalid object id" << owner << body->scene()->sceneId() << id;
		return nullptr;
	}

	QMutexLocker locker(&d->m_stepMutex);

	int newId = id;
	if (id == 0 && owner == 0)
		newId = ++d->m_nextBodyId;

	if (d->findObject(body->scene()->sceneId(), newId, owner)) {
		LOG_CERROR("scene") << "Object already exists" << owner << body->scene()->sceneId() << newId;
		return nullptr;
	}

	body->setObjectId(body->scene()->sceneId(), newId);

	return d->addObject(body, owner);
}


/**
 * @brief TiledGame::removeObject
 * @param body
 * @return
 */

bool TiledGame::removeObject(TiledObjectBody *body)
{
	QMutexLocker locker(&d->m_stepMutex);

	return d->removeObject(body);
}



/**
 * @brief TiledGame::flickableInteractive
 * @return
 */

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
 * @brief TiledGame::changeScene
 * @param object
 * @param from
 * @param to
 */

void TiledGame::changeScene(TiledObjectBody *object, TiledScene *to, const QPointF &toPoint)
{
	Q_ASSERT(object);
	Q_ASSERT(to);

	if (object->scene() == to) {
		object->emplace(toPoint);
	} else {
		object->setWorld(to->world(), toPoint);
	}

	IsometricEntity *entity = dynamic_cast<IsometricEntity*>(object);

	if (entity)
		entity->updateSprite();
}


/**
 * @brief TiledGame::worldStep
 * @param body
 */

void TiledGame::worldStep(TiledObjectBody *body)
{
	Q_ASSERT(body);
	body->worldStep();
}



/**
 * @brief TiledGame::addPlayerPosition
 * @param scene
 * @param position
 */

void TiledGame::addPlayerPosition(TiledScene *scene, const QPointF &position)
{
	auto it = std::find_if(d->m_playerPositionList.constBegin(), d->m_playerPositionList.constEnd(),
						   [scene, &position](const auto &p){
		return p.scene == scene && p.position == position;
	});

	if (it != d->m_playerPositionList.constEnd()) {
		LOG_CWARNING("game") << "Player position already loaded:" << it->sceneId << it->position;
		return;
	}

	d->m_playerPositionList.append(PlayerPosition{
									   scene ? scene->sceneId() : -1,
									   scene,
									   position
								   });
}




/**
 * @brief TiledGame::isGround
 * @param x
 * @param y
 * @return
 */

bool TiledGame::isGround(const TiledScene *scene, const qreal &x, const qreal &y) const
{
	QMutexLocker locker(&d->m_stepMutex);

	for (const auto &ptr : d->m_bodyList) {
		TiledObjectBody *b = ptr.body.get();
		if (!b || b->scene() != scene || !b->isBodyEnabled())
			continue;

		for (const b2::ShapeRef &sh : b->bodyShapes()) {
			if (!(sh.GetFilter().categoryBits & TiledObjectBody::FixtureGround))
				continue;

			if (sh.TestPoint(b2Vec2(x, y)))
				return true;
		}
	}

	return false;
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
	return d->m_transportList;
}


/**
 * @brief TiledGame::transportList
 * @return
 */

TiledTransportList &TiledGame::transportList()
{
	return d->m_transportList;
}




/**
 * @brief TiledGame::transport
 * @param object
 * @param transport
 * @return
 */

bool TiledGame::transport(TiledObject *object, TiledTransport *transport, TiledObject *transportBase)
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

void TiledGame::onMouseClick(const qreal &x, const qreal &y, const int &buttons, const int &modifiers)
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
 * @brief TiledGame::playerPositionList
 * @return
 */

const QVector<TiledGame::PlayerPosition> &TiledGame::playerPositionList() const
{
	return d->m_playerPositionList;
}



/**
 * @brief TiledGame::playerPositionsCount
 * @param sceneId
 * @return
 */

int TiledGame::playerPositionsCount(const int &sceneId) const
{
	int num = 0;

	for (const auto &p : d->m_playerPositionList) {
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

	for (const auto &p : d->m_playerPositionList) {
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

	for (const auto &p : d->m_playerPositionList) {
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

	for (const auto &p : d->m_playerPositionList) {
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



/**
 * @brief TiledGame::paused
 * @return
 */

bool TiledGame::paused() const
{
	return m_paused;
}


/**
 * @brief TiledGame::setPaused
 * @param newPaused
 */

void TiledGame::setPaused(bool newPaused)
{
	if (m_paused == newPaused)
		return;
	m_paused = newPaused;
	emit pausedChanged();


	if (m_tickTimer) {
		if (m_paused)
			m_tickTimer->pause();
		else
			m_tickTimer->resume();
	}

	/*for (const Scene &s : m_sceneList)
		s.scene->setRunning(!m_paused);*/

	Tiled::TilesetManager *manager = Tiled::TilesetManager::instance();
	manager->setAnimateTiles(!m_paused);
}



/**
 * @brief TiledGamePrivate::getObjects
 * @return
 */

template<typename T, typename T2>
std::vector<T *> TiledGamePrivate::getObjects()
{
	QMutexLocker locker(&m_stepMutex);

	std::vector<T *> list;

	for (const auto &ptr : m_bodyList) {
		if (T* o = dynamic_cast<T*>(ptr.body.get()))
			list.push_back(o);
	}

	return list;
}


/**
 * @brief TiledGamePrivate::getObjects
 * @param scene
 * @return
 */

template<typename T, typename T2>
std::vector<T *> TiledGamePrivate::getObjects(TiledScene *scene)
{
	std::vector<T *> list;

	if (!scene)
		return list;

	QMutexLocker locker(&m_stepMutex);

	for (const auto &ptr : m_bodyList) {
		if (T* o = dynamic_cast<T*>(ptr.body.get())) {
			if (o->scene() == scene)
				list.push_back(o);
		}
	}

	return list;
}


/**
 * @brief TiledGamePrivate::getObjects
 * @param world
 * @return
 */

template<typename T, typename T2>
std::vector<T *> TiledGamePrivate::getObjects(b2::World *world)
{
	std::vector<T *> list;

	if (!world)
		return list;

	QMutexLocker locker(&m_stepMutex);

	for (const auto &ptr : m_bodyList) {
		if (T* o = dynamic_cast<T*>(ptr.body.get())) {
			if (o->world() == world)
				list.push_back(o);
		}
	}

	return list;
}




/**
 * @brief TiledGamePrivate::startStepTimer
 */

TiledObjectBody* TiledGamePrivate::findObject(const TiledObjectBody::ObjectId &id)
{
	QMutexLocker l(&m_stepMutex);

	for (const TiledGame::Body &b : m_bodyList) {
		if (b.body && b.body->objectId() == id)
			return b.body.get();
	}

	return nullptr;
}


/**
 * @brief TiledGamePrivate::findObject
 * @param id
 * @param ownerId
 * @return
 */

TiledObjectBody *TiledGamePrivate::findObject(const TiledObjectBody::ObjectId &id, const int &ownerId)
{
	QMutexLocker l(&m_stepMutex);

	for (const TiledGame::Body &b : m_bodyList) {
		if (b.owner == ownerId && b.body && b.body->objectId() == id)
			return b.body.get();
	}

	return nullptr;
}


/**
 * @brief TiledGamePrivate::addObject
 * @param body
 * @param id
 * @param owner
 * @return
 */

TiledObjectBody *TiledGamePrivate::addObject(std::unique_ptr<TiledObjectBody> &body, const int &owner)
{
	QMutexLocker locker(&m_updateMutex);
	TiledObjectBody *ptr = m_addBodyList.emplace_back(std::move(body), owner).body.get();

	locker.unlock();

	if (m_stepTimerId == Qt::TimerId::Invalid)
		updateObjects();

	return ptr;
}




/**
 * @brief TiledGamePrivate::removeObject
 * @param body
 * @return
 */

bool TiledGamePrivate::removeObject(TiledObjectBody *body)
{
	QMutexLocker locker(&m_updateMutex);

	bool ret = m_bodyList.cend() != std::find_if(m_bodyList.cbegin(), m_bodyList.cend(), [body](const TiledGame::Body &b) { return b.body.get() == body; } );

	if (!ret)
		ret = m_addBodyList.cend() != std::find_if(m_addBodyList.cbegin(), m_addBodyList.cend(), [body](const TiledGame::Body &b) { return b.body.get() == body; } );

	if (ret)
		m_removeBodyList.push_back(body);

	locker.unlock();

	if (m_stepTimerId == Qt::TimerId::Invalid)
		updateObjects();

	return ret;
}


/**
 * @brief TiledGamePrivate::updateObjects
 */

void TiledGamePrivate::updateObjects()
{
	QMutexLocker locker(&m_updateMutex);

	if (!m_addBodyList.empty()) {
		for (auto &ptr : m_addBodyList)
			m_bodyList.push_back(std::move(ptr));

		m_addBodyList.clear();
	}

	if (!m_removeBodyList.empty()) {
		for (auto it = m_bodyList.begin(); it != m_bodyList.end(); ) {
			if (std::find(m_removeBodyList.cbegin(), m_removeBodyList.cend(), it->body.get()) != m_removeBodyList.cend()) {
				if (dynamic_cast<QObject*>(it->body.get()))
					dynamic_cast<QObject*>(it->body.release())->deleteLater();

				it = m_bodyList.erase(it);
			} else
				++it;
		}

		m_removeBodyList.clear();
	}
}




/**
 * @brief TiledGamePrivate::dumpObjects
 */

void TiledGamePrivate::dumpObjects()
{
	QMutexLocker locker(&m_stepMutex);

	LOG_CINFO("game") << "--------------------------------------------------------------";
	LOG_CINFO("game") << "*** DUMP OBJECTS ***";
	LOG_CINFO("game") << "--------------------------------------------------------------";

	for (const TiledGame::Body &b : m_bodyList) {
		if (b.body)
			LOG_CINFO("game") << b.owner << b.body->objectId().sceneId << b.body->objectId().id
							  << dynamic_cast<QObject*>(b.body.get())
							  << (b.body->bodyShapes().empty() ? 0 : b.body->bodyShapes().front().GetFilter().categoryBits);
		else
			LOG_CERROR("game") << b.owner << "BAD BODY";
	}

	LOG_CINFO("game") << "--------------------------------------------------------------";

}






void TiledGamePrivate::startStepTimer()
{
	m_stepElapsedTimer.start();
	m_stepTimerId = Qt::TimerId{q->startTimer(std::chrono::milliseconds{8}, Qt::PreciseTimer)};


	/*
	m_stepTimerRunning = true;

#ifndef Q_OS_WASM
	m_stepTimerThread.execInThread([this](){
#endif
		LOG_CTRACE("game") << "Step timer started";

		int lag = 0;
		std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now();

		long longestTime = 0;
		long frameCount = 0;
		long tickCount = 0;
		long fpsElapsedTime = 0;

		while (m_stepTimerRunning) {

			std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();

			std::chrono::milliseconds elapsedTime(std::chrono::duration_cast<std::chrono::milliseconds>(startTime - endTime));
			endTime = startTime;

			lag += static_cast<int>(elapsedTime.count());

			if (elapsedTime.count() == 0) { //the sim loop will not be run if lag was not increased prior ( previous frame normally )
				QThread::currentThread()->sleep(std::chrono::milliseconds{5});

				if (!m_stepTimerRunning)
					break;
			}

			const int fps = 60;
			const int lengthOfFrame = 1000 / fps;


			//game-sim loop, is independent of rendering
			std::chrono::time_point<std::chrono::steady_clock> oldTime = std::chrono::steady_clock::now();
			while (lag >= lengthOfFrame) {
				q->updateStepTimer();

				//finish tick
				lag -= lengthOfFrame;
				tickCount++;	//keep track of how many ticks weve done this second
			}

			//render the frame

			frameCount++;	//keep trrack of how many frames weve done this second

			//handle per-second stats
			fpsElapsedTime += static_cast<long>(elapsedTime.count());
			if (fpsElapsedTime >= 1000) {
				LOG_CTRACE("scene") <<  frameCount << ", " << tickCount;

				fpsElapsedTime = 0;
				frameCount = 0;
				tickCount = 0;
			}
		}

		LOG_CERROR("game") << "Step timer finished";
	});
	*/
}



/**
 * @brief TiledGamePrivate::stepWorlds
 */

void TiledGamePrivate::stepWorlds()
{
	QMutexLocker locker(&m_stepMutex);

	for (const Scene &ptr : std::as_const(m_sceneList)) {
		ptr.world->Step(1/60., 4);

		const b2SensorEvents sensors = ptr.world->GetSensorEvents();
		const b2ContactEvents events = ptr.world->GetContactEvents();

		for (int i=0; i<sensors.beginCount; ++i) {
			const b2SensorBeginTouchEvent &event = sensors.beginEvents[i];

			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.sensorShapeId).GetBody()))
				b->onShapeContactBegin(event.sensorShapeId, event.visitorShapeId);
			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.visitorShapeId).GetBody()))
				b->onShapeContactBegin(event.visitorShapeId, event.sensorShapeId);
		}

		for (int i=0; i<sensors.endCount; ++i) {
			const b2SensorEndTouchEvent &event = sensors.endEvents[i];
			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.sensorShapeId).GetBody()))
				b->onShapeContactEnd(event.sensorShapeId, event.visitorShapeId);
			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.visitorShapeId).GetBody()))
				b->onShapeContactEnd(event.visitorShapeId, event.sensorShapeId);
		}

		for (int i=0; i<events.beginCount; ++i) {
			const b2ContactBeginTouchEvent &event = events.beginEvents[i];

			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.shapeIdA).GetBody()))
				b->onShapeContactBegin(event.shapeIdA, event.shapeIdB);
			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.shapeIdB).GetBody()))
				b->onShapeContactBegin(event.shapeIdB, event.shapeIdA);
		}

		for (int i=0; i<events.endCount; ++i) {
			const b2ContactEndTouchEvent &event = events.endEvents[i];
			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.shapeIdA).GetBody()))
				b->onShapeContactEnd(event.shapeIdA, event.shapeIdB);
			if (TiledObjectBody *b = TiledObjectBody::fromBodyRef(b2::ShapeRef(event.shapeIdB).GetBody()))
				b->onShapeContactEnd(event.shapeIdB, event.shapeIdA);
		}
	}

	for (const TiledGame::Body &ptr : std::as_const(m_bodyList)) {
		if (ptr.body)
			q->worldStep(ptr.body.get());
	}

	updateObjects();
}
