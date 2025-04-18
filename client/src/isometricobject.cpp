/*
 * ---- Call of Suli ----
 *
 * isogameobject.cpp
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricObjectIface
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

#include "isometricobject.h"
#include "Logger.h"
#include <QtMath>



/**
 * @brief IsometricObject::IsometricObject
 * @param parent
 */

IsometricObject::IsometricObject(const QPointF &center, const qreal &radius, TiledGame *game, const cpBodyType &type)
	: TiledObject(center, radius, game, nullptr, type)
{

}

/**
 * @brief IsometricObjectIface::subZ
 * @return
 */

qreal IsometricObject::subZ() const
{
	return m_subZ;
}

void IsometricObject::setSubZ(qreal newSubZ)
{
	if (newSubZ >= 1.0)
		LOG_CERROR("scene") << "Invalid subZ value:" << newSubZ;

	if (qFuzzyCompare(m_subZ, newSubZ))
		return;
	m_subZ = newSubZ;
	emit subZChanged();
}



/**
 * @brief IsometricObjectIface::useDynamicZ
 * @return
 */

bool IsometricObject::useDynamicZ() const
{
	return m_useDynamicZ;
}

void IsometricObject::setUseDynamicZ(bool newUseDynamicZ)
{
	if (m_useDynamicZ == newUseDynamicZ)
		return;
	m_useDynamicZ = newUseDynamicZ;
	emit useDynamicZChanged();
}





/**
 * @brief IsometricObjectIface::defaultZ
 * @return
 */


qreal IsometricObject::defaultZ() const
{
	return m_defaultZ;
}

void IsometricObject::setDefaultZ(qreal newDefaultZ)
{
	if (qFuzzyCompare(m_defaultZ, newDefaultZ))
		return;
	m_defaultZ = newDefaultZ;
	emit defaultZChanged();
}




