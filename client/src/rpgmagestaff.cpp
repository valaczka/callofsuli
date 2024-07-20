/*
 * ---- Call of Suli ----
 *
 * rpgmagestaff.cpp
 *
 * Created on: 2024. 07. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMageStaff
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

#include "rpgmagestaff.h"
#include "tiledgame.h"


RpgMageStaff::RpgMageStaff(QObject *parent)
	: TiledWeapon{WeaponMageStaff, parent}
{
	m_icon = QStringLiteral("qrc:/internal/medal/Icon.3_03.png");
}



/**
 * @brief RpgMageStaff::setFromCast
 * @param cast
 */

void RpgMageStaff::setFromCast(const RpgPlayerCharacterConfig::CastType &cast)
{
	switch (cast) {
		case RpgPlayerCharacterConfig::CastInvisible:
			setCanCast(true);
			setBulletCount(0);
			break;

		case RpgPlayerCharacterConfig::CastFireball:
			setCanCast(false);
			setBulletCount(-1);
			break;

		case RpgPlayerCharacterConfig::CastInvalid:
			setCanCast(false);
			setBulletCount(0);
			break;
	}
}



/**
 * @brief RpgMageStaff::eventUseCast
 * @param cast
 * @param target
 */

void RpgMageStaff::eventUseCast(const RpgPlayerCharacterConfig::CastType &cast, TiledObject */*target*/)
{
	TiledObject *p = m_parentObject.data();

	if (!p)
		return;

	if (TiledGame *g = p->game()) {
		/*g->playSfx(target ? QStringLiteral(":/rpg/broadsword/broadsword2.mp3") : QStringLiteral(":/rpg/broadsword/broadsword1.mp3"),
				   p->scene(), p->body()->bodyPosition());*/

		switch (cast) {
			case RpgPlayerCharacterConfig::CastInvisible:
				g->playSfx(QStringLiteral(":/rpg/broadsword/broadsword1.mp3"),
						   p->scene(), p->body()->bodyPosition());
				break;

			case RpgPlayerCharacterConfig::CastFireball:
			case RpgPlayerCharacterConfig::CastInvalid:
				break;
		}
	}
}
