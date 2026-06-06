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
#include "rpgconfig.h"
#include <chipmunk/chipmunk.h>
#include <QRandomGenerator>
#include <random>




#define MAX_FUTURE_TICK					10


namespace Rpg {



struct PlayerPrivate;


/**
 * @brief The RpgLogicPrivate class
 */

class RpgLogicPrivate
{
private:
	RpgLogicPrivate(RpgLogic *logic) : q(logic) {}
	~RpgLogicPrivate() = default;


	quint32 nextIdTag();
	quint32 nextIdTag(Player *player, PlayerPrivate *p);

	quint32 nextLockId();


	// Players

	bool emplacePlayers();
	bool playerInitialize(entt::entity ent);

	void playerInputLoad(entt::entity ent, const std::vector<RpgStream::PlayerState> &list);
	void playerInputLoad(const quint32 &tag, const std::vector<RpgStream::PlayerState> &list) {
		playerInputLoad(q->entityFromIdTag(tag), list);
	}
	void playerInputLoad(const std::vector<RpgStream::PlayerStateList> &list,
						 const QSet<quint32> &acceptedInputList, const bool &delta = true);

	quint32 playerLock(entt::entity player, const quint32 &penalty = 1);
	bool playerUnlock(entt::entity player, const quint32 &penaltyTick = 0);
	quint32 playerDecreaseHp(entt::entity player, const quint32 &count = 1);
	quint32 playerIncreaseStreak(entt::entity player, const bool &skip);



	// Mp

	void loadMpEmitters(const std::vector<RpgStream::MpEmitter> &list);



	// Tower

	void loadTower(const std::vector<RpgStream::Tower> &list);
	void loadDefender(entt::entity towerEntity, const std::vector<RpgStream::Defender> &list);



	// DefenderObject

	entt::entity generateDefender(Player *player, PlayerPrivate *priv, const RpgStream::BaseDefenderObject::Type &type,
								  entt::entity defEnt, Defender *defender, const Chunk &chunk = {});

	// Events

	void eventInputLoad(const std::vector<RpgStream::Events> &list, const QSet<quint32> &acceptedInputList);
	void eventInputLoad(const std::vector<RpgStream::EventPlayer> &list, const QSet<quint32> &acceptedInputList);

	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	void eventStore(T &&event);

	template <typename T>
	entt::entity eventFinalStore(T &&event);

	template <typename T>
	entt::entity eventRealStore(T &&event);

	void storeRealEvents();
	bool addToCurrentEvents(entt::entity entity, RpgStream::Events *dst);



	// Render

	void preRenderEvents();
	void preRenderEventMpCreate(entt::entity ent);
	void preRenderEventPlayer(entt::entity event);
	void preRenderEventPlayerMpPick(const RpgStream::EventPlayer &event);
	void preRenderEventTower(const RpgStream::EventPlayer &event);
	void preRenderEventDefender(const RpgStream::EventPlayer &event);
	void preRenderEventAttackPlayer(const RpgStream::EventPlayer &event);
	void preRenderEventAttackDefender(const RpgStream::EventPlayer &event);
	void preRenderEventFailed(const RpgStream::EventPlayer &event);
	void preRenderEventChangeBullet(const RpgStream::EventPlayer &event);
	void preRenderEventChangeDefender(const RpgStream::EventPlayer &event);

	void preRenderEventDefenderDestroy(entt::entity ent);

	void renderEvents();
	void renderEvents(entt::entity mpent, const std::vector<EventMpPick *> &list);
	void renderEvents(entt::entity ent, const std::vector<EventTower*> &list);
	void renderEvents(entt::entity ent, const std::vector<EventDefenderPut*> &list);
	void renderEvents(EventDefenderAdd *event);
	void renderEvents(EventAttackPlayer *event);
	void renderEvents(EventAttackDefender *event);
	void renderEvents(EventChangeBullet *event);
	void renderEvents(EventChangeDefender *event);

	void renderEntityKnockbacks();

	void renderPlayerInputs();

	void renderFinal();
	void renderFinalTowers();
	void renderFinalDefenders();


	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	T &getEditableCurrentState(entt::entity ent);

	//RpgStream::Events &getEditableCurrentEventList();


	template <typename T>
	void renderFinal(entt::entity ent, T &&event);

private:
	RpgLogic *const q;
	quint32 m_lastLockId = 0;

	friend class RpgLogic;
};










// Internal (private) data for players

struct PlayerPrivate
{
	void load(const RpgStream::PlayerConfig &cfg);
	RpgStream::EntityConfig toEntityConfig() const;

	quint32 lastObjectId = 0;

	// Metric

	quint32 power = 0;

	quint32 push = 0;
	quint32 pushDist = 0;
	quint32 resist = 0;

	quint32 maxHp = 0;
	quint32 maxMp = 0;
	quint32 maxBullet = 0;

	QSet<RpgStream::BaseDefenderObject::Type> defenders;



	// Streak

	quint32 modSkipLock = 0;
	quint32 passedQuestions = 0;
	quint32 passedStreak = 0;

	bool canSkipLock() const;
	void increasePassedQuestions(const bool &skipped) {
		++passedQuestions;
		if (!skipped)
			++passedStreak;
	}

	void resetStreak() { passedStreak = 0; }
};







// Generate mp

class EventMpCreate : public RpgStream::BaseTickState
{
public:
	EventMpCreate() : RpgStream::BaseTickState() {}

	entt::entity emitter = entt::null;
	entt::entity player = entt::null;

	float capacityRatio = 1.0;
	float mpCount = 0;
	cpVect pos = cpvzero;
};





// Defender destroy after msec

class EventDefenderDestroy : public RpgStream::BaseTickState
{
public:
	EventDefenderDestroy() : RpgStream::BaseTickState() {}

	entt::entity defenderObject = entt::null;
	entt::entity container = entt::null;					// Ezen a defender-en van (ha azon van)
};





// Player respawn at start poisition after msec

class EventPlayerRespawn : public RpgStream::BaseTickState
{
public:
	EventPlayerRespawn() : RpgStream::BaseTickState() {}

