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

#include <libtiled/map.h>
#include <libtiled/objectgroup.h>
#include "rpgdefender.h"
#include "rpgdefenderfog.h"
#include "rpgplayer.h"








/**
 * @brief RpgDefender::RpgDefender
 * @param gameItem
 * @param pos
 */

RpgDefender::RpgDefender(RpgGameItem *gameItem, const Rpg::DefenderObject &config)
	: RpgEntity(gameItem, config.pos, 20., CP_BODY_TYPE_STATIC)
	, m_config(config)
	, m_visual(StateHidden)
{
	m_defaultMotor = std::make_unique<RpgDefenderMotor>(this);

	filterSet(RpgGameItem::FixtureDefender, RpgGameItem::FixtureAll);
	setSensor(true);

	m_visual.setBasePosition(TiledObjectBody::toPointF(config.pos));
}


/**
 * @brief RpgDefender::~RpgDefender
 */

RpgDefender::~RpgDefender()
{
	m_visual.clear();

	if (!m_scene) {
		LOG_CERROR("game") << "Missing scene" << this;
		return;
	}

	for (TiledQuick::TileLayerItem *layer : std::as_const(m_layerItems)) {
		m_scene->removeVisualItem(layer);
		layer->deleteLater();
	}

	m_layerItems.clear();
}



/**
 * @brief RpgDefender::createDefender
 * @param npc
 * @param gameItem
 * @param scene
 * @param pos
 * @return
 */

RpgDefender *RpgDefender::createDefender(const Rpg::DefenderObject &defender, RpgGameItem *gameItem, TiledScene *scene)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(scene);

	QString common;

	switch (defender.type) {
		case RpgStream::BaseDefenderObject::Fog:
			return gameItem->createObject<RpgDefenderFog>(RpgLogicObjectMapper::toObjectId(defender.idTag),
														  scene, gameItem,
														  defender);

		case RpgStream::BaseDefenderObject::Pulse:
			common = QStringLiteral("def_pulse.tmx");
			break;

		case RpgStream::BaseDefenderObject::Multiplier1:


		case RpgStream::BaseDefenderObject::None:
			LOG_CERROR("game") << "Invalid defender type" << defender.type;
			break;
	}

	if (!common.isEmpty())
		return gameItem->createObject<RpgDefenderCommon>(RpgLogicObjectMapper::toObjectId(defender.idTag),
														 scene,
														 common, gameItem, defender);

	return nullptr;
}





/**
 * @brief RpgDefender::updateVisibility
 */

