/*
 * ---- Call of Suli ----
 *
 * tiledreturnpathmotor.cpp
 *
 * Created on: 2024. 03. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledReturnPathMotor
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

#include "tiledreturnpathmotor.h"

TiledReturnPathMotor::TiledReturnPathMotor()
	: AbstractTiledMotor(ReturnPathMotor)
{

}


/**
 * @brief TiledReturnPathMotor::toSerializer
 * @return
 */

TiledReturnPathMotorSerializer TiledReturnPathMotor::toSerializer() const
{
	TiledReturnPathMotorSerializer data;

	data.points.reserve(m_path.size()*2);
	for (const QPointF &p : std::as_const(m_path)) {
		data.points << p.x() << p.y();
	}

	data.returning = m_isReturning;
	data.distance = m_pathMotor.currentDistance();

	return data;
}



/**
 * @brief TiledReturnPathMotor::fromSerializer
 * @param data
 * @return
 */

TiledReturnPathMotor *TiledReturnPathMotor::fromSerializer(const TiledReturnPathMotorSerializer &data)
{
	TiledReturnPathMotor *motor = new TiledReturnPathMotor;

	QPolygonF polygon;

	for (auto it = data.points.constBegin(); it != data.points.constEnd(); ++it) {
		qreal x = *it;
		++it;
		if (it != data.points.constEnd()) {
			polygon << QPointF{x, *it};
		}
	}

	motor->m_pathMotor.setPolygon(polygon);
	motor->m_pathMotor.setDirection(TiledPathMotor::Backward);
	motor->m_pathMotor.toDistance(data.distance);
	motor->m_isReturning = data.returning;

	return motor;
}


/**
 * @brief TiledReturnPathMotor::currentPosition
 * @return
 */

QPointF TiledReturnPathMotor::currentPosition() const
{
	if (!m_isReturning)
		return {0,0};

	return m_pathMotor.currentPosition();
}





/**
 * @brief TiledReturnPathMotor::moveBody
 * @param body
 * @param angle
 * @param radius
 */

void TiledReturnPathMotor::moveBody(TiledObjectBody *body, const float32 &angle, const qreal &radius)
{
	Q_ASSERT(body);

	if (m_isReturning) {
		if (m_pathMotor.currentSegment() >= 0)
			m_pathMotor.clearFromSegment(m_pathMotor.currentSegment());
		m_path = m_pathMotor.linesToPolygon();

		m_isReturning = false;
		m_waitTimer.invalidate();
	}

	const QPointF &currentPoint = body->bodyPosition();

	body->setLinearVelocity(TiledObjectBase::toPoint(angle, radius));

	if (!m_path.isEmpty() && m_lastAngle == angle)
		return;

	addPoint(currentPoint, angle);
}


/**
 * @brief TiledReturnPathMotor::placeCurrentPosition
 * @param body
 */

void TiledReturnPathMotor::placeCurrentPosition(TiledObjectBody *body, const float32 &angle)
{
	Q_ASSERT(body);

	if (m_isReturning) {
		if (m_pathMotor.currentSegment() >= 0)
			m_pathMotor.clearFromSegment(m_pathMotor.currentSegment());
		m_path = m_pathMotor.linesToPolygon();

		m_isReturning = false;
		m_waitTimer.invalidate();
	}

	const QPointF &currentPoint = body->bodyPosition();

	addPoint(currentPoint, angle);
}








/**
 * @brief TiledReturnPathMotor::addPoint
 * @param point
 * @param angle
 */
void TiledReturnPathMotor::addPoint(const QPointF &point, const float32 &angle)
{
	if (!m_path.isEmpty()) {
		QVector2D vector(m_path.last()-point);
		if (vector.length() < 50.)
			return;
	}


	// Check intersections

	if (m_path.size() > 1) {
		QLineF line(m_path.last(), point);

		auto prev = m_path.begin();
		for (auto it = m_path.begin(); it != m_path.end(); ++it) {
			if (it == m_path.begin())
				continue;

			QLineF l(*prev, *it);

			QPointF isp;

			if (l.intersects(line, &isp) == QLineF::BoundedIntersection) {
				while (it != m_path.end())
					it = m_path.erase(it);

				m_path.append(isp);
				break;
			}

			prev = it;
		}
	}

	m_path << point;
	m_lastAngle = angle;
}







/**
 * @brief TiledReturnPathMotor::finish
 * @param body
 */

void TiledReturnPathMotor::finish(TiledObjectBody *body)
{
	Q_ASSERT(body);

	const QPointF &currentPoint = body->bodyPosition();

	if (!m_path.isEmpty()) {
		QVector2D vector(m_path.last()-currentPoint);

		if (vector.length() >= 15.)
			m_path << currentPoint;

		m_pathMotor.setDirection(TiledPathMotor::Backward);
		m_pathMotor.setPolygon(m_path);
		m_waitTimer.restart();
	}

	m_isReturning = true;
	clearLastSeenPoint();
}



/**
 * @brief TiledReturnPathMotor::step
 * @param body
 * @param distance
 * @return
 */

bool TiledReturnPathMotor::step(const qreal &distance)
{
	if (!m_isReturning) {
		LOG_CERROR("scene") << "Not in returning state";
		return false;
	}

	if (!isReturnReady()) {
		LOG_CWARNING("scene") << "Return not ready";
		return false;
	}

	if (m_pathMotor.atBegin())
		return false;

	m_pathMotor.step(distance);
	return true;
}



QPolygonF TiledReturnPathMotor::path() const
{
	return m_path;
}

bool TiledReturnPathMotor::isReturning() const
{
	return m_isReturning;
}

void TiledReturnPathMotor::setIsReturning(bool newIsReturning)
{
	m_isReturning = newIsReturning;
}

qreal TiledReturnPathMotor::waitMsec() const
{
	return m_waitMsec;
}

void TiledReturnPathMotor::setWaitMsec(qreal newWaitMsec)
{
	m_waitMsec = newWaitMsec;
}

bool TiledReturnPathMotor::isReturnReady() const
{
	return m_waitTimer.isValid() && m_waitTimer.elapsed() >= m_waitMsec;
}

const std::optional<QPointF> &TiledReturnPathMotor::lastSeenPoint() const
{
	return m_lastSeenPoint;
}

void TiledReturnPathMotor::setLastSeenPoint(const QPointF &newLastSeenPoint)
{
	m_lastSeenPoint = newLastSeenPoint;
}

void TiledReturnPathMotor::clearLastSeenPoint()
{
	m_lastSeenPoint = std::nullopt;
}