	entt::entity player = entt::null;
};






/**
 *
 * @brief RpgLogic::RpgLogic
 */

RpgLogic::RpgLogic(const quint32 &lastAuthDiff)
	: d(new RpgLogicPrivate(this))
	, m_lastAuthTickDiff(lastAuthDiff)
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

entt::entity RpgLogic::playerAdd(const RpgStream::PlayerConfig &config, const RpgStream::Team &team)
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



	m_registry.emplace<Player>(entity, std::move(playerData), team);
	m_registry.emplace<PlayerPrivate>(entity).load(config);
	m_registry.emplace<PlayerStateInput>(entity);
	m_registry.emplace<PlayerStateOutput>(entity);


	return entity;
}



// OBSOLETE!!!!

void RpgLogic::emplacePlayers()
{
	d->emplacePlayers();
	//render();
}



/**
 * @brief RpgLogic::addKnockbackImpulse
 * @param targetState
 * @param attackerState
 * @param attacker
 * @param target
 * @return
 */

cpVect RpgLogic::addKnockbackImpulse(RpgStream::EntityState *targetState, const RpgStream::EntityState &attackerState,
									 const RpgStream::EntityConfig &attacker, const RpgStream::EntityConfig &target)
{
	Q_ASSERT(targetState);

	static const auto distanceFalloff = [](float distance, float maxDistance) {
		static constexpr float minDistance = 50.*2.*1.1;			// 2 target circle radius +10%

		if (distance <= minDistance)
			return 1.0f;

		if (distance >= maxDistance)
			return 0.0f;

		float t = 1.0f - ((distance-minDistance) / maxDistance);
		return t * t;
	};

	cpVect knockback = cpv(targetState->slideXAsFloat(), targetState->slideYAsFloat());

	const cpVect attPos = cpv(attackerState.posXAsFloat(), attackerState.posYAsFloat());
	const cpVect tgPos = cpv(targetState->posXAsFloat(), targetState->posYAsFloat());

	const cpVect delta = cpvsub(tgPos, attPos);
	const float distSq = cpvlengthsq(delta);
	const float dist = std::sqrt(distSq);

	LOG_CDEBUG("game") << "****" << dist << attacker.pushDist() << distanceFalloff(dist, attacker.pushDist());

	if (distSq < 0.001f)
		return knockback;

	if (dist > attacker.pushDist())
		return knockback;


	cpVect dir = cpvnormalize(delta);

	float power = attacker.push()
				  * distanceFalloff(dist, attacker.pushDist())
				  //* (100.0f / (100.0f + target.resist()))				// resistance factor
				  ;

	cpVect knock = cpvmult(dir, power);

	knockback = cpvclamp(cpvadd(knockback, knock), CFG_MAX_KNOCKBACK);

	targetState->setSlideXAsFloat(knockback.x);
	targetState->setSlideYAsFloat(knockback.y);

	LOG_CDEBUG("game") << "=====>" << knockback.x << knockback.y << cpvlength(knockback);

	return knockback;
}



/**
 * @brief RpgLogic::decayKnockback
 * @param targetState
 * @return
 */

cpVect RpgLogic::decayKnockback(RpgStream::EntityState *targetState)
{
	Q_ASSERT(targetState);

	cpVect knockback = cpv(targetState->slideXAsFloat(), targetState->slideYAsFloat());

	decayKnockback(knockback);

	targetState->setSlideXAsFloat(knockback.x);
	targetState->setSlideYAsFloat(knockback.y);

	return knockback;
}




/**
 * @brief RpgLogic::decayKnockback
 * @param vect
 * @return
 */

cpVect RpgLogic::decayKnockback(cpVect &knockback)
{
	static constexpr float dt = 1.f/60.f;
	static const float factor = std::exp(-CFG_KNOCKBACK_DECAY_PER_SEC * dt);

	knockback = cpvmult(knockback, factor);

	if (cpvlengthsq(knockback) < 1.f)
		knockback = cpvzero;

	return knockback;
}





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
 * @brief RpgLogicPrivate::playerLock
 * @param player
 * @param target
 * @return
 */

quint32 RpgLogicPrivate::playerLock(entt::entity player, const quint32 &penalty)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(player)) {
		LOG_CERROR("game") << "Invalid player";
		return 0;
	}

	Player *p = q->m_registry.try_get<Player>(player);

	if (!p) {
		LOG_CERROR("game") << "Invalid player";
		return 0;
	}

	if (q->m_registry.all_of<LockTag>(player)) {
		LOG_CERROR("game") << "Player already locked" << p->idTag();
		return 0;
	}


	const quint32 id = nextLockId();
	const quint32 expire = q->lastAuthTick() + 600; //30*60;

	q->m_registry.emplace<LockTag>(player, id, expire, penalty);

	return id;
}






/**
 * @brief RpgLogicPrivate::playerUnlock - sikertelen válasz esetén (levonjuk a büntetést)
 * @param player
 * @return
 */

bool RpgLogicPrivate::playerUnlock(entt::entity player, const quint32 &penaltyTick)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(player)) {
		LOG_CERROR("game") << "Invalid player entity";
		return false;
	}

	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (!pLock) {
		LOG_CERROR("game") << "LockTag missing";
		return false;
	}


	playerDecreaseHp(player, pLock->penalty);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	st.setLock(0);


	if (penaltyTick > 0) {
		if (PenaltyTag *pen = q->m_registry.try_get<PenaltyTag>(player)) {
			if (pen->expire > q->lastAuthTick())
				pen->expire += penaltyTick;
			else
				pen->expire = q->lastAuthTick() + penaltyTick;

			st.setPenalty(pen->expire);
		} else {
			const quint32 exp = q->lastAuthTick() + penaltyTick;

			q->m_registry.emplace<PenaltyTag>(player, exp);
			st.setPenalty(exp);
		}
	}

	q->m_registry.remove<LockTag>(player);


	q->m_registry.get<PlayerPrivate>(player).resetStreak();


	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventFailed);
	e.setTagId(q->m_registry.get<Player>(player).idTag());

	eventRealStore(std::move(e));

	return true;
}



