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

IsometricEnemy::IsometricEnemy(TiledGame *game, const qreal &radius)
	: IsometricEntity(game, radius, CP_BODY_TYPE_KINEMATIC)
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

	setSensorPolygon(m_metric.sensorLength, m_metric.sensorRange, FixtureTarget|FixturePlayerBody);
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
	return currentSpeedSq() >= POW2(m_metric.pursuitSpeed)*0.9;
}


/**
 * @brief IsometricEnemy::isWalking
 * @return
 */

bool IsometricEnemy::isWalking() const
{
	const float &l = currentSpeedSq();
	return l < POW2(m_metric.pursuitSpeed) && l > POW2(0.05);
}


/**
 * @brief IsometricEnemy::startInability
 */

void IsometricEnemy::startInability()
{
	if (m_metric.inabilityTime > 0 && m_game->tickTimer())
		m_inabilityTimer = m_game->tickTimer()->tickAddMsec(m_metric.inabilityTime);
}


/**
 * @brief IsometricEnemy::startSleeping
 */

bool IsometricEnemy::startSleeping()
{
	if (m_metric.sleepingTime > 0 && m_game->tickTimer()) {
		m_sleepingTimer = m_game->tickTimer()->tickAddMsec(m_metric.sleepingTime);
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
	setSubZ(0.5);
	emit becameAlive();
}





/**
 * @brief IsometricEnemy::onDead
 */

void IsometricEnemy::onDead()
{
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
	setSubZ(0.0);
	m_game->onEnemySleepingStart(this);

	emit becameAsleep();
}




/**
 * @brief IsometricEnemy::onSleepingEnd
 */

void IsometricEnemy::onSleepingEnd()
{
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
 * @brief IsometricEnemy::featureOverride
 * @param feature
 * @return true if the feature has overidden
 */

bool IsometricEnemy::featureOverride(const PlayerFeature &feature, IsometricPlayer *player) const
{
	Q_UNUSED(feature);
	Q_UNUSED(player);
	return false;
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

void IsometricEnemy::onShapeContactBegin(cpShape *self, cpShape *other)
{
	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(other).categories);

	IsometricPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
								  dynamic_cast<IsometricPlayer*>(base) :
								  nullptr;


	if (isBodyShape(self) && player) {
		if (!m_reachedPlayers.contains(player)) {
			m_reachedPlayers.append(player);
			eventPlayerReached(player);
		}
	} else if (sensorPolygon() == self && player && player->isBodyShape(other)) {
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

void IsometricEnemy::onShapeContactEnd(cpShape *self, cpShape *other)
{
	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(other).categories);

	IsometricPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody) ?
								  dynamic_cast<IsometricPlayer*>(base) :
								  nullptr;


	if (self == targetCircle() && player) {
		m_reachedPlayers.removeAll(player);
		eventPlayerLeft(player);
	} else if (sensorPolygon() == self && player) {
		removeContactedPlayer(player);
		eventPlayerDiscontacted(player);
	}
}


/**
 * @brief IsometricEnemy::onShapeAboutToDelete
 * @param shape
 */

void IsometricEnemy::onShapeAboutToDelete(cpShape *shape)
{
	removeContactedPlayer(dynamic_cast<IsometricPlayer*>(TiledObjectBody::fromShapeRef(shape)));
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
	IsometricPlayer *tmpPlayer = nullptr;

	if (m_player && m_contactedPlayers.contains(m_player) && m_player->isAlive() && !m_player->isLocked()) {
		if (rayCast(m_player->bodyPosition(), FixturePlayerBody).isVisible(m_player) && !featureOverride(FeatureVisibility, m_player)) {
			tmpPlayer = m_player;
		}
	}

	if (!tmpPlayer || featureOverride(FeatureReplaceFrom, tmpPlayer)) {
		QMap<float, IsometricPlayer *> pMap;

		for (IsometricPlayer *p : m_contactedPlayers) {
			if (!p || !p->isAlive() || p->isLocked() || p == tmpPlayer)
				continue;

			const RayCastInfo &info = rayCast(p->bodyPosition(), FixturePlayerBody);

			for (const RayCastInfoItem &item : info) {
				if (TiledObjectBody::fromShapeRef(item.shape) == p && item.visible && !featureOverride(FeatureVisibility, p))
					pMap.insert(distanceToPointSq(item.point), p);
			}
		}

		for (auto it = pMap.constBegin(); it != pMap.constEnd() && !targetPlayer; ++it) {
			if (!tmpPlayer || featureOverride(FeatureReplaceTo, *it))
				targetPlayer = *it;
		}
	}

	if (!targetPlayer && tmpPlayer)
		targetPlayer = tmpPlayer;



	// Update connected player, decision of pursuit


	std::optional<cpVect> pursuitPoint;
	bool attackWithoutPursuit = false;

	if (targetPlayer) {
		if (!m_player || m_autoHitTimer == -1)
			setPlayer(targetPlayer);

		const cpVect playerPosition = m_player->bodyPosition();

		if (!m_reachedPlayers.contains(m_player)) {
			if (featureOverride(FeatureDisablePursuit, m_player)) {
				stop();
			} else if (featureOverride(FeaturePursuit, m_player)) {
				pursuitPoint = playerPosition;
			} else if (featureOverride(FeatureAttackNotReached, m_player)) {
				pursuitPoint = playerPosition;
				attackWithoutPursuit = true;
				stop();
			} else if (m_metric.pursuitSpeed != 0) {		// Pursuit
				pursuitPoint = playerPosition;
			}
		} else {
			stop();
		}

		if (!featureOverride(FeatureRotate, m_player))
			rotateToPlayer(m_player);

		setPlayerDistance(distanceToPointSq(playerPosition));
	} else {
		if (m_player)
			setPlayer(nullptr);

		if (m_destinationPoint.has_value())
			pursuitPoint = m_destinationPoint;
		else if (m_destinationMotor && !m_destinationMotor->polygon().isEmpty())
			pursuitPoint = TiledObjectBody::toVect(m_destinationMotor->polygon().last());
	}


	// Pursuit

	if (pursuitPoint.has_value() && distanceToPointSq(pursuitPoint.value()) < POW2(5.))
		pursuitPoint = std::nullopt;

	if (!pursuitPoint)
		clearDestinationPoint();

	if (pursuitPoint.has_value()) {

		const cpVect &dst = pursuitPoint.value();

		bool findPath = true;

		if (m_destinationPoint.has_value() && cpveql(m_destinationPoint.value(), dst))
			findPath = false;
		else if (m_destinationMotor && !m_destinationMotor->polygon().isEmpty()) {
			const QPointF &p = m_destinationMotor->polygon().last();
			if (p.x() == dst.x && p.y() == dst.y)
				findPath = false;
		}

		if (findPath) {
			if (const auto &ptr = m_game->findShortestPath(this, dst)) {
				if (ptr->size() == 2) {
					setDestinationPoint(TiledObjectBody::toVect(ptr->last()));
				} else  {
					setDestinationPoint(ptr.value());
				}
			} else {
				setDestinationPoint(dst);
			}
		}

		if (m_player && attackWithoutPursuit && !enemyWorldStepNotReachedPlayer())
			return;

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite()))
			return stop();

		if (m_destinationPoint) {
			const RayCastInfo &map = rayCast(m_destinationPoint.value(), FixtureGround);

			if (!map.empty() && distanceToPointSq(map.front().point) <= POW2(m_metric.targetCircleRadius)) {
				clearDestinationPoint();
				stop();
			}
		}

		if (m_destinationPoint) {
			if (m_metric.returnSpeed != 0 && !m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(bodyPosition()));

			bool success = false;

			if (m_metric.speed > 0) {
				if (m_metric.pursuitSpeed > 0)
					success = moveTowards(m_destinationPoint.value(), m_metric.pursuitSpeed);
				else
					success = moveTowards(m_destinationPoint.value(), m_metric.speed);

				if (success && m_returnPathMotor)
					m_returnPathMotor->record(this);
			}


			if (!success) {
				clearDestinationPoint();
				stop();
			}

			enemyWorldStep();
			return;

		} else if (m_destinationMotor) {
			if (m_metric.returnSpeed != 0 && !m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(bodyPosition()));

			bool success = false;

			if (!m_destinationMotor->atEnd(this) && m_metric.speed > 0) {
				if (const QPolygonF &polygon = m_destinationMotor->polygon(); !polygon.isEmpty()) {
					m_destinationMotor->updateBody(this,
												   m_metric.pursuitSpeed > 0 ? m_metric.pursuitSpeed :
																			   m_metric.speed,
												   m_game->tickTimer());

					if (m_returnPathMotor)
						m_returnPathMotor->record(this);

					success = true;
				}
			}

			if (!success) {
				clearDestinationPoint();
				stop();
			}

			enemyWorldStep();
			return;

		} else {
			clearDestinationPoint();
			stop();
		}


	} else if (attackWithoutPursuit) {
		if (!enemyWorldStepNotReachedPlayer())
			return;

		if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite()))
			return stop();
	}


	if (m_returnPathMotor && !m_returnPathMotor->isReturning() && !m_returnPathMotor->hasReturned()) {
		m_returnPathMotor->finish(this, m_game->tickTimer());
		clearDestinationPoint();
	}

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

