/*
 * ---- Call of Suli ----
 *
 * isometricplayer.cpp
 *
 * Created on: 2024. 03. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricPlayer
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "isometricplayer.h"
#include "isometricenemy.h"
#include "tiledpickableiface.h"
#include "tiledscene.h"
#include "tiledgame.h"
#include "application.h"
#include "tiledspritehandler.h"


class IsometricPlayerPrivate {
private:
	/**
	 * @brief isEnemyContanctedAndReached
	 * @return
	 */

	bool isEnemyContanctedAndReached() const {
		return m_enemy && m_contactedEnemies.contains(m_enemy) && m_reachedEnemies.contains(m_enemy);
	}

	/**
	 * @brief contactedAndReachedEnemies
	 * @return
	 */

	QList<IsometricEnemy*> contactedAndReachedEnemies() const {
		QList<IsometricEnemy*> list;

		for (const auto &ptr : std::as_const(m_reachedEnemies)) {
			if (!ptr)
				continue;

			if (m_contactedEnemies.contains(ptr.data()))
				list.append(ptr.data());
		}

		return list;
	}


	/**
	 * @brief removeEnemy
	 * @param enemy
	 */

	void removeEnemy(IsometricEnemy *enemy) {
		m_contactedEnemies.removeAll(enemy);
		m_reachedEnemies.removeAll(enemy);
	}


	void clear() {
		///m_enemy = nullptr;
		m_contactedEnemies.clear();
		m_reachedEnemies.clear();
		m_destinationPoint.reset();
	}

	QPointer<IsometricEnemy> m_enemy;
	QList<QPointer<IsometricEnemy>> m_contactedEnemies;
	QList<QPointer<IsometricEnemy>> m_reachedEnemies;

	std::optional<QPointF> m_destinationPoint;

	friend class IsometricPlayer;
};



/**
 * @brief IsometricPlayer::IsometricPlayer
 * @param parent
 */

IsometricPlayer::IsometricPlayer(QQuickItem *parent)
	: IsometricCircleEntity(parent)
	, d(new IsometricPlayerPrivate)
{
	m_inabilityTimer.setRemainingTime(-1);
}


/**
 * @brief IsometricPlayer::~IsometricPlayer
 */

IsometricPlayer::~IsometricPlayer()
{
	delete d;
}



/**
 * @brief IsometricPlayer::entityWorldStep
 */

void IsometricPlayer::entityWorldStep(const qreal &factor)
{
	IsometricEnemy *e = nullptr;

	if (d->isEnemyContanctedAndReached() && checkEntityVisibility(m_body.get(), d->m_enemy, TiledObjectBody::FixtureEnemyBody, nullptr).has_value()) {
		e = d->m_enemy;
	} else {
		e = getVisibleEntity<IsometricEnemy*>(m_body.get(), d->contactedAndReachedEnemies(), TiledObjectBody::FixtureEnemyBody, nullptr);
	}

	if (e != d->m_enemy) {
		if (d->m_enemy)
			d->m_enemy->setGlowEnabled(false);
		setEnemy(e);
		if (d->m_enemy) {
			d->m_enemy->setGlowColor(Qt::red);
			d->m_enemy->setGlowEnabled(true);
		}
	}

	rotateBody(m_currentAngle);


	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		m_body->stop();
	} else {
		if (d->m_destinationPoint) {
			QLineF line(QPointF{0,0}, d->m_destinationPoint.value() - m_body->bodyPosition());
			if (line.length() >= m_speedRunLength*factor) {
				line.setLength(m_speedRunLength*factor);
				m_body->setLinearVelocity(line.p2());
			} else if (line.length() >= m_speedLength*factor) {
				line.setLength(m_speedLength*factor);
				m_body->setLinearVelocity(line.p2());
			} else {
				m_body->stop();
				atDestinationPointEvent();
				clearDestinationPoint();
			}

		} else {
			QLineF line(0,0, m_currentVelocity.x(), m_currentVelocity.y());
			line.setLength(line.length()*factor);
			m_body->setLinearVelocity(line.p2());
		}
	}

	updateSprite();
}








/**
 * @brief IsometricPlayer::initialize
 */

void IsometricPlayer::initialize()
{
	m_body->setBodyType(Box2DBody::Dynamic);

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);

	createVisual();
	createMarkerItem();

	TiledObjectSensorPolygon *p = addSensorPolygon(m_sensorLength, m_sensorRange);

	addTargetCircle(m_targetCircleRadius);

	p->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
					   TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable)
					   );

	p->virtualCircle()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle));
	p->virtualCircle()->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTrigger) |
										TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable) |
										TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer)
										);

	connect(p, &TiledObjectSensorPolygon::beginContact, this, &IsometricPlayer::sensorBeginContact);
	connect(p, &TiledObjectSensorPolygon::endContact, this, &IsometricPlayer::sensorEndContact);

	connect(m_fixture.get(), &TiledObjectSensorPolygon::beginContact, this, &IsometricPlayer::fixtureBeginContact);
	connect(m_fixture.get(), &TiledObjectSensorPolygon::endContact, this, &IsometricPlayer::fixtureEndContact);

	load();
	onAlive();
}



