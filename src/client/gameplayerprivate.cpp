/*
 * ---- Call of Suli ----
 *
 * gameplayerprivate.cpp
 *
 * Created on: 2020. 10. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GamePlayerPrivate
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
#include <QDir>

#include "entity.h"
#include "box2dbody.h"
#include "box2dfixture.h"
#include "cosclient.h"
#include "gameplayerprivate.h"

GamePlayerPrivate::GamePlayerPrivate(QQuickItem *parent)
	: GameEntityPrivate(parent)
	, m_ladderMode(LadderUnavaliable)
	, m_ladder(nullptr)
{
	connect(this, &GameEntityPrivate::cosGameChanged, this, &GamePlayerPrivate::onCosGameChanged);
	connect(this, &GamePlayerPrivate::bodyBeginContact, this, &GamePlayerPrivate::onBodyBeginContact);
	connect(this, &GamePlayerPrivate::bodyEndContact, this, &GamePlayerPrivate::onBodyEndContact);
}





/**
 * @brief GamePlayerPrivate::loadQrcData
 */

void GamePlayerPrivate::setQrcDir()
{
	CosGame *game = cosGame();

	if (!game)
		return;

	QString playerCharacter = game->playerCharacter();

	if (playerCharacter.isEmpty())
		return;

	QString dir = "qrc:/character/"+playerCharacter;

	setQrcDirName(dir);
}


/**
 * @brief GamePlayerPrivate::createFixtures
 */

void GamePlayerPrivate::createFixtures()
{
	qDebug() << "Game player private: create fixtures()";

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
	box->setCategories(Box2DFixture::Category1);
	box->setCollidesWith(Box2DFixture::Category1);

	f.append(&f, box);

	setBoundBox(box);




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
}


/**
 * @brief GamePlayerPrivate::ladderClimbUp
 */

void GamePlayerPrivate::ladderClimbUp()
{
	if (!m_ladder || !parentEntity())
		return;

	QObject *spriteSequence = qvariant_cast<QObject *>(parentEntity()->property("spriteSequence"));

	if (m_ladderMode == LadderUpAvailable) {
		int x = m_ladder->boundRect().x()+(m_ladder->boundRect().width()-parentEntity()->width())/2;
		parentEntity()->setX(x);
		setLadderMode(LadderClimb);
		if (spriteSequence)
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbup"));
	} else if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish) {
		int y = parentEntity()->y()-m_qrcData.value("climb", 1).toInt();
		parentEntity()->setY(y);
		if (y < m_ladder->boundRect().top() && spriteSequence && m_ladderMode == LadderClimb) {
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbupend"));
			setLadderMode(LadderClimbFinish);
		}
	}
}


/**
 * @brief GamePlayerPrivate::ladderClimbDown
 */

void GamePlayerPrivate::ladderClimbDown()
{
	if (!m_ladder || !parentEntity())
		return;

	QObject *spriteSequence = qvariant_cast<QObject *>(parentEntity()->property("spriteSequence"));

	if (m_ladderMode == LadderDownAvailable) {
		int x = m_ladder->boundRect().x()+(m_ladder->boundRect().width()-parentEntity()->width())/2;
		parentEntity()->setX(x);
		setLadderMode(LadderClimb);
		if (spriteSequence)
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbdown"));
	}  else if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish) {
		int delta = m_qrcData.value("climb", 1).toInt();
		int y = parentEntity()->y()+delta;
		int height = parentEntity()->height();

		if (m_ladderMode == LadderClimb) {
			if (y+height >= m_ladder->boundRect().bottom() && spriteSequence) {
				QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbdownend"));
				setLadderMode(LadderClimbFinish);
			} else {
				parentEntity()->setY(y);
			}
		}
	}
}


/**
 * @brief GamePlayerPrivate::ladderClimbFinish
 */

void GamePlayerPrivate::ladderClimbFinish()
{
	if (!m_ladder || !parentEntity())
		return;

	setLadder(nullptr);
	setLadderMode(LadderUnavaliable);
}


/**
 * @brief GamePlayerPrivate::onBodyBeginContact
 * @param other
 */

void GamePlayerPrivate::onBodyBeginContact(Box2DFixture *other)
{
	QVariant object = other->property("targetObject");
	QVariantMap data = other->property("targetData").toMap();

	if (!object.isValid())
		return;

	GameLadder *ladder = qvariant_cast<GameLadder *>(object);

	if (ladder && m_ladderMode == LadderUnavaliable) {
		QString dir = data.value("direction").toString();
		if (dir == "up") {
			setLadder(ladder);
			setLadderMode(LadderUpAvailable);
		} else if (dir == "down") {
			setLadder(ladder);
			setLadderMode(LadderDownAvailable);
		} else {
			setLadder(nullptr);
			setLadderMode(LadderUnavaliable);
		}
	}

}


/**
 * @brief GamePlayerPrivate::onBodyEndContact
 * @param other
 */

void GamePlayerPrivate::onBodyEndContact(Box2DFixture *other)
{
	QVariant object = other->property("targetObject");
	QVariantMap data = other->property("targetData").toMap();

	if (!object.isValid())
		return;

	GameLadder *ladder = qvariant_cast<GameLadder *>(object);

	if (ladder && m_ladderMode != LadderClimb) {
		if (ladder == m_ladder) {
			setLadder(nullptr);
			setLadderMode(LadderUnavaliable);
		}
	}
}

void GamePlayerPrivate::setLadderMode(GamePlayerPrivate::LadderMode ladderMode)
{
	if (m_ladderMode == ladderMode)
		return;

	m_ladderMode = ladderMode;
	emit ladderModeChanged(m_ladderMode);
}

void GamePlayerPrivate::setLadder(GameLadder *ladder)
{
	if (m_ladder == ladder)
		return;

	m_ladder = ladder;
	emit ladderChanged(m_ladder);
}




/**
 * @brief GamePlayerPrivate::onCosGameChanged
 * @param game
 */

void GamePlayerPrivate::onCosGameChanged(CosGame *)
{
	if (!cosGame())
		return;

	setQrcDir();
	loadQrcData();


}


