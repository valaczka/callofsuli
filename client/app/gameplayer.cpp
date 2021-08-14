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
#include <QTimer>

#include "entity.h"
#include "box2dbody.h"
#include "box2dfixture.h"
#include "cosclient.h"
#include "gameplayer.h"
#include "gameenemy.h"

GamePlayer::GamePlayer(QQuickItem *parent)
	: GameEntity(parent)
	, m_ladderMode(LadderUnavaliable)
	, m_isLadderDirectionUp(false)
	, m_ladder(nullptr)
	, m_enemy(nullptr)
	, m_defaultHp(0)
	, m_soundEffectRunNum(1)
	, m_soundEffectClimbNum(1)
	, m_soundEffectWalkNum(1)
	, m_soundEffectPainNum(1)
	, m_shield(0)
{
	connect(this, &GameEntity::cosGameChanged, this, &GamePlayer::onCosGameChanged);
	connect(this, &GamePlayer::bodyBeginContact, this, &GamePlayer::onBodyBeginContact);
	connect(this, &GamePlayer::bodyEndContact, this, &GamePlayer::onBodyEndContact);

	m_rayCastFlag = Box2DFixture::Category5;

	connect(this, &GamePlayer::rayCastItemsChanged, this, &GamePlayer::onRayCastReported);
}


/**
 * @brief GamePlayer::~GamePlayer
 */

GamePlayer::~GamePlayer()
{

}





/**
 * @brief GamePlayerPrivate::loadQrcData
 */

void GamePlayer::setQrcDir()
{
	CosGame *game = cosGame();

	if (!game)
		return;

	QString playerCharacter = game->gameMatch()->playerCharacter();

	if (playerCharacter.isEmpty())
		return;

	QString dir = "qrc:/character/"+playerCharacter;

	setQrcDirName(dir);
}


/**
 * @brief GamePlayerPrivate::createFixtures
 */

void GamePlayer::createFixtures()
{
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
	box->setCategories(Box2DFixture::Category2);
	box->setCollidesWith(Box2DFixture::Category1|Box2DFixture::Category5);

	f.append(&f, box);

	setBoundBox(box);




	// Create body polygon

	Box2DPolygon *polygon = new Box2DPolygon(parentEntity());

	polygon->setSensor(true);
	polygon->setCategories(Box2DFixture::Category3);
	polygon->setCollidesWith(Box2DFixture::Category3|Box2DFixture::Category4|Box2DFixture::Category6);

	QVariantList l;
	l << QVariant(QPoint(0,0))
	  << QVariant(QPoint(10,0))
	  << QVariant(QPoint(10,10))
	  << QVariant(QPoint(0,10));

	polygon->setVertices(l);

	f.append(&f, polygon);

	setBodyPolygon(polygon);

}


/**
 * @brief GamePlayerPrivate::ladderClimbUp
 */

void GamePlayer::ladderClimbUp()
{
	if (!m_ladder || !parentEntity())
		return;

	QObject *spriteSequence = qvariant_cast<QObject *>(parentEntity()->property("spriteSequence"));

	m_isLadderDirectionUp = true;

	if (m_ladderMode == LadderUpAvailable) {
		int x = m_ladder->boundRect().x()+(m_ladder->boundRect().width()-parentEntity()->width())/2;
		parentEntity()->setX(x);
		setLadderMode(LadderClimb);
		if (spriteSequence)
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbup"));
	} else if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish) {
		qreal y = parentEntity()->y()-m_qrcData.value("climb", 1).toReal();
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

void GamePlayer::ladderClimbDown()
{
	if (!m_ladder || !parentEntity())
		return;

	QObject *spriteSequence = qvariant_cast<QObject *>(parentEntity()->property("spriteSequence"));

	m_isLadderDirectionUp = false;

	if (m_ladderMode == LadderDownAvailable) {
		int x = m_ladder->boundRect().x()+(m_ladder->boundRect().width()-parentEntity()->width())/2;
		parentEntity()->setX(x);
		setLadderMode(LadderClimb);
		if (spriteSequence)
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbdown"));
	}  else if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish) {
		qreal delta = m_qrcData.value("climb", 1).toReal()*1.2;
		int y = parentEntity()->y()+delta;
		int height = parentEntity()->height();

		if (m_ladderMode == LadderClimb) {
			if (y+height >= m_ladder->boundRect().bottom() && spriteSequence) {
				QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::QueuedConnection, Q_ARG(QString, "climbdownend"));
				setLadderMode(LadderClimbFinish);
				parentEntity()->setY(m_ladder->boundRect().bottom()-height);
			} else {
				parentEntity()->setY(y);
			}
		}
	}
}


