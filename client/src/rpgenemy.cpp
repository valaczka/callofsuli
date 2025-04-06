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
#include "tiledspritehandler.h"
#include "rpggamedataiface_t.h"


RpgEnemy::RpgEnemy(const RpgGameData::EnemyBaseData::EnemyType &type, TiledScene *scene)
	: IsometricEnemy(scene)
	, RpgEnemyIface(type)
{

}


/**
 * @brief RpgEnemy::updateFromSnapshot
 * @param snapshot
 */

void RpgEnemy::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::Enemy> &snapshot)
{
	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}

	if (snapshot.s1.f < 0) {
		LOG_CERROR("game") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.current;
		stop();
		IsometricEntity::worldStep();
		return;
	}

	if (m_lastSnap >= 0 && snapshot.s1.f > m_lastSnap) {
		LOG_CERROR("game") << "SNAP ERROR" << m_lastSnap << snapshot.s1.f << snapshot.current << snapshot.s2.f;
	}
	m_lastSnap = snapshot.s2.f;


	if (snapshot.s1.st == RpgGameData::Enemy::EnemyHit) {
		LOG_CINFO("game") << "ENEMYHIT" << snapshot.current << snapshot.s1.f << snapshot.s1.p << snapshot.s1.a << snapshot.s2.f;

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
		}

	} /*else if (snapshot.s2.f >= 0 && snapshot.s2.f <= snapshot.current) {
		LOG_CDEBUG("game") << "---------------------skip" << snapshot.current << snapshot.s2.f;
	} else {
		if (snapshot.s1.p.size() > 1 && snapshot.s2.p.size() > 1) {
			QVector2D final(snapshot.s2.p.at(0), snapshot.s2.p.at(1));

			if (snapshot.s1.st == RpgGameData::Enemy::EnemyIdle &&
					snapshot.s2.st == RpgGameData::Enemy::EnemyIdle) {
				const b2Vec2 &vel = body().GetLinearVelocity();
				if (vel.x != 0. || vel.y != 0.) {
					//LOG_CINFO("game") << "FULL STOP ENEMY" << final;
					stop();
					emplace(final);
				}
				setCurrentAngle(snapshot.s2.a);
			} else if (snapshot.s2.st == RpgGameData::Enemy::EnemyIdle &&
					   distanceToPoint(final) < m_metric.speed / 60.) {
				//LOG_CINFO("game") << "STOP ENTITY" << final;
				stop();
				emplace(final);
				setCurrentAngle(snapshot.s2.a);
			} else if (snapshot.s2.st == RpgGameData::Enemy::EnemyMoving) {
				const float dist = distanceToPoint(final) * 1000. / (float) (snapshot.s2.f-snapshot.current);
				const float angle = angleToPoint(final);
				setCurrentAngle(angle);
				setSpeedFromAngle(angle, dist);

				//LOG_CDEBUG("game") << "DIST" << snapshot.current << snapshot.s1.f << snapshot.s1.st << snapshot.s2.f << snapshot.s2.st << dist;
			} else {
				LOG_CDEBUG("game") << "INVALID" << snapshot.current << snapshot.s1.f << snapshot.s1.st << snapshot.s2.f << snapshot.s2.st;
			}

		} else {
			LOG_CERROR("game")
					<< snapshot.s1.f
					<< snapshot.s1.st
					<< snapshot.s1.p
					<< "---"
					<< snapshot.s2.f
					<< snapshot.s2.st
					<< snapshot.s2.p ;
			//stop();
		}
	}*/

	else {
		entityMove(this, snapshot, RpgGameData::Enemy::EnemyIdle, RpgGameData::Enemy::EnemyMoving, m_metric.speed);
	}

	updateFromSnapshot(snapshot.s1);

	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}

	IsometricEntity::worldStep();
}


/**
 * @brief RpgEnemy::updateFromSnapshot
 * @param snap
 */

void RpgEnemy::updateFromSnapshot(const RpgGameData::Enemy &snap)
{
	setHp(snap.hp);
	m_armory->updateFromSnapshot(snap.arm);
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

		if (m_player->isLocked())
			return false;

		if (m_game->tickTimer()) {
			const auto tick = m_game->tickTimer()->currentTick();
			if (m_autoHitTimer >= 0 && m_autoHitTimer <= tick) {
				//autoAttackPlayer();
				attackPlayer(qobject_cast<RpgPlayer*>(m_player), defaultWeapon());
				m_autoHitTimer = tick + m_metric.autoAttackTime;
			} else if (m_autoHitTimer < 0) {
				m_autoHitTimer = tick + m_metric.firstAttackTime;
			}
		}

		return false;
	} else {
		m_autoHitTimer = -1;
	}

	return true;
}





/**
 * @brief RpgEnemy::enemyWorldStepOnVisiblePlayer
 * @return
 */

bool RpgEnemy::enemyWorldStepOnVisiblePlayer()
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

		if (m_player->isLocked())
			return false;

		if (m_game->tickTimer()) {
			const auto tick = m_game->tickTimer()->currentTick();

			if (m_playerDistance < m_metric.sensorLength*0.2 &&
					(m_autoHitTimer == -1 || tick-m_autoHitTimer > m_metric.autoAttackTime))
				m_autoHitTimer = tick;

			if (m_autoHitTimer >= 0 && m_autoHitTimer <= tick) {
				attackPlayer(qobject_cast<RpgPlayer*>(m_player), defaultWeapon());
				m_autoHitTimer = tick + m_metric.autoAttackTime;
			} else if (m_autoHitTimer < 0) {
				m_autoHitTimer = tick + m_metric.firstAttackTime;
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

	if (!weapon || !player || player->isLocked() || !g)
		return;


	if (weapon->canShot()) {
		LOG_CTRACE("game") << "Enemy shot player:" << this << player << weapon;

		if (auto fn = g->funcEnemyShot())
			fn(this, weapon, currentAngle());

	} else if (weapon->canHit()) {
		LOG_CTRACE("game") << "Enemy hit player:" << this << player << weapon;

		if (auto fn = g->funcEnemyHit())
			fn(this, player, weapon);
	}
}



/**
 * @brief RpgEnemy::serializeEnemy
 * @return
 */

RpgGameData::Enemy RpgEnemy::serializeEnemy() const
{
	RpgGameData::Enemy p;

	b2Vec2 pos = body().GetPosition();
	p.p = { pos.x, pos.y };
	p.a = currentAngle();
	p.hp = hp();

	if (TiledScene *s = scene())
		p.sc = s->sceneId();

	const b2Vec2 vel = body().GetLinearVelocity();

	if (vel.x != 0. || vel.y != 0.)
		p.st = RpgGameData::Enemy::EnemyMoving;
	else
		p.st = RpgGameData::Enemy::EnemyIdle;

	p.arm = m_armory->serialize();

	if (RpgPlayer *player = qobject_cast<RpgPlayer*>(m_player)) {
		p.tg = player->baseData();
	}

	return p;
}
