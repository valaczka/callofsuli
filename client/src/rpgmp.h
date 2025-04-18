/*
 * ---- Call of Suli ----
 *
 * rpgmp.h
 *
 * Created on: 2024. 07. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgMpPickable
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

#ifndef RPGMP_H
#define RPGMP_H

#include "rpgpickableobject.h"

class RpgGame;


/**
 * @brief The RpgMpPickable class
 */

class RpgMpPickable : public RpgPickableObject
{
	Q_OBJECT

public:
	RpgMpPickable(RpgGame *game);

	bool playerPick(RpgPlayer *player) override final;

	static bool pick(RpgPlayer *player, const int &amount);

	static int amount() { return m_amount; }

protected:
	void load() override final;

private:
	static const int m_amount;
};

#endif // RPGMP_H
