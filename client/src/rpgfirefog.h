/*
 * ---- Call of Suli ----
 *
 * rpgfirefog.h
 *
 * Created on: 2024. 07. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgFireFog
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

#ifndef RPGFIREFOG_H
#define RPGFIREFOG_H

#include "tiledweapon.h"



class RpgFireFogWeapon : public TiledWeapon
{
	Q_OBJECT

public:
	explicit RpgFireFogWeapon(QObject *parent = nullptr);

	bool protect(const WeaponType &) override final { return false; }
	bool canProtect(const WeaponType &) const override final { return false; }
	bool canAttack() const override final { return true; }

protected:
	IsometricBullet *createBullet(const qreal & = 0.) override final { return nullptr; }
};

#endif // RPGFIREFOG_H
