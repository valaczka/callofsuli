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
#include "rpgmp.h"


/**
 * @brief RpgPlayer::RpgPlayer
 * @param gameItem
 * @param center
 */

RpgPlayer::RpgPlayer(RpgGameItem *gameItem, const cpVect &center)
	: RpgEntity(gameItem, center, 25., CP_BODY_TYPE_DYNAMIC)
{
	m_defaultMotor = std::make_unique<RpgMotorPlayer>(this);

	filterSet(RpgGameItem::FixturePlayerBody,
			  RpgGameItem::FixtureGround | RpgGameItem::FixtureControl);

	m_currentChunk.setX(-1);
	m_currentChunk.setY(-1);

	addVirtualCircle(RpgGameItem::FixtureVirtualCircle, RpgGameItem::FixtureAll, 300.);
	setSensorPolygon(300., M_PI * 0.3, RpgGameItem::FixtureSensor, RpgGameItem::FixtureAll);


	setMaxHp(19);
	setMaxMp(19);
}



/**
 * @brief RpgPlayer::currentChunk
 * @return
 */

QPoint RpgPlayer::currentChunk() const
{
	return m_currentChunk;
}

void RpgPlayer::setCurrentChunk(QPoint newCurrentChunk)
{
	if (m_currentChunk == newCurrentChunk)
		return;
	m_currentChunk = newCurrentChunk;
	emit currentChunkChanged();
}

float RpgPlayer::chunkRadius() const
{
	return m_chunkRadius;
}

void RpgPlayer::setChunkRadius(float newChunkRadius)
{
	if (qFuzzyCompare(m_chunkRadius, newChunkRadius))
		return;
	m_chunkRadius = newChunkRadius;
	emit chunkRadiusChanged();
}

QPointF RpgPlayer::currentChunkCenter() const
{
	return m_currentChunkCenter;
}

