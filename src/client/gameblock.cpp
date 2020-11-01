/*
 * ---- Call of Suli ----
 *
 * gameblock.cpp
 *
 * Created on: 2020. 10. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameBlock
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

#include "gameblock.h"
#include "gameenemydata.h"
#include "gameladder.h"


GameBlock::GameBlock(QObject *parent)
	: QObject(parent)
	, m_enemies()
	, m_playerPosition()
	, m_ladders()
	, m_completed(false)
	, m_lastPosition(nullptr)
{

}

GameBlock::~GameBlock()
{
	m_enemies.clear();
	m_ladders.clear();
	m_playerPosition.clear();
}


/**
 * @brief GameBlock::addEnemy
 * @param enemy
 */

void GameBlock::addEnemy(GameEnemyData *enemy)
{
	if (!enemy)
		return;

	m_enemies.append(enemy);
	emit enemiesChanged(m_enemies);

	setCompleted(false);
}


/**
 * @brief GameBlock::addPlayerPosition
 * @param blockFrom
 * @param point
 */

Box2DBox* GameBlock::addPlayerPosition(const QPoint &point, QQuickItem *parent)
{
	GameObject *item = new GameObject(parent);

	qreal w = 50;
	qreal h = 150;
	qreal x = point.x()-w/2;
	qreal y = point.y()-h/2;

	item->setX(x);
	item->setY(y);
	item->setZ(0);
	item->setWidth(w);
	item->setHeight(h);
	item->setVisible(true);
	item->setDensity(1);
	item->setRestitution(0);
	item->setFriction(1);
	item->setSensor(true);
	item->setCategories(Box2DFixture::Category6);
	item->setCollidesWith(Box2DFixture::Category3);
	Box2DBox *box = item->createRectangularFixture();

	m_playerPosition.append(item);
	emit playerPositionChanged(m_playerPosition);

	return box;
}




/**
 * @brief GameBlock::addLadder
 * @param ladder
 */

void GameBlock::addLadder(GameLadder *ladder)
{
	if (!ladder)
		return;

	m_ladders.append(ladder);
	emit laddersChanged(m_ladders);
}


/**
 * @brief GameBlock::setEnemies
 * @param enemies
 */

void GameBlock::setEnemies(QList<GameEnemyData *> enemies)
{
	if (m_enemies == enemies)
		return;

	m_enemies = enemies;
	emit enemiesChanged(m_enemies);
}

void GameBlock::setLadders(QList<GameLadder *> ladders)
{
	if (m_ladders == ladders)
		return;

	m_ladders = ladders;
	emit laddersChanged(m_ladders);
}

void GameBlock::setCompleted(bool completed)
{
	if (m_completed == completed)
		return;

	m_completed = completed;
	emit completedChanged(m_completed);

	if (m_completed) {
		activateLadders();
	}
}

void GameBlock::setLastPosition(GameObject *lastPosition)
{
	if (m_lastPosition == lastPosition)
		return;

	m_lastPosition = lastPosition;
	emit lastPositionChanged(m_lastPosition);
}

/**
 * @brief GameBlock::recalculateActiveEnemies
 */

void GameBlock::recalculateActiveEnemies()
{
	int active = 0;

	foreach (GameEnemyData *enemy, m_enemies) {
		if (enemy->active())
			active++;
	}

	if (active == 0 && !m_enemies.isEmpty())
		setCompleted(true);
}


/**
 * @brief GameBlock::activateLadders
 */

void GameBlock::activateLadders()
{
	foreach (GameLadder *ladder, m_ladders) {
		ladder->setActive(true);
	}
}

