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


/**
 * @brief IsometricEnemy::IsometricEnemy
 * @param parent
 */

IsometricEnemy::IsometricEnemy(QQuickItem *parent)
	: IsometricCircleEntity(parent)
	//, IsometricEnemyIface()
{

}

IsometricEnemy *IsometricEnemy::createEnemy(QQuickItem *parent)
{
	IsometricEnemy *enemy = nullptr;
	TiledObjectBase::createFromCircle<IsometricEnemy>(&enemy, QPointF{}, 30, nullptr, parent);

	if (enemy) {
		enemy->load();
	}

	return enemy;
}



void IsometricEnemy::entityWorldStep()
{
	return;	/////


	if (m_movingDirection != Invalid)
		setCurrentDirection(m_movingDirection);

	if (!m_pathMotor) {
		LOG_CERROR("scene") << "NO MOTOR";
		return;
	}

	//if (!m_scene->joystickState().hasKeyboard && !m_scene->joystickState().hasTouch) {

		m_pathMotor->step(1.5);

		if (m_pathMotor->direction() == TiledPathMotor::Backward && m_pathMotor->atBegin())
			m_pathMotor->setDirection(TiledPathMotor::Forward);
		else if (m_pathMotor->direction() == TiledPathMotor::Forward && m_pathMotor->atEnd())
			m_pathMotor->setDirection(TiledPathMotor::Backward);

		//setBodyCenterPoint(m_pathMotor->currentPosition());

		QPointF nextPoint = m_pathMotor->currentPosition() /*- bodyCenterPoint()*/;
		if (nextPoint.x() > 10)
			nextPoint.setX(10);
		if (nextPoint.x() < -10)
			nextPoint.setX(-10);

		if (nextPoint.y() > 10)
			nextPoint.setY(10);
		if (nextPoint.y() < -10)
			nextPoint.setY(-10);

		m_body->setLinearVelocity(nextPoint);
	//}

	updateSprite();
}







void IsometricEnemy::load()
{
	LOG_CDEBUG("scene") << "Test";

	m_body->setBodyType(Box2DBody::Dynamic);

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.2);

	///setFixtureCenterVertical(0.8);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setCollidesWith(Box2DFixture::Category1);

	TiledObjectSensorPolygon *p = addSensorPolygon(50);

	Q_ASSERT(p);

	static bool b = false;

	if (!b) {
	connect(p, &TiledObjectSensorPolygon::beginContact, this, [](Box2DFixture *other) {
		if (!other->isSensor())
		LOG_CINFO("scene") << "CONTACT" << other;
	});

	connect(p, &TiledObjectSensorPolygon::endContact, this, [](Box2DFixture *other) {
		if (!other->isSensor())
		LOG_CINFO("scene") << "CONTACT END" << other;
	});

	b= true;
	}

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
			"frameX": 0,
			"frameY": 0,
			"frameCount": 4,
			"frameWidth": 128,
			"frameHeight": 128,
			"frameDuration": 80,
			"to": { "idle": 1 }
		},

		{
			"name": "run",
			"directions": [ 180, 135, 90, 45, 360, 315, 270, 225 ],
			"frameX": 512,
			"frameY": 0,
			"frameCount": 8,
			"frameWidth": 128,
			"frameHeight": 128,
			"frameDuration": 60,
			"to": { "run": 1 }
		}
	]
	})";


	IsometricObjectAlterableSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, path);

	setWidth(128);
	setHeight(128);


	nextAlteration();

	//m_spriteSequence->setProperty("currentSprite", "idle-4");
	//jumpToSprite("idle-4");
}






/**
 * @brief IsometricObject::updateSprite
 */

void IsometricEnemy::updateSprite()
{
	QString sprite = m_movingDirection != Invalid ? getSpriteName("run", m_movingDirection, m_currentAlteration) :
													getSpriteName("idle", m_currentDirection, m_currentAlteration);

	jumpToSprite(sprite);
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

void IsometricEnemy::loadPathMotor(const QPolygonF &polygon, const TiledPathMotor::Direction &direction)
{
	if (!m_pathMotor)
		m_pathMotor.reset(new TiledPathMotor);

	m_pathMotor->setPolygon(polygon);
	m_pathMotor->setDirection(direction);
	if (direction == TiledPathMotor::Forward)
		m_pathMotor->toBegin();
	else
		m_pathMotor->toEnd();

	//setBodyCenterPoint(m_pathMotor->currentPosition());
}


/**
 * @brief IsometricEnemyIface::pathMotor
 * @return
 */

TiledPathMotor*IsometricEnemy::pathMotor() const
{
	return m_pathMotor.get();
}
