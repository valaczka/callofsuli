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
#include "tiledobject.h"
#include "isometricentity.h"
#include <Logger.h>

TiledPathMotor::TiledPathMotor(const QPolygonF &polygon, const Direction &direction)
	: AbstractTiledMotor(PathMotor)
	, m_polygon(polygon)
	, m_direction(direction)
{
	loadLines();
}


/**
 * @brief TiledPathMotor::toSerializer
 * @return
 */

TiledPathMotorSerializer TiledPathMotor::toSerializer() const
{
	TiledPathMotorSerializer data;

	data.distance = m_currentDistance;
	data.forward = m_direction == Forward;

	return data;
}


/**
 * @brief TiledPathMotor::fromSerializer
 * @param data
 */

void TiledPathMotor::fromSerializer(const TiledPathMotorSerializer &data)
{
	setDirection(data.forward ? Forward : Backward);
	toDistance(data.distance);
}


/**
 * @brief TiledPathMotor::linesToPolygon
 * @return
 */

QPolygonF TiledPathMotor::linesToPolygon() const
{
	QPolygonF p;

	for (auto it = m_lines.constBegin(); it != m_lines.constEnd(); ++it) {
		if (it == m_lines.constBegin())
			p.append(it->line.p1());

		p.append(it->line.p2());
	}

	return p;
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
	return TiledObject::toRadian(m_currentAngle);
}


/**
 * @brief TiledPathMotor::loadLines
 */

void TiledPathMotor::loadLines()
{
	m_lines.clear();

	m_fullDistance = 0;

	if (m_polygon.isEmpty())
		return;

	auto prev = m_polygon.constBegin();
	for (auto it = m_polygon.constBegin(); it != m_polygon.constEnd(); ++it) {
		if (it == m_polygon.constBegin())
			continue;

		Line l;
		l.line.setP1(*prev);
		l.line.setP2(*it);
		l.length = l.line.length();
		l.angle = l.line.angle();
		l.speed = TiledObject::factorFromDegree(l.angle);
		m_fullDistance += l.length;
		m_lines.append(l);

		prev = it;
	}
}


/**
 * @brief TiledPathMotor::angleFromLine
 * @param line
 * @return
 */

qreal TiledPathMotor::angleFromLine(const Line &line) const
{
	if (m_direction == Forward)
		return line.angle;
	else if (line.angle < 180)
		return 180+line.angle;
	else
		return line.angle-180;
}

qint64 TiledPathMotor::waitAtBegin() const
{
	return m_waitAtBegin;
}

void TiledPathMotor::setWaitAtBegin(qint64 newWaitAtBegin)
{
	m_waitAtBegin = newWaitAtBegin;
}

qint64 TiledPathMotor::waitAtEnd() const
{
	return m_waitAtEnd;
}

void TiledPathMotor::setWaitAtEnd(qint64 newWaitAtEnd)
{
	m_waitAtEnd = newWaitAtEnd;
}


/**
 * @brief TiledPathMotor::currentSegment
 * @return
 */

int TiledPathMotor::currentSegment() const
{
	return m_currentSegment;
}


/**
 * @brief TiledPathMotor::removeAboveSegment
 * @param segment
 * @return
 */

