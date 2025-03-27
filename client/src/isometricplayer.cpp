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
public:
	enum EnemyFlag {
		Invalid = 0,
		Near = 1,
		Visible = 2,
		CanShot = 4,
		CanHit = 8
	};

	Q_DECLARE_FLAGS(EnemyFlags, EnemyFlag);


private:
	IsometricPlayerPrivate(IsometricPlayer *player)
		: m_player(player)
	{}


	struct EnemyData {
		IsometricEnemy *enemy = nullptr;
		EnemyFlags flags = Invalid;
		float distance = 0.;
	};

	std::vector<EnemyData> m_enemies;
	IsometricEnemy *m_currentEnemy = nullptr;

	std::vector<EnemyData>::iterator findEnemy(IsometricEnemy *enemy) {
		return std::find_if(m_enemies.begin(), m_enemies.end(), [enemy](const EnemyData &d) { return d.enemy == enemy; });
	}

	std::vector<EnemyData>::iterator removeEnemy(std::vector<EnemyData>::iterator it) {
		if (m_currentEnemy == it->enemy)
			m_currentEnemy = nullptr;

		return m_enemies.erase(it);
	}

	void removeEnemy(IsometricEnemy *enemy) {
		auto it = findEnemy(enemy);
		if (it != m_enemies.end())
			removeEnemy(it);
	}

	bool setEnemyFlag(IsometricEnemy *enemy, const EnemyFlag &flag, const bool &on = true) {
		bool newFlag = true;

		auto it = findEnemy(enemy);
		if (it != m_enemies.end()) {
			if (it->flags.testFlag(flag) == on)
				newFlag = false;
			else
				it->flags.setFlag(flag, on);

			if (it->flags == Invalid)
				removeEnemy(it);
		} else {
			EnemyFlags f;
			f.setFlag(flag, on);

			if (f != Invalid)
				m_enemies.emplace_back(enemy, f);
		}

		return newFlag;
	}

	void setEnemyDistance(IsometricEnemy *enemy, const float &dist) {
		auto it = findEnemy(enemy);
		if (it != m_enemies.end()) {
			it->distance = dist;
		}
	}

	void setEnemy(IsometricEnemy *enemy, const EnemyFlags &flags, const float &dist) {
		if (flags == Invalid) {
			removeEnemy(enemy);
			return;
		}

		auto it = findEnemy(enemy);
		if (it != m_enemies.end()) {
			it->flags |= flags;
			it->distance = dist;
		} else
			m_enemies.emplace_back(enemy, flags, dist);
	}

	void selectEnemyForShot(const float &range = 0.) {
		std::vector<EnemyData>::const_iterator it = m_enemies.cend();

		for (auto d = m_enemies.cbegin(); d != m_enemies.cend(); ++d) {
			if ((d->flags & (Visible|CanShot)) != (Visible|CanShot) || (range > 0. && d->distance > range))
				continue;

			if (it == m_enemies.cend() || d->distance < it->distance)
				it = d;
		}

		m_currentEnemy = (it == m_enemies.cend() ? nullptr : it->enemy);
	}

	void selectEnemyForHit() {
		std::vector<EnemyData>::const_iterator it = m_enemies.cend();

		for (auto d = m_enemies.cbegin(); d != m_enemies.cend(); ++d) {
			if ((d->flags & (Visible|CanHit))  != (Visible|CanHit))
				continue;

			if (it == m_enemies.cend() || d->distance < it->distance)
				it = d;
		}

		m_currentEnemy = (it == m_enemies.cend() ? nullptr : it->enemy);
	}


	void updateFromRayCast(const TiledReportedFixtureMap &map) {
		for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
			const bool inMap = map.contains(it->enemy);

			if (it->flags.testFlag(CanShot) && !it->flags.testFlag(CanHit) && !inMap)
				it = removeEnemy(it);
			else
				++it;
		}

		for (const TiledReportedFixture &f : map) {
			if (IsometricEnemy *e = dynamic_cast<IsometricEnemy*>(f.body))
				setEnemy(e, EnemyFlags(Visible|CanShot), m_player->distanceToPoint(f.point));
		}
	}


	void checkVisibility() {
		for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
			if (!it->enemy->isAlive() || it->enemy->isSleeping()) {
				it = removeEnemy(it);
				continue;
			}

			const TiledReportedFixtureMap map = m_player->rayCast(it->enemy->bodyPosition(), TiledObjectBody::FixtureEnemyBody, true);

			auto mapIt = map.find(it->enemy);

			const bool inMap = mapIt != map.cend();

			it->flags.setFlag(Visible, inMap);

			if (inMap) {
				it->distance = m_player->distanceToPoint(mapIt->point);
			} if (!inMap) {
				it->flags.setFlag(CanHit, false);
				it->flags.setFlag(CanShot, false);
			}


			if (it->flags == Invalid)
				it = removeEnemy(it);
			else
				++it;
		}
	}

	std::unique_ptr<TiledPathMotor> m_destinationMotor;
	std::optional<QVector2D> m_destinationPoint;
	IsometricPlayer *const m_player;

	friend class IsometricPlayer;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(IsometricPlayerPrivate::EnemyFlags);


