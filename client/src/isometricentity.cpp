/*
 * ---- Call of Suli ----
 *
 * isometricentity.cpp
 *
 * Created on: 2024. 03. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEntity
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

#include "isometricentity.h"

IsometricCircleEntity::IsometricCircleEntity(QQuickItem *parent)
	: IsometricObjectCircle(parent)
	, IsometricEntityIface()
{
	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setFixedRotation(true);
}






/**
 * @brief IsometricEntityIface::entityWorldStep
 * @param position
 */

void IsometricEntityIface::entityIfaceWorldStep(const qreal &factor, const QPointF &position, const TiledObject::Directions &availableDirections)
{
	if (qFuzzyCompare(position.x(), m_lastPosition.x()) && qFuzzyCompare(position.y(), m_lastPosition.y())) {
		setMovingDirection(TiledObject::Invalid);
		setMovingSpeed(0.);
		return;
	}

	QLineF line(m_lastPosition, position);
	setMovingDirection(TiledObject::nearestDirectionFromRadian(availableDirections,
															   TiledObject::toRadian(line.angle())));

	setMovingSpeed(line.length() / factor);
	m_lastPosition = position;
}



/**
 * @brief IsometricEntityIface::movingSpeed
 * @return
 */

qreal IsometricEntityIface::movingSpeed() const
{
	return m_movingSpeed;
}

void IsometricEntityIface::setMovingSpeed(qreal newMovingSpeed)
{
	m_movingSpeed = newMovingSpeed;
}



/**
 * @brief IsometricEntityIface::maxHp
 * @return
 */

int IsometricEntityIface::maxHp() const
{
	return m_maxHp;
}

void IsometricEntityIface::setMaxHp(int newMaxHp)
{
	if (m_maxHp == newMaxHp)
		return;
	m_maxHp = newMaxHp;
	emit maxHpChanged();
}



/**
 * @brief IsometricEntityIface::checkEntityVisibility
 * @param body
 * @param entity
 * @return
 */

std::optional<QPointF> IsometricEntityIface::checkEntityVisibility(TiledObjectBody *body, TiledObjectBase *entity,
																   const TiledObjectBody::FixtureCategory &category,
																   float32 *transparentGroundPtr)
{
	Q_ASSERT(body);
	Q_ASSERT(entity);

	const QPointF &entityPosition = entity->body()->bodyPosition();
	/*
	QList<QPointF> points;

	points.append(playerPosition);

	// Get tangents

	TiledObjectCircle *circleObj = dynamic_cast<TiledObjectCircle*>(player);

	if (circleObj) {
		Box2DCircle *fixture = circleObj->fixture();
		const float &radius = fixture->radius();
		const float centerX = playerPosition.x();
		const float centerY = playerPosition.y();

		const float bX = (body->bodyPosition().x() - centerX) / radius;
		const float bY = (body->bodyPosition().y() - centerY) / radius;

		const float xy = bX*bX + bY*bY;


		// point outside of circumfence, one tangent
		if (xy > 1.0) {
			float D = bY * sqrt(xy - 1.);

			float tx0 = (bX - D) / xy;
			float tx1 = (bX + D) / xy;

			float x0, x1, y0, y1;

			if (bY != 0.) {
				y0 = centerY + radius * (1. - tx0 * bX) / bY;
				y1 = centerY + radius * (1. - tx1 * bX) / bY;
			} else {
				D = radius * sqrt(1. - tx0 * tx0);
				y0 = centerY + D;
				y1 = centerY - D;
			}

			x0 = centerX + radius * tx0; //restore scale and position
			x1 = centerX + radius * tx1;

			points.append(QPointF{x0, y0});
			points.append(QPointF{x1, y1});
		}
	}

	for (const QPointF &p : points) {
*/

	float32 rayLength = 0.;
	const TiledReportedFixtureMap &map = body->rayCast(entityPosition, &rayLength);

	bool visible = false;

	for (auto it=map.constBegin(); it != map.constEnd(); ++it) {

		if (it->fixture->isSensor())
			continue;

		if (it->fixture->categories().testFlag(TiledObjectBody::fixtureCategory(category))) {
			visible = true;
			break;
		}

		if (it->fixture->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround))) {
			if (TiledObjectBody *body = qobject_cast<TiledObjectBody*>(it->fixture->getBody())) {
				if (body->opaque()) {
					visible = false;
					break;
				} else if (transparentGroundPtr) {
					*transparentGroundPtr = it.key() * rayLength;
				}
			}
		}
	}

	if (visible)
		return entityPosition /*p*/;
	/*	}*/

	return std::nullopt;
}



/**
 * @brief IsometricEntityIface::checkGroundDistance
 * @param body
 * @param targetPoint
 * @return -1.: no ground
 */

float32 IsometricEntityIface::checkGroundDistance(TiledObjectBody *body, const QPointF &targetPoint, float32 *lengthPtr)
{
	Q_ASSERT(body);

	if (body->baseObject() && body->baseObject()->scene()->isGroundContainsPoint(body->bodyPosition()))
		return 0.;

	float32 dist = -1.;

	const TiledReportedFixtureMap &map = body->rayCast(targetPoint, lengthPtr);

	for (auto it=map.constBegin(); it != map.constEnd(); ++it) {
		if (it->fixture->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround))) {
			if (dist == -1. || it.key() < dist)
				dist = it.key();
		}
	}

	return dist;
}




/**
 * @brief IsometricEntityIface::hp
 * @return
 */

int IsometricEntityIface::hp() const
{
	return m_hp;
}

void IsometricEntityIface::setHp(int newHp)
{
	if (m_hp == newHp)
		return;

	const bool isHurt = (newHp < m_hp);
	const bool isHealed = (newHp > m_hp);
	const bool resurrected = (m_hp <= 0 && newHp > 0);

	m_hp = newHp;
	emit hpChanged();

	if (m_hp <= 0)
		onDead();
	else if (isHurt)
		emit hurt();
	else if (resurrected)
		onAlive();
	else if (isHealed)
		emit healed();

}


/**
 * @brief IsometricEntityIface::movingDirection
 * @return
 */

TiledObject::Direction IsometricEntityIface::movingDirection() const
{
	return m_movingDirection;
}

void IsometricEntityIface::setMovingDirection(const TiledObject::Direction &newMovingDirection)
{
	if (m_movingDirection == newMovingDirection)
		return;
	m_movingDirection = newMovingDirection;
}
