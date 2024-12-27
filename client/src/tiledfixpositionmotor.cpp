/*
 * ---- Call of Suli ----
 *
 * tiledfixpositionmotor.cpp
 *
 * Created on: 2024. 03. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TiledFixPositionMotor
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

#include "tiledfixpositionmotor.h"


TiledFixPositionMotor::TiledFixPositionMotor()
	: AbstractTiledMotor(FixPositionMotor)
{

}




/**
 * @brief TiledFixPositionMotor::updateBody
 * @param object
 * @param maximumSpeed
 */

void TiledFixPositionMotor::updateBody(TiledObject *object, const float &, AbstractGame::TickTimer *)
{
	Q_ASSERT(object);
	object->stop();
	if (m_direction != TiledObject::Invalid)
		object->setCurrentDirection(m_direction);
}



/**
 * @brief TiledFixPositionMotor::point
 * @return
 */

QPointF TiledFixPositionMotor::point() const
{
	return m_point;
}

void TiledFixPositionMotor::setPoint(QPointF newPoint)
{
	m_point = newPoint;
}



/**
 * @brief TiledFixPositionMotor::direction
 * @return
 */

TiledObject::Direction TiledFixPositionMotor::direction() const
{
	return m_direction;
}

void TiledFixPositionMotor::setDirection(const TiledObject::Direction &newDirection)
{
	m_direction = newDirection;
}
