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
#include "rpggame_p.h"
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
 * @brief RpgLogicClient::addLocalIdTag
 * @param entitiy
 */

void RpgLogicClient::addLocalIdTag(entt::entity entity)
{
	QMutexLocker locker(&m_mutex);

	m_registry.emplace_or_replace<LocalIdTag>(entity);
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
 * @brief RpgLogicClient::estimatedServerTick
 * @return
 */

quint32 RpgLogicClient::estimatedServerTick() const {
	const quint32 delta = m_lastInputTimer.isValid() ? AbstractGame::TickTimer::msecToTick(m_lastInputTimer.elapsed()) : 0;

	if (m_serverRtt <= 0)
		return m_serverTick + delta;

	return m_serverTick + delta + AbstractGame::TickTimer::msecToTick(m_serverRtt/2.);
}











/**
 * @brief RpgLogicClientSingle::RpgLogicClientSingle
 */

RpgLogicClientSingle::RpgLogicClientSingle()
	: RpgLogicClient(0, 0)
{

}




/**
 * @brief RpgLogicClientSingle::start
 */

RpgStream::GameConfig RpgLogicClientSingle::start()
{
	Rpg::RpgLogicScope scope = getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	cfg->flags().setFlag(RpgStream::GameConfig::FlagDataCompleted);

	initialize();

	cfg->flags().setFlag(RpgStream::GameConfig::FlagPlaying);


	RpgStream::GameConfig config = *cfg;
	config.setStage(RpgStream::GameConfig::StageSelect);

	return config;
}



/**
 * @brief RpgLogicClientSingle::startGame
 * @return
 */

RpgStream::GameConfig RpgLogicClientSingle::startGame()
{
	Rpg::RpgLogicScope scope = getScope();

	// First render

	render();

	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	const RpgStream::GameConfig config = *cfg;

	// Rewind stage

	cfg->setStage(RpgStream::GameConfig::StageSelect);

	return config;
}






/**
 * @brief RpgLogicClientSingle::eventRealized
 * @param entity
 */

void RpgLogicClientSingle::eventRealized(entt::entity entity)
{
	RpgLogicScope scope = getScope();

	if (!scope.valid(entity))
		return;

	const quint32 tick = lastAuthTick();

	if (RpgStream::EventStageChanged *ev = scope.try_get<RpgStream::EventStageChanged>(entity)) {
		if (ev->config().stage() == RpgStream::GameConfig::StageWarmingUp) {
			for (auto e : scope.view<MpEmitter>()) {
				EventMpCreate evc = EventMpCreate::createMp(RpgStream::GameConfig::StageMain, tick);
				evc.emitter = e;

				ELOG_DEBUG << "Register WarmingUp MP create event for" << evc.tick();

				eventStore(std::move(evc));
			}
		}

		return;
	}


	if (EventMpEmitterEmpty *ev = scope.try_get<EventMpEmitterEmpty>(entity)) {
		if (!scope.valid(ev->emitter)) {
			ELOG_ERROR << "Invalid emitter";
			return;
		}

		ELOG_DEBUG << "Emitter empty" << scope.get<MpEmitter>(ev->emitter).idTag << "at" << tick;

		EventMpCreate evc = EventMpCreate::createMp(RpgStream::GameConfig::StageMain, tick);
		evc.emitter = ev->emitter;

		ELOG_DEBUG << "Register MP create event for" << evc.tick();

		eventStore(std::move(evc));
	}
}




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

	m_serverTick = full.serverTick();
	m_lastInputTimer.start();


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

	loadFullState(full.fullState());

	m_engine->updateStage(full.config(), full.serverTick());

}



/**
 * @brief RpgLogicClientMulti::loadFullState
 * @param full
 */

void RpgLogicClientMulti::loadFullState(const RpgStream::FullState &full)
{
	if (!m_engine) {
		LOG_CERROR("game") << "Missing engine";
		return;
	}

	m_serverTick = full.serverTick();
	m_lastInputTimer.start();

	{
		Rpg::RpgLogicScope scope = getScope();

		RpgStream::GameState *state = scope.getCtx<RpgStream::GameState>();

		if (!state) {
			scope.registerCtx<RpgStream::GameState>();
			state = scope.getCtx<RpgStream::GameState>();
		}

		*state = full.state();
	}


	if (full.flags().testFlag(RpgStream::FullState::Player))
		loadPlayers(full.players());


	if (full.flags().testFlag(RpgStream::FullState::Event))
		loadEvents(full.events());

	if (full.flags().testFlag(RpgStream::FullState::Tower))
		loadTowers(full.towers());

	if (full.flags().testFlag(RpgStream::FullState::Mp))
		loadMp(full.mps());

	if (full.flags().testFlag(RpgStream::FullState::Defender))
		loadDefenders(full.defenders());
}






