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


	TiledObject::Direction movingDirection() const;
	void setMovingDirection(const TiledObject::Direction &newMovingDirection);

	qreal maximumSpeed() const;
	void setMaximumSpeed(qreal newMaximumSpeed);

	QPointF maximizeSpeed(const QPointF &point) const;
	QPointF &maximizeSpeed(QPointF &point) const;

	int hp() const;
	void setHp(int newHp);

public:
	virtual void hpChanged() = 0;

protected:
	void entityIfaceWorldStep(const QPointF &position, const TiledObject::Directions &availableDirections);

	virtual void updateSprite() = 0;
	virtual void onAlive() = 0;
	virtual void onDead() = 0;


	TiledObject::Direction m_movingDirection = TiledObject::Invalid;
	qreal m_maximumSpeed = 10.;
	int m_hp = 0;

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

	Q_PROPERTY(int hp READ hp WRITE setHp NOTIFY hpChanged FINAL)

public:
	explicit IsometricCircleEntity(QQuickItem *parent = nullptr);

	void emplace(const QPointF &pos) {
		m_body->emplace(pos);
		updateSprite();
	}

protected:
	virtual void entityWorldStep() {}

	void worldStep() override final {
		entityIfaceWorldStep(position(), m_availableDirections);
		entityWorldStep();
	};

};

#endif // ISOMETRICENTITY_H
