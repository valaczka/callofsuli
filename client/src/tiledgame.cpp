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
#include "tiledobject.h"
#include "Logger.h"
#include "application.h"
#include "isometricentity.h"
#include "rpgenemyiface.h"
#include "tiledspritehandler.h"
#include "tileddebugdraw.h"
#include "utils_.h"
#include "libtcod/path.hpp"
#include <libtiled/objectgroup.h>
#include <libtiled/mapreader.h>
#include <libtiled/map.h>
#include <libtiled/imagecache.h>
#include <libtiled/tilesetmanager.h>
#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_structs.h>


#ifndef Q_OS_WASM
#include <enet/enet.h>
#endif

typedef std::unique_ptr<cpSpace, void (*)(cpSpace*)> unique_space_ptr;

/**
 * @brief The TiledGamePrivate class
 */

class TiledGamePrivate
{
private:
	TiledGamePrivate(TiledGame *game)
		: q(game)
	{ }

	struct Scene {
		Scene()
			: space(nullptr, &Scene::spaceDestroy)
		{}

		Scene(Scene &&o) noexcept
			: container(o.container)
			, scene(o.scene)
			, space(std::move(o.space))
		{}

		~Scene() { destroyScene(); }

		struct TcodMapData {
			std::unique_ptr<TCODMap> map;
			QRectF viewport;
			qreal chunkWidth = 0.;
			qreal chunkHeight = 0.;

			std::optional<QPolygonF> findShortestPath(const cpVect &from, const cpVect &to) const;
			std::optional<QPolygonF> findShortestPath(const qreal &x1, const qreal &y1, const qreal &x2, const qreal &y2) const;
		};

		static unique_space_ptr spaceCreate() {
			return unique_space_ptr(cpSpaceNew(), &Scene::spaceDestroy);
		}
		static void spaceDestroy(cpSpace *space);

		void reloadTcodMap();
		void destroyScene();

		int sceneId = -1;
		QQuickItem *container = nullptr;
		TiledScene *scene = nullptr;
		unique_space_ptr space;
		TcodMapData tcodMap;
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
	std::vector<T*> getObjects(cpSpace *world);

	TiledObjectBody *findObject(const TiledObjectBody::ObjectId &id);
	TiledObjectBody *findObject(const int &ownerId, const int &sceneId, const int &id) {
		return findObject(TiledObjectBody::ObjectId{.ownerId = ownerId, .sceneId = sceneId, .id = id});
	}

	TiledObjectBody *addObject(std::unique_ptr<TiledObjectBody> &body);
	bool removeObject(TiledObjectBody *body);

	void updateObjects();

	void stepWorlds();

