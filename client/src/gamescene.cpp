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
#include "qtimer.h"
#include "tiledpaintedlayer.h"
#include "utils.h"
#include <QRandomGenerator>

#ifndef Q_OS_WASM
#include "desktopclient.h"
#endif

Q_LOGGING_CATEGORY(lcScene, "app.scene")

/**
 * @brief GameScene::GameScene
 * @param parent
 */

GameScene::GameScene(QQuickItem *parent)
	: QQuickItem(parent)
	, m_timingTimer(new QTimer(this))
{
	qCDebug(lcScene).noquote() << tr("Scene created") << this;

	setImplicitWidth(200);
	setImplicitHeight(200);

	m_timingTimer->setInterval(m_timingTimerTimeoutMsec);
	m_timingTimer->start();

	loadGameData();

	connect(this, &GameScene::sceneStepSuccess, this, &GameScene::onSceneStepSuccess);
	connect(this, &GameScene::sceneLoadFailed, this, &GameScene::onSceneLoadFailed);
}


/**
 * @brief GameScene::~GameScene
 */

GameScene::~GameScene()
{
	delete m_timingTimer;

	qDeleteAll(m_childItems);
	qDeleteAll(m_ladders);
	qDeleteAll(m_tiledLayers);
	qDeleteAll(m_grounds);

	qCDebug(lcScene).noquote() << tr("Scene destroyed") << this;
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

	QString terrain = ml->terrain();

	QString terrainDir = terrain.section("/", 0, -2);
	int terrainLevel = terrain.section("/", -1, -1).toInt();

	if (!m_terrain.loadMap(terrainDir, terrainLevel)) {
		Application::instance()->messageError(tr("A harcmező nem tölthető be!"), tr("Nem lehet elindítani a játékot"));
		emit sceneLoadFailed();
		return;
	}

	setWidth(m_terrain.width());
	setHeight(m_terrain.height());

	loadTiledLayers();
	loadGroundLayer();
	loadLadderLayer();
	loadPlayerPositionLayer();
	loadTerrainObjectsLayer();

	++m_sceneLoadSteps;

	emit sceneStepSuccess();
}


/**
 * @brief GameScene::playSoundPlayerVoice
 * @param source
 */

void GameScene::playSoundPlayerVoice(const QString &source)
{
#ifndef Q_OS_WASM
	DesktopClient *client = qobject_cast<DesktopClient*>(Application::instance()->client());
	if (client)
		client->playSound(source, Sound::PlayerVoice);
#endif
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

	case Qt::Key_F3:
		zoomOverviewToggle();
		break;

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

	case Qt::Key_R:
		if (event->modifiers().testFlag(Qt::ShiftModifier))
			m_game->setRunning(!m_game->running());
		break;

	case Qt::Key_X:
		if (event->modifiers().testFlag(Qt::ShiftModifier) && m_mouseArea && m_mouseArea->property("containsMouse").toBool() && player)
			player->moveTo(m_mouseArea->property("mouseX").toReal(), m_mouseArea->property("mouseY").toReal(), true);

		break;
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
	bool error = false;
	const QString filename = ":/internal/game/parameters.json";

	m_gameData = Utils::fileToJsonObject(filename, &error);

	if (error)
		return;

	qCDebug(lcScene).noquote() << tr("Load game data from:") << filename;
}


/**
 * @brief GameScene::loadTiledLayers
 */

void GameScene::loadTiledLayers()
{
	Tiled::Map *map = m_terrain.map();

	if (!map) {
		qCWarning(lcScene).noquote() << tr("Invalid map");
		return;
	}

	for (auto layer = map->tileLayers().begin(); layer != map->tileLayers().end(); ++layer) {
		qCDebug(lcScene).noquote() << QObject::tr("Load tile layer:") << layer->name();

		Tiled::TileLayer *l = static_cast<Tiled::TileLayer*>(*layer);

		TiledPaintedLayer *paintedLayer = new TiledPaintedLayer(this, map, l);
		QVariant layerZ = layer->property("z");
		paintedLayer->setX(layer->offset().x());
		paintedLayer->setY(layer->offset().y());
		paintedLayer->setZ(layerZ.isValid() ? layerZ.toInt() : -1);
		paintedLayer->setOpacity(layer->opacity());
		paintedLayer->setVisible(layer->isVisible());
		paintedLayer->setWidth(paintedLayer->implicitWidth());
		paintedLayer->setHeight(paintedLayer->implicitHeight());

		m_tiledLayers.append(paintedLayer);
	}
}


/**
 * @brief GameScene::loadGroundLayer
 */

