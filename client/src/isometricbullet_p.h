/*
 * ---- Call of Suli ----
 *
 * isometricbullet_p.h
 *
 * Created on: 2024. 07. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef ISOMETRICBULLET_P_H
#define ISOMETRICBULLET_P_H

#include "tiledweapon.h"

/**
 * @brief The IsometricBulletPrivate class
 */

class IsometricBulletPrivate {
public:
	TiledWeapon *fromWeapon() const { return m_fromWeapon.get(); }
	const TiledWeapon::WeaponType &fromWeaponType() const { return m_fromWeaponType; }
	TiledObject *owner() const { return m_owner.get(); }

private:
	IsometricBulletPrivate()
	{}
	~IsometricBulletPrivate() = default;

	QPointer<TiledWeapon> m_fromWeapon;
	QPointer<TiledObject> m_owner;
	TiledWeapon::WeaponType m_fromWeaponType = TiledWeapon::WeaponInvalid;

	friend class IsometricBullet;
};



#endif // ISOMETRICBULLET_P_H
