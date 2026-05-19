/*
 * ---- Call of Suli ----
 *
 * rpgtower.cpp
 *
 * Created on: 2026. 05. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgTower
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

#include "rpgtower.h"




/**
 * @brief RpgTower::RpgTower
 * @param gameItem
 * @param pos
 * @param radius
 */

RpgTower::RpgTower(RpgGameItem *gameItem, Tiled::MapObject *object, Tiled::MapRenderer *renderer)
	: TiledObject(object, gameItem, renderer, CP_BODY_TYPE_STATIC)
	, m_gameItem(gameItem)
	, m_visual(RpgStream::TeamNone)
{
	filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixtureAll);
	setSensor(true);
}



/**
 * @brief RpgTower::initialize
 */

void RpgTower::initialize()
{
	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgTowerMarker.qml"));
	setDisplayName("TOWER 200");
}



/**
 * @brief RpgTower::worldStep
 */

void RpgTower::worldStep()
{
	TiledObjectBody::worldStep();


	Rpg::RpgLogicScope scope = m_gameItem->game()->rpgLogicClient()->getScope();
	auto entity = scope.entityFromIdTag(RpgLogicObjectMapper::getId(objectId()));

	if (entity == entt::null) {
		LOG_CERROR("game") << "Invalid entity" << this;
		return;
	}

	const RpgStream::TowerState *state = scope.getCurrentState<RpgStream::TowerState>(entity);

	if (!state) {
		LOG_CERROR("game") << "!!!";
		return;
	}

	setState(*state);
}




/**
 * @brief RpgTower::addLayers
 * @param layers
 */

void RpgTower::addLayers(const QMultiMap<RpgStream::Team, TiledQuick::TileLayerItem *> &layers)
{
	for (const auto &[layer, item] : layers.asKeyValueRange())
		m_visual.addLayer(layer, item);

	m_visual.refresh();
}


/**
 * @brief RpgTower::setVisualItem
 * @param item
 */

void RpgTower::setVisualItem(TiledVisualItem *item)
{
	m_visualItem = item;
	emit visualItemChanged();
}




/**
 * @brief RpgTower::synchronize
 */

void RpgTower::synchronize()
{
	const RpgStream::Team team = m_state.active() ? m_state.team() : RpgStream::TeamNone;

	if (m_visual.state() != team)
		m_visual.setState(team);

	setColor(RpgGameItem::teamColor().value(m_state.team()));
	setLoad(m_state.load());

	TiledObjectBody::synchronize();
}

RpgGameItem *RpgTower::gameItem() const
{
	return m_gameItem;
}


/**
 * @brief RpgTower::state
 * @return
 */

const RpgStream::TowerState &RpgTower::state() const
{
	return m_state;
}

void RpgTower::setState(const RpgStream::TowerState &newState)
{
	m_state = newState;
}


/**
 * @brief RpgTower::load
 * @return
 */

int RpgTower::load() const
{
	return m_load;
}

void RpgTower::setLoad(int newLoad)
{
	if (m_load == newLoad)
		return;
	m_load = newLoad;
	emit loadChanged();
}


/**
 * @brief RpgTower::color
 * @return
 */

QColor RpgTower::color() const
{
	return m_color;
}

void RpgTower::setColor(const QColor &newColor)
{
	if (m_color == newColor)
		return;
	m_color = newColor;
	emit colorChanged();
}
