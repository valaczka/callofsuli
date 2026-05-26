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





///
/// Registry components
///


// Player pick mp

struct EventMpPick {
	entt::entity mp;
	entt::entity player;
	int origMp = 0;				// Akinek kevesebb mp-je van, az kapja
};



// Player target tower

struct EventTower {
	entt::entity tower;
	entt::entity player;
	bool success = false;
};



// Player put tower defender

struct EventDefenderPut {
	entt::entity defender;
	entt::entity player;
};



// Add defender to chunk

struct EventDefenderAdd {
	Chunk chunk;
	entt::entity player;
};




// Player target player

struct EventAttackPlayer {
	entt::entity player;
	entt::entity target;
};



// Player target defender

struct EventAttackDefender {
	entt::entity player;
	entt::entity target;
};




/**
 * @brief The RpgLogicPrivate class
 */

class RpgLogicPrivate
{
private:
	RpgLogicPrivate(RpgLogic *logic) : q(logic) {}
	~RpgLogicPrivate() = default;


	quint32 nextIdTag();
	quint32 nextIdTag(Player *player);


	// Players

	bool emplacePlayers();
	bool playerInitialize(entt::entity ent);

	void playerInputLoad(entt::entity ent, const std::vector<RpgStream::PlayerState> &list);
	void playerInputLoad(const quint32 &tag, const std::vector<RpgStream::PlayerState> &list) {
		playerInputLoad(q->entityFromIdTag(tag), list);
	}
	void playerInputLoad(const std::vector<RpgStream::PlayerStateList> &list,
						 const QSet<quint32> &acceptedInputList, const bool &delta = true);



	// Mp

	void loadMpEmitters(const std::vector<RpgStream::MpEmitter> &list);
	void generateMp();



	// Tower

	void loadTower(const std::vector<RpgStream::Tower> &list);
	void loadDefender(entt::entity towerEntity, const std::vector<RpgStream::Defender> &list);



	// DefenderObject

	entt::entity generateDefender(Player *player, entt::entity defEnt, Defender *defender, const Chunk &chunk = {});

	// Events

	void eventInputLoad(const std::vector<RpgStream::Events> &list, const QSet<quint32> &acceptedInputList);
	void eventInputLoad(const std::vector<RpgStream::EventPlayer> &list, const QSet<quint32> &acceptedInputList);

	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	void eventStore(const T &event);

	template <typename T>
	entt::entity eventFinalStore(T &&event);



	// Render

	void preRenderEvents();
	void preRenderEventPlayer(entt::entity event);
	void preRenderEventPlayerMpPick(const RpgStream::EventPlayer &event);
	void preRenderEventTower(const RpgStream::EventPlayer &event);
	void preRenderEventDefender(const RpgStream::EventPlayer &event);
	void preRenderEventAttackPlayer(const RpgStream::EventPlayer &event);
	void preRenderEventAttackDefender(const RpgStream::EventPlayer &event);

	void preRenderEventDefenderDestroy(entt::entity ent);

	void renderEvents();
	void renderEvents(entt::entity mpent, const std::vector<EventMpPick *> &list);
	void renderEvents(entt::entity ent, const std::vector<EventTower*> &list);
	void renderEvents(entt::entity ent, const std::vector<EventDefenderPut*> &list);
	void renderEvents(const std::vector<EventDefenderAdd*> &list);

	void renderPlayerInputs();

	void renderFinal();
	void renderFinalTowers();
	void renderFinalDefenders();


	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	T &getEditableCurrentState(entt::entity ent);

	RpgStream::Events &getEditableCurrentEventList();


	template <typename T>
	void renderFinal(entt::entity ent, T &&event);

private:
	RpgLogic *const q;

	friend class RpgLogic;
};





// Generate mp

class EventMpCreate : public RpgStream::BaseTickState
{
public:
	EventMpCreate() : RpgStream::BaseTickState() {}
};





// Defender destroy after msec

class EventDefenderDestroy : public RpgStream::BaseTickState
{
public:
	EventDefenderDestroy() : RpgStream::BaseTickState() {}

	entt::entity defenderObject = entt::null;
	entt::entity container = entt::null;					// Ezen a defender-en van (ha azon van)
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
	registerCtx<EventsOutput>();
	registerCtx<RpgStream::GameState>();
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
	if (!m_registry.valid(entity))
		return;

	QMutexLocker locker(&m_mutex);

	m_registry.emplace_or_replace<IdTag>(entity, tag);
	m_registry.ctx().get<IdTagMapper>().map.insert(tag, entity);

	LOG_CDEBUG("game") << "ADD TAG" << tag;
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
	QMutexLocker locker(&m_mutex);

	PlayerPositionList l = data.playerPositionList();
	ChunkGrid grid = ChunkGrid::fromRpgStream(data.chunkGrid());

	m_registry.ctx().insert_or_assign<ChunkGrid>(std::move(grid));
	m_registry.ctx().insert_or_assign<PlayerPositionList>(std::move(l));

	d->loadMpEmitters(data.mpEmitterList());
	d->loadTower(data.towerList());
}


/**
 * @brief RpgLogic::isChunkEmpty
 * @param chunk
 * @return
 */

bool RpgLogic::isChunkEmpty(const Chunk &chunk) const
{
	QMutexLocker locker(&m_mutex);

	const ChunkGrid *grid = m_registry.ctx().find<ChunkGrid>();

	if (!grid) {
		LOG_CERROR("game") << "Missing ChunkGrid";
		return false;
	}

	if (!grid->isAccessible(chunk))
		return false;

	auto view = m_registry.view<Chunk>(entt::exclude<DeleteTag>);

	for (auto e : view) {
		if (m_registry.get<Chunk>(e) == chunk)
			return false;
	}

	return true;
}











/**
 * @brief RpgLogic::playerAdd
 * @param team
 * @return
 */

entt::entity RpgLogic::playerAdd(const RpgStream::Team &team)
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
	playerData.setCharacterResolved("character01a");
	playerData.setMaxHp(5);
	playerData.setMaxMp(13);


	m_registry.emplace<Player>(entity, std::move(playerData), team, 0u);
	m_registry.emplace<PlayerStateInput>(entity);
	m_registry.emplace<PlayerStateOutput>(entity);


	return entity;
}



