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
#include "tiledscene.h"
#include "tiledgame.h"

TiledReturnPathMotor::TiledReturnPathMotor(const cpVect &basePoint)
	: AbstractTiledMotor(ReturnPathMotor)
	, m_basePoint(basePoint)
{

}



/**
 * @brief TiledReturnPathMotor::updateBody
 * @param object
 * @param speed
 * @param timer
 */

void TiledReturnPathMotor::updateBody(TiledObject *object, const float &speed, AbstractGame::TickTimer *timer)
{
	Q_ASSERT (object);

	if (!m_isReturning)
		return;

	if (m_pathMotor) {
		if (m_pathMotor->atEnd(object)) {
			object->stop();
			m_pathMotor.reset();
			m_hasReturned = true;
			m_path.clear();
			setIsReturning(false);
		} else {
			m_pathMotor->updateBody(object, speed, timer);
		}
	} else {
		setIsReturning(false);
	}
}




/**
 * @brief TiledReturnPathMotor::basePoint
 * @return
 */

cpVect TiledReturnPathMotor::basePoint()
{
	return m_basePoint;
}







/**
 * @brief TiledReturnPathMotor::moveBody
 * @param body
 * @param angle
 * @param radius
 */

void TiledReturnPathMotor::moveBody(TiledObject *body, const cpVect &point, const float &speed)
{
	Q_ASSERT(body);

	const cpVect current = body->bodyPosition();

	if (m_isReturning) {
		m_isReturning = false;
		m_waitEnd = 0;

		if (m_path.size() > 1) {
			const int last = TiledPathMotor::getShortestSegment(m_path, current);

			if (last != -1)
				TiledPathMotor::clearFromSegment(&m_path, last+1);
		}

		if (m_pathMotor)
			m_pathMotor.reset();
	}

	m_hasReturned = false;

	const auto &angle = body->angleToPoint(point);
	body->moveTowards(point, speed);

	addPoint(current, angle);
}





/**
 * @brief TiledReturnPathMotor::record
 * @param body
 */

void TiledReturnPathMotor::record(TiledObject *body)
{
	Q_ASSERT(body);

	const cpVect current = body->bodyPosition();

	if (m_isReturning) {
		m_isReturning = false;
		m_waitEnd = 0;

		if (m_path.size() > 1) {
			const int last = TiledPathMotor::getShortestSegment(m_path, current);

			if (last != -1)
				TiledPathMotor::clearFromSegment(&m_path, last+1);
		}

		if (m_pathMotor)
			m_pathMotor.reset();
	}

	m_hasReturned = false;

	if (m_path.size() < 1) {
		addPoint(current, body->currentAngle());
		return;
	}

	const auto &angle = cpvtoangle(cpvsub(current, TiledObjectBody::toVect(m_path.last())));
	addPoint(current, angle);
}







/**
 * @brief TiledReturnPathMotor::finish
 * @param body
 */

void TiledReturnPathMotor::finish(TiledObject *body, AbstractGame::TickTimer *timer)
{
	Q_ASSERT(body);

	TiledScene *scene = body->scene();

	if (!scene) {
		LOG_CERROR("scene") << "Missing scene" << body;
		return;
	}

	m_pathMotor.reset(new TiledPathMotor);
	m_pathMotor->setDirection(TiledPathMotor::Forward);

	// A raycast pontatlan (átmegy az objektumokon, ezért kihagyjuk:
	// const auto &ptr = scene->findShortestPath(body, m_basePoint);

	TiledGame *g = scene->game();

	const auto &ptr = g->findShortestPath(body, m_basePoint);

	if (!ptr) {
		LOG_CTRACE("scene") << "No path from" << body->bodyPositionF() << "to" << m_basePoint.x << m_basePoint.y;

		const QPointF &currentPoint = body->bodyPositionF();

		if (!m_path.isEmpty()) {
			if (m_path.size() < 2 || QVector2D(m_path.last()).distanceToPoint(QVector2D(currentPoint)) >= 15.)
				m_path << currentPoint;

			QPolygonF rpath;
			rpath.reserve(m_path.size());

			std::reverse_copy(m_path.cbegin(), m_path.cend(), std::back_inserter(rpath));

			if (!(TiledObject::toVect(rpath.last()) == m_basePoint))
				rpath << TiledObject::toPointF(m_basePoint);

			m_pathMotor->setPolygon(rpath);
		} else {
			LOG_CERROR("scene") << "Missing return path";
		}

	} else if (ptr.value().size() < 2) {
		QPolygonF p;
		p << body->bodyPositionF() << ptr.value();
		m_pathMotor->setPolygon(p);
	} else {
		m_pathMotor->setPolygon(ptr.value());
	}


	if (timer)
		m_waitEnd = timer->tickAddMsec(m_waitMsec);

	m_isReturning = true;
	m_hasReturned = false;

}


/**
 * @brief TiledReturnPathMotor::path
 * @return
 */

QPolygonF TiledReturnPathMotor::path() const
{
	if (m_pathMotor)
		return m_pathMotor->polygon();
	else
		return m_path;
}




/**
 * @brief TiledReturnPathMotor::addPoint
 * @param point
 * @param angle
 */

void TiledReturnPathMotor::addPoint(const cpVect &point, const float &angle)
{
	if (m_path.size() > 1 && m_lastAngle == angle)
		return;

	if (m_path.size() > 1 && cpvlengthsq(cpvsub(TiledObject::toVect(m_path.last()), point)) < POW2(50.) )
		return;

	// Check intersections

	if (m_path.size() > 1) {
		QLineF line(m_path.last(), TiledObject::toPointF(point));

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

	m_path << TiledObject::toPointF(point);
	m_lastAngle = angle;
}



bool TiledReturnPathMotor::isReturning() const
{
	return m_isReturning;
}

void TiledReturnPathMotor::setIsReturning(bool newIsReturning)
{
	m_isReturning = newIsReturning;
}


/**
 * @brief TiledReturnPathMotor::hasReturned
 * @return
 */

bool TiledReturnPathMotor::hasReturned() const
{
	return m_hasReturned;
}



qint64 TiledReturnPathMotor::waitMsec() const
{
	return m_waitMsec;
}

void TiledReturnPathMotor::setWaitMsec(qint64 newWaitMsec)
{
	m_waitMsec = newWaitMsec;
}


/**
 * @brief TiledReturnPathMotor::isReturnReady
 * @return
 */

bool TiledReturnPathMotor::isReturnReady(AbstractGame::TickTimer *timer) const
{
	return !timer || m_waitEnd <= 0 || timer->currentTick() > m_waitEnd;
}


