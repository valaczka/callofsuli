/*
 * ---- Call of Suli ----
 *
 * rpgnpctowerattacker.cpp
 *
 * Created on: 2026. 07. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpcTowerAttacker
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

#include "rpgnpctowerattacker.h"



RpgNpcTowerAttacker::RpgNpcTowerAttacker(RpgGameItem *gameItem, const cpVect &center)
	: RpgNpc(gameItem, center)
{
	m_defaultMotor.reset(new Motor(this));
}



/**
 * @brief RpgNpcTowerAttacker::~RpgNpcTowerAttacker
 */

RpgNpcTowerAttacker::~RpgNpcTowerAttacker()
{

}


/**
 * @brief RpgNpcTowerAttacker::getControlledMotor
 * @return
 */

std::unique_ptr<RpgMotorNpcControlled> RpgNpcTowerAttacker::getControlledMotor()
{
	return std::unique_ptr<RpgMotorNpcControlled>(new RpgMotorNpcTowerAttacker(this));
}



/**
 * @brief RpgMotorNpcTowerAttacker::RpgMotorNpcTowerAttacker
 * @param npc
 */

RpgMotorNpcTowerAttacker::RpgMotorNpcTowerAttacker(RpgNpc *npc)
	: RpgMotorNpcControlled(npc)
{
	loadTowers();

	m_config.fromJson(npc->config().data);
}





/**
 * @brief RpgMotorNpcTowerAttacker::updateTarget
 */

void RpgMotorNpcTowerAttacker::updateTarget()
{
	if (!m_targetTower)
		return;

	if (m_targetTower && !m_targetTower->state().active()) {
		m_targetTower = nullptr;
		m_targetDefender = nullptr;
		return;
	}

	if (m_targetTowerReached) {
		clearDestination();

		if (m_targetDefender) {
			if (m_targetDefender->isAlive()) {
				m_npc->rotateToPoint(m_targetDefender->bodyPosition());
				return attackTarget();

			} else if (!m_targetDefender->isAlive()) {
				m_targetDefender = findNextDefender(m_targetTower);
				m_targetTowerReached = 0;
			}

		} else if (m_targetTower->canAttack() && m_targetTower->state().active()) {
			m_npc->rotateToPoint(m_targetTower->bodyPosition());
			return attackTarget();

		} else {
			m_targetTower = nullptr;
			m_targetDefender = nullptr;
			m_targetTowerReached = 0;
			return;
		}
	}

	const auto dest = destination();

	if (m_targetDefender) {
		if (m_npc->distanceToPointSq(m_targetDefender->bodyPosition()) < POW2(25)) {
			m_targetTowerReached = m_currentTick;
			return;
		}

		if (dest && dest->last() == m_targetDefender->bodyPositionF())
			return;
	}

	if (m_npc->distanceToPointSq(m_targetTower->bodyPosition()) < POW2(25)) {
		m_targetTowerReached = m_currentTick;
		return;
	}

	if (dest && dest->last() == m_targetTower->bodyPositionF())
		return;

	const auto path = m_gameItem->findShortestPath(m_npc,
												   m_targetDefender ? m_targetDefender->bodyPosition() :
																	  m_targetTower->bodyPosition());

	if (!path) {
		LOG_CERROR("game") << "No available path";
		return;
	}

	setDestination(path.value());
}





/**
 * @brief RpgMotorNpcTowerAttacker::updateMotor
 */

void RpgMotorNpcTowerAttacker::updateMotor()
{
	if (m_destinationPoint || m_destinationMotor)
		return;

	if (m_targetTower)
		return;

	if (m_towers.empty()) {
		LOG_CERROR("game") << "No tower";
		return;
	}

	++m_currentIdx;


	if (m_currentIdx >= (int) m_towers.size())
		m_currentIdx = 0;

	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();
	Rpg::ChunkGrid *grid = scope.getCtx<Rpg::ChunkGrid>();

	if (!grid) {
		LOG_CERROR("game") << "Missing grid";
		return;
	}


	cpVect dest = cpvzero;

	for (auto e : scope.view<Rpg::Tower>()) {
		const Rpg::Tower &t = scope.get<Rpg::Tower>(e);

		if (t.idTag != m_towers.at(m_currentIdx))
			continue;

		if (t.adjacentChunks.empty()) {
			LOG_CERROR("game") << "Missing chunks";
			return;
		}

		std::uniform_int_distribution<int> dist(0, t.adjacentChunks.size()-1);

		auto ch = t.adjacentChunks.at(dist(m_game->rpgLogicClient()->rnd()));
		dest = grid->chunkCenter(ch);
	}

	const auto path = m_gameItem->findShortestPath(m_npc, dest);

	if (!path) {
		LOG_CERROR("game") << "No available path";
		return;
	}

	setDestination(path.value());
}


/**
 * @brief RpgMotorNpcTowerAttacker::saveState
 * @param dest
 */

void RpgMotorNpcTowerAttacker::saveState(RpgStream::NpcState &dest)
{
	dest.setType(RpgStream::NpcData::TowerAttacker);

	if (m_targetTower)
		dest.setTarget(RpgLogicObjectMapper::getId(m_targetTower->objectId()));
	else
		dest.setTarget(0u);

	const auto ptr = destination();

	if (m_currentIdx >= 0 && m_currentIdx < (int) m_towers.size())
		dest.setDestinationTower(m_towers.at(m_currentIdx));

	if (ptr) {
		dest.setDestinationXAsFloat(ptr->last().x());
		dest.setDestinationYAsFloat(ptr->last().y());
	}
}





