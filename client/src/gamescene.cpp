/*
 * ---- Call of Suli ----
 *
 * gamescene.cpp
 *
 * Created on: 2022. 12. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameScene
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

#include "gamescene.h"
#include "actiongame.h"
#include "application.h"
#include "box2dfixture.h"
#include "gameladder.h"
#include "gameobject.h"
#include "gameplayer.h"
#include "gameterrain.h"
#include "gameplayerposition.h"
#include "libtiled/mapobject.h"
#include "libtiled/objectgroup.h"
#include "server.h"
#include "utils_.h"
#include <QRandomGenerator>

/**
 * @brief GameScene::GameScene
 * @param parent
 */

GameScene::GameScene(QQuickItem *parent)
	: QQuickItem(parent)
{
	LOG_CDEBUG("scene") << "Scene created" << this;

	setImplicitWidth(200);
	setImplicitHeight(200);

	loadGameData();

	connect(this, &GameScene::sceneStepSuccess, this, &GameScene::onSceneStepSuccess);
	connect(this, &GameScene::sceneLoadFailed, this, &GameScene::onSceneLoadFailed);
}


/**
 * @brief GameScene::~GameScene
 */

GameScene::~GameScene()
{
	m_grounds.clear();
	m_ladders.clear();
	m_childItems.clear();
	LOG_CDEBUG("scene") << "Scene destroyed" << this;
}


/**
 * @brief GameScene::game
 * @return
 */

ActionGame *GameScene::game() const
{
	return m_game;
}

void GameScene::setGame(ActionGame *newGame)
{
	if (m_game == newGame)
		return;
	m_game = newGame;
	emit gameChanged();
}


/**
 * @brief GameScene::loadTerrain
 * @param terrain
 */

void GameScene::load()
{
	if (!m_game->missionLevel()) {
		Application::instance()->messageError(tr("A küldetés nincs beállítva!"), tr("Nem lehet elindítani a játékot"));
		emit sceneLoadFailed();
		return;
	}

	GameMapMissionLevel *ml = m_game->missionLevel();

	const QString &terrain = ml->terrain();

	const QString &terrainDir = terrain.section("/", 0, -2);
	const int &terrainLevel = terrain.section("/", -1, -1).toInt();

	if (!m_terrain.loadMap(terrainDir, terrainLevel)) {
		Application::instance()->messageError(tr("A harcmező nem tölthető be!"), tr("Nem lehet elindítani a játékot"));
		emit sceneLoadFailed();
		return;
	}

	setWidth(m_terrain.width());
	setHeight(m_terrain.height());

	/*loadTiledLayers();
	loadGroundLayer();
	loadLadderLayer();
	loadPlayerPositionLayer();
	loadTerrainObjectsLayer();
	loadPickablesLayer();*/

	++m_sceneLoadSteps;

	emit sceneStepSuccess();
}


/**
 * @brief GameScene::playSoundPlayerVoice
 * @param source
 */

void GameScene::playSoundPlayerVoice(const QString &source)
{
	Application::instance()->client()->sound()->playSound(source, Sound::SfxChannel);
}


/**
 * @brief GameScene::playSound
 * @param source
 */

void GameScene::playSound(const QString &source)
{
	Application::instance()->client()->sound()->playSound(source, Sound::SfxChannel);
}


/**
 * @brief GameScene::playSoundVoiceOver
 * @param source
 */

void GameScene::playSoundVoiceOver(const QString &source)
{
	Application::instance()->client()->sound()->playSound(source, Sound::VoiceoverChannel);
}


/**
 * @brief GameScene::playSoundMusic
 * @param source
 */

void GameScene::playSoundMusic(const QString &source)
{
	Application::instance()->client()->sound()->playSound(source, Sound::MusicChannel);
}




/**
 * @brief GameScene::stopSoundMusic
 * @param source
 */

void GameScene::stopSoundMusic()
{
	Application::instance()->client()->sound()->stopMusic();
}






/**
 * @brief GameScene::keyPressEvent
 * @param event
 */

