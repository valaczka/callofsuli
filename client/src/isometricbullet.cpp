/*
 * ---- Call of Suli ----
 *
 * isometricbullet.cpp
 *
 * Created on: 2024. 03. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricBullet
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

#include "isometricbullet.h"
#include "isometricbullet_p.h"
#include "isometricenemy.h"
#include "tiledscene.h"



/**
 * @brief IsometricBullet::IsometricBullet
 * @param parent
 */

IsometricBullet::IsometricBullet(QQuickItem *parent)
	: IsometricObjectCircle(parent)
	, d(new IsometricBulletPrivate)
{

}



/**
 * @brief IsometricBullet::~IsometricBullet
 */

IsometricBullet::~IsometricBullet()
{
	delete d;
}




/**
 * @brief IsometricBullet::initialize
 */

void IsometricBullet::initialize()
{
	setZ(1);
	setDefaultZ(1);
	setSubZ(0.8);


	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setFixedRotation(true);
	m_body->setBullet(true);
	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround));
	m_fixture->setSensor(true);

	connect(m_fixture.get(), &Box2DCircle::beginContact, this, &IsometricBullet::fixtureBeginContact);
	//connect(m_fixture.get(), &Box2DCircle::endContact, this, &IsometricBullet::fixtureEndContact);

	load();
}



/**
 * @brief IsometricBullet::shot
 * @param from
 * @param direction
 * @param distance
 */

void IsometricBullet::shot(const QPointF &from, const Direction &direction)
{
	if (!m_scene)
		return;

	m_startPoint = from;
	m_body->emplace(from);
	setCurrentDirection(direction);
	m_direction = direction;
	m_angle = 0.;
	jumpToSprite("default", m_currentDirection);
	m_scene->appendToObjects(this);
}


/**
 * @brief IsometricBullet::shot
 * @param from
 * @param angle
 */

void IsometricBullet::shot(const QPointF &from, const qreal &angle)
{
	if (!m_scene)
		return;

	m_startPoint = from;
	m_body->emplace(from);
	setCurrentDirection(nearestDirectionFromRadian(angle));
	m_direction = Invalid;
	m_angle = angle;
	jumpToSprite("default", m_currentDirection);
	m_scene->appendToObjects(this);
}



/**
 * @brief IsometricBullet::shot
 * @param targets
 * @param from
 * @param direction
 */

void IsometricBullet::shot(const Targets &targets, const QPointF &from, const Direction &direction)
{
	setTargets(targets);
	shot(from, direction);
}


/**
 * @brief IsometricBullet::shot
 * @param targets
 * @param from
 * @param angle
 */

void IsometricBullet::shot(const Targets &targets, const QPointF &from, const qreal &angle)
{
	setTargets(targets);
	shot(from, angle);
}



/**
 * @brief IsometricBullet::worldStep
 */

void IsometricBullet::worldStep(const qreal &factor)
{
	if (m_currentDirection == Invalid) {
		m_body->stop();
		return;
	}

	const qreal &distance = QVector2D(m_startPoint - m_body->bodyPosition()).length();

	if (distance >= m_maxDistance) {
		overshootEvent();
		doAutoDelete();
		return;
	}

	if (m_direction != Invalid) {
		m_body->setLinearVelocity(TiledObjectBase::toPoint(directionToIsometricRaidan(m_direction), m_speed*factor));
	} else {
		m_body->setLinearVelocity(TiledObjectBase::toPoint(m_angle, m_speed*factor));
	}

	jumpToSprite("default", m_currentDirection);
}





/**
 * @brief IsometricBullet::maxDistance
 * @return
 */

qreal IsometricBullet::maxDistance() const
{
	return m_maxDistance;
}

void IsometricBullet::setMaxDistance(qreal newMaxDistance)
{
	if (qFuzzyCompare(m_maxDistance, newMaxDistance))
		return;
	m_maxDistance = newMaxDistance;
	emit maxDistanceChanged();
}

