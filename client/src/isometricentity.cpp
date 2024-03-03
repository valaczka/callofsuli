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
#include "box2dworld.h"

IsometricCircleEntity::IsometricCircleEntity(QQuickItem *parent)
	: IsometricObjectCircle(parent)
	, IsometricEntityIface()
{
	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setFixedRotation(true);
}


/**
 * @brief IsometricEntityIface::normalize
 * @param radian
 * @return
 */

qreal IsometricEntityIface::normalize(const qreal &radian) const
{
	if (radian < -M_PI || radian > M_PI) {
		LOG_CERROR("scene") << "Invalid radian:" << radian;
		return 0.;
	}

	if (radian < 0)
		return M_PI+M_PI+radian;
	else
		return radian;
}

/**
 * @brief IsometricEntityIface::unnormalize
 * @param normal
 * @return
 */

qreal IsometricEntityIface::unnormalize(const qreal &normal) const
{
	if (normal < 0 || normal > M_PI+M_PI) {
		LOG_CERROR("scene") << "Invalid normal:" << normal;
		return 0.;
	}

	if (normal > M_PI)
		return normal-M_PI-M_PI;
	else
		return normal;
}



/**
 * @brief IsometricCircleEntity::rotateBody
 */

void IsometricCircleEntity::rotateBody()
{
	if (!m_sensorPolygon)
		return;

	const qreal desiredAngle = directionToRadian(m_currentDirection);
	const qreal currentAngle = m_body->body()->GetAngle();

	if (qFuzzyCompare(desiredAngle, currentAngle)) {
		if (m_rotateAnimation.running)
			m_rotateAnimation.running = false;
		return;
	}

	const qreal currentNormal = normalize(currentAngle);
	const qreal desiredNormal = normalize(desiredAngle);

	if (!qFuzzyCompare(m_rotateAnimation.destAngle, desiredAngle) || !m_rotateAnimation.running) {
		m_rotateAnimation.destAngle = desiredAngle;

		const qreal diff = std::abs(currentNormal-desiredNormal);

		if (desiredNormal > currentNormal)
			m_rotateAnimation.clockwise = diff > M_PI;
		else
			m_rotateAnimation.clockwise = diff < M_PI;

		m_rotateAnimation.running = true;
	}


	if (!m_rotateAnimation.running)
		return;


	const qreal delta = std::min(0.1 * m_body->world()->timeStep()*60., M_PI_4);
	qreal newAngle = m_rotateAnimation.clockwise ? currentNormal - delta : currentNormal + delta;

	m_body->setAngularVelocity(0);

	// 0 átlépés miatt kell
	const qreal d = (desiredNormal == 0 && newAngle > M_PI+M_PI) ? M_PI+M_PI : desiredNormal;


	if ((newAngle > d && currentNormal < d) ||
			(newAngle < d && currentNormal > d)) {
		//////m_sensorPolygon->setRotation(desiredAngle);
		m_body->body()->SetTransform(m_body->body()->GetPosition(), desiredAngle );
		return;
	}

		if (newAngle > M_PI+M_PI)
			newAngle -= M_PI+M_PI;
		else if (newAngle < 0)
			newAngle += M_PI+M_PI;

		////m_sensorPolygon->setRotation(unnormalize(newAngle));
		m_body->body()->SetTransform(m_body->body()->GetPosition(), unnormalize(newAngle) );

}


/**
 * @brief IsometricEntityIface::entityWorldStep
 * @param position
 */

void IsometricEntityIface::entityIfaceWorldStep(const QPointF &position, const IsometricObjectIface::Directions &availableDirections)
{
	if (qFuzzyCompare(position.x(), m_lastPosition.x()) && qFuzzyCompare(position.y(), m_lastPosition.y()))
			return setMovingDirection(IsometricObjectIface::Invalid);

	QLineF line(m_lastPosition, position);
		setMovingDirection(IsometricObjectIface::nearestDirectionFromRadian(availableDirections,
																			IsometricObjectIface::toRadian(line.angle())));

	m_lastPosition = position;
}


/**
 * @brief IsometricEntityIface::movingDirection
 * @return
 */

IsometricObjectIface::Direction IsometricEntityIface::movingDirection() const
{
	return m_movingDirection;
}

void IsometricEntityIface::setMovingDirection(const IsometricObjectIface::Direction &newMovingDirection)
{
	if (m_movingDirection == newMovingDirection)
		return;
	m_movingDirection = newMovingDirection;
}
