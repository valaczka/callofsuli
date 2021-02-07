/*
 * ---- Call of Suli ----
 *
 * cosgame.cpp
 *
 * Created on: 2020. 10. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosGame
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <QQmlEngine>
#include <QTimer>
#include <QRandomGenerator>

#include "cosclient.h"

#include "mapobject.h"
#include "objectgroup.h"

#include "gameenemy.h"
#include "gameenemydata.h"
#include "gameenemysoldier.h"
#include "gameplayer.h"
#include "gamequestion.h"
#include "gameactivity.h"
#include "cosgame.h"

CosGame::CosGame(QQuickItem *parent)
	: Game(parent)
	, m_player(nullptr)
	, m_gameScene(nullptr)
	, m_playerStartPosition(nullptr)
	, m_running(true)
	, m_itemPage(nullptr)
	, m_question(nullptr)
	, m_terrainData(nullptr)
	, m_gameMatch(nullptr)
	, m_isPrepared(false)
	, m_activity(nullptr)
	, m_timer(new QTimer(this))
	, m_msecLeft(0)
	, m_activeEnemies(0)
	, m_matchTimer(new QTimer(this))
	, m_isStarted(false)
	, m_backgroundMusicFile("qrc:/sound/music/default_bg_music.mp3")
	, m_inventoryPickableList()
{
	connect(this, &Game::gameStateChanged, this, &CosGame::resetRunning);
	connect(this, &CosGame::gameStarted, this, &CosGame::onGameStarted);
	connect(this, &CosGame::gameAbortRequest, this, &CosGame::onGameFinishedLost);
	connect(this, &CosGame::gameLost, this, &CosGame::onGameFinishedLost);
	connect(this, &CosGame::gameTimeout, this, &CosGame::onGameFinishedLost);
	connect(this, &CosGame::gameCompleted, this, &CosGame::onGameFinishedSuccess);

	connect(m_timer, &QTimer::timeout, this, &CosGame::onTimerTimeout);

	connect(m_matchTimer, &QTimer::timeout, this, &CosGame::onGameMatchTimerTimeout);

	loadGameData();
}



/**
 * @brief CosGame::~CosGame
 */

CosGame::~CosGame()
{
	if (m_question)
		m_question->deleteLater();

	if (m_terrainData)
		m_terrainData->deleteLater();

	delete m_timer;
	delete m_matchTimer;
}



/**
 * @brief CosGame::loadScene
 */

void CosGame::loadScene()
{
	if (!m_gameScene || !m_gameMatch) {
		qWarning() << "Mission gameScene or gameMatch!";
		return;
	}

	GameScene *scene = qvariant_cast<GameScene *>(m_gameScene->property("scenePrivate"));

	if (!scene) {
		qWarning() << "Invalid scene!";
		emit gameSceneLoadFailed();
		return;
	}

	if (!scene->loadScene()) {
		qWarning() << "Scene load failed!";
		emit gameSceneLoadFailed();
		return;
	}

	foreach (GameBlock *block, m_terrainData->blocks()) {
		connect(block, &GameBlock::completedChanged, this, [=](bool completed){
			if (completed)
				emit gameMessageSent(tr("Area cleared"), 1);
		});
	}

	emit gameSceneLoaded();

}




/**
 * @brief CosGame::reloadEnemies
 */

