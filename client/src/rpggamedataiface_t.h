/*
 * ---- Call of Suli ----
 *
 * rpggamedataiface_t.h
 *
 * Created on: 2025. 04. 06.
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

#ifndef RPGGAMEDATAIFACE_T_H
#define RPGGAMEDATAIFACE_T_H


#include "rpggamedataiface.h"

/**
 *
 */

template<typename T, typename T2, typename T3, typename T4>
template<typename E>
inline QVector2D RpgGameDataInterface<T, T2, T3, T4>::entityMove(IsometricEntity *entity,
																 const RpgGameData::SnapshotInterpolation<T> &snapshot,
																 const E &idle,
																 const E &moving,
																 const qreal &speed)
{
	Q_ASSERT(entity);





	return QVector2D{};
}


#endif // RPGGAMEDATAIFACE_T_H
