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
#include "gameplayer.h"
#include "gamequestion.h"
#include "cosgame.h"

CosGame::CosGame(QQuickItem *parent)
	: Game(parent)
	, m_playerCharacter()
	, m_level(1)
	, m_player(nullptr)
	, m_gameScene(nullptr)
	, m_startHp(1)
	, m_playerStartPosition(nullptr)
	, m_startBlock(0)
	, m_running(true)
	, m_itemPage(nullptr)
	, m_question(nullptr)
	, m_terrainData(nullptr)
{
	connect(this, &Game::gameStateChanged, this, &CosGame::resetRunning);
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
}




/**
 * @brief CosGame::reloadEnemies
 */

void CosGame::recreateEnemies()
{
	if (!m_gameScene || !m_terrainData)
		return;

	qDebug() << "Recreate enemies";

	foreach (GameBlock *block, m_terrainData->blocks()) {
		if (block->completed()) {
			qDebug() << "Block completed" << block;
			continue;
		}

		qDebug() << "Create enemies for block" << block;

		foreach (GameEnemyData *data, block->enemies()) {
			if (data->enemy())
				continue;

			if (QRandomGenerator::global()->generate() % 4) {
				qDebug() << "add question";
				QVariantMap m;
				m.insert("szia", "szia");
				data->setQuestionData(m);
			}

			QQuickItem *enemy = nullptr;

			QMetaObject::invokeMethod(m_gameScene, "createComponent", Qt::AutoConnection,
									  Q_RETURN_ARG(QQuickItem *, enemy),
									  Q_ARG(int, data->enemyType()));

			if (enemy) {
				QMetaObject::invokeMethod(enemy, "loadSprites", Qt::QueuedConnection);
				data->setEnemy(enemy);

				GameEnemy *ep = data->enemyPrivate();
				if (ep) {
					ep->setEnemyData(data);
				}

				resetEnemy(data);
			}
		}

		block->recalculateActiveEnemies();
	}

	QTimer::singleShot(1000, this, [=](){
		setEnemiesMoving(true);
	});

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

	x += QRandomGenerator::global()->generate() % enemyData->boundRect().toRect().width();

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

	if (m.isEmpty())
		return 10;

	m = m.value(QVariant(m_level).toString(), QVariantMap()).toMap();

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
	if (!m_gameScene || !m_terrainData || m_player) {
		qWarning() << "Invalid scene or terrain or player exists";
		return;
	}

	QMetaObject::invokeMethod(m_gameScene, "createPlayer", Qt::AutoConnection,
							  Q_RETURN_ARG(QQuickItem *, m_player));

	if (!m_player) {
		emit playerChanged(m_player);
		qWarning() << "Cannot create player";
		return;
	}

	QPointF p(-1, -1);

	if (m_playerStartPosition) {
		p = QPointF(m_playerStartPosition->x(), m_playerStartPosition->y());
	} else if (!m_playerStartPosition && !m_terrainData->blocks().isEmpty()) {
		if (m_terrainData->blocks().contains(m_startBlock)) {
			GameBlock *block = m_terrainData->blocks().value(m_startBlock);
			if (!block->playerPositions().isEmpty())
				p = block->playerPositions().first();
		} else {
			GameBlock *block = m_terrainData->blocks().first();
			if (!block->playerPositions().isEmpty())
				p = block->playerPositions().first();
		}
	}

	if (p != QPointF(-1, -1)) {
		m_player->setX(p.x()-m_player->width());
		m_player->setY(p.y()-m_player->height());
	} else {
		qWarning() << "Available player position not found";
	}

	GamePlayer *pl = qvariant_cast<GamePlayer *>(m_player->property("entityPrivate"));
	if (pl) {
		connect(pl, &GamePlayer::die, this, &CosGame::onPlayerDied);
		pl->setDefaultHp(m_startHp);
		pl->setHp(m_startHp);
		pl->setIsAlive(true);
	} else {
		qWarning() << "Invalid cast" << m_player;
	}

	emit playerChanged(m_player);
}


/**
 * @brief CosGame::setPlayerCharacter
 * @param playerCharacter
 */

void CosGame::setPlayerCharacter(QString playerCharacter)
{
	if (m_playerCharacter == playerCharacter)
		return;

	m_playerCharacter = playerCharacter;
	emit playerCharacterChanged(m_playerCharacter);
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
}


/**
 * @brief CosGame::setLevel
 * @param level
 */

void CosGame::setLevel(int level)
{
	if (m_level == level)
		return;

	m_level = level;
	emit levelChanged(m_level);
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

void CosGame::setStartBlock(int startBlock)
{
	if (m_startBlock == startBlock)
		return;

	m_startBlock = startBlock;
	emit startBlockChanged(m_startBlock);
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

void CosGame::setQuestion(GameQuestion *question)
{
	if (m_question == question)
		return;

	m_question = question;
	emit questionChanged(m_question);
}

void CosGame::setStartHp(int startHp)
{
	if (m_startHp == startHp)
		return;

	m_startHp = startHp;
	emit startHpChanged(m_startHp);
}


/**
 * @brief CosGame::onLayersLoaded
 */

void CosGame::startGame()
{
	qDebug() << "START GAME";
	recreateEnemies();
	resetPlayer();

	emit gameStarted();
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
 * @brief CosGame::onPlayerDied
 */

void CosGame::onPlayerDied()
{
	qDebug() << "Player died";
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

void CosGame::recalculateBlocks()
{
	if (!m_terrainData)
		return;

	int active = 0;

	foreach (GameEnemyData *enemy, m_terrainData->enemies()) {
		if (enemy->active())
			active++;
	}

	qDebug() << "ACTIVE" << active;

	if (active == 0 && !m_terrainData->enemies().isEmpty()) {
		qDebug() << "************************************* GAME OVER ********************************";
		emit gameCompleted();
	}
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
 * @brief CosGame::setTerrain
 * @param terrain
 */

void CosGame::setTerrain(QString terrain)
{
	if (m_terrain == terrain)
		return;

	m_terrain = terrain;
	emit terrainChanged(m_terrain);
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

	setGameData(v.toMap());
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
 * @brief CosGame::loadTerrainData
 */

bool CosGame::loadTerrainData()
{
	if (!m_terrainData) {
		qWarning() << tr("Missing terrain data");
		return false;
	}

	if (m_terrain.isEmpty())
		return false;

	qDebug() << "Load terrain data" << m_terrain;


	if (!m_terrainData->loadTmxFile(QString(":/terrain/"+m_terrain+"/terrain.tmx"))) {
		qWarning() << "Terrain data load failed";
		return false;
	}

	foreach (GameBlock *block, m_terrainData->blocks()) {
		connect(block, &GameBlock::completedChanged, this, &CosGame::recalculateBlocks);
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
	});

	m_question->run();
}


