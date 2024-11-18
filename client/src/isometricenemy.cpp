/*
 * ---- Call of Suli ----
 *
 * isometricenemy.cpp
 *
 * Created on: 2024. 03. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEnemy
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

#include "isometricenemy.h"
#include "box2dworld.h"
#include "isometricplayer.h"
#include "tiledfixpositionmotor.h"
#include "tiledspritehandler.h"






/**
 * @brief IsometricEnemy::IsometricEnemy
 * @param parent
 */

IsometricEnemy::IsometricEnemy(QQuickItem *parent)
	: IsometricCircleEntity(parent)
	, IsometricEnemyIface()
{

}





/**
 * @brief IsometricEnemy::initialize
 */

void IsometricEnemy::initialize()
{
	m_body->setBodyType(Box2DBody::Kinematic);

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);

	createVisual();

	TiledObjectSensorPolygon *p = addSensorPolygon(m_metric.sensorLength, m_metric.sensorRange);

	addTargetCircle(m_metric.targetCircleRadius > 0. ? m_metric.targetCircleRadius : 50.);

	p->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody));

	connect(p, &TiledObjectSensorPolygon::beginContact, this, &IsometricEnemy::sensorBeginContact);
	connect(p, &TiledObjectSensorPolygon::endContact, this, &IsometricEnemy::sensorEndContact);

	connect(m_fixture.get(), &TiledObjectSensorPolygon::beginContact, this, &IsometricEnemy::fixtureBeginContact);
	connect(m_fixture.get(), &TiledObjectSensorPolygon::endContact, this, &IsometricEnemy::fixtureEndContact);

	load();
	onAlive();
}



/**
 * @brief IsometricEnemy::hasAbility
 * @return
 */

bool IsometricEnemy::hasAbility()
{
	if (!m_game->tickTimer() || m_inabilityTimer < 0)
		return true;

	if (m_inabilityTimer < m_game->tickTimer()->currentTick()) {
		m_inabilityTimer = -1;
		return true;
	}

	return false;
}


/**
 * @brief IsometricEnemy::isSleeping
 * @return
 */

bool IsometricEnemy::isSleeping()
{
	if (!m_game->tickTimer() || m_sleepingTimer < 0)
		return false;

	if (m_sleepingTimer < m_game->tickTimer()->currentTick()) {
		m_sleepingTimer = -1;
		return false;
	}

	return true;
}


/**
 * @brief IsometricEnemy::attackedByPlayer
 * @param player
 */

void IsometricEnemy::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	if (!isAlive() || isSleeping())
		return;

	if (!m_contactedPlayers.contains(player))
		m_contactedPlayers.append(QPointer(player));

	// Skip rotate on dagger

	if (weaponType == TiledWeapon::WeaponDagger) {
		return;
	}

	setPlayer(player);
	rotateToPlayer(player);
}


/**
 * @brief IsometricEnemy::startInability
 */

void IsometricEnemy::startInability()
{
	if (m_metric.inabilityTime > 0 && m_game->tickTimer())
		m_inabilityTimer = m_game->tickTimer()->currentTick() + m_metric.inabilityTime;
}


/**
 * @brief IsometricEnemy::startSleeping
 */

bool IsometricEnemy::startSleeping()
{
	if (m_metric.sleepingTime > 0 && m_game->tickTimer()) {
		m_sleepingTimer = m_game->tickTimer()->currentTick() + m_metric.sleepingTime;
		if (!m_isSleeping)
			onSleepingBegin();
		return true;
	}

	return false;
}



/**
 * @brief IsometricEnemy::eventKilledByPlayer
 * @param player
 */

void IsometricEnemy::eventKilledByPlayer(IsometricPlayer *player)
{
	if (player)
		player->removeEnemy(this);

	setPlayer(nullptr);
}



/**
 * @brief IsometricEnemy::attackPlayer
 * @param player
 */