qint64 IsometricBullet::bulletId() const
{
	return m_bulletId;
}

void IsometricBullet::setBulletId(qint64 newBulletId)
{
	if (m_bulletId == newBulletId)
		return;
	m_bulletId = newBulletId;
	emit bulletIdChanged();
}




/**
 * @brief IsometricBullet::fixtureBeginContact
 * @param other
 */

void IsometricBullet::fixtureBeginContact(Box2DFixture *other)
{
	if (m_impacted) {
		m_body->stop();
		return;
	}

	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround)) &&
			base && base->body()->opaque()) {
		setImpacted(true);
		m_body->stop();
		setCurrentDirection(Invalid);
		groundEvent(base);
		doAutoDelete();
		return;
	}


	bool hasTarget = false;


	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

	if (m_targets.testFlag(TargetEnemy) && enemy) {
		hasTarget = enemy->canBulletImpact(d->m_fromWeaponType);
	}

	if (IsometricPlayer *p = qobject_cast<IsometricPlayer*>(base); m_targets.testFlag(TargetPlayer) && p && !p->isLocked())
		hasTarget = true;

	if (!hasTarget)
		return;

	impactEvent(base);
}


/**
 * @brief IsometricBullet::doAutoDelete
 */

void IsometricBullet::doAutoDelete()
{
	if (m_scene)
		m_scene->removeFromObjects(this);

	setVisible(false);
	this->deleteLater();
}



/**
 * @brief IsometricBullet::setFromWeapon
 * @param newFromWeapon
 */

void IsometricBullet::setFromWeapon(TiledWeapon *newFromWeapon)
{
	d->m_fromWeapon = newFromWeapon;
	d->m_fromWeaponType = newFromWeapon ? newFromWeapon->weaponType() : TiledWeapon::WeaponInvalid;
	d->m_owner = newFromWeapon ? newFromWeapon->parentObject() : nullptr;
}



/**
 * @brief IsometricBullet::impactEvent
 * @param base
 */

void IsometricBullet::impactEvent(TiledObjectBase *base)
{
	if (!d->m_owner) {
		LOG_CWARNING("game") << "Missing owner, bullet automatic impact event failed";
		return;
	}

	TiledGame *game = d->m_owner->game();

	if (!game) {
		LOG_CWARNING("game") << "Missing game, bullet automatic impact event failed";
		return;
	}

	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);
	IsometricPlayer *player = qobject_cast<IsometricPlayer*>(base);

	if (enemy)
		game->playerAttackEnemy(d->m_owner, enemy, d->m_fromWeaponType);

	if (player)
		game->enemyAttackPlayer(d->m_owner, player, d->m_fromWeaponType);

	/// TODO: player attack player?

	setImpacted(true);
	m_body->stop();
	setCurrentDirection(Invalid);
	doAutoDelete();
}


/**
 * @brief IsometricBullet::targets
 * @return
 */

IsometricBullet::Targets IsometricBullet::targets() const
{
	return m_targets;
}

void IsometricBullet::setTargets(const Targets &newTargets)
{
	if (m_targets == newTargets)
		return;
	m_targets = newTargets;
	emit targetsChanged();
}



/**
 * @brief IsometricBullet::autoDelete
 * @return
 */

bool IsometricBullet::autoDelete() const
{
	return m_autoDelete;
}

void IsometricBullet::setAutoDelete(bool newAutoDelete)
{
	if (m_autoDelete == newAutoDelete)
		return;
	m_autoDelete = newAutoDelete;
	emit autoDeleteChanged();
}




/**
 * @brief IsometricBullet::impacted
 * @return
 */

bool IsometricBullet::impacted() const
{
	return m_impacted;
}

void IsometricBullet::setImpacted(bool newImpacted)
{
	if (m_impacted == newImpacted)
		return;
	m_impacted = newImpacted;
	emit impactedChanged();
}
