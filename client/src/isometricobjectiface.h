/*
 * ---- Call of Suli ----
 *
 * isometricobjectiface.h
 *
 * Created on: 2024. 03. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef ISOMETRICOBJECTIFACE_H
#define ISOMETRICOBJECTIFACE_H

#include "qobjectdefs.h"
#include "qtypes.h"

/**
 * @brief The IsometricObjectIface class
 */

class IsometricObjectIface
{
	Q_GADGET

public:
	IsometricObjectIface();

	// Object moving (facing) directions

	enum Direction {
		Invalid		= 0,
		NorthEast	= 45,
		East		= 90,
		SouthEast	= 135,
		South		= 180,
		SouthWest	= 225,
		West		= 270,
		NorthWest	= 315,
		North		= 360,
	};

	Q_ENUM(Direction);


	// Available sprite directions count

	enum Directions {
		None = 0,
		Direction_4,			// N, E, S, W
		Direction_8,			// N, NE, E, SE, S, SW, W, NW
		Direction_Infinite
	};

	Q_ENUM(Directions);



	static qreal toRadian(const qreal &angle);
	static qreal toDegree(const qreal &angle);
	static qreal radianFromDirection(const Direction &direction);
	static qreal factorFromDegree(const qreal &angle, const qreal &xyRatio = 2.);
	static qreal factorFromRadian(const qreal &angle, const qreal &xyRatio = 2.);

	static Direction nearestDirectionFromRadian(const Directions &directions, const qreal &angle);
	Direction nearestDirectionFromRadian(const qreal &angle) const { return nearestDirectionFromRadian(m_availableDirections, angle); };

	static qreal directionToRadian(const Direction &direction);

	Direction currentDirection() const;
	void setCurrentDirection(Direction newCurrentDirection);

	Directions availableDirections() const;
	void setAvailableDirections(Directions newAvailableDirections);

	qreal defaultZ() const;
	void setDefaultZ(qreal newDefaultZ);

	bool useDynamicZ() const;
	void setUseDynamicZ(bool newUseDynamicZ);

	qreal subZ() const;
	void setSubZ(qreal newSubZ);

protected:
	virtual void availableDirectionsChanged() = 0;
	virtual void currentDirectionChanged() = 0;
	virtual void defaultZChanged() = 0;
	virtual void useDynamicZChanged() = 0;
	virtual void subZChanged() = 0;
	virtual void onXYChanged() = 0;


protected:
	Direction m_currentDirection = Invalid;
	Directions m_availableDirections = None;
	qreal m_defaultZ = 0;
	qreal m_subZ = 0;
	bool m_useDynamicZ = true;
};




#endif // ISOMETRICOBJECTIFACE_H
