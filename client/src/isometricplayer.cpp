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
#include "tiledpickableiface.h"
#include "tiledscene.h"
#include "tiledgame.h"
#include "tiledspritehandler.h"


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

			if (m_contactedEnemies.contains(ptr.data()))
				list.append(ptr.data());
		}

		return list;
	}


	/**
	 * @brief removeEnemy
	 * @param enemy
	 */

	void removeEnemy(IsometricEnemy *enemy) {
		m_contactedEnemies.removeAll(enemy);
		m_reachedEnemies.removeAll(enemy);
	}


	void clear() {
		///m_enemy = nullptr;
		m_contactedEnemies.clear();
		m_reachedEnemies.clear();
		m_destinationMotor.reset();
		m_destinationPoint.reset();
	}

	QPointer<IsometricEnemy> m_enemy;
	QList<QPointer<IsometricEnemy>> m_contactedEnemies;
	QList<QPointer<IsometricEnemy>> m_reachedEnemies;

	std::unique_ptr<TiledPathMotor> m_destinationMotor;
	std::optional<QPointF> m_destinationPoint;

	friend class IsometricPlayer;
};



/**
 * @brief IsometricPlayer::IsometricPlayer
 * @param parent
 */

IsometricPlayer::IsometricPlayer(TiledScene *scene)
	: IsometricEntity(scene)
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
 * @brief IsometricPlayer::entityWorldStep
 */

void IsometricPlayer::entityWorldStep(const qreal &factor)
{
	IsometricEnemy *e = nullptr;

	if (d->isEnemyContanctedAndReached() && checkEntityVisibility(this, d->m_enemy, TiledObjectBody::FixtureEnemyBody, nullptr).has_value()) {
		e = d->m_enemy;
	} else {
		e = getVisibleEntity<IsometricEnemy*>(this, d->contactedAndReachedEnemies(), TiledObjectBody::FixtureEnemyBody, nullptr);
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


	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		//clearDestinationPoint();
		stop();
	} else if (!hasAbility()) {
		stop();
	} else {
		if (d->m_destinationPoint) {
			QLineF line(QPointF{0,0}, d->m_destinationPoint.value() - bodyPosition());
			if (line.length() >= m_speedRunLength*factor) {
				line.setLength(m_speedRunLength*factor);
				setSpeed(line.p2());				// TiledObjectBase::toPoint()
			} else if (line.length() >= m_speedLength*factor) {
				line.setLength(m_speedLength*factor);
				setSpeed(line.p2());
			} else {
				stop();
				emplace(d->m_destinationPoint.value());
				clearDestinationPoint();
				atDestinationPointEvent();
			}
		} else if (d->m_destinationMotor) {
			if (d->m_destinationMotor->atEnd()) {
				stop();
				clearDestinationPoint();
				atDestinationPointEvent();
			} else if (const QPolygonF &polygon = d->m_destinationMotor->polygon(); !polygon.isEmpty()) {
				const float distance = QVector2D(bodyPosition()).distanceToPoint(QVector2D(polygon.last()));

				if (distance >= m_speedRunLength*factor*10.) {				// Hogy a végén szépen lassan gyalogoljon csak
					d->m_destinationMotor->updateBody(this, m_speedRunLength*factor, m_game->tickTimer());
				} else {
					d->m_destinationMotor->updateBody(this, m_speedLength*factor, m_game->tickTimer());
				}

				setCurrentAngle(d->m_destinationMotor->currentAngleRadian());
			} else {
				stop();
				clearDestinationPoint();
			}
		} else {
			QLineF line(0,0, m_currentVelocity.x(), m_currentVelocity.y());
			line.setLength(line.length()*factor);
			setSpeed(line.p2());
		}
	}

	updateSprite();
}








/**
 * @brief IsometricPlayer::initialize
 */

void IsometricPlayer::initialize()
{
	setZ(1);
	setDefaultZ(1);
	setSubZ(0.5);

	b2::Body::Params bParams;
	bParams.type = b2BodyType::b2_dynamicBody;
	bParams.fixedRotation = true;

	b2::Shape::Params params;
	params.density = 1.f;
	params.friction = 1.f;
	params.restitution = 0.f;
	/*params.filter = TiledObjectBody::getFilter(FixtureTarget,
											   FixtureTarget | FixturePlayerBody | FixtureEnemyBody | FixtureGround);*/

	createFromCircle({0.f, 0.f}, 30., nullptr, bParams, params);

	createVisual();

	createMarkerItem();

	/*TiledObjectSensorPolygon *p = addSensorPolygon(m_sensorLength, m_sensorRange);

	addTargetCircle(m_targetCircleRadius);

	p->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
					   TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable)
					   );

	p->virtualCircle()->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureVirtualCircle));
	p->virtualCircle()->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTrigger) |
										TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable) |
										TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer)
										);

	connect(p, &TiledObjectSensorPolygon::beginContact, this, &IsometricPlayer::sensorBeginContact);
	connect(p, &TiledObjectSensorPolygon::endContact, this, &IsometricPlayer::sensorEndContact);

	connect(m_fixture.get(), &TiledObjectSensorPolygon::beginContact, this, &IsometricPlayer::fixtureBeginContact);
	connect(m_fixture.get(), &TiledObjectSensorPolygon::endContact, this, &IsometricPlayer::fixtureEndContact);*/

	load();
	onAlive();
}



