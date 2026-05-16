/*
 * ---- Call of Suli ----
 *
 * rpgmp.cpp
 *
 * Created on: 2026. 05. 15.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMp
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

#include "rpgmp.h"
#include "tiledvisualitem.h"


RpgMp::RpgMp(RpgGameItem *gameItem, const cpVect &emitterCenter, const cpVect &pos, const quint32 &duration)
	: RpgObject(gameItem, emitterCenter, 15., CP_BODY_TYPE_KINEMATIC)
{
	m_defaultMotor = std::make_unique<RpgEasingMotor>(this,
											  pos,
											  gameItem->tickTimer()->currentTick() + duration,
											  emitterCenter,
											  gameItem->tickTimer()->currentTick()
											  );


	filterSet(RpgGameItem::FixtureInvalid, RpgGameItem::FixtureInvalid);
	setSensor(true);
}


/**
 * @brief RpgMp::initialize
 */

void RpgMp::initialize()
{
	Q_ASSERT(scene());

	TiledVisualItem *item = scene()->addVisualItem();
	m_visualItem = item;

	item->setSource(QUrl::fromLocalFile(QStringLiteral(":/rpg/mp/pickable.png")));
	item->setVisible(true);
}




/**
 * @brief RpgMp::onMotorStepped
 */

void RpgMp::onMotorStepped()
{
	RpgEasingMotor *m = dynamic_cast<RpgEasingMotor*>(m_defaultMotor.get());

	if (m && m->finished() && !m_activated) {
		cpBodySetType(body(), CP_BODY_TYPE_STATIC);
		filterSet(RpgGameItem::FixtureControl, RpgGameItem::FixturePlayerBody | RpgGameItem::FixtureSensor | RpgGameItem::FixturePlayerTarget);

		m_activated = true;
	}
}


