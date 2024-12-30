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
#include "isometricplayer.h"
#include "tiledfixpositionmotor.h"
#include "tiledspritehandler.h"






/**
 * @brief IsometricEnemy::IsometricEnemy
 * @param parent
 */

IsometricEnemy::IsometricEnemy(TiledScene *scene)
	: IsometricEntity(scene)
	, IsometricEnemyIface()
{

}





/**
 * @brief IsometricEnemy::initialize
 */

void IsometricEnemy::initialize()
{
	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	b2::Body::Params bParams;
	bParams.type = b2BodyType::b2_kinematicBody;
	bParams.fixedRotation = true;

	b2::Shape::Params params;
	params.density = 1.f;
	params.friction = 1.f;
	params.restitution = 0.f;
	params.filter = TiledObjectBody::getFilter(FixtureEnemyBody, FixturePlayerBody | FixtureTarget);

	createFromCircle({0.f, 0.f}, 15., nullptr, bParams, params);
	setSensorPolygon(m_metric.sensorLength, m_metric.sensorRange, FixtureTarget);
	addTargetCircle(m_metric.targetCircleRadius > 0. ? m_metric.targetCircleRadius : 50.);

	createVisual();

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
 * @brief IsometricEnemy::isRunning
 * @return
 */

bool IsometricEnemy::isRunning() const
{
	// 60 FPS
	return currentSpeed().length() >= m_metric.pursuitSpeed*0.9/60;
}


/**
 * @brief IsometricEnemy::isWalking
 * @return
 */

bool IsometricEnemy::isWalking() const
{
	const auto &l = currentSpeed().length();
	return l < m_metric.pursuitSpeed/60 && l > 0.05;
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
		if (weapon->shot(IsometricBullet::TargetPlayer, bodyPosition(), angleToPoint(player->bodyPosition())))
			playAttackEffect(weapon);
	}
}





/**
 * @brief IsometricEnemy::entityWorldStep
 */
/*
void IsometricEnemy::entityWorldStep(const qreal &factor)
{
	if (!isAlive() || isSleeping()) {
		stop();
		jumpToSprite("death", m_facingDirection);
		return;
	}

	if (m_isSleeping)
		onSleepingEnd();

	if (m_movingDirection != Invalid)
		setFacingDirection(m_movingDirection);


	//QPointF visiblePoint;
	float transparentGnd = -1.0;

	IsometricPlayer *myPlayer = getVisibleEntity<QPointer<IsometricPlayer>>(this, m_contactedPlayers,
																			TiledObjectBody::FixturePlayerBody,
																			&transparentGnd
																			///// / *, &visiblePoint * /);


	float angle = 0.;
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
				stop();
			}
		} else {
			stop();
		}
	} else {
		if (m_player) {
			if (m_returnPathMotor) {
				m_returnPathMotor->setLastSeenPoint(m_player->bodyPosition());
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
					if (const float &d = checkGroundDistance(this, ptr.value()); d != -1. && d < 0.05) {
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


	//TiledObjectBody::setFixtureCollidesWithFlag(m_fixture.get(), TiledObjectBody::FixtureGround, !m_returnPathMotor);


	if (isPursuit) {
		if (m_player && m_player->isLocked()) {
			stop();
			updateSprite();
			return;
		}

		if (!enemyWorldStepOnVisiblePlayer(angle, factor)) {
			updateSprite();
			return;
		}

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
			stop();
			updateSprite();
			return;
		}

		if (transparentGnd >= 0. && transparentGnd < 35 /// / * m_fixture->radius()*2.3 * /) {
			stop();
		} else if (m_metric.returnSpeed != 0) {
			if (!m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(bodyPosition()));

			if (m_metric.pursuitSpeed > 0 && m_playerDistance > m_metric.pursuitSpeed*factor) {
				m_returnPathMotor->moveBody(this, angle, m_metric.pursuitSpeed*factor);
			} else {
				m_returnPathMotor->moveBody(this, angle, m_metric.speed*factor);
			}
		} else if (m_metric.speed > 0) {
			if (m_metric.pursuitSpeed > 0 && m_playerDistance > m_metric.pursuitSpeed*factor) {
				setSpeedFromAngle(angle, m_metric.pursuitSpeed);

			} else {
				setSpeedFromAngle(angle, m_metric.speed);
			}
		} else {
			stop();
		}


		enemyWorldStep();
		updateSprite();
		return;
	} else if (attackWithoutPursuit) {
		if (!enemyWorldStepOnVisiblePlayer(angle, factor)) {
			updateSprite();
			return;
		}

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
			stop();
			updateSprite();
			return;
		}
	}

	if (m_returnPathMotor && !m_returnPathMotor->isReturning() && !m_returnPathMotor->hasReturned()) {
		m_returnPathMotor->finish(this, m_game->tickTimer());
	}


	if (enemyWorldStep()) {
		//rotateBody(directionToRadian(m_currentDirection));
		stepMotor(factor);
	}

	updateSprite();
}

*/

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
		stop();

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