void CosGame::recreateEnemies()
{
	if (!m_gameScene || !m_terrainData)
		return;

	qDebug() << "Recreate enemies";

	QVector<GameEnemyData *> allEnemyDataList;

	QMap<int, GameBlock *> b = m_terrainData->blocks();
	QMap<int, GameBlock *>::const_iterator it;
	QVector<GameEnemy *> createdEnemies;

	for (it = b.constBegin(); it != b.constEnd(); ++it) {
		GameBlock *block = it.value();

		if (block->completed()) {
			qDebug() << "Block completed" << it.key() << block;
			continue;
		}

		qDebug() << "Create enemies for block" << it.key() << block;

		QVector<GameEnemyData *> enemyDataList;

		foreach (GameEnemyData *data, block->enemies()) {
			if (data->enemy())
				continue;

			QQuickItem *enemy = nullptr;

			QMetaObject::invokeMethod(m_gameScene, "createComponent", Qt::DirectConnection,
									  Q_RETURN_ARG(QQuickItem *, enemy),
									  Q_ARG(int, data->enemyType())
									  );

			if (enemy) {
				data->setEnemy(enemy);

				GameEnemy *ep = data->enemyPrivate();

				if (data->enemyType() == GameEnemyData::EnemySoldier) {
					GameEnemySoldier *soldier = qobject_cast<GameEnemySoldier *>(ep);
					if (soldier) {
						QStringList slist = m_gameData.value("soldiers").toStringList();
						int x = QRandomGenerator::global()->bounded(slist.size());
						soldier->setSoldierType(slist.at(x));
					}
				}

				QMetaObject::invokeMethod(enemy, "loadSprites", Qt::DirectConnection);

				if (ep) {
					ep->setEnemyData(data);
					ep->setBlock(it.key());

					createdEnemies.append(ep);

					if (m_activity) {
						connect(ep, &GameEnemy::killed, m_activity, &GameActivity::onEnemyKilled);
						connect(ep, &GameEnemy::killMissed, m_activity, &GameActivity::onEnemyKillMissed);
					}

					connect(ep, &GameEnemy::killed, this, &CosGame::recalculateActiveEnemies);
				}

				resetEnemy(data);

				enemyDataList.append(data);
			}
		}

		setPickables(&enemyDataList, it.key());

		allEnemyDataList.append(enemyDataList);

		block->recalculateActiveEnemies();
	}

	setPickables(&allEnemyDataList, -1);


	if (m_activity)
		m_activity->createTargets(createdEnemies);



	recalculateActiveEnemies();

	if (createdEnemies.size() > 1)
		emit gameMessageSent(tr("%1 more objectives created").arg(createdEnemies.size()), 2);
	else if (createdEnemies.size() == 1)
		emit gameMessageSent(tr("1 more objective created"), 2);
}

/**
 * @brief CosGame::resetEnemy
 * @param enemyData
 */

void CosGame::resetEnemy(GameEnemyData *enemyData)
{
	qDebug() << "Reset enemy" << enemyData;

	if (!enemyData) {
		qWarning() << "Invalid enemy item";
		return;
	}

	QQuickItem *item = enemyData->enemy();

	if (!item) {
		qWarning() << "Missing enemy QQuickItem*" << enemyData;
		return;
	}

	qreal x = enemyData->boundRect().left();
	qreal y = enemyData->boundRect().top();

	x += enemyData->boundRect().toRect().width()/2;

	bool facingLeft = true;

	if (x+item->width() > enemyData->boundRect().right()) {
		x = enemyData->boundRect().right()-item->width();
	} else {
		if (QRandomGenerator::global()->generate() % 2 == 1)
			facingLeft = false;
	}

	item->setX(x);
	item->setY(y-item->height());
	item->setProperty("facingLeft", facingLeft);

	enemyData->setActive(true);
}


/**
 * @brief CosGame::setEnemiesMoving
 * @param moving
 */

void CosGame::setEnemiesMoving(const bool &moving)
{
	if (!m_terrainData)
		return;

	foreach (GameEnemyData *data, m_terrainData->enemies()) {
		GameEnemy *e = data->enemyPrivate();
		if (e)
			e->setMoving(moving);
	}
}




/**
 * @brief CosGame::deathlyAttackDistance
 * @return
 */

qreal CosGame::deathlyAttackDistance()
{
	QVariantMap m = m_gameData.value("level", QVariantMap()).toMap();

	if (m.isEmpty() || !m_gameMatch)
		return 10;

	int level = m_gameMatch->level();

	m = m.value(QVariant(level).toString(), QVariantMap()).toMap();

	if (m.isEmpty())
		return 10;

	m = m.value("player", QVariantMap()).toMap();

	if (m.isEmpty())
		return 10;

	return m.value("deathlyAttackDistance", 10).toReal();
}