void GameScene::keyPressEvent(QKeyEvent *event)
{
	/*if (!event->isAutoRepeat())
		qDebug(lcScene).noquote() << tr("Key press event:") << event;*/

	const int &key = event->key();
	GamePlayer *player = m_game->player();

	if (!m_game->running() || m_sceneState != ScenePlay)
		return;

	switch (key) {

	case Qt::Key_Shift:
		if (player) player->setMovingFlag(GamePlayer::SlowModifier);
		break;

	case Qt::Key_Left:
		if (player) {
			if (event->modifiers().testFlag(Qt::ShiftModifier)) player->setMovingFlag(GamePlayer::SlowModifier);
			player->setMovingFlag(GamePlayer::MoveLeft);
		}
		break;

	case Qt::Key_Right:
		if (player) {
			if (event->modifiers().testFlag(Qt::ShiftModifier)) player->setMovingFlag(GamePlayer::SlowModifier);
			player->setMovingFlag(GamePlayer::MoveRight);
		}
		break;

	case Qt::Key_Up:
		if (player) player->setMovingFlag(GamePlayer::MoveUp);
		break;

	case Qt::Key_Down:
		if (player) player->setMovingFlag(GamePlayer::MoveDown);
		break;

	case Qt::Key_Space:
		if (player) player->shot();
		return;

	case Qt::Key_Return:
	case Qt::Key_Enter:
		m_game->pickablePick();
		break;

	case Qt::Key_F3:
		zoomOverviewToggle();
		break;

	case Qt::Key_W:
		game()->toolUse(GamePickable::PickableWater);
		break;

	case Qt::Key_P:
		game()->toolUse(GamePickable::PickablePliers);
		break;

	case Qt::Key_I:
		game()->toolUse(GamePickable::PickableCamouflage);
		break;

	}




	if (Application::instance()->client()->debug()) {
		switch (key) {
		case Qt::Key_F10:
			setShowObjects(true);
			break;

		case Qt::Key_F11:
			setShowEnemies(true);
			break;

		case Qt::Key_D:
			if (event->modifiers().testFlag(Qt::ShiftModifier))
				setDebugView(!m_debugView);
			break;

		case Qt::Key_N:
			if (event->modifiers().testFlag(Qt::ShiftModifier) && event->modifiers().testFlag(Qt::ControlModifier))
				m_game->killAllEnemy();
			break;

		case Qt::Key_X:
			if (event->modifiers().testFlag(Qt::ShiftModifier) && m_mouseArea && m_mouseArea->property("containsMouse").toBool() && player)
				player->moveTo(m_mouseArea->property("mouseX").toReal(), m_mouseArea->property("mouseY").toReal(), true);

			break;
		}
	}
}


/**
 * @brief GameScene::keyReleaseEvent
 * @param event
 */

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
	if (!m_game->running() || m_sceneState != ScenePlay)
		return;

	if (event->isAutoRepeat())
		return;

	//qDebug(lcScene).noquote() << tr("Key release event:") << event;

	const int &key = event->key();
	GamePlayer *player = m_game->player();

	switch (key) {
	case Qt::Key_Shift:
		if (player) player->setMovingFlag(GamePlayer::SlowModifier, false);
		break;

	case Qt::Key_Left:
		if (player) {
			if (event->modifiers().testFlag(Qt::ShiftModifier)) player->setMovingFlag(GamePlayer::SlowModifier, false);
			player->setMovingFlag(GamePlayer::MoveLeft, false);
		}
		break;

	case Qt::Key_Right:
		if (player) {
			if (event->modifiers().testFlag(Qt::ShiftModifier)) player->setMovingFlag(GamePlayer::SlowModifier, false);
			player->setMovingFlag(GamePlayer::MoveRight, false);
		}
		break;

	case Qt::Key_Up:
		if (player) player->setMovingFlag(GamePlayer::MoveUp, false);
		break;

	case Qt::Key_Down:
		if (player) player->setMovingFlag(GamePlayer::MoveDown, false);
		break;

	case Qt::Key_F10:
		setShowObjects(false);
		break;

	case Qt::Key_F11:
		setShowEnemies(false);
		break;
	}

}