/**
 * @brief RpgMotorNpcTowerAttacker::getMovementSpeed
 * @return
 */

int RpgMotorNpcTowerAttacker::getMovementSpeed()
{
	if (m_targetTower)
		return m_npc->config().run;
	else
		return m_npc->config().walk;
}




/**
 * @brief RpgMotorNpcTowerAttacker::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgMotorNpcTowerAttacker::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgMotorNpcControlled::onShapeContactBegin(self, other);

	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (self == m_npc->sensorPolygon()) {
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			if (!m_targetTower && tower->state().active() && tower->state().team() != m_npc->team()) {
				m_targetTower = tower;
				m_targetTowerReached = 0;
				m_lastAttack = 0;
				m_targetDefender = findNextDefender(m_targetTower);

				if (!m_targetDefender && !tower->canAttack()) {
					m_targetTower = nullptr;
					m_targetTowerReached = 0;
					m_lastAttack = 0;
					m_targetDefender = nullptr;
				}
			}
		}
	}

	if (self == m_npc->targetCircle() || m_npc->isBodyShape(self)) {
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			if (!m_targetDefender && tower->canAttack() && tower->state().active() && tower->state().team() != m_npc->team()) {
				m_targetTower = tower;
				m_targetTowerReached = m_currentTick;
			}
		}

		if (RpgDefender *defender = dynamic_cast<RpgDefender*>(otherBody)) {
			RpgTower *tower = defender->tower();
			if (tower && tower->state().active() && defender->isAlive() && tower->state().team() != m_npc->team()) {
				m_targetTower = tower;
				m_targetTowerReached = m_currentTick;
				m_targetDefender = defender;
			}
		}
	}
}


/**
 * @brief RpgMotorNpcTowerAttacker::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgMotorNpcTowerAttacker::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgMotorNpcControlled::onShapeContactEnd(self, other);

	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (self == m_npc->targetCircle() || m_npc->isBodyShape(self)) {
		if (RpgTower *tower = dynamic_cast<RpgTower*>(otherBody)) {
			if (m_targetTowerReached && m_targetTower == tower) {
				m_targetTower = nullptr;
				m_targetDefender = nullptr;
				m_targetTowerReached = 0;
			}
		}

		if (RpgDefender *defender = dynamic_cast<RpgDefender*>(otherBody)) {
			RpgTower *tower = defender->tower();
			if (m_targetTowerReached && m_targetTower == tower) {
				m_targetTowerReached = 0;
				m_targetDefender = nullptr;
			}
		}
	}
}





/**
 * @brief RpgMotorNpcTowerAttacker::processEventAt
 * @param tick
 */

void RpgMotorNpcTowerAttacker::processEventAt(const qint64 &/*tick*/)
{
	// Skip attack events...

}




/**
 * @brief RpgMotorNpcTowerAttacker::loadTowers
 */

void RpgMotorNpcTowerAttacker::loadTowers()
{
	Q_ASSERT(m_game);
	Q_ASSERT(m_game->rpgLogicClient());

	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	for (auto e : scope.view<Rpg::Tower>()) {
		const Rpg::Tower &t = scope.get<Rpg::Tower>(e);
		m_towers.push_back(t.idTag);

	}
}





/**
 * @brief RpgMotorNpcTowerAttacker::attackTarget
 */

void RpgMotorNpcTowerAttacker::attackTarget()
{
	if (!m_npc->canAttack())
		return;

	if (!m_targetDefender && !m_targetTower)
		return;

	if (m_currentTick < m_lastAttack + AbstractGame::TickTimer::msecToTick(m_config.attackDelay))
		return;

	m_lastAttack = m_currentTick;

	RpgStream::EventNpc e(RpgStream::EventNpc::EventAttack);

	if (m_targetDefender && m_targetDefender->isAlive())
		e.setTargetId(RpgLogicObjectMapper::getId(m_targetDefender->objectId()));
	else if (m_targetTower && m_targetTower->canAttack() && m_targetTower->state().active())
		e.setTargetId(RpgLogicObjectMapper::getId(m_targetTower->objectId()));
	else
		return;

	e.setSeq(m_npc->nextEventId());

	m_npc->jumpToSprite("attack", m_npc->facingDirection());

	m_eventList.emplace_back(std::move(e));
}





/**
 * @brief RpgMotorNpcTowerAttacker::findNextDefender
 * @param tower
 * @return
 */

RpgDefender *RpgMotorNpcTowerAttacker::findNextDefender(RpgTower *tower) const
{
	if (!tower)
		return nullptr;

	for (RpgDefenderPoint *p : tower->defenderPoints()) {
		if (RpgDefender *def = p->defender(); def && def->isAlive() && def->team() != m_npc->team()) {
			return def;
		}
	}

	for (RpgDefender *def : tower->defenders()) {
		if (def && def->team() != m_npc->team())
			return def;
	}

	return nullptr;
}




/**
 * @brief RpgNpcTowerAttacker::Motor::processEventAt
 * @param tick
 */

void RpgNpcTowerAttacker::Motor::processEventAt(const qint64 &/*tick*/)
{
	for (const RpgStream::EventNpc &e : m_incomingEventList) {
		if (e.type() == RpgStream::EventNpc::EventAttack) {
			m_npc->jumpToSprite("attack", m_npc->facingDirection());
		}
	}
}
