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

#ifndef ISOGAMEOBJECT_H
#define ISOGAMEOBJECT_H

#include "tiledmapobject.h"
#include <QQmlEngine>

class IsoGameObject : public TiledMapObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Direction currentDirection READ currentDirection WRITE setCurrentDirection NOTIFY currentDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)

public:
	IsoGameObject();

	// Object moving (facing) directions

	enum Direction {
		Invalid		= 0,
		North		= 1,
		NorthEast	= 1 << 1,
		East		= 1 << 2,
		SouthEast	= 1 << 3,
		South		= 1 << 4,
		SouthWest	= 1 << 5,
		West		= 1 << 6,
		NorthWest	= 1 << 7
	};

	Q_ENUM(Direction)


	// Available sprite directions count

	enum Directions {
		None = 0,
		Direction_4,			// N, E, S, W
		Direction_8,			// N, NE, E, SE, S, SW, W, NW
		Direction_Infinite
	};

	Q_ENUM(Directions)


	static qreal angleFromDirection(const Direction &direction);

	static Direction directionFromAngle(const Directions &directions, const qreal &angle);
	Q_INVOKABLE Direction directionFromAngle(const qreal &angle) const { return directionFromAngle(m_availableDirections, angle); };

	Direction currentDirection() const;
	void setCurrentDirection(Direction newCurrentDirection);

	Q_INVOKABLE void load();
	Q_INVOKABLE void updateSprite();

	Directions availableDirections() const;
	void setAvailableDirections(Directions newAvailableDirections);

signals:
	void availableDirectionsChanged();
	void currentDirectionChanged();

protected:
	void onSceneConnected() override;
	virtual bool initSprites() { return false; }

private:
	void onJoystickStateChanged();

protected:
	Direction m_currentDirection = Invalid;
	Directions m_availableDirections = None;

	bool m_isRun = false;

private:

};


#endif // ISOGAMEOBJECT_H
