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
#include "isometricenemy.h"
#include "qtimer.h"
#include "tiledscene.h"
#include "tiledgame.h"

IsometricBullet::IsometricBullet(QQuickItem *parent)
	: IsometricObjectCircle(parent)
{
	LOG_CTRACE("scene") << "bullet create" << this;
}



/**
 * @brief IsometricBullet::~IsometricBullet
 */

IsometricBullet::~IsometricBullet()
{
	LOG_CTRACE("scene") << "bullet destroy" << this;
}



/**
 * @brief IsometricBullet::createBullet
 * @param game
 * @param scene
 * @return
 */

IsometricBullet *IsometricBullet::createBullet(TiledGame *game, TiledScene *scene)
{
	IsometricBullet *bullet = nullptr;
	TiledObjectBase::createFromCircle<IsometricBullet>(&bullet, QPointF{}, 20, nullptr, scene);

	if (bullet) {
		bullet->m_body->setBullet(true);
		bullet->setGame(game);
		bullet->setScene(scene);
		bullet->load();
	}

	return bullet;
}



/**
 * @brief IsometricBullet::shot
 * @param from
 * @param direction
 * @param distance
 */

void IsometricBullet::shot(const QPointF &from, const Direction &direction)
{
	m_startPoint = from;
	m_body->emplace(from);
	setCurrentDirection(direction);
	m_direction = direction;
	m_angle = 0.;
	jumpToSprite("base", m_currentDirection, "none");
}


/**
 * @brief IsometricBullet::shot
 * @param from
 * @param angle
 */

void IsometricBullet::shot(const QPointF &from, const qreal &angle)
{
	m_startPoint = from;
	m_body->emplace(from);
	setCurrentDirection(nearestDirectionFromRadian(angle));
	m_direction = Invalid;
	m_angle = angle;
	jumpToSprite("base", m_currentDirection, "none");
}



/**
 * @brief IsometricBullet::worldStep
 */

void IsometricBullet::worldStep()
{
	if (m_currentDirection == Invalid) {
		m_body->stop();
		return;
	}

	const qreal &distance = QVector2D(m_startPoint - m_body->bodyPosition()).length();

	if (distance >= m_maxDistance) {
		m_scene->removeFromObjects(this);
		this->deleteLater();
		return;
	}

	if (m_direction != Invalid) {
		m_body->setLinearVelocity(TiledObjectBase::toPoint(directionToIsometricRaidan(m_direction), 30.));
	} else {
		m_body->setLinearVelocity(TiledObjectBase::toPoint(m_angle, 30.));
	}

	jumpToSprite("base", m_currentDirection, "none");
}



/**
 * @brief IsometricBullet::load
 */

void IsometricBullet::load()
{
	LOG_CDEBUG("scene") << "Test bullet";

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.7);
	setWidth(64);
	setHeight(64);

	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setFixedRotation(true);
	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround));
	m_fixture->setSensor(true);

	QString path = ":/";

	createVisual();
	setAvailableDirections(Direction_8);


	QString test = R"({
					   "alterations": {
		"none": "lightning.png",
		"stuck": "arrow_stuck.png"
	},
	"sprites": [
		{
			"name": "base",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 0,
			"y": 0,
			"count": 4,
			"width": 64,
			"height": 64,
			"duration": 30
		}
	]
	})";


	IsometricObjectAlterableSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, path);

	setBodyOffset(0, 25);



	connect(m_fixture.get(), &Box2DCircle::beginContact, this, [this](Box2DFixture *other) {
		if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround))) {
			setImpacted(true);
			LOG_CTRACE("scene") << "GROUND";
			m_body->stop();
			setCurrentDirection(Invalid);
			m_scene->removeFromObjects(this);
			this->deleteLater();
			return;
		}

		if (m_impacted)
			return;

		TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy) {
			setImpacted(true);
			enemy->setHp(0);
			m_body->stop();
			setCurrentDirection(Invalid);
			m_scene->removeFromObjects(this);
			this->deleteLater();
		}

	});
}

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
