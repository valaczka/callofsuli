/*
 * ---- Call of Suli ----
 *
 * rpgconfig.cpp
 *
 * Created on: 2025. 04. 02.
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


#include "rpglogic.h"
#include "rpgstream.h"
#include <chipmunk/chipmunk.h>
#include <QRandomGenerator>
#include <random>


#define MAX_FUTURE_TICK					10

#define STD_DEV_MAX_ATTEMPTS			10000
#define TARGET_STD_DEV					1.5




/*
void PlayerBaseData::assign(const QList<PlayerBaseData *> &dst, const int &num)
{
	if (dst.empty() || num <= dst.size())
		return;

	const int size = dst.size();

	static const auto standard_deviation = [](const std::vector<int>& data) -> double {
		double mean = accumulate(data.begin(), data.end(), 0.0) / data.size();
		double sum_sq_diff = 0.0;
		for (int val : data) {
			sum_sq_diff += (val - mean) * (val - mean);
		}
		return sqrt(sum_sq_diff / data.size());
	};

	std::random_device rd;
	std::mt19937 gen(rd());

	std::vector<int> dist;			// destination
	const int remainder = num % size;


	try {
		std::vector<int> tmp;

		for (int attempt = 0; attempt < STD_DEV_MAX_ATTEMPTS; ++attempt) {
			std::vector<int> base(size, num / size);

			// Véletlenszerűen szétosztjuk a maradékot

			std::vector<int> indices(size);
			std::iota(indices.begin(), indices.end(), 0);
			std::shuffle(indices.begin(), indices.end(), gen);
			for (int i = 0; i < remainder; ++i) {
				base[indices[i]] += 1;
			}

			// Másolat, amin a véletlen szórásnövelés történik

			dist = base;

			if (size * 2 >= num) {
				qWarning() << "Not enough target" << num << "vs." << size;
				throw 1;
			}

			tmp = base;

			// Véletlenszerű átcsoportosítások

			std::uniform_int_distribution<> dis(0, size - 1);
			for (int i = 0; i < 10 * size; ++i) {
				int from = dis(gen);
				int to = dis(gen);
				if (from != to && dist[from] > 0) {
					dist[from] -= 1;
					dist[to] += 1;
				}
			}

			double std = standard_deviation(dist);
			if (abs(std - TARGET_STD_DEV) < 0.1) {
				throw 1;
			}
		}

		qWarning() << "Standard deviation calculation failed";

		dist = tmp;

		// dist ok

	} catch (int e) {
		// dist ok
	}

	for (int i=0; i<(int) dist.size() && i<dst.size(); ++i) {
		dst.at(i)->rq = dist.at(i);
	}
}


*/

namespace Rpg {


/**
 * @brief The RpgLogicPrivate class
 */

class RpgLogicPrivate
{
private:
	RpgLogicPrivate(RpgLogic *logic) : q(logic) {}
	~RpgLogicPrivate() = default;


	// Players

	bool emplacePlayers();
	bool initializePlayer(entt::entity ent);

	void playerInputLoad(entt::entity ent, const std::vector<RpgStream::PlayerState> &list);
	void playerInputLoad(const quint32 &tag, const std::vector<RpgStream::PlayerState> &list) {
		playerInputLoad(q->entityFromIdTag(tag), list);
	}
	void playerInputLoad(const RpgStream::PlayerStateEntityList &list, const bool &delta = true);		// TAGID check!!!



	// Mp

	void loadMpEmitters(const std::vector<RpgStream::MpEmitter> &list);
	void generateMp();

	// Events

	void eventInputLoad(const RpgStream::EventList &list);		// TAGID check!!!
	void eventInputLoad(const std::vector<RpgStream::EventPlayerList> &list);		// TAGID check!!!



private:
	RpgLogic *const q;