/**
 * @brief GameScene::loadGameData
 */

void GameScene::loadGameData()
{
	const QString filename = QStringLiteral(":/internal/game/parameters.json");

	const auto &c = Utils::fileToJsonObject(filename);

	if (c)
		m_gameData = *c;
	else
		return;

	LOG_CDEBUG("scene") << "Load game data from:" << filename;
}


/**
 * @brief GameScene::loadTiledLayers
 */

void GameScene::loadTiledLayers()
{
	if (!m_terrain.imageTerrain().isEmpty()) {
		LOG_CDEBUG("scene") <<  "Load terrain tile image:" << qPrintable(m_terrain.imageTerrain());
		emit imageTerrainChanged();
	}

	if (!m_terrain.imageOver().isEmpty()) {
		LOG_CDEBUG("scene") <<  "Load terrain over image:" << qPrintable(m_terrain.imageOver());
		emit imageOverChanged();
	}
}


/**
 * @brief GameScene::loadGroundLayer
 */

void GameScene::loadGroundLayer()
{
	Tiled::ObjectGroup *layer = objectLayer(QStringLiteral("Ground"));

	if (!layer) {
		LOG_CWARNING("scene") << "Missing ground layer";
		return;
	}

	LOG_CDEBUG("scene") << "Load ground layer";

	foreach (Tiled::MapObject *object, layer->objects()) {
		QRectF rect(object->x(), object->y(), object->width(), object->height());

		GameObject *item = new GameObject(this);
		item->setX(rect.x());
		item->setY(rect.y());
		item->setZ(0);
		item->setWidth(rect.width());
		item->setHeight(rect.height());
		item->setVisible(true);

		Box2DBox *fixture = new Box2DBox(item);

		fixture->setX(0);
		fixture->setY(0);
		fixture->setWidth(rect.width());
		fixture->setHeight(rect.height());

		fixture->setDensity(1);
		fixture->setFriction(1);
		fixture->setRestitution(0);
		fixture->setCategories(CATEGORY_GROUND);
		fixture->setCollidesWith(CATEGORY_GROUND|CATEGORY_PLAYER|CATEGORY_ENEMY);

		item->body()->addFixture(fixture);

		item->bodyComplete();

		m_grounds.append(item);
	}

}



/**
 * @brief GameScene::loadLadderLayer
 */

void GameScene::loadLadderLayer()
{
	Tiled::ObjectGroup *layer = objectLayer(QStringLiteral("Ladders"));

	if (!layer) {
		LOG_CWARNING("scene") << "Missing ladders layer";
		return;
	}

	LOG_CDEBUG("scene") << "Load ladders layer";

	foreach (Tiled::MapObject *object, layer->objects()) {
		if (object->shape() != Tiled::MapObject::Rectangle)
			continue;

		QRectF rect(object->x(), object->y(), object->width(), object->height());

		GameLadder *ladder = qobject_cast<GameLadder*>(GameObject::createFromFile(QStringLiteral("GameLadder.qml"), this, false));

		if (!ladder) {
			LOG_CERROR("scene") << "Ladder creation error";
			continue;
		}

		ladder->setParentItem(this);
		ladder->setScene(this);
		ladder->setBoundRect(rect);

		const int &blockTop = object->property("blockTop").toInt();
		const int &blockBottom = object->property("blockBottom").toInt();

		if (blockTop > 0 && blockBottom > 0) {
			ladder->setBlockTop(blockTop);
			ladder->setBlockBottom(blockBottom);
			ladder->setActive(false);
		} else {
			ladder->setActive(true);
		}

		ladder->bodyComplete();

		m_ladders.append(ladder);
	}
}


/**
 * @brief GameScene::loadSceneObjectLayer
 */

