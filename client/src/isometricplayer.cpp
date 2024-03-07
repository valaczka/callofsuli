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
#include "tiledgame.h"

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

	if (m_hp < 3)
		m_currentAlteration = "none";
	else if (m_hp < 6)
		m_currentAlteration = "sword";

	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection, m_currentAlteration);
	} else if (m_spriteHandler->currentSprite() != "swing") {
		jumpToSprite("block", m_currentDirection, m_currentAlteration);
	}
}


/**
 * @brief IsometricPlayer::hit
 */

void IsometricPlayer::hit()
{
	jumpToSprite("swing", m_currentDirection, m_currentAlteration);
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


	QString path = ":/";

	createVisual();
	setAvailableDirections(Direction_8);


	QString test = R"({
					   "alterations": {
		"none": "char_none.png",
		"all": "char_all.png",
		"shield": "char_shield.png",
		"sword": "char_sword.png"
	},
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
			"name": "swing",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 1536,
			"y": 0,
			"count": 4,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		},

		{
			"name": "block",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 2048,
			"y": 0,
			"count": 2,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		},

		{
			"name": "die",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 2304,
			"y": 0,
			"count": 6,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		}
	]
	})";


	connect(m_fixture.get(), &Box2DCircle::beginContact, this, [this](Box2DFixture *other) {
		if (!other->categories().testFlag(Box2DFixture::Category4))
			return;

		TiledObjectBody *body = qobject_cast<TiledObjectBody*>(other->getBody());
		TiledObjectBase *base = body ? body->baseObject() : nullptr;
		TiledTransport *transport = m_game ? m_game->transportList().find(base) : nullptr;

		LOG_CINFO("scene") << "CONTACT" << other << transport << (transport ? transport->name() : nullptr);

		setCurrentTransport(transport);
	});


	connect(m_fixture.get(), &Box2DCircle::endContact, this, [this](Box2DFixture *other) {
		if (!other->categories().testFlag(Box2DFixture::Category4))
			return;

		LOG_CINFO("scene") << "CONTACT END" << other;

		setCurrentTransport(nullptr);
	});



	IsometricObjectAlterableSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, path);

	setWidth(128);
	setHeight(128);
	m_body->setBodyOffset(0, 0.4*64);

	m_currentAlteration = "all";
}


/**
 * @brief IsometricPlayer::updateSprite
 */

void IsometricPlayer::updateSprite()
{
	if (m_hp <= 0) {
		jumpToSprite("die", m_currentDirection, m_currentAlteration);
		return;
	}

	if (m_spriteHandler->currentSprite() == "block" || m_spriteHandler->currentSprite() == "swing")
		jumpToSpriteLater("idle", m_currentDirection, m_currentAlteration);
	else if (m_movingDirection != Invalid)
		jumpToSprite("run", m_movingDirection, m_currentAlteration);
	else
		jumpToSprite("idle", m_currentDirection, m_currentAlteration);
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
	Q_ASSERT(m_game);

	const auto &state = m_game->joystickState();

	if (state.hasKeyboard || state.hasTouch)
		setCurrentDirection(nearestDirectionFromRadian(state.angle));


	if (state.distance > 0.9) {
		const qreal radius = 6.;					/// speed
		m_body->setLinearVelocity(TiledObjectBase::toPoint(state.angle, radius));
	} else {
		m_body->setLinearVelocity(QPointF{0,0});
	}
}



/**
 * @brief IsometricPlayer::currentTransport
 * @return
 */

TiledTransport *IsometricPlayer::currentTransport() const
{
	return m_currentTransport;
}

void IsometricPlayer::setCurrentTransport(TiledTransport *newCurrentTransport)
{
	if (m_currentTransport == newCurrentTransport)
		return;
	m_currentTransport = newCurrentTransport;
	emit currentTransportChanged();
}