/**
 * @brief RpgLogicPrivate::playerDecreaseHp
 * @param player
 * @param count
 * @return
 */

quint32 RpgLogicPrivate::playerDecreaseHp(entt::entity player, const quint32 &count)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(player)) {
		LOG_CERROR("game") << "Invalid player entity";
		return false;
	}

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);


	if (st.hp() == 0) {
		LOG_CWARNING("game") << "Player isn't alive";
		return 0;
	}

	quint32 nextHp = st.hp() - std::min(count, st.hp());

	st.setHp(nextHp);


	if (nextHp > 0)
		return nextHp;


	quint32 nextTick = q->lastAuthTick() + CFG_PLAYER_RESPAWN;


	// Register respawn event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventRespawn);
	e.setTagId(q->m_registry.get<Player>(player).idTag());
	e.setAt(nextTick);

	eventRealStore(std::move(e));


	LOG_CINFO("game") << "RESPAWN AT" << nextTick;

	EventPlayerRespawn ev;
	ev.setTick(nextTick);
	ev.player = player;

	eventStore(std::move(ev));


	// Register mp create event

	if (st.mp() < 2)
		return nextHp;

	LOG_CINFO("game") << "CHANGE MP" << q->lastAuthTick()+2 << "POS" << st.entityState().posXAsFloat() << st.entityState().posYAsFloat();

	quint32 eCount = std::floor((float) st.mp() / 2.);

	st.setMp(st.mp() - eCount);

	EventMpCreate evc;
	evc.setTick(q->lastAuthTick()+2);
	evc.player = player;
	evc.mpCount = eCount;
	evc.pos.x = st.entityState().posXAsFloat();
	evc.pos.y = st.entityState().posYAsFloat();

	eventStore(std::move(evc));

	return nextHp;
}



/**
 * @brief RpgLogicPrivate::playerIncreaseStreak
 * @param player
 * @return
 */

quint32 RpgLogicPrivate::playerIncreaseStreak(entt::entity player, const bool &skip)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(player)) {
		LOG_CERROR("game") << "Invalid player entity";
		return 0;
	}


	Player *p = q->m_registry.try_get<Player>(player);

	Q_ASSERT(p);

	PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	pp.increasePassedQuestions(skip);


	// Streak rewards

	struct Reward {
		quint32 point;
		quint32 hp;
	};

	static const QHash<quint32, Reward> hash = {
		{ 3, {.point=15, .hp=0} },
		{ 5, {.point=20, .hp=1} },
		{ 7, {.point=30, .hp=1} },
		{ 10, {.point=50, .hp=2} },
		{ 12, {.point=75, .hp=2} },
		{ 15, {.point=100, .hp=3} },
	};


	const auto it = hash.find(pp.passedStreak);

	if (it == hash.constEnd())
		return pp.passedStreak;



	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (st.hp() == 0) {
		LOG_CWARNING("game") << "Player isn't alive";
		return pp.passedStreak;
	}

	st.setHp(st.hp() + it->hp);




	// Register respawn event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventStreak);
	e.setTagId(q->m_registry.get<Player>(player).idTag());
	e.setAt(pp.passedStreak);

	eventRealStore(std::move(e));


	RpgStream::GameState &state = q->m_registry.ctx().get<RpgStream::GameState>();

	quint32 point = it->point * pp.power;

	if (p->team == RpgStream::TeamA)
		state.setPtsA(state.ptsA() + point);
	else if (p->team == RpgStream::TeamB)
		state.setPtsB(state.ptsB() + point);

	LOG_CINFO("game") << "PLAYER STREAK" << pp.passedStreak << it->hp << point;


	return pp.passedStreak;
}






/**
 * @brief RpgLogicPrivate::loadMpEmitters
 * @param list
 */

void RpgLogicPrivate::loadMpEmitters(const std::vector<RpgStream::MpEmitter> &list)
{
	QMutexLocker locker(&q->m_mutex);

	for (const RpgStream::MpEmitter &e : list) {
		auto entity = q->m_registry.create();

		MpEmitter emitter = MpEmitter::fromRpgStream(e);

		q->m_registry.emplace<MpEmitter>(entity, std::move(emitter));


		EventMpCreate ev;
		ev.setTick(300);
		ev.emitter = entity;

		LOG_CDEBUG("game") << "REGISTER MP EVENT" << ev.tick();

		eventStore(std::move(ev));
	}



}



/**
 * @brief RpgLogicPrivate::generateMp
 */

void RpgLogicPrivate::preRenderEventMpCreate(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent)) {
		LOG_CERROR("game") << "Missing entity";
		return;
	}

	const EventMpCreate &event = q->m_registry.get<EventMpCreate>(ent);


	int num = 0;
	float radius = 100.;
	cpVect center = cpvzero;

	MpEmitter *emitter = nullptr;

	if (q->m_registry.valid(event.emitter)) {
		emitter = q->m_registry.try_get<MpEmitter>(event.emitter);

		if (!emitter) {
			LOG_CERROR("game") << "Invalid emitter";
			return;
		}

		num = (emitter->capacity > 0 ? emitter->capacity : 5) * event.capacityRatio;
		radius = emitter->radius > 0 ? emitter->radius : 75.;

		center = emitter->pos;

	} else if (q->m_registry.valid(event.player)) {
		Player *player = q->m_registry.try_get<Player>(event.player);

		if (!player) {
			LOG_CERROR("game") << "Invalid player";
			return;
		}

		num = event.mpCount;
		radius = 125.;
		center = event.pos;

	} else {
		LOG_CERROR("game") << "Missing emitter";
		return;
	}



	const float startRadian = QRandomGenerator::global()->generateDouble() * M_PI;
	const float step = 2.*M_PI / num;

	for (int i=0; i<num; ++i) {
		auto entity = q->m_registry.create();

		const float rad = startRadian + i*step;

		radius *= (0.5 + QRandomGenerator::global()->generateDouble()*0.5);

		cpVect pos = cpvmult(cpvforangle(rad), radius);

		Mp &mp = q->m_registry.emplace<Mp>(entity);
		mp.idTag = nextIdTag();
		mp.emitter = emitter ? event.emitter : entt::null;
		mp.pos = cpvadd(center, pos);
		mp.origin = center;

		if (emitter)
			emitter->mpList.push_back(entity);

		LOG_CDEBUG("game") << "ADD MP" << mp.idTag << rad << mp.pos.x << mp.pos.y ;
	}


	// Register event

	if (emitter) {
		RpgStream::EventMpEmitter ev;
		ev.setTagId(emitter->idTag);

		eventRealStore(std::move(ev));
	}

}




