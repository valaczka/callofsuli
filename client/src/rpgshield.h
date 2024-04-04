/*
 * ---- Call of Suli ----
 *
 * rpgshield.h
 *
 * Created on: 2024. 03. 19.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgShield
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

#ifndef RPGSHIELD_H
#define RPGSHIELD_H

#include "rpgpickableobject.h"
#include "tiledweapon.h"


/**
 * @brief The RpgShield class
 */

class RpgShield : public TiledWeapon
{
	Q_OBJECT

public:
	explicit RpgShield(QObject *parent = nullptr);

	bool protect(const WeaponType &weapon) override final;
	bool canProtect(const WeaponType &weapon) const override final;
	bool canAttack() const override final { return false; }

protected:
	IsometricBullet *createBullet() override final { return nullptr; }
};






/**
 * @brief The RpgArrowPickable class
 */

class RpgShieldPickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgShieldPickable(QQuickItem *parent = nullptr);

	void playerPick(RpgPlayer *player) override final;
	void playerThrow(RpgPlayer *player) override final;

protected:
	void load() override final;
	virtual void onActivated() override final;

};

#endif // RPGSHIELD_H