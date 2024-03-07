/*
 * ---- Call of Suli ----
 *
 * isometricentity.cpp
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

#include "isometricentity.h"

IsometricCircleEntity::IsometricCircleEntity(QQuickItem *parent)
	: IsometricObjectCircle(parent)
	, IsometricEntityIface()
{
	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setFixedRotation(true);
}




/**
 * @brief IsometricEntityIface::maximumSpeed
 * @return
 */

qreal IsometricEntityIface::maximumSpeed() const
{
	return m_maximumSpeed;
}

void IsometricEntityIface::setMaximumSpeed(qreal newMaximumSpeed)
{
	m_maximumSpeed = newMaximumSpeed;
}




/**
 * @brief IsometricEntityIface::maximizeSpeed
 * @param point
 * @return
 */

QPointF IsometricEntityIface::maximizeSpeed(const QPointF &point) const
{
	QPointF p;

	p.setX(std::max(-m_maximumSpeed, std::min(point.x(), m_maximumSpeed)));
	p.setY(std::max(-m_maximumSpeed, std::min(point.y(), m_maximumSpeed)));

	return p;
}



/**
 * @brief IsometricEntityIface::maximizeSpeed
 * @param point
 * @return
 */

QPointF &IsometricEntityIface::maximizeSpeed(QPointF &point) const
{
	point.setX(std::max(-m_maximumSpeed, std::min(point.x(), m_maximumSpeed)));
	point.setY(std::max(-m_maximumSpeed, std::min(point.y(), m_maximumSpeed)));
	return point;
}




/**
 * @brief IsometricEntityIface::entityWorldStep
 * @param position
 */

void IsometricEntityIface::entityIfaceWorldStep(const QPointF &position, const TiledObject::Directions &availableDirections)
{
	if (qFuzzyCompare(position.x(), m_lastPosition.x()) && qFuzzyCompare(position.y(), m_lastPosition.y()))
		return setMovingDirection(TiledObject::Invalid);

	QLineF line(m_lastPosition, position);
	setMovingDirection(TiledObject::nearestDirectionFromRadian(availableDirections,
															   TiledObject::toRadian(line.angle())));

	m_lastPosition = position;
}


/**
 * @brief IsometricEntityIface::game
 * @return
 */

TiledGame *IsometricEntityIface::game() const
{
	return m_game;
}

void IsometricEntityIface::setGame(TiledGame *newGame)
{
	m_game = newGame;
}



/**
 * @brief IsometricEntityIface::hp
 * @return
 */

int IsometricEntityIface::hp() const
{
	return m_hp;
}

void IsometricEntityIface::setHp(int newHp)
{
	if (m_hp == newHp)
		return;
	m_hp = newHp;
	emit hpChanged();

	if (m_hp <= 0)
		onDead();
}


/**
 * @brief IsometricEntityIface::movingDirection
 * @return
 */

TiledObject::Direction IsometricEntityIface::movingDirection() const
{
	return m_movingDirection;
}

void IsometricEntityIface::setMovingDirection(const TiledObject::Direction &newMovingDirection)
{
	if (m_movingDirection == newMovingDirection)
		return;
	m_movingDirection = newMovingDirection;
}
