/*
 * ---- Call of Suli ----
 *
 * rpgdagger.cpp
 *
 * Created on: 2024. 07. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgDagger
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

#include "rpgdagger.h"
#include "rpgplayer.h"

RpgDagger::RpgDagger(QObject *parent)
	: TiledWeapon{WeaponDagger, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.8_21.png");
	m_canHit = true;
}



/**
 * @brief RpgDagger::eventAttack
 * @param target
 */

void RpgDagger::eventAttack(TiledObject *target)
{
	TiledObject *p = m_parentObject.data();

	if (!p)
		return;

	if (TiledGame *g = p->game(); g && target) {
		g->playSfx(QStringLiteral(":/rpg/dagger/dagger.mp3"), p->scene(), p->body()->bodyPosition());
	}
}


/**
 * @brief RpgDaggerPickable::RpgDaggerPickable
 * @param parent
 */

RpgDaggerPickable::RpgDaggerPickable(QQuickItem *parent)
	: RpgPickableObject(PickableDagger, parent)
{
	m_activateEffect.reset(new TiledEffectSpark(TiledEffectSpark::SparkAllOrange, this));
}


/**
 * @brief RpgDaggerPickable::playerPick
 * @param player
 * @return
 */

bool RpgDaggerPickable::playerPick(RpgPlayer *player)
{
	if (!player)
		return false;

	TiledWeapon *weapon = player->armory()->weaponFind(TiledWeapon::WeaponLongsword);

	if (!weapon)
		weapon = player->armory()->weaponAdd(new RpgDagger);

	weapon->setBulletCount(weapon->bulletCount()+1);
	weapon->setPickedBulletCount(weapon->pickedBulletCount()+1);

	if (m_game)
		m_game->message(tr("1 dagger gained"));

	player->armory()->setCurrentWeaponIf(weapon, TiledWeapon::WeaponHand);

	return true;
}


/**
 * @brief RpgDaggerPickable::load
 */

void RpgDaggerPickable::load()
{
	loadDefault(QStringLiteral("dagger"));
}
