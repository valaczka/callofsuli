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
#include <Logger.h>

TiledPathMotor::TiledPathMotor(const QPolygonF &polygon, const Direction &direction)
	: AbstractTiledMotor(PathMotor)
	, m_polygon(polygon)
	, m_direction(direction)
{
	loadLines();
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

	if (m_polygon.isEmpty())
		LOG_CERROR("scene") << "Empty path";

	loadLines();
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
 * @brief TiledPathMotor::loadLines
 */

void TiledPathMotor::loadLines()
{
	m_lines.clear();

	auto prev = m_polygon.constBegin();
	for (auto it = m_polygon.constBegin(); it != m_polygon.constEnd(); ++it) {
		if (it == m_polygon.constBegin())
			continue;

		Line l;
		l.line.setP1(*prev);
		l.line.setP2(*it);
		l.angle = l.line.angle();
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

float TiledPathMotor::lastSegmentFactor() const
{
	return m_lastSegmentFactor;
}

int TiledPathMotor::lastSegment() const
{
	return m_lastSegment;
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
 * @brief TiledPathMotor::removeAboveSegment
 * @param segment
 * @return
 */

bool TiledPathMotor::clearFromSegment(const int &segment)
{
	if (segment < 0 || segment >= m_lines.size())
		return false;

	m_lines.erase(m_lines.begin()+segment, m_lines.end());

	return true;
}


/**
 * @brief TiledPathMotor::clearToSegment
 * @param segment
 * @return
 */

bool TiledPathMotor::clearToSegment(const int &segment)
{
	if (segment < 0 || segment >= m_lines.size())
		return false;

	m_lines.erase(m_lines.begin(), m_lines.begin()+segment);

	return true;
}





/**
 * @brief TiledPathMotor::getShortestPoint
 * @param pos
 */

cpVect TiledPathMotor::getShortestPoint(const cpVect &pos, float *dstDistance, int *dstSegment, float *dstFactor)
{
	float distance = -1;
	cpVect vector = cpvzero;
	int segment = -1;
	int n = 0;
	float factor = -1;

	for (const Line &l : m_lines) {
		cpVect dest = cpvzero;
		float f = -1.;

		float d = TiledObject::shortestDistance(pos, l.line, &dest, &f);

		if (distance == -1 || d < distance) {
			distance = d;
			vector = dest;
			segment = n;
			factor = f;
		}

		++n;
	}

	if (dstDistance)
		*dstDistance = distance;

	if (dstSegment)
		*dstSegment = segment;

	if (dstFactor)
		*dstFactor = factor;

	return vector;
}


/**
 * @brief TiledPathMotor::getLastSegmentPoint
 * @return
 */

std::optional<cpVect> TiledPathMotor::getLastSegmentPoint()
{
	if (m_lastSegment < 0 || m_lastSegment >= m_lines.size())
		return std::nullopt;

	return TiledObject::toVect(m_lines.at(m_lastSegment).line.pointAt(m_lastSegmentFactor));
}




/**
 * @brief TiledPathMotor::updateBody
 * @param body
 * @param maximumSpeed
 */

void TiledPathMotor::updateBody(TiledObject *body, const float &speed, AbstractGame::TickTimer *timer)
{
	Q_ASSERT(body);

	const float threshold = 2*speed/60.;			// 60 fps-nél 2 frame alatt megtett távolság a küszöb

	if (m_lastSegment >= 0 && m_lastSegment < m_lines.size()) {
		float factor = -1.;
		float d = TiledObject::shortestDistance(body->bodyPosition(), m_lines.at(m_lastSegment).line, nullptr, &factor);

		if (d > threshold) {
			m_lastSegment = -1;
			m_lastSegmentFactor = -1.;
		} else {
			m_lastSegmentFactor = factor;
		}
	}

	const auto lastPoint = getLastSegmentPoint();

	if (!lastPoint || body->distanceToPointSq(lastPoint.value()) > POW2(threshold)) {
		int dstSegment = -1;
		float dstDistance = -1;
		float dstFactor = -1.;
		cpVect dst = getShortestPoint(body->bodyPosition(), &dstDistance, &dstSegment, &dstFactor);

		if (dstSegment < 0) {
			LOG_CERROR("scene") << "Invalid line segment" << body << m_lines.size();
			return;
		}

		if (dstDistance < threshold) {
			m_lastSegment = dstSegment;
			m_lastSegmentFactor = std::max(dstFactor, (float) 0.);
		}

		body->moveTowards(dst, speed);
		return;
	}


	// Ha nincs zárva és a végén vagyunk

	if (!isClosed()) {
		const WaitTimerState s = waitTimerState(timer);

		if (m_direction == Backward && atBegin()) {
			if (s == Invalid && m_waitAtBegin > 0) {
				m_waitTimerEnd = timer->tickAddMsec(m_waitAtBegin);
				body->stop();
				return;
			} else if (s == Running) {
				body->stop();
				return;
			} else {
				m_waitTimerEnd = 0;
				setDirection(Forward);
			}
		} else if (m_direction == Forward && atEnd()) {
			if (s == Invalid && m_waitAtEnd > 0) {
				m_waitTimerEnd = timer->tickAddMsec(m_waitAtBegin);
				body->stop();
				return;
			} else if (s == Running) {
				body->stop();
				return;
			} else {
				m_waitTimerEnd = 0;
				setDirection(Backward);
			}

			setDirection(Backward);
		}
	}



	// Near of the lastSegmentPoint

	const QLineF &line = m_lines.at(m_lastSegment).line;
	const float delta = (speed/60.)/line.length();

	if (m_direction == Forward) {
		if (m_lastSegmentFactor + delta > 1.) {
			if (m_lastSegment+1 >= m_lines.size() && !isClosed()) {
				m_lastSegmentFactor = 1.0;
			} else {
				if (m_lastSegment+1 >= m_lines.size() && isClosed())
					m_lastSegment = 0;
				else
					++m_lastSegment;

				m_lastSegmentFactor = 0.;
			}
		} else {
			m_lastSegmentFactor += delta;
		}

		body->moveTowards(TiledObject::toVect(line.p2()), speed);
	} else if (m_direction == Backward) {
		if (m_lastSegmentFactor - delta < 0.) {
			if (m_lastSegment-1 < 0 && !isClosed()) {
				m_lastSegmentFactor = 0.;
			} else {
				if (m_lastSegment-1 < 0 && isClosed())
					m_lastSegment = m_lines.size()-1;
				else
					--m_lastSegment;

				m_lastSegmentFactor = 1.0;
			}
		} else {
			m_lastSegmentFactor -= delta;
		}

		body->moveTowards(TiledObject::toVect(line.p1()), speed);
	}

}


/**
 * @brief TiledPathMotor::basePoint
 * @return
 */

cpVect TiledPathMotor::basePoint()
{
	if (m_polygon.isEmpty())
		return {};
	else
		return TiledObject::toVect(m_polygon.first());
}





/**
 * @brief TiledPathMotor::atBegin
 * @return
 */

bool TiledPathMotor::atBegin() const
{
	return m_lastSegment == 0 && m_lastSegmentFactor < 0.0001;
}


/**
 * @brief TiledPathMotor::atEnd
 * @return
 */

bool TiledPathMotor::atEnd() const
{
	return m_lastSegment >= m_lines.size()-1 && m_lastSegmentFactor >= 1.0;
}


/**
 * @brief TiledPathMotor::waitTimerState
 * @param timer
 * @return
 */

TiledPathMotor::WaitTimerState TiledPathMotor::waitTimerState(AbstractGame::TickTimer *timer) const
{
	if (!timer)
		return Invalid;

	if (m_waitTimerEnd <= 0)
		return Invalid;
	else if (timer->currentTick() > m_waitTimerEnd)
		return Overdue;
	else
		return Running;
}


/**
 * @brief TiledPathMotor::getShortestSegment
 * @param polygon
 * @param pos
 * @return
 */

int TiledPathMotor::getShortestSegment(const QPolygonF &polygon, const cpVect &pos)
{
	float distance = -1;
	int segment = -1;
	int n = 0;

	auto prev = polygon.constBegin();
	for (auto it = polygon.constBegin(); it != polygon.constEnd(); ++it) {
		if (it == polygon.constBegin())
			continue;

		float f = -1.;

		float d = TiledObject::shortestDistance(pos, *prev, *it, nullptr, &f);

		if (distance == -1 || d < distance) {
			distance = d;
			segment = n;
		}

		prev = it;
		++n;
	}

	return segment;
}




/**
 * @brief TiledPathMotor::clearFromSegment
 * @param polygon
 * @param segment
 * @return
 */

bool TiledPathMotor::clearFromSegment(QPolygonF *polygon, const int &segment)
{
	Q_ASSERT(polygon);

	if (segment < 0 || segment >= polygon->size())
		return false;

	polygon->erase(polygon->begin()+segment, polygon->end());

	return true;
}


/**
 * @brief TiledPathMotor::clearToSegment
 * @param polygon
 * @param segment
 * @return
 */

bool TiledPathMotor::clearToSegment(QPolygonF *polygon, const int &segment)
{
	Q_ASSERT(polygon);

	if (segment < 0 || segment >= polygon->size())
		return false;

	polygon->erase(polygon->begin(), polygon->begin()+segment);

	return true;
}



