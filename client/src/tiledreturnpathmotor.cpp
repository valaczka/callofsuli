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
#include "utils_.h"

TiledReturnPathMotor::TiledReturnPathMotor(const QPointF &basePoint)
	: AbstractTiledMotor(ReturnPathMotor)
	, m_basePoint(basePoint)
{

}



/**
 * @brief TiledReturnPathMotor::updateBody
 * @param object
 * @param distance
 * @param timer
 */

void TiledReturnPathMotor::updateBody(TiledObject *object, const float &distance, AbstractGame::TickTimer *timer)
{
	Q_ASSERT (object);

	if (!m_isReturning)
		return;

	if (m_pathMotor) {
		if (m_pathMotor->atEnd()) {
			object->body()->stop();
			object->body()->setIsRunning(false);
			m_pathMotor.reset();
			m_hasReturned = true;
			m_path.clear();
			setIsReturning(false);
		} else {
			m_pathMotor->updateBody(object, distance, timer);
			object->rotateBody(m_pathMotor->currentAngleRadian());
		}
	} else {
		setIsReturning(false);
	}
}




/**
 * @brief TiledReturnPathMotor::basePoint
 * @return
 */

QPointF TiledReturnPathMotor::basePoint()
{
	return m_basePoint;
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
		m_isReturning = false;
		m_waitEnd = 0;

		if (m_path.size() > 1) {
			const int last = TiledPathMotor::getShortestSegment(m_path, body->bodyPosition());

			if (last != -1)
				TiledPathMotor::clearFromSegment(&m_path, last+1);
		}

		if (m_pathMotor)
			m_pathMotor.reset();
	}

	m_hasReturned = false;

	body->setLinearVelocity(TiledObjectBase::toPoint(angle, radius));

	addPoint(body->bodyPosition(), angle);
}










/**
 * @brief TiledReturnPathMotor::finish
 * @param body
 */

void TiledReturnPathMotor::finish(TiledObjectBody *body, AbstractGame::TickTimer *timer)
{
	Q_ASSERT(body);
	Q_ASSERT(body->baseObject());

	TiledScene *scene = body->baseObject()->scene();

	if (!scene) {
		LOG_CERROR("scene") << "Missing scene" << body << body->baseObject();
		return;
	}

	m_pathMotor.reset(new TiledPathMotor);
	m_pathMotor->setDirection(TiledPathMotor::Forward);

	const auto &ptr = scene->findShortestPath(body->bodyPosition(), m_basePoint);

	if (!ptr) {
		LOG_CTRACE("scene") << "No path from" << body->bodyPosition() << "to" << m_basePoint;

		const QPointF &currentPoint = body->bodyPosition();

		if (!m_path.isEmpty()) {
			if (m_path.size() < 2 || QVector2D(m_path.last()).distanceToPoint(QVector2D(currentPoint)) >= 15.)
				m_path << currentPoint;

			QPolygonF rpath;
			rpath.reserve(m_path.size());

			std::reverse_copy(m_path.cbegin(), m_path.cend(), std::back_inserter(rpath));

			if (rpath.last() != m_basePoint)
				rpath << m_basePoint;

			m_pathMotor->setPolygon(rpath);
		} else {
			LOG_CERROR("scene") << "Missing return path";
		}

	} else if (ptr.value().size() < 2) {
		QPolygonF p;
		p << body->bodyPosition() << ptr.value();
		m_pathMotor->setPolygon(p);
	} else {
		m_pathMotor->setPolygon(ptr.value());
	}


	if (timer)
		m_waitEnd = timer->currentTick() + m_waitMsec;

	m_isReturning = true;
	m_hasReturned = false;
	clearLastSeenPoint();

}




/**
 * @brief TiledReturnPathMotor::addPoint
 * @param point
 * @param angle
 */

void TiledReturnPathMotor::addPoint(const QPointF &point, const float &angle)
{
	if (m_path.size() > 1 && m_lastAngle == angle)
		return;

	if (m_path.size() > 1 && QVector2D(point).distanceToPoint(QVector2D(m_path.last())) < 50.)
		return;

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


/**
 * @brief TiledReturnPathMotor::lastSeenPoint
 * @return
 */

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

