/*
 * ---- Call of Suli ----
 *
 * isometricentity.h
 *
 * Created on: 2024. 03. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEntity
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

#ifndef ISOMETRICENTITY_H
#define ISOMETRICENTITY_H

#include "isometricobject.h"
#include <QQmlEngine>


/**
 * @brief The IsometricEntity class
 */

class IsometricEntityIface
{
public:
	IsometricEntityIface()
	{}


	IsometricObjectIface::Direction movingDirection() const;
	void setMovingDirection(const IsometricObjectIface::Direction &newMovingDirection);

protected:
	void entityIfaceWorldStep(const QPointF &position, const IsometricObjectIface::Directions &availableDirections);
	qreal normalize(const qreal &radian) const;
	qreal unnormalize(const qreal &normal) const;

	IsometricObjectIface::Direction m_movingDirection = IsometricObjectIface::Invalid;

private:
	QPointF m_lastPosition;
};



/**
 * @brief The IsometricCircleEntity class
 */

class IsometricCircleEntity : public IsometricObjectCircle, public IsometricEntityIface
{
	Q_OBJECT
	QML_ELEMENT

public:
	explicit IsometricCircleEntity(QQuickItem *parent = nullptr);

protected:
	virtual void entityWorldStep() {}

	void worldStep() override final {
		//entityIfaceWorldStep(position(), m_availableDirections);
		rotateBody();
		entityWorldStep();
	};

private:
	struct RotateAnimation {
		bool running = false;
		qreal destAngle = 0;
		bool clockwise = true;
	};

	RotateAnimation m_rotateAnimation;

	void rotateBody();
};

#endif // ISOMETRICENTITY_H