void IsometricEnemy::attackPlayer(IsometricPlayer *player, TiledWeapon *weapon)
{
	if (!weapon || !player || player->isLocked())
		return;

	if (weapon->canHit()) {
		LOG_CTRACE("game") << "Enemy hit player:" << this << player << weapon;
		if (weapon->hit(player))
			playAttackEffect(weapon);
	} else if (weapon->canShot()) {
		LOG_CTRACE("game") << "Enemy shot player:" << this << player << weapon;
		if (weapon->shot(IsometricBullet::TargetPlayer, m_body->bodyPosition(), angleToPoint(player->body()->bodyPosition())))
			playAttackEffect(weapon);
	}
}





/**
 * @brief IsometricEnemy::entityWorldStep
 */

void IsometricEnemy::entityWorldStep(const qreal &factor)
{
	if (!isAlive() || isSleeping()) {
		m_body->stop();
		m_body->setIsRunning(false);
		jumpToSprite("death", m_currentDirection);
		return;
	}

	if (m_isSleeping)
		onSleepingEnd();

	if (m_movingDirection != Invalid)
		setCurrentDirection(m_movingDirection);


	//QPointF visiblePoint;
	float32 transparentGnd = -1.0;

	IsometricPlayer *myPlayer = getVisibleEntity<QPointer<IsometricPlayer>>(m_body.get(), m_contactedPlayers,
																			TiledObjectBody::FixturePlayerBody,
																			&transparentGnd
																			/*, &visiblePoint*/);


	float32 angle = 0.;
	bool isPursuit = false;
	bool attackWithoutPursuit = false;

	if (myPlayer) {
		if (!m_player)
			setPlayer(myPlayer);

		if (m_returnPathMotor && !myPlayer->isLocked())
			m_returnPathMotor->clearLastSeenPoint();

		qreal dist;

		rotateToPlayer(m_player, &angle, &dist);
		setPlayerDistance(dist);

		if (!m_reachedPlayers.contains(m_player)) {
			if (m_metric.pursuitSpeed > 0) {		// Pursuit
				isPursuit = true;
			} else {								// No pursuit
				attackWithoutPursuit = true;
				m_body->stop();
				m_body->setIsRunning(false);
			}
		} else {
			m_body->stop();
			m_body->setIsRunning(false);
		}
	} else {
		if (m_player) {
			if (m_returnPathMotor) {
				m_returnPathMotor->setLastSeenPoint(m_player->body()->bodyPosition());
				isPursuit = true;
			}

			setPlayer(nullptr);
			//setPlayerDistance(-1.);
		}

		if (m_returnPathMotor && !m_returnPathMotor->isReturning()) {
			const auto &ptr = m_returnPathMotor->lastSeenPoint();

			if (ptr) {
				const qreal &dist = distanceToPoint(ptr.value());

				if (dist >= std::max(std::max(m_metric.pursuitSpeed, m_metric.speed)*factor, 10.)){
					if (const float32 &d = checkGroundDistance(m_body.get(), ptr.value()); d != -1. && d < 0.05) {
						isPursuit = false;
					} else {
						rotateToPoint(ptr.value(), &angle);
						isPursuit = true;
					}
				} else {
					isPursuit = false;
				}
			}
		}
	}


	TiledObjectBody::setFixtureCollidesWithFlag(m_fixture.get(), TiledObjectBody::FixtureGround, !m_returnPathMotor);


	if (isPursuit) {
		if (m_player && m_player->isLocked()) {
			m_body->stop();
			m_body->setIsRunning(false);
			updateSprite();
			return;
		}

		if (!enemyWorldStepOnVisiblePlayer(angle, factor)) {
			updateSprite();
			return;
		}

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
			m_body->stop();
			m_body->setIsRunning(false);
			updateSprite();
			return;
		}

		if (transparentGnd >= 0. && transparentGnd < m_fixture->radius()*2.3) {
			m_body->stop();
			m_body->setIsRunning(false);
		} else if (m_metric.returnSpeed != 0) {
			if (!m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(m_body->bodyPosition()));

			if (m_metric.pursuitSpeed > 0 && m_playerDistance > m_metric.pursuitSpeed*factor) {
				m_body->setIsRunning(true);
				m_returnPathMotor->moveBody(m_body.get(), angle, m_metric.pursuitSpeed*factor);
			} else {
				m_body->setIsRunning(false);
				m_returnPathMotor->moveBody(m_body.get(), angle, m_metric.speed*factor);
			}
		} else if (m_metric.speed > 0) {
			if (m_metric.pursuitSpeed > 0 && m_playerDistance > m_metric.pursuitSpeed*factor) {
				m_body->setIsRunning(true);
				m_body->setLinearVelocity(
							TiledObjectBase::toPoint(angle, m_metric.pursuitSpeed*factor));

			} else {
				m_body->setIsRunning(false);
				m_body->setLinearVelocity(
							TiledObjectBase::toPoint(angle, m_metric.speed*factor));
			}
		} else {
			m_body->stop();
			m_body->setIsRunning(false);
		}

		// --- show path ---
		/*if (m_returnPathMotor) {
			QVariantList list;
			for (const auto &point : m_returnPathMotor->path())
				list.append(point);
			m_scene->setTestPoints(list);
		}*/
		// -----------


		enemyWorldStep();
		updateSprite();
		return;
	} else if (attackWithoutPursuit) {
		if (!enemyWorldStepOnVisiblePlayer(angle, factor)) {
			updateSprite();
			return;
		}

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
			m_body->stop();
			m_body->setIsRunning(false);
			updateSprite();
			return;
		}
	}

	if (m_returnPathMotor && !m_returnPathMotor->isReturning() && !m_returnPathMotor->hasReturned()) {
		m_returnPathMotor->finish(m_body.get(), m_game->tickTimer());
	}


	if (enemyWorldStep()) {
		//rotateBody(directionToRadian(m_currentDirection));
		stepMotor(factor);
	}

	updateSprite();
}



