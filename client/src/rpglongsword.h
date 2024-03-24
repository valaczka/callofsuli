/*
 * ---- Call of Suli ----
 *
 * rpglongsword.h
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

#ifndef RPGLONGSWORD_H
#define RPGLONGSWORD_H

#include "tiledweapon.h"
#include "rpgpickableobject.h"

class RpgPlayer;


/**
 * @brief The RpgLongsword class
 */

class RpgLongsword : public TiledWeapon, public RpgPickableWeaponIface
{
	Q_OBJECT

public:
	explicit RpgLongsword(QObject *parent = nullptr);

	bool protect(const WeaponType &weapon) override final;
	bool canProtect(const WeaponType &weapon) const override final;
	bool canAttack() const override final { return true; }
	virtual RpgPickableObject::PickableType toPickable() const override { return RpgPickableObject::PickableLongsword; }
	virtual RpgPickableObject::PickableType toBulletPickable() const override { return RpgPickableObject::PickableInvalid; }

protected:
	IsometricBullet *createBullet() override final { return nullptr; }
	void eventAttack() override final;
};




/**
 * @brief The RpgArrowPickable class
 */

class RpgLongswordPickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgLongswordPickable(QQuickItem *parent = nullptr);

	void playerPick(RpgPlayer *player) override final;
	void playerThrow(RpgPlayer *player) override final;

protected:
	void load() override final;

};

#endif // RPGLONGSWORD_H