RpgUdpEngine *RpgLogicClientMulti::engine() const
{
	return m_engine;
}

void RpgLogicClientMulti::setEngine(RpgUdpEngine *newEngine)
{
	m_engine = newEngine;
}




/**
 * @brief RpgLogicClientMulti::loadPlayers
 * @param list
 */

void RpgLogicClientMulti::loadPlayers(const std::vector<RpgStream::PlayerStateList> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	IdTagMapper *mapper = scope.getCtx<IdTagMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::PlayerStateList &s : list) {
		entt::entity player = mapper->get(s.tagId());

		if (!scope.valid(player)) {
			LOG_CERROR("game") << "Invalid player" << s.tagId();
			continue;
		}

		PlayerStateOutput *out = scope.try_get<PlayerStateOutput>(player);

		if (!out) {
			LOG_CERROR("game") << "Invalid player" << s.tagId();
			continue;
		}

		for (const RpgStream::PlayerState &state : s.state()) {
			out->insert(state);
		}
	}
}



/**
 * @brief RpgLogicClientMulti::loadEvents
 * @param list
 */

void RpgLogicClientMulti::loadEvents(const std::vector<RpgStream::Events> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	EventsOutput *out = scope.getCtx<EventsOutput>();

	Q_ASSERT(out);

	for (const RpgStream::Events &event : list)
		out->insert(event);
}





/**
 * @brief RpgLogicClientMulti::loadTowers
 * @param list
 */

void RpgLogicClientMulti::loadTowers(const std::vector<RpgStream::TowerState> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	IdTagMapper *mapper = scope.getCtx<IdTagMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::TowerState &s : list) {
		entt::entity tower = mapper->get(s.tagId());

		if (!scope.valid(tower)) {
			LOG_CERROR("game") << "Invalid tower" << s.tagId();
			continue;
		}

		TowerStateOutput *out = scope.try_get<TowerStateOutput>(tower) ;

		if (!out) {
			LOG_CERROR("game") << "Invalid tower" << s.tagId();
			continue;
		}

		out->insert(s);
	}
}




/**
 * @brief RpgLogicClientMulti::loadMp
 * @param list
 */

void RpgLogicClientMulti::loadMp(const std::vector<RpgStream::MpData> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	std::unordered_map<quint32, entt::entity> entities;
	entities.reserve(list.size());

	for (auto e : scope.view<Mp>()) {
		if (!scope.valid(e))
			continue;

		entities[scope.get<Mp>(e).idTag] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());

	for (const RpgStream::MpData &p : list) {
		ids.insert(p.tagId());

		auto it = entities.find(p.tagId());

		if (it == entities.end()) {
			auto entity = m_registry.create();

			Mp &mp = m_registry.emplace<Mp>(entity);
			mp.idTag = p.tagId();
			mp.pos = cpv(p.posXAsFloat(), p.posYAsFloat());
			mp.origin = cpv(p.origXAsFloat(), p.origYAsFloat());

			entitySetIdTag(entity, mp.idTag);

			LOG_CDEBUG("game") << "ADD MP" << mp.idTag << mp.pos.x << mp.pos.y ;
		}
	}

	for (const auto &[id, e] : entities) {
		if (ids.contains(id))
			continue;

		m_registry.emplace_or_replace<DeleteTag>(e);
	}
}






/**
 * @brief RpgLogicClientMulti::loadDefenders
 * @param list
 */

void RpgLogicClientMulti::loadDefenders(const std::vector<RpgStream::DefenderState> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	IdTagMapper *mapper = scope.getCtx<IdTagMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::DefenderState &s : list) {
		entt::entity defender = mapper->get(s.tagId());

		if (!scope.valid(defender)) {
			LOG_CERROR("game") << "Invalid defender" << s.tagId();
			continue;
		}

		DefenderStateOutput *out = scope.try_get<DefenderStateOutput>(defender);

		if (!out) {
			LOG_CERROR("game") << "Invalid defender" << s.tagId();
			continue;
		}

		out->insert(s);
	}
}



}		// end of namespace
