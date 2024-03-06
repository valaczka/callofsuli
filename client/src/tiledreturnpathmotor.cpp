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

	m_isReturning = false;

	const QPointF &currentPoint = body->bodyPosition();

	body->setLinearVelocity(TiledObjectBase::toPoint(angle, radius));

	if (!m_path.isEmpty() && m_lastAngle == angle)
		return;

	if (!m_path.isEmpty()) {
		QVector2D vector(m_path.last()-currentPoint);
		if (vector.length() < 50)
			return;
	}

	m_path << currentPoint;
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
	m_path << currentPoint;

	m_pathMotor.setDirection(TiledPathMotor::Backward);
	m_pathMotor.setPolygon(m_path);

	m_isReturning = true;
}



/**
 * @brief TiledReturnPathMotor::stepBack
 * @param body
 * @param radius
 * @return
 */

bool TiledReturnPathMotor::stepBack(const qreal &radius)
{
	if (!m_isReturning) {
		LOG_CERROR("scene") << "Not in returning state";
		return false;
	}

	if (m_pathMotor.atBegin())
		return false;

	m_pathMotor.step(radius);
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
