/*
 * ---- Call of Suli ----
 *
 * rpgmagestaff.h
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

#ifndef RPGMAGESTAFF_H
#define RPGMAGESTAFF_H

#include "tiledweapon.h"
#include "rpgpickableobject.h"
#include "rpgplayer.h"

class RpgMageStaff : public TiledWeapon, public RpgPickableWeaponIface
{
	Q_OBJECT

public:
	explicit RpgMageStaff(QObject *parent = nullptr);

	bool protect(const WeaponType &) override final { return false; };
	bool canProtect(const WeaponType &) const override final { return false; };
	bool canAttack() const override final { return canShot(); }

	virtual RpgPickableObject::PickableType toPickable() const override { return RpgPickableObject::PickableInvalid; }
	virtual RpgPickableObject::PickableType toBulletPickable() const override { return RpgPickableObject::PickableInvalid; }

	void setFromCast(const RpgPlayerCharacterConfig::CastType &cast);
	void eventUseCast(const RpgPlayerCharacterConfig::CastType &cast, TiledObject *target = nullptr);

protected:
	IsometricBullet *createBullet(const qreal & = 0.) override final { return nullptr; }
};



/*
class RpgDaggerPickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgDaggerPickable(QQuickItem *parent = nullptr);

	bool playerPick(RpgPlayer *player) override final;

protected:
	void load() override final;

};
*/

#endif // RPGMAGESTAFF_H