// OBSOLETE!!!!

void RpgLogic::emplacePlayers()
{
	d->emplacePlayers();
	render();
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

void RpgLogicPrivate::playerInputLoad(const std::vector<RpgStream::PlayerStateList> &list,
									  const QSet<quint32> &acceptedInputList, const bool &delta)
{
	for (const RpgStream::PlayerStateList &ps : list) {
		if (acceptedInputList.empty() || acceptedInputList.contains(ps.tagId()))
			playerInputLoad(ps.tagId(), delta ? ps.extractStateVector() : ps.state());
		else
			LOG_CERROR("game") << "Unacceptable input" << ps.tagId();
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

		MpEmitter emitter = MpEmitter::fromRpgStream(e);

		LOG_CDEBUG("game") << "ADD EMITTER" << emitter.idTag << emitter.pos.x << emitter.pos.y;

		q->m_registry.emplace<MpEmitter>(entity, std::move(emitter));
	}


	EventMpCreate e;
	e.setTick(300);

	LOG_CDEBUG("game") << "REGISTER MP EVENT" << e.tick();

	eventStore(std::move(e));

}



/**
 * @brief RpgLogicPrivate::generateMp
 */

void RpgLogicPrivate::generateMp()
{
	LOG_CWARNING("game") << "GENERATE MP";

	QMutexLocker locker(&q->m_mutex);

	// Register events

	RpgStream::Events &eList = getEditableCurrentEventList();


	auto view = q->m_registry.view<MpEmitter>();

	for (auto e : view) {
		MpEmitter &emitter = q->m_registry.get<MpEmitter>(e);

		const int num = emitter.capacity > 0 ? emitter.capacity : 5;

		const float startRadian = QRandomGenerator::global()->generateDouble() * M_PI;
		const float step = 2.*M_PI / num;

		for (int i=0; i<num; ++i) {
			auto entity = q->m_registry.create();

			const float rad = startRadian + i*step;
			float radius = emitter.radius > 0 ? emitter.radius : 75.;

			radius *= (0.5 + QRandomGenerator::global()->generateDouble()*0.5);

			cpVect pos = cpvmult(cpvforangle(rad), radius);

			Mp &mp = q->m_registry.emplace<Mp>(entity);
			mp.idTag = nextIdTag();
			mp.emitter = e;
			mp.pos = cpvadd(emitter.pos, pos);

			emitter.mpList.push_back(entity);

			LOG_CDEBUG("game") << "ADD MP" << emitter.idTag << "->" << mp.idTag << rad << mp.pos.x << mp.pos.y << "||" << emitter.mpList.size();
		}


		// Register event

		RpgStream::EventMpEmitter ev;
		ev.setTagId(emitter.idTag);
		eList.emitter().emplace_back(std::move(ev));
	}
}




/**
 * @brief RpgLogicPrivate::loadTower
 * @param list
 */

void RpgLogicPrivate::loadTower(const std::vector<RpgStream::Tower> &list)
{
	LOG_CINFO("game") << "Load towers";

	QMutexLocker locker(&q->m_mutex);

	for (const RpgStream::Tower &e : list) {
		auto entity = q->m_registry.create();

		Tower tower = Tower::fromRpgStream(e);

		LOG_CDEBUG("game") << "ADD TOWER" << tower.idTag;

		q->entitySetIdTag(entity, tower.idTag);

		q->m_registry.emplace<Tower>(entity, std::move(tower));

		loadDefender(entity, e.defenders());


		TowerStateOutput &out = q->m_registry.emplace<TowerStateOutput>(entity);

		RpgStream::TowerState state;

		state.setTick(q->lastAuthTick());
		state.setTeam(RpgStream::TeamNone);
		state.setLoad(0);
		state.setLockedUntil(0);
		state.setActive(false);
		state.setHasDefender(false);

		out.append(std::move(state));
	}
}



/**
 * @brief RpgLogicPrivate::loadDefender
 * @param towerEntity
 * @param list
 */

void RpgLogicPrivate::loadDefender(entt::entity towerEntity, const std::vector<RpgStream::Defender> &list)
{
	LOG_CINFO("game") << "Load defenders";

	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(towerEntity)) {
		LOG_CERROR("game") << "Invalid entity";
		return;
	}


	Tower *tower = q->m_registry.try_get<Tower>(towerEntity);

	if (!tower) {
		LOG_CERROR("game") << "Invalid entity";
		return;
	}


	for (const RpgStream::Defender &e : list) {
		auto entity = q->m_registry.create();

		Defender defender = Defender::fromRpgStream(e);

		defender.tower = towerEntity;
		tower->defenderList.push_back(entity);

		LOG_CDEBUG("game") << "ADD DEFENDER" << defender.idTag << "TO TOWER" << tower->idTag;

		q->entitySetIdTag(entity, defender.idTag);

		q->m_registry.emplace<Defender>(entity, std::move(defender));
	}
}



/**
 * @brief RpgLogicPrivate::generateDefender
 * @param player
 * @param defender
 * @return
 */

entt::entity RpgLogicPrivate::generateDefender(Player *player, entt::entity defEnt, Defender *defender, const Chunk &chunk)
{
	Q_ASSERT(player);


	QMutexLocker locker(&q->m_mutex);

	auto entity = q->m_registry.create();


	DefenderObject &d = q->m_registry.emplace<DefenderObject>(entity);
	d.idTag = nextIdTag(player);

	if (defender) {
		defender->object = entity;

		d.pos = defender->pos;
		d.defender = defEnt;
	} else {
		ChunkGrid *grid = q->m_registry.ctx().find<ChunkGrid>();
		Q_ASSERT(grid);

		d.pos = grid->chunkCenter(chunk);

		q->m_registry.emplace_or_replace<Chunk>(entity, chunk);
	}


	d.type = RpgStream::BaseDefenderObject::Dummy;
	d.team = player->team;
	d.maxHp = 5;									// TODO





	/// State

	DefenderStateOutput &out = q->m_registry.emplace<DefenderStateOutput>(entity);

	RpgStream::DefenderState state;

	state.setTick(q->lastAuthTick());
	state.setTagId(d.idTag);
	state.setType(d.type);
	state.setHp(5);



	// EXTRACT

	if (d.type == RpgStream::BaseDefenderObject::Dummy) {
		DefenderDummyObject &dummy = q->m_registry.emplace<DefenderDummyObject>(entity);
		dummy.dummy = 125;

		state.setDummy(125);
	}


	out.append(std::move(state));


	LOG_CINFO("game") << "DEFENDER GENERATED" << d.idTag;

	return entity;
}