/**
 * @brief CosGame::resetPlayerCharacter
 */

void CosGame::resetPlayer()
{
	if (!m_gameScene || !m_terrainData || m_player || !m_gameMatch) {
		qWarning() << "Invalid scene or terrain or player exists";
		return;
	}

	QMetaObject::invokeMethod(m_gameScene, "createPlayer", Qt::DirectConnection,
							  Q_RETURN_ARG(QQuickItem *, m_player));

	if (!m_player) {
		emit playerChanged(m_player);
		qWarning() << "Cannot create player";
		return;
	}

	QPointF p(-1, -1);

	int startBlock = m_gameMatch->startBlock();

	if (m_playerStartPosition) {
		p = QPointF(m_playerStartPosition->x(), m_playerStartPosition->y());
	} else if (!m_playerStartPosition && !m_terrainData->blocks().isEmpty()) {
		if (m_terrainData->blocks().contains(startBlock)) {
			GameBlock *block = m_terrainData->blocks().value(startBlock);
			if (!block->playerPositions().isEmpty())
				p = block->playerPositions().at(0);
		} else {
			GameBlock *block = m_terrainData->blocks().first();
			if (!block->playerPositions().isEmpty())
				p = block->playerPositions().at(0);
		}
	}

	if (p != QPointF(-1, -1)) {
		m_player->setX(p.x()-m_player->width());
		m_player->setY(p.y()-m_player->height());
	} else {
		qWarning() << "Available player position not found";
	}


	int startHp = m_gameMatch->startHp();

	GamePlayer *pl = qvariant_cast<GamePlayer *>(m_player->property("entityPrivate"));
	if (pl) {
		connect(pl, &GamePlayer::die, this, &CosGame::onPlayerDied);
		pl->setDefaultHp(startHp);
		pl->setHp(startHp);
		pl->setIsAlive(true);
	} else {
		qWarning() << "Invalid cast" << m_player;
	}

	emit playerChanged(m_player);

	setEnemiesMoving(true);
}



/**
 * @brief CosGame::setGameData
 * @param gameData
 */

void CosGame::setGameData(QVariantMap gameData)
{
	if (m_gameData == gameData)
		return;

	m_gameData = gameData;
	emit gameDataChanged(m_gameData);
	emit levelDataChanged(levelData());
}




/**
 * @brief CosGame::setLastPosition
 * @param object
 */

void CosGame::setLastPosition()
{
	QObject *o = sender();

	if (!o) {
		qWarning() << "Invalid sender";
		return;
	}

	Box2DFixture *fixture = qobject_cast<Box2DFixture *>(o);

	if (!fixture) {
		qWarning() << "Invalid fixture" << o;
		return;
	}

	Box2DBody *body = fixture->getBody();

	if (!body) {
		qWarning() << "Invalid body" << fixture;
		return;
	}

	GameObject *item = qobject_cast<GameObject *>(body->target());

	if (!item) {
		qWarning() << "Invalid target" << fixture;
		return;
	}

	setPlayerStartPosition(item);
}



void CosGame::setGameScene(QQuickItem *gameScene)
{
	if (m_gameScene == gameScene)
		return;

	m_gameScene = gameScene;
	emit gameSceneChanged(m_gameScene);
}



void CosGame::setPlayerStartPosition(GameObject *playerStartPosition)
{
	if (m_playerStartPosition == playerStartPosition)
		return;

	m_playerStartPosition = playerStartPosition;
	emit playerStartPositionChanged(m_playerStartPosition);
}

void CosGame::setRunning(bool running)
{
	if (m_running == running)
		return;

	m_running = running;
	emit runningChanged(m_running);
}

void CosGame::setItemPage(QQuickItem *itemPage)
{
	if (m_itemPage == itemPage)
		return;

	m_itemPage = itemPage;
	emit itemPageChanged(m_itemPage);
}



