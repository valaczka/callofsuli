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
	setDefaultZ(1);
	setSubZ(0.5);

	if (m_metric.targetCircleRadius <= 0)
		m_metric.targetCircleRadius = 50.;

	setSensorPolygon(m_metric.sensorLength, m_metric.sensorRange, FixtureTarget);
	addTargetCircle(m_metric.targetCircleRadius);

	createVisual();

	m_visualItem->setZ(1);

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
	if (!weapon || !player || player->isLocked() || !m_game)
		return;

	if (weapon->canHit()) {
		LOG_CTRACE("game") << "Enemy hit player:" << this << player << weapon;
		if (weapon->hit(player))
			playAttackEffect(weapon);
	} else if (weapon->canShot()) {
		LOG_CTRACE("game") << "Enemy shot player:" << this << player << weapon;
		if (m_game->shot(this, weapon, scene(), IsometricBullet::TargetPlayer, angleToPoint(player->bodyPosition())))
			playAttackEffect(weapon);
	}
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

bool IsometricEnemy::enemyWorldStepOnVisiblePlayer()
{
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

void IsometricEnemy::stepMotor()
{
	if (!m_motor) {
		LOG_CERROR("game") << "Missing enemy motor:" << objectId().sceneId << objectId().id;
		return;
	}

	if (m_returnPathMotor) {
		if (m_returnPathMotor->isReturning() && !m_returnPathMotor->isReturnReady(m_game->tickTimer())) {
			stop();
			return;
		}

		if (m_returnPathMotor->hasReturned() || m_metric.returnSpeed == 0.) {
			stop();
			m_returnPathMotor.reset();
		} else {
			m_returnPathMotor->updateBody(this, m_metric.returnSpeed > 0. ? m_metric.returnSpeed : m_metric.speed, m_game->tickTimer());
			return;
		}

	}

	m_motor->updateBody(this, m_metric.speed, m_game->tickTimer());
}









/**
 * @brief IsometricEnemy::rotateToPlayer
 * @param player
 */

void IsometricEnemy::rotateToPlayer(IsometricPlayer *player, const bool &forced)
{
	if (!player || !m_metric.rotateToPlayer)
		return;

	rotateToPoint(player->bodyPosition(), forced);
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
			m_reachedPlayers.append(player);
			eventPlayerReached(player);
		}
	} else if (isEqual(sensorPolygon(), self) && player) {
		if (!m_contactedPlayers.contains(player)) {
			m_contactedPlayers.append(player);
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
 * @brief IsometricEnemy::worldStep
 */

void IsometricEnemy::worldStep()
{
	IsometricEntity::worldStep();

	if (!isAlive() || isSleeping())
		return stop();

	if (m_isSleeping)
		onSleepingEnd();


	// Find already connected player, or find the nearest one

	IsometricPlayer *targetPlayer = nullptr;
	[[deprecated]] TiledReportedFixtureMap targetRayMap;

	if (m_player && m_contactedPlayers.contains(m_player)) {
		if (TiledReportedFixtureMap map = rayCast(m_player->bodyPosition(), FixturePlayerBody, false); map.contains(m_player)) {
			targetPlayer = m_player;
			targetRayMap = map;
		}
	} else {
		struct Data {
			IsometricPlayer *player = nullptr;
			TiledReportedFixtureMap map;
		};

		QMap<float, Data> maps;

		for (IsometricPlayer *p : m_contactedPlayers) {
			if (!p)
				continue;

			Data d;

			d.map = rayCast(p->bodyPosition(), FixturePlayerBody, false);

			if (const auto &it = d.map.find(p); it != d.map.cend()) {
				d.player = p;
				maps.insert(it.key(), d);
			}
		}

		if (!maps.isEmpty()) {
			targetPlayer = maps.first().player;
			targetRayMap = maps.first().map;
		}
	}



	// Update connected player, decision of pursuit


	std::optional<QPointF> pursuitPoint;
	bool attackWithoutPursuit = false;

	if (targetPlayer) {
		if (!m_player)
			setPlayer(targetPlayer);

		if (m_returnPathMotor && !m_player->isLocked())
			m_returnPathMotor->clearLastSeenPoint();

		if (!m_reachedPlayers.contains(m_player)) {
			if (m_metric.pursuitSpeed > 0) {		// Pursuit
				pursuitPoint = m_player->bodyPosition();
			} else {								// No pursuit
				attackWithoutPursuit = true;
				stop();
			}
		} else {
			stop();
		}

		const qreal dist = distanceToPoint(m_player->bodyPosition());

		rotateToPlayer(m_player);
		setPlayerDistance(dist);
	} else {
		if (m_player) {
			if (m_returnPathMotor)
				m_returnPathMotor->setLastSeenPoint(m_player->bodyPosition());

			setPlayer(nullptr);
		}

		if (m_returnPathMotor && !m_returnPathMotor->isReturning())
			pursuitPoint = m_returnPathMotor->lastSeenPoint();
	}



	// Pursuit

	/*const b2BodyType type = (m_returnPathMotor && m_returnPathMotor->isReturning() ? b2_kinematicBody : b2_dynamicBody);

	if (body().GetType() != type) {
		LOG_CINFO("game") << "SET FLAG" << this << type;
		body().SetType(type);
	}*/


	if (pursuitPoint.has_value() && distanceToPoint(pursuitPoint.value()) < 5.)
		pursuitPoint = std::nullopt;

	if (pursuitPoint.has_value()) {
		if (m_player && m_player->isLocked())
			return stop();

		if (!enemyWorldStepOnVisiblePlayer())
			return;

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite()))
			return stop();

		const QVector2D dst(pursuitPoint.value());
		const TiledReportedFixtureMap map = rayCast(pursuitPoint.value(), FixtureGround, true);

		if (!map.isEmpty() && distanceToPoint(map.first().point) <= m_metric.targetCircleRadius+5.) {	// inkább body size kéne
			stop();
		} else {
			if (m_metric.returnSpeed != 0) {
				if (!m_returnPathMotor)
					m_returnPathMotor.reset(new TiledReturnPathMotor(bodyPosition()));

				if (m_metric.pursuitSpeed > 0)
					m_returnPathMotor->moveBody(this, dst, m_metric.pursuitSpeed);
				else
					m_returnPathMotor->moveBody(this, dst, m_metric.speed);
			} else if (m_metric.speed > 0) {
				if (m_metric.pursuitSpeed > 0)
					moveTowards(dst, m_metric.pursuitSpeed);
				else
					moveTowards(dst, m_metric.speed);
			} else {
				stop();
			}

			enemyWorldStep();
			return;
		}

	} else if (attackWithoutPursuit) {
		if (!enemyWorldStepOnVisiblePlayer())
			return;

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite()))
			return stop();
	}


	if (m_returnPathMotor && !m_returnPathMotor->isReturning() && !m_returnPathMotor->hasReturned())
		m_returnPathMotor->finish(this, m_game->tickTimer());

	if (enemyWorldStep())
		stepMotor();
}




/**
 * @brief IsometricEnemy::synchronize
 */

void IsometricEnemy::synchronize()
{
	IsometricEntity::synchronize();
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



