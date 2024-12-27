/*
 * ---- Call of Suli ----
 *
 * rpgcoin.cpp
 *
 * Created on: 2024. 07. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgCoinPickable
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

#include "rpgcoin.h"
#include "rpgplayer.h"

const int RpgCoinPickable::m_amount = 100;


RpgCoinPickable::RpgCoinPickable(TiledScene *scene)
	: RpgPickableObject(PickableCoin, scene)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllBlue, this));
}



/**
 * @brief RpgCoinPickable::playerPick
 * @param player
 * @return
 */

bool RpgCoinPickable::playerPick(RpgPlayer */*player*/)
{
	return true;
}


/**
 * @brief RpgCoinPickable::amount
 * @param hasQuestions
 * @return
 */

int RpgCoinPickable::amount(const bool &hasQuestions)
{
	if (hasQuestions)
		return m_amount;
	else
		return m_amount * 0.1;
}



/**
 * @brief RpgCoinPickable::load
 */

void RpgCoinPickable::load()
{
	loadDefault(QStringLiteral("coin"));
}
