/*
 * ---- Call of Suli ----
 *
 * isogameobject.cpp
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

#include "isogameobject.h"
#include "Logger.h"
#include "box2dfixture.h"
#include <QtMath>

IsoGameObject::IsoGameObject()
	: TiledMapObject()
{
	m_body->setFixedRotation(true);
}



/**
 * @brief IsoGameObject::angleFromDirection
 * @param directions
 * @param direction
 * @return
 */

qreal IsoGameObject::angleFromDirection(const Direction &direction)
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
 * @brief IsoGameObject::directionFromAngle
 * @param angle
 * @param directions
 * @return
 */

IsoGameObject::Direction IsoGameObject::directionFromAngle(const Directions &directions, const qreal &angle)
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



IsoGameObject::Direction IsoGameObject::currentDirection() const
{
	return m_currentDirection;
}

void IsoGameObject::setCurrentDirection(Direction newCurrentDirection)
{
	if (m_currentDirection == newCurrentDirection)
		return;
	m_currentDirection = newCurrentDirection;
	emit currentDirectionChanged();
}


/**
 * @brief IsoGameObject::load
 */

void IsoGameObject::load()
{
	LOG_CDEBUG("scene") << "Test";

	QString path = "/home/valaczka/Projektek/_callofsuli-resources/isometric/character/avatar/character_w_clothes.png";

	createVisual();
	setAvailableDirections(Direction_8);

	m_body->setBodyType(Box2DBody::Dynamic);

	QList<Direction> rows = {
		West,
		NorthWest,
		North,
		NorthEast,
		East,
		SouthEast,
		South,
		SouthWest
	};


	for (int i=0; i<rows.size(); ++i) {
		TiledMapObjectSprite s;
		s.frameX = 0;
		s.frameY = i*128;
		s.frameCount = 4;
		s.frameWidth = 128;
		s.frameHeight = 128;
		s.frameDuration = 80;
		s.name = QString("idle-%1").arg(rows.at(i));
		LOG_CDEBUG("scene") << "Add" << s.name;
		s.to = {
			{ s.name, 1 }
		};
		appendSprite(path, s);
	}

	for (int i=0; i<rows.size(); ++i) {
		TiledMapObjectSprite s;
		s.frameX = 512;
		s.frameY = i*128;
		s.frameCount = 8;
		s.frameWidth = 128;
		s.frameHeight = 128;
		s.frameDuration = 60;
		s.name = QString("run-%1").arg(rows.at(i));
		s.to = {
			{ s.name, 1 }
		};
		appendSprite(path, s);
	}


	setImplicitWidth(128);
	setImplicitHeight(128);

	connect(this, &IsoGameObject::currentDirectionChanged, this, &IsoGameObject::updateSprite);

	connect(this, &IsoGameObject::xChanged, this, [this]() {
		QPointF p = position() + QPointF(width()/2, height()/2);
		if (m_scene)
			setZ(m_scene->getDynamicZ(p, 1));

	});

	connect(this, &IsoGameObject::yChanged, this, [this]() {
		QPointF p = position() + QPointF(width()/2, height()/2);
		if (m_scene)
			setZ(m_scene->getDynamicZ(p, 1));
	});

	//m_spriteSequence->setProperty("currentSprite", "idle-4");
	//jumpToSprite("idle-4");

}


/**
 * @brief IsoGameObject::updateSprite
 */

void IsoGameObject::updateSprite()
{
	if (m_currentDirection == Invalid)
		return;

	QPointF dir;

	const qreal radius = 7;

	if (m_isRun) {
		const qreal d = angleFromDirection(m_currentDirection);
		dir.setX(radius * cos(d));
		dir.setY(radius * -sin(d));
	}

	m_body->setLinearVelocity(dir);

	QString sprite = m_isRun ? QString("run-%1").arg(m_currentDirection) : QString("idle-%1").arg(m_currentDirection);

	jumpToSprite(sprite);
}




/**
 * @brief IsoGameObject::availableDirections
 * @return
 */

IsoGameObject::Directions IsoGameObject::availableDirections() const
{
	return m_availableDirections;
}

void IsoGameObject::setAvailableDirections(Directions newAvailableDirections)
{
	if (m_availableDirections == newAvailableDirections)
		return;
	m_availableDirections = newAvailableDirections;
	emit availableDirectionsChanged();
}


/**
 * @brief IsoGameObject::onSceneConnected
 */

void IsoGameObject::onSceneConnected()
{
	Q_ASSERT(m_scene);

	connect(m_scene, &TiledScene::joystickStateChanged, this, &IsoGameObject::onJoystickStateChanged);
}

void IsoGameObject::onJoystickStateChanged()
{
	Q_ASSERT(m_scene);

	m_isRun = m_scene->joystickState().distance >= 0.5;

	if (m_scene->joystickState().hasKeyboard || m_scene->joystickState().hasTouch)
		setCurrentDirection(directionFromAngle(m_scene->joystickState().angle));

	updateSprite();
}