void CosGame::setGameMatch(GameMatch *gameMatch)
{
	if (m_gameMatch == gameMatch)
		return;

	m_gameMatch = gameMatch;
	emit gameMatchChanged(m_gameMatch);
	emit levelDataChanged(levelData());

	if (m_gameMatch) {
		m_gameMatch->setParent(this);
		m_matchTimer->start(10000);
	} else {
		m_matchTimer->stop();
	}
}



void CosGame::setIsPrepared(bool isPrepared)
{
	if (m_isPrepared == isPrepared)
		return;

	m_isPrepared = isPrepared;
	emit isPreparedChanged(m_isPrepared);
}

void CosGame::setActivity(GameActivity *activity)
{
	if (m_activity == activity)
		return;

	m_activity = activity;

	if (m_activity) {
		connect(m_activity, &GameActivity::prepareSucceed, this, &CosGame::startGame);
	}

	emit activityChanged(m_activity);
}

void CosGame::setActiveEnemies(int activeEnemies)
{
	if (m_activeEnemies == activeEnemies)
		return;

	m_activeEnemies = activeEnemies;
	emit activeEnemiesChanged(m_activeEnemies);
}

void CosGame::setIsStarted(bool isStarted)
{
	if (m_isStarted == isStarted)
		return;

	m_isStarted = isStarted;
	emit isStartedChanged(m_isStarted);
}

void CosGame::setBackgroundMusicFile(QString backgroundMusicFile)
{
	if (m_backgroundMusicFile == backgroundMusicFile)
		return;

	m_backgroundMusicFile = backgroundMusicFile;
	emit backgroundMusicFileChanged(m_backgroundMusicFile);
}


/**
 * @brief CosGame::onLayersLoaded
 */

void CosGame::startGame()
{
	qDebug() << "START GAME";
	m_msecLeft = m_gameMatch->duration()*1000;
	emit msecLeftChanged(m_msecLeft);

	loadPickables();

	recreateEnemies();

	setIsPrepared(true);
	setRunning(true);
}


/**
 * @brief CosGame::abortGame
 */

void CosGame::abortGame()
{
	qDebug() << "ABORT GAME";
	setRunning(false);
	emit gameAbortRequest();
}


/**
 * @brief CosGame::resetHp
 */

void CosGame::resetHp()
{
	if (!m_player || !m_gameMatch) {
		qWarning() << "Invalid player or game match";
		return;
	}

	GamePlayer *pl = qvariant_cast<GamePlayer *>(m_player->property("entityPrivate"));
	if (!pl) {
		qWarning() << "Invalid player";
		return;
	}

	pl->setHp(m_gameMatch->startHp());
}



/**
 * @brief CosGame::addSecs
 * @param secs
 */

void CosGame::addSecs(const int &secs)
{
	m_msecLeft += secs*1000;
}




/**
 * @brief CosGame::onPlayerDied
 */

void CosGame::onPlayerDied()
{
	qDebug() << "Player died";

	if (m_question)
		m_question->forceDestroy();

	setPlayer(nullptr);
	recreateEnemies();
	resetPlayer();
}



/**
 * @brief CosGame::resetRunning
 */

void CosGame::resetRunning()
{
	if (gameState() == Bacon2D::Running) {
		setRunning(true);
	} else {
		setRunning(false);
	}
}


/**
 * @brief CosGame::recalculateBlocks
 */

void CosGame::recalculateActiveEnemies()
{
	if (!m_terrainData)
		return;

	int active = 0;

	foreach (GameEnemyData *enemy, m_terrainData->enemies()) {
		if (enemy->active())
			active++;
	}

	setActiveEnemies(active);

	if (active == 0 && !m_terrainData->enemies().isEmpty()) {
		emit gameCompleted();
	}
}



/**
 * @brief CosGame::onTimerTimeout
 */

void CosGame::onTimerTimeout()
{
	m_msecLeft -= m_timer->interval();

	if (m_msecLeft <= 0) {
		m_timer->stop();
		m_msecLeft = 0;
		emit gameTimeout();
	}

	emit msecLeftChanged(m_msecLeft);
}


/**
 * @brief CosGame::onGameMatchTimerTimeout
 */