	static cpBool collisionBegin(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
	static void collisionEnd(cpArbiter *arb, cpSpace *space, cpDataPointer userData);


	TiledGame *const q;

	qint64 m_currentFrame = 0;
	QRecursiveMutex m_stepMutex;

	std::vector<std::unique_ptr<Scene>> m_sceneList;
	std::vector<std::unique_ptr<TiledObjectBody> > m_bodyList;

	std::vector<TiledObjectBody*> m_removeBodyList;

	int m_nextBodyId = 0;
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
	for (const auto &s : d->m_sceneList) {
		s->scene->stopMusic();
		if (s->scene->game() == this)
			s->scene->setGame(nullptr);

		s->scene->m_space = nullptr;
		s->scene->m_sceneId = -1;
	}

	m_currentScene = nullptr;


	for (const auto &b : d->m_bodyList) {
		b->deleteBody();
	}

	d->m_bodyList.clear();
	d->m_sceneList.clear();

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

	startTimer(std::chrono::milliseconds{8}, Qt::PreciseTimer);

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

	for (const auto &s : std::as_const(d->m_sceneList)) {
		if (s->scene)
			list.append(s->scene);
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
						   [&id](const auto &s){
		return s->scene && s->scene->sceneId() == id;
	});

	if (it == d->m_sceneList.cend())
		return nullptr;

	return it->get()->scene;
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

	std::unique_ptr<TiledGamePrivate::Scene> item = std::make_unique<TiledGamePrivate::Scene>();

	item->container = qobject_cast<QQuickItem*>(component.create());

	if (!item->container) {
		LOG_CERROR("game") << "Scene create error" << component.errorString();
		return false;
	}


	item->sceneId = def.id;
	item->space = item->spaceCreate();

	cpSpaceSetCollisionBias(item->space.get(), cpfpow(1.0f - 0.5f, 60.0f));

	item->scene = qvariant_cast<TiledScene*>(item->container->property("scene"));
	Q_ASSERT(item->scene);

	item->scene->setGame(this);
	item->scene->m_space = item->space.get();
	item->scene->m_sceneId = item->sceneId;
	cpSpaceSetUserData(item->space.get(), item->scene);

	item->container->setParentItem(this);
	item->container->setParent(this);

	if (!item->scene->load(QUrl(basePath+'/'+def.file))) {
		LOG_CERROR("game") << "Scene load error" << def.file;
		item->container->deleteLater();
		return false;
	}

	item->reloadTcodMap();

	item->scene->setAmbientSound(def.ambient);
	item->scene->setBackgroundMusic(def.music);
	item->scene->setSceneEffect(def.effect);



	cpCollisionHandler *handler = cpSpaceAddDefaultCollisionHandler(item->space.get());
	handler->beginFunc = &TiledGamePrivate::collisionBegin;
	handler->separateFunc = &TiledGamePrivate::collisionEnd;
	handler->userData = d;

	QMutexLocker locker(&d->m_stepMutex);

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

	TiledObjectBody *mapObject = createObject<TiledObjectBody>(-1, scene, object->id(),
															   object, this, renderer, CP_BODY_TYPE_STATIC);

	if (!mapObject)
		return nullptr;

	mapObject->filterSet(TiledObjectBody::FixtureGround);

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
 * @brief TiledGame::synchronize
 */

void TiledGame::synchronize()
{
	QMutexLocker locker(&d->m_stepMutex);

	for (const auto &ptr : std::as_const(d->m_bodyList)) {
		if (ptr->scene() == m_currentScene)
			ptr->synchronize();
	}

	locker.unlock();

	for (const auto &ptr : std::as_const(d->m_sceneList)) {
		ptr->scene->reorderObjectsZ(d->getObjects<TiledObject>(ptr->scene));

		if (ptr->scene->m_debugDraw)
			ptr->scene->m_debugDraw->update();
	}
}



/**
 * @brief TiledGame::timeSteppedEvent
 */

void TiledGame::timeSteppedEvent()
{

}

void TiledGame::timeStepPrepareEvent()
{

}

void TiledGame::timeBeforeWorldStepEvent(const qint64 &tick)
{
	Q_UNUSED(tick);
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

	///changeScene(object, newScene, newObject->bodyPosition());

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

	for (const auto &ptr : std::as_const(d->m_bodyList)) {
		if (ptr->scene() == scene)
			ptr->debugDraw(debugDraw);
	}
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
	if (m_paused || !m_tickTimer)
		return;

	const qint64 &currentTick = m_tickTimer->currentTick();

	if (currentTick < 0) {
		timeStepPrepareEvent();
		return;
	}

	const qint64 frames = currentTick - d->m_currentFrame;

	if (frames <= 0)
		return;


	QMutexLocker locker(&d->m_stepMutex);

	timeStepPrepareEvent();

	for (qint64 tick=currentTick-frames+1; tick<=currentTick; ++tick) {
		timeBeforeWorldStepEvent(tick);

		d->stepWorlds();

		timeAfterWorldStepEvent(tick);
	}

	d->m_currentFrame = currentTick;

	if (frames > 12)
		LOG_CERROR("scene") << "Render lag:" << frames << "frames";
	else if (frames > 8)
		LOG_CWARNING("scene") << "Render lag:" << frames << "frames";
	else if (frames > 2)
		LOG_CDEBUG("scene") << "Render lag:" << frames << "frames";
	else if (frames > 1)
		LOG_CTRACE("scene") << "Render lag:" << frames << "frames";


	timeSteppedEvent();

	d->updateObjects();

	synchronize();
}


/**
 * @brief TiledGame::addObject
 * @param bodyPtr
 */

TiledObjectBody *TiledGame::addObject(std::unique_ptr<TiledObjectBody> &body, const int &sceneId, const int &id, const int &owner)
{
	Q_ASSERT(body);

	if (id < 0 || (id == 0 && owner != 0)) {
		LOG_CERROR("scene") << "Invalid object id" << owner << sceneId << id;
		return nullptr;
	}

	QMutexLocker locker(&d->m_stepMutex);

	int newId = id;
	if (id == 0 && owner == 0)
		newId = ++d->m_nextBodyId;

	if (d->findObject(sceneId, newId, owner)) {
		LOG_CERROR("scene") << "Object already exists" << owner << sceneId << newId;
		return nullptr;
	}

	body->setObjectId(owner, sceneId, newId);

	return d->addObject(body);
}




/**
 * @brief TiledGame::initSpace
 * @param body
 * @param scene
 * @return
 */

bool TiledGame::initSpace(TiledObjectBody *body, TiledScene *scene)
{
	if (!body || !scene)
		return false;

	cpSpace *newSpace = scene->m_space;

	Q_ASSERT(newSpace);

	QMutexLocker locker(&d->m_stepMutex);

	if (cpSpaceIsLocked(newSpace)) {
		LOG_CERROR("scene") << "Space locked" << this;
		return false;
	}

	body->setSpace(newSpace);

	return true;
}




/**
 * @brief TiledGame::changeSpace
 * @param body
 * @param newSpace
 * @return
 */

bool TiledGame::changeSpace(TiledObjectBody *body, cpSpace *newSpace)
{
	if (!body)
		return false;

	cpSpace *old = body->space();

	if (old == newSpace)
		return true;

	QMutexLocker locker(&d->m_stepMutex);

	if (old && cpSpaceIsLocked(old)) {
		LOG_CERROR("scene") << "Space locked" << this;
		return false;
	}


	if (!newSpace) {
		LOG_CTRACE("scene") << "Remove body" << this;
		body->deleteBody();
		return true;
	}

	if (old)
		cpSpaceRemoveBody(old, body->body());

	cpSpaceAddBody(newSpace, body->body());

	static const auto fn = [](cpBody *body, cpShape *shape, void *) {
		LOG_CERROR("scene") << "ADD shape" << shape << body;
		if (shape->space)
			cpSpaceRemoveShape(shape->space, shape);				// also cpBodyRemoveShape
		cpSpaceAddShape(body->space, shape);					// also cpBodyAddShape
	};

	cpBodyEachShape(body->body(), fn, nullptr);

	body->onSpaceChanged();

	return true;
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
 * @brief TiledGame::worldStep
 * @param body
 */

void TiledGame::worldStep(TiledObjectBody *body)
{
	Q_ASSERT(body);
	body->worldStep();
}



void TiledGame::timeAfterWorldStepEvent(const qint64 &tick)
{
	Q_UNUSED(tick);
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
 * @brief TiledGame::iterateOverBodies
 * @param func
 */

void TiledGame::iterateOverBodies(const std::function<void (TiledObjectBody *)> &func)
{
	if (!func)
		return;

	QMutexLocker locker(&d->m_stepMutex);

	for (const auto &ptr : d->m_bodyList) {
		if (TiledObjectBody *b = ptr.get())
			func(b);
	}
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
		if (T* o = dynamic_cast<T*>(ptr.get()))
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
		if (T* o = dynamic_cast<T*>(ptr.get())) {
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
std::vector<T *> TiledGamePrivate::getObjects(cpSpace *world)
{
	std::vector<T *> list;

	if (!world)
		return list;

	QMutexLocker locker(&m_stepMutex);

	for (const auto &ptr : m_bodyList) {
		if (T* o = dynamic_cast<T*>(ptr.get())) {
			if (o->space() == world)
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

	for (const auto &b : m_bodyList) {
		if (b && b->objectId() == id)
			return b.get();
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

TiledObjectBody *TiledGamePrivate::addObject(std::unique_ptr<TiledObjectBody> &body)
{
	QMutexLocker locker(&m_stepMutex);

	return m_bodyList.emplace_back(std::move(body)).get();
}




/**
 * @brief TiledGamePrivate::removeObject
 * @param body
 * @return
 */

bool TiledGamePrivate::removeObject(TiledObjectBody *body)
{
	QMutexLocker locker(&m_stepMutex);

	bool ret = m_bodyList.cend() != std::find_if(m_bodyList.cbegin(), m_bodyList.cend(), [body](const auto &b) { return b.get() == body; } );

	if (ret)
		m_removeBodyList.push_back(body);

	return ret;
}


/**
 * @brief TiledGamePrivate::updateObjects
 */

void TiledGamePrivate::updateObjects()
{
	QMutexLocker locker(&m_stepMutex);

	if (!m_removeBodyList.empty()) {
		for (auto it = m_bodyList.begin(); it != m_bodyList.end(); ) {
			if (std::find(m_removeBodyList.cbegin(), m_removeBodyList.cend(), it->get()) != m_removeBodyList.cend()) {
				if (dynamic_cast<QObject*>(it->get()))
					dynamic_cast<QObject*>(it->release())->deleteLater();

				it = m_bodyList.erase(it);
			} else
				++it;
		}

		m_removeBodyList.clear();
	}
}






/**
 * @brief TiledGamePrivate::stepWorlds
 */

void TiledGamePrivate::stepWorlds()
{
	QMutexLocker locker(&m_stepMutex);

	for (const auto &ptr : std::as_const(m_sceneList)) {
		cpSpaceStep(ptr->space.get(), 1./60.);
	}

	for (const auto &ptr : std::as_const(m_bodyList)) {
		if (ptr)
			q->worldStep(ptr.get());
	}
}












/**
 * @brief TiledScene::reloadTcodMap
 */

void TiledGame::reloadTcodMap(cpSpace *space)
{
	if (!space)
		return;

	QMutexLocker locker(&d->m_stepMutex);

	auto it = std::find_if(d->m_sceneList.begin(),
						   d->m_sceneList.end(), [space](const auto &ptr) {
		return ptr->space.get() == space;
	});

	if (it == d->m_sceneList.end()) {
		LOG_CERROR("scene") << "Invalid space" << space;
		return;
	}

	it->get()->reloadTcodMap();
}



/**
 * @brief TiledScene::findShortestPath
 * @param body
 * @param to
 * @return
 */

std::optional<QPolygonF> TiledGame::findShortestPath(TiledObjectBody *body, const cpVect &to) const
{
	if (!body)
		return std::nullopt;

	QMutexLocker locker(&d->m_stepMutex);

	cpSpace *space = body->space();

	if (!space)
		return std::nullopt;

	const RayCastInfo &info = body->rayCast(to, TiledObjectBody::FixtureGround);

	bool isWalkable = true;

	for (const RayCastInfoItem &item : info) {
		if (!item.walkable)
			isWalkable = false;
		break;
	}

	if (isWalkable)
		return QPolygonF() << body->bodyPositionF() << TiledObject::toPointF(to);


	const auto it = std::find_if(d->m_sceneList.cbegin(),
								 d->m_sceneList.cend(), [space](const auto &ptr) {
		return ptr->space.get() == space;
	});

	if (it == d->m_sceneList.cend())
		return std::nullopt;

	return it->get()->tcodMap.findShortestPath(body->bodyPosition(), to);
}



/**
 * @brief TiledScene::findShortestPath
 * @param body
 * @param x2
 * @param y2
 * @return
 */

std::optional<QPolygonF> TiledGame::findShortestPath(TiledObjectBody *body, const qreal &x2, const qreal &y2) const
{
	return findShortestPath(body, cpv(x2, y2));
}




/**
 * @brief TiledGamePrivate::Scene::TcodMapData::findShortestPath
 * @param from
 * @param to
 * @return
 */

std::optional<QPolygonF> TiledGamePrivate::Scene::TcodMapData::findShortestPath(const cpVect &from, const cpVect &to) const
{
	return findShortestPath(from.x, from.y, to.x, to.y);
}



/**
 * @brief TiledGamePrivate::Scene::TcodMapData::findShortestPath
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @return
 */

std::optional<QPolygonF> TiledGamePrivate::Scene::TcodMapData::findShortestPath(const qreal &x1, const qreal &y1, const qreal &x2, const qreal &y2) const
{
	if (qFuzzyCompare(x1, x2) && qFuzzyCompare(y1, y2))
		return std::nullopt;

	if (!map)
		return std::nullopt;

	//TCODPath path(m_tcodMap.map.get());
	TCODDijkstra path(map.get(), 1.0);

	const int chX1 = std::floor((x1-viewport.left())/chunkWidth);
	const int chX2 = std::floor((x2-viewport.left())/chunkWidth);
	const int chY1 = std::floor((y1-viewport.top())/chunkHeight);
	const int chY2 = std::floor((y2-viewport.top())/chunkHeight);

	if (!map->isInBounds(chX1, chY1) ||
			!map->isInBounds(chX2, chY2))
		return std::nullopt;

	if (!map->isWalkable(chX1, chY1))
		return std::nullopt;

	// Ha közel vagyunk (same chunk), nem is számolunk

	if (chX1 == chX2 && chY1 == chY2)
		return QPolygonF() << QPointF(x1, y1) << QPointF(x2, y2);

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
						   viewport.left() + (x+0.5) * chunkWidth,
						   viewport.top() + (y+0.5) * chunkHeight
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




void TiledGamePrivate::Scene::spaceDestroy(cpSpace *space)
{
	if (!space)
		return;
	cpSpaceFree(space);
}




/**
 * @brief TiledGamePrivate::Scene::reloadTcodMap
 */

void TiledGamePrivate::Scene::reloadTcodMap()
{
	if (!space || cpSpaceIsLocked(space.get())) {
		LOG_CERROR("scene") << "Missing or locked space" << this;
		return;
	}


	tcodMap.map.reset();

	if (scene && !scene->viewport().isEmpty()) {
		LOG_CTRACE("scene") << "Replace viewport" << space.get() << scene->viewport();
		tcodMap.viewport = scene->viewport();
	}


	if (tcodMap.viewport.isEmpty()) {
		LOG_CERROR("scene") << "Invalid viewport" << space.get();
		return;
	}

	static const qreal chunkSize = 30.;

	const int wSize = std::ceil(tcodMap.viewport.width() / chunkSize);
	const int hSize = std::ceil(tcodMap.viewport.height() / chunkSize);

	LOG_CDEBUG("scene") << "Reload tcod map" << wSize << hSize << space.get();

	tcodMap.chunkWidth = tcodMap.viewport.width() / wSize;
	tcodMap.chunkHeight = tcodMap.viewport.height() / hSize;

	tcodMap.map.reset(new TCODMap(wSize, hSize));

	Q_ASSERT(tcodMap.map.get());

	tcodMap.map->clear(true, true);

	for (int i=0; i<wSize; ++i) {
		for (int j=0; j<hSize; ++j) {
			cpShape *chunk = cpBoxShapeNew(NULL,
										   tcodMap.chunkWidth,
										   tcodMap.chunkHeight,
										   0);
			cpTransform tr = cpTransformIdentity;
			tr.tx = tcodMap.viewport.left() + tcodMap.chunkWidth*(i+0.5);
			tr.ty = tcodMap.viewport.top() + tcodMap.chunkHeight*(j+0.5);
			cpShapeUpdate(chunk, tr);


			struct _d {
				int i, j;
				TCODMap *map;
			};

			_d d;
			d.i = i;
			d.j = j;
			d.map = tcodMap.map.get();

			static const auto fn = [](cpShape *shape, cpContactPointSet *, void *data) {
				if (cpShapeGetFilter(shape).categories & TiledObjectBody::FixtureGround) {
					_d *d = (_d*)(data);
					d->map->setProperties(d->i, d->j, true, false);
				}
			};

			cpSpaceShapeQuery(space.get(), chunk, fn, &d);

			cpShapeFree(chunk);
		}
	}
}



/**
 * @brief TiledGamePrivate::Scene::destroyScene
 */

void TiledGamePrivate::Scene::destroyScene()
{
	LOG_CTRACE("scene") << "DESTROY SCENE" << this << space.get();

	space.reset();
}





/**
 * @brief TiledGamePrivate::collisionBegin
 * @param arb
 * @param space
 * @param userData
 * @return
 */

cpBool TiledGamePrivate::collisionBegin(cpArbiter *arb, cpSpace *, cpDataPointer)
{
	//TiledGamePrivate *d = (TiledGamePrivate *) userData;

	CP_ARBITER_GET_SHAPES(arb, shapeA, shapeB);

	TiledObjectBody *bodyA = TiledObjectBody::fromBodyRef(cpShapeGetBody(shapeA));
	TiledObjectBody *bodyB = TiledObjectBody::fromBodyRef(cpShapeGetBody(shapeB));

	if (!bodyA || !bodyB)
		return true;

	bodyA->onShapeContactBegin(shapeA, shapeB);
	bodyB->onShapeContactBegin(shapeB, shapeA);

	return true;
}




/**
 * @brief TiledGamePrivate::collisionEnd
 * @param arb
 * @param space
 * @param userData
 */

void TiledGamePrivate::collisionEnd(cpArbiter *arb, cpSpace *, cpDataPointer)
{
	//TiledGamePrivate *d = (TiledGamePrivate *) userData;

	CP_ARBITER_GET_SHAPES(arb, shapeA, shapeB);

	TiledObjectBody *bodyA = TiledObjectBody::fromBodyRef(cpShapeGetBody(shapeA));
	TiledObjectBody *bodyB = TiledObjectBody::fromBodyRef(cpShapeGetBody(shapeB));

	if (!bodyA || !bodyB)
		return;

	bodyA->onShapeContactEnd(shapeA, shapeB);
	bodyB->onShapeContactEnd(shapeB, shapeA);

}