bool TiledPathMotor::clearFromSegment(const int &segment)
{
	if (segment < 0 || segment >= m_lines.size())
		return false;

	m_lines.erase(m_lines.cbegin()+segment, m_lines.cend());

	return true;
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
 * @brief TiledPathMotor::updateBody
 * @param object
 * @param maximumSpeed
 */

void TiledPathMotor::updateBody(TiledObject *object, const qreal &maximumSpeed)
{
	Q_ASSERT(object);
	Q_ASSERT(object->body());

	TiledObjectBody *body = object->body();

	if (!isClosed()) {
		if (m_direction == Backward && atBegin()) {
			const WaitTimerState &s = waitTimerState();

			if (s == Invalid && m_waitAtBegin > 0) {
				waitTimerStart(m_waitAtBegin);
				body->stop();
				return;
			} else if (s == Running) {
				body->stop();
				return;
			} else {
				waitTimerStop();
				setDirection(Forward);
			}
		} else 	if (m_direction == Forward && atEnd()) {
			const WaitTimerState &s = waitTimerState();

			if (s == Invalid && m_waitAtEnd > 0) {
				waitTimerStart(m_waitAtEnd);
				body->stop();
				return;
			} else if (s == Running) {
				body->stop();
				return;
			} else {
				waitTimerStop();
				setDirection(Backward);
			}
		}
	}

	body->setLinearVelocity(IsometricEntityIface::maximizeSpeed(m_currentPosition - body->bodyPosition(), maximumSpeed));
}


/**
 * @brief TiledPathMotor::toBegin
 * @return
 */

bool TiledPathMotor::toBegin()
{
	if (m_lines.isEmpty())
		return false;

	const Line &line = m_lines.first();

	m_currentDistance = 0;
	m_currentPosition = line.line.p1();
	m_currentAngle = angleFromLine(line);
	m_currentSegment = 0;

	return true;
}


/**
 * @brief TiledPathMotor::toEnd
 * @return
 */

bool TiledPathMotor::toEnd()
{
	if (m_lines.isEmpty())
		return false;

	const Line &line = m_lines.last();

	m_currentDistance = m_fullDistance;
	m_currentPosition = line.line.p2();
	m_currentAngle = angleFromLine(line);
	m_currentSegment = m_lines.size()-1;

	return true;
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

		if (i < m_lines.size()-1 && line.length < rest) {
			rest -= line.length;
			continue;
		}

		if (i == m_lines.size()-1 && std::abs(rest-line.length) < 0.00001) {
			m_currentPosition = line.line.p2();
		} else {
			QLineF cLine = line.line;
			cLine.setLength(rest);
			m_currentPosition = cLine.p2();
		}

		m_currentDistance = distance;
		m_currentAngle = angleFromLine(line);

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
	if (m_currentSegment < 0 || m_currentSegment >= m_lines.size() || distance <= 0.)
		return false;

	qreal rest = m_currentDistance;

	for (int i=0; i<m_currentSegment; ++i)
		rest -= m_lines.at(i).length;


	if (direction == Forward) {
		for (int i=m_currentSegment; i<m_lines.size(); ++i) {
			const Line &line = m_lines.at(i);
			const qreal fd = distance * line.speed;

			if (i < m_lines.size()-1 && rest+fd > line.length) {
				rest -= line.length;
				continue;
			}

			if (rest+fd > line.length && m_polygon.isClosed()) {
				rest -= line.length;
				m_currentSegment = 0;
				m_currentDistance = 0;
				i=-1;
				continue;
			}

			if (i == m_lines.size()-1 && rest+fd >= line.length) {
				m_currentPosition = line.line.p2();
				m_currentDistance = m_fullDistance;
			} else {
				QLineF cLine = line.line;
				cLine.setLength(rest+fd);
				m_currentPosition = cLine.p2();
				m_currentDistance += fd;
			}

			m_currentAngle = angleFromLine(line);
			m_currentSegment = i;

			return true;
		}
	} else {
		qreal backDistance = 0;

		for (int i=m_currentSegment; i>=0 && backDistance < distance; --i) {
			const Line &line = m_lines.at(i);
			const qreal fd = distance * line.speed;

			if (rest < 0)
				rest = line.length;

			if (i > 0 && rest-fd < 0) {
				backDistance += fd-rest;
				rest = -1;
				continue;
			}

			if (rest-fd < 0 && m_polygon.isClosed()) {
				backDistance += fd-rest;
				rest = -1;
				m_currentSegment = m_lines.size()-1;
				m_currentDistance = m_fullDistance;
				i=m_lines.size();
				continue;
			}

			if (i == 0 && rest-fd < 0) {
				m_currentPosition = line.line.p1();
				m_currentDistance = 0;
				/*backDistance = distance;
				rest = -1;*/
			} else {
				QLineF cLine = line.line;
				cLine.setLength(rest-fd);
				m_currentPosition = cLine.p2();
				m_currentDistance -= fd;
				/*backDistance += fd;
				rest = -1;*/
			}

			m_currentAngle = angleFromLine(line);
			m_currentSegment = i;

			return true;
		}
	}

	return false;
}



/**
 * @brief TiledPathMotor::waitTimerStart
 * @param msec
 */

void TiledPathMotor::waitTimerStart(const qint64 &msec)
{
	m_waitForMsec = msec;
	if (msec > 0)
		m_waitTimer.start();
	else
		m_waitTimer.invalidate();
}


/**
 * @brief TiledPathMotor::waitTimerStop
 */

void TiledPathMotor::waitTimerStop()
{
	m_waitForMsec = 0;
	m_waitTimer.invalidate();
}



/**
 * @brief TiledPathMotor::waitTimerState
 * @return
 */

TiledPathMotor::WaitTimerState TiledPathMotor::waitTimerState() const
{
	if (m_waitForMsec == 0 || !m_waitTimer.isValid())
		return Invalid;
	else if (m_waitTimer.elapsed() < m_waitForMsec)
		return Running;
	else
		return Overdue;
}



