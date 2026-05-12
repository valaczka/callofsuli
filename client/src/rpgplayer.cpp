/*
 * ---- Call of Suli ----
 *
 * rpgplayer.cpp
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgPlayer
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

#include "rpgplayer.h"


/**
 * @brief RpgPlayer::RpgPlayer
 * @param gameItem
 * @param center
 */

RpgPlayer::RpgPlayer(RpgGameItem *gameItem, const QPointF &center)
	: RpgEntity(gameItem, center, 25., CP_BODY_TYPE_DYNAMIC)
{
	m_defaultMotor = std::make_unique<RpgMotorPlayer>(this);

	filterSet(RpgGameItem::FixturePlayerBody,
			  RpgGameItem::FixtureGround);
}



/**
 * @brief RpgMotorPlayer::RpgMotorPlayer
 * @param player
 */

RpgMotorPlayer::RpgMotorPlayer(RpgPlayer *player)
	: RpgMotorEntity(player)
	, m_player(player)
{
	Q_ASSERT(m_player);
}



/**
 * @brief RpgMotorPlayer::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorPlayer::beforeWorldStep(const qint64 &tick, entt::entity &entity)
{
	/*Rpg::RpgLogicScope scope = m_game->rpgLogic().getScope();

	auto [player, map] = scope.try_get<Rpg::Player, Rpg::PlayerTickMap>(entity);

	if (map->map.isEmpty())
		return false;

	const quint32 last = m_game->rpgLogic().lastAuthTick();

	auto it = map->map.lowerBound(last);

	if (it == map->map.end() || it.key() > last) {
		return false;
	}


	m_current = it.value();

	LOG_CDEBUG("game") << "LOAD" << tick
					   << (player ? player ->playerData.playerId() : -1)
					   << "---" << last
					   << "LAST"
					   << m_current->entityState().velXAsFloat() << m_current->entityState().velYAsFloat();


	return true;*/

	return false;
}



/**
 * @brief RpgMotorPlayer::updateBody
 * @param object
 */

void RpgMotorPlayer::updateBody(TiledObject *)
{
	if (!m_current) {
		m_player->stop();
		return;
	}

	cpVect to = cpvadd(m_player->bodyPosition(), cpv(m_current->entityState().velXAsFloat(), m_current->entityState().velYAsFloat()));

	m_player->moveToPoint(to);

	m_current.reset();
}



/**
 * @brief RpgMotorPlayerControlled::RpgMotorPlayerControlled
 * @param player
 */

RpgMotorPlayerControlled::RpgMotorPlayerControlled(RpgPlayer *player)
	: AbstractRpgMotor(player)
	, m_player(player)
{
	Q_ASSERT(m_player);
}




/**
 * @brief RpgMotorPlayerControlled::updateBody
 * @param object
 */

void RpgMotorPlayerControlled::updateBody(TiledObject *)
{
	if (m_currentJoystickState.distance >= 1.0) {
		m_player->setSpeedFromAngle(m_currentJoystickState.angle, 200);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else if (m_currentJoystickState.distance > 0.5) {
		m_player->setSpeedFromAngle(m_currentJoystickState.angle, 100);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else {
		m_player->stop();
	}
}





/**
 * @brief RpgMotorPlayerControlled::afterWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorPlayerControlled::afterWorldStep(const qint64 &tick, entt::entity &entity)
{
	if (m_currentJoystickState.distance > 0.1) {
		/*Rpg::RpgLogicScope scope = m_game->rpgLogicClient().getScope();

		auto [player, map] = scope.try_get<Rpg::Player, Rpg::PlayerTickMap>(entity);

		LOG_CINFO("game") << "CURR" << tick << m_player->bodyPositionF()
						  << (player ? player ->playerData.playerId() : -1)
						  << "---" << (map ? map->map.size() : -1)
						  << "LAST"
						  << (map && !map->map.isEmpty() ? map->map.last().entityState().posXAsFloat() : -1);
						  */

		RpgStream::PlayerState st;
		st.entityState().setTick(tick);
		st.entityState().setPosXAsFloat(m_player->bodyPosition().x);
		st.entityState().setPosYAsFloat(m_player->bodyPosition().y);

		m_statePull.append(std::move(st));

		RpgStream::PlayerState out;
		std::vector<RpgStream::PlayerState> list;




		if (!m_statePull.extract(out, list)) {
			LOG_CERROR("game") << "NO";
		} else {
			std::erase_if(list, [t = tick-120](const RpgStream::PlayerState &st) {
				return st.entityState().tick() < t;
			});


			LOG_CINFO("game") << "---------------------------";

			LOG_CDEBUG("game") << out.entityState().tick() << "POS" << out.entityState().posXAsFloat() << out.entityState().posYAsFloat();

			for (const RpgStream::PlayerState &s : list) {
				LOG_CDEBUG("game") << s.entityState().tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat();
			}
/*
			RpgStream::PlayerStateList stream;
			stream.setIsDeltaMode(true);
			stream.compressStateVector(list, out);

			LOG_CWARNING("game") << "---------------------------";

			LOG_CDEBUG("game") << out.entityState().tick() << "POS" << out.entityState().posXAsFloat() << out.entityState().posYAsFloat();

			for (const RpgStream::PlayerState &s : stream.state()) {
				LOG_CDEBUG("game") << s.entityState().tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat()
								   << "|" << s.entityState().deltaMask() << s.entityState().hasPosXDeltaMask() << s.entityState().hasPosYDeltaMask();
			}

			LOG_CERROR("game") << "---------------------------";

			std::vector<RpgStream::PlayerState> test = stream.extractStateVector(out);

			LOG_CDEBUG("game") << out.entityState().tick() << "POS" << out.entityState().posXAsFloat() << out.entityState().posYAsFloat();

			for (const RpgStream::PlayerState &s : test) {
				LOG_CDEBUG("game") << s.entityState().tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat();
			}
			*/
		}

		//map->map.insert(tick, std::move(st));

		return true;
	}

	return false;
}




/**
 * @brief RpgMotorPlayerControlled::currentJoystickState
 * @return
 */

TiledGame::JoystickState RpgMotorPlayerControlled::currentJoystickState() const
{
	return m_currentJoystickState;
}

void RpgMotorPlayerControlled::setCurrentJoystickState(const TiledGame::JoystickState &newCurrentJoystickState)
{
	m_currentJoystickState = newCurrentJoystickState;
}

