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

#include "cosclient.h"

#include "mapobject.h"
#include "objectgroup.h"

#include "gameenemy.h"
#include "cosgame.h"

CosGame::CosGame(QQuickItem *parent)
	: Game(parent)
	, m_playerCharacter()
	, m_level(1)
	, m_enemies()
	, m_player(nullptr)
	, m_currentBlock(0)
	, m_previousBlock(-1)
	, m_ladders()
{
	loadGameData();
}


/**
 * @brief CosGame::~CosGame
 */

CosGame::~CosGame()
{
	qDeleteAll(m_enemies.begin(), m_enemies.end());
	m_enemies.clear();

	qDeleteAll(m_blocks.begin(), m_blocks.end());
	m_blocks.clear();

	qDeleteAll(m_ladders.begin(), m_ladders.end());
	m_ladders.clear();
}




/**
 * @brief CosGame::addEnemy
 * @param enemy
 */

void CosGame::addEnemy(GameEnemyData *enemy)
{
	if (!enemy)
		return;

	m_enemies.append(enemy);

	int blocknum = enemy->block();
	if (blocknum > 0 && m_blocks.contains(blocknum))
		m_blocks.value(blocknum)->addEnemy(enemy);


	emit enemiesChanged(m_enemies);
	emit activeEnemiesChanged(activeEnemies());
}



/**
 * @brief CosGame::addPlayerPosition
 */

void CosGame::addPlayerPosition(const int &block, const int &blockFrom, const int &x, const int &y)
{
	if (!m_blocks.contains(block)) {
		qWarning() << "Invalid block" << block;
		return;
	}

	QPoint p(x,y);
	m_blocks[block]->addPlayerPosition(blockFrom, p);
}



/**
 * @brief CosGame::reloadEnemies
 */

