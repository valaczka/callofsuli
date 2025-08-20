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


void IsometricObject::setSubZ(qreal newSubZ)
{
	if (newSubZ >= 1.0)
		LOG_CERROR("scene") << "Invalid subZ value:" << newSubZ;

	if (qFuzzyCompare(m_subZ, newSubZ))
		return;

	TiledObjectBody::setSubZ(newSubZ);

	emit subZChanged();
}



/**
 * @brief IsometricObjectIface::useDynamicZ
 * @return
 */

void IsometricObject::setUseDynamicZ(bool newUseDynamicZ)
{
	if (m_useDynamicZ == newUseDynamicZ)
		return;

	TiledObjectBody::setUseDynamicZ(newUseDynamicZ);

	emit useDynamicZChanged();
}





/**
 * @brief IsometricObjectIface::defaultZ
 * @return
 */

void IsometricObject::setDefaultZ(qreal newDefaultZ)
{
	if (qFuzzyCompare(m_defaultZ, newDefaultZ))
		return;

	TiledObjectBody::setDefaultZ(newDefaultZ);

	emit defaultZChanged();
}




