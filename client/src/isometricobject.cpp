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
#include "box2dfixture.h"
#include "tiledscene.h"
#include <QtMath>

IsometricObjectIface::IsometricObjectIface()

{

}


/**
 * @brief IsometricObjectIface::toRadian
 * @param angle
 * @return
 */

qreal IsometricObjectIface::toRadian(const qreal &angle)
{
	//return toRadians(angle);
	if (angle <= 180.)
		return angle * M_PI / 180.;
	else
		return -(360-angle) * M_PI / 180.;
}


/**
 * @brief IsometricObjectIface::toDegree
 * @param angle
 * @return
 */

qreal IsometricObjectIface::toDegree(const qreal &angle)
{
	//return toDegrees(angle);
	if (angle >= 0)
		return angle * 180 / M_PI;
	else
		return 360+(angle * 180 / M_PI);
}



/**
 * @brief IsometricObjectIface::radianFromDirection
 * @param directions
 * @param direction
 * @return
 */

qreal IsometricObjectIface::radianFromDirection(const Direction &direction)
{
	switch (direction) {
		case West: return M_PI; break;
		case NorthWest: return M_PI - atan(0.5); break;
		case North: return M_PI_2; break;
		case NorthEast: return atan(0.5); break;
		case East: return 0; break;
		case SouthEast: return -atan(0.5); break;
		case South: return -M_PI_2; break;
		case SouthWest: return -M_PI + atan(0.5); break;
		case Invalid: return 0; break;
	}

	return 0;
}


/**
 * @brief IsometricObjectIface::factorFromDegree
 * @param angle
 * @param xyRatio
 * @return
 */

qreal IsometricObjectIface::factorFromDegree(const qreal &angle, const qreal &xyRatio)
{
	return factorFromRadian(toRadian(angle), xyRatio);
}


/**
 * @brief IsometricObjectIface::factorFromRadian
 * @param angle
 * @param xyRatio
 * @return
 */

qreal IsometricObjectIface::factorFromRadian(const qreal &angle, const qreal &xyRatio)
{
	Q_ASSERT(xyRatio != 0);

	if (angle == 0 || angle == M_PI)
		return 1.0;

	qreal r = 0.;

	if (angle > M_PI_2)
		r = (M_PI-angle)/M_PI_2;
	else if (angle > 0)
		r = angle/M_PI_2;
	else if (angle > -M_PI_2)
		r = -angle/M_PI_2;
	else
		r = (M_PI+angle)/M_PI_2;

	return std::lerp(1., 0.7, r);
}




/**
 * @brief IsometricObject::directionFromRadian
 * @param angle
 * @param directions
 * @return
 */
/*
IsometricObject::Direction IsometricObject::directionFromRadian(const Directions &directions, const qreal &angle)
{
	static const std::map<qreal, Direction, std::greater<qreal>> map8 = {
		{ M_PI * 7/8	, West },
		{ M_PI * 5/8	, NorthWest },
		{ M_PI * 3/8	, North },
		{ M_PI * 1/8	, NorthEast },
		{ M_PI * -1/8	, East },
		{ M_PI * -3/8	, SouthEast },
		{ M_PI * -5/8	, South },
		{ M_PI * -7/8	, SouthWest },
		{ M_PI * -1		, West },
	};

	static const std::map<qreal, Direction, std::greater<qreal>> map4 = {
		{ M_PI * 3/4	, West },
		{ M_PI * 1/4	, North },
		{ M_PI * -1/4	, East },
		{ M_PI * -3/4	, South },
		{ M_PI * -1		, West },
	};


	const std::map<qreal, Direction, std::greater<qreal>> *ptr = nullptr;

	switch (directions) {
		case Direction_4:
			ptr = &map4;
			break;
		case Direction_8:
		case Direction_Infinite:
			ptr = &map8;
			break;
		default:
			break;
	}

	if (!ptr)
		return Invalid;

	for (const auto &[a, dir] : *ptr) {
		if (angle >= a)
			return dir;
	}

	return Invalid;
}
*/

/**
 * @brief IsometricObjectIface::nearestDirectionFromRadian
 * @param directions
 * @param angle
 * @return
 */