/**
 * @brief RpgLogicPrivate::loadTower
 * @param list
 */

void RpgLogicPrivate::loadTower(const std::vector<RpgStream::Tower> &list)
{
	QMutexLocker locker(&q->m_mutex);

	for (const RpgStream::Tower &e : list) {
		auto entity = q->m_registry.create();

		Tower tower = Tower::fromRpgStream(e);

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

entt::entity RpgLogicPrivate::generateDefender(Player *player, PlayerPrivate *priv,
											   const RpgStream::BaseDefenderObject::Type &type,
											   entt::entity defEnt, Defender *defender, const Chunk &chunk)
{
	Q_ASSERT(player);
	Q_ASSERT(priv);


	QMutexLocker locker(&q->m_mutex);

	auto entity = q->m_registry.create();


	DefenderObject &d = q->m_registry.emplace<DefenderObject>(entity);
	d.idTag = nextIdTag(player, priv);

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


	d.type = type;
	d.team = player->team;



	/// State

	DefenderStateOutput &out = q->m_registry.emplace<DefenderStateOutput>(entity);

	RpgStream::DefenderState state;

	state.setTick(q->lastAuthTick());
	state.setTagId(d.idTag);
	state.setType(d.type);



	// EXTRACT

	if (d.type == RpgStream::BaseDefenderObject::Dummy) {
		DefenderDummyObject &dummy = q->m_registry.emplace<DefenderDummyObject>(entity);
		dummy.dummy = 125;

		state.setDummy(125);

		state.setHp(1);
		d.maxHp = 5;
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

	for (RpgStream::EventPlayer l : list) {
		if (!acceptedInputList.empty() && !acceptedInputList.contains(l.tagId())) {
			LOG_CERROR("game") << "Unacceptable input" << l.tagId() << l.type() << l.tick();
		} else {
			eventStore(std::move(l));
		}
	}
}




/**
 * @brief RpgLogicPrivate::storeRealEvents
 */

void RpgLogicPrivate::storeRealEvents()
{
	QMutexLocker locker(&q->m_mutex);

	EventsOutput &out = q->m_registry.ctx().get<EventsOutput>();

	auto view = q->m_registry.view<EventRealTag>();

	RpgStream::Events currentEvents;

	for (auto e : view) {
		q->eventRealized(e);

		addToCurrentEvents(e, &currentEvents);
	}

	q->m_registry.insert<DeleteTag>(view.begin(), view.end());


	// Store events

	if (currentEvents.flags() != RpgStream::Events::Null) {
		currentEvents.setTick(q->lastAuthTick());
		out.append(std::move(currentEvents));
	}
}



/**
 * @brief RpgLogicPrivate::addToCurrentEvents
 * @param entity
 * @param dst
 * @return
 */

bool RpgLogicPrivate::addToCurrentEvents(entt::entity entity, RpgStream::Events *dst)
{
	if (!dst)
		return false;

	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(entity)) {
		LOG_CERROR("game") << "Invalid entity";
		return false;
	}

	if (RpgStream::EventMpEmitter *e = q->m_registry.try_get<RpgStream::EventMpEmitter>(entity)) {
		dst->flags().setFlag(RpgStream::Events::Emitter);
		dst->emitter().push_back(*e);
		return true;
	}

	if (RpgStream::EventPlayer *e = q->m_registry.try_get<RpgStream::EventPlayer>(entity)) {
		dst->flags().setFlag(RpgStream::Events::Player);
		dst->player().push_back(*e);
		return true;
	}

	return false;
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

		case RpgStream::EventPlayer::EventFailed:
			preRenderEventFailed(e);
			break;


		case RpgStream::EventPlayer::EventChangeBullet:
			preRenderEventChangeBullet(e);
			break;

		case RpgStream::EventPlayer::EventChangeDefender:
			preRenderEventChangeDefender(e);
			break;

		case RpgStream::EventPlayer::EventRespawn:
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

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);
	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);

	if (st->hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st->tick() << st->hp();
		return;
	}

	if (st->mp() >= pp.maxMp) {
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


	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);


	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			LOG_CWARNING("game") << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			LOG_CWARNING("game") << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const quint32 tick = q->lastAuthTick()+1;
	const Player &pp = q->m_registry.get<Player>(player);
	const PlayerPrivate &ppp = q->m_registry.get<PlayerPrivate>(player);

	if (PenaltyTag *pen = q->m_registry.try_get<PenaltyTag>(player);
			pen && pen->expire > tick) {
		LOG_CERROR("game") << "Player has penalty" << pp.idTag();
		return;
	}

	//const Tower &tt = q->m_registry.get<Tower>(tower);
	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);
	const RpgStream::TowerState *t = q->getCurrentState<RpgStream::TowerState>(tower);

	if (st->hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st->tick() << st->hp();
		return;
	}


	if (t->lockedUntil() > tick) {
		LOG_CWARNING("game") << "Tower can't be attacked" << st->tick() << event.target() << t->lockedUntil();
		return;
	}

	if (t->hasDefender() && t->team() != pp.team) {
		LOG_CWARNING("game") << "Tower has defender" << st->tick() << event.target();
		return;
	}



	EventTower final;
	final.player = player;
	final.tower = tower;
	final.lock = (event.lockId() == 0);
	if (final.lock) final.skipLock = ppp.canSkipLock();

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

	if (st->defender() == RpgStream::BaseDefenderObject::None || !st->hasDefender()) {
		LOG_CWARNING("game") << "Player hasn't defender" << st->tick();
		return;
	}




	if (!q->m_registry.valid(defender)) {										// Emplace on chunk
		const Chunk &chunk = Chunk::fromRpgStream(event.chunk());

		if (!q->isChunkEmpty(chunk)) {
			LOG_CWARNING("game") << "Chunk is not accessible" << st->tick() << chunk.x << chunk.y;
			return;
		}


		EventDefenderAdd final;
		final.player = player;
		final.chunk = chunk;
		final.type = st->defender();

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

		//const Tower &tt = q->m_registry.get<Tower>(dd.tower);
		const RpgStream::TowerState *t = q->getCurrentState<RpgStream::TowerState>(dd.tower);

		if (!t->active()) {
			LOG_CWARNING("game") << "Tower isn't active" << st->tick();
			return;
		}

		if (t->team() != pp.team) {
			LOG_CWARNING("game") << "Tower doesn't belong to player's team" << st->tick();
			return;
		}


		EventDefenderPut final;
		final.player = player;
		final.defender = defender;
		final.type = st->defender();

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

	if (stPlayer.hp() == 0) {
		LOG_CWARNING("game") << "Player isn't alive" << stPlayer.tick() << stPlayer.hp();
		return;
	}

	if (stPlayer.bullet() == 0) {
		LOG_CWARNING("game") << "Player hasn't enough bullet" << stPlayer.tick() << stPlayer.bullet();
		return;
	}

	if (stTarget.hp() == 0) {
		LOG_CWARNING("game") << "Target isn't alive" << stPlayer.tick() << stTarget.hp();
		return;
	}


	EventAttackPlayer final;
	final.player = player;
	final.target = target;

	eventFinalStore(std::move(final));
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

	const PlayerPrivate &ppp = q->m_registry.get<PlayerPrivate>(player);

	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);


	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			LOG_CWARNING("game") << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			LOG_CWARNING("game") << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const quint32 tick = q->lastAuthTick()+1;

	if (PenaltyTag *pen = q->m_registry.try_get<PenaltyTag>(player);
			pen && pen->expire > tick) {
		LOG_CERROR("game") << "Player has penalty" << pp->idTag();
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



	EventAttackDefender final;
	final.player = player;
	final.target = target;
	final.lock = (event.lockId() == 0);
	if (final.lock) final.skipLock = ppp.canSkipLock();

	eventFinalStore(std::move(final));
}



