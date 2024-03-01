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
#include "Logger.h"
#include "box2dfixture.h"
#include "tiledscene.h"

IsometricEntity::IsometricEntity(QQuickItem *parent)
	: IsometricObject(parent)
{
	load();
}


TiledPathMotor &IsometricEntity::motor()
{
	return m_motor;
}


/**
 * @brief IsometricEntity::worldStep
 */

void IsometricEntity::worldStep()
{
	Q_ASSERT(m_scene);

	m_isRun = m_scene->joystickState().distance >= 0.5;

	/*if (m_scene->joystickState().hasKeyboard || m_scene->joystickState().hasTouch)
		setCurrentDirection(directionFromAngle(m_scene->joystickState().angle));

	updateSprite();*/

	if (m_isRun) {
		if (m_scene->joystickState().angle > M_PI * 3/8 || m_scene->joystickState().angle < M_PI * -5/8)
			m_motor.setDirection(TiledPathMotor::Backward);
		else
			m_motor.setDirection(TiledPathMotor::Forward);

		m_motor.step(3.);

		setBodyCenterPoint(m_motor.currentPosition());
		m_body->setAwake(true);

		setCurrentDirection(nearestDirectionFromAngle(m_motor.currentAngleRadian()));
	}

	updateSprite();
}




/**
 * @brief IsometricEntity::bodyCenterPoint
 * @return
 */

QPointF IsometricEntity::bodyCenterPoint() const
{
	if (!m_fixture)
		return QRectF(x(), y(), width(), height()).center();

	QPointF p(m_fixture->x()+m_fixture->radius(),
			  m_fixture->y()+m_fixture->radius());

	p += position();

	return p;
}


/**
 * @brief IsometricEntity::setBodyCenterPoint
 * @param pos
 */

void IsometricEntity::setBodyCenterPoint(const QPointF &pos)
{
	if (!m_fixture)
		return setPosition(pos - QPointF(width()/2, height()/2));

	QPointF p(m_fixture->x()+m_fixture->radius(),
			  m_fixture->y()+m_fixture->radius());

	setPosition(pos - p);
}




void IsometricEntity::load()
{
	LOG_CDEBUG("scene") << "Test";

	setX(1800);
	setY(400);
	setZ(1);
	setDefaultZ(1);

	m_body->setBodyType(Box2DBody::Dynamic);

	m_fixture = new Box2DCircle();
	m_fixture->setRadius(15);
	m_fixture->setDensity(1);
	m_fixture->setRestitution(0);
	m_fixture->setFriction(1);
	m_fixture->setCategories(Box2DFixture::Category2);
	m_fixture->setCollidesWith(Box2DFixture::Category1);

	connect(parentItem(), &QQuickItem::widthChanged, this, &IsometricEntity::fixtureUpdate);
	connect(parentItem(), &QQuickItem::heightChanged, this, &IsometricEntity::fixtureUpdate);
	connect(m_fixture, &Box2DCircle::radiusChanged, this, &IsometricEntity::fixtureUpdate);

	m_body->addFixture(m_fixture);
	m_defaultFixture.reset(m_fixture);

	fixtureUpdate();
	bodyComplete();

	QString path = "/home/valaczka/Projektek/_callofsuli-resources/isometric/character/jonhdh/";

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

	connect(this, &IsometricObject::currentDirectionChanged, this, &IsometricEntity::updateSprite);

	nextAlteration();

	//m_spriteSequence->setProperty("currentSprite", "idle-4");
	//jumpToSprite("idle-4");
}


/**
 * @brief IsometricEntity::fixtureUpdate
 */

void IsometricEntity::fixtureUpdate()
{
	Q_ASSERT(m_fixture);

	m_fixture->setX((width() - 2*m_fixture->radius()) * 0.5);
	m_fixture->setY((height() - 2*m_fixture->radius()) * 0.8);
}







/**
 * @brief IsometricObject::updateSprite
 */

void IsometricEntity::updateSprite()
{
	if (m_currentDirection == Invalid)
		return;

	/*QPointF dir;

	const qreal radius = 7;

	if (m_isRun) {
		const qreal d = angleFromDirection(m_currentDirection);
		dir.setX(radius * cos(d));
		dir.setY(radius * -sin(d));
	}

	m_body->setLinearVelocity(dir);*/

	QString sprite = m_isRun ? getSpriteName("run", m_currentDirection, m_currentAlteration) :
							   getSpriteName("idle", m_currentDirection, m_currentAlteration);

	jumpToSprite(sprite);
}



void IsometricEntity::nextAlteration()
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

