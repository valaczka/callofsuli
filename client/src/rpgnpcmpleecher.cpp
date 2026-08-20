/*
 * ---- Call of Suli ----
 *
 * rpgnpcmpleecher.cpp
 *
 * Created on: 2026. 07. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgNpcMpLeecher
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

#include "rpgnpcmpleecher.h"
#include "rpgplayer.h"



/**
 * @brief RpgNpcMpLeecher::RpgNpcMpLeecher
 * @param gameItem
 * @param center
 */

RpgNpcMpLeecher::RpgNpcMpLeecher(RpgGameItem *gameItem, const cpVect &center)
	: RpgNpc(gameItem, center)
{
	m_defaultMotor.reset(new Motor(this));
}


/**
 * @brief RpgNpcMpLeecher::~RpgNpcMpLeecher
 */

RpgNpcMpLeecher::~RpgNpcMpLeecher()
{

}


/**
 * @brief RpgNpcMpLeecher::getControlledMotor
 * @return
 */

std::unique_ptr<RpgMotorNpcControlled> RpgNpcMpLeecher::getControlledMotor()
{
	return std::unique_ptr<RpgMotorNpcControlled>(new RpgMotorNpcMpLeecher(this));
}


/**
 * @brief RpgMotorNpcMpLeecher::RpgMotorNpcMpLeecher
 * @param npc
 */

RpgMotorNpcMpLeecher::RpgMotorNpcMpLeecher(RpgNpc *npc)
	: RpgMotorNpcControlled(npc)
{
	m_config.fromJson(npc->config().data);
}



/**
 * @brief RpgMotorNpcMpLeecher::updateTarget
 */

void RpgMotorNpcMpLeecher::updateTarget()
{
	RpgPlayer *p = qobject_cast<RpgPlayer*>(m_npc->targetEntity());

	if (!p)
		return;

	if (!p->isAlive() || p->mp() <= 0 || p->team() == m_npc->team() || p->invisible()) {
		m_npc->setTargetEntity(nullptr);
		m_targetReached = 0;
		return;
	}

	{
		Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();
		if (scope.logic()->checkInFog(p->bodyPosition(), p->team())) {
			m_npc->setTargetEntity(nullptr);
			m_targetReached = 0;
			return;
		}
	}

	if (m_targetReached) {
		clearDestination();

		m_npc->rotateToPoint(p->bodyPosition());
		return attackTarget();
	}


	const auto dest = destination();

	// body radius = 25

	if (m_npc->distanceToPointSq(p->bodyPosition()) < POW2(50)) {
		m_targetReached = m_currentTick;
		return;
	}

	if (dest && p->distanceToPointSq(dest->last()) < POW2(75))
		return;

	const auto path = m_gameItem->findShortestPath(m_npc, p->bodyPosition());

	if (!path) {
		LOG_CERROR("game") << "No available path";
		return;
	}

	setDestination(path.value());
}


/**
 *
 *
 *
 *
 * @brief RpgMotorNpcMpLeecher::updateMotor
 */

void RpgMotorNpcMpLeecher::updateMotor()
{
	if (m_destinationPoint || m_destinationMotor)
		return;

	if (m_npc->targetEntity())
		return;

	RpgMotorNpcControlled::updateMotor();
}





/**
 * @brief RpgMotorNpcMpLeecher::saveState
 * @param dest
 */

void RpgMotorNpcMpLeecher::saveState(RpgStream::NpcState &dest)
{
	dest.setType(RpgStream::NpcData::MpLeecher);

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
 * @brief RpgMotorNpcMpLeecher::getMovementSpeed
 * @return
 */

int RpgMotorNpcMpLeecher::getMovementSpeed()
{
	if (m_npc->targetEntity())
		return m_npc->config().run;
	else
		return m_npc->config().walk;
}



/**
 * @brief RpgMotorNpcMpLeecher::onShapeContactBegin
 * @param self
 * @param other
 */

void RpgMotorNpcMpLeecher::onShapeContactBegin(cpShape *self, cpShape *other)
{
	RpgMotorNpcControlled::onShapeContactBegin(self, other);

	TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (self == m_npc->sensorPolygon()) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(otherBody)) {
			if (!m_npc->targetEntity() && player->isAlive() && player->team() != m_npc->team()
					&& player->mp() > 0 && !player->invisible()) {
				m_npc->setTargetEntity(player);
				m_targetReached = 0;
				m_lastAttack = 0;
			}
		}
	}


	// Ezt nem tudjuk megcsinálni, mert a bodyk nem ütköznek egymással

	/*if (m_npc->isBodyShape(self)) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(otherBody); player && player->isBodyShape(other)) {
			if (player->isAlive() && player->team() != m_npc->team() && player->mp() > 0) {

				LOG_CDEBUG("game") << "TARGET REACHED" << player << m_currentTick;

				m_target = player;
				m_targetReached = m_currentTick;
			}
		}
	}*/
}



/**
 * @brief RpgMotorNpcMpLeecher::onShapeContactEnd
 * @param self
 * @param other
 */

void RpgMotorNpcMpLeecher::onShapeContactEnd(cpShape *self, cpShape *other)
{
	RpgMotorNpcControlled::onShapeContactEnd(self, other);

	/*TiledObjectBody *otherBody = TiledObjectBody::fromShapeRef(other);

	if (!otherBody) {
		LOG_CERROR("game") << "****ERR";
		return;
	}

	if (self == m_npc->targetCircle() || m_npc->isBodyShape(self)) {
		if (RpgPlayer *player = dynamic_cast<RpgPlayer*>(otherBody); player && player->targetCircle() == other) {
			if (m_target && m_target == player) {
				m_targetReached = 0;
			}
		}
	}*/
}







/**
 * @brief RpgMotorNpcMpLeecher::attackTarget
 */


void RpgMotorNpcMpLeecher::attackTarget()
{
	if (!m_npc->canAttack())
		return;

	RpgPlayer *p = qobject_cast<RpgPlayer*>(m_npc->targetEntity());

	if (!p)
		return;

	if (m_currentTick < m_lastAttack + AbstractGame::TickTimer::msecToTick(m_config.attackDelay))
		return;

	if (p->locked())
		return;

	QSet<TiledObjectBody*> list;

	RpgMotorEntity::queryContactedVisibleBodies(m_entity, &list,
												RpgGameItem::FixturePlayerBody | RpgGameItem::FixturePlayerTarget,
												RpgGameItem::FixtureGround,
												0., { RpgMotorEntity::QueryBody | RpgMotorEntity::QueryTarget });

	if (!list.contains(p)) {
		m_targetReached = 0;
		return;
	}


	m_lastAttack = m_currentTick;

	RpgStream::EventNpc e(RpgStream::EventNpc::EventAttack);

	if (p && p->isAlive() && p->mp() > 0 && !p->invisible())
		e.setTargetId(RpgLogicObjectMapper::getId(p->objectId()));
	else
		return;

	e.setSeq(m_npc->nextEventId());

	m_npc->jumpToSprite("cast", m_npc->facingDirection());

	m_eventList.emplace_back(std::move(e));
}



/**
 * @brief RpgNpcMpLeecher::Motor::processEventAt
 * @param tick
 */

void RpgNpcMpLeecher::Motor::processEventAt(const qint64 &tick)
{
	Q_UNUSED(tick);

	for (const RpgStream::EventNpc &e : m_incomingEventList) {
		if (e.type() == RpgStream::EventNpc::EventAttack) {
			m_npc->jumpToSprite("cast", m_npc->facingDirection());
		}
	}
}
