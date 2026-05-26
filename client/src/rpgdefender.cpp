/*
 * ---- Call of Suli ----
 *
 * rpgdefender.cpp
 *
 * Created on: 2026. 05. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDefender
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

#include "rpgdefender.h"





/**
 * @brief RpgDefender::RpgDefender
 * @param gameItem
 * @param pos
 */

RpgDefender::RpgDefender(RpgGameItem *gameItem, const cpVect &pos)
	: RpgEntity(gameItem, pos, 20., CP_BODY_TYPE_STATIC)
{
	m_defaultMotor = std::make_unique<RpgDefenderMotor>(this);

	filterSet(RpgGameItem::FixtureDefender, RpgGameItem::FixtureAll);
	setSensor(true);

}


/**
 * @brief RpgDefender::initialize
 */

void RpgDefender::initialize()
{
	Q_ASSERT(scene());

	TiledVisualItem *item = scene()->addVisualItem();
	m_visualItem = item;

	item->setSource(QUrl::fromLocalFile(QStringLiteral(":/rpg/time/pickable.png")));
	item->setVisible(true);
}


/**
 * @brief RpgDefender::defenderPoint
 * @return
 */

RpgDefenderPoint *RpgDefender::defenderPoint() const
{
	return m_defenderPoint;
}

RpgTower *RpgDefender::tower() const
{
	return m_tower;
}

void RpgDefender::setDefenderPoint(RpgDefenderPoint *newDefenderPoint)
{
	m_defenderPoint = newDefenderPoint;
}

void RpgDefender::setTower(RpgTower *newTower)
{
	m_tower = newTower;
}

RpgStream::Team RpgDefender::team() const
{
	return m_team;
}

void RpgDefender::setTeam(RpgStream::Team newTeam)
{
	m_team = newTeam;
}




/**
 * @brief RpgDefenderMotor::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgDefenderMotor::beforeWorldStep(const qint64 &/*tick*/, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const RpgStream::DefenderState *state = scope.getCurrentState<RpgStream::DefenderState>(entity);

	if (!state) {
		LOG_CERROR("game") << "!!!";
		return false;
	}

	m_defender->setHp(state->hp());

	return true;
}
