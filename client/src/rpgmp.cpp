/*
 * ---- Call of Suli ----
 *
 * rpgmp.cpp
 *
 * Created on: 2024. 07. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMpPickable
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
#include "rpgplayer.h"


const int RpgMpPickable::m_amount = 25;


RpgMpPickable::RpgMpPickable(QQuickItem *parent)
	: RpgPickableObject(PickableMp, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkBall1, this));
}


/**
 * @brief RpgMpPickable::playerPick
 * @param player
 * @return
 */

bool RpgMpPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	if (player->config().cast == RpgPlayerCharacterConfig::CastInvalid) {
		if (m_game)
			m_game->messageColor(tr("No superpower"), QColor::fromRgbF(0.8, 0., 0.));
		return false;
	}

	return pick(player, m_amount);
}



/**
 * @brief RpgMpPickable::pick
 * @param player
 * @param game
 * @param amount
 * @return
 */

bool RpgMpPickable::pick(RpgPlayer *player, const int &amount)
{
	if (!player)
		return false;

	TiledGame *game = player->game();

	player->setMp(player->mp() + amount);

	if (game)
		game->messageColor(tr("+%1 MP gained").arg(amount), QColor::fromString(QStringLiteral("#F06292")));

	return true;
}



/**
 * @brief RpgMpPickable::load
 */

void RpgMpPickable::load()
{
	loadDefault(QStringLiteral("mp"));
}