/**
 * @brief RpgLogic::eventLoad
 * @param list
 */

void RpgLogicPrivate::eventInputLoad(const std::vector<RpgStream::Events> &list, const QSet<quint32> &acceptedInputList)
{
	const quint32 &minTick = q->lastAuthTick();
	const quint32 &maxTick = q->m_serverTick + MAX_FUTURE_TICK;

	for (const RpgStream::Events &e : list) {
		if (e.tick() < minTick || e.tick() > maxTick) {
			LOG_CERROR("game") << "Unacceptable input" << e.tick() << "TICK:" << minTick << maxTick;
		} else {
			eventInputLoad(e.player(), acceptedInputList);
		}
	}

}


/**
 * @brief RpgLogic::eventPlayerInputLoad
 * @param list
 */

void RpgLogicPrivate::eventInputLoad(const std::vector<RpgStream::EventPlayer> &list, const QSet<quint32> &acceptedInputList)
{
	if (list.empty())
		return;

	for (const RpgStream::EventPlayer &l : list) {
		LOG_CWARNING("game") << "####" << l.tagId() << l.type() << l.tick();

		if (!acceptedInputList.empty() && !acceptedInputList.contains(l.tagId())) {
			LOG_CERROR("game") << "Unacceptable input" << l.tagId() << l.type() << l.tick();
		} else {
			eventStore(l);
		}
	}
}



/**
 * @brief RpgLogicPrivate::renderEventPlayer
 * @param event
 */

void RpgLogicPrivate::preRenderEventPlayer(entt::entity event)
{
	QMutexLocker locker(&q->m_mutex);

	RpgStream::EventPlayer &e = q->m_registry.get<RpgStream::EventPlayer>(event);


	switch (e.type()) {
		case RpgStream::EventPlayer::EventMpPick:
			preRenderEventPlayerMpPick(e);
			break;

		case RpgStream::EventPlayer::EventTower:
			preRenderEventTower(e);
			break;

		case RpgStream::EventPlayer::EventDefender:
			preRenderEventDefender(e);
			break;

		case RpgStream::EventPlayer::EventAttackPlayer:
			preRenderEventAttackPlayer(e);
			break;

		case RpgStream::EventPlayer::EventAttackDefender:
			preRenderEventAttackDefender(e);
			break;

		case RpgStream::EventPlayer::EventNone:
			break;
	}
}



/**
 * @brief RpgLogicPrivate::renderEventPlayerMpPick
 * @param event
 */

void RpgLogicPrivate::preRenderEventPlayerMpPick(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventMpPick) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity mp = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}

	if (!q->m_registry.valid(mp)) {
		LOG_CWARNING("game") << "Mp entity not found" << event.target();
		return;
	}

	const Player &p = q->m_registry.get<Player>(player);
	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);

	if (st->hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st->tick() << st->hp();
		return;
	}

	if (st->mp() >= p.playerData.maxMp()) {
		LOG_CWARNING("game") << "Player reached max mp" << st->tick() << st->mp();
		return;
	}


	// Event pick

	EventMpPick final;
	final.player = player;
	final.mp = mp;
	final.origMp = st->mp();

	eventFinalStore(std::move(final));
}






/**
 * @brief RpgLogicPrivate::preRenderEventTower
 * @param event
 */

void RpgLogicPrivate::preRenderEventTower(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventTower) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity tower = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}

	if (!q->m_registry.valid(tower)) {
		LOG_CWARNING("game") << "Tower entity not found" << event.target();
		return;
	}

	const Player &pp = q->m_registry.get<Player>(player);
	const Tower &tt = q->m_registry.get<Tower>(tower);
	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);
	const RpgStream::TowerState *t = q->getCurrentState<RpgStream::TowerState>(tower);

	if (st->hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st->tick() << st->hp();
		return;
	}

	const quint32 tick = q->lastAuthTick()+1;

	if (t->lockedUntil() > tick) {
		LOG_CWARNING("game") << "Tower can't be attacked" << st->tick() << event.target() << t->lockedUntil();
		return;
	}

	if (t->hasDefender() && t->team() != pp.team) {
		LOG_CWARNING("game") << "Tower has defender" << st->tick() << event.target();
		return;
	}


	LOG_CDEBUG("game") << "STORE EVENT" << pp.playerData.playerId() << tt.idTag << event.success();


	EventTower final;
	final.player = player;
	final.tower = tower;
	final.success = event.success();

	eventFinalStore(std::move(final));
}



/**
 * @brief RpgLogicPrivate::preRenderEventDefender
 * @param event
 */

void RpgLogicPrivate::preRenderEventDefender(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventDefender) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity defender = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}


	// Player state

	const Player &pp = q->m_registry.get<Player>(player);

	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);

	if (st->hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st->tick() << st->hp();
		return;
	}




	if (!q->m_registry.valid(defender)) {										// Emplace on chunk
		const Chunk &chunk = Chunk::fromRpgStream(event.chunk());

		if (!q->isChunkEmpty(chunk)) {
			LOG_CWARNING("game") << "Chunk is not accessible" << st->tick() << chunk.x << chunk.y;
			return;
		}

		LOG_CDEBUG("game") << "STORE EVENT" << pp.playerData.playerId() << chunk.x << chunk.y;


		EventDefenderAdd final;
		final.player = player;
		final.chunk = chunk;

		eventFinalStore(std::move(final));


	} else {

		const Defender &dd = q->m_registry.get<Defender>(defender);

		if (!q->m_registry.valid(dd.tower)) {
			LOG_CWARNING("game") << "Tower entity not found" << event.target();
			return;
		}

		if (q->m_registry.valid(dd.object)) {
			LOG_CWARNING("game") << "The defender already contains an object" << event.target();
			return;
		}

		const Tower &tt = q->m_registry.get<Tower>(dd.tower);
		const RpgStream::TowerState *t = q->getCurrentState<RpgStream::TowerState>(dd.tower);

		if (!t->active()) {
			LOG_CWARNING("game") << "Tower isn't active" << st->tick();
			return;
		}

		if (t->team() != pp.team) {
			LOG_CWARNING("game") << "Tower doesn't belong to player's team" << st->tick();
			return;
		}


		LOG_CDEBUG("game") << "STORE EVENT" << pp.playerData.playerId() << tt.idTag << event.success();


		EventDefenderPut final;
		final.player = player;
		final.defender = defender;

		eventFinalStore(std::move(final));
	}
}




