/*
 * ---- Call of Suli ----
 *
 * gameenemyprivate.cpp
 *
 * Created on: 2020. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemyPrivate
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

#include "gameenemy.h"

GameEnemy::GameEnemy(QQuickItem *parent)
	: GameEntity(parent)
	, m_moving(false)
	, m_notice(false)
	, m_armed(false)
	, m_enemyData(nullptr)
{

}

void GameEnemy::setMoving(bool moving)
{
	if (m_moving == moving)
		return;

	m_moving = moving;
	emit movingChanged(m_moving);
}

void GameEnemy::setNotice(bool notice)
{
	if (m_notice == notice)
		return;

	m_notice = notice;
	emit noticeChanged(m_notice);
}

void GameEnemy::setArmed(bool armed)
{
	if (m_armed == armed)
		return;

	m_armed = armed;
	emit armedChanged(m_armed);
}

void GameEnemy::setEnemyData(GameEnemyData *enemyData)
{
	if (m_enemyData == enemyData)
		return;

	m_enemyData = enemyData;
	emit enemyDataChanged(m_enemyData);
}