bool IsometricEnemy::enemyWorldStepOnVisiblePlayer(const float &angle, const qreal &factor)
{
	Q_UNUSED(angle);
	Q_UNUSED(factor);

	if (!isAlive() || isSleeping())
		return false;

	if (m_metric.autoAttackTime <= 0 || m_metric.firstAttackTime <= 0)
		return true;

	if (defaultWeapon() && defaultWeapon()->canShot() &&
			m_player && m_contactedPlayers.contains(m_player) && m_player->isAlive()) {
		stop();

		if (m_metric.returnSpeed != 0) {
			if (!m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(bodyPosition()));
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
	emplace(m_motor->basePoint());
}





/**
 * @brief IsometricEnemy::onAlive
 */

void IsometricEnemy::onAlive()
{
	setBodyEnabled(true);
	setSubZ(0.5);
	emit becameAlive();
}





/**
 * @brief IsometricEnemy::onDead
 */

void IsometricEnemy::onDead()
{
	setBodyEnabled(false);
	setSubZ(0.0);
	m_game->onEnemyDead(this);
	emit becameDead();
}



/**
 * @brief IsometricEnemy::onSleepingBegin
 */

void IsometricEnemy::onSleepingBegin()
{
	m_isSleeping = true;
	setBodyEnabled(false);
	setSubZ(0.0);
	m_game->onEnemySleepingStart(this);

	emit becameAsleep();
}




/**
 * @brief IsometricEnemy::onSleepingEnd
 */

void IsometricEnemy::onSleepingEnd()
{
	setBodyEnabled(true);
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
		LOG_CERROR("game") << "Missing enemy motor:" << objectId().sceneId << objectId().id;
		return;
	}

	if (m_returnPathMotor) {
		if (m_returnPathMotor->isReturning() && !m_returnPathMotor->isReturnReady(m_game->tickTimer())) {
			stop();
			rotateBody(directionToRadian(m_facingDirection));
			return;
		}

		if (m_returnPathMotor->hasReturned() || m_metric.returnSpeed == 0.) {
			stop();
			m_returnPathMotor.reset();
		} else {
			m_returnPathMotor->updateBody(this, m_metric.returnSpeed > 0. ? m_metric.returnSpeed*factor : m_metric.speed*factor, m_game->tickTimer());
			return;
		}

	}

	m_motor->updateBody(this, m_metric.speed*factor, m_game->tickTimer());

	rotateBody(directionToRadian(m_facingDirection));
}









/**
 * @brief IsometricEnemy::rotateToPlayer
 * @param player
 */

void IsometricEnemy::rotateToPlayer(IsometricPlayer *player, float *anglePtr, qreal *distancePtr)
{
	if (!player || !m_metric.rotateToPlayer)
		return;

	rotateToPoint(player->bodyPosition(), anglePtr, distancePtr);
}




/**
 * @brief IsometricEnemy::onShapeContactBegin
 * @param self
 * @param other
 */

void IsometricEnemy::onShapeContactBegin(b2::ShapeRef self, b2::ShapeRef other)
{
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	IsometricPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
								  dynamic_cast<IsometricPlayer*>(base) :
								  nullptr;


	if (isAny(bodyShapes(), self) && player) {
		if (!m_reachedPlayers.contains(player)) {
			m_reachedPlayers.append(QPointer(player));
			eventPlayerReached(player);
		}
	} else if (isEqual(sensorPolygon(), self) && player) {
		if (!m_contactedPlayers.contains(player)) {
			m_contactedPlayers.append(QPointer(player));
			eventPlayerContacted(player);
		}
	}
}


/**
 * @brief IsometricEnemy::onShapeContactEnd
 * @param self
 * @param other
 */

void IsometricEnemy::onShapeContactEnd(b2::ShapeRef self, b2::ShapeRef other)
{
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	IsometricPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
								  dynamic_cast<IsometricPlayer*>(base) :
								  nullptr;


	if (isEqual(self, targetCircle()) && player) {
		m_reachedPlayers.removeAll(player);
		eventPlayerLeft(player);
	} else if (isEqual(sensorPolygon(), self) && player) {
		removeContactedPlayer(player);
		eventPlayerDiscontacted(player);
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