/**
 * @brief RpgLogicPrivate::preRenderEventAttackPlayer
 * @param event
 */

void RpgLogicPrivate::preRenderEventAttackPlayer(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventAttackPlayer) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}

	///const Player &pp = q->m_registry.get<Player>(player);
	Player *tt = q->m_registry.try_get<Player>(target);

	if (!tt) {
		LOG_CWARNING("game") << "Target entity not found" << event.target();
		return;
	}


	RpgStream::PlayerState &stPlayer = getEditableCurrentState<RpgStream::PlayerState>(player);
	RpgStream::PlayerState &stTarget = getEditableCurrentState<RpgStream::PlayerState>(target);

	if (stPlayer.hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << stPlayer.tick() << stPlayer.hp();
		return;
	}

	if (stTarget.hp() <= 0) {
		LOG_CWARNING("game") << "Target isn't alive" << stPlayer.tick() << stTarget.hp();
		return;
	}


	//// TODO
	///
	///


	if (stTarget.hp() > 0)
		stTarget.setHp(stTarget.hp()-1);

	LOG_CINFO("game") << "TARGET HP MINUS" << stTarget.hp() << "tick" << stTarget.tick();
}





/**
 * @brief RpgLogicPrivate::preRenderEventAttackDefender
 * @param event
 */

void RpgLogicPrivate::preRenderEventAttackDefender(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventAttackDefender) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}

	Player *pp = q->m_registry.try_get<Player>(player);
	DefenderObject *tt = q->m_registry.try_get<DefenderObject>(target);

	if (!pp) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}

	if (!tt) {
		LOG_CWARNING("game") << "DefenderObject entity not found" << event.target();
		return;
	}

	if (pp->team == tt->team) {
		LOG_CWARNING("game") << "DefenderObject's team matches" << event.target();
		return;
	}

	RpgStream::PlayerState &stPlayer = getEditableCurrentState<RpgStream::PlayerState>(player);
	RpgStream::DefenderState &stTarget = getEditableCurrentState<RpgStream::DefenderState>(target);

	if (stPlayer.hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << stPlayer.tick() << stPlayer.hp();
		return;
	}

	if (stTarget.hp() <= 0) {
		LOG_CWARNING("game") << "Target isn't alive" << stPlayer.tick() << stTarget.hp();
		return;
	}


	if (tt->type == RpgStream::BaseDefenderObject::Dummy) {
		//// TODO
		///

		DefenderDummyObject *dummy = q->m_registry.try_get<DefenderDummyObject>(target);

		if (!dummy) {
			LOG_CWARNING("game") << "DefenderDummyObject entity not found" << event.target();
			return;
		}

		stTarget.setDummy(18);
	}

	if (stTarget.hp() > 0)
		stTarget.setHp(stTarget.hp()-1);

	LOG_CINFO("game") << "TARGET HP MINUS" << stTarget.hp() << "tick" << stTarget.tick();
}






/**
 * @brief RpgLogicPrivate::preRenderEventDefenderDestroy
 * @param event
 */

void RpgLogicPrivate::preRenderEventDefenderDestroy(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent)) {
		LOG_CERROR("game") << "Missing entity";
		return;
	}

	const EventDefenderDestroy &event = q->m_registry.get<EventDefenderDestroy>(ent);

	if (!q->m_registry.valid(event.defenderObject)) {
		LOG_CERROR("game") << "Missing Defender entity";
		return;
	}

	q->m_registry.emplace_or_replace<DeleteTag>(event.defenderObject);

	LOG_CINFO("game") << "DESTROY DEFENDER";


	if (!q->m_registry.valid(event.container))
		return;

	Defender *def = q->m_registry.try_get<Defender>(event.container);

	if (!def) {
		LOG_CERROR("game") << "Invalid defender entity";
		return;
	}

	def->object = entt::null;

	LOG_CINFO("game") << "CLEAR OBJECT";
}





/**
 * @brief RpgLogicPrivate::renderEvents
 */

void RpgLogicPrivate::renderEvents()
{
	QMutexLocker locker(&q->m_mutex);

	auto view = q->m_registry.view<EventProcessingTag>();


	std::map<entt::entity, std::vector<EventMpPick*> > listMpPick;
	std::map<entt::entity, std::vector<EventTower*> > listTower;
	std::map<entt::entity, std::vector<EventDefenderPut*> > listDefenderPut;
	std::vector<EventDefenderAdd*> listDefenderAdd;



	for (auto e : view) {
		if (EventMpPick *event = q->m_registry.try_get<EventMpPick>(e))
			listMpPick[event->mp].push_back(event);
		else if (EventTower *event = q->m_registry.try_get<EventTower>(e))
			listTower[event->tower].push_back(event);
		else if (EventDefenderPut *event = q->m_registry.try_get<EventDefenderPut>(e))
			listDefenderPut[event->defender].push_back(event);
		else if (EventDefenderAdd *event = q->m_registry.try_get<EventDefenderAdd>(e))
			listDefenderAdd.push_back(event);
	}



	for (const auto &[mp, list] : listMpPick)
		renderEvents(mp, list);

	for (const auto &[e, list] : listTower)
		renderEvents(e, list);

	for (const auto &[mp, list] : listDefenderPut)
		renderEvents(mp, list);

	renderEvents(listDefenderAdd);


	q->m_registry.insert<DeleteTag>(view.begin(), view.end());

}


/**
 * @brief RpgLogicPrivate::renderEvents
 * @param list
 */

