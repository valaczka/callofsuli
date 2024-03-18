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
#include "isometricwerebear.h"



const QHash<QString, IsometricEnemyIface::EnemyType> IsometricEnemyIface::m_typeHash = {
	{ QStringLiteral("enemy"), EnemyWerebear }
};



/**
 * @brief IsometricEnemy::IsometricEnemy
 * @param parent
 */

IsometricEnemy::IsometricEnemy(const EnemyType &type, QQuickItem *parent)
	: IsometricCircleEntity(parent)
	, IsometricEnemyIface(type)
{
	m_inabilityTimer.setRemainingTime(-1);
	m_autoHitTimer.setRemainingTime(-1);

}



/**
 * @brief IsometricEnemy::createEnemy
 * @param parent
 * @return
 */

IsometricEnemy *IsometricEnemy::createEnemy(const EnemyType &type, const QString &subtype, TiledGame *game, TiledScene *scene)
{
	IsometricEnemy *enemy = nullptr;

	switch (type) {
		case EnemyWerebear: {
			IsometricWerebear *e = nullptr;
			TiledObjectBase::createFromCircle<IsometricWerebear>(&e, QPointF{}, 30, nullptr, game);
			e->setWerebearType(subtype);
			enemy = e;
			break;
		}

		case EnemyInvalid:
			LOG_CERROR("game") << "Invalid enemy type" << type;
			return nullptr;
	}

	if (enemy) {
		enemy->setParent(game);
		enemy->setGame(game);
		enemy->setScene(scene);
		enemy->initialize();
	}

	return enemy;
}



/**
 * @brief IsometricEnemy::initialize
 */

