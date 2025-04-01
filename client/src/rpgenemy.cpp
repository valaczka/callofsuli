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
	updateFromSnapshot(snapshot.s1);
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
