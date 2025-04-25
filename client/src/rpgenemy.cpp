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


	QVector2D speed;

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

	} else {
		speed = entityMove(this, snapshot, RpgGameData::Enemy::EnemyIdle, RpgGameData::Enemy::EnemyMoving, m_metric.speed);
	}

	updateFromSnapshot(snapshot.s1);

	if (m_moveDisabledSpriteList.contains(m_spriteHandler->currentSprite())) {
		stop();
	} else if (!hasAbility()) {
		stop();
	}

	IsometricEntity::worldStep();

	if (!speed.isNull())
		overrideCurrentSpeed(speed);
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

	if (!weapon || !player || player->isLocked() || !g)
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
 * @brief RpgEnemy::serializeEnemy
 * @return
 */

RpgGameData::Enemy RpgEnemy::serializeEnemy() const
{
	RpgGameData::Enemy p;

	p.p = toPosList(bodyPosition());
	p.a = desiredBodyRotation(); //currentAngle();
	p.hp = hp();

	if (TiledScene *s = scene())
		p.sc = s->sceneId();

	const cpVect vel = cpBodyGetVelocity(body());

	if (vel.x != 0. || vel.y != 0.)
		p.st = RpgGameData::Enemy::EnemyMoving;
	else
		p.st = RpgGameData::Enemy::EnemyIdle;

	p.cv = { (float) vel.x, (float) vel.y };

	p.arm = m_armory->serialize();

	if (RpgPlayer *player = qobject_cast<RpgPlayer*>(m_player)) {
		p.tg = player->baseData();
	}

	return p;
}
