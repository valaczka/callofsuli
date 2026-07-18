/*
 * ---- Call of Suli ----
 *
 * rpgcontrol.cpp
 *
 * Created on: 2026. 07. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgControl
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

#include "rpgcontrol.h"




/**
 * @brief RpgControl::RpgControl
 * @param gameItem
 * @param config
 */

RpgControl::RpgControl(RpgGameItem *gameItem, const Rpg::Control &config)
	: RpgObject(gameItem, config.pos, 20., CP_BODY_TYPE_STATIC)
	, m_config(config)
{
	m_defaultMotor = std::make_unique<RpgControlMotor>(this);

	filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixtureAll);
	setSensor(true);

	setSubZ(0.5);
}



/**
 * @brief RpgControl::~RpgControl
 */

RpgControl::~RpgControl()
{
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
 * @brief RpgControl::createControl
 * @param config
 * @param gameItem
 * @param scene
 * @return
 */

RpgControl *RpgControl::createControl(const Rpg::Control &config, RpgGameItem *gameItem, TiledScene *scene)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(scene);

	QString common;
	QHash<StateCommon, QString> baseImage;

	switch (config.type) {
		case RpgStream::ControlData::Chest:
			common = QStringLiteral("def_pulse.tmx");
			/*baseImage = {
				{ StateNormal, QStringLiteral(":/rpg/time/pickable.png") },
				{ StateActive, QStringLiteral(":/rpg/time/pickable.png") },
			};*/
			break;

		case RpgStream::ControlData::None:
			LOG_CERROR("game") << "Invalid control type" << config.type;
			break;
	}

	if (!common.isEmpty())
		return gameItem->createObject<RpgControlCommon>(RpgLogicObjectMapper::toObjectId(config.idTag),
														 scene,
														 common, gameItem, config, baseImage,
														"Chest #1");

	return nullptr;
}





/**
 * @brief RpgControl::config
 * @return
 */

const Rpg::Control &RpgControl::config() const
{
	return m_config;
}


/**
 * @brief RpgControl::isAlive
 * @return
 */

bool RpgControl::isAlive() const
{
	return m_isAlive;
}

void RpgControl::setIsAlive(bool newIsAlive)
{
	if (m_isAlive == newIsAlive)
		return;
	m_isAlive = newIsAlive;
	emit isAliveChanged();

	setSubZ(m_isAlive ? 0.5 : 0.0);
}




/**
 * @brief RpgControlMotor::beforeWorldStep
 * @param tick
 * @param entity
 * @return
 */

bool RpgControlMotor::beforeWorldStep(const qint64 &/*tick*/, entt::entity &entity)
{
	Rpg::RpgLogicScope scope = m_game->rpgLogicClient()->getScope();

	const RpgStream::ControlState *state = scope.getCurrentState<RpgStream::ControlState>(entity);

	if (!state) {
		LOG_CERROR("game") << "Invalid control state";
		return false;
	}

	m_control->setIsAlive(state->isAlive());

	return beforeWorldStepControl(*state, entity);
}
