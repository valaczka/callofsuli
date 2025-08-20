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



/**
 * @brief The IsometricObject class
 */

class IsometricObject : public TiledObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(qreal defaultZ READ defaultZ WRITE setDefaultZ NOTIFY defaultZChanged FINAL)
	Q_PROPERTY(bool useDynamicZ READ useDynamicZ WRITE setUseDynamicZ NOTIFY useDynamicZChanged FINAL)
	Q_PROPERTY(qreal subZ READ subZ WRITE setSubZ NOTIFY subZChanged FINAL)

public:
	IsometricObject(const QPointF &center, const qreal &radius, TiledGame *game, const cpBodyType &type = CP_BODY_TYPE_DYNAMIC);

	void setDefaultZ(qreal newDefaultZ);
	void setUseDynamicZ(bool newUseDynamicZ);
	void setSubZ(qreal newSubZ);

signals:
	void defaultZChanged();
	void useDynamicZChanged();
	void subZChanged();
};


#endif // ISOMETRICOBJECT_H
