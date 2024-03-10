/*
 * ---- Call of Suli ----
 *
 * isometricenemy.cpp
 *
 * Created on: 2024. 03. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricEnemy
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

#include "isometricenemy.h"
#include "box2dworld.h"
#include "isometricplayer.h"
#include "tiledspritehandler.h"


/**
 * @brief IsometricEnemy::IsometricEnemy
 * @param parent
 */

IsometricEnemy::IsometricEnemy(QQuickItem *parent)
	: IsometricCircleEntity(parent)
	, IsometricEnemyIface()
{

}



/**
 * @brief IsometricEnemy::createEnemy
 * @param parent
 * @return
 */

IsometricEnemy *IsometricEnemy::createEnemy(TiledScene *scene)
{
	IsometricEnemy *enemy = nullptr;
	TiledObjectBase::createFromCircle<IsometricEnemy>(&enemy, QPointF{}, 30, nullptr, scene);

	if (enemy) {
		enemy->setScene(scene);
		enemy->load();
	}

	return enemy;
}




/**
 * @brief IsometricEnemy::entityWorldStep
 */

void IsometricEnemy::entityWorldStep()
{
	if (m_movingDirection != Invalid)
		setCurrentDirection(m_movingDirection);


	if (m_contactedPlayer) {
		if (checkPlayerVisibility(m_body.get(), m_contactedPlayer) && m_contactedPlayer->hp() > 0) {
			setPlayer(m_contactedPlayer);

			float32 angle = 0;
			QPointF dest;

			rotateToPlayer(m_player, &angle, &dest);

			const qreal &dist = QVector2D(dest).length();
			setPlayerDistance(dist);

			if (dist > m_metric.playerDistance) {
				if (m_metric.pursuitSpeed > 0) {		// Pursuit
					if (m_metric.returnSpeed != 0) {
						if (!m_returnPathMotor)
							m_returnPathMotor.reset(new TiledReturnPathMotor);
						/** TiledObject::factorFromRadian(angle)*/		/// --- pursuit speeed
						m_returnPathMotor->moveBody(m_body.get(), angle,
													m_metric.pursuitSpeed > 0 ? m_metric.pursuitSpeed : m_metric.speed);
					} else if (m_metric.speed > 0) {
						m_body->setLinearVelocity(maximizeSpeed(TiledObjectBase::toPoint(angle, m_metric.speed)));
					} else {
						m_body->stop();
					}
				} else {								// No pursuit
					m_body->stop();
				}
			} else {
				m_body->stop();
			}


			/*
			QVariantList list;
			list << m_body->bodyPosition() - QPointF(0, 64)
				 << m_contactedPlayer->body()->bodyPosition() - QPointF(0, 64);

			m_scene->setTestPoints(list);
			*/

			/*if (m_returnPathMotor) {
				QVariantList list;
				for (const auto &point : m_returnPathMotor->path())
					list.append(point);
				m_scene->setTestPoints(list);
			}*/

			enemyWorldStep();
			updateSprite();
			return;
		} else {
			setPlayer(nullptr);
			setPlayerDistance(-1.);
		}
	}

	if (m_returnPathMotor && !m_returnPathMotor->isReturning()) {
		m_returnPathMotor->finish(m_body.get());

		/*QVariantList list;
		for (const auto &point : m_returnPathMotor->path())
			list.append(point);
		m_scene->setTestPoints(list);*/
	}

	if (enemyWorldStep()) {
		rotateBody(directionToRadian(m_currentDirection));
		stepMotor();
	}

	updateSprite();
}



/**
 * @brief IsometricEnemy::enemyWorldStep
 */

bool IsometricEnemy::enemyWorldStep()
{
	if (m_player && m_playerDistance <= m_metric.playerDistance && m_player->hp() > 0) {
		m_body->stop();
		if (m_hitTimer.isValid()) {
			if (m_hitTimer.elapsed() > 1500) {
				LOG_CWARNING("scene") << "HIT";
				m_player->hurt();
				jumpToSprite("hit", m_currentDirection, m_currentAlteration);
				m_hitTimer.restart();
			}
		} else {
			LOG_CWARNING("scene") << "HIT";
			m_player->hurt();
			jumpToSprite("hit", m_currentDirection, m_currentAlteration);
			m_hitTimer.start();
		}
	} else {
		m_hitTimer.invalidate();
	}

	return true;
}