/**
 * @brief IsometricPlayer::hasAbility
 * @return
 */

bool IsometricPlayer::hasAbility() const
{
	return m_inabilityTimer.isForever() || m_inabilityTimer.hasExpired();
}




/**
 * @brief IsometricPlayer::currentAngle
 * @return
 */

qreal IsometricPlayer::currentAngle() const
{
	return m_currentAngle;
}

void IsometricPlayer::setCurrentAngle(qreal newCurrentAngle)
{
	if (qFuzzyCompare(m_currentAngle, newCurrentAngle))
		return;
	m_currentAngle = newCurrentAngle;
	emit currentAngleChanged();

	setCurrentDirection(nearestDirectionFromRadian(m_currentAngle));
}


/**
 * @brief IsometricPlayer::removeEnemy
 * @param enemy
 */

void IsometricPlayer::removeEnemy(IsometricEnemy *enemy)
{
	d->removeEnemy(enemy);
}



/**
 * @brief IsometricPlayer::onAlive
 */

void IsometricPlayer::onAlive()
{
	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setActive(true);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer)
							   );

	m_sensorPolygon->setLength(m_sensorLength);
	m_sensorPolygon->setRange(m_sensorRange);
	setSubZ(0.5);
	emit becameAlive();
}




/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	m_sensorPolygon->setLength(10.);
	setSubZ(0.0);
	setCurrentVelocity(QPointF{});

	m_game->onPlayerDead(this);

	emit becameDead();
}



/**
 * @brief IsometricPlayer::startInability
 */

void IsometricPlayer::startInability()
{
	if (m_inabilityTime > 0)
		m_inabilityTimer.setRemainingTime(m_inabilityTime);
}


/**
 * @brief IsometricPlayer::startInability
 * @param msec
 */

void IsometricPlayer::startInability(const int &msec)
{
	if (m_inabilityTime > 0)
		m_inabilityTimer.setRemainingTime(msec);
}






/**
 * @brief IsometricPlayer::protectWeapon
 * @param list
 * @param weaponType
 * @return
 */

bool IsometricPlayer::protectWeapon(TiledWeaponList *weaponList, const TiledWeapon::WeaponType &weaponType)
{
	Q_ASSERT(weaponList);

	if (weaponList->empty())
		return false;

	std::vector<TiledWeapon*> list;

	// Ordering weapons by enum

	for (TiledWeapon *weapon : std::as_const(*weaponList)) {
		if (weapon->canProtect(weaponType))
			list.push_back(weapon);
	}

	if (list.empty())
		return false;

	std::sort(list.begin(), list.end(), [](TiledWeapon *w1, TiledWeapon *w2) {
		return w1->weaponType() < w2->weaponType();
	});

	for (TiledWeapon *w : std::as_const(list))
		if (w->protect(weaponType))
			return true;

	return false;
}


/**
 * @brief IsometricPlayer::reachedEnemies
 * @return
 */

QList<IsometricEnemy *> IsometricPlayer::reachedEnemies() const
{
	if (d->m_reachedEnemies.isEmpty())
		return {};

	QList<IsometricEnemy *> list;
	list.reserve(d->m_reachedEnemies.size());

	for (const auto &ptr : d->m_reachedEnemies)
		list.append(ptr.data());

	return list;
}



/**
 * @brief IsometricPlayer::contactedAndReachedEnemies
 * @return
 */

QList<IsometricEnemy *> IsometricPlayer::contactedAndReachedEnemies() const
{
	return d->contactedAndReachedEnemies();
}


/**
 * @brief IsometricPlayer::clearData
 */

void IsometricPlayer::clearData()
{
	d->clear();
}


/**
 * @brief IsometricPlayer::isLocked
 * @return
 */

bool IsometricPlayer::isLocked() const
{
	return m_isLocked;
}

void IsometricPlayer::setIsLocked(bool newIsLocked)
{
	if (m_isLocked == newIsLocked)
		return;
	m_isLocked = newIsLocked;
	emit isLockedChanged();
	if (m_isLocked)
		m_body->stop();
}




/**
 * @brief IsometricPlayer::sensorBeginContact
 * @param other
 */

void IsometricPlayer::sensorBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);

	if (!base)
		return;

	if (IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base)) {
		if (!d->m_contactedEnemies.contains(enemy))
			d->m_contactedEnemies.append(QPointer(enemy));
	}
}



/**
 * @brief IsometricPlayer::sensorEndContact
 * @param other
 */

void IsometricPlayer::sensorEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

	if (!base)
		return;

	if (enemy)
		d->m_contactedEnemies.removeAll(QPointer(enemy));
}



