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
#include "rpgudpengine.h"

namespace Rpg {

RpgLogicClient::RpgLogicClient(const quint32 &lastAuthDiff, const quint32 &jitterDiff)
	: RpgLogic(lastAuthDiff+1)						// nem lehet 0, mert akkor nem engedne feldolgozni semmit
	, m_jitterDiff(jitterDiff)
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

	QPair<qint32, qint32> ch = grid->getAccessibleChunk(point.x, point.y);

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


/**
 * @brief RpgLogicClient::getChunkFromVector
 * @param point
 * @param angle
 * @param centerPtr
 * @return
 */

QPoint RpgLogicClient::getChunkFromVector(const cpVect &point, const float &angle, cpVect *centerPtr)
{
	RpgLogicScope scope = getScope();
	ChunkGrid *grid = scope.getCtx<ChunkGrid>();

	return getChunkFromVector(cpvadd(point,
									 TiledObjectBody::vectorFromAngle(angle,
																	  std::max(grid->chunkSize.width(),
																			   grid->chunkSize.height())*1.1)),
							  centerPtr);
}



/**
 * @brief RpgLogicClient::eventRealized
 * @param entity
 */

void RpgLogicClient::eventRealized(entt::entity entity)
{
	RpgLogicScope scope = getScope();

	if (!scope.valid(entity))
		return;

	if (EventTowerActiveChanged *e = scope.try_get<EventTowerActiveChanged>(entity)) {
		LOG_CINFO("game") << "CHANGED" << e->team << e->active;
	}
}









/**
 * @brief RpgLogicClientSingle::RpgLogicClientSingle
 */

RpgLogicClientSingle::RpgLogicClientSingle() : RpgLogicClient(0, 0) {}




/**
 * @brief RpgLogicClientMulti::RpgLogicClientMulti
 */

RpgLogicClientMulti::RpgLogicClientMulti()
	: RpgLogicClient(6, 6)
{
	registerCtx<RpgLogicControlledObjects>();
}


/**
 * @brief RpgLogicClientMulti::loadFull
 * @param full
 */

void RpgLogicClientMulti::loadFull(const RpgStream::Full &full)
{
	if (!m_engine) {
		LOG_CERROR("game") << "Missing engine";
		return;
	}

	Rpg::RpgLogicScope scope = getScope();

	RpgLogicControlledObjects *objs = scope.getCtx<RpgLogicControlledObjects>();

	Q_ASSERT(objs);

	for (const RpgStream::FullPlayerMap &m : full.map()) {
		if (m.peerId() != m_engine->peerId())
			continue;

		objs->player = m.player();
		objs->entities.clear();
		objs->entities.reserve(m.entities().size());

		for (const RpgStream::FullMapTag &t : m.entities())
			objs->entities.insert(t.tagId());

	}

	fullStateLoad(full.fullState(), {});

	render(true);
}


RpgUdpEngine *RpgLogicClientMulti::engine() const
{
	return m_engine;
}

void RpgLogicClientMulti::setEngine(RpgUdpEngine *newEngine)
{
	m_engine = newEngine;
}



}		// end of namespace
