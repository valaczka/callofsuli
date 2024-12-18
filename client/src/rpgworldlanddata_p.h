/*
 * ---- Call of Suli ----
 *
 * rpgworldlanddata_p.h
 *
 * Created on: 2024. 10. 31.
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

#ifndef RPGWORLDLANDDATA_P_H
#define RPGWORLDLANDDATA_P_H

#include "rpguserwallet.h"
#include "rpgworldlanddata.h"


/**
 * @brief The RpgWorldLandDataPrivate class
 */

class RpgWorldLandDataPrivate
{
private:
	RpgWorldLandDataPrivate(RpgWorldLandData *data);
	~RpgWorldLandDataPrivate() = default;

	RpgWorldLandData *const q;
	QPointer<RpgUserWallet> m_walletMap;

	friend class RpgWorldLandData;
};

#endif // RPGWORLDLANDDATA_P_H
