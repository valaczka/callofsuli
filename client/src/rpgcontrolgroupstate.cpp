/*
 * ---- Call of Suli ----
 *
 * rpgcontrolgroupstate.cpp
 *
 * Created on: 2024. 10. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlGroupState
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

#include "rpgcontrolgroupstate.h"
#include <libtiled/grouplayer.h>
#include <libtiled/imagelayer.h>
#include "tiledvisualitem.h"

RpgControlGroupState::RpgControlGroupState(const Type &type, RpgGame *game, TiledScene *scene, Tiled::GroupLayer *group)
	: RpgControlGroup(type, game, scene)
{
	Q_ASSERT(game);
	Q_ASSERT(scene);
	Q_ASSERT(group);
	//Q_ASSERT(renderer);

	setBasePosition(group->position()+group->offset());
}



/**
 * @brief RpgControlGroupState::states
 * @return
 */

const QVector<RpgControlGroupState::State> &RpgControlGroupState::states() const
{
	return m_states;
}


/**
 * @brief RpgControlGroupState::state
 * @param id
 * @return
 */

QVector<RpgControlGroupState::State>::const_iterator RpgControlGroupState::cstate(const int &id) const
{
	if (id == -1)
		return std::find_if(m_states.cbegin(), m_states.cend(), [this](const State &s){
			return s.id == m_currentState;
		});
	else
		return std::find_if(m_states.cbegin(), m_states.cend(), [&id](const State &s){
			return s.id == id;
		});
}


/**
 * @brief RpgControlGroupState::state
 * @param id
 * @return
 */

QVector<RpgControlGroupState::State>::iterator RpgControlGroupState::state(const int &id)
{
	if (id == -1)
		return std::find_if(m_states.begin(), m_states.end(), [this](const State &s){
			return s.id == m_currentState;
		});
	else
		return std::find_if(m_states.begin(), m_states.end(), [&id](const State &s){
			return s.id == id;
		});
}


/**
 * @brief RpgControlGroupState::stateAdd
 * @param state
 * @return
 */

bool RpgControlGroupState::stateAdd(const State &state)
{
	if (std::find_if(m_states.cbegin(), m_states.cend(), [&state](const State &s){
					 return s.id == state.id;
}) != m_states.cend()) {
		LOG_CERROR("scene") << "State already exists" << state.id;
		return false;
	}

	m_states.append(state);

	return true;
}


/**
 * @brief RpgControlGroupState::refreshVisualItem
 */

void RpgControlGroupState::refreshVisualItem() const
{
	if (!m_visualItem)
		return;

	const auto &ptr = cstate();

	if (ptr == m_states.cend()) {
		m_visualItem->setSource({});
		m_visualItem->setVisible(false);
	} else {
		m_visualItem->setSource(ptr->image);
		m_visualItem->setVisible(true);
		m_visualItem->setPosition(m_basePosition + ptr->relativePosition);
	}
}





/**
 * @brief RpgControlGroupState::basePosition
 * @return
 */

QPointF RpgControlGroupState::basePosition() const
{
	return m_basePosition;
}

void RpgControlGroupState::setBasePosition(QPointF newBasePosition)
{
	if (m_basePosition == newBasePosition)
		return;
	m_basePosition = newBasePosition;
}




/**
 * @brief RpgControlGroupState::currentState
 * @return
 */

int RpgControlGroupState::currentState() const
{
	return m_currentState;
}

void RpgControlGroupState::setCurrentState(int newCurrentState)
{
	if (m_currentState == newCurrentState)
		return;
	m_currentState = newCurrentState;

	refreshVisualItem();
}




/**
 * @brief RpgControlGroupState::visualItem
 * @return
 */

TiledVisualItem *RpgControlGroupState::visualItem() const
{
	return m_visualItem;
}

void RpgControlGroupState::setVisualItem(TiledVisualItem *newVisualItem)
{
	if (m_visualItem == newVisualItem)
		return;
	m_visualItem = newVisualItem;

	refreshVisualItem();
}




/**
 * @brief RpgControlGroupState::createVisualItem
 * @return
 */

TiledVisualItem *RpgControlGroupState::createVisualItem(Tiled::GroupLayer *layer)
{
	if (!m_scene)
		return nullptr;

	TiledVisualItem *item = m_scene->addVisualItem();

	Q_ASSERT(item);

	item->setName(layer->name());

	if (layer && layer->hasProperty(QStringLiteral("z"))) {
		item->setZ(layer->property(QStringLiteral("z")).toInt());
	} else {
		item->setZ(0);
	}

	setVisualItem(item);

	return item;
}
