/*
 * ---- Call of Suli ----
 *
 * rpgbroadsword.cpp
 *
 * Created on: 2024. 07. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgBroadsword
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

#include "rpgbroadsword.h"
#include "tiledgame.h"


/**
 * @brief RpgBroadsword::RpgBroadsword
 * @param parent
 */

RpgBroadsword::RpgBroadsword(QObject *parent)
	: TiledWeapon{WeaponBroadsword, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.6_26.png");
	m_canHit = true;
	m_repeaterIdle = 375;
}



/**
 * @brief RpgBroadsword::eventAttack
 * @param target
 */

void RpgBroadsword::eventAttack(TiledObject *target)
{
	TiledObject *p = m_parentObject.data();

	if (!p)
		return;

	if (TiledGame *g = p->game()) {
		g->playSfx(target ? QStringLiteral(":/rpg/broadsword/broadsword2.mp3") : QStringLiteral(":/rpg/broadsword/broadsword1.mp3"),
				   p->scene(), p->body()->bodyPosition());
	}
}