void IsometricEnemyIface::loadFixPositionMotor(const TiledGame::EnemyMotorData &data, const TiledObject::Direction &direction)
{
	Q_ASSERT(!m_motor.get());
	Q_ASSERT(!data.path.isEmpty());

	if (data.rotation) {
		TiledRotationMotor *motor = new TiledRotationMotor;

		motor->setPoint(TiledObject::toVect(data.path.first()));
		motor->setFrom(data.from);
		motor->setTo(data.to);
		motor->setDirection(data.direction);
		motor->setSteps(data.steps);
		motor->setWaitMs(data.wait);

		m_motor.reset(motor);

		onPathMotorLoaded(AbstractTiledMotor::RotationMotor);
	} else {
		TiledFixPositionMotor *motor = new TiledFixPositionMotor;

		motor->setPoint(TiledObject::toVect(data.path.first()));
		motor->setDirection(direction);

		m_motor.reset(motor);

		onPathMotorLoaded(AbstractTiledMotor::FixPositionMotor);
	}
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
	return m_playerDistanceSq;
}

void IsometricEnemyIface::setPlayerDistance(float newPlayerDistance)
{
	if (qFuzzyCompare(m_playerDistanceSq, newPlayerDistance))
		return;
	m_playerDistanceSq = newPlayerDistance;
	emit playerDistanceChanged();
}



