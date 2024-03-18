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
#include "isometricenemy.h"
#include "tiledscene.h"
#include "tiledgame.h"
#include "application.h"


class IsometricPlayerPrivate {
private:
	/**
	 * @brief isEnemyContanctedAndReached
	 * @return
	 */

	bool isEnemyContanctedAndReached() const {
		return m_enemy && m_contactedEnemies.contains(m_enemy) && m_reachedEnemies.contains(m_enemy);
	}

	/**
	 * @brief contactedAndReachedEnemies
	 * @return
	 */

	QList<IsometricEnemy*> contactedAndReachedEnemies() const {
		QList<IsometricEnemy*> list;

		for (const auto &ptr : std::as_const(m_reachedEnemies)) {
			if (!ptr)
				continue;

			if (m_contactedEnemies.contains(ptr.get()))
				list.append(ptr.get());
		}

		return list;
	}

	QPointer<IsometricEnemy> m_enemy;
	QList<QPointer<IsometricEnemy>> m_contactedEnemies;
	QList<QPointer<IsometricEnemy>> m_reachedEnemies;

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
	m_inabilityTimer.setRemainingTime(-1);
}


/**
 * @brief IsometricPlayer::~IsometricPlayer
 */

IsometricPlayer::~IsometricPlayer()
{
	delete d;
}



/**
 * @brief IsometricPlayer::entityWorldStep
 */

void IsometricPlayer::entityWorldStep()
{
	IsometricEnemy *e = nullptr;

	if (d->isEnemyContanctedAndReached() && checkEntityVisibility(m_body.get(), d->m_enemy, TiledObjectBody::FixtureEnemyBody).has_value()) {
		e = d->m_enemy;
	} else {
		e = getVisibleEntity<IsometricEnemy*>(m_body.get(), d->contactedAndReachedEnemies(), TiledObjectBody::FixtureEnemyBody);
	}

	if (e != d->m_enemy) {
		if (d->m_enemy)
			d->m_enemy->setGlowEnabled(false);
		setEnemy(e);
		if (d->m_enemy) {
			d->m_enemy->setGlowColor(Qt::red);
			d->m_enemy->setGlowEnabled(true);
		}
	}


	rotateBody(m_currentAngle);
	updateSprite();
}








/**
 * @brief IsometricPlayer::initialize
 */

void IsometricPlayer::initialize()
{
	m_body->setBodyType(Box2DBody::Dynamic);

	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	m_fixture->setDensity(1);
	m_fixture->setFriction(1);
	m_fixture->setRestitution(0);

	createVisual();
	createMarkerItem();

	TiledObjectSensorPolygon *p = addSensorPolygon(m_sensorLength, m_sensorRange);

	addTargetCircle(m_targetCircleRadius);

	p->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody));

	connect(p, &TiledObjectSensorPolygon::beginContact, this, &IsometricPlayer::sensorBeginContact);
	connect(p, &TiledObjectSensorPolygon::endContact, this, &IsometricPlayer::sensorEndContact);

	connect(m_fixture.get(), &TiledObjectSensorPolygon::beginContact, this, &IsometricPlayer::fixtureBeginContact);
	connect(m_fixture.get(), &TiledObjectSensorPolygon::endContact, this, &IsometricPlayer::fixtureEndContact);

	load();
	onAlive();
}



/**
 * @brief IsometricPlayer::hasAbility
 * @return
 */

bool IsometricPlayer::hasAbility() const
{
	return m_inabilityTimer.isForever() || m_inabilityTimer.hasExpired();
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
 * @brief IsometricPlayer::onAlive
 */

void IsometricPlayer::onAlive()
{
	m_body->setBodyType(Box2DBody::Dynamic);
	m_body->setActive(true);
	m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport)
							   );
	m_sensorPolygon->setLength(m_sensorLength);
	m_sensorPolygon->setRange(m_sensorRange);
	setSubZ(0.5);
	emit becameAlive();
}




/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	m_sensorPolygon->setLength(10.);
	setSubZ(0.0);
	emit becameDead();
}



/**
 * @brief IsometricPlayer::startInabililty
 */

void IsometricPlayer::startInabililty()
{
	if (m_inabilityTime > 0)
		m_inabilityTimer.setRemainingTime(m_inabilityTime);
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
 * @brief IsometricPlayer::sensorBeginContact
 * @param other
 */

void IsometricPlayer::sensorBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

	if (!enemy)
		return;

	if (!d->m_contactedEnemies.contains(enemy))
		d->m_contactedEnemies.append(enemy);
}



/**
 * @brief IsometricPlayer::sensorEndContact
 * @param other
 */

void IsometricPlayer::sensorEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);
	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

	if (!enemy)
		return;

	d->m_contactedEnemies.removeAll(enemy);
}



/**
 * @brief IsometricPlayer::fixtureBeginContact
 * @param other
 */

void IsometricPlayer::fixtureBeginContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);

	if (!base)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy && !d->m_reachedEnemies.contains(enemy)) {
			d->m_reachedEnemies.append(enemy);
			onEnemyReached(enemy);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (!m_currentTransport && transport)
			setCurrentTransport(transport);
	}
}





/**
 * @brief IsometricPlayer::fixtureEndContact
 * @param other
 */

void IsometricPlayer::fixtureEndContact(Box2DFixture *other)
{
	TiledObjectBase *base = TiledObjectBase::getFromFixture(other);

	if (!base)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy) {
			d->m_reachedEnemies.removeAll(enemy);
			onEnemyLeft(enemy);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (m_currentTransport == transport && transport)
			setCurrentTransport(nullptr);
	}
}



/**
 * @brief IsometricPlayer::onJoystickStateChanged
 */

void IsometricPlayer::onJoystickStateChanged(const TiledGame::JoystickState &state)
{
	if (!isAlive())
		return;

	if (state.hasKeyboard || state.hasTouch) {
		setCurrentAngle(state.angle);
	}

	if (state.distance > 0.9) {
		m_body->setLinearVelocity(TiledObjectBase::toPoint(state.angle, m_speedLength));
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



/**
 * @brief IsometricPlayer::enemy
 * @return
 */

IsometricEnemy *IsometricPlayer::enemy() const
{
	return d->m_enemy.get();
}

void IsometricPlayer::setEnemy(IsometricEnemy *newEnemy)
{
	if (d->m_enemy == newEnemy)
		return;
	d->m_enemy = newEnemy;
	emit enemyChanged();
}