/**
 * @brief GamePlayerPrivate::ladderClimbFinish
 */

void GamePlayer::ladderClimbFinish()
{
	if (!m_ladder || !parentEntity())
		return;

	if (m_isLadderDirectionUp)
		parentEntity()->setY(m_ladder->boundRect().top()-parentEntity()->height());

	setLadder(nullptr);
	setLadderMode(LadderUnavaliable);
}


/**
 * @brief GamePlayerPrivate::onBodyBeginContact
 * @param other
 */

void GamePlayer::onBodyBeginContact(Box2DFixture *other)
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

	if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish)
		return;


	GamePickable *pickable = qvariant_cast<GamePickable *>(object);

	if (pickable) {
		m_cosGame->addPickable(pickable);
	}
}


/**
 * @brief GamePlayerPrivate::onBodyEndContact
 * @param other
 */

void GamePlayer::onBodyEndContact(Box2DFixture *other)
{
	QVariant object = other->property("targetObject");
	//QVariantMap data = other->property("targetData").toMap();

	if (!object.isValid())
		return;

	GameLadder *ladder = qvariant_cast<GameLadder *>(object);

	if (ladder && m_ladderMode != LadderClimb) {
		if (ladder == m_ladder) {
			setLadder(nullptr);
			setLadderMode(LadderUnavaliable);
		}
	}

	GamePickable *pickable = qvariant_cast<GamePickable *>(object);

	if (pickable) {
		m_cosGame->removePickable(pickable);
	}
}

void GamePlayer::setLadderMode(GamePlayer::LadderMode ladderMode)
{
	if (m_ladderMode == ladderMode)
		return;

	m_ladderMode = ladderMode;
	emit ladderModeChanged(m_ladderMode);
}

void GamePlayer::setLadder(GameLadder *ladder)
{
	if (m_ladder == ladder)
		return;

	m_ladder = ladder;
	emit ladderChanged(m_ladder);
}


void GamePlayer::setEnemy(GameEnemy *enemy)
{
	if (m_enemy == enemy)
		return;

	if (m_enemy) {
		disconnect(m_enemy, &GameEnemy::die, this, &GamePlayer::onEnemyDied);
		m_enemy->setAimedByPlayer(false);
	}

	m_enemy = enemy;
	emit enemyChanged(m_enemy);

	if (m_enemy) {
		m_enemy->setAimedByPlayer(true);
		connect(m_enemy, &GameEnemy::die, this, &GamePlayer::onEnemyDied);
	}
}



void GamePlayer::setDefaultHp(int defaultHp)
{
	if (m_defaultHp == defaultHp)
		return;

	m_defaultHp = defaultHp;
	emit defaultHpChanged(m_defaultHp);
}


void GamePlayer::setShield(int shield)
{
	if (m_shield == shield)
		return;

	m_shield = shield;
	emit shieldChanged(m_shield);
}




/**
 * @brief GamePlayer::hurtByEnemy
 * @param enemy
 */

void GamePlayer::hurtByEnemy(GameEnemy *enemy, const bool &canProtect)
{
	emit underAttack();

	if (canProtect && m_shield > 0) {
		setShield(m_shield-1);
	} else {
		decreaseHp();

		if (hp() == 0)
			emit killedByEnemy(enemy);
		else
			emit hurt(enemy);
	}
}

/**
 * @brief GamePlayer::killByEnemy
 * @param enemy
 */

