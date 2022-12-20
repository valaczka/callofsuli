/*
 * ---- Call of Suli ----
 *
 * OLDGamePlayerprivate.cpp
 *
 * Created on: 2020. 10. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OLDGamePlayerPrivate
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
#include <QQmlListReference>
#include <QQmlProperty>


#include "entity.h"
#include "box2dbody.h"
#include "box2dfixture.h"
#include "cosclient.h"
#include "OLDGamePlayer.h"
#include "gameenemy.h"

OLDGamePlayer::OLDGamePlayer(QQuickItem *parent)
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
	, m_moveToPoint(0,0)
	, m_moveToItem(nullptr)
	, m_fire(nullptr)
	, m_fence(nullptr)
	, m_invisible(false)
	, m_teleport(nullptr)
{
	connect(this, &GameEntity::cosGameChanged, this, &OLDGamePlayer::onCosGameChanged);
	connect(this, &OLDGamePlayer::bodyBeginContact, this, &OLDGamePlayer::onBodyBeginContact);
	connect(this, &OLDGamePlayer::bodyEndContact, this, &OLDGamePlayer::onBodyEndContact);

	m_rayCastFlag = Box2DFixture::Category5;
}


/**
 * @brief OLDGamePlayer::~OLDGamePlayer
 */

OLDGamePlayer::~OLDGamePlayer()
{

}





/**
 * @brief OLDGamePlayerPrivate::loadQrcData
 */

void OLDGamePlayer::setQrcDir()
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
 * @brief OLDGamePlayerPrivate::createFixtures
 */

void OLDGamePlayer::createFixtures()
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
 * @brief OLDGamePlayerPrivate::ladderClimbUp
 */

void OLDGamePlayer::ladderClimbUp()
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
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::DirectConnection, Q_ARG(QString, "climbup"));
	} else if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish) {
		qreal y = parentEntity()->y()-m_qrcData.value("climb", 1).toReal();
		parentEntity()->setY(y);
		if (y < m_ladder->boundRect().top()-parentEntity()->height()) {
			ladderClimbFinish();
			if (spriteSequence)
				QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::DirectConnection, Q_ARG(QString, "idle"));
		} else if (y < m_ladder->boundRect().top() && spriteSequence && m_ladderMode == LadderClimb) {
			setLadderMode(LadderClimbFinish);
			if (spriteSequence)
				QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::DirectConnection, Q_ARG(QString, "climbupend"));
		}
	}
}


/**
 * @brief OLDGamePlayerPrivate::ladderClimbDown
 */

void OLDGamePlayer::ladderClimbDown()
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
			QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::DirectConnection, Q_ARG(QString, "climbdown"));
	}  else if (m_ladderMode == LadderClimb || m_ladderMode == LadderClimbFinish) {
		qreal delta = m_qrcData.value("climb", 1).toReal()*1.2;
		int y = parentEntity()->y()+delta;
		int height = parentEntity()->height();

		if (y+height >= m_ladder->boundRect().bottom()) {
			int newY = m_ladder->boundRect().bottom()-height;
			ladderClimbFinish();
			parentEntity()->setY(newY);
			if (spriteSequence)
				QMetaObject::invokeMethod(spriteSequence, "jumpTo", Qt::DirectConnection, Q_ARG(QString, "climbdownend"));
		} else {
			parentEntity()->setY(y);
		}
	}
}


/**
 * @brief OLDGamePlayerPrivate::ladderClimbFinish
 */

void OLDGamePlayer::ladderClimbFinish()
{
	if (!m_ladder || !parentEntity())
		return;

	if (m_isLadderDirectionUp)
		parentEntity()->setY(m_ladder->boundRect().top()-parentEntity()->height());

	setLadder(nullptr);
	setLadderMode(LadderUnavaliable);
}


/**
 * @brief OLDGamePlayerPrivate::onBodyBeginContact
 * @param other
 */

