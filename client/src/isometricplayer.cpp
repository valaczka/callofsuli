/*
 * ---- Call of Suli ----
 *
 * isometricplayer.cpp
 *
 * Created on: 2024. 03. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * IsometricPlayer
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

#include "isometricplayer.h"
#include "tiledscene.h"
#include "tiledspritehandler.h"

/**
 * @brief IsometricPlayer::IsometricPlayer
 * @param parent
 */

IsometricPlayer::IsometricPlayer(QQuickItem *parent)
	: IsometricCircleEntity(parent)
{

}


/**
 * @brief IsometricPlayer::createPlayer
 * @param parent
 * @return
 */

IsometricPlayer *IsometricPlayer::createPlayer(QQuickItem *parent)
{
	IsometricPlayer *player = nullptr;
	TiledObjectBase::createFromCircle<IsometricPlayer>(&player, QPointF{}, 30, nullptr, parent);

	if (player) {
		player->load();
	}

	return player;
}


/**
 * @brief IsometricPlayer::entityWorldStep
 */

void IsometricPlayer::entityWorldStep()
{
	rotateBody(directionToRadian(m_currentDirection));
	updateSprite();
}


/**
 * @brief IsometricPlayer::hurt
 */

void IsometricPlayer::hurt()
{
	setHp(m_hp-1);

	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection);
	} else {
		jumpToSprite("hurt", m_currentDirection);
	}
}


/**
 * @brief IsometricPlayer::onSceneConnected
 */

void IsometricPlayer::onSceneConnected()
{
	if (!m_scene)
		return;

	connect(m_scene, &TiledScene::joystickStateChanged, this, &IsometricPlayer::onJoystickStateChanged);

}


/**
 * @brief IsometricPlayer::load
 */

void IsometricPlayer::load()
{
	LOG_CDEBUG("scene") << "Test";

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);
	setHp(25);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setProperty("player", QVariant::fromValue(this));
	onAlive();

	addSensorPolygon(200);


	QString path = ":/character_w_clothes.png";

	createVisual();
	setAvailableDirections(Direction_8);


	QString test = R"({
	"sprites": [
		{
			"name": "idle",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 0,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 80
		},

		{
			"name": "run",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 512,
			"y": 0,
			"count": 8,
			"width": 128,
			"height": 128,
			"duration": 60
		},

		{
			"name": "hurt",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 2048,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 60
		},

		{
			"name": "die",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 2560,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		}
	]
	})";


	IsometricObjectSpriteList json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSpriteList(path, json);

	setWidth(128);
	setHeight(128);
	m_body->setBodyOffset(0, 0.4*64);
}


/**
 * @brief IsometricPlayer::updateSprite
 */

void IsometricPlayer::updateSprite()
{
	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection);
		return;
	}

	if (m_spriteHandler->currentSprite() == "hurt")
		jumpToSpriteLater("idle", m_currentDirection);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection);
	else
		jumpToSprite("idle", m_currentDirection);
}



/**
 * @brief IsometricPlayer::onAlive
 */

void IsometricPlayer::onAlive()
{
	LOG_CINFO("scene") << "ALIVE";
	m_body->setBodyType(Box2DBody::Dynamic);
	m_fixture->setCategories(Box2DFixture::Category2);
	setSubZ(0.5);
}


/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	LOG_CINFO("scene") << "DEAD";
	m_body->setBodyType(Box2DBody::Static);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	setSubZ(0.0);
}



/**
 * @brief IsometricPlayer::onJoystickStateChanged
 */

void IsometricPlayer::onJoystickStateChanged()
{
	if (!m_scene || m_scene->controlledItem() != this)
		return;

	if (m_scene->joystickState().hasKeyboard || m_scene->joystickState().hasTouch)
		setCurrentDirection(nearestDirectionFromRadian(m_scene->joystickState().angle));


	if (m_scene->joystickState().distance > 0.9) {
		const qreal radius = 6.;					/// speed
		m_body->setLinearVelocity(TiledObjectBase::toPoint(m_scene->joystickState().angle, radius));
	} else {
		m_body->setLinearVelocity(QPointF{0,0});
	}
}
