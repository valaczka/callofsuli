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
#include <rpgconfig.h>
#include "rpgdefender.h"
#include "rpgplayer.h"
#include <QPointer>



const int RpgTower::m_maxLockTime = AbstractGame::TickTimer::tickToMsec(CFG_TOWER_LOCK)/1000.;


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
	, m_visual(TowerTeamNone)
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
		reloadDefenderLayersVisibility();
	}


	const quint64 tick = m_gameItem->tickTimer()->currentTick();

	if (tick > 0 && state->lockedUntil() > tick) {
		setLockTime((AbstractGame::TickTimer::tickToMsec(state->lockedUntil() - tick))/1000);
		setDisplayName(tr("LOCKED %1s").arg(m_lockTime));
	} else {
		setLockTime(0);
		setDisplayName(QStringLiteral("POWER GENERATOR"));
	}

	setCanAttack(!state->hasDefender() && state->lockedUntil() < tick);


	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	if (!mapper) {
		LOG_CERROR("game") << "Missing RpgLogicObjectMapper";
		return;
	}

	QList<QPointer<RpgDefender>> defenders;

	defenders.reserve(state->defenders().size());

	for (const RpgStream::FullMapTag &tag : state->defenders()) {
		RpgDefender *def = qobject_cast<RpgDefender*>(mapper->get(tag.tagId()));

		if (!def) {
			LOG_CERROR("game") << "Invalid defender" << tag.tagId();
			continue;
		}

		defenders.append(def);
	}

	m_defenders.swap(defenders);
}




/**
 * @brief RpgTower::addLayers
 * @param layers
 */

void RpgTower::addLayers(const QMultiMap<RpgStream::Team, TiledQuick::TileLayerItem *> &layers)
{
	static const QHash<RpgStream::Team, TowerTeam> converter = {
		{ RpgStream::TeamA, TowerTeamPlayer },
		{ RpgStream::TeamB, TowerTeamOpponent },
	};

	for (const auto &[layer, item] : layers.asKeyValueRange())
		m_visual.addLayer(converter.value(layer, TowerTeamNone), item);

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

	if (!item)
		return;

	connect(item, &TiledVisualItem::glowEnabledChanged, this, [this, item](){
		m_markerItem->setVisible(item->glowEnabled());
	});
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
 * @param team
 */

void RpgTower::setDefenderLayersVisible(const RpgStream::Team &team)
{
	m_defenderLayers = team;
	reloadDefenderLayersVisibility();
}


/**
 * @brief RpgTower::reloadDefenderLayersVisibility
 */

void RpgTower::reloadDefenderLayersVisibility()
{
	for (RpgDefenderPoint *p : std::as_const(m_defenderPoints)) {
		if (QQuickItem *item = p->visualItem())
			item->setVisible(m_visible && m_defenderLayers == m_state.team() && m_state.active() && !p->defender());
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
		m_visual.setState(TowerTeamNone);
		reloadDefenderLayersVisibility();

		filterSet(RpgGameItem::FixtureInvalid, RpgGameItem::FixtureInvalid);
	} else {
		filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixtureAll);
	}

	if (m_scatterPoint.isValid())
		m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Visibility,
													  m_visible);

	for (TiledObjectBody *b : std::as_const(m_excludeList)) {
		b->filterSet(m_visible ? RpgGameItem::FixtureExcluded : RpgGameItem::FixtureInvalid,
					 m_visible ? RpgGameItem::FixtureAll : RpgGameItem::FixtureInvalid);
	}
}




/**
 * @brief RpgTower::synchronize
 */

void RpgTower::synchronize()
{
	if (!m_visible || !m_gameItem || !m_gameItem->game() || ! m_gameItem->game()->controlledPlayer())
		return;

	const TowerTeam team = m_state.active() ?
							   (m_gameItem->game()->controlledPlayer()->team() == m_state.team() ?
									TowerTeamPlayer : TowerTeamOpponent) :
							   TowerTeamNone;

	if (m_visual.state() != team)
		m_visual.setState(team);

	setColor(m_gameItem->game()->getColor(m_state.team(), RpgGame::colorNeutral()));
	setLoad(m_state.load());

	if (TiledVisualItem *item = qobject_cast<TiledVisualItem*>(m_visualItem)) {
		item->setGlowColor(m_gameItem->game()->getColor(m_state.team(), RpgGame::colorGlow()));
	}

	if (m_scatterPoint.isValid()) {
		if (m_state.active())
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Color, m_color);
		else
			m_scatterPoint.scatter->setPointConfiguration(m_scatterPoint.index, QXYSeries::PointConfiguration::Color,
														  QColorConstants::Svg::gray);
	}

	TiledObjectBody::synchronize();
}


/**
 * @brief RpgTower::excludeList
 * @return
 */

const QList<TiledObjectBody *> &RpgTower::excludeList() const
{
	return m_excludeList;
}


void RpgTower::setExcludeList(const QList<TiledObjectBody *> &newExcludeList)
{
	m_excludeList = newExcludeList;
}


/**
 * @brief RpgTower::defenders
 * @return
 */

const QList<QPointer<RpgDefender> > &RpgTower::defenders() const
{
	return m_defenders;
}


/**
 * @brief RpgTower::maxLockTime
 * @return
 */

int RpgTower::maxLockTime()
{
	return m_maxLockTime;
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

int RpgTower::lockTime() const
{
	return m_lockTime;
}

void RpgTower::setLockTime(int newLockTime)
{
	if (m_lockTime == newLockTime)
		return;
	m_lockTime = newLockTime;
	emit lockTimeChanged();
}