/**
 * @brief RpgLogicPrivate::preRenderEventFailed
 * @param event
 */

void RpgLogicPrivate::preRenderEventFailed(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventFailed) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}

	quint32 penalty = 0;


	if (entt::entity tg = q->entityFromIdTag(event.target()); q->m_registry.valid(tg)) {
		if (q->m_registry.try_get<Tower>(tg)) {
			penalty = CFG_PENALTY_TOWER;
		}
	}

	playerUnlock(player, penalty);
}




/**
 * @brief RpgLogicPrivate::preRenderEventChangeBullet
 * @param event
 */

void RpgLogicPrivate::preRenderEventChangeBullet(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventChangeBullet) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}


	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			LOG_CWARNING("game") << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			LOG_CWARNING("game") << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}


	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);
	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (st.hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st.tick() << st.hp();
		return;
	}

	if (st.mp() < CFG_MP_CHANGE_BULLET) {
		LOG_CWARNING("game") << "Player hasn't enough MP" << st.tick() << st.mp();
		return;
	}



	EventChangeBullet final;
	final.player = player;
	final.lock = (event.lockId() == 0);
	if (final.lock) final.skipLock = pp.canSkipLock();

	eventFinalStore(std::move(final));
}




/**
 * @brief RpgLogicPrivate::preRenderEventChangeDefender
 * @param event
 */

