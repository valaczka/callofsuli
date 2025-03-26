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

IsometricBullet::IsometricBullet(TiledScene *scene)
	: IsometricObject(scene)
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

void IsometricBullet::initialize(TiledWeapon *weapon)
{
	setDefaultZ(1);
	setSubZ(0.8);

	m_speed = 300.;

	load();

	if (m_visualItem)
		m_visualItem->setZ(1);

	setFromWeapon(weapon);
}




/**
 * @brief IsometricBullet::shot
 * @param from
 * @param angle
 */

void IsometricBullet::shot(const QPointF &from, const qreal &angle)
{
	if (!scene())
		return;

	m_startPoint = QVector2D(from);
	emplace(from);
	setFacingDirection(nearestDirectionFromRadian(angle));
	body().SetTransform(body().GetPosition(), b2MakeRot(angle));
	body().SetAwake(true);
	setSpeedFromAngle(angle, m_speed);
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

void IsometricBullet::worldStep()
{
	if (m_facingDirection == Invalid) {
		stop();
		return;
	}

	const qreal &distance = distanceToPoint(m_startPoint);

	if (distance >= m_maxDistance) {
		overshootEvent();
		doAutoDelete();
		return;
	}
}


/**
 * @brief IsometricBullet::synchronize
 */

void IsometricBullet::synchronize()
{
	jumpToSprite("default", m_facingDirection);
	IsometricObject::synchronize();
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



/**
 * @brief IsometricBullet::doAutoDelete
 */

void IsometricBullet::doAutoDelete()
{
	if (m_visualItem)
		m_visualItem->setVisible(false);
	stop();
	setBodyEnabled(false);
	emit autoDeleteRequest(this);
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
 * @brief IsometricBullet::onShapeContactBegin
 * @param self
 * @param other
 */

void IsometricBullet::onShapeContactBegin(b2::ShapeRef, b2::ShapeRef other)
{
	if (m_impacted) {
		stop();
		return;
	}


	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	IsometricEnemy *enemy = categories.testFlag(FixtureTarget) || categories.testFlag(FixtureEnemyBody) ?
								dynamic_cast<IsometricEnemy*>(base) :
								nullptr;

	IsometricPlayer *player = categories.testFlag(FixtureTarget) || categories.testFlag(FixturePlayerBody)  ?
								  dynamic_cast<IsometricPlayer*>(base) :
								  nullptr;



	if (categories.testFlag(TiledObjectBody::FixtureGround) && base->opaque()) {
		setImpacted(true);
		stop();
		groundEvent(base);
		doAutoDelete();
		return;
	}


	bool hasTarget = false;

	if (m_targets.testFlag(TargetEnemy) && enemy) {
		hasTarget = enemy->canBulletImpact(d->m_fromWeaponType);
	}

	if (m_targets.testFlag(TargetPlayer) && player && !player->isLocked()) {
		hasTarget = true;
	}

	if (!hasTarget)
		return;

	impactEvent(base);
}



/**
 * @brief IsometricBullet::impactEvent
 * @param base
 */

void IsometricBullet::impactEvent(TiledObjectBody *base)
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


	IsometricEnemy *enemy = dynamic_cast<IsometricEnemy*>(base);
	IsometricPlayer *player = dynamic_cast<IsometricPlayer*>(base);

	LOG_CINFO("game") << "IMPACT" << d->m_owner << enemy << player << d->m_fromWeaponType;

	if (enemy)
		game->playerAttackEnemy(d->m_owner, enemy, d->m_fromWeaponType);

	if (player)
		game->enemyAttackPlayer(d->m_owner, player, d->m_fromWeaponType);

	/// TODO: player attack player?

	setImpacted(true);
	stop();
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
