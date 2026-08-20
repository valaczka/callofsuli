/*
 * ---- Call of Suli ----
 *
 * rpgnpcplayerattacker.cpp
 *
 * Created on: 2026. 08. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpcPlayerAttacker
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

#include "rpgnpcplayerattacker.h"
#include "rpgentity.h"
#include "rpgplayer.h"
#include "rpgnpc.h"



/**
 * @brief RpgNpcPlayerAttacker::RpgNpcPlayerAttacker
 * @param gameItem
 * @param center
 */

RpgNpcPlayerAttacker::RpgNpcPlayerAttacker(RpgGameItem *gameItem, const cpVect &center)
	: RpgNpc(gameItem, center)
{
	m_defaultMotor.reset(new Motor(this));
}


/**
 * @brief RpgNpcPlayerAttacker::~RpgNpcPlayerAttacker
 */

RpgNpcPlayerAttacker::~RpgNpcPlayerAttacker()
{

}


/**
 * @brief RpgNpcPlayerAttacker::getControlledMotor
 * @return
 */

std::unique_ptr<RpgMotorNpcControlled> RpgNpcPlayerAttacker::getControlledMotor()
{
	return std::unique_ptr<RpgMotorNpcControlled>(new RpgMotorNpcPlayerAttacker(this));
}



/**
 * @brief RpgNpcPlayerAttacker::Motor::processEventAt
 * @param tick
 */

void RpgNpcPlayerAttacker::Motor::processEventAt(const qint64 &tick)
{
	Q_UNUSED(tick);

	RpgPlayer *cp = m_game->controlledPlayer();

	for (const RpgStream::EventNpc &e : m_incomingEventList) {
		if (e.type() == RpgStream::EventNpc::EventAttack) {
			m_npc->jumpToSprite(m_config.bow ? "bow" : "attack", m_npc->facingDirection());

			if (cp && RpgLogicObjectMapper::getId(cp) == e.targetId()) {
				m_game->gameItem()->playSfx(m_config.bow ?
												QStringLiteral(":/rpg/broadsword/broadsword1.mp3") :
												QStringLiteral(":/rpg/shortbow/swish_2.mp3"),
											m_npc->scene(), m_npc->bodyPositionF());
			}
		}
	}
}


/**
 * @brief RpgMotorNpcPlayerAttacker::RpgMotorNpcPlayerAttacker
 * @param npc
 */

RpgMotorNpcPlayerAttacker::RpgMotorNpcPlayerAttacker(RpgNpc *npc)
	: RpgMotorNpcControlled(npc)
{
	m_config.fromJson(npc->config().data);
}




/**
 * @brief RpgMotorNpcPlayerAttacker::updateTarget
 */

void RpgMotorNpcPlayerAttacker::updateTarget()
{
	if (!m_targetList.isEmpty()) {
		QSet<TiledObjectBody*> list;

		RpgMotorEntity::queryContactedVisibleBodies(m_entity, &list,
													RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget |
													RpgGameItem::FixtureNpcBody | RpgGameItem::FixtureNpcTarget,
													RpgGameItem::FixtureGround,
													0., { RpgMotorEntity::QueryBody | RpgMotorEntity::QueryTarget | QuerySensorPolygon });

		for (auto it = m_targetList.begin(); it != m_targetList.end(); ) {
			RpgEntity *e = (*it);

			if (list.contains(e) && e->isAlive() && e->team() != m_npc->team())
				++it;
			else
				it = m_targetList.erase(it);
		}
	}



	if (m_targetList.empty()) {
		m_head = 0;
		return;
	}

	m_pursuit = false;

	clearDestination();

	attackTarget();
}




/**
 * @brief RpgMotorNpcPlayerAttacker::updateMotor
 */