void GameScene::loadGroundLayer()
{
	Tiled::ObjectGroup *layer = objectLayer("Ground");

	if (!layer) {
		qCWarning(lcScene).noquote() << tr("Missing ground layer");
		return;
	}

	qCDebug(lcScene).noquote() << tr("Load ground layer");

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
	Tiled::ObjectGroup *layer = objectLayer("Ladders");

	if (!layer) {
		qCWarning(lcScene).noquote() << tr("Missing ladders layer");
		return;
	}

	qCDebug(lcScene).noquote() << tr("Load ladders layer");

	foreach (Tiled::MapObject *object, layer->objects()) {
		if (object->shape() != Tiled::MapObject::Rectangle)
			continue;

		QRectF rect(object->x(), object->y(), object->width(), object->height());

		GameLadder *ladder = qobject_cast<GameLadder*>(GameObject::createFromFile("GameLadder.qml", this));

		if (!ladder) {
			qCCritical(lcScene).noquote() << tr("Ladder creation error");
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
	qCDebug(lcScene).noquote() << tr("Load terrain objects layer");

	QHash<GameTerrain::ObjectType, QString> list;

	list.insert(GameTerrain::Fire, "GameFire.qml");
	list.insert(GameTerrain::Fence, "GameFence.qml");

	for (auto it = list.constBegin(); it != list.constEnd(); ++it) {
		const GameTerrain::ObjectType &type = it.key();
		const QString &qml = it.value();

		foreach(const GameTerrain::ObjectData &data, m_terrain.objects(type)) {
			GameObject *object = GameObject::createFromFile(qml, this);

			if (!object) {
				qCCritical(lcScene).noquote() << tr("Terrain object creation error:") << type << qml;
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

			addChildItem(object);
		}
	}
}




/**
 * @brief GameScene::loadPlayerPositionLayer
 */

void GameScene::loadPlayerPositionLayer()
{
	qCDebug(lcScene).noquote() << tr("Load player position layer");

	foreach(const GameTerrain::PlayerPositionData &data, m_terrain.playerPositions()) {
		m_grounds.append(new GamePlayerPosition(data, this));
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
		qCWarning(lcScene()).noquote() << tr("Invalid map");
		return nullptr;
	}

	for (auto layer = map->objectGroups().begin(); layer != map->objectGroups().end(); ++layer) {
		if (layer->name() == name)
			return static_cast<Tiled::ObjectGroup*>(*layer);
	}

	return nullptr;
}


/**
 * @brief GameScene::terrain
 * @return
 */

const GameTerrain &GameScene::terrain() const
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

		if (m_gameData.value("level").toObject().contains(key)) {
			r = m_gameData.value("level").toObject().value(key).toObject();
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
	qCDebug(lcScene).noquote() << tr("Create player");

	if (m_game->player()) {
		qCWarning(lcScene).noquote() << tr("Player already exists");
		return;
	}

	GamePlayer *player = GamePlayer::create(this);
	GameTerrain::PlayerPositionData pos = getPlayerPosition();
	pos.point.setY(pos.point.y()-player->height());

	player->setPosition(pos.point);

	m_game->setPlayer(player);

	addChildItem(player);

	QCoreApplication::processEvents();
}



/**
 * @brief GameScene::timingTimerTimeoutMsec
 * @return
 */

int GameScene::timingTimerTimeoutMsec() const
{
	return m_timingTimerTimeoutMsec;
}


/**
 * @brief GameScene::timingTimer
 * @return
 */

QTimer *GameScene::timingTimer() const
{
	return m_timingTimer;
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
	qCDebug(lcScene).noquote() << tr("Scene prepared");

	++m_sceneLoadSteps;

	emit sceneStepSuccess();
}


/**
 * @brief GameScene::onSceneStepSuccess
 */

void GameScene::onSceneStepSuccess()
{
	qCDebug(lcScene).noquote() << tr("Scene step:") << m_sceneLoadSteps;

	if (m_sceneLoadSteps < 2)
		return;

	m_game->createQuestions();
	m_game->createEnemyLocations();
	m_game->createFixEnemies();
	m_game->createInventory();

	m_game->pageItem()->setState("run");
}


/**
 * @brief GameScene::onSceneLoadFailed
 */

void GameScene::onSceneLoadFailed()
{
	m_game->pageItem()->setProperty("closeDisabled", "");
	m_game->pageItem()->setProperty("onPageClose", QVariant::Invalid);
	m_game->pageItem()->setProperty("closeQuestion", "");

	m_game->unloadPageItem();

}


/**
 * @brief GameScene::onSceneAnimationReady
 */

void GameScene::onSceneAnimationReady()
{
	m_game->pageItem()->setProperty("closeDisabled", "");

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
	qCDebug(lcScene).noquote() << tr("Activate ladders in block:") << block;

	foreach (GameLadder *ladder, m_ladders) {
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
		qCDebug(lcScene).noquote() << tr("Player position reached:") << position->position();
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

/**
 * @brief GameScene::addChildItem
 * @param item
 */

void GameScene::addChildItem(QQuickItem *item)
{
	m_childItems.append(item);
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