void GameScene::loadTerrainObjectsLayer()
{
	LOG_CDEBUG("scene") << "Load terrain objects layer";

	QHash<GameTerrain::ObjectType, QString> list;

	list.insert(GameTerrain::Fire, QStringLiteral("GameFire.qml"));
	list.insert(GameTerrain::Fence, QStringLiteral("GameFence.qml"));

	for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
		const GameTerrain::ObjectType &type = it.key();
		const QString &qml = it.value();

		foreach(const GameTerrain::ObjectData &data, m_terrain.objects(type)) {
			GameObject *object = GameObject::createFromFile(qml, this, false);

			if (!object) {
				LOG_CERROR("scene") << "Terrain object creation error:" << type << qml;
				continue;
			}

			object->setParentItem(this);
			object->setScene(this);

			switch (type) {
			case GameTerrain::Fire:
				object->setX(data.point.x()-(object->width()/2));
				object->setY(data.point.y()-object->height()+10);                       // +10: az animáció korrekciója miatt lejjebb kell tenni
				break;
			case GameTerrain::Fence:
				object->setX(data.point.x()-(object->width()/2));
				object->setY(data.point.y()-object->height());
				break;

			default:
				object->setPosition(data.point);
			}

			object->bodyComplete();
		}
	}
}




/**
 * @brief GameScene::loadPlayerPositionLayer
 */

void GameScene::loadPlayerPositionLayer()
{
	LOG_CDEBUG("scene") << "Load player position layer";

	foreach(const GameTerrain::PlayerPositionData &data, m_terrain.playerPositions()) {
		m_grounds.append(new GamePlayerPosition(data, this));
	}
}



/**
 * @brief GameScene::loadPickablesLayer
 */

void GameScene::loadPickablesLayer()
{
	LOG_CDEBUG("scene") << "Load pickables layer";

	foreach(const GameTerrain::PickableData &data, m_terrain.pickables()) {
		m_game->createPickable(data.data, data.point);
	}

}







/**
 * @brief GameScene::objectLayer
 * @param name
 * @return
 */

Tiled::ObjectGroup *GameScene::objectLayer(const QString &name) const
{
	Tiled::Map *map = m_terrain.map();

	if (!map) {
		LOG_CWARNING("scene") << "Invalid map";
		return nullptr;
	}

	for (auto layer = map->objectGroups().begin(); layer != map->objectGroups().end(); ++layer) {
		if (layer->name() == name)
			return static_cast<Tiled::ObjectGroup*>(*layer);
	}

	return nullptr;
}





/**
 * @brief GameScene::onTimerTimeout
 */

void GameScene::onTimerTimeout()
{
	qreal factor = 1.0;

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


	foreach (GameObject *o, m_gameObjects)
		if (o)
			o->onTimingTimerTimeout(m_timingTimerTimeoutMsec, factor);


	foreach (GameObject *o, m_gameObjects) {
		GameEntity *e = qobject_cast<GameEntity*>(o);
		if (e)
			e->performRayCast();
	}
}



/**
 * @brief GameScene::fpsSet
 */

void GameScene::fpsSet()
{
	double fps = m_fpsList.at(m_fpsIndex);
	LOG_CINFO("scene") << "Set fps:" << fps;

#ifdef QT_DEBUG
	if (m_game)
		m_game->message(QString("FPS SET %1").arg(fps));
#endif

	if (m_world)
		m_world->setTimeStep(1./fps);
}


/**
 * @brief GameScene::fpsIncrease
 */

void GameScene::fpsIncrease()
{
	if (m_fpsIndex > 0) {
		--m_fpsIndex;
		fpsSet();
	}
}


/**
 * @brief GameScene::fpsDecrease
 */

void GameScene::fpsDecrease()
{
	if (m_fpsIndex < m_fpsList.size()-1) {
		++m_fpsIndex;
		fpsSet();
	}
}




/**
 * @brief GameScene::terrain
 * @return
 */

const GameTerrainMap &GameScene::terrain() const
{
	return m_terrain;
}


/**
 * @brief GameScene::getPlayerPosition
 * @return
 */

