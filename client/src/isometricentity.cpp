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

IsometricEntity::IsometricEntity(TiledScene *scene)
	: IsometricObject(scene)
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
 * @brief IsometricEntity::synchronize
 */

void IsometricEntity::synchronize()
{
	IsometricObject::synchronize();
	updateSprite();
}



/**
 * @brief IsometricEntityIface::checkEntityVisibility
 * @param body
 * @param entity
 * @return
 */

std::optional<QPointF> IsometricEntity::checkEntityVisibility(TiledObjectBody *body, TiledObject *entity,
															  const TiledObjectBody::FixtureCategory &category,
															  float *transparentGroundPtr)
{
	Q_ASSERT(body);
	Q_ASSERT(entity);

	const QPointF &entityPosition = entity->bodyPosition();
	/*
	QList<QPointF> points;

	points.append(playerPosition);

	// Get tangents

	TiledObjectCircle *circleObj = dynamic_cast<TiledObjectCircle*>(player);

	if (circleObj) {
		Box2DCircle *fixture = circleObj->fixture();
		const float &radius = fixture->radius();
		const float centerX = playerPosition.x();
		const float centerY = playerPosition.y();

		const float bX = (body->bodyPosition().x() - centerX) / radius;
		const float bY = (body->bodyPosition().y() - centerY) / radius;

		const float xy = bX*bX + bY*bY;


		// point outside of circumfence, one tangent
		if (xy > 1.0) {
			float D = bY * sqrt(xy - 1.);

			float tx0 = (bX - D) / xy;
			float tx1 = (bX + D) / xy;

			float x0, x1, y0, y1;

			if (bY != 0.) {
				y0 = centerY + radius * (1. - tx0 * bX) / bY;
				y1 = centerY + radius * (1. - tx1 * bX) / bY;
			} else {
				D = radius * sqrt(1. - tx0 * tx0);
				y0 = centerY + D;
				y1 = centerY - D;
			}

			x0 = centerX + radius * tx0; //restore scale and position
			x1 = centerX + radius * tx1;

			points.append(QPointF{x0, y0});
			points.append(QPointF{x1, y1});
		}
	}

	for (const QPointF &p : points) {
*/

	float rayLength = 0.;
	const TiledReportedFixtureMap &map = body->rayCast(entityPosition, category);

	bool visible = false;

	for (auto it=map.begin(); it != map.end(); ++it) {
		b2::ShapeRef r = it->shape;

		if (r.IsSensor())
			continue;

		if (FixtureCategories::fromInt(r.GetFilter().categoryBits).testFlag(category)) {
			visible = true;
			break;
		}

		if (FixtureCategories::fromInt(r.GetFilter().categoryBits).testFlag(TiledObjectBody::FixtureGround)) {
			if (TiledObjectBody *body = TiledObjectBody::fromBodyRef(r.GetBody())) {
				if (body->opaque()) {
					visible = false;
					break;
				} else if (transparentGroundPtr) {
					*transparentGroundPtr = it.key() * rayLength;
				}
			}
		}
	}

	if (visible)
		return entityPosition /*p*/;
	/*	}*/

	return std::nullopt;
}



/**
 * @brief IsometricEntityIface::checkGroundDistance
 * @param body
 * @param targetPoint
 * @return -1.: no ground
 */

float IsometricEntity::checkGroundDistance(TiledObjectBody *body, const QPointF &targetPoint, float *lengthPtr)
{
	Q_ASSERT(body);

	return -1;

	/*if (body && body->scene()->isGroundContainsPoint(body->bodyPosition()))
		return 0.;

	float dist = -1.;

	const TiledReportedFixtureMap &map = body->rayCast(targetPoint, lengthPtr);

	for (auto it=map.constBegin(); it != map.constEnd(); ++it) {
		if (it->fixture->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround))) {
			if (dist == -1. || it.key() < dist)
				dist = it.key();
		}
	}

	return dist;*/
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