/**
 * @brief IsometricPlayer::hasAbility
 * @return
 */

bool IsometricPlayer::hasAbility()
{
	if (!m_game->tickTimer() || m_inabilityTimer < 0)
		return true;

	if (m_inabilityTimer < m_game->tickTimer()->currentTick()) {
		m_inabilityTimer = -1;
		return true;
	}

	return false;
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
 * @brief IsometricPlayer::removeEnemy
 * @param enemy
 */

void IsometricPlayer::removeEnemy(IsometricEnemy *enemy)
{
	d->removeEnemy(enemy);
}



/**
 * @brief IsometricPlayer::onAlive
 */

void IsometricPlayer::onAlive()
{
	setBodyEnabled(true);

	/*m_fixture->setCategories(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePlayerBody));
	m_fixture->setCollidesWith(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureGround) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureSensor) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport) |
							   TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer)
							   );

	m_sensorPolygon->setLength(m_sensorLength);
	m_sensorPolygon->setRange(m_sensorRange);*/
	setSubZ(0.5);
	emit becameAlive();
}




/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	setBodyEnabled(false);

	/*m_body->setBodyType(Box2DBody::Static);
	m_body->setActive(false);
	m_fixture->setCategories(Box2DFixture::None);
	m_fixture->setCollidesWith(Box2DFixture::None);
	m_sensorPolygon->setLength(10.);*/
	setSubZ(0.0);
	setCurrentVelocity(QPointF{});

	m_game->onPlayerDead(this);

	emit becameDead();
}



/**
 * @brief IsometricPlayer::startInability
 */

void IsometricPlayer::startInability()
{
	if (m_inabilityTime > 0 && m_game->tickTimer()) {
		m_inabilityTimer = m_game->tickTimer()->currentTick() + m_inabilityTime;
		//clearDestinationPoint();
	}
}


/**
 * @brief IsometricPlayer::startInability
 * @param msec
 */

void IsometricPlayer::startInability(const int &msec)
{
	if (m_inabilityTime > 0 && m_game->tickTimer()) {
		m_inabilityTimer = m_game->tickTimer()->currentTick() + msec;
		//clearDestinationPoint();
	}
}






/**
 * @brief IsometricPlayer::protectWeapon
 * @param list
 * @param weaponType
 * @return
 */

bool IsometricPlayer::protectWeapon(TiledWeaponList *weaponList, const TiledWeapon::WeaponType &weaponType)
{
	Q_ASSERT(weaponList);

	if (weaponList->empty())
		return false;

	std::vector<TiledWeapon*> list;

	// Ordering weapons by enum

	for (TiledWeapon *weapon : std::as_const(*weaponList)) {
		if (weapon->canProtect(weaponType))
			list.push_back(weapon);
	}

	if (list.empty())
		return false;

	std::sort(list.begin(), list.end(), [](TiledWeapon *w1, TiledWeapon *w2) {
		return w1->weaponType() < w2->weaponType();
	});

	for (TiledWeapon *w : std::as_const(list))
		if (w->protect(weaponType))
			return true;

	return false;
}


/**
 * @brief IsometricPlayer::reachedEnemies
 * @return
 */

QList<IsometricEnemy *> IsometricPlayer::reachedEnemies() const
{
	if (d->m_reachedEnemies.isEmpty())
		return {};

	QList<IsometricEnemy *> list;
	list.reserve(d->m_reachedEnemies.size());

	for (const auto &ptr : d->m_reachedEnemies)
		list.append(ptr.data());

	return list;
}



/**
 * @brief IsometricPlayer::contactedAndReachedEnemies
 * @return
 */

QList<IsometricEnemy *> IsometricPlayer::contactedAndReachedEnemies() const
{
	return d->contactedAndReachedEnemies();
}


/**
 * @brief IsometricPlayer::clearData
 */

void IsometricPlayer::clearData()
{
	d->clear();
}


/**
 * @brief IsometricPlayer::isLocked
 * @return
 */

bool IsometricPlayer::isLocked() const
{
	return m_isLocked;
}

void IsometricPlayer::setIsLocked(bool newIsLocked)
{
	if (m_isLocked == newIsLocked)
		return;
	m_isLocked = newIsLocked;
	emit isLockedChanged();
	if (m_isLocked) {
		stop();
		clearDestinationPoint();
	}
}




/**
 * @brief IsometricPlayer::sensorBeginContact
 * @param other
 */
