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
	: TiledWeapon{WeaponShortbow, parent}
{
	m_icon = QStringLiteral("qrc:/Qaterial/Icons/bow-arrow.svg");
}



/**
 * @brief RpgShortbow::protect
 * @param weapon
 * @return
 */

bool RpgShortbow::protect(const WeaponType &/*weapon*/)
{
	return false;
}


/**
 * @brief RpgShortbow::canProtect
 * @param weapon
 * @return
 */

bool RpgShortbow::canProtect(const WeaponType &/*weapon*/) const
{
	return false;
}



/**
 * @brief RpgShortbow::createBullet
 * @return
 */


IsometricBullet *RpgShortbow::createBullet()
{
	RpgPlayer *p = player();
	if (!p) {
		LOG_CERROR("game") << "Missing player";
		return nullptr;
	}

	return RpgArrow::createBullet(p->game(), p->scene());
}



/**
 * @brief RpgShortbow::eventAttack
 */

void RpgShortbow::eventAttack()
{
	RpgPlayer *p = player();

	if (!p)
		return;

	if (TiledGame *g = p->game()) {
		g->playSfx(QStringLiteral(":/character/rpgShortbow/swish_2.wav"), p->scene(), p->body()->bodyPosition());
	}
}



/**
 * @brief RpgShortbow::player
 * @return
 */

RpgPlayer *RpgShortbow::player() const
{
	return qobject_cast<RpgPlayer*>(m_parentObject.get());
}
