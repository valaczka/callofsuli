/*
 * ---- Call of Suli ----
 *
 * rpgcontrollight.cpp
 *
 * Created on: 2025. 05. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControlLight
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

#include "rpgcontrollight.h"


RpgControlLight::RpgControlLight(const RpgGameData::ControlBaseData &data, const RpgGameData::ControlLight::State state)
	: RpgControl<RpgGameData::ControlLight, RpgGameData::ControlBaseData>(RpgConfig::ControlLight)
	, m_state(state)
{
	m_objectId.id = data.id;
	m_objectId.ownerId = data.o;
	m_objectId.sceneId = data.s;
}



/**
 * @brief RpgControlLight::updateFromSnapshot
 * @param snapshot
 */

void RpgControlLight::updateFromSnapshot(const RpgGameData::SnapshotInterpolation<RpgGameData::ControlLight> &snapshot)
{
	if (snapshot.s1.f < 0 && snapshot.last.f < 0) {
		LOG_CERROR("scene") << "Invalid tick" << snapshot.s1.f << snapshot.s2.f << snapshot.last.f << snapshot.current;
		return;
	}

	RpgControlLight::updateFromSnapshot(snapshot.last);
}


/**
 * @brief RpgControlLight::updateFromSnapshot
 * @param snap
 */

void RpgControlLight::updateFromSnapshot(const RpgGameData::ControlLight &snap)
{
	setState(snap.st);
}



/**
 * @brief RpgControlLight::serializeThis
 * @return
 */

RpgGameData::ControlLight RpgControlLight::serializeThis() const
{
	RpgGameData::ControlLight c;

	c.sc = m_objectId.sceneId;
	c.st = m_state;

	return c;
}


/**
 * @brief RpgControlLight::updateState
 */

void RpgControlLight::updateState()
{
	if (!m_visualItem)
		return;

	// setProperty vs. setOpacity - Behavior not working with setOpacity

	m_visualItem->setProperty("opacity", m_state == RpgGameData::ControlLight::LightOn ? 1.0 : 0.0);
}



/**
 * @brief RpgControlLight::visualItem
 * @return
 */

QQuickItem *RpgControlLight::visualItem() const
{
	return m_visualItem;
}

void RpgControlLight::setVisualItem(QQuickItem *newVisualItem)
{
	m_visualItem = newVisualItem;

	if (m_visualItem)
		updateState();
}




/**
 * @brief RpgControlLight::state
 * @return
 */

const RpgGameData::ControlLight::State &RpgControlLight::state() const
{
	return m_state;
}

void RpgControlLight::setState(const RpgGameData::ControlLight::State &newState)
{
	if (m_state == newState)
		return;

	m_state = newState;

	updateState();
}
