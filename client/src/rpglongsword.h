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

class RpgPlayer;

class RpgLongsword : public TiledWeapon
{
	Q_OBJECT

public:
	explicit RpgLongsword(QObject *parent = nullptr);

	bool protect(const WeaponType &weapon) override final;
	bool canProtect(const WeaponType &weapon) const override final;

protected:
	IsometricBullet *createBullet() override final { return nullptr; }
	void eventAttack() override final;

private:
	RpgPlayer *player() const;
};

#endif // RPGLONGSWORD_H