void RpgMotorNpcPlayerAttacker::updateMotor()
{
	if (m_destinationPoint || m_destinationMotor)
		return;

	if (!m_targetList.empty())
		return;

	if (m_npc->targetEntity() && m_pursuit) {
		m_npc->setTargetEntity(nullptr);
		m_pursuit = false;
	} else if (m_npc->targetEntity() && m_npc->targetEntity()->isAlive() && !m_pursuit) {
		if (const auto path = m_gameItem->findShortestPath(m_npc, m_npc->targetEntity()->bodyPosition())) {
			setDestination(path.value());
			m_pursuit = true;
			return;
		}
	}

	RpgMotorNpcControlled::updateMotor();
}



/**
 * @brief RpgMotorNpcPlayerAttacker::saveState
 * @param dest
 */

void RpgMotorNpcPlayerAttacker::saveState(RpgStream::NpcState &dest)
{
	dest.setType(RpgStream::NpcData::PlayerAttacker);

	if (m_npc->targetEntity())
		dest.setTarget(RpgLogicObjectMapper::getId(m_npc->targetEntity()->objectId()));
	else
		dest.setTarget(0u);

	const auto ptr = destination();

	if (ptr) {
		dest.setDestinationXAsFloat(ptr->last().x());
		dest.setDestinationYAsFloat(ptr->last().y());
	}
}




/**
 * @brief RpgMotorNpcPlayerAttacker::getMovementSpeed
 * @return
 */

int RpgMotorNpcPlayerAttacker::getMovementSpeed()
{
	if (m_pursuit)
		return m_npc->config().run;
	else
		return m_npc->config().walk;
}



/**
 * @brief RpgMotorNpcPlayerAttacker::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgMotorNpcPlayerAttacker::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgMotorNpcControlled::onShapeContactBegin(self, other);

	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (self == m_npc->sensorPolygon()) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(otherBody)) {
			if (player->isAlive() && player->team() != m_npc->team()) {
				m_targetList.append(player);
			}
		} else if (RpgNpc *npc = dynamic_cast<RpgNpc*>(otherBody); npc && m_npc->team() != RpgStream::TeamNone) {
			if (npc->isAlive() && npc->team() == Rpg::RpgLogic::oppositeTeam(m_npc->team())) {
				m_targetList.append(npc);
			}
		}
	}
}


/**
 * @brief RpgMotorNpcPlayerAttacker::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgMotorNpcPlayerAttacker::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgMotorNpcControlled::onShapeContactEnd(self, other);

	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (self == m_npc->sensorPolygon()) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(otherBody)) {
			m_targetList.removeAll(player);
		} else if (RpgNpc *npc = dynamic_cast<RpgNpc*>(otherBody)) {
			m_targetList.removeAll(npc);
		}
	}
}





/**
 * @brief RpgMotorNpcPlayerAttacker::attackTarget
 */

void RpgMotorNpcPlayerAttacker::attackTarget()
{
	if (!m_npc->canAttack())
		return;

	if (m_targetList.empty())
		return;

	if (m_currentTick < m_lastAttack + AbstractGame::TickTimer::msecToTick(m_config.attackDelay))
		return;

	RpgEntity *tg = nullptr;

	for (int i=0; i<m_targetList.size(); ++i) {
		RpgEntity *e = m_targetList.at(m_head++ % m_targetList.size());

		if (!e->isAlive())
			continue;

		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(e)) {
			if (player->locked())
				continue;
		}

		tg = e;
		break;
	}

	if (!tg)
		return;

	m_npc->setTargetEntity(tg);

	m_npc->rotateToPoint(tg->bodyPosition(), true);

	m_lastAttack = m_currentTick;

	RpgStream::EventNpc e(RpgStream::EventNpc::EventAttack);

	e.setTargetId(RpgLogicObjectMapper::getId(tg->objectId()));

	e.setSeq(m_npc->nextEventId());

	m_npc->jumpToSprite(m_config.bow ? "bow" : "attack", m_npc->facingDirection());

	if (tg && tg == m_game->controlledPlayer()) {
		m_game->gameItem()->playSfx(m_config.bow ?
										QStringLiteral(":/rpg/broadsword/broadsword1.mp3") :
										QStringLiteral(":/rpg/shortbow/swish_2.mp3"),
									tg->scene(), tg->bodyPositionF());
	}

	m_eventList.emplace_back(std::move(e));
}