void RpgDefender::updateVisibility()
{
	if (!m_rpgGame || !m_rpgGame->controlledPlayer()) {
		LOG_CERROR("game") << "Invalid game or player";
		return;
	}

	if (m_visibleToAll || m_team == m_rpgGame->controlledPlayer()->team())
		m_visual.setState(isAlive() ? (m_hasTarget ? StateActive : StateNormal) : StateDestroyed);
	else
		m_visual.setState(StateHidden);

	if (TiledVisualItem *item = m_visual.imageItem())
		item->setVisible(m_visual.state() != StateHidden);

	if (m_markerItem)
		m_markerItem->setVisible(m_visual.state() != StateHidden && m_marked);
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


const Rpg::DefenderObject &RpgDefender::config() const
{
	return m_config;
}

bool RpgDefender::visibleToAll() const
{
	return m_visibleToAll;
}

void RpgDefender::setVisibleToAll(bool newVisibleToAll)
{
	if (m_visibleToAll == newVisibleToAll)
		return;
	m_visibleToAll = newVisibleToAll;
	emit visibleToAllChanged();
	updateVisibility();
}

bool RpgDefender::hasTarget() const
{
	return m_hasTarget;
}

void RpgDefender::setHasTarget(bool newHasTarget)
{
	if (m_hasTarget == newHasTarget)
		return;
	m_hasTarget = newHasTarget;
	emit hasTargetChanged();

	updateVisibility();
}


/**
 * @brief RpgDefender::setMarked
 * @param marked
 */

void RpgDefender::setMarked(const bool &marked)
{
	m_marked = marked;

	updateVisibility();
}


/**
 * @brief RpgDefender::onAlive
 */

void RpgDefender::onAlive()
{
	updateVisibility();
}


/**
 * @brief RpgDefender::onDead
 */

void RpgDefender::onDead()
{
	updateVisibility();
}



/**
 * @brief RpgDefender::updateColor
 */

void RpgDefender::updateColor()
{
	QColor color = m_rpgGame->getColor(m_team);

	if (m_markerItem) {
		m_markerItem->setProperty("progressBarColor", color);
		m_markerItem->setProperty("labelColor", color);
	}

	if (TiledVisualItem *item = m_visual.imageItem()) {
		item->setGlowColor(color);
	}

	updateVisibility();
}



/**
 * @brief RpgDefender::loadFromCommonMap
 * @param name
 * @return
 */

bool RpgDefender::loadFromCommonMap(const QString &name)
{
	m_scene = scene();

	Q_ASSERT(m_scene);

	const Tiled::Map *map = m_rpgGame->commonMap(name);
	Tiled::MapRenderer *renderer = m_rpgGame->commonRenderer(name);

	if (!map) {
		LOG_CERROR("game") << "Invalid map";
		return false;
	}

	if (!renderer) {
		LOG_CERROR("game") << "Invalid renderer";
		return false;
	}

	static const QHash<QString, State> stateHash = {
		{ "active", StateActive },
		{ "destroyed", StateDestroyed },
		{ "normal", StateNormal }
	};

	std::optional<QPointF> ref;

	for (Tiled::Layer *layer : map->layers()) {
		if (Tiled::ObjectGroup *gr = layer->asObjectGroup()) {
			for (Tiled::MapObject *object : std::as_const(gr->objects())) {
				if (object->className() == QStringLiteral("base")) {
					ref = renderer->pixelToScreenCoords(object->position())+gr->totalOffset();
				}
			}
		}
	}

	TiledQuick::TileLayerItem *layerNormal = nullptr;
	TiledQuick::TileLayerItem *layerActive = nullptr;

	for (Tiled::Layer *layer : map->layers()) {
		if (Tiled::TileLayer *tl = layer->asTileLayer()) {
			TiledQuick::TileLayerItem *layerItem = m_scene->addTileLayer(tl, renderer);

			const State st = stateHash.value(tl->className(), StateNormal);

			if (st == StateNormal)
				layerNormal = layerItem;
			else if (st == StateActive)
				layerActive = layerItem;

			QPointF r;
			if (ref.has_value())
				r = ref.value();
			else {
				r.setX(layerItem->width()/2);
				r.setY(layerItem->height()/2);
			}

			layerItem->setZ(scene()->getDynamicZ(layerItem->position() + m_visual.basePosition()));
			layerItem->setPosition(layerItem->position() + m_visual.basePosition() - r);

			m_layerItems.append(layerItem);
			m_visual.addLayer(st, layerItem);
		}
	}

	// Duplicate Active and Normal state if missing

	if (layerNormal && !layerActive)
		m_visual.addLayer(StateActive, layerNormal);
	else if (!layerNormal && layerActive)
		m_visual.addLayer(StateNormal, layerActive);

	addMarkerItem();

	updateColor();

	return true;
}





/**
 * @brief RpgDefender::addMarkerItem
 */

void RpgDefender::addMarkerItem()
{
	m_markerItem = createMarkerItem(QStringLiteral("qrc:/RpgDefenderMarker.qml"));
	m_markerItem->setVisible(false);
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
		LOG_CERROR("game") << "Invalid defender state";
		return false;
	}

	m_defender->setHp(state->hp());
	m_defender->setHasTarget(state->targetId() > 0);
	m_defender->setVisibleToAll(state->visible());

	return true;
}