/*
void IsometricPlayer::sensorBeginContact(Box2DFixture *other)
{
	TiledObject *base = TiledObject::getFromFixture(other);

	if (!base)
		return;

	if (IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base)) {
		if (!d->m_contactedEnemies.contains(enemy))
			d->m_contactedEnemies.append(QPointer(enemy));
	}
}


void IsometricPlayer::sensorEndContact(Box2DFixture *other)
{
	TiledObject *base = TiledObject::getFromFixture(other);
	IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

	if (!base)
		return;

	if (enemy)
		d->m_contactedEnemies.removeAll(QPointer(enemy));
}

void IsometricPlayer::fixtureBeginContact(Box2DFixture *other)
{
	TiledObject *base = TiledObject::getFromFixture(other);

	if (!base)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy && !d->m_reachedEnemies.contains(enemy)) {
			d->m_reachedEnemies.append(QPointer(enemy));
			onEnemyReached(enemy);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (!m_currentTransport && transport) {
			setCurrentTransport(transport);
			m_currentTransportBase = base;
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable))) {
		TiledObject *object = qobject_cast<TiledObject*>(base);
		TiledPickableIface *iface = dynamic_cast<TiledPickableIface*>(base);
		if (object && iface) {
			onPickableReached(object);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer))) {
		TiledContainer *container = base->property("tiledContainer").value<TiledContainer*>();

		if (!m_currentContainer && container)
			setCurrentContainer(container);
	}
}




void IsometricPlayer::fixtureEndContact(Box2DFixture *other)
{
	TiledObject *base = TiledObject::getFromFixture(other);

	if (!base)
		return;

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTarget)) ||
			other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureEnemyBody))) {

		IsometricEnemy *enemy = qobject_cast<IsometricEnemy*>(base);

		if (enemy) {
			d->m_reachedEnemies.removeAll(QPointer(enemy));
			onEnemyLeft(enemy);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (m_currentTransport == transport && transport) {
			setCurrentTransport(nullptr);
			m_currentTransportBase = nullptr;
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixturePickable))) {
		TiledObject *object = qobject_cast<TiledObject*>(base);
		TiledPickableIface *iface = dynamic_cast<TiledPickableIface*>(base);
		if (object && iface) {
			//removePickable(object);
			onPickableLeft(object);
		}
	}

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureContainer))) {
		TiledContainer *container = base->property("tiledContainer").value<TiledContainer*>();

		if (m_currentContainer == container && container)
			setCurrentContainer(nullptr);
	}
}

*/


/**
 * @brief IsometricPlayer::currentContainer
 * @return
 */

TiledContainer *IsometricPlayer::currentContainer() const
{
	return m_currentContainer;
}

void IsometricPlayer::setCurrentContainer(TiledContainer *newCurrentContainer)
{
	if (m_currentContainer == newCurrentContainer)
		return;
	m_currentContainer = newCurrentContainer;
	emit currentContainerChanged();
}





/**
 * @brief IsometricPlayer::currentVelocity
 * @return
 */

QPointF IsometricPlayer::currentVelocity() const
{
	return m_currentVelocity;
}

void IsometricPlayer::setCurrentVelocity(QPointF newCurrentVelocity)
{
	if (m_currentVelocity == newCurrentVelocity)
		return;
	m_currentVelocity = newCurrentVelocity;
	emit currentVelocityChanged();
}



/**
 * @brief IsometricPlayer::onJoystickStateChanged
 */

void IsometricPlayer::onJoystickStateChanged(const TiledGame::JoystickState &state)
{
	if (!isAlive())
		return;

	clearDestinationPoint();

	if (m_isLocked) {
		stop();
		return;
	}

	if (state.hasKeyboard || state.hasTouch) {
		setCurrentAngle(state.angle);
	}

	if (state.distance > 0.95) {
		setCurrentVelocity(TiledObject::toPoint(state.angle, m_speedRunLength));
	} else if (state.distance > 0.45) {
		setCurrentVelocity(TiledObject::toPoint(state.angle, m_speedLength));
	} else {
		setCurrentVelocity({0.,0.});
	}
}



/**
 * @brief IsometricPlayer::setDestinationPoint
 * @param polygon
 */

void IsometricPlayer::setDestinationPoint(const QPolygonF &polygon)
{
	if (!isAlive())
		return;

	if (m_isLocked) {
		clearDestinationPoint();
		stop();
		return;
	}

	if (polygon.size() == 1)
		return setDestinationPoint(polygon.first());

	d->m_destinationMotor.reset(new TiledPathMotor(polygon));
	d->m_destinationPoint.reset();
}



/**
 * @brief IsometricPlayer::setDestinationPoint
 * @param point
 */

void IsometricPlayer::setDestinationPoint(const QPointF &point)
{
	if (!isAlive())
		return;

	if (m_isLocked) {
		clearDestinationPoint();
		stop();
		return;
	}

	QLineF l(bodyPosition(), point);
	setCurrentAngle(toRadian(l.angle()));

	d->m_destinationPoint = point;
	d->m_destinationMotor.reset();
}


/**
 * @brief IsometricPlayer::clearDestinationPoint
 */

void IsometricPlayer::clearDestinationPoint()
{
	d->m_destinationMotor.reset();
	d->m_destinationPoint.reset();
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
	return d->m_enemy.data();
}

void IsometricPlayer::setEnemy(IsometricEnemy *newEnemy)
{
	if (d->m_enemy == newEnemy)
		return;
	d->m_enemy = newEnemy;
	emit enemyChanged();
}
