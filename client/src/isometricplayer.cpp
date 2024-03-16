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
#include "isometricbullet.h"
#include "isometricenemy.h"
#include "tiledscene.h"
#include "tiledspritehandler.h"
#include "tiledgame.h"
#include "application.h"


class IsometricPlayerPrivate {
private:
	QPointer<IsometricEnemy> m_enemy;
	QList<QPointer<IsometricEnemy>> m_contactedEnemies;

	friend class IsometricPlayer;
};



/**
 * @brief IsometricPlayer::IsometricPlayer
 * @param parent
 */

IsometricPlayer::IsometricPlayer(QQuickItem *parent)
	: IsometricCircleEntity(parent)
	, d(new IsometricPlayerPrivate)
{

}


/**
 * @brief IsometricPlayer::~IsometricPlayer
 */

IsometricPlayer::~IsometricPlayer()
{
	delete d;
}


/**
 * @brief IsometricPlayer::createPlayer
 * @param parent
 * @return
 */

IsometricPlayer *IsometricPlayer::createPlayer(TiledGame *game, TiledScene *scene)
{
	IsometricPlayer *player = nullptr;
	TiledObjectBase::createFromCircle<IsometricPlayer>(&player, QPointF{}, 50, nullptr, game);

	if (player) {
		player->setParent(game);
		player->setGame(game);
		player->setScene(scene);
		player->load();
	}

	return player;
}


/**
 * @brief IsometricPlayer::entityWorldStep
 */

void IsometricPlayer::entityWorldStep()
{
	IsometricEnemy *e = getVisibleEntity<QPointer<IsometricEnemy>>(m_body.get(), d->m_contactedEnemies, TiledObjectBody::FixtureEnemyBody);

	if (e != d->m_enemy) {
		if (d->m_enemy)
			d->m_enemy->setGlowEnabled(false);
		d->m_enemy = e;
		if (d->m_enemy) {
			d->m_enemy->setGlowColor(Qt::red);
			d->m_enemy->setGlowEnabled(true);
		}
	}


	rotateBody(m_currentAngle);
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
	} else if (m_spriteHandler->currentSprite() != "swing" && m_spriteHandler->currentSprite() != "shoot") {
		jumpToSprite("block", m_currentDirection, m_currentAlteration);
	}
}


/**
 * @brief IsometricPlayer::hit
 */

void IsometricPlayer::hit()
{
	jumpToSprite("swing", m_currentDirection, m_currentAlteration);

	LOG_CTRACE("scene") << "SWORD" << d->m_enemy;

	if (d->m_enemy)
		d->m_enemy->attackedByPlayer(this);
}



/**
 * @brief IsometricPlayer::shot
 */

void IsometricPlayer::shot()
{
	IsometricBullet *bullet = IsometricBullet::createBullet(m_game, m_scene);

	if (!bullet) {
		LOG_CERROR("scene") << "Bullet error";
		return;
	}

	jumpToSprite("shoot", m_currentDirection, m_currentAlteration);

	m_scene->appendToObjects(bullet);

	bullet->shot(m_body->bodyPosition(), m_currentAngle);
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
	setMaxHp(25);
	setHp(25);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);
	m_fixture->setProperty("player", QVariant::fromValue(this));
	onAlive();

	auto p = addSensorPolygon(200, M_PI_2);

	p->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody));


	QString path = ":/";

	createVisual();
	createMarkerItem();
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
			"duration": 120
		},

		{
			"name": "run",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 512,
			"y": 0,
			"count": 8,
			"width": 128,
			"height": 128,
			"duration": 66
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
			"x": 2304,
			"y": 0,
			"count": 2,
			"width": 128,
			"height": 128,
			"duration": 60,
			"loops": 1
		},

		{
			"name": "shoot",
			"directions": [ 270, 315, 360, 45, 90, 135, 180, 225 ],
			"x": 3584,
			"y": 0,
			"count": 4,
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
			"duration": 100,
			"loops": 1
		}
	]
	})";


	connect(p, &TiledObjectSensorPolygon::beginContact, this, [this](Box2DFixture *other) {
		LOG_CTRACE("scene") << "------contact" << other << other->categories() << other->property("enemy");
		if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

			IsometricEnemy *enemy = other->property("enemy").value<IsometricEnemy*>();

			if (enemy && !d->m_contactedEnemies.contains(enemy))
				d->m_contactedEnemies.append(enemy);

			LOG_CINFO("scene") << "ENEMY CONTACT" << enemy;

			return;
		}
	});

	connect(p, &TiledObjectSensorPolygon::endContact, this, [this](Box2DFixture *other) {
		if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

			IsometricEnemy *enemy = other->property("enemy").value<IsometricEnemy*>();

			if (enemy)
				d->m_contactedEnemies.removeAll(enemy);

			LOG_CTRACE("scene") << "ENEMY CONTACT END" << enemy;

			return;
		}
	});

	connect(m_fixture.get(), &Box2DCircle::beginContact, this, [this](Box2DFixture *other) {
		if (!other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport)))
			return;


		TiledObjectBody *body = qobject_cast<TiledObjectBody*>(other->getBody());
		TiledObjectBase *base = body ? body->baseObject() : nullptr;
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		LOG_CINFO("scene") << "CONTACT" << other << transport << (transport ? transport->name() : nullptr);

		setCurrentTransport(transport);
	});


	connect(m_fixture.get(), &Box2DCircle::endContact, this, [this](Box2DFixture *other) {
		if (!other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport)))
			return;

		LOG_CINFO("scene") << "CONTACT END" << other;

		setCurrentTransport(nullptr);
	});



	IsometricObjectAlterableSprite json;
	json.fromJson(QJsonDocument::fromJson(test.toUtf8()).object());

	appendSprite(json, path);

	setWidth(128);
	setHeight(128);
	setBodyOffset(0, 0.4*64);

	m_currentAlteration = "all";
}



/**
 * @brief IsometricPlayer::currentAngle
 * @return
 */

qreal IsometricPlayer::currentAngle() const
{
	return m_currentAngle;
}

void IsometricPlayer::setCurrentAngle(qreal newCurrentAngle)
{
	if (qFuzzyCompare(m_currentAngle, newCurrentAngle))
		return;
	m_currentAngle = newCurrentAngle;
	emit currentAngleChanged();

	setCurrentDirection(nearestDirectionFromRadian(m_currentAngle));
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

	if (m_spriteHandler->currentSprite() == "block" || m_spriteHandler->currentSprite() == "swing" ||
			m_spriteHandler->currentSprite() == "shoot")
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
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport)
							   );
	setSubZ(0.5);
}


/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	LOG_CINFO("scene") << "DEAD";
	m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	setSubZ(0.0);
}


/**
 * @brief IsometricPlayer::createMarkerItem
 */

void IsometricPlayer::createMarkerItem()
{
	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/TiledPlayerMarker.qml"), this);

	QQuickItem *item = qobject_cast<QQuickItem*>(component.createWithInitialProperties(
												 QVariantMap{
													 { QStringLiteral("target"), QVariant::fromValue(this) }
												 }));

	if (!item) {
		LOG_CERROR("scene") << "TiledPlayerMarker error" << component.errorString();
		return;
	}

	item->setParent(this);
}



/**
 * @brief IsometricPlayer::onJoystickStateChanged
 */

void IsometricPlayer::onJoystickStateChanged(const TiledGame::JoystickState &state)
{
	if (state.hasKeyboard || state.hasTouch) {
		setCurrentAngle(state.angle);
	}

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
