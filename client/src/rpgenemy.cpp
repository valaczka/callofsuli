/*
 * ---- Call of Suli ----
 *
 * rpgenemy.cpp
 *
 * Created on: 2025. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEnemy
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

#include "rpgenemy.h"
#include "rpgplayer.h"
#include "rpggame.h"
#include "rpggamedataiface_t.h"


RpgEnemy::RpgEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, RpgGame *game, const qreal &radius)
	: IsometricEnemy(game, radius)
	, RpgEnemyIface(type)
{

}


/**
 * @brief RpgEnemy::updateFromSnapshot
 * @param snapshot
 */

void RpgEnemy::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Enemy> &snapshot)
{

	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;

		stop();
		IsometricEntity::worldStep();
		return;
	}


	cpVect speed = cpvzero;

	try {
		if (snapshot.s1.st == RpgGameData::Enemy::EnemyHit ||
				snapshot.s1.st == RpgGameData::Enemy::EnemyShot ) {
			if (const qint64 t = m_stateLastRenderedTicks.value(snapshot.s1.st); t >= snapshot.s1.f)
				throw 1;


			if (!snapshot.s1.p.isEmpty()) {
				if (fnEmplace(this, cpv(snapshot.s1.p.at(0), snapshot.s1.p.at(1)), snapshot.s1.a))
					LOG_CDEBUG("scene") << "[Simulation] Forced emplace" << snapshot.s1.p << snapshot.s1.a;
			} else {
				LOG_CERROR("scene") << "Missing hitpoint" << snapshot.s1.f;
			}

			m_stateLastRenderedTicks.insert(snapshot.s1.st, snapshot.s1.f);
		}


		if (snapshot.s1.st == RpgGameData::Enemy::EnemyHit) {
			auto wptr = RpgArmory::weaponCreate(snapshot.s1.arm.cw);

			if (wptr) {
				TiledObject *target = nullptr;

				if (RpgGame *g = qobject_cast<RpgGame*>(m_game)) {
					target = dynamic_cast<TiledObject*>(g->findBody(
															TiledObjectBody::ObjectId{
																.ownerId = snapshot.s1.tg.o,
																.sceneId = snapshot.s1.tg.s,
																.id = snapshot.s1.tg.id
															}));
				}

				wptr->setParentObject(this);
				playAttackEffect(wptr.get());
				wptr->playAttack(target);

				LOG_CWARNING("scene") << "REAL ENEMY HIT" << snapshot.current << snapshot.s1.f << snapshot.s1.p << bodyPositionF()
									  << snapshot.s1.a << bodyRotation();
			}

			throw -1;
		} else if (snapshot.s1.st == RpgGameData::Enemy::EnemyShot) {
			auto wptr = RpgArmory::weaponCreate(snapshot.s1.arm.cw);

			if (wptr) {
				wptr->setParentObject(this);
				playAttackEffect(wptr.get());
				wptr->playAttack(nullptr);
			}

			LOG_CINFO("scene") << "REAL ENEMY SHOT" << snapshot.current << snapshot.s1.f << snapshot.s1.p << snapshot.s1.a << snapshot.s2.f;

			throw -1;
		} else {
			throw 1;
		}
	} catch (int e) {

		if (e > 0) {
			speed = entityMove(this, snapshot,
							   RpgGameData::Enemy::EnemyIdle, RpgGameData::Enemy::EnemyMoving,
							   m_metric.speed, 2*m_metric.pursuitSpeed,
							   nullptr);
		}

	}


	if (snapshot.s1.f >= 0)
		updateFromSnapshot(snapshot.s1);
	else
		updateFromSnapshot(snapshot.last);

	/*if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}*/

	IsometricEntity::worldStep();

	if (!(speed == cpvzero))
		overrideCurrentSpeed(speed);
}





/**
 * @brief RpgEnemy::updateFromSnapshot
 * @param snap
 */

void RpgEnemy::updateFromSnapshot(const RpgGameData::Enemy &snap)
{
	setHp(snap.hp);

	if (snap.st == RpgGameData::Enemy::EnemyAttack) {
		if (RpgGame *g = qobject_cast<RpgGame*>(m_game); g && g->actionRpgGame()) {
			if (TiledObject *target = dynamic_cast<TiledObject*>(g->findBody(
																	 TiledObjectBody::ObjectId{
																	 .ownerId = snap.tg.o,
																	 .sceneId = snap.tg.s,
																	 .id = snap.tg.id
		}))) {

				LOG_CINFO("scene") << "REAL ENEMY ATTACK" << snap.f <<
									  snap.p << snap.a << target->objectId().id;

				if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(target))
					player->attackedByEnemy(this, snap.arm.cw, false, true);
			}

		}

		LOG_CINFO("scene") << "*********** ENEMY attack" << snap.tg.o << snap.tg.id << snap.arm.cw;

		// Ne cserélje ki a fegyvert

		RpgGameData::Armory arm = snap.arm;
		arm.cw = RpgGameData::Weapon::WeaponInvalid;
		m_armory->updateFromSnapshot(arm);

	} else {
		m_armory->updateFromSnapshot(snap.arm);
	}
}



