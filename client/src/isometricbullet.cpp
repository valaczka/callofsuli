/*
 * ---- Call of Suli ----
 *
 * isometricbullet.cpp
 *
 * Created on: 2024. 03. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricBullet
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

#include "isometricbullet.h"
#include "tiledscene.h"



/**
 * @brief IsometricBullet::IsometricBullet
 * @param parent
 */

IsometricBullet::IsometricBullet(TiledGame *game, const cpBodyType &type)
	: IsometricObject(QPointF(), 10., game, type)
{

}



/**
 * @brief IsometricBullet::~IsometricBullet
 */

IsometricBullet::~IsometricBullet()
{

}




/**
 * @brief IsometricBullet::initialize
 */

void IsometricBullet::initialize()
{
	setDefaultZ(1);
	setSubZ(0.8);

	m_speed = 300.;

	load();

	if (m_visualItem)
		m_visualItem->setZ(1);
}




/**
 * @brief IsometricBullet::shot
 * @param from
 * @param angle
 */

void IsometricBullet::shot(const QPointF &from, const qreal &angle)
{
	if (!scene())
		return;

	m_startPoint = QVector2D(from);
	emplace(from);
	setFacingDirection(nearestDirectionFromRadian(angle));
	cpBodySetAngle(body(), angle);
	setSpeedFromAngle(angle, m_speed);
}





/**
 * @brief IsometricBullet::worldStep
 */

void IsometricBullet::worldStep()
{
	if (m_facingDirection == Invalid) {
		stop();
		return;
	}

	const qreal &distance = distanceToPoint(m_startPoint);

	if (distance >= m_maxDistance) {
		overshootEvent();
		return;
	}
}


/**
 * @brief IsometricBullet::synchronize
 */

void IsometricBullet::synchronize()
{
	jumpToSprite("default", m_facingDirection);
	IsometricObject::synchronize();
}





/**
 * @brief IsometricBullet::maxDistance
 * @return
 */

qreal IsometricBullet::maxDistance() const
{
	return m_maxDistance;
}

void IsometricBullet::setMaxDistance(qreal newMaxDistance)
{
	if (qFuzzyCompare(m_maxDistance, newMaxDistance))
		return;
	m_maxDistance = newMaxDistance;
	emit maxDistanceChanged();
}



/**
 * @brief IsometricBullet::doAutoDelete
 */

void IsometricBullet::disableBullet()
{
	if (m_visualItem)
		m_visualItem->setVisible(false);
	stop();
}





/**
 * @brief IsometricBullet::onShapeContactBegin
 * @param self
 * @param other
 */

void IsometricBullet::onShapeContactBegin(cpShape *, cpShape *other)
{
	if (m_impacted) {
		stop();
		return;
	}

	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	impactEvent(base, other);
}



/**
 * @brief IsometricBullet::impacted
 * @return
 */

bool IsometricBullet::impacted() const
{
	return m_impacted;
}

void IsometricBullet::setImpacted(bool newImpacted)
{
	if (m_impacted == newImpacted)
		return;
	m_impacted = newImpacted;
	emit impactedChanged();
}
