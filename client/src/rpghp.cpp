/*
 * ---- Call of Suli ----
 *
 * rpghp.cpp
 *
 * Created on: 2024. 03. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgHpPickable
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

#include "rpghp.h"
#include "rpgplayer.h"


RpgHpPickable::RpgHpPickable(QQuickItem *parent)
	: RpgPickableObject(PickableHp, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}


/**
 * @brief RpgHpPickable::playerPick
 * @param player
 */

bool RpgHpPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	player->setHp(player->hp()+1);

	return true;
}





/**
 * @brief RpgHpPickable::load
 */

void RpgHpPickable::load()
{
	loadDefault(QStringLiteral("hp"));
}

