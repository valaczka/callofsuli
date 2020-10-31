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
#include "scene.h"
#include "box2dbody.h"
#include "gameplayer.h"
#include <QTimer>

GameEnemy::GameEnemy(QQuickItem *parent)
	: GameEntity(parent)
	, m_moving(false)
	, m_armed(false)
	, m_enemyData(nullptr)
	, m_attackTimer(nullptr)
	, m_attackTimerElapsed(0)
	, m_castAttackFraction(0.5)
	, m_player(nullptr)
	, m_attackRunning(false)
	, m_msecBeforeAttack(5000)
	, m_aimedByPlayer(false)
{
	connect(this, &GameEnemy::cosGameChanged, this, &GameEnemy::onGameChanged);

	m_attackTimer = new QTimer(this);
	m_attackTimer->setInterval(100);
	connect(m_attackTimer, &QTimer::timeout, this, &GameEnemy::attackTimerTimeout);

	m_rayCastFlag = Box2DFixture::Category3;
	m_rayCastEnabled = true;

	connect(this, &GameEnemy::rayCastItemsChanged, this, &GameEnemy::onRayCastReported);
}


/**
 * @brief GameEnemy::~GameEnemy
 */

GameEnemy::~GameEnemy()
{
	if (m_attackTimer)
		delete m_attackTimer;
}


/**
 * @brief GameEnemy::msecLeftAttack
 * @return
 */

int GameEnemy::msecLeftAttack() const
{
	if (!m_attackTimer->isActive())
		return -1;

	int r = m_msecBeforeAttack - m_attackTimerElapsed;

	if (r<0)
		r = 0;

	return r;
}


/**
 * @brief GameEnemy::tryAttack
 * @param player
 */

void GameEnemy::tryAttack(GamePlayer *player)
{
	// TODO: question scene

	qDebug() << this << "ATTACK succesful";

	setAimedByPlayer(false);

	emit killed();

	player->attackSuccesful(this);
}





/**
 * @brief GameEnemy::attackTimerTimeout
 */

void GameEnemy::attackTimerTimeout()
{
	m_attackTimerElapsed += m_attackTimer->interval();
	emit msecLeftAttackChanged();

	if (m_attackTimerElapsed >= m_msecBeforeAttack) {
		m_attackTimer->stop();
		if (m_player) {
			emit attackPlayer();
			m_player->killedByEnemy(this);
		}
	}

}


/**
 * @brief GameEnemy::onRayCastReported
 * @param items
 */

void GameEnemy::onRayCastReported(QMultiMap<qreal, QQuickItem *> items)
{
	GamePlayer *player = nullptr;
	qreal fraction = -1.0;

	QMapIterator<qreal, QQuickItem *> i(items);

	while (i.hasNext()) {
		i.next();

		GamePlayer *p = qvariant_cast<GamePlayer *>(i.value()->property("entityPrivate"));

		if (p && p->isAlive()) {
			player = p;
			fraction = i.key();
		}
	}

	setPlayer(player);

	if (player && fraction != -1.0 && fraction < m_castAttackFraction) {
		emit attackPlayer();
		player->killedByEnemy(this);
	}
}



/**
 * @brief GameEnemy::onPlayerDied
 */


void GameEnemy::onPlayerDied()
{
	m_player = nullptr;
	emit playerChanged(m_player);

	m_attackTimer->stop();
	m_attackTimerElapsed = 0;
	emit msecLeftAttackChanged();
}




void GameEnemy::setMoving(bool moving)
{
	if (m_moving == moving)
		return;

	m_moving = moving;
	emit movingChanged(m_moving);
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


void GameEnemy::setCastAttackFraction(qreal castAttackFraction)
{
	if (qFuzzyCompare(m_castAttackFraction, castAttackFraction))
		return;

	m_castAttackFraction = castAttackFraction;
	emit castAttackFractionChanged(m_castAttackFraction);
}

void GameEnemy::setPlayer(GamePlayer *player)
{
	if (m_player == player)
		return;

	if (m_player) {
		disconnect(m_player, &GamePlayer::die, this, &GameEnemy::onPlayerDied);
	}

	m_player = player;
	emit playerChanged(m_player);

	m_attackTimer->stop();
	m_attackTimerElapsed = 0;
	emit msecLeftAttackChanged();

	if (m_player) {
		connect(m_player, &GamePlayer::die, this, &GameEnemy::onPlayerDied);
		m_attackTimer->start();
	}

}

void GameEnemy::setAttackRunning(bool attackRunning)
{
	if (m_attackRunning == attackRunning)
		return;

	m_attackRunning = attackRunning;
	emit attackRunningChanged(m_attackRunning);
}

void GameEnemy::setMsecBeforeAttack(int msecBeforeAttack)
{
	if (m_msecBeforeAttack == msecBeforeAttack)
		return;

	m_msecBeforeAttack = msecBeforeAttack;
	emit msecBeforeAttackChanged(m_msecBeforeAttack);
	emit msecLeftAttackChanged();
}

void GameEnemy::setAimedByPlayer(bool aimedByPlayer)
{
	if (m_aimedByPlayer == aimedByPlayer)
		return;

	m_aimedByPlayer = aimedByPlayer;
	emit aimedByPlayerChanged(m_aimedByPlayer);
}



/**
 * @brief GameEnemy::onGameChanged
 */

void GameEnemy::onGameChanged()
{
	if (!m_cosGame)
		return;


	QVariantMap m = m_cosGame->gameData().value("level", QVariantMap()).toMap();

	if (m.isEmpty())
		return;

	m = m.value(QVariant(m_cosGame->level()).toString(), QVariantMap()).toMap();

	if (m.isEmpty())
		return;

	QVariantMap me = m.value("enemy").toMap();

	onGameDataReady(me);

}
