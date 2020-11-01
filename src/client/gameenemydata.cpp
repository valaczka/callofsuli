/*
 * ---- Call of Suli ----
 *
 * gameenemy.cpp
 *
 * Created on: 2020. 10. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemy
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

#include <QDebug>
#include "gameenemydata.h"
#include "gameenemy.h"

GameEnemyData::GameEnemyData(QObject *parent)
	: QObject(parent)
	, m_boundRect()
	, m_active(false)
	, m_block(nullptr)
	, m_enemy(nullptr)
	, m_enemyType(EnemySoldier)
{

}


/**
 * @brief GameEnemyData::enemyPrivate
 * @return
 */

GameEnemy *GameEnemyData::enemyPrivate() const
{
	return m_enemy ? qvariant_cast<GameEnemy *>(m_enemy->property("entityPrivate")) : nullptr;
}





void GameEnemyData::setBoundRect(QRectF boundRect)
{
	if (m_boundRect == boundRect)
		return;

	m_boundRect = boundRect;
	emit boundRectChanged(m_boundRect);
}

void GameEnemyData::setActive(bool active)
{
	if (m_active == active)
		return;

	m_active = active;
	emit activeChanged(m_active);
}

void GameEnemyData::setBlock(GameBlock *block)
{
	if (m_block == block)
		return;

	m_block = block;
	emit blockChanged(m_block);
}

void GameEnemyData::setEnemy(QQuickItem *enemy)
{
	if (m_enemy == enemy)
		return;

	m_enemy = enemy;
	emit enemyChanged(m_enemy);
}

void GameEnemyData::setEnemyType(GameEnemyData::EnemyType enemyType)
{
	if (m_enemyType == enemyType)
		return;

	m_enemyType = enemyType;
	emit enemyTypeChanged(m_enemyType);
}


/**
 * @brief GameEnemyData::enemyDied
 */

void GameEnemyData::enemyDied()
{
	setEnemy(nullptr);
	setActive(false);
}


/**
 * @brief GameEnemyData::enemyKilled
 */

void GameEnemyData::enemyKilled()
{
	setActive(false);

	if (m_block)
		m_block->recalculateActiveEnemies();
}

