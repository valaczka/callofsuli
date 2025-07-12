/*
 * ---- Call of Suli ----
 *
 * rpgbroadsword.h
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

#ifndef RPGBROADSWORD_H
#define RPGBROADSWORD_H

#include "rpgarmory.h"

class RpgPlayer;

class RpgBroadsword : public RpgWeapon
{
	Q_OBJECT

public:
	explicit RpgBroadsword(const int &subType = 0, QObject *parent = nullptr);

protected:
	void eventAttack(TiledObject *target) override final;
};


#endif // RPGBROADSWORD_H