void GamePlayer::killByEnemy(GameEnemy *enemy)
{
	setHp(0);
	emit killedByEnemy(enemy);
}


/**
 * @brief GamePlayer::attackByGun
 */

void GamePlayer::attackByGun()
{
	if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish)
		return;

	if (!m_isAlive)
		return;

	emit attack();

	if (!m_enemy)
		return;

	m_cosGame->tryAttack(this, m_enemy);
}


/**
 * @brief GamePlayer::playSoundEffect
 * @param effect
 */

QString GamePlayer::playSoundEffect(const QString &effect)
{
	QString newSource = "";

	if (effect == "run") {
		if (m_soundEffectRunNum > 1) {
			newSource = "qrc:/sound/sfx/run2.ogg";
			m_soundEffectRunNum = 1;
		} else {
			newSource = "qrc:/sound/sfx/run1.ogg";
			m_soundEffectRunNum = 2;
		}
	} else if (effect == "walk") {
		if (m_soundEffectWalkNum > 1) {
			newSource = "qrc:/sound/sfx/step2.ogg";
			m_soundEffectWalkNum = 1;
		} else {
			newSource = "qrc:/sound/sfx/step1.ogg";
			m_soundEffectWalkNum = 2;
		}
	} else if (effect == "climb") {
		if (m_soundEffectClimbNum > 1) {
			newSource = "qrc:/sound/sfx/ladderup2.ogg";
			m_soundEffectClimbNum = 1;
		} else {
			newSource = "qrc:/sound/sfx/ladderup1.ogg";
			m_soundEffectClimbNum = 2;
		}
	} else if (effect == "ladder") {
		newSource = "qrc:/sound/sfx/ladder.ogg";
	} else if (effect == "pain") {
		if (m_soundEffectPainNum > 2) {
			newSource = "qrc:/sound/sfx/pain3.ogg";
			m_soundEffectClimbNum = 1;
		} else 	if (m_soundEffectPainNum == 2) {
			newSource = "qrc:/sound/sfx/pain2.ogg";
			m_soundEffectPainNum = 3;
		} else {
			newSource = "qrc:/sound/sfx/pain1.ogg";
			m_soundEffectPainNum = 2;
		}
	}

	return newSource;
}


/**
 * @brief GamePlayer::attackSuccesful
 * @param enemy
 */

void GamePlayer::attackSuccesful(GameEnemy *enemy)
{
	emit attackSucceed(enemy);
}


/**
 * @brief GamePlayer::attackFailed
 * @param enemy
 */

void GamePlayer::attackFailed(GameEnemy *enemy)
{
	hurtByEnemy(enemy);
}






/**
 * @brief GamePlayerPrivate::onCosGameChanged
 * @param game
 */

void GamePlayer::onCosGameChanged(CosGame *)
{
	if (!cosGame())
		return;

	setQrcDir();
	loadQrcData();

	QVariantMap m = m_cosGame->gameData().value("level", QVariantMap()).toMap();

	if (m.isEmpty())
		return;

	int level = m_cosGame->gameMatch()->level();

	m = m.value(QVariant(level).toString(), QVariantMap()).toMap();

	if (m.isEmpty())
		return;

	QVariantMap me = m.value("player").toMap();

	setRayCastElevation(me.value("rayCastElevation", m_rayCastElevation).toReal());
	setRayCastLength(me.value("rayCastLength", m_rayCastLength).toReal());

}




/**
 * @brief GamePlayer::onRayCastReported
 * @param items
 */

void GamePlayer::onRayCastReported(QMultiMap<qreal, QQuickItem *> items)
{
	GameEnemy *enemy = nullptr;

	foreach(QQuickItem *item, items) {
		GameEnemy *e = qvariant_cast<GameEnemy *>(item->property("entityPrivate"));

		if (e && e->isAlive()) {
			enemy = e;
			break;
		}
	}

	setEnemy(enemy);
}


/**
 * @brief GamePlayer::onEnemyDied
 * @param enemy
 */

void GamePlayer::onEnemyDied()
{
	m_enemy = nullptr;
	emit enemyChanged(m_enemy);
}