void RpgLogicPrivate::renderEvents(entt::entity mpent, const std::vector<EventMpPick*> &list)
{
	QMutexLocker locker(&q->m_mutex);

	if (list.empty() || !q->m_registry.valid(mpent))
		return;


	Mp *mp = q->m_registry.try_get<Mp>(mpent);

	Q_ASSERT(mp);

	LOG_CINFO("game") << "PICK MP" << mp->idTag << list.size();

	EventMpPick *final = nullptr;

	for (EventMpPick *e : list) {
		if (!final || e->origMp < final->origMp)
			final = e;
	}



	// Register player state

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(final->player);

	st.setMp(st.mp()+1);

	LOG_CINFO("game") << "PLAYER PICK MP" << st.mp() << "tick" << st.tick();


	// Register mp state (delete

	q->m_registry.emplace_or_replace<DeleteTag>(mpent);




	// Register event

	RpgStream::Events &eList = getEditableCurrentEventList();

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventMpPick);
	e.setTagId(q->m_registry.get<Player>(final->player).idTag());
	e.setTarget(mp->idTag);

	eList.player().emplace_back(std::move(e));



	// Next emitter event

	if (MpEmitter *emitter = q->m_registry.try_get<MpEmitter>(mp->emitter)) {
		std::erase(emitter->mpList, mpent);

		if (emitter->mpList.empty()) {
			LOG_CINFO("game") << "NEW MP";
			EventMpCreate e;
			e.setTick(q->lastAuthTick()+300);

			eventStore(e);
		}
	} else {
		LOG_CERROR("game") << "INVALID EMITTER";
	}

}



/**
 * @brief RpgLogicPrivate::renderEvents
 * @param tower
 */

void RpgLogicPrivate::renderEvents(entt::entity ent, const std::vector<EventTower *> &list)
{
	QMutexLocker locker(&q->m_mutex);

	if (list.empty() || !q->m_registry.valid(ent))
		return;

	Tower *tower = q->m_registry.try_get<Tower>(ent);


	Q_ASSERT(tower);

	RpgStream::Events &eList = getEditableCurrentEventList();

	RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(ent);

	int load = st.load();

	for (EventTower *e : list) {
		Player *player = q->m_registry.try_get<Player>(e->player);

		Q_ASSERT(player);

		if (!e->success) {
			// Register player state

			RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(e->player);

			if (st.hp() > 1)
				st.setHp(st.hp()-1);

			LOG_CINFO("game") << "PLAYER HP MINUS" << st.hp() << "tick" << st.tick();


		} else {
			if (st.team() == RpgStream::TeamNone) {
				st.setTeam(player->team);
				load = 40;
			} else if (player->team == st.team()) {
				load += 40;
			} else {
				load -= 30;
			}
		}

		// Register event

		RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventTower);
		ev.setTagId(player->idTag());
		ev.setTarget(tower->idTag);
		ev.setSuccess(e->success);

		eList.player().emplace_back(std::move(ev));
	}


	if (load < 0) {
		load *= -1;
		st.setTeam(st.team() == RpgStream::TeamA ? RpgStream::TeamB : RpgStream::TeamA);
	}

	st.setLoad(std::min(100u, (quint32) load));

	if (load >= 100) {
		st.setActive(true);
		st.setLockedUntil(q->lastAuthTick()+1 + 300);
	}

	if (load < 80 && st.active())
		st.setActive(false);


	LOG_CINFO("game") << "TOWER VALUE" << st.load() << "tick" << st.tick() << "TEAM" << st.team();

}




/**
 * @brief RpgLogicPrivate::renderEvents
 * @param ent
 * @param list
 */

void RpgLogicPrivate::renderEvents(entt::entity ent, const std::vector<EventDefenderPut *> &list)
{
	QMutexLocker locker(&q->m_mutex);

	if (list.empty() || !q->m_registry.valid(ent))
		return;

	Defender *d = q->m_registry.try_get<Defender>(ent);

	Q_ASSERT(d);

	Tower *tower = q->m_registry.try_get<Tower>(d->tower);

	Q_ASSERT(tower);

	LOG_CINFO("game") << "PUT DEFENDER" << d->idTag << list.size();

	EventDefenderPut *final = list.front();			// Arbitrary select - TODO: earlier locked


	// Register player and tower state

	Player *p = q->m_registry.try_get<Player>(final->player);

	Q_ASSERT(p);

	RpgStream::PlayerState &sp = getEditableCurrentState<RpgStream::PlayerState>(final->player);
	RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(d->tower);


	st.setHasDefender(true);


	entt::entity object = generateDefender(p, ent, d);

	// Create defender object




	/*st.setMp(st.mp()+1);

	LOG_CINFO("game") << "PLAYER PICK MP" << st.mp() << "tick" << st.tick();*/






	// Register event

	RpgStream::Events &eList = getEditableCurrentEventList();

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
	e.setTagId(q->m_registry.get<Player>(final->player).idTag());
	e.setTarget(d->idTag);

	eList.player().emplace_back(std::move(e));


}






/**
 * @brief RpgLogicPrivate::renderEvents
 * @param ent
 * @param list
 */

void RpgLogicPrivate::renderEvents(const std::vector<EventDefenderAdd *> &list)
{
	if (list.empty())
		return;

	QMutexLocker locker(&q->m_mutex);


	EventDefenderAdd *final = list.front();			// Arbitrary select - TODO: earlier locked

	LOG_CINFO("game") << "Add DEFENDER" << final->chunk.x << final->chunk.y;

	// Register player

	Player *p = q->m_registry.try_get<Player>(final->player);

	Q_ASSERT(p);

	RpgStream::PlayerState &sp = getEditableCurrentState<RpgStream::PlayerState>(final->player);


	entt::entity object = generateDefender(p, entt::null, nullptr, final->chunk);

	// Create defender object




	/*st.setMp(st.mp()+1);

	LOG_CINFO("game") << "PLAYER PICK MP" << st.mp() << "tick" << st.tick();*/






	// Register event

	RpgStream::Events &eList = getEditableCurrentEventList();

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
	e.setTagId(q->m_registry.get<Player>(final->player).idTag());
	e.setChunk(final->chunk.toRpgStream());

	eList.player().emplace_back(std::move(e));

}




/**
 * @brief RpgLogicPrivate::renderPlayerInputs
 */

