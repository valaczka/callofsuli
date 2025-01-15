/*
 * ---- Call of Suli ----
 *
 * rpgkeypickable.cpp
 *
 * Created on: 2024. 04. 14.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgKeyPickable
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

#include "rpgkeypickable.h"



RpgKeyPickable::RpgKeyPickable(TiledScene *scene)
	: RpgPickableObject(RpgGameData::Pickable::PickableKey, scene)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkBall1, this));
}


/**
 * @brief RpgKeyPickable::playerPick
 * @return
 */

bool RpgKeyPickable::playerPick(RpgPlayer *)
{
	return true;
}


/**
 * @brief RpgKeyPickable::load
 */

void RpgKeyPickable::load()
{
	loadDefault(QStringLiteral("key"));
}


