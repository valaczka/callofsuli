/*
 * ---- Call of Suli ----
 *
 * tiledpathmotor.cpp
 *
 * Created on: 2024. 03. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledPathMotor
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

#include "tiledpathmotor.h"
#include <Logger.h>

TiledPathMotor::TiledPathMotor(const QPolygonF &polygon, const Direction &direction)
	: m_polygon(polygon)
	, m_direction(direction)
{
	loadLines();
}

/**
 * @brief TiledPathMotor::polygon
 * @return
 */

QPolygonF TiledPathMotor::polygon() const
{
	return m_polygon;
}

void TiledPathMotor::setPolygon(const QPolygonF &newPolygon)
{
	m_polygon = newPolygon;
	loadLines();
	if (m_direction == Forward)
		toBegin();
	else
		toEnd();
}


/**
 * @brief TiledPathMotor::direction
 * @return
 */

TiledPathMotor::Direction TiledPathMotor::direction() const
{
	return m_direction;
}

void TiledPathMotor::setDirection(Direction newDirection)
{
	m_direction = newDirection;
}


/**
 * @brief TiledPathMotor::currentDistance
 * @return
 */

qreal TiledPathMotor::currentDistance() const
{
	return m_currentDistance;
}

qreal TiledPathMotor::currentAngle() const
{
	return m_currentAngle;
}


/**
 * @brief TiledPathMotor::currentAngleRadian
 * @return
 */

qreal TiledPathMotor::currentAngleRadian() const
{
	if (m_currentAngle <= 180.)
		return m_currentAngle * M_PI / 180.;
	else
		return -(360-m_currentAngle) * M_PI / 180.;
}


/**
 * @brief TiledPathMotor::loadLines
 */

void TiledPathMotor::loadLines()
{
	m_lines.clear();

	m_fullDistance = 0;

	auto prev = m_polygon.constBegin();
	for (auto it = m_polygon.constBegin(); it != m_polygon.constEnd(); ++it) {
		if (it == m_polygon.constBegin())
			continue;

		Line l;
		l.line.setP1(*prev);
		l.line.setP2(*it);
		l.length = l.line.length();
		l.angle = l.line.angle();
		m_fullDistance += l.length;
		m_lines.append(l);

		prev = it;

		LOG_CDEBUG("scene") << "++" << l.line << l.length << l.angle;
	}

	LOG_CINFO("scene") << "LOADED" << m_fullDistance << m_lines.size();
}



/**
 * @brief TiledPathMotor::currentPosition
 * @return
 */

QPointF TiledPathMotor::currentPosition() const
{
	return m_currentPosition;
}


/**
 * @brief TiledPathMotor::fullDistance
 * @return
 */

qreal TiledPathMotor::fullDistance() const
{
	return m_fullDistance;
}




/**
 * @brief TiledPathMotor::toDistance
 * @param distance
 */

bool TiledPathMotor::toDistance(const qreal &distance)
{
	qreal rest = distance;

	for (int i=0; i<m_lines.size(); ++i) {
		const Line &line = m_lines.at(i);
		if (line.length < rest) {
			rest -= line.length;
			continue;
		}

		QLineF cLine = line.line;
		cLine.setLength(rest);

		m_currentPosition = cLine.p2();
		m_currentDistance = distance;

		if (m_direction == Forward)
			m_currentAngle = line.angle;
		else if (line.angle < 180)
			m_currentAngle = 180+line.angle;
		else
			m_currentAngle = line.angle-180;

		return true;
	}

	return false;
}


/**
 * @brief TiledPathMotor::step
 * @param distance
 * @return
 */

bool TiledPathMotor::step(const qreal &distance)
{
	return step(distance, m_direction);
}


/**
 * @brief TiledPathMotor::step
 * @param distance
 * @param direction
 * @return
 */

bool TiledPathMotor::step(const qreal &distance, const Direction &direction)
{
	qreal d = m_currentDistance;

	if (direction == Forward) {
		d += distance;
		if (d > m_fullDistance) {
			if (m_polygon.isClosed())
				d = d-m_fullDistance;
			else
				d = m_fullDistance;
		}
	} else {
		d -= distance;
		if (d < 0.0) {
			if (m_polygon.isClosed())
				d = m_fullDistance - d;
			else
				d = 0.0;
		}
	}

	return toDistance(d);
}