void CosGame::onGameMatchTimerTimeout()
{
	if (!m_gameMatch || !m_activity || !m_activity->client())
		return;

	int gameId = m_gameMatch->gameId();

	if (gameId == -1)
		return;

	Client *client = m_activity->client();

	QJsonObject o;
	o["id"] = gameId;
	o["xp"] = m_gameMatch->xp();
	client->socketSend(CosMessage::ClassStudent, "gameUpdate", o);
}




/**
 * @brief CosGame::onGameStarted
 */

void CosGame::onGameStarted()
{
	resetPlayer();
	m_timer->start(100);
	setIsStarted(true);
}


/**
 * @brief CosGame::onGameFinishedSuccess
 */

void CosGame::onGameFinishedSuccess()
{
	m_timer->stop();
	m_matchTimer->stop();

	if (!m_gameMatch || !m_activity || !m_activity->client())
		return;

	emit m_gameMatch->gameWin(m_gameMatch->missionUuid(), m_gameMatch->level());

	Client *client = m_activity->client();

	client->stopSound(m_backgroundMusicFile);
	QTimer::singleShot(400, [=]() {
		client->playSound("qrc:/sound/sfx/win.ogg", CosSound::GameSound);
		client->playSound("qrc:/sound/voiceover/game_over.ogg", CosSound::VoiceOver);
		client->playSound("qrc:/sound/voiceover/you_win.ogg", CosSound::VoiceOver);
	});

	int gameId = m_gameMatch->gameId();

	if (gameId == -1)
		return;


	QJsonObject o;
	o["id"] = gameId;
	o["xp"] = m_gameMatch->xp();
	o["success"] = true;
	client->socketSend(CosMessage::ClassStudent, "gameFinish", o);
}


/**
 * @brief CosGame::onGameFinishedLost
 */

void CosGame::onGameFinishedLost()
{
	m_timer->stop();
	m_matchTimer->stop();

	if (!m_gameMatch || !m_activity || !m_activity->client())
		return;

	emit m_gameMatch->gameLose(m_gameMatch->missionUuid(), m_gameMatch->level());

	Client *client = m_activity->client();

	client->playSound("qrc:/sound/voiceover/game_over.ogg", CosSound::VoiceOver);
	client->playSound("qrc:/sound/voiceover/you_lose.ogg", CosSound::VoiceOver);

	int gameId = m_gameMatch->gameId();

	if (gameId == -1)
		return;


	QJsonObject o;
	o["id"] = gameId;
	o["xp"] = m_gameMatch->xp();
	o["success"] = false;
	client->socketSend(CosMessage::ClassStudent, "gameFinish", o);
}









/**
 * @brief CosGame::setPlayer
 * @param player
 */

void CosGame::setPlayer(QQuickItem *player)
{
	if (m_player == player)
		return;

	m_player = player;

	emit playerChanged(m_player);
}



/**
 * @brief CosGame::loadGameData
 */

void CosGame::loadGameData()
{
	QVariant v = Client::readJsonFile(QString("qrc:/internal/game/parameters.json"));

	if (!v.isValid()) {
		qWarning() << "Invalid json data";
		return;
	}

	QVariantMap m = v.toMap();

	m["soldiers"] = loadSoldierData();

	setGameData(m);
}



/**
 * @brief CosGame::loadSoldierData
 * @param map
 */

QStringList CosGame::loadSoldierData()
{
	QDirIterator it(":/soldiers", {"data.json"}, QDir::Files, QDirIterator::Subdirectories);

	QStringList list;

	while (it.hasNext()) {
		QString realname = it.next();
		list.append(realname.section('/',-2,-2));
	}

	return list;
}


/**
 * @brief CosGame::loadPickables
 */

