/*
 * ---- Call of Suli ----
 *
 * rpgentity.cpp
 *
 * Created on: 2026. 05. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEntity
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

#include "rpgentity.h"



RpgEntity::RpgEntity(RpgGameItem *gameItem, const cpVect &center, const qreal &radius, const cpBodyType &type)
	: RpgObject(gameItem, center, radius, type)
{

}

int RpgEntity::hp() const
{
	return m_hp;
}

void RpgEntity::setHp(int newHp)
{
	if (m_hp == newHp)
		return;

	bool isHurt = newHp < m_hp;
	bool isHealed = newHp > m_hp;
	bool isResurrected = (isHealed && m_hp <= 0);

	m_hp = newHp;
	emit hpChanged();

	if (isHurt)
		emit hurt();

	if (isHealed)
		emit healed();

	if (m_hp == 0) {
		emit becameDead();
		onDead();
	}

	if (isResurrected) {
		emit becameAlive();
		onAlive();
	}


}

int RpgEntity::maxHp() const
{
	return m_maxHp;
}

void RpgEntity::setMaxHp(int newMaxHp)
{
	if (m_maxHp == newMaxHp)
		return;
	m_maxHp = newMaxHp;
	emit maxHpChanged();
}




/**
 * @brief RpgMotorEntity::RpgMotorEntity
 * @param entity
 */

RpgMotorEntity::RpgMotorEntity(RpgEntity *entity)
	: AbstractRpgMotor(entity)
	, m_entity(entity)
{

}



/**
 * @brief RpgMotorEntity::queryContactedBodies
 * @param shape
 */

void RpgMotorEntity::queryContactedBodies(cpSpace *space, cpShape *shape, QSet<TiledObjectBody *> *dst, const cpBitmask &categories)
{
	Q_ASSERT(space);
	Q_ASSERT(shape);
	Q_ASSERT(dst);

	if (cpSpaceIsLocked(space)) {
		LOG_CERROR("game") << "Locked space";
		return;
	}

	struct _d {
		cpBitmask mask;
		QSet<TiledObjectBody *> *bodies = nullptr;
	};

	_d d;
	d.mask = categories;
	d.bodies = dst;

	static const auto fn = [](cpShape *shape, cpContactPointSet *, void *data) {
		TiledObjectBody *body = TiledObjectBody::fromShapeRef(shape);

		if (!body) {
			LOG_CERROR("game") << "Invalid body";
			return;
		}

		_d *d = (_d*)(data);

		if (cpShapeGetFilter(shape).categories & d->mask) {
			d->bodies->insert(body);
		}
	};

	cpSpaceShapeQuery(space, shape, fn, &d);
}


/**
 * @brief RpgMotorEntity::queryContactedBodies
 * @param body
 * @param dst
 * @param categories
 * @param flags
 */

void RpgMotorEntity::queryContactedBodies(TiledObjectBody *body, QSet<TiledObjectBody *> *dst, const cpBitmask &categories, const QueryFlags &flags)
{
	Q_ASSERT(body);
	Q_ASSERT(dst);

	if (flags == QueryNone)
		return;

	if (flags.testFlag(QueryBody)) {
		for (cpShape *sh : body->bodyShapes())
			queryContactedBodies(body->space(), sh, dst, categories);
	}

	if (flags.testFlag(QuerySensorPolygon) && body->sensorPolygon())
		queryContactedBodies(body->space(), body->sensorPolygon(), dst, categories);

	if (flags.testFlag(QueryTarget) && body->targetCircle())
		queryContactedBodies(body->space(), body->targetCircle(), dst, categories);

	if (flags.testFlag(QueryVirtualCircle) && body->virtualCircle())
		queryContactedBodies(body->space(), body->virtualCircle(), dst, categories);
}



/**
 * @brief RpgMotorEntity::queryContactedVisibleBodies
 * @param body
 * @param dst
 * @param categories
 * @param ground
 * @param flags
 */

void RpgMotorEntity::queryContactedVisibleBodies(TiledObjectBody *body, QSet<TiledObjectBody *> *dst, const cpBitmask &categories,
												 const cpBitmask &ground, const float &maxDist, const QueryFlags &flags)
{
	Q_ASSERT(body);
	Q_ASSERT(dst);

	if (flags == QueryNone)
		return;

	queryContactedBodies(body, dst, categories, flags);

	const QRectF r = body->bodyAABB();

	for (auto it = dst->cbegin(); it != dst->cend(); ) {
		TiledObjectBody *b = *it;
		RayCastInfo info = body->rayCast(b->bodyPosition(), ground);

		// Ha fedik egymást, akkor a raycast nem fogja látni

		if (r.contains(TiledObjectBody::toPointF(b->bodyPosition()))) {
			++it;
			continue;
		}

		if (b == body || !info.isVisible(b) ||
				(maxDist > 0 && body->distanceToPointSq(b->bodyPosition()) > maxDist*maxDist)) {
			it = dst->erase(it);
			continue;
		}

		++it;
	}
}