void RpgLogicPrivate::preRenderEventChangeDefender(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventChangeDefender) {
		LOG_CERROR("game") << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		LOG_CWARNING("game") << "Player entity not found" << event.tagId();
		return;
	}


	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			LOG_CWARNING("game") << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			LOG_CWARNING("game") << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (st.hp() <= 0) {
		LOG_CWARNING("game") << "Player isn't alive" << st.tick() << st.hp();
		return;
	}

	const RpgStream::BaseDefenderObject::Type dType = st.defender();

	if (dType == RpgStream::BaseDefenderObject::None) {
		LOG_CWARNING("game") << "Player didn't select defender" << st.tick() << st.defender();
		return;
	}

	if (!pp.defenders.contains(dType)) {
		LOG_CWARNING("game") << "Player can't have this defender" << st.tick() << st.defender();
		return;
	}

	if (st.hasDefender()) {
		LOG_CWARNING("game") << "Player already have a defender" << st.tick() << st.defender();
		return;
	}

	if (st.mp() < RpgStream::BaseDefenderObject::requiredMp(dType)) {
		LOG_CWARNING("game") << "Player hasn't enough MP" << st.tick() << st.mp();
		return;
	}


	EventChangeDefender final;
	final.player = player;
	final.type = dType;
	final.lock = (event.lockId() == 0);
	if (final.lock) final.skipLock = pp.canSkipLock();

	eventFinalStore(std::move(final));
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



	for (auto e : view) {
		if (EventMpPick *event = q->m_registry.try_get<EventMpPick>(e))
			listMpPick[event->mp].push_back(event);
		else if (EventTower *event = q->m_registry.try_get<EventTower>(e))
			listTower[event->tower].push_back(event);
		else if (EventDefenderPut *event = q->m_registry.try_get<EventDefenderPut>(e))
			listDefenderPut[event->defender].push_back(event);
		else if (EventDefenderAdd *event = q->m_registry.try_get<EventDefenderAdd>(e))
			renderEvents(event);
		else if (EventAttackPlayer *event = q->m_registry.try_get<EventAttackPlayer>(e))
			renderEvents(event);
		else if (EventAttackDefender *event = q->m_registry.try_get<EventAttackDefender>(e))
			renderEvents(event);
		else if (EventChangeBullet *event = q->m_registry.try_get<EventChangeBullet>(e))
			renderEvents(event);
		else if (EventChangeDefender *event = q->m_registry.try_get<EventChangeDefender>(e))
			renderEvents(event);
	}



	for (const auto &[mp, list] : listMpPick)
		renderEvents(mp, list);

	for (const auto &[e, list] : listTower)
		renderEvents(e, list);

	for (const auto &[mp, list] : listDefenderPut)
		renderEvents(mp, list);


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

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventMpPick);
	e.setTagId(q->m_registry.get<Player>(final->player).idTag());
	e.setTarget(mp->idTag);

	eventRealStore(std::move(e));



	// Next emitter event

	if (MpEmitter *emitter = q->m_registry.try_get<MpEmitter>(mp->emitter)) {
		std::erase(emitter->mpList, mpent);

		if (emitter->mpList.empty()) {
			LOG_CINFO("game") << "NEW MP";
			EventMpCreate e;
			e.setTick(q->lastAuthTick()+300);
			e.emitter = mp->emitter;

			eventStore(std::move(e));
		}
	} else {
		LOG_CERROR("game") << "Invalid emitter entity";
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

	const RpgStream::TowerState *last = q->getLastState<RpgStream::TowerState>(ent);
	RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(ent);

	int load = st.load();
	bool changed = false;

	for (EventTower *e : list) {
		Player *player = q->m_registry.try_get<Player>(e->player);

		Q_ASSERT(player);

		RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(e->player);



		if (e->lock) {
			if (e->skipLock) {
				pst.setLock(0);
			} else {
				const quint32 lockId = playerLock(e->player);

				if (lockId == 0) {
					continue;
				}

				pst.setLock(lockId);

				continue;
			}
		} else {
			pst.setLock(0);

			q->m_registry.remove<LockTag>(e->player);
		}


		playerIncreaseStreak(e->player, e->skipLock);


		if (st.team() == RpgStream::TeamNone) {
			st.setTeam(player->team);
			load = 40;
		} else if (player->team == st.team()) {
			load += 40;
		} else {
			load -= 30;
		}

		changed = true;

		// Register event

		RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventTower);
		ev.setTagId(player->idTag());
		ev.setTarget(tower->idTag);

		eventRealStore(std::move(ev));
	}



	if (!changed)
		return;

	if (load < 0) {
		load *= -1;
		st.setTeam(st.team() == RpgStream::TeamA ? RpgStream::TeamB : RpgStream::TeamA);
	}

	st.setLoad(std::min(100u, (quint32) load));

	if (load >= 100) {
		st.setActive(true);
		st.setLockedUntil(q->lastAuthTick()+1 + CFG_TOWER_LOCK);
	}

	if (load < CFG_TOWER_INACTIVE && st.active())
		st.setActive(false);


	// Register event

	if (last) {
		if (last->active() != st.active() || last->team() != st.team()) {
			if (last->active() && last->team() != st.team()) {
				EventTowerActiveChanged ev;
				ev.tower = ent;
				ev.active = false;
				ev.team = last->team();

				eventRealStore(std::move(ev));
			}


			EventTowerActiveChanged ev;
			ev.tower = ent;
			ev.active = st.active();
			ev.team = st.team();

			eventRealStore(std::move(ev));
		}
	}

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

	EventDefenderPut *final = list.front();			// Arbitrary select - TODO: earlier locked


	// Register player and tower state

	Player *p = q->m_registry.try_get<Player>(final->player);
	PlayerPrivate *pp = q->m_registry.try_get<PlayerPrivate>(final->player);

	Q_ASSERT(p);
	Q_ASSERT(pp);

	RpgStream::PlayerState &sp = getEditableCurrentState<RpgStream::PlayerState>(final->player);
	RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(d->tower);


	st.setHasDefender(true);


	//entt::entity object =
	generateDefender(p, pp, final->type, ent, d);

	sp.setHasDefender(false);



	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
	e.setTagId(q->m_registry.get<Player>(final->player).idTag());
	e.setTarget(d->idTag);

	eventRealStore(std::move(e));


}






