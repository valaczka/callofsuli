/*
 * ---- Call of Suli ----
 *
 * gameenemysoldierprivate.cpp
 *
 * Created on: 2020. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameEnemySoldierPrivate
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

#include "gameenemysoldier.h"
#include "box2dbody.h"
#include "box2dfixture.h"
#include "scene.h"

#include <QTimer>

GameEnemySoldier::GameEnemySoldier(QQuickItem *parent)
	: GameEnemy(parent)
	, m_movingTimer(nullptr)
	, m_msecBeforeTurn(5000)
	, m_atBound(false)
	, m_turnElapsedMsec(-1)
{
	m_movingTimer = new QTimer(this);
	m_movingTimer->setInterval(60);

	connect(m_movingTimer, &QTimer::timeout, this, &GameEnemySoldier::onMovingTimerTimeout);
	connect(this, &GameEnemySoldier::cosGameChanged, this, &GameEnemySoldier::onCosGameChanged);
	connect(this, &GameEnemySoldier::movingChanged, this, [=]() {
		if (m_moving)
			m_movingTimer->start();
		else
			m_movingTimer->stop();
	});

}


/**
 * @brief GameEnemySoldier::~GameEnemySoldier
 */

GameEnemySoldier::~GameEnemySoldier()
{
	if (m_movingTimer)
		delete m_movingTimer;

	m_movingTimer = nullptr;
}


/**
 * @brief GameEnemySoldier::setQrcDir
 */

void GameEnemySoldier::setQrcDir()
{
	CosGame *game = cosGame();

	if (!game)
		return;

	QString dir = "qrc:/character/character2";			// TODO

	setQrcDirName(dir);
}


/**
 * @brief GameEnemySoldier::createFixtures
 */

void GameEnemySoldier::createFixtures()
{
	qDebug() << "Game enemy private: create fixtures()";

	QVariant body = parentEntity()->property("body");

	if (!body.isValid()) {
		qWarning() << "Invalid property: body" << parentEntity();
		return;
	}


	Box2DBody *b2body = qvariant_cast<Box2DBody*>(body);

	if (!b2body) {
		qWarning() << "Invalid variant cast" << body;
		return;
	}

	QQmlListProperty<Box2DFixture> f = b2body->fixtures();


	// Create bound box

	Box2DBox *box = new Box2DBox(parentEntity());

	box->setWidth(10);
	box->setHeight(10);
	box->setX(0);
	box->setY(0);
	box->setSensor(false);


	box->setRestitution(0);
	box->setFriction(0);
	box->setCategories(Box2DFixture::Category5);
	box->setCollidesWith(Box2DFixture::Category1);

	f.append(&f, box);

	setBoundBox(box);


	/*

	// Create body polygon

	Box2DPolygon *polygon = new Box2DPolygon(parentEntity());

	polygon->setSensor(true);
	polygon->setCategories(Box2DFixture::Category2);
	polygon->setCollidesWith(Box2DFixture::Category2|Box2DFixture::Category3);

	QVariantList l;
	l << QVariant(QPoint(0,0))
	  << QVariant(QPoint(10,0))
	  << QVariant(QPoint(10,10))
	  << QVariant(QPoint(0,10));

	polygon->setVertices(l);

	f.append(&f, polygon);

	setBodyPolygon(polygon);


	QQmlListProperty<Box2DFixture> f2 = b2body->fixtures();

	for (int i=0; i<f2.count(&f2); ++i) {
		qDebug() << i << f.at(&f2, i);
	}
	*/
}

void GameEnemySoldier::setMsecBeforeTurn(int msecBeforeTurn)
{
	if (m_msecBeforeTurn == msecBeforeTurn)
		return;

	m_msecBeforeTurn = msecBeforeTurn;
	emit msecBeforeTurnChanged(m_msecBeforeTurn);
}

void GameEnemySoldier::setAtBound(bool atBound)
{
	if (m_atBound == atBound)
		return;

	m_atBound = atBound;
	emit atBoundChanged(m_atBound);
}


/**
 * @brief GameEnemySoldier::onGameDataReady
 * @param map
 */

void GameEnemySoldier::onGameDataReady(const QVariantMap &map)
{
	if (!map.contains("soldier"))
		return;

	QVariantMap m = map.value("soldier").toMap();

	setRayCastElevation(m.value("rayCastElevation", m_rayCastElevation).toReal());
	setRayCastLength(m.value("rayCastLength", m_rayCastLength).toReal());
	setMsecBeforeTurn(m.value("msecBeforeTurn", m_msecBeforeTurn).toInt());
	setCastAttackFraction(m.value("castAttackFraction", m_castAttackFraction).toReal());
	setMsecBeforeAttack(m.value("msecBeforeAttack", m_msecBeforeAttack).toInt());

}



/**
 * @brief GameEnemySoldier::onCosGameChanged
 */

void GameEnemySoldier::onCosGameChanged(CosGame *)
{
	if (!cosGame())
		return;

	setQrcDir();
	loadQrcData();
}


/**
 * @brief GameEnemySoldier::onMovingTimerTimeout
 */

void GameEnemySoldier::onMovingTimerTimeout()
{
	if (!m_enemyData) {
		qWarning() << "Missing enemy data";
		return;
	}

	if (m_cosGame->gameState() != Bacon2D::Running)
		return;

	if (m_player || m_attackRunning || !m_isAlive)
		return;

	Entity *entity = parentEntity();

	bool facingLeft = entity->property("facingLeft").toBool();

	if (m_atBound) {
		m_turnElapsedMsec += m_movingTimer->interval();

		if (m_turnElapsedMsec >= m_msecBeforeTurn) {
			entity->setProperty("facingLeft", !facingLeft);
			setAtBound(false);
			m_turnElapsedMsec = -1;
		}
	} else {
		int x = entity->property("x").toInt();
		int delta = m_qrcData.value("walk", 3).toInt();

		if (facingLeft) {
			if (x-delta < m_enemyData->boundRect().left()) {
				setAtBound(true);
				m_turnElapsedMsec = 0;
			} else {
				entity->setX(x-delta);
			}
		} else {
			if (x+delta > m_enemyData->boundRect().right() - entity->width()) {
				setAtBound(true);
				m_turnElapsedMsec = 0;
			} else {
				entity->setX(x+delta);
			}
		}
	}
}