/**
 * @brief IsometricEnemy::onPathMotorLoaded
 */

void IsometricEnemy::onPathMotorLoaded(const AbstractTiledMotor::Type &/*type*/)
{
	m_body->emplace(m_motor->currentPosition());
}



/**
 * @brief IsometricEnemy::onAlive
 */

void IsometricEnemy::onAlive()
{

}

void IsometricEnemy::onDead()
{

}



/**
 * @brief IsometricEnemy::stepMotor
 */

void IsometricEnemy::stepMotor()
{
	if (!m_motor) {
		LOG_CERROR("scene") << "Missing motor" << this;
		return;
	}

	if (m_returnPathMotor) {
		if (m_metric.returnSpeed != 0. && m_returnPathMotor->stepBack(m_metric.returnSpeed > 0. ? m_metric.returnSpeed : m_metric.speed)) {
			m_body->setLinearVelocity(maximizeSpeed(m_returnPathMotor->currentPosition() - m_body->bodyPosition()));
			return;
		} else {
			/*QVariantList list;
			for (const auto &point : m_returnPathMotor->path())
				list.append(point);
			m_scene->setTestPoints(list);*/

			m_returnPathMotor.reset();
		}
	}

	if (m_metric.speed <= 0.)
		return;

	if (m_motor->type() == AbstractTiledMotor::PathMotor) {
		TiledPathMotor *motor = pathMotor();
		Q_ASSERT(motor);

		motor->step(m_metric.speed);

		if (motor->direction() == TiledPathMotor::Backward && motor->atBegin())
			motor->setDirection(TiledPathMotor::Forward);
		else if (motor->direction() == TiledPathMotor::Forward && motor->atEnd())
			motor->setDirection(TiledPathMotor::Backward);

		m_body->setLinearVelocity(maximizeSpeed(motor->currentPosition() - m_body->bodyPosition()));
	}
}





/**
 * @brief IsometricEnemy::rotateToPlayer
 * @param player
 */

void IsometricEnemy::rotateToPlayer(IsometricPlayer *player, float32 *anglePtr, QPointF *vectorPtr)
{
	if (!player || !m_metric.rotateToPlayer)
		return;

	QPointF p = player->body()->bodyPosition() - m_body->bodyPosition();

	float32 angle = atan2(-p.y(), p.x());

	setCurrentDirection(nearestDirectionFromRadian(angle));
	m_body->body()->SetTransform(m_body->body()->GetPosition(), angle);

	if (anglePtr)
		*anglePtr = angle;

	if (vectorPtr)
		*vectorPtr = p;
}







void IsometricEnemy::load()
{
	LOG_CDEBUG("scene") << "Test";

	m_body->setBodyType(Box2DBody::Dynamic);

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	///setFixtureCenterVertical(0.8);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setCategories(Box2DFixture::Category3);
	m_fixture->setCollidesWith(Box2DFixture::Category1|Box2DFixture::Category2);

	TiledObjectSensorPolygon *p = addSensorPolygon(300);

	Q_ASSERT(p);

	m_metric.speed = 2.;
	m_metric.returnSpeed = 3.;
	m_metric.pursuitSpeed = 3.;



	connect(p, &TiledObjectSensorPolygon::beginContact, this, [this](Box2DFixture *other) {
		if (!other->categories().testFlag(Box2DFixture::Category2))
			return;

		IsometricPlayer *player = other->property("player").value<IsometricPlayer*>();

		LOG_CINFO("scene") << "CONTACT" << other << player;

		m_contactedPlayer = player;
		if (!player)
			setPlayer(nullptr);
	});

	connect(p, &TiledObjectSensorPolygon::endContact, this, [this](Box2DFixture *other) {
		if (!other->categories().testFlag(Box2DFixture::Category2))
			return;

		LOG_CINFO("scene") << "CONTACT END" << other;

		m_contactedPlayer = nullptr;
		setPlayer(nullptr);
	});


	QString path = ":/";

	createVisual();
	setAvailableDirections(Direction_8);


	QString test = R"({
					   "alterations": {
		"wnormal": "werebear_white_shirt.png",
		"warmor": "werebear_white_armor.png",
		"bnormal": "werebear_brown_shirt.png",
		"barmor": "werebear_brown_armor.png"
	},
	"sprites": [
		{
			"name": "idle",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 0,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 80
		},

		{
			"name": "run",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 512,
			"y": 0,
			"count": 8,
			"width": 128,
			"height": 128,
			"duration": 60
		},

		{
			"name": "hit",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 1536,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 80
		},

		{
			"name": "hurt",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 2048,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 80
		},


		{
			"name": "die",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 2560,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 80
		}

	]
	})";


	IsometricObjectAlterableSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, path);

	setWidth(128);
	setHeight(128);
	m_body->setBodyOffset(0, 0.45*64);


	nextAlteration();

	//m_spriteSequence->setProperty("currentSprite", "idle-4");
	//jumpToSprite("idle-4");
}






