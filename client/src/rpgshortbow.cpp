/*
 * ---- Call of Suli ----
 *
 * rpgshortbow.cpp
 *
 * Created on: 2024. 03. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgShortbow
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

#include "rpgshortbow.h"
#include "rpgarrow.h"
#include "rpgplayer.h"

RpgShortbow::RpgShortbow(QObject *parent)
	: RpgWeapon{RpgGameData::Weapon::WeaponShortbow, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.4_60.png");
}





/**
 * @brief RpgShortbow::eventAttack
 */

void RpgShortbow::eventAttack(TiledObject *)
{
	TiledObject *p = m_parentObject.data();

	if (!p)
		return;

	if (m_disableTimerRepeater) {
		if (m_effectPlayed)
			return;

		m_effectPlayed = true;
	}

	if (TiledGame *g = p->game()) {
		g->playSfx(QStringLiteral(":/rpg/shortbow/swish_2.mp3"), p->scene(), p->bodyPosition());
	}
}




/**
 * @brief RpgShortbowPickable::RpgShortbowPickable
 * @param parent
 */

RpgShortbowPickable::RpgShortbowPickable(RpgGame *game)
	: RpgPickableObject(RpgGameData::PickableBaseData::PickableShortbow, game)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}




/**
 * @brief RpgShortbowPickable::playerPick
 * @param player
 */

bool RpgShortbowPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	static const int num = 10;

	RpgWeapon *weapon = player->armory()->weaponAdd(RpgGameData::Weapon::WeaponShortbow);

	weapon->setBulletCount(weapon->bulletCount()+num);
	weapon->setPickedBulletCount(weapon->pickedBulletCount()+num);

	if (m_game)
		m_game->message(tr("1 shortbow gained"));

	player->armory()->setCurrentWeaponIf(weapon, RpgGameData::Weapon::WeaponHand);

	return true;
}





/**
 * @brief RpgShortbowPickable::load
 */

void RpgShortbowPickable::load()
{
	loadDefault(QStringLiteral("shortbow"));
}
