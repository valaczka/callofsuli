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
#include "tiledpathmotor.h"
#include <QQmlEngine>

/**
 * @brief The IsometricObjectSpriteDirection class
 */

class IsometricObjectSprite : public TiledObjectSprite
{
	Q_GADGET

public:
	IsometricObjectSprite()
		: startRow(0)
		, startColumn(-1)
	{}

	QS_SERIALIZABLE

	QS_FIELD(int, startRow)
	QS_FIELD(int, startColumn)
	QS_COLLECTION(QList, int, directions)
};



/**
 * @brief The IsometricObjectSprite class
 */

class IsometricObjectSpriteList : public QSerializer
{
	Q_GADGET

public:
	IsometricObjectSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, IsometricObjectSprite, sprites)
};






/**
 * @brief The TiledMapObjectAlterableSprite class
 */

class IsometricObjectAlterableSprite : public QSerializer
{
	Q_GADGET

public:
	IsometricObjectAlterableSprite()
	{}

	QS_SERIALIZABLE
	QS_QT_DICT(QMap, QString, QString, alterations)
	QS_COLLECTION_OBJECTS(QList, IsometricObjectSprite, sprites)
};





/**
 * @brief The TiledMapObjectAlterableSpriteList class
 */

class IsometricObjectAlterableSpriteList : public QSerializer
{
	Q_GADGET

public:
	IsometricObjectAlterableSpriteList()
	{}

	QS_SERIALIZABLE

	QS_COLLECTION_OBJECTS(QList, IsometricObjectAlterableSprite, list)
};




/**
 * @brief The IsometricObject class
 */

class IsometricObject : public TiledObject
{
	Q_OBJECT
	QML_ELEMENT

	Q_PROPERTY(Direction currentDirection READ currentDirection WRITE setCurrentDirection NOTIFY currentDirectionChanged FINAL)
	Q_PROPERTY(Directions availableDirections READ availableDirections WRITE setAvailableDirections NOTIFY availableDirectionsChanged FINAL)
	Q_PROPERTY(qreal defaultZ READ defaultZ WRITE setDefaultZ NOTIFY defaultZChanged FINAL)
	Q_PROPERTY(bool useDynamicZ READ useDynamicZ WRITE setUseDynamicZ NOTIFY useDynamicZChanged FINAL)

public:
	IsometricObject(QQuickItem *parent = nullptr);

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

	static Direction nearestDirectionFromAngle(const Directions &directions, const qreal &angle);
	Q_INVOKABLE Direction nearestDirectionFromAngle(const qreal &angle) const { return nearestDirectionFromAngle(m_availableDirections, angle); };

	Direction currentDirection() const;
	void setCurrentDirection(Direction newCurrentDirection);


	//virtual void worldStep() override;

	Directions availableDirections() const;
	void setAvailableDirections(Directions newAvailableDirections);

	qreal defaultZ() const;
	void setDefaultZ(qreal newDefaultZ);

	bool useDynamicZ() const;
	void setUseDynamicZ(bool newUseDynamicZ);

signals:
	void availableDirectionsChanged();
	void currentDirectionChanged();
	void defaultZChanged();
	void useDynamicZChanged();

protected:
	void onXYChanged();
	//void onSceneConnected() override;

	bool appendSprite(const QString &source, const IsometricObjectSprite &sprite);
	bool appendSpriteList(const QString &source, const IsometricObjectSpriteList &spriteList);
	bool appendSprite(const IsometricObjectAlterableSprite &sprite, const QString &path = QStringLiteral(""));
	bool appendSpriteList(const IsometricObjectAlterableSpriteList &sprite, const QString &path = QStringLiteral(""));
	static QString getSpriteName(const QString &sprite, const Direction &direction = Invalid, const QString &alteration = QStringLiteral(""));

protected:
	Direction m_currentDirection = Invalid;
	Directions m_availableDirections = None;
	qreal m_defaultZ = 0;
	bool m_useDynamicZ = true;
};


#endif // ISOMETRICOBJECT_H