	friend class RpgLogic;
};





/**
 * @brief RpgLogic::RpgLogic
 */

RpgLogic::RpgLogic(const quint32 &lastAuthDiff, const quint32 &jitterDiff)
	: d(new RpgLogicPrivate(this))
	, m_lastAuthTickDiff(lastAuthDiff)
	, m_jitterDiff(jitterDiff)
{
	registerCtx<RpgStream::GameConfig>();
	registerCtx<ChunkGrid>();
	registerCtx<PlayerPositionList>();
	registerCtx<IdTagMapper>();
	registerCtx<Events>();
}

/**
 * @brief RpgLogic::~RpgLogic
 */

RpgLogic::~RpgLogic()
{
	delete d;
	d = nullptr;
}


/**
 * @brief RpgLogic::getScope
 * @return
 */

RpgLogicScope RpgLogic::getScope()
{
	return RpgLogicScope(this);
}






/**
 * @brief RpgLogic::packId
 * @param scene
 * @param owner
 * @param id
 * @return
 */

quint32 RpgLogic::packId(const quint32 &scene, const quint32 &owner, const quint32 &id)
{
	// 4 bit: scene (0-15)
	// 8 bit: owner (0-255)
	// 16 bit: object (0-65535)

	quint32 out = 0;

	out |= (static_cast<uint32_t>(scene & 0x0F)) << 24;
	out |= (static_cast<uint32_t>(owner & 0xFF)) << 16;
	out |= (static_cast<uint32_t>(id & 0xFFFF));

	return out;
}


/**
 * @brief RpgLogic::unpackId
 * @param from
 * @param scene
 * @param owner
 * @param id
 */

void RpgLogic::unpackId(const quint32 &from, quint32 &scene, quint32 &owner, quint32 &id)
{
	scene = (from >> 24) & 0x0F;
	owner = (from >> 16) & 0xFF;
	id = from & 0xFFFF;
}


/**
 * @brief RpgLogic::entityAddTagId
 * @param entity
 * @param tag
 */

void RpgLogic::entitySetIdTag(entt::entity &entity, const quint32 &tag)
{
	if (entity == entt::null)
		return;

	QMutexLocker locker(&m_mutex);

	m_registry.emplace_or_replace<IdTag>(entity, tag);
	m_registry.ctx().get<IdTagMapper>().map.insert(tag, entity);
}


/**
 * @brief RpgLogic::entityFromIdTag
 * @param tag
 * @return
 */

entt::entity RpgLogic::entityFromIdTag(const quint32 &tag) const
{
	QMutexLocker locker(&m_mutex);
	return m_registry.ctx().get<IdTagMapper>().get(tag);
}




/**
 * @brief RpgLogic::loadMapData
 * @param data
 */

void RpgLogic::loadMapData(const RpgStream::MapData &data)
{
	PlayerPositionList l = data.playerPositionList();

	QMutexLocker locker(&m_mutex);
	m_registry.ctx().insert_or_assign<ChunkGrid>(ChunkGrid::fromRpgStream(data.chunkGrid()));
	m_registry.ctx().insert_or_assign<PlayerPositionList>(std::move(l));

	d->loadMpEmitters(data.mpEmitterList());
}











/**
 * @brief RpgLogic::playerAdd
 * @param team
 * @return
 */

entt::entity RpgLogic::playerAdd(const TeamTag::Team &team)
{
	QMutexLocker locker(&m_mutex);
	auto view = m_registry.view<Player>();

	quint8 next = 1;

	for (const auto &e : view) {
		const auto &player = view.get<Player>(e);
		if (player.playerData.playerId() >= next)
			next = player.playerData.playerId() + 1;
	}

	auto entity = m_registry.create();

	RpgStream::PlayerData playerData;
	playerData.setPlayerId(next);
	playerData.setCharacterResolved("testCharacter01");


	m_registry.emplace<Player>(entity, std::move(playerData), 0u);
	m_registry.emplace<TeamTag>(entity, team);
	m_registry.emplace<PlayerStateInput>(entity);
	m_registry.emplace<PlayerStateOuput>(entity);

	return entity;
}



// OBSOLETE!!!!

void RpgLogic::emplacePlayers()
{
	d->emplacePlayers();
}


/**
 * @brief RpgLogic::serverTick
 * @return
 */





/**
 * @brief RpgLogic::playerInputLoad
 * @param list
 */

void RpgLogicPrivate::playerInputLoad(entt::entity ent, const std::vector<RpgStream::PlayerState> &list)
{
	QMutexLocker locker(&q->m_mutex);

	Player *p = q->m_registry.try_get<Player>(ent);
	PlayerStateInput *input = q->m_registry.try_get<PlayerStateInput>(ent);

	if (!p || !input) {
		LOG_CERROR("engine") << "Invalid player";
		return;
	}

	input->load(list, q->lastAuthTick(), q->m_serverTick + MAX_FUTURE_TICK);
}




/**
 * @brief RpgLogic::playerInputLoad
 * @param list
 */

void RpgLogicPrivate::playerInputLoad(const RpgStream::PlayerStateEntityList &list, const bool &delta)
{
	/// TAG ID CHECK!

	for (const RpgStream::PlayerStateList &ps : list.list()) {
		playerInputLoad(ps.tagId(), delta ? ps.extractStateVector() : ps.state());
	}
}


/**
 * @brief RpgLogicPrivate::loadMpEmitters
 * @param list
 */

void RpgLogicPrivate::loadMpEmitters(const std::vector<RpgStream::MpEmitter> &list)
{
	LOG_CINFO("game") << "Load emitters";

	QMutexLocker locker(&q->m_mutex);

	for (const RpgStream::MpEmitter &e : list) {
		auto entity = q->m_registry.create();

		MpEmitter &emitter = q->m_registry.emplace<MpEmitter>(entity);

		emitter.id = ++MpEmitter::lastId;
		emitter.pos.setX(e.posXAsFloat());
		emitter.pos.setY(e.posYAsFloat());

		LOG_CDEBUG("game") << "ADD EMITTER" << emitter.id << emitter.pos;

	}
}



/**
 * @brief RpgLogicPrivate::generateMp
 */

void RpgLogicPrivate::generateMp()
{
	LOG_CWARNING("game") << "GENERATE MP";

	QMutexLocker locker(&q->m_mutex);

	auto view = q->m_registry.view<MpEmitter>();

	for (auto e : view) {
		const MpEmitter &emitter = q->m_registry.get<MpEmitter>(e);

		for (int i=0; i<6; ++i) {
			auto entity = q->m_registry.create();

			Mp &mp = q->m_registry.emplace<Mp>(entity);
			mp.idTag = q->packId(1, 0, ++q->m_lastObjectId);
			mp.emitter = e;
			mp.pos = emitter.pos + QPointF(30*i, 35*i);

			LOG_CDEBUG("game") << "ADD MP" << emitter.id << "->" << mp.idTag << mp.pos;
		}
	}
}


/**
 * @brief RpgLogic::eventLoad
 * @param list
 */

void RpgLogicPrivate::eventInputLoad(const RpgStream::EventList &list)
{
	if (list.flags().testFlags(RpgStream::EventList::Player))
		eventInputLoad(list.playerList());

}


/**
 * @brief RpgLogic::eventPlayerInputLoad
 * @param list
 */

void RpgLogicPrivate::eventInputLoad(const std::vector<RpgStream::EventPlayerList> &list)
{
	const quint32 &minTick = q->lastAuthTick();
	const quint32 &maxTick = q->m_serverTick + MAX_FUTURE_TICK;

	/// TAG ID CHECK!

	QMutexLocker locker(&q->m_mutex);

	Events* events = q->m_registry.ctx().find<Events>();

	if (!events) {
		LOG_CERROR("game") << "Missing events";
		return;
	}

	for (const RpgStream::EventPlayerList &l : list) {
		LOG_CWARNING("game") << "####" << l.tagId() << l.list().size();

		events->player.load(l.tagId(), l.list(), minTick, maxTick);
	}
}







/**
 * @brief RpgLogic::emplacePlayers - elhelyezzük a játékosokat a kezdőpontjaikra
 * @return
 */

bool RpgLogicPrivate::emplacePlayers()
{
	RpgLogicScope scope = q->getScope();
	PlayerPositionList *list = scope.getCtx<PlayerPositionList>();

	Q_ASSERT(list);

	if (list->empty()) {
		LOG_CWARNING("game") << "Empty player position list";
		return false;
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(list->begin(), list->end(), g);

	QHash<TeamTag::Team, std::vector<RpgStream::PlayerPosition> > map;

	map.insert(TeamTag::TeamNone, {});
	map.insert(TeamTag::TeamA, {});
	map.insert(TeamTag::TeamB, {});

	for (const RpgStream::PlayerPosition &p : *list) {
		map[QVariant(p.team()).value<TeamTag::Team>()].emplace_back(p);
	}

	bool success = true;

	auto view = q->m_registry.view<Player, TeamTag>(entt::exclude<RpgStream::PlayerPosition>);

	for (auto &player : view) {
		const TeamTag &t = q->m_registry.get<TeamTag>(player);

		std::vector<RpgStream::PlayerPosition> &plist = map[t.team];

		if (plist.empty()) {
			LOG_CERROR("game") << "Not enough player position for" << t.team;
			success = false;
			continue;
		}

		q->m_registry.emplace<RpgStream::PlayerPosition>(player, plist.back());

		plist.pop_back();

		initializePlayer(player);
	}


	auto v = q->m_registry.view<Player>();

	for (auto &player : v) {
		auto p = q->m_registry.get<Player>(player);

		LOG_CINFO("game") << "***************PLAYER" << p.playerData.playerId() << p.playerData.character();

		if (TeamTag *t = q->m_registry.try_get<TeamTag>(player)) {
			LOG_CDEBUG("game") << "   - team:" << t->team;
		}

		if (RpgStream::PlayerPosition *p = q->m_registry.try_get<RpgStream::PlayerPosition>(player)) {
			LOG_CDEBUG("game") << "   - pos:" << p->posXAsFloat() << p->posYAsFloat();
		}
	}


	return success;
}


/**
 * @brief RpgLogic::initializePlayer
 * @param ent
 * @return
 */

bool RpgLogicPrivate::initializePlayer(entt::entity ent)
{
	if (ent == entt::null)
		return false;

	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.try_get<Player>(ent)) {
		LOG_CERROR("game") << "Invalid player";
		return false;
	}

	RpgStream::PlayerState &state = q->m_registry.emplace_or_replace<RpgStream::PlayerState>(ent);
	RpgStream::PlayerPosition *p = q->m_registry.try_get<RpgStream::PlayerPosition>(ent);

	state.setHp(12);

	state.entityState().setPosX(p ? p->posX() : 0.f);
	state.entityState().setPosY(p ? p->posY() : 0.f);
	state.entityState().setVelX(0);
	state.entityState().setVelY(0);

	return true;
}





/**
 * @brief RpgLogic::fullStateLoad
 * @param full
 */

void RpgLogic::fullStateLoad(const RpgStream::FullState &full)
{
	if (full.flags().testFlag(RpgStream::FullState::Player))
		d->playerInputLoad(full.players(), full.isDeltaMode());

	if (full.flags().testFlag(RpgStream::FullState::Events))
		d->eventInputLoad(full.events());
}



/**
 * @brief RpgLogic::render
 */

void RpgLogic::render()
{
	QMutexLocker locker(&m_mutex);

	++m_serverTick;

	if (m_serverTick <= m_lastAuthTickDiff)
		return;


	renderEvents();

	entt::entity p1 = entityFromIdTag(RpgLogic::packId(0, 1, 0));

	auto [player, input, output] = m_registry.try_get<Player, PlayerStateInput, PlayerStateOuput>(p1);

	if (!player || !input) {
		LOG_CERROR("game") << "ERR";
		return;
	}


	if (!input->map().empty()) {
		const quint32 fromTick = lastAuthTick()-1;

		const RpgStream::PlayerState *last = input->last(fromTick);
		const RpgStream::PlayerState *current = input->at(lastAuthTick());



		if (last && current) {
			const float dx = current->entityState().posXAsFloat() - last->entityState().posXAsFloat();
			const float dy = current->entityState().posYAsFloat() - last->entityState().posYAsFloat();

			auto view = m_registry.view<Player, PlayerStateOuput>();

			for (auto p : view) {
				RpgStream::PlayerState *state = m_registry.try_get<RpgStream::PlayerState>(p);
				if (!state) {
					LOG_CERROR("game") << "NO POS";
					continue;
				}


				cpVect v = cpvadd(cpv(state->entityState().posXAsFloat(), state->entityState().posYAsFloat()),
								  cpv(dx, dy));

				state->setTick(lastAuthTick());
				state->entityState().setPosXAsFloat(v.x);
				state->entityState().setPosYAsFloat(v.y);

				PlayerStateOuput &out = m_registry.get<PlayerStateOuput>(p);

				out.append(*state);

			}
		}

		if (current) {
			output->append(*current);
		}

		input->clear(lastAuthTick());

	}

	if (m_serverTick == 600)
		d->generateMp();

	/*qint32 dx = (qint32) toIt->entityState().posX() - (qint32) fromIt->entityState().posX();
	qint32 dy = (qint32) toIt->entityState().posY() - (qint32) fromIt->entityState().posY();

	LOG_CDEBUG("game") << "FROM" << fromTick << "POS" << dx << dy << "TO" << m_lastAuthTick;

	tick->map.erase(tick->map.begin(), toIt);


	RpgStream::PlayerState ps;
	ps.entityState().setVelX(dx);
	ps.entityState().setVelY(dy);

	auto view = m_registry.view<Rpg::Player>();

	for (entt::entity e : view) {
		auto [player, tick] = m_registry.try_get<Rpg::Player, Rpg::PlayerTickMap>(e);

		if (!player || !tick) {
			LOG_CERROR("game") << "ERR";
			continue;
		}

		if (player->playerData.playerId() == 1)
			continue;

		tick->map.insert(m_lastAuthTick, ps);

		tick->map.erase(tick->map.begin(), tick->map.lowerBound(m_lastAuthTick));
	}*/
}



/**
 * @brief RpgLogic::renderEvents
 */

void RpgLogic::renderEvents()
{
	QMutexLocker locker(&m_mutex);

	const quint32 tick = lastAuthTick();

	Events* events = m_registry.ctx().find<Events>();

	if (!events) {
		LOG_CERROR("game") << "Missing events";
		return;
	}


	// Player events

	if (std::vector<std::pair<quint32, RpgStream::EventPlayer> > *ePlayer = events->player.at(tick)) {
		for (const auto &ptr : *ePlayer) {
			switch (ptr.second.type()) {
				case RpgStream::EventPlayer::EventTest: {
					playerAdd(TeamTag::TeamB);
					emplacePlayers();

					break;
				}

				case RpgStream::EventPlayer::EventNone:
					break;
			}
		}
	}
}



/**
 * @brief ChunkGrid::fromRpgStream
 * @param grid
 * @return
 */

ChunkGrid ChunkGrid::fromRpgStream(const RpgStream::ChunkGrid &grid)
{
	ChunkGrid ch;

	ch.viewport.setLeft(grid.viewportXAsFloat());
	ch.viewport.setTop(grid.viewportYAsFloat());
	ch.viewport.setWidth(grid.viewportWidthAsFloat());
	ch.viewport.setHeight(grid.viewportHeightAsFloat());

	ch.chunkSize.setWidth(grid.chunkWidthAsFloat());
	ch.chunkSize.setHeight(grid.chunkHeightAsFloat());

	for (const RpgStream::Chunk &c : grid.excludeList())
		ch.excludeSet.insert(QPair<quint32, quint32>(c.x(), c.y()));

	return ch;
}




/**
 * @brief ChunkGrid::toRpgStream
 * @return
 */

RpgStream::ChunkGrid ChunkGrid::toRpgStream() const
{
	RpgStream::ChunkGrid grid;

	grid.setViewportXAsFloat(viewport.left());
	grid.setViewportYAsFloat(viewport.top());
	grid.setViewportWidthAsFloat(viewport.width());
	grid.setViewportHeightAsFloat(viewport.height());

	grid.setChunkWidthAsFloat(chunkSize.width());
	grid.setChunkHeightAsFloat(chunkSize.height());

	std::vector<RpgStream::Chunk> list;
	list.reserve(excludeSet.size());

	for (const QPair<qint32, qint32> &ch : excludeSet)
		list.emplace_back(ch.first, ch.second);

	grid.setExcludeList(list);

	return grid;
}


/**
 * @brief ChunkGrid::getChunk
 * @param x
 * @param y
 * @return
 */

QPair<qint32, qint32> ChunkGrid::getChunk(const float &x, const float &y) const
{
	QPair<qint32, qint32> pos(-1, -1);

	if (chunkSize.isNull() || !chunkSize.isValid())
		return pos;

	pos.first = std::floor(std::clamp(x, 0.f, (float) viewport.width()) / chunkSize.width());
	pos.second = std::floor(std::clamp(y, 0.f, (float) viewport.height()) / chunkSize.height());

	return pos;
}
















}		// end namespace