void RpgLogicPrivate::renderPlayerInputs()
{
	QMutexLocker locker(&q->m_mutex);

	const quint32 tick = q->lastAuthTick();

	auto view = q->m_registry.view<Player>();

	for (auto e : view) {
		auto [player, input, output] =
				q->m_registry.try_get<Player, PlayerStateInput, PlayerStateOutput>(e);

		const RpgStream::PlayerState *lastInput = input ? input->at(tick) : nullptr;
		const RpgStream::PlayerState *current = q->getCurrentState<RpgStream::PlayerState>(e);

		if (!current && !lastInput)
			continue;

		if (!current) {
			LOG_CERROR("game") << "MISSING STATE" << tick;
			continue;
		}


		RpgStream::PlayerState state = *current;

		// Ha van input, akkor csak az engedélyezett mezőket írjuk felül

		if (current->hp() > 0 && lastInput) {
			state.setEntityState(lastInput->entityState());
		}

		renderFinal(e, std::move(state));

		if (input)
			input->clear(tick);
	}

}



/**
 * @brief RpgLogicPrivate::renderFinal
 */

void RpgLogicPrivate::renderFinal()
{
	QMutexLocker locker(&q->m_mutex);

	renderFinalDefenders();
	renderFinalTowers();


}




/**
 * @brief RpgLogicPrivate::renderFinalTowers
 */

void RpgLogicPrivate::renderFinalTowers()
{
	QMutexLocker locker(&q->m_mutex);

	auto view = q->m_registry.view<Tower>();

	for (auto e : view) {
		const RpgStream::TowerState *current = q->m_registry.try_get<RpgStream::TowerState>(e);

		if (!current) {
			//LOG_CERROR("game") << "MISSING STATE" << tick;
			continue;
		}

		RpgStream::TowerState state = *current;

		renderFinal(e, std::move(state));
	}



	if (q->lastAuthTick() % 60 == 0) {
		RpgStream::GameState &state = q->m_registry.ctx().get<RpgStream::GameState>();

		for (auto e : view) {
			const RpgStream::TowerState *current = q->getCurrentState<RpgStream::TowerState>(e);

			if (!current) {
				LOG_CERROR("game") << "Missing state" << q->lastAuthTick();
				continue;
			}

			if (current->active()) {
				if (current->team() == RpgStream::TeamA)
					state.setPtsA(state.ptsA() + 15);
				else if (current->team() == RpgStream::TeamB)
					state.setPtsB(state.ptsB() + 15);
			}
		}
	}
}




/**
 * @brief RpgLogicPrivate::renderFinalDefenders
 */

void RpgLogicPrivate::renderFinalDefenders()
{
	QMutexLocker locker(&q->m_mutex);


	for (auto e : q->m_registry.view<DefenderObject>()) {
		const RpgStream::DefenderState *current = q->m_registry.try_get<RpgStream::DefenderState>(e);

		if (!current) {
			//LOG_CERROR("game") << "MISSING STATE" << tick;
			continue;
		}

		RpgStream::DefenderState state = *current;


		if (state.hp() == 0) {
			const DefenderObject &def = q->m_registry.get<DefenderObject>(e);

			LOG_CINFO("game") << "DESTORY DEFENDER" << def.idTag;

			EventDefenderDestroy ev;
			ev.defenderObject = e;
			ev.container = def.defender;
			ev.setTick(q->lastAuthTick()+
					   (def.defender == entt::null ? 1 : 300));

			eventStore(ev);


			// Defender destroyed, sync tower

			if (q->m_registry.valid(def.defender)) {
				if (Defender *dd = q->m_registry.try_get<Defender>(def.defender)) {
					if (!q->m_registry.valid(dd->tower)) {
						LOG_CERROR("game") << "Invalid tower of defender";
					} else if (Tower *tower = q->m_registry.try_get<Tower>(dd->tower)) {
						RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(dd->tower);

						bool has = false;

						for (auto e : tower->defenderList) {
							if (!q->m_registry.valid(e))
								continue;

							Defender *td = q->m_registry.try_get<Defender>(e);

							if (!td) {
								LOG_CERROR("game") << "Invalid defender";
								continue;
							}

							if (!q->m_registry.valid(td->object))
								continue;

							if (const RpgStream::DefenderState *dst = q->getCurrentState<RpgStream::DefenderState>(td->object)) {
								if (dst->hp() > 0) {
									has = true;
									break;
								}
							} else {
								LOG_CERROR("game") << "Invalid DefenderObject state";
							}
						}

						st.setHasDefender(has);

						if (!has)
							LOG_CWARNING("game") << "*****NO HAS ******";

					} else {
						LOG_CERROR("game") << "Invalid tower entity";
					}
				} else {
					LOG_CERROR("game") << "Invalid defender entity";
				}
			}

		}

		renderFinal(e, std::move(state));
	}
}



/**
 * @brief RpgLogicPrivate::getEditableCurrentEventList
 * @return
 */

RpgStream::Events &RpgLogicPrivate::getEditableCurrentEventList()
{
	QMutexLocker locker(&q->m_mutex);

	if (q->m_registry.ctx().contains<RpgStream::Events>())
		return q->m_registry.ctx().get<RpgStream::Events>();

	RpgStream::Events ee;

	return q->m_registry.ctx().emplace<RpgStream::Events>(std::move(ee));
}







/**
 * @brief RpgLogicPrivate::nextIdTag
 * @return
 */

quint32 RpgLogicPrivate::nextIdTag()
{
	QMutexLocker locker(&q->m_mutex);
	return q->packId(1, 0, ++q->m_lastObjectId);
}


/**
 * @brief RpgLogicPrivate::nextIdTag
 * @param player
 * @return
 */

quint32 RpgLogicPrivate::nextIdTag(Player *player)
{
	Q_ASSERT(player);

	QMutexLocker locker(&q->m_mutex);
	return q->packId(1, player->playerData.playerId(), ++player->lastObjectId);
}