/**
 * @brief RpgLogicPrivate::renderEvents
 * @param ent
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventDefenderAdd *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	LOG_CINFO("game") << "Add DEFENDER" << event->chunk.x << event->chunk.y;

	Player *p = q->m_registry.try_get<Player>(event->player);
	PlayerPrivate *pp = q->m_registry.try_get<PlayerPrivate>(event->player);

	Q_ASSERT(p);
	Q_ASSERT(pp);

	RpgStream::PlayerState &sp = getEditableCurrentState<RpgStream::PlayerState>(event->player);


	//entt::entity object =
	generateDefender(p, pp, event->type, entt::null, nullptr, event->chunk);

	sp.setHasDefender(false);



	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventDefender);
	e.setTagId(q->m_registry.get<Player>(event->player).idTag());
	e.setChunk(event->chunk.toRpgStream());

	eventRealStore(std::move(e));

}



/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventAttackPlayer *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	LOG_CINFO("game") << "Attack Player";

	Player *p = q->m_registry.try_get<Player>(event->player);
	Player *t = q->m_registry.try_get<Player>(event->target);

	PlayerPrivate *pCfg = q->m_registry.try_get<PlayerPrivate>(event->player);
	PlayerPrivate *tCfg = q->m_registry.try_get<PlayerPrivate>(event->target);

	Q_ASSERT(p);
	Q_ASSERT(t);
	Q_ASSERT(pCfg);
	Q_ASSERT(tCfg);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(event->player);
	RpgStream::PlayerState &tg = getEditableCurrentState<RpgStream::PlayerState>(event->target);

	if (st.bullet() > 0)
		st.setBullet(st.bullet()-1);


	// Todo ..

	playerDecreaseHp(event->target, 1);


	cpVect knockback = RpgLogic::addKnockbackImpulse(&tg.entityState(), st.entityState(), pCfg->toEntityConfig(), tCfg->toEntityConfig());
	if (!cpveql(knockback, cpvzero))
		q->m_registry.emplace_or_replace<KnockbackTag>(event->target, q->lastAuthTick());

	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackPlayer);
	e.setTagId(p->idTag());
	e.setTarget(t->idTag());

	eventRealStore(std::move(e));
}



/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventAttackDefender *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	LOG_CINFO("game") << "Attack Defender";

	Player *p = q->m_registry.try_get<Player>(event->player);
	DefenderObject *t = q->m_registry.try_get<DefenderObject>(event->target);

	Q_ASSERT(p);
	Q_ASSERT(t);


	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);
	RpgStream::DefenderState &st = getEditableCurrentState<RpgStream::DefenderState>(event->target);

	if (event->lock) {
		if (event->skipLock) {
			pst.setLock(0);
		} else {
			const quint32 lockId = playerLock(event->player);

			if (lockId == 0)
				return;

			pst.setLock(lockId);

			return;
		}

	} else {
		pst.setLock(0);

		q->m_registry.remove<LockTag>(event->player);
	}




	playerIncreaseStreak(event->player, event->skipLock);


	if (t->type == RpgStream::BaseDefenderObject::Dummy) {
		//// TODO
		///

		DefenderDummyObject *dummy = q->m_registry.try_get<DefenderDummyObject>(event->target);

		if (!dummy) {
			LOG_CWARNING("game") << "DefenderDummyObject entity not found";
			return;
		}

		st.setDummy(18);
	}



	if (st.hp() > 0)
		st.setHp(st.hp()-1);



	// Register event

	RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventAttackDefender);
	ev.setTagId(p->idTag());
	ev.setTarget(t->idTag);

	eventRealStore(std::move(ev));
}





/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventChangeBullet *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	LOG_CINFO("game") << "Change bullet";

	Player *p = q->m_registry.try_get<Player>(event->player);

	Q_ASSERT(p);

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(event->player);

	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);

	if (event->lock) {
		if (event->skipLock) {
			pst.setLock(0);
		} else {
			const quint32 lockId = playerLock(event->player);

			if (lockId == 0)
				return;

			pst.setLock(lockId);

			return;
		}

	} else {
		pst.setLock(0);

		q->m_registry.remove<LockTag>(event->player);
	}


	playerIncreaseStreak(event->player, event->skipLock);

	if (pst.mp() > CFG_MP_CHANGE_BULLET)
		pst.setMp(pst.mp()-CFG_MP_CHANGE_BULLET);
	else
		pst.setMp(0);

	pst.setBullet(pp.maxBullet);



	// Register event

	RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventChangeBullet);
	ev.setTagId(p->idTag());

	eventRealStore(std::move(ev));
}




/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventChangeDefender *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	LOG_CINFO("game") << "Change defender";

	Player *p = q->m_registry.try_get<Player>(event->player);

	Q_ASSERT(p);


	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);

	if (event->lock) {
		if (event->skipLock) {
			pst.setLock(0);
		} else {
			const quint32 lockId = playerLock(event->player);

			if (lockId == 0)
				return;

			pst.setLock(lockId);

			return;
		}

	} else {
		pst.setLock(0);

		q->m_registry.remove<LockTag>(event->player);
	}


	playerIncreaseStreak(event->player, event->skipLock);


	const quint32 mp = RpgStream::BaseDefenderObject::requiredMp(event->type);

	if (pst.mp() > mp)
		pst.setMp(pst.mp()-mp);
	else
		pst.setMp(mp);

	pst.setHasDefender(true);



	// Register event

	RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventChangeDefender);
	ev.setTagId(p->idTag());

	eventRealStore(std::move(ev));
}




/**
 * @brief RpgLogicPrivate::renderEntityKnockbacks
 */

void RpgLogicPrivate::renderEntityKnockbacks()
{
	QMutexLocker locker(&q->m_mutex);

	if (q->lastAuthTick() < 1) {
		LOG_CINFO("game") << "SKIP";
		return;
	}

	auto view = q->m_registry.view<KnockbackTag>();

	for (auto e : view) {
		const quint32 &tick = q->m_registry.get<KnockbackTag>(e).tick;

		if (tick == q->lastAuthTick())
			continue;

		if (q->m_registry.try_get<Player>(e)) {
			RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(e);

			const cpVect knock = RpgLogic::decayKnockback(&st.entityState());

			if (cpveql(knock, cpvzero)) {
				LOG_CINFO("game") << "REMOVE KNOCKBACK";

				q->m_registry.remove<KnockbackTag>(e);
			}

		} else {
			LOG_CERROR("game") << "Invalid knockback entity";
		}
	}

}






/**
 * @brief RpgLogicPrivate::renderPlayerInputs
 */

