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

	void setEnemyDistanceSq(IsometricEnemy *enemy, const float &dist) {
		auto it = findEnemy(enemy);
		if (it != m_enemies.end()) {
			it->distance = dist;
		}
	}

	// Squared!

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
			if ((d->flags & (Visible|CanShot)) != (Visible|CanShot) || (range > 0. && d->distance > POW2(range)))
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


	void updateFromRayCast(const RayCastInfo &list) {
		for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
			const bool inMap = list.isVisible(it->enemy);

			if (it->flags.testFlag(CanShot) && !it->flags.testFlag(CanHit) && !inMap)
				it = removeEnemy(it);
			else
				++it;
		}

		for (const RayCastInfoItem &f : list) {
			if (IsometricEnemy *e = dynamic_cast<IsometricEnemy*>(TiledObjectBody::fromShapeRef(f.shape)))
				setEnemy(e, EnemyFlags(Visible|CanShot), m_player->distanceToPointSq(f.point));
		}
	}


	void checkVisibility() {
		for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
			if (!it->enemy->isAlive() || it->enemy->isSleeping()) {
				it = removeEnemy(it);
				continue;
			}

			const RayCastInfo &map = m_player->rayCast(it->enemy->bodyPosition(), TiledObjectBody::FixtureEnemyBody);

			auto mapIt = std::find_if(map.cbegin(), map.cend(), [&it](const RayCastInfoItem &item){
				return TiledObjectBody::fromShapeRef(item.shape) == it->enemy;
			});

			const bool inMap = mapIt != map.cend();

			it->flags.setFlag(Visible, inMap && mapIt->visible);

			if (inMap) {
				it->distance = m_player->distanceToPointSq(mapIt->point);
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

	IsometricPlayer *const m_player;

	friend class IsometricPlayer;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(IsometricPlayerPrivate::EnemyFlags);


/**
 * @brief IsometricPlayer::IsometricPlayer
 * @param parent
 */

IsometricPlayer::IsometricPlayer(TiledGame *game, const qreal &radius, const cpBodyType &type)
	: IsometricEntity(game, radius, type)
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

	//createMarkerItem();

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
		addVirtualCircle(FixtureTrigger | FixtureControl, m_sensorLength);
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
	setSubZ(0.5);
	emit becameAlive();

	if (m_visualItem)
		m_visualItem->setProperty("ellipseSize", 2);

	filterSet(TiledObjectBody::FixturePlayerBody,
					  TiledObjectBody::FixtureCategories(TiledObjectBody::FixtureAll)
					  .setFlag(TiledObjectBody::FixturePlayerBody, false)
					  .setFlag(TiledObjectBody::FixtureVirtualCircle, false)
					  //.setFlag(TiledObjectBody::FixtureSensor, false)
					  );
}




/**
 * @brief IsometricPlayer::onDead
 */

void IsometricPlayer::onDead()
{
	setSubZ(0.0);
	setCurrentVelocity({});

	m_game->onPlayerDead(this);

	emit becameDead();

	if (m_visualItem)
		m_visualItem->setProperty("ellipseSize", 0);

	filterSet(TiledObjectBody::FixtureInvalid, TiledObjectBody::FixtureInvalid);
}



/**
 * @brief IsometricPlayer::startInability
 */

void IsometricPlayer::startInability()
{
	if (m_inabilityTime > 0 && m_game->tickTimer()) {
		m_inabilityTimer = m_game->tickTimer()->tickAddMsec(m_inabilityTime);
	}
}


/**
 * @brief IsometricPlayer::startInability
 * @param msec
 */

void IsometricPlayer::startInability(const int &msec)
{
	if (m_inabilityTime > 0 && m_game->tickTimer()) {
		m_inabilityTimer = m_game->tickTimer()->tickAddMsec(msec);
	}
}



/**
 * @brief IsometricPlayer::updateEnemies
 * @param canShot
 */

void IsometricPlayer::updateEnemies(const float &shotRange)
{
	if (shotRange > 0.) {
		cpVect dest = cpvadd(bodyPosition(), vectorFromAngle(bodyRotation(), shotRange));
		d->updateFromRayCast(rayCast(dest, FixtureTarget));
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

void IsometricPlayer::onShapeContactBegin(cpShape *self, cpShape *other)
{
	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(other).categories);


	IsometricEnemy *enemy = categories.testFlag(FixtureTarget) ?
								dynamic_cast<IsometricEnemy*>(base) :
								nullptr;
	IsometricEnemy *enemyBody = categories.testFlag(FixtureEnemyBody)  ?
									dynamic_cast<IsometricEnemy*>(base) :
									nullptr;

	if (self == targetCircle() && enemyBody) {
		d->setEnemyFlag(enemyBody, IsometricPlayerPrivate::Near);
	} else if (isBodyShape(self) && enemy) {
		if (rayCast(enemy->bodyPosition(), FixtureEnemyBody).isWalkable(enemy)) {
			if (d->setEnemyFlag(enemy, IsometricPlayerPrivate::CanHit)) {
				onEnemyReached(enemy);
			}
		} else {
			d->setEnemyFlag(enemy, IsometricPlayerPrivate::CanHit, false);
		}
	}

}




/**
 * @brief IsometricPlayer::fixtureEnd
 * @param shape
 */

void IsometricPlayer::onShapeContactEnd(cpShape *self, cpShape *other)
{
	TiledObjectBody *base = TiledObjectBody::fromShapeRef(other);

	if (!base)
		return;

	const FixtureCategories categories = FixtureCategories::fromInt(cpShapeGetFilter(other).categories);

	IsometricEnemy *enemy = categories.testFlag(FixtureTarget) ?
								dynamic_cast<IsometricEnemy*>(base) :
								nullptr;
	IsometricEnemy *enemyBody = categories.testFlag(FixtureEnemyBody)  ?
									dynamic_cast<IsometricEnemy*>(base) :
									nullptr;

	if (self == targetCircle() && enemyBody) {
		d->setEnemyFlag(enemyBody, IsometricPlayerPrivate::Near, false);
		if (d->setEnemyFlag(enemyBody, IsometricPlayerPrivate::CanHit, false))
			onEnemyLeft(enemyBody);
	} else if (isBodyShape(self) && enemy) {
		if (d->setEnemyFlag(enemy, IsometricPlayerPrivate::CanHit, false))
			onEnemyLeft(enemy);
	}
}


/**
 * @brief IsometricPlayer::onShapeAboutToDelete
 * @param shape
 */

void IsometricPlayer::onShapeAboutToDelete(cpShape *shape)
{
	IsometricEnemy *enemy = dynamic_cast<IsometricEnemy*>(TiledObjectBody::fromShapeRef(shape));
	if (enemy)
		d->removeEnemy(enemy);
}






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
		if (m_destinationPoint) {
			if (!moveTowardsLimited(m_destinationPoint.value(), m_speedLength, m_speedRunLength*0.5, m_speedRunLength)) {
				stop();
				emplace(m_destinationPoint.value());
				clearDestinationPoint();
				atDestinationPointEvent();
			}
		} else if (m_destinationMotor) {
			if (m_destinationMotor->atEnd(this)) {
				stop();
				clearDestinationPoint();
				atDestinationPointEvent();
			} else if (const QPolygonF &polygon = m_destinationMotor->polygon(); !polygon.isEmpty()) {
				const float distance = distanceToPointSq(polygon.last());

				if (distance >= POW2(m_speedRunLength*0.5)) {				// Hogy a végén szépen lassan gyalogoljon csak
					m_destinationMotor->updateBody(this, m_speedRunLength, m_game->tickTimer());
				} else {
					m_destinationMotor->updateBody(this, m_speedLength, m_game->tickTimer());
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
 * @brief IsometricPlayer::canSetDestinationPoint
 * @return
 */

bool IsometricPlayer::canSetDestinationPoint() const
{
	return !m_isLocked;
}


/**
 * @brief IsometricPlayer::isRunning
 * @return
 */

bool IsometricPlayer::isRunning() const
{
	// 60 FPS
	return currentSpeedSq() >= POW2(m_speedRunLength)*0.9;
}


/**
 * @brief IsometricPlayer::isWalking
 * @return
 */

bool IsometricPlayer::isWalking() const
{
	// 60 FPS
	const float &l = currentSpeedSq();
	return l < POW2(m_speedRunLength) && l > POW2(0.05);
}





/**
 * @brief IsometricPlayer::currentVelocity
 * @return
 */

cpVect IsometricPlayer::currentVelocity() const
{
	return m_currentVelocity;
}



void IsometricPlayer::setCurrentVelocity(const cpVect &newCurrentVelocity)
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

	if (state.distance >= 1.0) {
		setCurrentVelocity(vectorFromAngle(state.angle, m_speedRunLength));
	} else if (state.distance > 0.5) {
		setCurrentVelocity(vectorFromAngle(state.angle, m_speedLength));
	} else {
		setCurrentVelocity(cpvzero);
	}
}



/**
 * @brief IsometricPlayer::setDestinationPoint
 * @param polygon
 */





/**
 * @brief IsometricPlayer::setDestinationPoint
 * @param point
 */




/**
 * @brief IsometricPlayer::clearDestinationPoint
 */






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
