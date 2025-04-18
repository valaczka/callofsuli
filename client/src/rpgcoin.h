/*
 * ---- Call of Suli ----
 *
 * rpgcoin.h
 *
 * Created on: 2024. 07. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgCoinPickable
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

#ifndef RPGCOIN_H
#define RPGCOIN_H

#include "rpgpickableobject.h"

class RpgCoinPickable : public RpgPickableObject
{
	Q_OBJECT

public:
	RpgCoinPickable(RpgGame *game);

	bool playerPick(RpgPlayer *player) override final;

	static int amount(const bool &hasQuestions);

protected:
	void load() override final;

private:
	static const int m_amount;
};


#endif // RPGCOIN_H
