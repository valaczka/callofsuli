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
	: TiledWeapon{WeaponSword, parent}
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
	RpgPlayer *p = player();

	if (!p)
		return;

	if (TiledGame *g = p->game()) {
		g->playSfx(QStringLiteral(":/character/rpgLongsword/swing.wav"), p->scene(), p->body()->bodyPosition());
	}
}


/**
 * @brief RpgLongsword::player
 * @return
 */

RpgPlayer *RpgLongsword::player() const
{
	return qobject_cast<RpgPlayer*>(m_parentObject.get());
}