/**
 * @brief IsometricObject::updateSprite
 */

void IsometricEnemy::updateSprite()
{
	if (m_spriteHandler->currentSprite() == "hit")
		jumpToSpriteLater("idle", m_currentDirection, m_currentAlteration);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection, m_currentAlteration);
	else
		jumpToSprite("idle", m_currentDirection, m_currentAlteration);
}



void IsometricEnemy::nextAlteration()
{
	if (m_availableAlterations.isEmpty())
		return;

	int idx = m_availableAlterations.indexOf(m_currentAlteration);

	if (idx >= 0 && idx < m_availableAlterations.size()-1)
		m_currentAlteration = m_availableAlterations.at(idx+1);
	else
		m_currentAlteration = m_availableAlterations.at(0);

	LOG_CWARNING("scene") << "CURRENT ALTER" << m_currentAlteration;

	updateSprite();
}




/**
 * @brief IsometricEnemyIface::loadPathMotor
 * @param polygon
 */

void IsometricEnemyIface::loadPathMotor(const QPolygonF &polygon, const TiledPathMotor::Direction &direction)
{
	Q_ASSERT(!m_motor.get());

	TiledPathMotor *motor = new TiledPathMotor;

	motor->setPolygon(polygon);
	motor->setDirection(direction);
	if (direction == TiledPathMotor::Forward)
		motor->toBegin();
	else
		motor->toEnd();

	m_motor.reset(motor);

	onPathMotorLoaded(AbstractTiledMotor::PathMotor);
}


/**
 * @brief IsometricEnemyBase::player
 * @return
 */

IsometricPlayer *IsometricEnemyIface::player() const
{
	return m_player;
}

void IsometricEnemyIface::setPlayer(IsometricPlayer *newPlayer)
{
	if (m_player == newPlayer)
		return;
	m_player = newPlayer;
	emit playerChanged();
}



/**
 * @brief IsometricEnemyIface::checkPlayerVisibility
 * @param body
 * @param player
 * @return
 */

bool IsometricEnemyIface::checkPlayerVisibility(TiledObjectBody *body, TiledObjectBase *player)
{
	Q_ASSERT(body);
	Q_ASSERT(player);

	const TiledReportedFixtureMap &map = body->rayCast(player->body()->bodyPosition());

	for (auto it=map.constBegin(); it != map.constEnd(); ++it) {
		if (it->fixture->isSensor())
			continue;

		if (it->fixture->categories().testFlag(Box2DFixture::Category2))
			return true;

		if (it->fixture->categories().testFlag(Box2DFixture::Category1))
			return false;
	}

	return false;
}


float IsometricEnemyIface::playerDistance() const
{
	return m_playerDistance;
}

void IsometricEnemyIface::setPlayerDistance(float newPlayerDistance)
{
	if (qFuzzyCompare(m_playerDistance, newPlayerDistance))
		return;
	m_playerDistance = newPlayerDistance;
	emit playerDistanceChanged();
}


