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
	, m_msecBetweenAttack(3000)
	, m_aimedByPlayer(false)
	, m_block(0)
	, m_xpGained(false)
{
	connect(this, &GameEnemy::cosGameChanged, this, &GameEnemy::onGameChanged);

	m_attackTimer = new QTimer(this);
	m_attackTimer->setInterval(100);
	connect(m_attackTimer, &QTimer::timeout, this, &GameEnemy::attackTimerTimeout);

	m_rayCastFlag = Box2DFixture::Category3;
	m_rayCastEnabled = true;
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

	if (m_attackRunning)
		return 0;

	int r = m_msecBeforeAttack - m_attackTimerElapsed;

	if (r<0)
		r = 0;

	return r;
}





/**
 * @brief GameEnemy::killByPlayer
 * @param player
 */

void GameEnemy::killByPlayer(GamePlayer *player, const bool &isEmptyQuestion)
{
	decreaseHp();

	qreal playerX = player->parentEntity()->x();
	qreal meX = parentEntity()->x();
	bool facingLeft = parentEntity()->property("facingLeft").toBool();

	if (m_enemyData->enemyType() != GameEnemyData::EnemySniper && (!isEmptyQuestion || !isAlive() || m_cosGame->gameMatch()->level() > 2))
	{
		if (playerX <= meX && !facingLeft)
			parentEntity()->setProperty("facingLeft", true);
		else if (playerX > meX && facingLeft)
			parentEntity()->setProperty("facingLeft", false);
	}

	if (isAlive())
		return;

	setAimedByPlayer(false);

	emit killed(this);
}


/**
 * @brief GameEnemy::missedByPlayer
 * @param player
 */

void GameEnemy::missedByPlayer(GamePlayer *player)
{
	qDebug() << "Missed by player" << this;

	qreal playerX = player->parentEntity()->x();
	qreal meX = parentEntity()->x();
	bool facingLeft = parentEntity()->property("facingLeft").toBool();

	if (playerX <= meX && !facingLeft)
		parentEntity()->setProperty("facingLeft", true);
	else if (playerX > meX && facingLeft)
		parentEntity()->setProperty("facingLeft", false);

	emit killMissed(this);

	player->attackFailed(this);
}





/**
 * @brief GameEnemy::attackTimerTimeout
 */

void GameEnemy::attackTimerTimeout()
{
	if (!m_cosGame || !m_cosGame->running())
		return;

	m_attackTimerElapsed += m_attackTimer->interval();
	emit msecLeftAttackChanged();

	if (m_attackRunning) {
		if (m_attackTimerElapsed >= m_msecBetweenAttack) {
			m_attackTimerElapsed = 0;
			attackPlayer();
		}
	} else {
		if (m_attackTimerElapsed >= m_msecBeforeAttack) {
			m_attackTimerElapsed = 0;
			setAttackRunning(true);
		}
	}
}


/**
 * @brief GameEnemy::attackPlayer
 */

void GameEnemy::attackPlayer()
{
	if (m_player && m_isAlive) {
		emit attack();
		if (m_enemyData->enemyType() == GameEnemyData::EnemySniper)
			m_player->killByEnemy(this);
		else
			m_player->hurtByEnemy(this, true);
	}
}


/**
 * @brief GameEnemy::onRayCastReported
 * @param items
 */

void GameEnemy::rayCastItemsReported(const QMultiMap<qreal, QQuickItem *> &items)
{
	if (!m_isAlive) {
		setPlayer(nullptr);
		return;
	}

	GamePlayer *player = nullptr;
	qreal fraction = -1.0;

	QMapIterator<qreal, QQuickItem *> i(items);

	while (i.hasNext()) {
		i.next();

		GamePlayer *p = qvariant_cast<GamePlayer *>(i.value()->property("entityPrivate"));

		if (p && p->isAlive() && !p->invisible()) {
			player = p;
			fraction = i.key();
		}
	}

	bool newFacingLeft = parentEntity()->property("facingLeft").toBool();

	if (m_player && !player) {
		qreal playerX = m_player->parentEntity()->x();
		qreal meX = parentEntity()->x();

		if (playerX <= meX)
			newFacingLeft = true;
		else if (playerX > meX)
			newFacingLeft = false;
	}

	setPlayer(player);

	if (player && fraction != -1.0) {
		qreal dist = cosGame()->deathlyAttackDistance();
		if (dist > 0 && m_rayCastLength*fraction <= dist) {
			emit attack();
			setAttackRunning(true);
			m_player->killByEnemy(this);
		} else if (fraction < m_castAttackFraction) {
			setAttackRunning(true);
		}
	} else {
		parentEntity()->setProperty("facingLeft", newFacingLeft);
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

	if (m_enemyData) {
		connect(this, &GameEnemy::die, m_enemyData, &GameEnemyData::enemyDied);
		connect(this, &GameEnemy::killed, m_enemyData, &GameEnemyData::enemyKilled);
	}
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
	setAttackRunning(false);
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

	if (m_attackRunning)
		attackPlayer();
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

void GameEnemy::setMsecBetweenAttack(int msecBetweenAttack)
{
	if (m_msecBetweenAttack == msecBetweenAttack)
		return;

	m_msecBetweenAttack = msecBetweenAttack;
	emit msecBetweenAttackChanged(m_msecBetweenAttack);
}

void GameEnemy::setXpGained(bool xpGained)
{
	if (m_xpGained == xpGained)
		return;

	m_xpGained = xpGained;
	emit xpGainedChanged(m_xpGained);
}



/**
 * @brief GameEnemy::onGameChanged
 */

void GameEnemy::onGameChanged()
{
	if (!m_cosGame)
		return;


	/*QVariantMap m = m_cosGame->gameData().value("level", QVariantMap()).toMap();

	if (m.isEmpty())
		return;

	int level = m_cosGame->gameMatch()->level();

	m = m.value(QVariant(level).toString(), QVariantMap()).toMap();

	if (m.isEmpty())
		return;*/

	QVariantMap leveldata = m_cosGame->levelData();

	QVariantMap me = leveldata.value("enemy").toMap();

	onGameDataReady(me);

}

