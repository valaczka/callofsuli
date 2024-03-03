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
	updateSprite();
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

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	//mapObject->fixture()->setCategories(Box2DFixture::Category1);

	///setFixtureCenterVertical(0.8);

	//setFixtureCenterVertical(0.0);
	//setFixtureCenterHorizontal(0.0);

	addSensorPolygon(200);

	//QString path = "/home/valaczka/Projektek/_callofsuli-resources/isometric/character/avatar/character_w_clothes.png";

	QString path = ":/character_w_clothes.png";

	createVisual();
	setAvailableDirections(Direction_8);


	QString test = R"({
	"sprites": [
		{
			"name": "idle",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
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
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
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


	IsometricObjectSpriteList json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSpriteList(path, json);

	setWidth(128);
	setHeight(128);
}


/**
 * @brief IsometricPlayer::updateSprite
 */

void IsometricPlayer::updateSprite()
{
	QString sprite = m_movingDirection != Invalid ? getSpriteName("run", m_movingDirection) :
													getSpriteName("idle", m_currentDirection);

	jumpToSprite(sprite);
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
		/*QLineF l = QLineF::fromPolar(3, IsometricObject::radianToAngle(m_scene->joystickState().angle));
		m_body->setLinearVelocity(l.p2());*/
		QPointF dir;
		const qreal radius = 6.;

		//const qreal d = m_scene->joystickState().angle;
		const qreal d = m_scene->joystickState().angle;
		dir.setX(radius * cos(d));
		dir.setY(radius * -sin(d));
		m_body->setLinearVelocity(dir);
	} else {
		m_body->setLinearVelocity(QPointF{0,0});
	}
}
