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



const TiledObjectBody::DrawBodyStyle RpgDefenderPoint::m_style = {
	.color = QColorConstants::Svg::cyan,
	.lineWidth = 1.,
	.filled = true,
	.outlined = true
};


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



RpgTower::~RpgTower()
{

}



/**
 * @brief RpgTower::initialize
 */

void RpgTower::initialize()
{
	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgTowerMarker.qml"));
	m_markerItem->setVisible(false);
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

	if (!scope.valid(entity)) {
		LOG_CERROR("game") << "Invalid entity" << this;
		return;
	}

	const RpgStream::TowerState *state = scope.getCurrentState<RpgStream::TowerState>(entity);

	if (!state) {
		LOG_CERROR("game") << "!!!";
		return;
	}

	const bool oldActive = m_state.active();
	setState(*state);

	if (state->active() != oldActive) {
		LOG_CERROR("game") << "ACTIVE CHANGED" << state->active() << m_state.active() << m_defenderLayers;
		reloadDefenderLayersVisibility();
	}

	setCanAttack(!state->hasDefender() && state->lockedUntil() < m_gameItem->tickTimer()->currentTick() /*&&
							  state->lockId() == 0*/);
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
 * @brief RpgTower::addDefenderPoints
 * @param list
 */

void RpgTower::addDefenderPoints(const QList<RpgDefenderPoint *> &list)
{
	m_defenderPoints.append(list);

	for (RpgDefenderPoint *p : list)
		if (p) p->setTower(this);
}


/**
 * @brief RpgTower::setDefenderLayersVisible
 * @param visible
 */

void RpgTower::setDefenderLayersVisible(const bool visible)
{
	m_defenderLayers = visible;
	reloadDefenderLayersVisibility();
}


/**
 * @brief RpgTower::reloadDefenderLayersVisibility
 */

void RpgTower::reloadDefenderLayersVisibility()
{
	for (RpgDefenderPoint *p : std::as_const(m_defenderPoints)) {
		if (QQuickItem *item = p->visualItem())
			item->setVisible(m_visible && m_defenderLayers && m_state.active() && !p->defender());
	}
}



/**
 * @brief RpgTower::setVisible
 * @param visible
 */

void RpgTower::setVisible(const bool &visible)
{
	m_visible = visible;

	if (m_visualItem)
		m_visualItem->setVisible(m_visible);

	if (!m_visible) {
		m_visual.setState(RpgStream::TeamNone);
		m_markerItem->setVisible(false);
		reloadDefenderLayersVisibility();

		filterSet(RpgGameItem::FixtureInvalid, RpgGameItem::FixtureInvalid);
	} else {
		filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixtureAll);
	}

	if (m_scatterPoint.isValid())
		m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Visibility,
													  m_visible);

}




/**
 * @brief RpgTower::synchronize
 */

void RpgTower::synchronize()
{
	if (!m_visible)
		return;

	const RpgStream::Team team = m_state.active() ? m_state.team() : RpgStream::TeamNone;

	if (m_visual.state() != team)
		m_visual.setState(team);

	setColor(RpgGameItem::teamColor().value(m_state.team()));
	setLoad(m_state.load());


	if (m_scatterPoint.isValid()) {
		if (m_state.active())
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Color, m_color);
		else
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Color,
														  QColorConstants::Svg::gray);
	}

	TiledObjectBody::synchronize();
}

QQuickItem *RpgTower::markerItem() const
{
	return m_markerItem;
}

const QList<RpgDefenderPoint *> &RpgTower::defenderPoints() const
{
	return m_defenderPoints;
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


/**
 * @brief RpgTower::canAttack
 * @return
 */

bool RpgTower::canAttack() const
{
	return m_canAttack;
}

void RpgTower::setCanAttack(bool newCanAttack)
{
	if (m_canAttack == newCanAttack)
		return;
	m_canAttack = newCanAttack;
	emit canAttackChanged();
}





/**
 * @brief RpgDefenderPoint::RpgDefenderPoint
 * @param center
 * @param game
 * @param renderer
 * @param offset
 */

RpgDefenderPoint::RpgDefenderPoint(const QPointF &center, TiledGame *game,
								   Tiled::MapRenderer *renderer, const QPointF &offset)
	: TiledObjectBody(center, 25., game, renderer, CP_BODY_TYPE_STATIC, offset)
{
	m_drawBodyStyle = m_style;

	setSensor(true);
	filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixtureAll);
}

RpgDefender *RpgDefenderPoint::defender() const
{
	return m_defender;
}

void RpgDefenderPoint::setDefender(RpgDefender *newDefender)
{
	m_defender = newDefender;
}

RpgTower *RpgDefenderPoint::tower() const
{
	return m_tower;
}

void RpgDefenderPoint::setTower(RpgTower *newTower)
{
	m_tower = newTower;
}