void RpgLogicPrivate::renderPlayerInputs()
{
	QMutexLocker locker(&q->m_mutex);


	if (q->lastAuthTick() < 1) {
		LOG_CINFO("game") << "SKIP";
		return;
	}

	const quint32 tick = q->lastAuthTick();			// Mert

	auto view = q->m_registry.view<Player>();

	for (auto e : view) {
		auto [player, input] =
				q->m_registry.try_get<Player, PlayerStateInput>(e);

		const RpgStream::PlayerState *lastInput = input ? input->at(tick) : nullptr;
		const RpgStream::PlayerState *current = q->getCurrentState<RpgStream::PlayerState>(e);

		if (!current && !lastInput)
			continue;

		if (!current) {
			LOG_CERROR("game") << "MISSING STATE" << tick;
			continue;
		}


		RpgStream::PlayerState state = *current;

		state.setTick(tick);

		// Ha van input, akkor csak az engedélyezett mezőket írjuk felül

		if (current->hp() > 0 && lastInput) {
			state.entityState().setPosX(lastInput->entityState().posX());
			state.entityState().setPosY(lastInput->entityState().posY());
			state.entityState().setVelX(lastInput->entityState().velX());
			state.entityState().setVelY(lastInput->entityState().velY());
			state.entityState().setAngle(lastInput->entityState().angle());
			state.entityState().setFacing(lastInput->entityState().facing());
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
					   (def.defender == entt::null ? 1 : CFG_DEFENDER_DESTROY));

			eventStore(std::move(ev));


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

quint32 RpgLogicPrivate::nextIdTag(Player *player, PlayerPrivate *p)
{
	Q_ASSERT(player);
	Q_ASSERT(p);

	QMutexLocker locker(&q->m_mutex);
	return q->packId(1, player->playerData.playerId(), ++p->lastObjectId);
}




/**
 * @brief RpgLogicPrivate::nextLockId
 * @return
 */

quint32 RpgLogicPrivate::nextLockId()
{
	QMutexLocker locker(&q->m_mutex);
	return ++m_lastLockId;
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

	PlayerPrivate &priv = q->m_registry.get<PlayerPrivate>(ent);
	RpgStream::PlayerPosition *p = q->m_registry.try_get<RpgStream::PlayerPosition>(ent);

	RpgStream::PlayerState &state = getEditableCurrentState<RpgStream::PlayerState>(ent);

	state.setTick(q->lastAuthTick());
	state.setHp(priv.maxHp);
	state.setLock(0);

	state.entityState().setPosX(p ? p->posX() : 0.f);
	state.entityState().setPosY(p ? p->posY() : 0.f);
	state.entityState().setVelX(0);
	state.entityState().setVelY(0);


	///state.setBullet(cfg.maxBullet());			/// ez törlendő!!!
	priv.defenders.insert(RpgStream::BaseDefenderObject::Dummy);
	state.setDefender(RpgStream::BaseDefenderObject::Dummy);


	priv.modSkipLock = 3;



	LOG_CINFO("game") << "INITIELIZES" << player->playerData.playerId() << q->lastAuthTick()
					  << state.entityState().posXAsFloat() << state.entityState().posYAsFloat() << state.hp()
					  << "***MP" << state.mp();


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


	std::vector<RpgStream::Events> eventList = m_registry.ctx().get<EventsOutput>().extractAtLeast(lastAuthTick()-3, maxTick);

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
	d->renderEntityKnockbacks();
	d->renderPlayerInputs();
	d->renderFinal();

	d->storeRealEvents();



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
			preRenderEventMpCreate(e);
		} else if (q->m_registry.all_of<EventDefenderDestroy>(e)) {
			preRenderEventDefenderDestroy(e);
		} else if (EventPlayerRespawn *ev = q->m_registry.try_get<EventPlayerRespawn>(e)) {
			playerInitialize(ev->player);
		}

		q->m_registry.emplace<DeleteTag>(e);
	}


	// Remove penalties

	for (auto e : q->m_registry.view<PenaltyTag>()) {
		if (q->m_registry.get<PenaltyTag>(e).expire < tick) {
			q->m_registry.remove<PenaltyTag>(e);
			LOG_CDEBUG("game") << "PENALTY ENDS";
		}
	}


	// Remove outdated locks

	for (auto e : q->m_registry.view<Player, LockTag>()) {
		if (q->m_registry.get<LockTag>(e).expire < tick) {
			playerUnlock(e, CFG_PENALTY_AUTO_UNLOCK);

			LOG_CINFO("game") << "UNLOCKED";
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
void RpgLogicPrivate::eventStore(T &&event)
{
	QMutexLocker locker(&q->m_mutex);

	auto entity = q->m_registry.create();

	q->m_registry.emplace<EventTag>(entity, event.tick());
	q->m_registry.emplace<T>(entity, std::move(event));
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




/**
 * @brief RpgLogicPrivate::eventRealStore
 * @param event
 * @return
 */


template<typename T>
entt::entity RpgLogicPrivate::eventRealStore(T &&event)
{
	QMutexLocker locker(&q->m_mutex);

	auto final = q->m_registry.create();
	q->m_registry.emplace<EventRealTag>(final);
	q->m_registry.emplace<T>(final, std::move(event));

	return final;
}





/**
 * @brief PlayerPrivate::canSkipLock
 * @return
 */

void PlayerPrivate::load(const RpgStream::PlayerConfig &cfg)
{
	power = std::max((quint8) 1, cfg.power());

	float factor = 1.0 + (power-1) * 0.1;

	push = cfg.entity().push() * factor;
	pushDist = cfg.entity().pushDist() * factor;
	resist = cfg.entity().resist() * factor;


	factor = 1.0 + (power-1) * 0.25;

	maxHp = cfg.entity().maxHp() * factor;
	maxMp = cfg.maxMp() * factor;
	maxBullet = cfg.maxBullet() * factor;


	const int maxDefender = cfg.power() > 5 ? 3 :
											  cfg.power() > 2 ? 2 : 1;

	for (const RpgStream::BaseDefenderObject::Type &d : cfg.defenders()) {
		if (defenders.size() >= maxDefender)
			break;

		defenders.insert(d);
	}


	// Streak

	modSkipLock = std::max((quint32) 2, (quint32) std::ceil(5-((cfg.power()-1)/2.)));

}



/**
 * @brief PlayerPrivate::toEntityConfig
 * @return
 */

RpgStream::EntityConfig PlayerPrivate::toEntityConfig() const
{
	RpgStream::EntityConfig cfg;

	cfg.setPush(push);
	cfg.setPushDist(pushDist);
	cfg.setResist(resist);

	return cfg;
}




/**
 * @brief PlayerPrivate::canSkipLock
 * @return
 */

bool PlayerPrivate::canSkipLock() const
{
	// Ha 0 vagy 1, akkor mindet ki lehet hagyni

	if (modSkipLock == 0 || modSkipLock == 1)
		return true;


	if ((passedQuestions+1) % modSkipLock == 0)
		return true;

	return false;
}


















}		// end namespace