IsometricObjectIface::Direction IsometricObjectIface::nearestDirectionFromRadian(const Directions &directions, const qreal &angle)
{
	static const std::map<qreal, Direction, std::greater<qreal>> map8 = {
		{ M_PI			, West },
		{ M_PI_4 * 3	, NorthWest },
		{ M_PI_2		, North },
		{ M_PI_4		, NorthEast },
		{ 0				, East },
		{ -M_PI_4		, SouthEast },
		{ -M_PI_2		, South },
		{ -M_PI_4 * 3	, SouthWest },
		{ -M_PI			, West },
	};

	static const std::map<qreal, Direction, std::greater<qreal>> map4 = {
		{ M_PI		, West },
		{ M_PI_2	, North },
		{ 0			, East },
		{ -M_PI_2	, South },
		{ -M_PI		, West },
	};

	qreal diff = -1;
	Direction ret = Invalid;

	const std::map<qreal, Direction, std::greater<qreal>> *ptr = nullptr;

	switch (directions) {
		case Direction_4:
			ptr = &map4;
			break;
		case Direction_8:
		case Direction_Infinite:
			ptr = &map8;
			break;
		default:
			break;
	}

	if (!ptr)
		return Invalid;

	for (const auto &[a, dir] : *ptr) {
		const qreal d = std::abs(a-angle);
		if (diff == -1 || d < diff) {
			diff = d;
			ret = dir;
		}
	}

	return ret;
}


/**
 * @brief IsometricObjectIface::directionToRadian
 * @param direction
 * @return
 */

qreal IsometricObjectIface::directionToRadian(const Direction &direction)
{
	static const QMap<Direction, qreal> map8 = {
		{ West ,		M_PI		},
		{ NorthWest ,	M_PI_4 * 3	},
		{ North ,		M_PI_2		},
		{ NorthEast ,	M_PI_4		},
		{ East ,		0			},
		{ SouthEast ,	-M_PI_4		},
		{ South ,		-M_PI_2		},
		{ SouthWest ,	-M_PI_4 * 3	},
		{ West ,		-M_PI		},
	};

	return map8.value(direction, 0);
}




/**
 * @brief IsometricObjectIface::currentDirection
 * @return
 */

IsometricObjectIface::Direction IsometricObjectIface::currentDirection() const
{
	return m_currentDirection;
}

void IsometricObjectIface::setCurrentDirection(Direction newCurrentDirection)
{
	if (m_currentDirection == newCurrentDirection)
		return;
	m_currentDirection = newCurrentDirection;
	emit currentDirectionChanged();
}






/**
 * @brief IsometricObjectIface::availableDirections
 * @return
 */

IsometricObjectIface::Directions IsometricObjectIface::availableDirections() const
{
	return m_availableDirections;
}

void IsometricObjectIface::setAvailableDirections(Directions newAvailableDirections)
{
	if (m_availableDirections == newAvailableDirections)
		return;
	m_availableDirections = newAvailableDirections;
	emit availableDirectionsChanged();
}



/**
 * @brief IsometricObjectIface::subZ
 * @return
 */

qreal IsometricObjectIface::subZ() const
{
	return m_subZ;
}

void IsometricObjectIface::setSubZ(qreal newSubZ)
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

bool IsometricObjectIface::useDynamicZ() const
{
	return m_useDynamicZ;
}

void IsometricObjectIface::setUseDynamicZ(bool newUseDynamicZ)
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

qreal IsometricObjectIface::defaultZ() const
{
	return m_defaultZ;
}

void IsometricObjectIface::setDefaultZ(qreal newDefaultZ)
{
	if (qFuzzyCompare(m_defaultZ, newDefaultZ))
		return;
	m_defaultZ = newDefaultZ;
	emit defaultZChanged();
}







void IsometricObject::onXYChanged(IsometricObjectIface *isoobject, TiledObjectBase *object)
{
	if (!object || !object->scene() || !isoobject)
		return;

	QPointF p = object->position() + QPointF(object->width()/2, object->height()/2);
	object->setZ(object->scene()->getDynamicZ(p, isoobject->defaultZ()) + isoobject->subZ());
}