void OLDGamePlayer::onBodyBeginContact(Box2DFixture *other)
{
	QVariant object = other->property("targetObject");
	QVariantMap data = other->property("targetData").toMap();

	if (!object.isValid())
		return;

	if (data.value("fireDie", false).toBool()) {
		emit diedByBurn();
		return;
	} else if (data.value("fire", false).toBool()) {
		setFire(qvariant_cast<QQuickItem*>(object));
		return;
	} else if (data.value("fence", false).toBool()) {
		setFence(qvariant_cast<QQuickItem*>(object));
		return;
	} else if (data.value("teleport", false).toBool()) {
		setTeleport(qvariant_cast<QQuickItem*>(object));
		return;
	}

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
 * @brief OLDGamePlayerPrivate::onBodyEndContact
 * @param other
 */

void OLDGamePlayer::onBodyEndContact(Box2DFixture *other)
{
	QVariant object = other->property("targetObject");
	QVariantMap data = other->property("targetData").toMap();

	if (!object.isValid())
		return;

	if (data.value("fire", false).toBool()) {
		QQuickItem *i = qvariant_cast<QQuickItem*>(object);
		if (m_fire == i)
			setFire(nullptr);
		return;
	} else 	if (data.value("fence", false).toBool()) {
		QQuickItem *i = qvariant_cast<QQuickItem*>(object);
		if (m_fence == i)
			setFence(nullptr);
		return;
	} else 	if (data.value("teleport", false).toBool()) {
		QQuickItem *i = qvariant_cast<QQuickItem*>(object);
		if (m_teleport == i)
			setTeleport(nullptr);
		return;
	}

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

void OLDGamePlayer::setLadderMode(OLDGamePlayer::LadderMode ladderMode)
{
	if (m_ladderMode == ladderMode)
		return;

	m_ladderMode = ladderMode;
	emit ladderModeChanged(m_ladderMode);
}

void OLDGamePlayer::setLadder(GameLadder *ladder)
{
	if (m_ladder == ladder)
		return;

	m_ladder = ladder;
	emit ladderChanged(m_ladder);
}


void OLDGamePlayer::setEnemy(GameEnemy *enemy)
{
	if (m_enemy == enemy)
		return;

	if (m_enemy) {
		disconnect(m_enemy, &GameEnemy::die, this, &OLDGamePlayer::onEnemyDied);
		m_enemy->setAimedByPlayer(false);
	}

	m_enemy = enemy;
	emit enemyChanged(m_enemy);

	if (m_enemy) {
		m_enemy->setAimedByPlayer(true);
		connect(m_enemy, &GameEnemy::die, this, &OLDGamePlayer::onEnemyDied);
	}
}



void OLDGamePlayer::setDefaultHp(int defaultHp)
{
	if (m_defaultHp == defaultHp)
		return;

	m_defaultHp = defaultHp;
	emit defaultHpChanged(m_defaultHp);
}


void OLDGamePlayer::setShield(int shield)
{
	if (m_shield == shield)
		return;

	m_shield = shield;
	emit shieldChanged(m_shield);
}




/**
 * @brief OLDGamePlayer::hurtByEnemy
 * @param enemy
 */

void OLDGamePlayer::hurtByEnemy(GameEnemy *enemy, const bool &canProtect)
{
	emit underAttack();

	if (m_cosGame && m_cosGame->gameMatch() && m_cosGame->gameMatch()->invincible()) {
		return;
	}

	if (canProtect && m_shield > 0) {
		setShield(m_shield-1);
	} else {
		decreaseHp();

		if (m_cosGame && m_cosGame->gameMatch()) {
			m_cosGame->gameMatch()->setIsFlawless(false);
		}

		if (hp() == 0)
			emit killedByEnemy(enemy);
		else
			emit hurt(enemy);
	}
}

/**
 * @brief OLDGamePlayer::killByEnemy
 * @param enemy
 */

void OLDGamePlayer::killByEnemy(GameEnemy *enemy)
{
	if (m_cosGame && m_cosGame->gameMatch() && m_cosGame->gameMatch()->invincible()) {
		return;
	}

	setHp(0);

	if (m_cosGame && m_cosGame->gameMatch()) {
		m_cosGame->gameMatch()->setIsFlawless(false);
	}

	emit killedByEnemy(enemy);
}


/**
 * @brief OLDGamePlayer::attackByGun
 */

void OLDGamePlayer::attackByGun()
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
 * @brief OLDGamePlayer::operateFire
 */

void OLDGamePlayer::operate(QQuickItem *item)
{
	if (!item || !parentEntity())
		return;

	if (!m_isAlive)
		return;

	if (m_cosGame && !m_cosGame->running())
		return;

	setMoveToPoint(QPointF(0,0));
	setMoveToItem(nullptr);

	QPointF left = item->property("operatingPointLeft").toPointF() + item->position();
	QPointF right = item->property("operatingPointRight").toPointF() + item->position();

	qreal myLeft = parentEntity()->x()+m_boundBox->x();
	qreal myRight = parentEntity()->x()+m_boundBox->x()+m_boundBox->width();

	if (myRight < left.x()) {
		setMoveToItem(item);
		setMoveToPoint(left);
		autoMove();
	} else if (myLeft > right.x()) {
		setMoveToItem(item);
		setMoveToPoint(right);
		autoMove();
	} else {
		operateReal(item);
	}

}


/**
 * @brief OLDGamePlayer::autoMoveUpdate
 * @param item
 */

void OLDGamePlayer::autoMove()
{
	if (m_moveToPoint == QPointF(0,0))
		return;

	if (!parentEntity())
		return;

	qreal realX = parentEntity()->x()+m_boundBox->x();

	if (m_moveToPoint.x() < realX) {
		qreal distance = realX - m_moveToPoint.x();
		if (distance <= 5) {
			operateReal(m_moveToItem);
		} else
			emit autoMoveWalkRequest(true);
	} else {
		qreal distance = m_moveToPoint.x() - (realX+m_boundBox->width());
		if (distance <= 5) {
			operateReal(m_moveToItem);
		} else
			emit autoMoveWalkRequest(false);
	}

}


/**
 * @brief OLDGamePlayer::teleportToNext
 */

void OLDGamePlayer::teleportToNext()
{
	if (!m_teleport)
		return;

	if (!parentEntity())
		return;

	QQuickItem *sceneItem = m_cosGame->gameScene();

	qDebug() << "SCENE" << sceneItem;

	if (!sceneItem)
		return;

	GameScene *scene = qvariant_cast<GameScene *>(sceneItem->property("scenePrivate"));

	qDebug() << "SCENE*" << scene;

	if (!scene)
		return;

	if (scene->teleports().isEmpty())
		return;

	int idx = scene->teleports().indexOf(m_teleport);
	idx++;

	if (idx >= scene->teleports().size())
		idx = 0;

	const QQuickItem *next = scene->teleports().at(idx);

	qDebug() << "GOTO" << next << next->x() << next->y();

	parentEntity()->setX(next->x());
	parentEntity()->setY(next->y());

}


/**
 * @brief OLDGamePlayer::playSoundEffect
 * @param effect
 */

QString OLDGamePlayer::playSoundEffect(const QString &effect)
{
	QString newSource = "";

	if (effect == "run") {
		if (m_soundEffectRunNum > 1) {
			newSource = "qrc:/sound/sfx/run2.mp3";
			m_soundEffectRunNum = 1;
		} else {
			newSource = "qrc:/sound/sfx/run1.mp3";
			m_soundEffectRunNum = 2;
		}
	} else if (effect == "walk") {
		if (m_soundEffectWalkNum > 1) {
			newSource = "qrc:/sound/sfx/step2.mp3";
			m_soundEffectWalkNum = 1;
		} else {
			newSource = "qrc:/sound/sfx/step1.mp3";
			m_soundEffectWalkNum = 2;
		}
	} else if (effect == "climb") {
		if (m_soundEffectClimbNum > 1) {
			newSource = "qrc:/sound/sfx/ladderup2.mp3";
			m_soundEffectClimbNum = 1;
		} else {
			newSource = "qrc:/sound/sfx/ladderup1.mp3";
			m_soundEffectClimbNum = 2;
		}
	} else if (effect == "ladder") {
		newSource = "qrc:/sound/sfx/ladder.mp3";
	} else if (effect == "pain") {
		if (m_soundEffectPainNum > 2) {
			newSource = "qrc:/sound/sfx/pain3.mp3";
			m_soundEffectClimbNum = 1;
		} else 	if (m_soundEffectPainNum == 2) {
			newSource = "qrc:/sound/sfx/pain2.mp3";
			m_soundEffectPainNum = 3;
		} else {
			newSource = "qrc:/sound/sfx/pain1.mp3";
			m_soundEffectPainNum = 2;
		}
	}

	return newSource;
}




/**
 * @brief OLDGamePlayer::attackFailed
 * @param enemy
 */

void OLDGamePlayer::attackFailed(GameEnemy *enemy)
{
	hurtByEnemy(enemy);
}






/**
 * @brief OLDGamePlayerPrivate::onCosGameChanged
 * @param game
 */

void OLDGamePlayer::onCosGameChanged(CosGame *)
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
 * @brief OLDGamePlayer::onRayCastReported
 * @param items
 */

void OLDGamePlayer::rayCastItemsReported(const QMultiMap<qreal, QQuickItem *> &items)
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
 * @brief OLDGamePlayer::onEnemyDied
 * @param enemy
 */

void OLDGamePlayer::onEnemyDied()
{
	m_enemy = nullptr;
	emit enemyChanged(m_enemy);
}


/**
 * @brief OLDGamePlayer::operateReal
 * @param item
 */

void OLDGamePlayer::operateReal(QQuickItem *item)
{
	if (!item)
		return;

	if (!m_cosGame)
		return;

	GameMatch *match = m_cosGame->gameMatch();

	if (!match)
		return;

	setMoveToItem(nullptr);
	setMoveToPoint(QPointF(0,0));

	if (item == m_fire) {
		if (match->water() < 1)
			return;
		setFire(nullptr);
		match->setWater(qMax(match->water()-1, 0));
	} else if (item == m_fence) {
		if (match->pliers() < 1)
			return;
		setFence(nullptr);
	} else {
		return;
	}

	emit operateRequest(item);
	QMetaObject::invokeMethod(item, "operate", Qt::QueuedConnection);

	return;
}




QPointF OLDGamePlayer::moveToPoint() const
{
	return m_moveToPoint;
}

void OLDGamePlayer::setMoveToPoint(QPointF newMoveToPoint)
{
	if (m_moveToPoint == newMoveToPoint)
		return;

	m_moveToPoint = newMoveToPoint;
	emit moveToPointChanged();

	if (newMoveToPoint == QPointF(0,0)) {
		setMoveToItem(nullptr);
	}
}

QQuickItem *OLDGamePlayer::moveToItem() const
{
	return m_moveToItem;
}

void OLDGamePlayer::setMoveToItem(QQuickItem *newMoveToItem)
{
	if (m_moveToItem == newMoveToItem)
		return;
	m_moveToItem = newMoveToItem;
	emit moveToItemChanged();
}

QQuickItem *OLDGamePlayer::fire() const
{
	return m_fire;
}

void OLDGamePlayer::setFire(QQuickItem *newFire)
{
	if (m_fire == newFire)
		return;
	m_fire = newFire;
	emit fireChanged();
}

QQuickItem *OLDGamePlayer::fence() const
{
	return m_fence;
}

void OLDGamePlayer::setFence(QQuickItem *newFence)
{
	if (m_fence == newFence)
		return;
	m_fence = newFence;
	emit fenceChanged();
}

bool OLDGamePlayer::invisible() const
{
	return m_invisible;
}

void OLDGamePlayer::setInvisible(bool newInvisible)
{
	if (m_invisible == newInvisible)
		return;
	m_invisible = newInvisible;
	emit invisibleChanged();
}

QQuickItem *OLDGamePlayer::teleport() const
{
	return m_teleport;
}

void OLDGamePlayer::setTeleport(QQuickItem *newTeleport)
{
	if (m_teleport == newTeleport)
		return;
	m_teleport = newTeleport;
	emit teleportChanged(m_teleport);
}
