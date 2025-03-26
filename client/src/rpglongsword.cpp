/*
 * ---- Call of Suli ----
 *
 * rpglongsword.cpp
 *
 * Created on: 2024. 03. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLongsword
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

#include "rpglongsword.h"
#include "rpgplayer.h"


/**
 * @brief RpgLongsword::RpgLongsword
 * @param parent
 */

RpgLongsword::RpgLongsword(QObject *parent)
	: TiledWeapon{WeaponLongsword, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.6_98.png");
	m_canHit = true;
}


/**
 * @brief RpgLongsword::protect
 * @param weapon
 * @return
 */

bool RpgLongsword::protect(const WeaponType &)
{
	return false;
}


/**
 * @brief RpgLongsword::canProtect
 * @param weapon
 * @return
 */

bool RpgLongsword::canProtect(const WeaponType &) const
{
	return false;
}


/**
 * @brief RpgLongsword::eventAttack
 */

void RpgLongsword::eventAttack(TiledObject *target)
{
	TiledObject *p = m_parentObject.data();

	if (!p)
		return;

	if (TiledGame *g = p->game()) {
		g->playSfx(target ? QStringLiteral(":/rpg/longsword/swing2.mp3") : QStringLiteral(":/rpg/longsword/swing.mp3"),
				   p->scene(), p->bodyPosition());
	}
}



/**
 * @brief RpgLongswordPickable::RpgLongswordPickable
 * @param parent
 */

RpgLongswordPickable::RpgLongswordPickable(TiledScene *scene)
	: RpgPickableObject(RpgGameData::PickableBaseData::PickableLongsword, scene)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}




/**
 * @brief RpgLongswordPickable::playerPick
 * @param player
 */

bool RpgLongswordPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponLongsword);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgLongsword);

	weapon->setBulletCount(weapon->bulletCount()+1);
	weapon->setPickedBulletCount(weapon->pickedBulletCount()+1);

	if (m_game)
		m_game->message(tr("1 sword gained"));

	player->armory()->setCurrentWeaponIf(weapon, TiledWeapon::WeaponHand);

	return true;
}






/**
 * @brief RpgLongswordPickable::load
 */

void RpgLongswordPickable::load()
{
	loadDefault(QStringLiteral("longsword"));
}