/**
 * @brief IsometricEnemy::enemyWorldStep
 */

bool IsometricEnemy::enemyWorldStep()
{
	if (!isAlive() || isSleeping())
		return true;

	if (m_metric.autoAttackTime <= 0 || m_metric.firstAttackTime <= 0)
		return true;

	if (m_player && m_reachedPlayers.contains(m_player) && m_player->isAlive()) {
		m_body->stop();
		m_body->setIsRunning(false);

		if (!hasAbility())
			return false;

		if (m_player->isLocked())
			return false;

		if (m_game->tickTimer()) {
			const auto tick = m_game->tickTimer()->currentTick();
			if (m_autoHitTimer >= 0 && m_autoHitTimer <= tick) {
				attackPlayer(m_player, defaultWeapon());
				m_autoHitTimer = tick + m_metric.autoAttackTime;
			} else if (m_autoHitTimer < 0) {
				m_autoHitTimer = tick + m_metric.firstAttackTime;
			}
		}

		return false;
	} else {
		m_autoHitTimer = -1;
	}

	return true;
}



/**
 * @brief IsometricEnemy::enemyWorldStepOnVisiblePlayer
 * @return
 */

bool IsometricEnemy::enemyWorldStepOnVisiblePlayer(const float32 &angle, const qreal &factor)
{
	Q_UNUSED(angle);
	Q_UNUSED(factor);

	if (!isAlive() || isSleeping())
		return false;

	if (m_metric.autoAttackTime <= 0 || m_metric.firstAttackTime <= 0)
		return true;

	if (defaultWeapon() && defaultWeapon()->canShot() &&
			m_player && m_contactedPlayers.contains(m_player) && m_player->isAlive()) {
		m_body->stop();
		m_body->setIsRunning(false);

		if (m_metric.returnSpeed != 0) {
			if (!m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(m_body->bodyPosition()));
		}

		if (!hasAbility())
			return false;

		if (m_player->isLocked())
			return false;

		if (m_game->tickTimer()) {
			const auto tick = m_game->tickTimer()->currentTick();

			if (m_playerDistance < m_metric.sensorLength*0.2 &&
					(m_autoHitTimer == -1 || tick-m_autoHitTimer > m_metric.autoAttackTime))
				m_autoHitTimer = tick;

			if (m_autoHitTimer >= 0 && m_autoHitTimer <= tick) {
				attackPlayer(m_player, defaultWeapon());
				m_autoHitTimer = tick + m_metric.autoAttackTime;
			} else if (m_autoHitTimer < 0) {
				m_autoHitTimer = tick + m_metric.firstAttackTime;
			}
		}

		return false;
	} else {
		//m_autoHitTimer.setRemainingTime(-1);
	}

	return true;
}