/**
 * @brief IsometricPlayer::fixtureBeginContact
 * @param other
 */

void IsometricPlayer::fixtureBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);

	if (!base)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy && !d->m_reachedEnemies.contains(enemy)) {
			d->m_reachedEnemies.append(QPointer(enemy));
			onEnemyReached(enemy);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (!m_currentTransport && transport) {
			setCurrentTransport(transport);
			m_currentTransportBase = base;
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable))) {
		TiledObject *object = qobject_cast<TiledObject*>(base);
		TiledPickableIface *iface = dynamic_cast<TiledPickableIface*>(base);
		if (object && iface) {
			onPickableReached(object);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer))) {
		TiledContainer *container = base->property("tiledContainer").value<TiledContainer*>();

		if (!m_currentContainer && container)
			setCurrentContainer(container);
	}
}





/**
 * @brief IsometricPlayer::fixtureEndContact
 * @param other
 */

void IsometricPlayer::fixtureEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);

	if (!base)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy) {
			d->m_reachedEnemies.removeAll(QPointer(enemy));
			onEnemyLeft(enemy);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (m_currentTransport == transport && transport) {
			setCurrentTransport(nullptr);
			m_currentTransportBase = nullptr;
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable))) {
		TiledObject *object = qobject_cast<TiledObject*>(base);
		TiledPickableIface *iface = dynamic_cast<TiledPickableIface*>(base);
		if (object && iface) {
			//removePickable(object);
			onPickableLeft(object);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer))) {
		TiledContainer *container = base->property("tiledContainer").value<TiledContainer*>();

		if (m_currentContainer == container && container)
			setCurrentContainer(nullptr);
	}
}




/**
 * @brief IsometricPlayer::currentContainer
 * @return
 */

TiledContainer *IsometricPlayer::currentContainer() const
{
	return m_currentContainer;
}

void IsometricPlayer::setCurrentContainer(TiledContainer *newCurrentContainer)
{
	if (m_currentContainer == newCurrentContainer)
		return;
	m_currentContainer = newCurrentContainer;
	emit currentContainerChanged();
}





/**
 * @brief IsometricPlayer::currentVelocity
 * @return
 */

QPointF IsometricPlayer::currentVelocity() const
{
	return m_currentVelocity;
}

void IsometricPlayer::setCurrentVelocity(QPointF newCurrentVelocity)
{
	if (m_currentVelocity == newCurrentVelocity)
		return;
	m_currentVelocity = newCurrentVelocity;
	emit currentVelocityChanged();
}



/**
 * @brief IsometricPlayer::onJoystickStateChanged
 */

void IsometricPlayer::onJoystickStateChanged(const TiledGame::JoystickState &state)
{
	if (!isAlive())
		return;

	clearDestinationPoint();

	if (m_isLocked) {
		m_body->stop();
		return;
	}

	if (state.hasKeyboard || state.hasTouch) {
		setCurrentAngle(state.angle);
	}

	if (state.distance > 0.95) {
		setCurrentVelocity(TiledObjectBase::toPoint(state.angle, m_speedRunLength));
		m_body->setIsRunning(true);
	} else if (state.distance > 0.45) {
		setCurrentVelocity(TiledObjectBase::toPoint(state.angle, m_speedLength));
		m_body->setIsRunning(false);
	} else {
		setCurrentVelocity({0.,0.});
		m_body->setIsRunning(false);
	}
}



/**
 * @brief IsometricPlayer::setDestinationPoint
 * @param x
 */

void IsometricPlayer::setDestinationPoint(const qreal &x, const qreal &y)
{
	if (!isAlive())
		return;

	if (m_isLocked) {
		clearDestinationPoint();
		m_body->stop();
		return;
	}

	QLineF l(m_body->bodyPosition(), QPointF{x,y});
	setCurrentAngle(toRadian(l.angle()));

	d->m_destinationPoint.emplace(x, y);
}


/**
 * @brief IsometricPlayer::clearDestinationPoint
 */

void IsometricPlayer::clearDestinationPoint()
{
	d->m_destinationPoint.reset();
}


/**
 * @brief IsometricPlayer::currentTransport
 * @return
 */

TiledTransport *IsometricPlayer::currentTransport() const
{
	return m_currentTransport;
}

void IsometricPlayer::setCurrentTransport(TiledTransport *newCurrentTransport)
{
	if (m_currentTransport == newCurrentTransport)
		return;
	m_currentTransport = newCurrentTransport;
	emit currentTransportChanged();
}



/**
 * @brief IsometricPlayer::enemy
 * @return
 */

IsometricEnemy *IsometricPlayer::enemy() const
{
	return d->m_enemy.data();
}

void IsometricPlayer::setEnemy(IsometricEnemy *newEnemy)
{
	if (d->m_enemy == newEnemy)
		return;
	d->m_enemy = newEnemy;
	emit enemyChanged();
}