/**
 * @brief IsometricPlayer::IsometricPlayer
 * @param parent
 */

IsometricPlayer::IsometricPlayer(TiledScene *scene)
	: IsometricEntity(scene)
	, d(new IsometricPlayerPrivate(this))
{
}


/**
 * @brief IsometricPlayer::~IsometricPlayer
 */

IsometricPlayer::~IsometricPlayer()
{
	delete d;
	d = nullptr;
}






/**
 * @brief IsometricPlayer::initialize
 */

void IsometricPlayer::initialize()
{
	setDefaultZ(1);
	setSubZ(0.5);

	m_speedLength = 108;
	m_speedRunLength = 210;

	//setSensorPolygon(m_sensorLength, M_PI*2./3., FixtureTarget | FixturePickable);
	//addVirtualCircle(FixtureTrigger | FixturePickable | FixtureContainer, m_sensorLength);
	addTargetCircle(m_targetCircleRadius);

	createVisual();

	m_visualItem->setZ(1);

	createMarkerItem();

	load();
	onAlive();
}


/**
 * @brief IsometricPlayer::setVirtualCircle
 * @param on
 */

void IsometricPlayer::setVirtualCircle(const bool &on)
{
	if (on)
		addVirtualCircle(FixtureTrigger | FixturePickable | FixtureContainer, m_sensorLength);
	else
		removeVirtualCircle();
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

	setSubZ(0.5);
	emit becameAlive();
}




/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	setBodyEnabled(false);

	setSubZ(0.0);
	setCurrentVelocity({});

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
	}
}



/**
 * @brief IsometricPlayer::updateEnemies
 * @param canShot
 */

void IsometricPlayer::updateEnemies(const float &shotRange)
{
	if (shotRange > 0.) {
		QVector2D radius(bodyPosition());
		radius += TiledObject::vectorFromAngle(bodyRotation(), shotRange);
		d->updateFromRayCast(rayCast(radius.toPointF(), FixtureTarget, true));
	} else {
		d->updateFromRayCast({});
	}

	d->checkVisibility();

	if (shotRange > 0.)
		d->selectEnemyForShot();
	else
		d->selectEnemyForHit();
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
	if (d->m_enemies.empty())
		return {};

	QList<IsometricEnemy *> list;
	list.reserve(d->m_enemies.size());

	for (const auto &ptr : d->m_enemies)
		if (ptr.flags & (IsometricPlayerPrivate::Visible|IsometricPlayerPrivate::CanHit))
			list.append(ptr.enemy);

	return list;
}



/**
 * @brief IsometricPlayer::clearData
 */

void IsometricPlayer::clearData()
{
	d->m_enemies.clear();
	d->m_currentEnemy = nullptr;
}


/**
 * @brief IsometricPlayer::destinationMotor
 * @return
 */

