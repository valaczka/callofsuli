/*
 * ---- Call of Suli ----
 *
 * rpglogicclient.cpp
 *
 * Created on: 2026. 05. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicClient
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

#include "rpglogicclient.h"
#include "rpggame.h"
#include "rpgobject.h"

namespace Rpg {

RpgLogicClient::RpgLogicClient(const quint32 &lastAuthDiff, const quint32 &jitterDiff)
	: RpgLogic(lastAuthDiff, jitterDiff)
{
	registerCtx<RpgLogicObjectMapper>();
}


/**
 * @brief RpgLogicClient::removeFromMapper
 */

void RpgLogicClient::removeFromMapper(RpgObject *object)
{
	if (!object)
		return;

	RpgLogicScope scope = getScope();
	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	mapper->map.remove(mapper->getId(object->objectId()));
}



/**
 * @brief RpgLogicClient::getChunkFromVector
 * @param point
 * @return
 */

QPoint RpgLogicClient::getChunkFromVector(const cpVect &point, cpVect *centerPtr)
{
	RpgLogicScope scope = getScope();
	ChunkGrid *grid = scope.getCtx<ChunkGrid>();

	QPair<qint32, qint32> ch = grid->getAccessibleChunk(TiledObjectBody::toPointF(point));

	if (centerPtr) {
		if (ch.first < 0 || ch.second < 0) {
			*centerPtr = cpv(-1., -1.);
		} else {
			*centerPtr = cpv(grid->viewport.left() + grid->chunkSize.width() * (ch.first + 0.5),
							 grid->viewport.top() + grid->chunkSize.height() * (ch.second + 0.5));
		}
	}

	return QPoint(ch.first, ch.second);
}



}		// end of namespace
