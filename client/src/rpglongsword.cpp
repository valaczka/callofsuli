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
#include "tiledgame.h"


/**
 * @brief RpgLongsword::RpgLongsword
 * @param parent
 */

RpgLongsword::RpgLongsword(QObject *parent)
	: RpgWeapon{RpgGameData::Weapon::WeaponLongsword, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.6_98.png");
	m_canHit = true;
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
				   p->scene(), p->bodyPositionF());
	}
}