GameTerrain::PlayerPositionData GameScene::getPlayerPosition()
{
	while (!m_playerPositions.isEmpty()) {
		GamePlayerPosition *p = m_playerPositions.top();

		if (m_game->closedBlocks().contains(p->data().block)) {
			return p->data();
		}

		m_playerPositions.pop();
	}

	return m_terrain.defaultPlayerPosition();
}


/**
 * @brief GameScene::sceneState
 * @return
 */

GameScene::SceneState GameScene::sceneState() const
{
	return m_sceneState;
}

void GameScene::setSceneState(SceneState newSceneState)
{
	if (m_sceneState == newSceneState)
		return;
	m_sceneState = newSceneState;
	emit sceneStateChanged();

	if (m_sceneState == ScenePlay)
		emit sceneStarted();
}


/**
 * @brief GameScene::gameData
 * @return
 */

const QJsonObject &GameScene::gameData() const
{
	return m_gameData;
}




/**
 * @brief GameScene::levelData
 * @param level
 * @return
 */

QJsonObject GameScene::levelData(int level) const
{
	if (level < 0)
		level = m_game->level();

	QJsonObject r;

	while (level > 0) {
		const QString &key = QString::number(level);

		if (m_gameData.value(QStringLiteral("level")).toObject().contains(key)) {
			r = m_gameData.value(QStringLiteral("level")).toObject().value(key).toObject();
			break;
		}

		--level;
	}

	return r;
}



/**
 * @brief GameScene::createPlayer
 */

void GameScene::createPlayer()
{
	LOG_CDEBUG("scene") << "Create player";

	if (m_game->player()) {
		LOG_CWARNING("scene") << "Player already exists";
		return;
	}

	QString character = Application::instance()->client()->server() ? Application::instance()->client()->server()->user()->character() :
																	  QStringLiteral("");

	GamePlayer *player = GamePlayer::create(this, character);
	GameTerrain::PlayerPositionData pos = getPlayerPosition();
	pos.point.setY(pos.point.y()-player->height());

	player->setPosition(pos.point);

	m_game->setPlayer(player);
}







/**
 * @brief GameScene::ladders
 * @return
 */

const QList<QPointer<GameLadder>> &GameScene::ladders() const
{
	return m_ladders;
}


/**
 * @brief GameScene::world
 * @return
 */

Box2DWorld *GameScene::world() const
{
	return m_world;
}

void GameScene::setWorld(Box2DWorld *newWorld)
{
	if (m_world == newWorld)
		return;
	m_world = newWorld;
	emit worldChanged();

	if (m_world) {
		fpsSet();
		connect(m_world, &Box2DWorld::stepped, this, &GameScene::onTimerTimeout);
	}
}


/**
 * @brief GameScene::zoomOverview
 * @return
 */


bool GameScene::zoomOverview() const
{
	return m_zoomOverview;
}

void GameScene::setZoomOverview(bool newZoomOverview)
{
	if (m_zoomOverview == newZoomOverview)
		return;
	m_zoomOverview = newZoomOverview;
	emit zoomOverviewChanged(m_zoomOverview);
}


/**
 * @brief GameScene::zoomOverviewToggle
 */

void GameScene::zoomOverviewToggle()
{
	setZoomOverview(!m_zoomOverview);
}


/**
 * @brief GameScene::onScenePrepared
 */

void GameScene::onScenePrepared()
{
	LOG_CDEBUG("scene") << "Scene prepared";

	loadTiledLayers();
	loadGroundLayer();
	loadLadderLayer();
	loadPlayerPositionLayer();
	loadTerrainObjectsLayer();
	loadPickablesLayer();

	++m_sceneLoadSteps;

	emit sceneStepSuccess();
}


/**
 * @brief GameScene::onSceneStepSuccess
 */

void GameScene::onSceneStepSuccess()
{
	LOG_CDEBUG("scene") << "Scene step:" << m_sceneLoadSteps;

	if (m_sceneLoadSteps < 2)
		return;

	m_game->createQuestions();
	m_game->createEnemyLocations();
	m_game->createFixEnemies();
	m_game->createInventory();

	m_game->pageItem()->setState(QStringLiteral("run"));

	playSoundMusic(m_game->backgroundMusic());
}


