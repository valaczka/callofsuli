/*
 * ---- Call of Suli ----
 *
 * rpglongbow.h
 *
 * Created on: 2024. 03. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLongbow
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

#ifndef RPGLONGBOW_H
#define RPGLONGBOW_H

#include "rpgpickableobject.h"
#include "tiledweapon.h"


/**
 * @brief The RpgLongbow class
 */

class RpgLongbow : public TiledWeapon
{
	Q_OBJECT
public:
	explicit RpgLongbow(QObject *parent = nullptr);

	bool protect(const WeaponType &) override final { return false; }
	bool canProtect(const WeaponType &) const override final { return false; }
	bool canAttack() const override final { return true; }

protected:
	void eventAttack(TiledObject *target) override final;

private:
	bool m_effectPlayed = false;
};




/**
 * @brief The RpgArrowPickable class
 */

class RpgLongbowPickable : public RpgPickableObject
{
	Q_OBJECT
	QML_ELEMENT

public:
	RpgLongbowPickable(TiledScene *scene = nullptr);

	bool playerPick(RpgPlayer *player) override final;

protected:
	void load() override final;

};

#endif // RPGLONGBOW_H