/**
 * @brief IsometricEnemy::onPathMotorLoaded
 */

void IsometricEnemy::onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/)
{
	m_body->emplace(m_motor->basePoint());
}





/**
 * @brief IsometricEnemy::onAlive
 */

void IsometricEnemy::onAlive()
{
	m_body->setBodyType(Box2DBody::Kinematic);
	m_body->setActive(true);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor));

	m_sensorPolygon->setLength(m_metric.sensorLength);
	m_sensorPolygon->setRange(m_metric.sensorRange);
	setSubZ(0.5);

	emit becameAlive();
}





/**
 * @brief IsometricEnemy::onDead
 */

void IsometricEnemy::onDead()
{
	m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	setSubZ(0.0);
	m_sensorPolygon->setLength(10.);

	m_game->onEnemyDead(this);

	emit becameDead();
}



/**
 * @brief IsometricEnemy::onSleepingBegin
 */

void IsometricEnemy::onSleepingBegin()
{
	m_isSleeping = true;

	m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	setSubZ(0.0);

	m_game->onEnemySleepingStart(this);

	emit becameAsleep();
}




/**
 * @brief IsometricEnemy::onSleepingEnd
 */

void IsometricEnemy::onSleepingEnd()
{
	m_body->setBodyType(Box2DBody::Kinematic);
	m_body->setActive(true);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor));

	setSubZ(0.5);

	m_isSleeping = false;

	m_game->onEnemySleepingEnd(this);

	emit becameAwake();
}






/**
 * @brief IsometricEnemy::stepMotor
 */

void IsometricEnemy::stepMotor(const qreal &factor)
{
	if (!m_motor) {
		LOG_CERROR("game") << "Missing enemy motor:" << m_objectId.sceneId << m_objectId.id;
		return;
	}

	if (m_returnPathMotor) {
		if (m_returnPathMotor->isReturning() && !m_returnPathMotor->isReturnReady(m_game->tickTimer())) {
			m_body->stop();
			m_body->setIsRunning(false);
			rotateBody(directionToRadian(m_currentDirection));
			return;
		}

		if (m_returnPathMotor->hasReturned() || m_metric.returnSpeed == 0.) {
			m_body->stop();
			m_returnPathMotor.reset();
		} else {
			m_body->setIsRunning(false);
			m_returnPathMotor->updateBody(this, m_metric.returnSpeed > 0. ? m_metric.returnSpeed*factor : m_metric.speed*factor, m_game->tickTimer());
			return;
		}

	}

	m_body->setIsRunning(false);
	m_motor->updateBody(this, m_metric.speed*factor, m_game->tickTimer());

	rotateBody(directionToRadian(m_currentDirection));
}





/**
 * @brief IsometricEnemy::rotateToPlayer
 * @param player
 */

void IsometricEnemy::rotateToPlayer(IsometricPlayer *player, float32 *anglePtr, qreal *distancePtr)
{
	if (!player || !m_metric.rotateToPlayer)
		return;

	rotateToPoint(player->body()->bodyPosition(), anglePtr, distancePtr);
}