void RpgPlayer::setCurrentChunkCenter(QPointF newCurrentChunkCenter)
{
	if (m_currentChunkCenter == newCurrentChunkCenter)
		return;
	m_currentChunkCenter = newCurrentChunkCenter;
	emit currentChunkCenterChanged();
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

bool RpgMotorPlayer::beforeWorldStep(const qint64 &, entt::entity &entity)
{
	const qint64 jittered = m_game->rpgLogicClient()->jitterTick();

	if (jittered == 0)
		return false;

	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	auto [player, map] = scope.try_get<Rpg::Player, Rpg::PlayerStateOutput>(entity);


	if (!player || !map)
		return false;

	const RpgStream::PlayerState *st = map->at(jittered);

	if (!st)
		return false;

	m_current = *st;

	return true;
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

	cpVect to = cpv(m_current->entityState().posXAsFloat(),
					m_current->entityState().posYAsFloat());

	m_player->moveToPoint(to);

	m_current.reset();
}



/**
 * @brief RpgMotorPlayerControlled::RpgMotorPlayerControlled
 * @param player
 */

RpgMotorPlayerControlled::RpgMotorPlayerControlled(RpgPlayer *player)
	: RpgDestinationMotor(player)
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
	if (m_destinationPoint) {
		if (!m_player->moveTowardsLimited(m_destinationPoint.value(), 100, 250*0.5, 250)) {
			m_player->stop();
			m_player->emplace(m_destinationPoint.value());
			m_destinationPoint = std::nullopt;
		}
	} else if (m_destinationMotor) {
		if (m_destinationMotor->atEnd(m_player)) {
			m_player->stop();
			m_destinationMotor.reset();
		} else if (const QPolygonF &polygon = m_destinationMotor->polygon(); !polygon.isEmpty()) {
			const float distance = m_player->distanceToPointSq(polygon.last());

			if (distance >= POW2(250*0.5)) {				// Hogy a végén szépen lassan gyalogoljon csak
				m_destinationMotor->setSpeed(250);
				m_destinationMotor->updateBody(m_player);
			} else {
				m_destinationMotor->setSpeed(100);
				m_destinationMotor->updateBody(m_player);
			}
		} else {
			m_player->stop();
			m_destinationMotor.reset();
		}

	} else if (m_currentJoystickState.distance >= 1.0) {
		m_player->setSpeedFromAngle(m_currentJoystickState.angle, 200);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else if (m_currentJoystickState.distance > 0.5) {
		m_player->setSpeedFromAngle(m_currentJoystickState.angle, 100);
		m_player->rotateBody(m_currentJoystickState.angle);
	} else {
		m_player->stop();
		if (m_currentJoystickState.distance > 0.1)
			m_player->rotateBody(m_currentJoystickState.angle);
	}

	cpVect ahead = m_player->bodyPosition()+TiledObjectBody::vectorFromAngle(m_player->desiredBodyRotation(), m_player->chunkRadius());
	cpVect center;

	const QPoint ch = m_game->rpgLogicClient()->getChunkFromVector(ahead, &center);
	m_player->setCurrentChunk(ch);
	m_player->setCurrentChunkCenter(TiledObjectBody::toPointF(center));

}



/**
 * @brief RpgMotorPlayerControlled::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgMotorPlayerControlled::beforeWorldStep(const qint64 &tick, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const RpgStream::PlayerState *state = scope.getCurrentState<RpgStream::PlayerState>(entity);

	const quint32 myId = RpgLogicObjectMapper::getId(m_player->objectId());

	if (Rpg::EventsOutput *events = scope.getCtx<Rpg::EventsOutput>()) {
		if (const RpgStream::Events *e = events->at(tick)) {
			for (const RpgStream::EventPlayer &p : e->player()) {
				if (p.tagId() != myId)
					continue;

				if (p.type() == RpgStream::EventPlayer::EventMpPick) {
					LOG_CINFO("game") << "**************************************** MP PICKED *****************";
				}
			}
		}
	} else {
		LOG_CERROR("game") << "NO EVENTS CTX";
	}

	if (!state) {
		LOG_CERROR("game") << "!!!";
		return false;
	}

	m_player->setHp(state->hp());
	m_player->setMp(state->mp());

	return true;
}





/**
 * @brief RpgMotorPlayerControlled::afterWorldStep
 * @param tick
 * @param state
 * @return
 */

bool RpgMotorPlayerControlled::afterWorldStep(const qint64 &tick, RpgStream::FullState *state)
{
	if (!state)
		return false;

	const quint32 tagId = RpgLogicObjectMapper::getId(m_player->objectId());

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
		st.setTick(tick);
		st.entityState().setPosXAsFloat(m_player->bodyPosition().x);
		st.entityState().setPosYAsFloat(m_player->bodyPosition().y);

		m_statePull.append(std::move(st));

		std::vector<RpgStream::PlayerState> list = m_statePull.extract(m_game->gameMode() == RpgGame::MultiPlayerHost ? 6 : 1);			// SINGLE PLAYER: 1


		/*LOG_CINFO("game") << "---------------------------";

		for (const RpgStream::PlayerState &s : list) {
			LOG_CDEBUG("game") << s.tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat();
		}*/

		RpgStream::PlayerStateList sl;
		sl.setTagId(tagId);
		sl.setIsDeltaMode(state->isDeltaMode());
		if (state->isDeltaMode())
			sl.compressStateVector(std::move(list));
		else
			sl.setState(std::move(list));

		state->flags().setFlag(RpgStream::FullState::Player);
		state->players().push_back(std::move(sl));



		/*	for (const RpgStream::PlayerStateList &l : state->players().list()) {
			LOG_CDEBUG("game") << "####" << l.tagId() << l.isDeltaMode();

			for (const RpgStream::PlayerState &s : l.state()) {
				LOG_CDEBUG("game") << "#" << s.tick() << "POS" << s.entityState().posXAsFloat() << s.entityState().posYAsFloat()
								   << "|" << s.entityState().deltaMask() << s.entityState().hasPosXDeltaMask() << s.entityState().hasPosYDeltaMask();
			}
		}*/

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

		//map->map.insert(tick, std::move(st));
	}


	if (!m_eventList.empty()) {

		for (RpgStream::EventPlayer &e : m_eventList) {
			e.setTagId(tagId);
			e.setTick(tick);
		}

		RpgStream::Events events;
		events.setTick(tick);
		events.setPlayer(m_eventList);

		state->flags().setFlag(RpgStream::FullState::Event);
		state->events().emplace_back(std::move(events));

		m_eventList.clear();
	}


	return true;
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





/**
 * @brief RpgMotorPlayerControlled::eventTest
 */

void RpgMotorPlayerControlled::eventTest()
{
	LOG_CINFO("game") << "EVENT TEST";

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventTest);

	m_eventList.emplace_back(std::move(e));
}


/**
 * @brief RpgMotorPlayerControlled::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgMotorPlayerControlled::onShapeContactBegin(cpShape *self, cpShape *other)
{
	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (m_player->isBodyShape(self)) {
		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT MP" << RpgLogicObjectMapper::getId(mp->objectId());
			eventMpPick(mp);
		}
	}
}


/**
 * @brief RpgMotorPlayerControlled::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgMotorPlayerControlled::onShapeContactEnd(cpShape *self, cpShape *other)
{
	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (m_player->isBodyShape(self)) {
		if (RpgMp *mp = dynamic_cast<RpgMp*>(otherBody)) {
			LOG_CINFO("game") << "CONTACT MP END" << RpgLogicObjectMapper::getId(mp->objectId());
		}
	}
}


/**
 * @brief RpgMotorPlayerControlled::eventMpPick
 * @param mp
 */

void RpgMotorPlayerControlled::eventMpPick(RpgMp *mp)
{
	if (!mp)
		return;

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventMpPick);
	e.setMp(RpgLogicObjectMapper::getId(mp->objectId()));

	m_eventList.emplace_back(std::move(e));
}


int RpgPlayer::mp() const
{
	return m_mp;
}

void RpgPlayer::setMp(int newMp)
{
	if (m_mp == newMp)
		return;
	m_mp = newMp;
	emit mpChanged();
}

int RpgPlayer::maxMp() const
{
	return m_maxMp;
}

void RpgPlayer::setMaxMp(int newMaxMp)
{
	if (m_maxMp == newMaxMp)
		return;
	m_maxMp = newMaxMp;
	emit maxMpChanged();
}