void CosGame::recreateEnemies(QQuickItem *scene)
{
	if (!scene)
		return;

	qDebug() << "Recreate enemies";

	foreach (GameEnemyData *data, m_enemies) {
		if (data->enemy())
			continue;

		QQuickItem *enemy = nullptr;

		QMetaObject::invokeMethod(scene, "createComponent", Qt::AutoConnection,
								  Q_RETURN_ARG(QQuickItem *, enemy),
								  Q_ARG(int, data->enemyType()));

		if (enemy) {
			QMetaObject::invokeMethod(enemy, "loadSprites", Qt::QueuedConnection);
			data->setEnemy(enemy);

			if (data->enemyPrivate())
				data->enemyPrivate()->setEnemyData(data);

			resetEnemy(data);
		}
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

	int x = enemyData->boundRect().left();
	int y = enemyData->boundRect().top();

	x += qrand() % enemyData->boundRect().width();

	bool facingLeft = true;

	if (x+item->width() > enemyData->boundRect().right()) {
		x = enemyData->boundRect().right()-item->width();
	} else {
		if (qrand() % 2 == 1)
			facingLeft = false;
	}

	item->setX(x);
	item->setY(y-item->height());
	item->setProperty("facingLeft", facingLeft);
	/*QObject *spriteSequence = qvariant_cast<QObject *>(item->property("spriteSequence"));

	if (spriteSequence)
		QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "idle"));*/

}


/**
 * @brief CosGame::setEnemiesMoving
 * @param moving
 */

void CosGame::setEnemiesMoving(const bool &moving)
{
	foreach (GameEnemyData *data, m_enemies) {
		data->enemyPrivate()->setMoving(moving);
	}
}



/**
 * @brief CosGame::addLadder
 * @param ladder
 */

void CosGame::addLadder(GameLadder *ladder)
{
	if (!ladder)
		return;

	m_ladders.append(ladder);

	int blockTop = ladder->blockTop();
	if (blockTop > 0 && m_blocks.contains(blockTop))
		m_blocks.value(blockTop)->addLadder(ladder);

	int blockBottom = ladder->blockBottom();
	if (blockBottom > 0 && m_blocks.contains(blockBottom))
		m_blocks.value(blockBottom)->addLadder(ladder);

	emit laddersChanged(m_ladders);
}



/**
 * @brief CosGame::activeEnemies
 * @return
 */

int CosGame::activeEnemies() const
{
	int num = 0;
	foreach (GameEnemyData *e, m_enemies) {
		if (e->active())
			num++;
	}
	return num;
}


/**
 * @brief CosGame::placePlayerCharacter
 */

void CosGame::placePlayer()
{
	qDebug() << "placePlayer()";

	if (!m_player) {
		qWarning() << "Invalid player item";
		return;
	}

	int x = -1;
	int y = -1;

	if (m_blocks.contains(m_currentBlock)) {
		GameBlock *block = m_blocks.value(m_currentBlock);

		if (block->playerPosition().contains(m_previousBlock)) {
			QPoint p = block->playerPosition().value(m_previousBlock);
			x = p.x();
			y = p.y();

			qDebug() << "Player position:" << m_currentBlock << m_previousBlock << x << y;
		}
	}

	if (x == -1 && y == -1) {
		qInfo() << "No player position found";

		foreach (GameBlock *block, m_blocks) {
			qDebug() << "-- block" << block;

			foreach (QPoint p, block->playerPosition()) {
				qDebug() << "---- point" << p;
				if (p.x() > -1 && p.y() > -1) {
					x = p.x();
					y = p.y();
					break;
				}
			}

			if (x>-1 && y>-1)
				break;
		}

		qDebug() << "First available position" << x << y;
	}

	m_player->setX(x-m_player->width());
	m_player->setY(y-m_player->height());
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

void CosGame::setEnemies(QList<GameEnemyData *> enemies)
{
	if (m_enemies == enemies)
		return;

	m_enemies = enemies;
	emit enemiesChanged(m_enemies);
	emit activeEnemiesChanged(activeEnemies());
}

void CosGame::setBlocks(QMap<int, GameBlock *> blocks)
{
	if (m_blocks == blocks)
		return;

	m_blocks = blocks;
	emit blocksChanged(m_blocks);
}

void CosGame::setCurrentBlock(int currentBlock)
{
	if (m_currentBlock == currentBlock)
		return;

	m_currentBlock = currentBlock;
	emit currentBlockChanged(m_currentBlock);
}

void CosGame::setPreviousBlock(int previousBlock)
{
	if (m_previousBlock == previousBlock)
		return;

	m_previousBlock = previousBlock;
	emit previousBlockChanged(m_previousBlock);
}

void CosGame::setLadders(QList<GameLadder *> ladders)
{
	if (m_ladders == ladders)
		return;

	m_ladders = ladders;
	emit laddersChanged(m_ladders);
}


/**
 * @brief CosGame::setPlayer
 * @param player
 */

void CosGame::setPlayer(QQuickItem *player)
{
	qDebug() << "set player" << player;

	if (m_player == player)
		return;

	m_player = player;

	placePlayer();

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


void CosGame::setTerrainData(QVariantMap terrainData)
{
	if (m_terrainData == terrainData)
		return;

	m_terrainData = terrainData;
	emit terrainDataChanged(m_terrainData);
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
 * @brief CosGame::loadTerrainData
 */

void CosGame::loadTerrainData()
{
	if (m_terrain.isEmpty())
		return;

	if (!m_terrainData.isEmpty()) {
		qWarning() << "Terrain data already loaded";
		return;
	}

	QVariant v = Client::readJsonFile(QString("qrc:/terrain/"+m_terrain+"/data.json"));

	if (!v.isValid()) {
		qWarning() << "Invalid json data";
		return;
	}

	setTerrainData(v.toMap());

	loadBlocks();
}


/**
 * @brief CosGame::loadBlocks
 */

void CosGame::loadBlocks()
{
	qDebug() << "LOAD BLOCKS";

	qDeleteAll(m_blocks.begin(), m_blocks.end());
	m_blocks.clear();

	int block_count = m_terrainData.value("blocks", 0).toInt();

	for (int i=0; i<block_count; i++) {
		int num = i+1;

		GameBlock *block = new GameBlock(this);
		m_blocks.insert(num, block);
	}

	emit blocksLoaded();
}