TiledPathMotor *IsometricPlayer::destinationMotor() const
{
	return d->m_destinationMotor.get();
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
 * @brief IsometricPlayer::fixtureBegin
 * @param shape
 */

void IsometricPlayer::onShapeContactBegin(b2::ShapeRef self, b2::ShapeRef other)
{
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	IsometricEnemy *enemy = categories.testFlag(FixtureTarget) ?
								dynamic_cast<IsometricEnemy*>(base) :
								nullptr;
	IsometricEnemy *enemyBody = categories.testFlag(FixtureEnemyBody)  ?
									dynamic_cast<IsometricEnemy*>(base) :
									nullptr;
	TiledPickableIface *pickable = categories.testFlag(FixturePickable) ?
									   dynamic_cast<TiledPickableIface*>(base) :
									   nullptr;


	if (isEqual(self, targetCircle()) && enemyBody) {
		d->setEnemyFlag(enemyBody, IsometricPlayerPrivate::Near);
	} else if (isAny(bodyShapes(), self) && enemy) {
		TiledReportedFixtureMap map = rayCast(enemy->bodyPosition(), FixtureEnemyBody, true);
		if (map.contains(enemy) && !map.containsTransparentGround()) {
			if (d->setEnemyFlag(enemy, IsometricPlayerPrivate::CanHit)) {
				onEnemyReached(enemy);
			}
		} else {
			d->setEnemyFlag(enemy, IsometricPlayerPrivate::CanHit, false);
		}
	} else if (isAny(bodyShapes(), self) && pickable) {
		onPickableReached(base);
	}

	/*if (base->categories().testFlag(TiledObjectBody::FixtureTransport)) {
		TiledTransport *transport = game() ? game()->transportList().find(base) : nullptr;

		if (!m_currentTransport && transport) {
			setCurrentTransport(transport);
			m_currentTransportBase = base;
		}
	}*/


}




/**
 * @brief IsometricPlayer::fixtureEnd
 * @param shape
 */

void IsometricPlayer::onShapeContactEnd(b2::ShapeRef self, b2::ShapeRef other)
{
	TiledObjectBody *base = TiledObjectBody::fromBodyRef(other.GetBody());

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(other.GetFilter().categoryBits);
	IsometricEnemy *enemy = categories.testFlag(FixtureTarget)  ?
								dynamic_cast<IsometricEnemy*>(base) :
								nullptr;

	IsometricEnemy *enemyBody = categories.testFlag(FixtureEnemyBody)  ?
									dynamic_cast<IsometricEnemy*>(base) :
									nullptr;

	TiledPickableIface *pickable = categories.testFlag(FixturePickable) ?
									   dynamic_cast<TiledPickableIface*>(base) :
									   nullptr;

	if (isEqual(self, targetCircle()) && enemyBody) {
		d->setEnemyFlag(enemyBody, IsometricPlayerPrivate::Near, false);
		if (d->setEnemyFlag(enemyBody, IsometricPlayerPrivate::CanHit, false))
			onEnemyLeft(enemyBody);
	} else if (isAny(bodyShapes(), self) && enemy) {
		if (d->setEnemyFlag(enemy, IsometricPlayerPrivate::CanHit, false))
			onEnemyLeft(enemy);
	} else if (isAny(bodyShapes(), self) && pickable) {
		onPickableLeft(base);
	}
}



/**
 * @brief IsometricPlayer::sensorBeginContact
 * @param other
 */
/*


void IsometricPlayer::fixtureBeginContact(Box2DFixture *other)
{

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (!m_currentTransport && transport) {
			setCurrentTransport(transport);
			m_currentTransportBase = base;
		}
	}

}




void IsometricPlayer::fixtureEndContact(Box2DFixture *other)
{

	if (other->categories().testFlag(TiledObjectBody::fixtureCategory(TiledObjectBody::FixtureTransport))) {
		TiledTransport *transport = m_scene->game() ? m_scene->game()->transportList().find(base) : nullptr;

		if (m_currentTransport == transport && transport) {
			setCurrentTransport(nullptr);
			m_currentTransportBase = nullptr;
		}
	}



*/





/**
 * @brief IsometricPlayer::worldStep
 */

void IsometricPlayer::worldStep() {
	IsometricEntity::worldStep();

	/// updateEnemies!!! -> RpgPlayer

	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		//clearDestinationPoint();
		stop();
	} else if (!hasAbility()) {
		stop();
	} else {
		if (d->m_destinationPoint) {
			if (!moveTowards(d->m_destinationPoint.value(), m_speedLength, m_speedRunLength*0.5, m_speedRunLength)) {
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
				const float distance = distanceToPoint(polygon.last());

				if (distance >= m_speedRunLength*0.5) {				// Hogy a végén szépen lassan gyalogoljon csak
					d->m_destinationMotor->updateBody(this, m_speedRunLength, m_game->tickTimer());
				} else {
					d->m_destinationMotor->updateBody(this, m_speedLength, m_game->tickTimer());
				}
			} else {
				stop();
				clearDestinationPoint();
			}
		} else {
			setSpeed(m_currentVelocity);
		}
	}
}



/**
 * @brief IsometricPlayer::synchronize
 */

void IsometricPlayer::synchronize()
{
	IsometricEntity::synchronize();

	if (m_enemy != d->m_currentEnemy) {
		if (m_enemy)
			m_enemy->setGlowEnabled(false);

		setEnemy(d->m_currentEnemy);

		if (m_enemy) {
			m_enemy->setGlowColor(Qt::red);
			m_enemy->setGlowEnabled(true);
		}
	}
}


/**
 * @brief IsometricPlayer::isRunning
 * @return
 */

bool IsometricPlayer::isRunning() const
{
	// 60 FPS
	return currentSpeed() >= m_speedRunLength*0.9/60;
}


/**
 * @brief IsometricPlayer::isWalking
 * @return
 */

bool IsometricPlayer::isWalking() const
{
	// 60 FPS
	const float &l = currentSpeed();
	return l < m_speedRunLength/60 && l > 0.05;
}





/**
 * @brief IsometricPlayer::currentVelocity
 * @return
 */

QVector2D IsometricPlayer::currentVelocity() const
{
	return m_currentVelocity;
}



void IsometricPlayer::setCurrentVelocity(QVector2D newCurrentVelocity)
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

	if (state.distance > 0.85) {
		setCurrentVelocity(vectorFromAngle(state.angle, m_speedRunLength));
	} else if (state.distance > 0.4) {
		setCurrentVelocity(vectorFromAngle(state.angle, m_speedLength));
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

	d->m_destinationPoint = QVector2D(point);
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
	return m_enemy;
}

void IsometricPlayer::setEnemy(IsometricEnemy *newEnemy)
{
	if (m_enemy == newEnemy)
		return;
	m_enemy = newEnemy;
	emit enemyChanged();
}
