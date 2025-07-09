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


/**
 * @brief IsometricEntity::IsometricEntity
 * @param parent
 */

IsometricEntity::IsometricEntity(TiledGame *game, const qreal &radius, const cpBodyType &type)
	: IsometricObject(QPointF(), radius, game, type)
{

}




/**
 * @brief IsometricEntityIface::maxHp
 * @return
 */

int IsometricEntity::maxHp() const
{
	return m_maxHp;
}

void IsometricEntity::setMaxHp(int newMaxHp)
{
	if (m_maxHp == newMaxHp)
		return;
	m_maxHp = newMaxHp;
	emit maxHpChanged();
}



/**
 * @brief IsometricEntity::setDestinationPoint
 * @param polygon
 */

void IsometricEntity::setDestinationPoint(const QPolygonF &polygon)
{
	if (!isAlive())
		return;

	if (!canSetDestinationPoint()) {
		clearDestinationPoint();
		stop();
		return;
	}

	if (polygon.size() == 1)
		return setDestinationPoint(toVect(polygon.first()));

	m_destinationMotor.reset(new TiledPathMotor(polygon));
	m_destinationPoint.reset();
}


/**
 * @brief IsometricEntity::setDestinationPoint
 * @param point
 */

void IsometricEntity::setDestinationPoint(const cpVect &point)
{
	if (!isAlive())
		return;

	if (!canSetDestinationPoint()) {
		clearDestinationPoint();
		stop();
		return;
	}

	m_destinationPoint = point;
	m_destinationMotor.reset();
}


/**
 * @brief IsometricEntity::clearDestinationPoint
 */

void IsometricEntity::clearDestinationPoint()
{
	m_destinationMotor.reset();
	m_destinationPoint.reset();
}





/**
 * @brief IsometricEntity::synchronize
 */

void IsometricEntity::synchronize()
{
	updateSprite();
	IsometricObject::synchronize();
}







/**
 * @brief IsometricEntityIface::hp
 * @return
 */

int IsometricEntity::hp() const
{
	return m_hp;
}

void IsometricEntity::setHp(int newHp)
{
	if (m_hp == newHp)
		return;

	const bool isHurt = (newHp < m_hp);
	const bool isHealed = (newHp > m_hp);
	const bool resurrected = (m_hp <= 0 && newHp > 0);

	m_hp = newHp;
	emit hpChanged();

	if (m_hp <= 0)
		onDead();
	else if (isHurt)
		emit hurt();
	else if (resurrected)
		onAlive();
	else if (isHealed)
		emit healed();

}

