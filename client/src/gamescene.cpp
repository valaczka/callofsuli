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
#include "gameenemysoldier.h"
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
	if (!m_terrain.loadMap("Test", 1))
		return;

	setWidth(m_terrain.width());
	setHeight(m_terrain.height());

	loadTiledLayers();
	loadGroundLayer();
	loadLadderLayer();
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

	if (!m_game->running())
		player = nullptr;

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
	if (event->isAutoRepeat())
		return;

	//qDebug(lcScene).noquote() << tr("Key release event:") << event;

	const int &key = event->key();
	GamePlayer *player = m_game->player();

	/*if (!m_game->running())
		player = nullptr;*/

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
		fixture->setCategories(Box2DFixture::Category1);
		fixture->setCollidesWith(Box2DFixture::Category1|Box2DFixture::Category2|Box2DFixture::Category5);

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

	const QString &key = QString::number(level);

	return m_gameData.value("level").toObject().value(key).toObject();
}



/**
 * @brief GameScene::createPlayer
 */

void GameScene::createPlayer()
{
	qDebug() << "CREATE PLAYER" << m_game->player();

	if (m_game->player()) {
		m_game->setPlayer(nullptr);
		qDebug() << "ALREADY";
	}

	GamePlayer *player = GamePlayer::create(this);
	GameTerrain::PlayerPositionData pos = m_terrain.defaultPlayerPosition();
	pos.point.setY(pos.point.y()-player->height());

	player->setPosition(pos.point);
	player->setMaxHp(5);
	player->setHp(3);

	m_game->setPlayer(player);

	connect(player, &GamePlayer::died, this, &GameScene::createPlayer);

	addChildItem(player);
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

	m_game->pageItem()->setProperty("closeDisabled", "");
	m_game->pageItem()->setState("run");
	m_game->setRunning(true);


	foreach (auto e, m_terrain.enemies()) {
		if (e.type != GameTerrain::EnemySoldier)
			continue;

		GameEnemySoldier *soldier = GameEnemySoldier::create(this, e);

		soldier->setX(e.rect.left());
		soldier->setY(e.rect.bottom()-soldier->height());

		soldier->setMaxHp(QRandomGenerator::global()->bounded(1, 5));
		soldier->setHp(QRandomGenerator::global()->bounded(1, 5));

		addChildItem(soldier);

		soldier->startMovingAfter(2500);

		QCoreApplication::processEvents();
	}


	createPlayer();
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