void CosGame::loadPickables()
{
	if (!m_gameMatch)
		return;

	GameMap::MissionLevel *level = m_gameMatch->missionLevel();

	if (!level)
		return;

	QHash<QByteArray, GameEnemyData::InventoryType> inventoryTypes = GameEnemyData::inventoryTypes();

	foreach (GameMap::Inventory *inventory, level->inventories()) {
		QByteArray module = inventory->module();
		if (!inventoryTypes.contains(module)) {
			qWarning() << "Invalid inventory module" << module;
			continue;
		}

		int block = inventory->block();
		int count = inventory->count();
		GameEnemyData::InventoryType t = inventoryTypes.value(module);

		if (block < 1)
			block = -1;

		if (!m_inventoryPickableList.contains(block)) {
			m_inventoryPickableList[block] = QVector<GameInventoryPickable>();
		}

		m_inventoryPickableList[block].append(GameInventoryPickable(t.type, t.data, count));
	}
}



/**
 * @brief CosGame::setPickables
 * @param enemyList
 * @param block
 */

void CosGame::setPickables(QVector<GameEnemyData *> *enemyList, const int &block)
{
	if (!enemyList || !m_inventoryPickableList.contains(block))
		return;

	QVector<GameInventoryPickable> &list = m_inventoryPickableList[block];

	while (!enemyList->isEmpty()) {
		bool hasPickable = false;

		for (int i=0; i<list.size() && !enemyList->isEmpty(); i++) {
			if (!list[i].count)
				continue;

			GameEnemyData *e = enemyList->takeAt(QRandomGenerator::global()->bounded(enemyList->size()));

			e->setPickableType(list[i].type);
			e->setPickableData(list[i].data);

			list[i].count--;

			if (list[i].count)
				hasPickable = true;
		}

		if (!hasPickable)
			break;
	}

	return;
}





/**
 * @brief CosGame::setTerrainData
 * @param terrainData
 */

void CosGame::addTerrainData(QList<TiledPaintedLayer *> *tiledLayers, QQuickItem *tiledLayersParent)
{
	if (m_terrainData) {
		qWarning() << "Terrain data already loaded";
		return;
	}

	m_terrainData = new GameTerrain(tiledLayers, tiledLayersParent, this);
}


/**
 * @brief CosGame::levelData
 * @return
 */

QVariantMap CosGame::levelData() const
{
	int level = m_gameMatch ? m_gameMatch->level() : 1;
	QVariantMap data = m_gameData.value("level").toMap();

	while (!data.isEmpty() && level > 0) {
		QString key = QVariant(level).toString();
		if (data.contains(key)) {
			return data.value(key).toMap();
		}

		--level;
	}

	return QVariantMap();
}


/**
 * @brief CosGame::loadTerrainData
 */

bool CosGame::loadTerrainData()
{
	if (!m_terrainData) {
		qWarning() << tr("Missing terrain data");
		return false;
	}

	if (!m_gameMatch || m_gameMatch->terrain().isEmpty())
		return false;

	QString terrain = m_gameMatch->terrain();

	qDebug() << "Load terrain data" << terrain;


	if (!m_terrainData->loadTmxFile(QString(":/terrain/"+terrain+"/terrain.tmx"))) {
		qWarning() << "Terrain data load failed";
		return false;
	}

	QString datafile = ":/terrain/"+terrain+"/data.json";

	if (QFile::exists(datafile)) {
		QVariantMap m = Client::readJsonFile(datafile).toMap();
		QString bgMusic = m.value("backgroundMusic").toString();

		if (!bgMusic.isEmpty())
			setBackgroundMusicFile("qrc:/terrain/"+terrain+"/"+bgMusic);
	}

	return true;
}







/**
 * @brief CosGame::tryAttack
 * @param enemy
 */

void CosGame::tryAttack(GamePlayer *player, GameEnemy *enemy)
{
	qDebug() << player << "--- ATTACK ---" << enemy;

	if (!player || !enemy) {
		qWarning() << "Invalid player or invalid enemy";
		return;
	}

	if (m_question) {
		qWarning() << "Question already exists";
		return;
	}

	m_question = new GameQuestion(this, player, enemy, this);
	connect(m_question, &GameQuestion::finished, this, [=]() {
		m_question->deleteLater();
		m_question = nullptr;
		emit questionChanged(nullptr);
	});
	emit questionChanged(m_question);

	m_question->run();
}