/**
 * @brief RpgLogicPrivate::emplacePlayers
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

	QHash<RpgStream::Team, std::vector<RpgStream::PlayerPosition> > map;

	map.insert(RpgStream::TeamNone, {});
	map.insert(RpgStream::TeamA, {});
	map.insert(RpgStream::TeamB, {});

	for (const RpgStream::PlayerPosition &p : *list) {
		map[QVariant(p.team()).value<RpgStream::Team>()].emplace_back(p);
	}

	bool success = true;

	auto view = q->m_registry.view<Player>(entt::exclude<RpgStream::PlayerPosition>);

	for (auto &player : view) {
		const Player &p = q->m_registry.get<Player>(player);

		std::vector<RpgStream::PlayerPosition> &plist = map[p.team];

		if (plist.empty()) {
			LOG_CERROR("game") << "Not enough player position for" << p.team;
			success = false;
			continue;
		}

		q->m_registry.emplace<RpgStream::PlayerPosition>(player, plist.back());

		plist.pop_back();

		playerInitialize(player);
	}


	auto v = q->m_registry.view<Player>();

	for (auto &player : v) {
		auto p = q->m_registry.get<Player>(player);

		LOG_CINFO("game") << "***************PLAYER" << p.playerData.playerId() << p.playerData.character();

		LOG_CDEBUG("game") << "   - team:" << p.team;

		if (RpgStream::PlayerPosition *p = q->m_registry.try_get<RpgStream::PlayerPosition>(player)) {
			LOG_CDEBUG("game") << "   - pos:" << p->posXAsFloat() << p->posYAsFloat();
		}

		if (const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player)) {
			LOG_CDEBUG("game") << "   - state:" << st->tick() << st->hp() << st->entityState().posXAsFloat() << st->entityState().posYAsFloat();
		}
	}


	return success;
}






/**
 * @brief RpgLogic::initializePlayer
 * @param ent
 * @return
 */

bool RpgLogicPrivate::playerInitialize(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent))
		return false;


	Player *player = q->m_registry.try_get<Player>(ent);

	if (!player) {
		LOG_CERROR("game") << "Invalid player";
		return false;
	}

	RpgStream::PlayerPosition *p = q->m_registry.try_get<RpgStream::PlayerPosition>(ent);

	RpgStream::PlayerState state;

	state.setTick(q->lastAuthTick());
	state.setHp(player->playerData.maxHp());
	state.setMp(0);

	state.entityState().setPosX(p ? p->posX() : 0.f);
	state.entityState().setPosY(p ? p->posY() : 0.f);
	state.entityState().setVelX(0);
	state.entityState().setVelY(0);

	LOG_CINFO("game") << "INITIELIZES" << player->playerData.playerId() << q->lastAuthTick()
					  << state.entityState().posXAsFloat() << state.entityState().posYAsFloat() << state.hp();

	q->m_registry.get<PlayerStateOutput>(ent).append(std::move(state));


	return true;
}





/**
 * @brief RpgLogic::fullStateLoad (to InputPull)
 * @param full
 */

void RpgLogic::fullStateLoad(const RpgStream::FullState &full, const QSet<quint32> &acceptedInputList)
{
	if (full.flags().testFlag(RpgStream::FullState::Player))
		d->playerInputLoad(full.players(), acceptedInputList, full.isDeltaMode());

	if (full.flags().testFlag(RpgStream::FullState::Event))
		d->eventInputLoad(full.events(), acceptedInputList);
}



/**
 * @brief RpgLogic::getFullState
 * @param maxTick
 * @return
 */

RpgStream::FullState RpgLogic::getFullState(const int &maxTick)
{
	RpgStream::FullState full;

	QMutexLocker locker(&m_mutex);

	full.setServerAuthTick(lastAuthTick());
	full.setState(m_registry.ctx().get<RpgStream::GameState>());


	std::vector<RpgStream::Events> eventList = m_registry.ctx().get<EventsOutput>().extract(maxTick);

	if (!eventList.empty()) {
		full.setEvents(std::move(eventList));
		full.flags().setFlag(RpgStream::FullState::Event);
	}

	for (auto e : m_registry.view<Player, PlayerStateOutput>()) {
		const auto &[player, output] = m_registry.get<Player, PlayerStateOutput>(e);

		RpgStream::PlayerStateList pl;
		pl.setTagId(player.idTag());
		pl.setIsDeltaMode(true);
		pl.compressStateVector(output.extract(maxTick));

		full.players().emplace_back(std::move(pl));
	}

	full.flags().setFlag(RpgStream::FullState::Player);

	for (auto e : m_registry.view<Mp>()) {
		const Mp &mp = m_registry.get<Mp>(e);

		RpgStream::MpData d;
		d.setTagId(mp.idTag);
		d.setPosXAsFloat(mp.pos.x);
		d.setPosYAsFloat(mp.pos.y);

		full.mps().emplace_back(std::move(d));
	}

	full.flags().setFlag(RpgStream::FullState::Mp);


	for (auto e : m_registry.view<Tower, TowerStateOutput>()) {
		const auto &[tower, output] = m_registry.get<Tower, TowerStateOutput>(e);

		const RpgStream::TowerState *last = output.last();

		if (!last) {
			LOG_CERROR("game") << "Internal error";
			continue;
		}

		RpgStream::TowerState st = *last;

		st.setTagId(tower.idTag);

		full.towers().emplace_back(std::move(st));
	}

	full.flags().setFlag(RpgStream::FullState::Tower);


	for (auto e : m_registry.view<DefenderObject>()) {
		const auto &[defender, output] = m_registry.get<DefenderObject, DefenderStateOutput>(e);

		const RpgStream::DefenderState *last = output.last();

		if (!last) {
			LOG_CERROR("game") << "Internal error";
			continue;
		}

		RpgStream::DefenderState st = *last;

		st.setTagId(defender.idTag);
		st.setType(defender.type);

		full.defenders().emplace_back(std::move(st));
	}

	full.flags().setFlag(RpgStream::FullState::Defender);

	return full;
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


	d->preRenderEvents();
	d->renderEvents();
	d->renderPlayerInputs();
	d->renderFinal();


	// Store event

	if (m_registry.ctx().contains<RpgStream::Events>()) {
		RpgStream::Events e = m_registry.ctx().get<RpgStream::Events>();
		e.setTick(lastAuthTick());

		m_registry.ctx().get<EventsOutput>().append(std::move(e));
		m_registry.ctx().erase<RpgStream::Events>();
	}



	// Remove DeleteTag entities

	IdTagMapper &mapper = m_registry.ctx().get<IdTagMapper>();
	auto view = m_registry.view<DeleteTag>();

	for (auto e : view) {
		if (IdTag *tag = m_registry.try_get<IdTag>(e)) {
			LOG_CINFO("game") << "REMOVED" << tag->id;
			mapper.map.remove(tag->id);
			m_registry.remove<IdTag>(e);
		}
	}

	m_registry.destroy(view.begin(), view.end());

}