void IsometricEnemy::initialize()
{
	m_body->setBodyType(Box2DBody::Dynamic);

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

bool IsometricEnemy::hasAbility() const
{
	return m_inabilityTimer.isForever() || m_inabilityTimer.hasExpired();
}


/**
 * @brief IsometricEnemy::attackedByPlayer
 * @param player
 */

void IsometricEnemy::attackedByPlayer(IsometricPlayer *player, const TiledWeapon::WeaponType &/*weaponType*/)
{
	if (!isAlive())
		return;

	if (!m_contactedPlayers.contains(player))
		m_contactedPlayers.append(player);

	setPlayer(player);
	rotateToPlayer(player);
}


/**
 * @brief IsometricEnemy::startInabililty
 */

void IsometricEnemy::startInabililty()
{
	if (m_metric.inabilityTime > 0)
		m_inabilityTimer.setRemainingTime(m_metric.inabilityTime);
}



/**
 * @brief IsometricEnemy::attackPlayer
 * @param player
 */

void IsometricEnemy::attackPlayer(IsometricPlayer *player, TiledWeapon *weapon)
{
	if (!weapon || !player)
		return;

	if (weapon->canHit()) {
		LOG_CTRACE("game") << "Enemy hit player:" << this << player << weapon;
		//m_game->enemyAttackPlayer(this, player, weapon->weaponType());
		weapon->hit(player);
		jumpToSprite(m_autoAttackSprite);			///TODO
	} else if (weapon->canShot()) {
		LOG_CTRACE("game") << "Enemy shot player:" << this << player << weapon;
		weapon->shot(IsometricBullet::TargetPlayer, m_body->bodyPosition(), angleToPoint(player->body()->bodyPosition()));
	}
}




/**
 * @brief IsometricEnemy::entityWorldStep
 */

void IsometricEnemy::entityWorldStep()
{
	if (!isAlive()) {
		m_body->stop();
		jumpToSprite("die", m_currentDirection);
		return;
	}

	if (m_movingDirection != Invalid)
		setCurrentDirection(m_movingDirection);


	//QPointF visiblePoint;
	IsometricPlayer *myPlayer = getVisibleEntity<QPointer<IsometricPlayer>>(m_body.get(), m_contactedPlayers,
																			TiledObjectBody::FixturePlayerBody
																			/*, &visiblePoint*/);


	float32 angle = 0.;
	bool isPursuit = false;

	if (myPlayer) {
		setPlayer(myPlayer);

		if (m_returnPathMotor)
			m_returnPathMotor->clearLastSeenPoint();

		qreal dist;

		rotateToPlayer(m_player, &angle, &dist);
		setPlayerDistance(dist);

		if (!m_reachedPlayers.contains(m_player)) {
			if (m_metric.pursuitSpeed > 0) {		// Pursuit
				isPursuit = true;
			} else {								// No pursuit
				m_body->stop();
			}
		} else {
			m_body->stop();
		}
	} else {
		if (m_player) {
			if (m_returnPathMotor) {
				m_returnPathMotor->setLastSeenPoint(m_player->body()->bodyPosition());
				isPursuit = true;
			}

			setPlayer(nullptr);
			setPlayerDistance(-1.);
		}

		if (m_returnPathMotor && !m_returnPathMotor->isReturning()) {
			const auto &ptr = m_returnPathMotor->lastSeenPoint();

			if (ptr) {
				const qreal &dist = distanceToPoint(ptr.value());

				if (dist >= std::max(std::max(m_metric.pursuitSpeed, m_metric.speed), 10.)){
					rotateToPoint(ptr.value(), &angle);
					isPursuit = true;
				} else {
					isPursuit = false;
				}
			}
		}
	}

	TiledObjectBody::setFixtureCollidesWithFlag(m_fixture.get(), TiledObjectBody::FixtureGround, !m_returnPathMotor);


	if (isPursuit) {
		if (m_metric.returnSpeed != 0) {
			if (!m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor);

			/** TiledObject::factorFromRadian(angle)*/		/// --- pursuit speeed
			m_returnPathMotor->moveBody(m_body.get(), angle,
										m_metric.pursuitSpeed > 0 ? m_metric.pursuitSpeed : m_metric.speed);
		} else if (m_metric.speed > 0) {
			m_body->setLinearVelocity(maximizeSpeed(TiledObjectBase::toPoint(angle, m_metric.speed)));
		} else {
			m_body->stop();
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
	}

	if (m_returnPathMotor && !m_returnPathMotor->isReturning()) {
		m_returnPathMotor->finish(m_body.get());
	}


	if (enemyWorldStep()) {
		rotateBody(directionToRadian(m_currentDirection));
		stepMotor();
	}

	updateSprite();
}



/**
 * @brief IsometricEnemy::enemyWorldStep
 */

bool IsometricEnemy::enemyWorldStep()
{
	if (!isAlive())
		return true;

	if (m_metric.autoAttackTime <= 0 || !m_autoAttackSprite)
		return true;

	if (m_player && m_reachedPlayers.contains(m_player) && m_player->isAlive()) {
		m_body->stop();

		if (!hasAbility())
			return false;

		if (m_autoHitTimer.hasExpired() || m_autoHitTimer.isForever()) {
			attackPlayer(m_player, m_defaultWeapon.get());
			m_autoHitTimer.setRemainingTime(m_metric.autoAttackTime);
		}

		return false;
	} else {
		m_autoHitTimer.setRemainingTime(-1);
	}

	return true;
}




/**
 * @brief IsometricEnemy::onPathMotorLoaded
 */

void IsometricEnemy::onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/)
{
	m_body->emplace(m_motor->currentPosition());
}





/**
 * @brief IsometricEnemy::onAlive
 */

void IsometricEnemy::onAlive()
{
	m_body->setBodyType(Box2DBody::Dynamic);
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

	emit becameDead();
}



/**
 * @brief IsometricEnemy::stepMotor
 */

void IsometricEnemy::stepMotor()
{
	if (!m_motor) {
		LOG_CERROR("game") << "Missing enemy motor:" << m_objectId.sceneId << m_objectId.id;
		return;
	}

	if (m_returnPathMotor) {
		if (m_returnPathMotor->isReturning() && !m_returnPathMotor->isReturnReady()) {
			m_body->stop();
			return;
		}
		if (m_metric.returnSpeed != 0. && m_returnPathMotor->step(m_metric.returnSpeed > 0. ? m_metric.returnSpeed : m_metric.speed)) {
			m_body->setLinearVelocity(maximizeSpeed(m_returnPathMotor->currentPosition() - m_body->bodyPosition()));
			return;
		} else {
			m_returnPathMotor.reset();
		}
	}

	m_motor->step(m_metric.speed);
	m_motor->updateBody(this, m_maximumSpeed);
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
 * @brief IsometricEnemy::rotateToPoint
 * @param point
 * @param anglePtr
 * @param vectorPtr
 */

void IsometricEnemy::rotateToPoint(const QPointF &point, float32 *anglePtr, qreal *distancePtr)
{
	const QPointF p = point - m_body->bodyPosition();

	float32 angle = atan2(-p.y(), p.x());

	setCurrentDirection(nearestDirectionFromRadian(angle));
	m_body->body()->SetTransform(m_body->body()->GetPosition(), angle);

	if (anglePtr)
		*anglePtr = angle;

	if (distancePtr)
		*distancePtr = QVector2D(p).length();
}



/**
 * @brief IsometricEnemy::angleToPoint
 * @param point
 * @return
 */

float32 IsometricEnemy::angleToPoint(const QPointF &point) const
{
	const QPointF p = point - m_body->bodyPosition();
	return atan2(-p.y(), p.x());
}



/**
 * @brief IsometricEnemy::distanceToPoint
 * @param point
 * @return
 */

qreal IsometricEnemy::distanceToPoint(const QPointF &point) const
{
	return QVector2D(point - m_body->bodyPosition()).length();
}






/**
 * @brief IsometricObject::updateSprite
 */

void IsometricEnemy::updateSprite()
{
	if (m_hp <= 0)
		jumpToSprite("die", m_currentDirection);
	else if (m_spriteHandler->currentSprite() == QStringLiteral("hit") || m_spriteHandler->currentSprite() == QStringLiteral("hurt"))
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection);
	else
		jumpToSprite("idle", m_currentDirection);
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

	if (!m_contactedPlayers.contains(player))
		m_contactedPlayers.append(player);
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

	m_contactedPlayers.removeAll(player);
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
			m_reachedPlayers.append(player);
			onPlayerReached(player);
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

		m_reachedPlayers.removeAll(player);
		onPlayerLeft(player);
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
	if (direction == TiledPathMotor::Forward)
		motor->toBegin();
	else
		motor->toEnd();

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