/**
 * @brief RpgEnemy::isLastSnapshotValid
 * @param snap
 * @return
 */

bool RpgEnemy::isLastSnapshotValid(const RpgGameData::Enemy &snap, const RpgGameData::Enemy &lastSnap) const
{
	if (lastSnap.f < 0)
		return false;

	if (lastSnap.hp != snap.hp) {
		LOG_CDEBUG("scene") << "%%%%%%%%%%%%%% HP override";
		return false;
	}

	if (snap.st == RpgGameData::Enemy::EnemyAttack) {
		LOG_CDEBUG("scene") << "%%%%%%%%%%%%%% ATTACK override";
		return false;
	}

	return true;
}






/**
 * @brief RpgEnemy::enemyWorldStep
 * @return
 */

bool RpgEnemy::enemyWorldStep()
{
	if (!isAlive() || isSleeping())
		return true;

	if (m_metric.autoAttackTime <= 0 || m_metric.firstAttackTime <= 0)
		return true;

	if (m_player && m_reachedPlayers.contains(m_player) && m_player->isAlive()) {
		stop();

		if (!hasAbility())
			return false;

		if (m_player->isLocked() || featureOverride(FeatureVisibility, m_player))
			return false;

		if (checkFeature(RpgGameData::Player::FeatureLockEnemy, qobject_cast<RpgPlayer*>(m_player)))
			return false;

		if (checkFeature(RpgGameData::Player::FeatureFreeWalk, qobject_cast<RpgPlayer*>(m_player)))
			return true;


		if (m_game->tickTimer()) {
			if (m_autoHitTimer >= 0 && m_autoHitTimer <= m_game->tickTimer()->currentTick()) {
				//autoAttackPlayer();
				attackPlayer(qobject_cast<RpgPlayer*>(m_player), defaultWeapon());
				m_autoHitTimer = m_game->tickTimer()->tickAddMsec(m_metric.autoAttackTime);
			} else if (m_autoHitTimer < 0) {
				m_autoHitTimer = m_game->tickTimer()->tickAddMsec(m_metric.firstAttackTime);
			}
		}

		return false;
	} else {
		m_autoHitTimer = -1;

		if (m_player && !(m_player->isLocked() || featureOverride(FeatureVisibility, m_player))) {
			if (checkFeature(RpgGameData::Player::FeatureLockEnemy, qobject_cast<RpgPlayer*>(m_player)))
				return false;
		}

	}

	return true;
}





/**
 * @brief RpgEnemy::enemyWorldStepNotReachedPlayer
 * @return
 */

bool RpgEnemy::enemyWorldStepNotReachedPlayer()
{
	if (!isAlive() || isSleeping())
		return false;

	if (m_metric.autoAttackTime <= 0 || m_metric.firstAttackTime <= 0)
		return true;

	if (defaultWeapon() && defaultWeapon()->canShot() &&
			m_player && m_contactedPlayers.contains(m_player) && m_player->isAlive()) {
		stop();

		if (m_metric.returnSpeed != 0) {
			if (!m_returnPathMotor)
				m_returnPathMotor.reset(new TiledReturnPathMotor(bodyPosition()));
		}

		if (!hasAbility())
			return false;

		if (m_player->isLocked() || featureOverride(FeatureVisibility, m_player))
			return false;

		if (checkFeature(RpgGameData::Player::FeatureLockEnemy, qobject_cast<RpgPlayer*>(m_player)))
			return false;

		if (checkFeature(RpgGameData::Player::FeatureFreeWalk, qobject_cast<RpgPlayer*>(m_player)))
			return true;

		if (m_game->tickTimer()) {
			const auto tick = m_game->tickTimer()->currentTick();

			if (m_playerDistanceSq < POW2(m_metric.sensorLength*0.2) &&
					(m_autoHitTimer == -1 || tick-m_autoHitTimer > AbstractGame::TickTimer::msecToTick(m_metric.autoAttackTime)))
				m_autoHitTimer = tick;

			if (m_autoHitTimer >= 0 && m_autoHitTimer <= tick) {
				attackPlayer(qobject_cast<RpgPlayer*>(m_player), defaultWeapon());
				m_autoHitTimer = m_game->tickTimer()->tickAddMsec(m_metric.autoAttackTime);
			} else if (m_autoHitTimer < 0) {
				m_autoHitTimer = m_game->tickTimer()->tickAddMsec(m_metric.firstAttackTime);
			}
		}

		return false;
	}

	return true;
}





/**
 * @brief RpgEnemy::attackPlayer
 * @param player
 * @param weapon
 */