/**
 * @brief RpgLogic::renderEvents
 */

void RpgLogicPrivate::preRenderEvents()
{
	QMutexLocker locker(&q->m_mutex);

	const quint32 tick = q->lastAuthTick()+1;


	auto view = q->m_registry.view<EventTag>();


	for (auto e : view) {
		if (q->m_registry.get<EventTag>(e).tick > tick)
			continue;

		if (q->m_registry.all_of<RpgStream::EventPlayer>(e)) {
			preRenderEventPlayer(e);
		} else if (q->m_registry.all_of<EventMpCreate>(e)) {
			generateMp();
		} else if (q->m_registry.all_of<EventDefenderDestroy>(e)) {
			preRenderEventDefenderDestroy(e);
		}

		q->m_registry.emplace<DeleteTag>(e);
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



/**
 * @brief ChunkGrid::chunkCenter
 * @param x
 * @param y
 * @return
 */

cpVect ChunkGrid::chunkCenter(const int &x, const int &y) const
{
	if (x < 0 || y < 0) {
		return cpv(-1., -1.);
	} else {
		return cpv(viewport.left() + chunkSize.width() * (x + 0.5),
				   viewport.top() + chunkSize.height() * (y + 0.5));
	}
}


/**
 * @brief MpEmitter::fromRpgStream
 * @param stream
 * @return
 */

MpEmitter MpEmitter::fromRpgStream(const RpgStream::MpEmitter &stream)
{
	MpEmitter e;

	e.idTag = stream.tagId();
	e.pos.x = stream.posXAsFloat();
	e.pos.y = stream.posYAsFloat();
	e.radius = stream.radiusAsFloat();
	e.capacity = stream.capacity();

	return e;
}


/**
 * @brief MpEmitter::toRpgStream
 * @return
 */

RpgStream::MpEmitter MpEmitter::toRpgStream() const
{
	RpgStream::MpEmitter stream;

	stream.setTagId(idTag);
	stream.setPosXAsFloat(pos.x);
	stream.setPosYAsFloat(pos.y);
	stream.setRadiusAsFloat(radius);
	stream.setCapacity(capacity);

	return stream;
}



/**
 * @brief RpgLogicPrivate::eventStore
 * @param event
 */

template<class T, typename T2>
void RpgLogicPrivate::eventStore(const T &event)
{
	QMutexLocker locker(&q->m_mutex);

	auto entity = q->m_registry.create();

	q->m_registry.emplace<EventTag>(entity, event.tick());
	q->m_registry.emplace<T>(entity, event);
}





/**
 * @brief RpgLogicPrivate::eventFinalStore
 * @param event
 */

template<typename T>
entt::entity RpgLogicPrivate::eventFinalStore(T &&event)
{
	QMutexLocker locker(&q->m_mutex);

	auto final = q->m_registry.create();
	q->m_registry.emplace<EventProcessingTag>(final);
	q->m_registry.emplace<T>(final, std::move(event));

	return final;
}




template<typename T>
void RpgLogicPrivate::renderFinal(entt::entity ent, T &&event)
{
	QMutexLocker locker(&q->m_mutex);

	q->m_registry.get<BaseStatePull<T> >(ent).append(std::move(event));

	q->m_registry.remove<T>(ent);
}



template<class T, typename T2>
T &RpgLogicPrivate::getEditableCurrentState(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (q->m_registry.all_of<T>(ent))
		return q->m_registry.get<T>(ent);

	if (const BaseStatePull<T> *pull = q->m_registry.try_get<BaseStatePull<T> >(ent)) {
		if (const T* last = pull->last())
			return q->m_registry.emplace<T>(ent, *last);
		else
			return q->m_registry.emplace<T>(ent);
	}

	return q->m_registry.emplace<T>(ent);
}




/**
 * @brief Tower::fromRpgStream
 * @param stream
 * @return
 */

Tower Tower::fromRpgStream(const RpgStream::Tower &stream)
{
	Tower e;

	e.idTag = stream.tagId();

	return e;
}



/**
 * @brief Tower::toRpgStream
 * @return
 */

RpgStream::Tower Tower::toRpgStream() const
{
	RpgStream::Tower stream;

	stream.setTagId(idTag);

	return stream;
}



/**
 * @brief Defender::fromRpgStream
 * @param stream
 * @return
 */

Defender Defender::fromRpgStream(const RpgStream::Defender &stream)
{
	Defender e;

	e.idTag = stream.tagId();
	e.pos.x = stream.posXAsFloat();
	e.pos.y = stream.posYAsFloat();

	return e;
}



/**
 * @brief Defender::toRpgStream
 * @return
 */

RpgStream::Defender Defender::toRpgStream() const
{
	RpgStream::Defender stream;

	stream.setTagId(idTag);
	stream.setPosXAsFloat(pos.x);
	stream.setPosYAsFloat(pos.y);

	return stream;
}


/**
 * @brief Chunk::fromRpgStream
 * @param stream
 * @return
 */

Chunk Chunk::fromRpgStream(const RpgStream::Chunk &stream)
{
	Chunk e;

	e.x = stream.x();
	e.y = stream.y();

	return e;
}


/**
 * @brief Chunk::toRpgStream
 * @return
 */

RpgStream::Chunk Chunk::toRpgStream() const
{
	RpgStream::Chunk stream;

	stream.setX(x);
	stream.setY(y);

	return stream;
}


/**
 * @brief RpgLogicScope::setDeleteTag
 * @param ent
 */

void RpgLogicScope::setDeleteTag(const entt::entity ent)
{
	if (!m_logic->m_registry.valid(ent)) {
		LOG_CERROR("game") << "Invalid entity";
		return;
	}

	m_logic->m_registry.emplace_or_replace<DeleteTag>(ent);
}


/**
 * @brief RpgLogicScope::destroyDeleteTags
 */

void RpgLogicScope::destroyDeleteTags()
{
	auto view = m_logic->m_registry.view<DeleteTag>();
	m_logic->m_registry.destroy(view.begin(), view.end());
}


















}		// end namespace
