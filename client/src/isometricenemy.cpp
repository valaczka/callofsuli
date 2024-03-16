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
#include "tiledfixpositionmotor.h"
#include "tiledspritehandler.h"



const QHash<QString, IsometricEnemyIface::EnemyType> IsometricEnemyIface::m_typeHash = {
	{ QStringLiteral("enemy"), EnemyWerebear }
};



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

IsometricEnemy *IsometricEnemy::createEnemy(const EnemyType &type, TiledGame *game, TiledScene *scene)
{
	IsometricEnemy *enemy = nullptr;

	switch (type) {
		case EnemyWerebear:
			TiledObjectBase::createFromCircle<IsometricEnemy>(&enemy, QPointF{}, 30, nullptr, game);
			break;

		case EnemyInvalid:
			LOG_CERROR("game") << "Invalid enemy type" << type;
			return nullptr;
	}

	if (enemy) {
		enemy->setParent(game);
		enemy->setGame(game);
		enemy->setScene(scene);
		enemy->load();
	}

	return enemy;
}


/**
 * @brief IsometricEnemy::attackedByPlayer
 * @param player
 */

void IsometricEnemy::attackedByPlayer(IsometricPlayer *player)
{
	if (!m_contactedPlayers.contains(player))
		m_contactedPlayers.append(player);

	setPlayer(player);
	rotateToPlayer(player);

	setHp(m_hp-1);

	m_hitTimer.invalidate();

	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection, m_currentAlteration);
	} else {
		jumpToSprite("hurt", m_currentDirection, m_currentAlteration);
	}
}




/**
 * @brief IsometricEnemy::entityWorldStep
 */

void IsometricEnemy::entityWorldStep()
{
	if (m_hp <= 0) {
		m_body->stop();
		jumpToSprite("die", m_currentDirection, m_currentAlteration);
		return;
	}

	if (m_movingDirection != Invalid)
		setCurrentDirection(m_movingDirection);


	//QPointF visiblePoint;
	IsometricPlayer *myPlayer = getVisibleEntity<QPointer<IsometricPlayer>>(m_body.get(), m_contactedPlayers,
																  TiledObjectBody::FixturePlayerBody
																  /*, &visiblePoint*/);


	float32 angle = 0.;
	bool isPursuit = false;

	if (myPlayer) {
		setPlayer(myPlayer);

		if (m_returnPathMotor)
			m_returnPathMotor->clearLastSeenPoint();

		qreal dist;

		rotateToPlayer(m_player, &angle, &dist);
		setPlayerDistance(dist);

		if (dist > m_metric.playerDistance) {
			if (m_metric.pursuitSpeed > 0) {		// Pursuit
				isPursuit = true;
			} else {								// No pursuit
				m_body->stop();
			}
		} else {
			m_body->stop();
		}
	} else {
		if (m_player) {
			if (m_returnPathMotor) {
				m_returnPathMotor->setLastSeenPoint(m_player->body()->bodyPosition());
				isPursuit = true;
			}

			setPlayer(nullptr);
			setPlayerDistance(-1.);
		}

		if (m_returnPathMotor && !m_returnPathMotor->isReturning()) {
			const auto &ptr = m_returnPathMotor->lastSeenPoint();

			if (ptr) {
				const qreal &dist = distanceToPoint(ptr.value());

				if (dist >= m_metric.playerDistance) {
					rotateToPoint(ptr.value(), &angle);
					isPursuit = true;
				} else {
					isPursuit = false;
				}
			}
		}
	}

	TiledObjectBody::setFixtureCollidesWithFlag(m_fixture.get(), TiledObjectBody::FixtureGround, !m_returnPathMotor);


	if (isPursuit) {
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

		// --- show path ---
		/*if (m_returnPathMotor) {
			QVariantList list;
			for (const auto &point : m_returnPathMotor->path())
				list.append(point);
			m_scene->setTestPoints(list);
		}*/
		// -----------


		enemyWorldStep();
		updateSprite();
		return;
	}

	if (m_returnPathMotor && !m_returnPathMotor->isReturning()) {
		m_returnPathMotor->finish(m_body.get());
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
		} else if (m_spriteHandler->currentSprite() != QStringLiteral("hurt")) {
			LOG_CWARNING("scene") << "HIT FIRST";
			m_player->hurt();
			jumpToSprite("hit", m_currentDirection, m_currentAlteration);
			m_hitTimer.start();
		}

		return false;
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
	LOG_CINFO("scene") << "ALIVE" << this << "ENEMYBODY";
	m_body->setBodyType(Box2DBody::Dynamic);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody));
	setSubZ(0.5);
}

void IsometricEnemy::onDead()
{
	LOG_CINFO("scene") << "DEAD";
	m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	setSubZ(0.0);
	m_sensorPolygon->setLength(10.);
}



/**
 * @brief IsometricEnemy::stepMotor
 */

