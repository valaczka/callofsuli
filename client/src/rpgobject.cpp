/*
 * ---- Call of Suli ----
 *
 * rpgobject.cpp
 *
 * Created on: 2026. 05. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgObject
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

#include "rpgobject.h"


/**
 * @brief RpgObject::RpgObject
 * @param center
 * @param radius
 * @param game
 * @param type
 */

RpgObject::RpgObject(RpgGameItem *gameItem, const QPointF &center, const qreal &radius, const cpBodyType &type)
	: IsometricObject(center, radius, gameItem, type)
{
	Q_ASSERT(gameItem);
	Q_ASSERT(gameItem->game());

	m_rpgGame = gameItem->game();
}