/**
 * @brief RpgMotorEntity::getNearest
 * @param body
 * @param dst
 * @return
 */

TiledObjectBody *RpgMotorEntity::getNearest(TiledObjectBody *body, const QSet<TiledObjectBody *> &dst)
{
	Q_ASSERT(body);

	TiledObjectBody *ret = nullptr;
	float dist = 0;

	for (TiledObjectBody *b : dst) {
		Q_ASSERT(b);

		if (!ret) {
			ret = b;
			dist = body->distanceToPointSq(b->bodyPosition());
			continue;
		} else if (float d = body->distanceToPointSq(b->bodyPosition()); d < dist) {
			ret = b;
			dist = d;
		}
	}

	return ret;
}



/**
 * @brief RpgMotorEntity::sort
 * @param body
 * @param dst
 * @return
 */

QMultiMap<float, TiledObjectBody *> RpgMotorEntity::sort(TiledObjectBody *body, const QSet<TiledObjectBody *> &dst)
{
	QMultiMap<float, TiledObjectBody *> ret;

	for (TiledObjectBody *b : dst) {
		Q_ASSERT(b);
		float dist = body->distanceToPointSq(b->bodyPosition());
		ret.insert(dist, b);
	}

	return ret;
}





/**
 * @brief RpgMotorEntity::findNearestTarget
 * @param category
 * @return
 */

RpgEntity *RpgMotorEntity::findNearestTarget(const cpBitmask &category)
{
	QSet<TiledObjectBody*> list;

	RpgMotorEntity::queryContactedVisibleBodies(m_entity, &list, category, RpgGameItem::FixtureGround);

	QMultiMap<float, TiledObjectBody *> bds = RpgMotorEntity::sort(m_entity, list);

	for (TiledObjectBody *b : std::as_const(bds)) {
		RpgEntity *e = dynamic_cast<RpgEntity*>(b);

		if (!e || !e->isAlive())
			continue;

		if (e->team() == m_entity->team())
			continue;

		return e;
	}

	return nullptr;
}




/**
 * @brief RpgMotorEntity::findNearestTarget
 * @param rayDest
 * @param category
 * @return
 */

RpgEntity *RpgMotorEntity::findNearestTarget(const cpVect &rayDest, const cpBitmask &category)
{
	RayCastInfo ray = m_entity->rayCast(rayDest, RpgGameItem::FixtureGround, category, 2.);

	for (const RayCastInfoItem &i : ray) {
		if (!i.visible)
			continue;

		RpgEntity *e = dynamic_cast<RpgEntity*>(TiledObjectBody::fromShapeRef(i.shape));

		if (!e || !e->isAlive())
			continue;

		if (e->team() == m_entity->team())
			continue;

		return e;
	}

	return nullptr;
}



/**
 * @brief RpgDestinationMotor::RpgDestinationMotor
 * @param entity
 */

RpgDestinationMotor::RpgDestinationMotor(RpgEntity *entity)
	: RpgMotorEntity(entity)
{

}


/**
 * @brief RpgDestinationMotor::setDestination
 * @param polygon
 */

void RpgDestinationMotor::setDestination(const QPolygonF &polygon)
{
	if (polygon.size() == 1)
		return setDestination(TiledObjectBody::toVect(polygon.first()));

	m_destinationMotor.reset(new TiledPathMotor(m_gameItem->tickTimer(), polygon));
	m_destinationPoint = std::nullopt;
}


/**
 * @brief RpgDestinationMotor::setDestination
 * @param point
 */

void RpgDestinationMotor::setDestination(const cpVect &point)
{
	m_destinationMotor.reset();
	m_destinationPoint = point;
}


/**
 * @brief RpgDestinationMotor::clearDestination
 */

void RpgDestinationMotor::clearDestination()
{
	m_destinationMotor.reset();
	m_destinationPoint = std::nullopt;
}


/**
 * @brief RpgDestinationMotor::destination
 * @return
 */

std::optional<QPolygonF> RpgDestinationMotor::destination() const
{
	if (m_destinationMotor)
		return m_destinationMotor->polygon();
	else if (m_destinationPoint) {
		QPolygonF p;
		p << m_entity->bodyPositionF();
		p << TiledObjectBody::toPointF(m_destinationPoint.value());
		return p;
	}

	return std::nullopt;
}



/**
 * @brief RpgEntity::team
 * @return
 */

RpgStream::Team RpgEntity::team() const
{
	return m_team;
}

void RpgEntity::setTeam(RpgStream::Team newTeam)
{
	m_team = newTeam;

	updateColor();
}


bool RpgEntity::locked() const
{
	return m_locked;
}

void RpgEntity::setLocked(bool newLocked)
{
	if (m_locked == newLocked)
		return;
	m_locked = newLocked;
	emit lockedChanged();
}