void IsometricEnemy::stepMotor()
{
	if (!m_motor) {
		LOG_CERROR("game") << "Missing enemy motor:" << m_objectId.sceneId << m_objectId.id;
		return;
	}

	if (m_returnPathMotor) {
		if (m_returnPathMotor->isReturning() && !m_returnPathMotor->isReturnReady()) {
			m_body->stop();
			return;
		}
		if (m_metric.returnSpeed != 0. && m_returnPathMotor->step(m_metric.returnSpeed > 0. ? m_metric.returnSpeed : m_metric.speed)) {
			m_body->setLinearVelocity(maximizeSpeed(m_returnPathMotor->currentPosition() - m_body->bodyPosition()));
			return;
		} else {
			m_returnPathMotor.reset();
		}
	}

	m_motor->step(m_metric.speed);
	m_motor->updateBody(this, m_maximumSpeed);
}





/**
 * @brief IsometricEnemy::rotateToPlayer
 * @param player
 */

void IsometricEnemy::rotateToPlayer(IsometricPlayer *player, float32 *anglePtr, qreal *distancePtr)
{
	if (!player || !m_metric.rotateToPlayer)
		return;

	rotateToPoint(player->body()->bodyPosition(), anglePtr, distancePtr);
}



/**
 * @brief IsometricEnemy::rotateToPoint
 * @param point
 * @param anglePtr
 * @param vectorPtr
 */

void IsometricEnemy::rotateToPoint(const QPointF &point, float32 *anglePtr, qreal *distancePtr)
{
	const QPointF p = point - m_body->bodyPosition();

	float32 angle = atan2(-p.y(), p.x());

	setCurrentDirection(nearestDirectionFromRadian(angle));
	m_body->body()->SetTransform(m_body->body()->GetPosition(), angle);

	if (anglePtr)
		*anglePtr = angle;

	if (distancePtr)
		*distancePtr = QVector2D(p).length();
}



/**
 * @brief IsometricEnemy::angleToPoint
 * @param point
 * @return
 */

float32 IsometricEnemy::angleToPoint(const QPointF &point) const
{
	const QPointF p = point - m_body->bodyPosition();
	return atan2(-p.y(), p.x());
}



/**
 * @brief IsometricEnemy::distanceToPoint
 * @param point
 * @return
 */

qreal IsometricEnemy::distanceToPoint(const QPointF &point) const
{
	return QVector2D(point - m_body->bodyPosition()).length();
}







void IsometricEnemy::load()
{
	LOG_CDEBUG("scene") << "Test";

	m_body->setBodyType(Box2DBody::Dynamic);

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	setHp(5);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor));

	m_fixture->setProperty("enemy", QVariant::fromValue(this));
	//onAlive();

	TiledObjectSensorPolygon *p = addSensorPolygon(m_metric.sensorLength, m_metric.sensorRange);

	addTargetCircle();

	Q_ASSERT(p);

	p->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody));

	m_metric.speed = 2.;
	m_metric.returnSpeed = 3.;
	m_metric.pursuitSpeed = 3.;




	connect(p, &TiledObjectSensorPolygon::beginContact, this, [this](Box2DFixture *other) {
		IsometricPlayer *player = other->property("player").value<IsometricPlayer*>();

		LOG_CINFO("scene") << "CONTACT" << other << player;

		if (player && !m_contactedPlayers.contains(player))
			m_contactedPlayers.append(player);
	});

	connect(p, &TiledObjectSensorPolygon::endContact, this, [this](Box2DFixture *other) {
		IsometricPlayer *player = other->property("player").value<IsometricPlayer*>();

		LOG_CINFO("scene") << "CONTACT END" << other << player;

		if (player && m_contactedPlayers.contains(player))
			m_contactedPlayers.removeAll(player);
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
			"duration": 60,
			"loops": 1
		},

		{
			"name": "hurt",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 2048,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		},


		{
			"name": "die",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"x": 2048,
			"y": 0,
			"count": 8,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		}

	]
	})";


	IsometricObjectAlterableSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, path);

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.45*64);


	nextAlteration();

	//m_spriteSequence->setProperty("currentSprite", "idle-4");
	//jumpToSprite("idle-4");
}






/**
 * @brief IsometricObject::updateSprite
 */

void IsometricEnemy::updateSprite()
{
	if (m_hp <= 0)
		jumpToSprite("die", m_currentDirection, m_currentAlteration);
	else if (m_spriteHandler->currentSprite() == QStringLiteral("hit") || m_spriteHandler->currentSprite() == QStringLiteral("hurt"))
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
	motor->setWaitAtBegin(3500);
	motor->setWaitAtEnd(3500);
	if (direction == TiledPathMotor::Forward)
		motor->toBegin();
	else
		motor->toEnd();

	m_motor.reset(motor);

	onPathMotorLoaded(AbstractTiledMotor::PathMotor);
}


/**
 * @brief IsometricEnemyIface::loadFixPositionMotor
 * @param point
 * @param defaultAngle
 */

void IsometricEnemyIface::loadFixPositionMotor(const QPointF &point, const TiledObject::Direction &direction)
{
	Q_ASSERT(!m_motor.get());

	TiledFixPositionMotor *motor = new TiledFixPositionMotor;

	motor->setPoint(point);
	motor->setDirection(direction);

	m_motor.reset(motor);

	onPathMotorLoaded(AbstractTiledMotor::FixPositionMotor);
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