/**
 * @brief GameScene::onSceneLoadFailed
 */

void GameScene::onSceneLoadFailed()
{
	m_game->pageItem()->setProperty("closeDisabled", QStringLiteral(""));
#if QT_VERSION < 0x060000
	m_game->pageItem()->setProperty("onPageClose", QVariant::Invalid);
#else
	m_game->pageItem()->setProperty("onPageClose", QVariant(QMetaType::fromType<QJSValue>()));
#endif
	m_game->pageItem()->setProperty("closeQuestion", QStringLiteral(""));

	m_game->unloadPageItem();

	emit m_game->gameFinished(AbstractGame::Fail);

}


/**
 * @brief GameScene::onSceneAnimationReady
 */

void GameScene::onSceneAnimationReady()
{
	m_game->pageItem()->setProperty("closeDisabled", QStringLiteral(""));

	m_game->recreateEnemies();

	createPlayer();


	setSceneState(ScenePlay);
}


/**
 * @brief GameScene::activateLaddersInBlock
 * @param block
 */

void GameScene::activateLaddersInBlock(const int &block)
{
	LOG_CDEBUG("scene") << "Activate ladders in block:" << block;

	foreach (GameLadder *ladder, m_ladders) {
		if (!ladder)
			continue;

		if (ladder->blockTop() == block || ladder->blockBottom() == block)
			ladder->setActive(true);
	}
}


/**
 * @brief GameScene::setPlayerPosition
 * @param position
 */

void GameScene::setPlayerPosition(GamePlayerPosition *position)
{
	if (position && (m_playerPositions.isEmpty() || m_playerPositions.top() != position)) {
		LOG_CDEBUG("scene") << "Player position reached:" << position->position();
		m_playerPositions.push(position);
	}
}


/**
 * @brief GameScene::debugView
 * @return
 */

bool GameScene::debugView() const
{
	return m_debugView;
}

void GameScene::setDebugView(bool newDebugView)
{
	if (m_debugView == newDebugView)
		return;
	m_debugView = newDebugView;
	emit debugViewChanged();
}

bool GameScene::showObjects() const
{
	return m_showObjects;
}

void GameScene::setShowObjects(bool newShowObjects)
{
	if (m_showObjects == newShowObjects)
		return;
	m_showObjects = newShowObjects;
	emit showObjectsChanged(m_showObjects);
}

bool GameScene::showEnemies() const
{
	return m_showEnemies;
}

void GameScene::setShowEnemies(bool newShowEnemies)
{
	if (m_showEnemies == newShowEnemies)
		return;
	m_showEnemies = newShowEnemies;
	emit showEnemiesChanged(m_showEnemies);
}


QQuickItem *GameScene::mouseArea() const
{
	return m_mouseArea;
}

void GameScene::setMouseArea(QQuickItem *newMouseArea)
{
	if (m_mouseArea == newMouseArea)
		return;
	m_mouseArea = newMouseArea;
	emit mouseAreaChanged();
}

QQuickItem *GameScene::messageList() const
{
	return m_messageList;
}

void GameScene::setMessageList(QQuickItem *newMessageList)
{
	if (m_messageList == newMessageList)
		return;
	m_messageList = newMessageList;
	emit messageListChanged();
}



const QString &GameScene::imageTerrain() const
{
	return m_terrain.imageTerrain();
}

const QString &GameScene::imageOver() const
{
	return m_terrain.imageOver();
}


/**
 * @brief GameScene::gameObjectAdd
 * @param object
 */

void GameScene::gameObjectAdd(GameObject *object)
{
	if (!m_gameObjects.contains(object)) {
		m_gameObjects.append(object);
		connect(object, &GameObject::destroyed, this, [this, object]() {
			for (auto it=m_gameObjects.begin(); it != m_gameObjects.end(); ) {
				if (it->data() == object)
					it = m_gameObjects.erase(it);
				else
					++it;
			}
		}, Qt::DirectConnection);
	}
}

