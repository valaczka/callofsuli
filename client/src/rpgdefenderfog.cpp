/*
 * ---- Call of Suli ----
 *
 * rpgdefenderfog.cpp
 *
 * Created on: 2026. 07. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDefenderFog
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

#include "rpgdefenderfog.h"
#include "application.h"
#include "tiledeffectfog.h"
#include "rpgplayer.h"



/**
 * @brief RpgDefenderFog::RpgDefenderFog
 * @param gameItem
 * @param pos
 */

RpgDefenderFog::RpgDefenderFog(RpgGameItem *gameItem, const Rpg::DefenderObject &config)
	: RpgDefender(gameItem, config)
{

}


/**
 * @brief RpgDefenderFog::~RpgDefenderFog
 */

RpgDefenderFog::~RpgDefenderFog()
{
	if (m_image) {
		m_image->stop();
		m_image->deleteLater();
		m_image = nullptr;
	}
}




/**
 * @brief RpgDefenderFog::initialize
 */

void RpgDefenderFog::initialize()
{
	m_scene = scene();

	Q_ASSERT(m_scene);

	TiledVisualItem *item = m_scene->addVisualItem();
	m_visualItem = item;

	m_visual.setImageItem(item);


	m_visual.addSource(StateActive, QUrl::fromLocalFile(QStringLiteral(":/rpg/time/pickable.png")));
	m_visual.addSource(StateDestroyed, QUrl::fromLocalFile(QStringLiteral(":/rpg/key/pickable.png")));


	QQmlComponent component(Application::instance()->engine(), QStringLiteral("qrc:/RpgDefenderFogVisual.qml"), this);

	m_image = qobject_cast<TiledEffectFog*>(component.create());

	if (!m_image) {
		LOG_CERROR("scene") << "QML item create error" << component.errorString();
		return;
	}

	m_image->setParentItem(m_scene);
	m_image->setGame(m_rpgGame->gameItem());
	m_image->setSize(QSizeF(cfgDefenderFog.radius*2, cfgDefenderFog.radius*2));

	m_image->setPosition(m_visual.basePosition() - QPointF(cfgDefenderFog.radius, cfgDefenderFog.radius));

	float z = m_scene->getDynamicZ(m_visual.basePosition());

	m_image->setZ(z+0.9);

	onAlive();

	//m_visualItem->setZ(z);

	addMarkerItem();

	updateColor();

}





/**
 * @brief RpgDefenderFog::updateVisibility
 */

void RpgDefenderFog::updateVisibility()
{
	if (!m_image)
		return;

	if (!isAlive()) {
		m_image->setVisible(false);
		m_visual.setState(StateDestroyed);
		return;
	}

	if (!m_rpgGame || !m_rpgGame->controlledPlayer()) {
		LOG_CERROR("game") << "Invalid game or player";
		return;
	}

	if (m_visibleToAll || m_team == m_rpgGame->controlledPlayer()->team()) {
		m_image->setVisible(true);
		m_visual.setState(StateActive);
	} else {
		m_image->setVisible(false);
		m_visual.setState(StateHidden);
	}

	if (m_team == m_rpgGame->controlledPlayer()->team())
		m_image->setOpacity(0.7);
	else
		m_image->setOpacity(1.0);

	if (m_markerItem)
		m_markerItem->setVisible(m_visual.state() != StateHidden && m_marked && isAlive());
}



/**
 * @brief RpgDefenderFog::updateColor
 */

void RpgDefenderFog::updateColor()
{
	RpgDefender::updateColor();

	if (m_image)
		m_image->setProperty("tintColor", m_rpgGame->getColor(m_team).lighter());
}