/**
 * @brief IsometricEnemy::sensorBeginContact
 * @param other
 */

void IsometricEnemy::sensorBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	IsometricPlayer *player = qobject_cast<IsometricPlayer*>(base);

	if (!player)
		return;

	if (!m_contactedPlayers.contains(player)) {
		m_contactedPlayers.append(QPointer(player));
		eventPlayerContacted(player);
	}
}



/**
 * @brief IsometricEnemy::sensorEndContact
 * @param other
 */

void IsometricEnemy::sensorEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	IsometricPlayer *player = qobject_cast<IsometricPlayer*>(base);

	if (!player)
		return;

	removeContactedPlayer(player);
	eventPlayerDiscontacted(player);
}




/**
 * @brief IsometricEnemy::fixtureBeginContact
 * @param other
 */

void IsometricEnemy::fixtureBeginContact(Box2DFixture *other)
{
	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody))) {

		TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
		IsometricPlayer *player = qobject_cast<IsometricPlayer*>(base);

		if (!player)
			return;

		if (!m_reachedPlayers.contains(player)) {
			m_reachedPlayers.append(QPointer(player));
			eventPlayerReached(player);
		}
	}
}




/**
 * @brief IsometricEnemy::fixtureEndContact
 * @param other
 */

void IsometricEnemy::fixtureEndContact(Box2DFixture *other)
{
	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody))) {

		TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
		IsometricPlayer *player = qobject_cast<IsometricPlayer*>(base);

		if (!player)
			return;

		m_reachedPlayers.removeAll(QPointer(player));
		eventPlayerLeft(player);
	}
}






/**
 * @brief IsometricEnemyIface::loadPathMotor
 * @param polygon
 */

void IsometricEnemyIface::loadPathMotor(const QPolygonF &polygon, const TiledPathMotor::Direction &direction)
{
	Q_ASSERT(!m_motor.get());

	TiledPathMotor *motor = new TiledPathMotor;

	motor->setPolygon(polygon);
	motor->setDirection(direction);
	motor->setWaitAtBegin(3500);
	motor->setWaitAtEnd(3500);

	m_motor.reset(motor);

	onPathMotorLoaded(AbstractTiledMotor::PathMotor);
}


/**
 * @brief IsometricEnemyIface::loadFixPositionMotor
 * @param point
 * @param defaultAngle
 */

void IsometricEnemyIface::loadFixPositionMotor(const QPointF &point, const TiledObject::Direction &direction)
{
	Q_ASSERT(!m_motor.get());

	TiledFixPositionMotor *motor = new TiledFixPositionMotor;

	motor->setPoint(point);
	motor->setDirection(direction);

	m_motor.reset(motor);

	onPathMotorLoaded(AbstractTiledMotor::FixPositionMotor);
}


/**
 * @brief IsometricEnemyBase::player
 * @return
 */

IsometricPlayer *IsometricEnemyIface::player() const
{
	return m_player;
}

void IsometricEnemyIface::setPlayer(IsometricPlayer *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();
}


/**
 * @brief IsometricEnemyIface::removeContactedPlayer
 * @param player
 */

void IsometricEnemyIface::removeContactedPlayer(IsometricPlayer *player)
{
	if (!player)
		return;

	m_contactedPlayers.removeAll(player);
	m_reachedPlayers.removeAll(player);
	if (m_player == player)
		setPlayer(nullptr);
}





float IsometricEnemyIface::playerDistance() const
{
	return m_playerDistance;
}

void IsometricEnemyIface::setPlayerDistance(float newPlayerDistance)
{
	if (qFuzzyCompare(m_playerDistance, newPlayerDistance))
		return;
	m_playerDistance = newPlayerDistance;
	emit playerDistanceChanged();
}