void RpgEnemy::attackPlayer(RpgPlayer *player, RpgWeapon *weapon)
{
	RpgGame *g = qobject_cast<RpgGame*>(m_game);

	if (!weapon || !player || player->isLocked() || !g || featureOverride(FeatureVisibility, m_player))
		return;


	if (weapon->canShot()) {
		LOG_CTRACE("game") << "Enemy shot player:" << this << player << weapon;

		g->enemyShot(this, weapon, currentAngle());

	} else if (weapon->canHit()) {
		LOG_CTRACE("game") << "Enemy hit player:" << this << player << weapon;

		g->enemyHit(this, player, weapon);
	}
}



/**
 * @brief RpgEnemy::onAlive
 */

void RpgEnemy::onAlive()
{
	filterSet(TiledObjectBody::FixtureEnemyBody,

			  TiledObjectBody::FixtureGround |
			  TiledObjectBody::FixturePlayerBody |
			  TiledObjectBody::FixtureTarget);

	if (m_visualItem)
		m_visualItem->setProperty("ellipseSize", 2);

	IsometricEnemy::onAlive();
}



/**
 * @brief RpgEnemy::onDead
 */

void RpgEnemy::onDead()
{
	filterSet(TiledObjectBody::FixtureInvalid, FixtureInvalid);

	if (m_visualItem)
		m_visualItem->setProperty("ellipseSize", 0);

	IsometricEnemy::onDead();
}




/**
 * @brief RpgEnemy::featureOverride
 * @param feature
 * @param player
 * @return
 */

bool RpgEnemy::featureOverride(const PlayerFeature &feature, IsometricPlayer *player) const
{
	if (RpgPlayer *p = qobject_cast<RpgPlayer*>(player))
		return featureOverride(feature, p);
	else
		return false;
}



/**
 * @brief RpgEnemy::featureOverride
 * @param feature
 * @param player
 * @return
 */

bool RpgEnemy::featureOverride(const PlayerFeature &feature, RpgPlayer *player) const
{
	if (!player)
		return false;

	if (feature == FeatureVisibility) {
		//if (checkFeature(RpgPlayerCharacterConfig::FeatureCamouflage, player))
		//	return true;

		return player->isHiding() || (player->config().cast == RpgPlayerCharacterConfig::CastInvisible &&
									  player->castTimerActive());
	}


	if (feature == FeatureDisablePursuit) {
		if (checkFeature(RpgGameData::Player::FeatureFreeWalk, player))
			return true;

		if (checkFeature(RpgGameData::Player::FeatureLockEnemy, player))
			return true;
	}

	if (feature == FeatureRotate) {
		if (checkFeature(RpgGameData::Player::FeatureLockEnemy, player))
			return false;

		if (checkFeature(RpgGameData::Player::FeatureFreeWalk, player))
			return true;
	}

	if (feature == FeatureReplaceFrom) {
		if (checkFeature(RpgGameData::Player::FeatureLockEnemy, player) ||
				checkFeature(RpgGameData::Player::FeatureFreeWalk, player) )
			return true;
	}

	if (feature == FeatureReplaceTo) {
		if ((player->config().features & (RpgGameData::Player::FeatureFreeWalk |
										  RpgGameData::Player::FeatureCamouflage)) == 0)
			return true;
	}

	return false;
}


/**
 * @brief RpgEnemy::checkFeature
 * @param feature
 * @param player
 * @return
 */

bool RpgEnemy::checkFeature(const RpgGameData::Player::Feature &feature, RpgPlayer *player) const
{
	if (!player)
		return false;

	return (player->config().features.testFlag(feature) && !m_config.playerFeatures.testFlag(feature));
}



/**
 * @brief RpgEnemy::serializeEnemy
 * @return
 */

RpgGameData::Enemy RpgEnemy::serializeEnemy() const
{
	RpgGameData::Enemy p;

	p.p = toPosList(bodyPosition());
	p.a = currentAngle();

	if (std::abs(p.a) < 0.0000001)
		p.a = 0.;

	p.hp = hp();

	if (TiledScene *s = scene())
		p.sc = s->sceneId();

	const cpVect vel = cpBodyGetVelocity(body());

	if (vel.x != 0. || vel.y != 0.)
		p.st = RpgGameData::Enemy::EnemyMoving;
	else
		p.st = RpgGameData::Enemy::EnemyIdle;

	p.cv = toPosList(vel);

	p.arm = m_armory->serialize();

	if (RpgPlayer *player = qobject_cast<RpgPlayer*>(m_player)) {
		p.tg = player->baseData();
	}

	return p;
}

const RpgEnemyConfig &RpgEnemy::config() const
{
	return m_config;
}

void RpgEnemy::setConfig(const RpgEnemyConfig &newConfig)
{
	m_config = newConfig;
}


/**
 * @brief RpgEnemy::isWatchingPlayer
 * @param player
 * @return
 */

bool RpgEnemy::isWatchingPlayer(RpgPlayer *player) const
{
	if (!player || !player->isAlive() || player->isLocked() || !m_contactedPlayers.contains(player))
		return false;

	if (rayCast(player->bodyPosition(), FixturePlayerBody).isVisible(player) && !featureOverride(FeatureVisibility, player))
		return true;

	return false;
}
