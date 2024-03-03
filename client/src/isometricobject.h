/*
 * ---- Call of Suli ----
 *
 * isogameobject.h
 *
 * Created on: 2024. 02. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsoGameObject
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

#ifndef ISOMETRICOBJECT_H
#define ISOMETRICOBJECT_H

#include "tiledobject.h"
#include <QQmlEngine>
#include "isometricobjectiface.h"



/**
 * @brief The IsometricObject class
 */

class IsometricObject : public TiledObject, public IsometricObjectIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Direction currentDirection READ currentDirection WRITE setCurrentDirection NOTIFY currentDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)
	Q_PROPERTY(qreal defaultZ READ defaultZ WRITE setDefaultZ NOTIFY defaultZChanged FINAL)
	Q_PROPERTY(bool useDynamicZ READ useDynamicZ WRITE setUseDynamicZ NOTIFY useDynamicZChanged FINAL)
	Q_PROPERTY(qreal subZ READ subZ WRITE setSubZ NOTIFY subZChanged FINAL)

public:
	IsometricObject(QQuickItem *parent = nullptr)
		: TiledObject(parent)
		, IsometricObjectIface()
	{}


	static void onXYChanged(IsometricObjectIface *isoobject, TiledObjectBase *object);

signals:
	void availableDirectionsChanged() override;
	void currentDirectionChanged() override;
	void defaultZChanged() override;
	void useDynamicZChanged() override;
	void subZChanged() override;

protected:
	void onXYChanged() override { onXYChanged(this, this); }
};



/**
 * @brief The TiledObjectBasePolygon class
 */

class IsometricObjectCircle : public TiledObjectCircle, public IsometricObjectIface
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Direction currentDirection READ currentDirection WRITE setCurrentDirection NOTIFY currentDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)
	Q_PROPERTY(qreal defaultZ READ defaultZ WRITE setDefaultZ NOTIFY defaultZChanged FINAL)
	Q_PROPERTY(bool useDynamicZ READ useDynamicZ WRITE setUseDynamicZ NOTIFY useDynamicZChanged FINAL)
	Q_PROPERTY(qreal subZ READ subZ WRITE setSubZ NOTIFY subZChanged FINAL)

public:
	explicit IsometricObjectCircle(QQuickItem *parent = 0)
		: TiledObjectCircle(parent)
		, IsometricObjectIface()
	{}


signals:
	void availableDirectionsChanged() override;
	void currentDirectionChanged() override;
	void defaultZChanged() override;
	void useDynamicZChanged() override;
	void subZChanged() override;

protected:
	void onXYChanged() override { IsometricObject::onXYChanged(this, this); }

};
#endif // ISOMETRICOBJECT_H
