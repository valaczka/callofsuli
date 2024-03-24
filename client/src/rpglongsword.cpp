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
	m_icon = QStringLiteral("qrc:/Qaterial/Icons/sword.svg");
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

void RpgLongsword::eventAttack()
{
	TiledObject *p = m_parentObject.get();

	if (!p)
		return;

	if (TiledGame *g = p->game()) {
		g->playSfx(QStringLiteral(":/rpg/longsword/swing.mp3"), p->scene(), p->body()->bodyPosition());
	}
}



/**
 * @brief RpgLongswordPickable::RpgLongswordPickable
 * @param parent
 */

RpgLongswordPickable::RpgLongswordPickable(QQuickItem *parent)
	: RpgPickableObject(PickableLongsword, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}




/**
 * @brief RpgLongswordPickable::playerPick
 * @param player
 */

void RpgLongswordPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponLongsword);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgLongsword);

	weapon->setBulletCount(weapon->bulletCount()+1);

	if (m_game)
		m_game->message(tr("1 longsword gained"));

	player->armory()->setCurrentWeapon(weapon);
}





/**
 * @brief RpgLongswordPickable::playerThrow
 * @param player
 */

void RpgLongswordPickable::playerThrow(RpgPlayer *player)
{

}



/**
 * @brief RpgLongswordPickable::load
 */

void RpgLongswordPickable::load()
{
	loadDefault(QStringLiteral("longsword"));
}
