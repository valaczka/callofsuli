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
#include <random>




#define MAX_FUTURE_TICK					10


namespace Rpg {




// Step defenders

struct EventDefenderStep {
	entt::entity defender;
	std::unordered_set<entt::entity> targets;
};




struct PlayerPrivate;


/**
 * @brief The RpgLogicPrivate class
 */

class RpgLogicPrivate
{
private:
	RpgLogicPrivate(RpgLogic *logic) : q(logic) {}
	~RpgLogicPrivate() = default;

#ifdef USE_LOGGER
	Logger *_logger() const { return q->m_logger; }
#endif


	/// Owners
	///
	/// 0: Libtiled
	/// 1-127: Player
	/// 128-: RpgLogic
	///
	/// RpgLogic owners
	/// 128: MP
	/// 129: NPC
	/// ...

	enum LogicIdOwner {
		IdMp = 128,
		IdNpc,
		IdControl
	};

	quint32 nextIdTag(const LogicIdOwner &ownerId, const quint32 &scene = 1);
	quint32 nextIdTag(Player *player, PlayerPrivate *p, const quint32 &scene = 1);

	quint32 nextLockId();



	// Initialize

	void mapInitialize();
	void loadHeat(const quint8 &heat);



	// Players

	void playerUpdate(const std::vector<RpgStream::PlayerData> &list);
	void playerUpdate(entt::entity ent, const RpgStream::PlayerData &data);

	bool emplacePlayers();
	bool playerInitialize(entt::entity ent);

	void playerInputLoad(entt::entity ent, const std::vector<RpgStream::PlayerState> &list);
	void playerInputLoad(const quint32 &tag, const std::vector<RpgStream::PlayerState> &list) {
		playerInputLoad(q->entityFromIdTag(tag), list);
	}
	void playerInputLoad(const std::vector<RpgStream::PlayerStateList> &list,
						 EventWindowHash *acceptedInputList = nullptr, const bool &delta = true);

	quint32 playerLock(entt::entity player, const quint32 &penalty = 1);
	bool playerUnlock(entt::entity player, const quint32 &penaltyTick = 0);
	quint32 playerDecreaseHp(entt::entity player, const quint32 &count = 1);
	quint32 playerIncreaseStreak(entt::entity player, const bool &skip);




	// Mp

	void loadMpEmitters(const std::vector<RpgStream::MpEmitter> &list);

	void mpEmitterUpdate(const std::vector<RpgStream::MpEmitter> &list);


	// Tower

	void loadTower(const std::vector<RpgStream::Tower> &list);
	void loadDefender(entt::entity towerEntity, Tower &tower, const std::vector<RpgStream::Defender> &list);

	void towerUpdate(const std::vector<RpgStream::Tower> &list);




	// DefenderObject

	entt::entity generateDefender(Player *player, PlayerPrivate *priv, const RpgStream::BaseDefenderObject::Type &type,
								  entt::entity defEnt, Defender *defender, const Chunk &chunk = {});

	void defenderUpdate(const std::vector<RpgStream::BaseDefenderObject> &list);
	void defenderUpdate(entt::entity entity, const RpgStream::BaseDefenderObject &stream, DefenderObject *object,
						RpgStream::DefenderState *state);

	entt::entity defenderNextTarget(DefenderObject &defender) const;
	entt::entity defenderRender(DefenderObject *defender, RpgStream::DefenderState &state);
	void defenderRenderPulse(DefenderObject *defender, entt::entity target);

	bool checkInFog(entt::entity entity, const bool &isAttacker) const;
	bool checkInFog(const cpVect &pos, const RpgStream::Team &team) const;

	// Npc

	entt::entity generateNpc(Player *player, PlayerPrivate *priv, const RpgStream::NpcData &data,
							 const cpVect &pos = cpvzero, quint32 *idTagPtr = nullptr);

	bool npcInitialize(entt::entity ent);

	void npcUpdate(const std::vector<RpgStream::NpcData> &list);
	void npcUpdate(entt::entity entity, const RpgStream::NpcData &stream, Npc *object, RpgStream::NpcState *state);
	void npcUpdate(entt::entity entity, const Npc &object, RpgStream::NpcState &state);

	void npcInputLoad(const std::vector<RpgStream::NpcStateList> &list,
					  EventWindowHash *acceptedInputList = nullptr, const bool &delta = true);

	void npcInputLoad(entt::entity ent, const std::vector<RpgStream::NpcState> &list);
	void npcInputLoad(const quint32 &tag, const std::vector<RpgStream::NpcState> &list) {
		npcInputLoad(q->entityFromIdTag(tag), list);
	}

	void npcRenderInput(const Npc &npc, const RpgStream::NpcState &input, RpgStream::NpcState &dest);
	QString npcFullState(const Npc &npc, const RpgStream::NpcState &state) const;

	quint32 npcDecreaseHp(entt::entity npc, const quint32 &count = 1);



	// Utility

	bool utility(Player *player, PlayerPrivate *priv, const RpgStream::PlayerConfig::Utility &type,
				 entt::entity target);
	bool utilityMissionary(Utility &utility, entt::entity target);
	bool utilitySniper(Utility &utility, entt::entity target);



	// Events

	void eventInputLoad(const std::vector<RpgStream::Events> &list, EventWindowHash *acceptedInputList);
	void eventInputLoad(const std::vector<RpgStream::EventPlayer> &list, EventWindowHash *acceptedInputList);
	void eventInputLoad(const std::vector<RpgStream::EventNpc> &list, EventWindowHash *acceptedInputList);

	template <typename T>
	entt::entity eventFinalStore(T &&event);

	template <typename T>
	entt::entity eventRealStore(T &&event);

	void storeRealEvents();
	bool addToCurrentEvents(entt::entity entity, RpgStream::Events *dst);

	void changeStage(const RpgStream::GameConfig::Stage &stage);
	void autoSelectInventory();



	// Internal events

	void onEventStageChanged(const RpgStream::EventStageChanged &event);



	// Controls

	template <class T>
	entt::entity controlAdd(const RpgStream::ControlData &data, const T &controlData, quint32 *tagIdPtr = nullptr);

	void controlUpdate(const std::vector<RpgStream::ControlData> &list);
	void controlUpdate(entt::entity entity, const RpgStream::ControlData &stream, Control *object, RpgStream::ControlState *state);

	bool preRenderControlCheck(Control *control, const RpgStream::ControlState &state, Player *player);
	void controlUse(entt::entity entity, Control *control, Player *player, PlayerPrivate *priv);


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
	void preRenderEventChangeUtility(const RpgStream::EventPlayer &event);
	void preRenderEventReplaceDefender(const RpgStream::EventPlayer &event);
	void preRenderEventReplaceUtility(const RpgStream::EventPlayer &event);
	void preRenderEventUseUtility(const RpgStream::EventPlayer &event);
	void preRenderEventUseControl(const RpgStream::EventPlayer &event);

	void preRenderEventNpcCreate(entt::entity ent);
	void preRenderEventNpc(entt::entity event);
	void preRenderEventNpcAttack(const RpgStream::EventNpc &event);
	void preRenderEventNpcAttack(entt::entity entity, entt::entity target,
								 Npc* npc, DefenderObject *object, const RpgStream::EventNpc &event);
	void preRenderEventNpcAttack(entt::entity entity, entt::entity target,
								 Npc* npc, Tower *object, const RpgStream::EventNpc &event);
	void preRenderEventNpcAttack(entt::entity entity, entt::entity target,
								 Npc* npc, Player *object, const RpgStream::EventNpc &event);

	void preRenderEventDefenderDestroy(entt::entity ent);

	void preRenderDefenders();

	void preRenderUtilities();

	void preRenderEventControlStateChange(entt::entity ent);

	void renderEvents();
	void renderEvents(entt::entity mpent, const std::vector<EventMpPick *> &list);
	void renderEvents(entt::entity ent, const std::vector<EventTower*> &list);
	void renderEvents(entt::entity ent, const std::vector<EventDefenderPut*> &list);
	void renderEvents(entt::entity ent, const std::vector<EventControl*> &list);
	void renderEvents(EventDefenderAdd *event);
	void renderEvents(EventAttackPlayer *event);
	void renderEvents(EventAttackNpc *event);
	void renderEvents(EventAttackDefender *event);
	void renderEvents(EventChangeBullet *event);
	void renderEvents(EventChangeDefender *event);
	void renderEvents(EventChangeUtility *event);
	void renderEvents(EventUtility *event);
	void renderEvents(EventNpcAttackDefender *event);
	void renderEvents(EventNpcAttackTower *event);
	void renderEvents(EventNpcAttackPlayer *event);
	void renderEvents(EventDefenderStep *event);
	void renderEvents(EventPatchPlayer *event);

	void renderEntityKnockbacks();

	void renderPlayerInputs(const bool &first);
	void renderNpcInputs(const bool &first);

	void renderFinal();
	void renderFinalTowers();
	void renderFinalDefenders();
	void renderFinalStage();
	void renderFinalControls();


	void removeDeleteTags();


	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	T &getEditableCurrentState(entt::entity ent);

	//RpgStream::Events &getEditableCurrentEventList();


	template <typename T>
	void renderFinal(entt::entity ent, T &&event);

private:
	RpgLogic *const q;
	quint32 m_lastLockId = 0;

	QHash<LogicIdOwner, quint32> m_lastObjectId;

	qint64 m_lastFullLoadTick = -1;

	bool m_requireFull = false;					// Szükséges-e Full-t küldeni a render() végén, vagy elég FullState

	std::optional<RpgStream::GameConfig::Stage> m_oldStage;

	friend class RpgLogic;
};










// Internal (private) data for players

struct PlayerPrivate
{
	void load(const RpgStream::PlayerConfig &cfg, RpgStream::PlayerData &dst, const bool &isHosted);
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

	quint32 towerPlus = 0;
	quint32 towerMinus = 0;

	quint32 penaltyMsec = 0;

	QSet<RpgStream::BaseDefenderObject::Type> defenders;
	QSet<RpgStream::PlayerConfig::Utility> utilities;



	// Streak

	quint32 modSkipLock = 0;
	quint32 passedQuestions = 0;
	quint32 passedStreak = 0;
	quint32 passedQuestStreak = 0;

	quint32 requiredStreak = 0;

	bool canSkipLock() const;
	void increasePassedQuestions(const bool &skipped) {
		++passedQuestions;
		if (!skipped) {
			++passedStreak;
			passedQuestStreak = std::max(passedQuestStreak, passedStreak);
		}
	}

	void resetStreak() {
		passedStreak = 0;

		if (passedQuestStreak < requiredStreak)
			passedQuestStreak = 0;
	}
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




// Stage change

class EventStageChange : public RpgStream::BaseTickState
{
public:
	EventStageChange() : RpgStream::BaseTickState() {}

	RpgStream::GameConfig::Stage stage = RpgStream::GameConfig::StageInit;
};








/**
 *
 * @brief RpgLogic::RpgLogic
 */

RpgLogic::RpgLogic(const quint32 &lastAuthDiff)
	: d(new RpgLogicPrivate(this))
	, m_lastAuthTickDiff(lastAuthDiff)
	, m_rnd(std::random_device{}())
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
 * @brief RpgLogic::rewindStage
 * @param oldStage
 */

void RpgLogic::rewindStage(const RpgStream::GameConfig::Stage &/*oldStage*/)
{
	// Itt nem haszálnjuk, csak a RpgLogicClient-nél érdekes
}




/**
 * @brief RpgLogic::initializeTowers
 */

std::unordered_set<entt::entity> RpgLogic::initializeTowers()
{
	QMutexLocker locker(&m_mutex);

	// Towers

	ELOG_DEBUG << "Randomize towers";

	std::vector<entt::entity> towers;

	for (entt::entity e : m_registry.view<Tower>())
		towers.push_back(e);

	std::shuffle(towers.begin(), towers.end(), m_rnd);
	std::unordered_set<entt::entity> r;

	r.reserve(CFG_TOWER_COUNT);

	for (entt::entity e : towers) {
		if (r.size() >= CFG_TOWER_COUNT)
			break;

		r.insert(e);
	}

	return r;
}




/**
 * @brief RpgLogic::initializeEmitters
 */

std::unordered_set<entt::entity> RpgLogic::initializeEmitters()
{
	QMutexLocker locker(&m_mutex);

	const quint64 numEmitter = 2;

	// Mp emitters

	ELOG_DEBUG << "Randomize MP emitters";

	std::vector<entt::entity> emitters;

	for (entt::entity e : m_registry.view<MpEmitter>())
		emitters.push_back(e);

	std::shuffle(emitters.begin(), emitters.end(), m_rnd);

	std::unordered_set<entt::entity> r;

	r.reserve(numEmitter);

	for (entt::entity e : emitters) {
		if (r.size() >= numEmitter)
			break;

		r.insert(e);
	}

	return r;
}



/**
 * @brief RpgLogic::initializeStages
 */

void RpgLogic::initializeStages()
{
	// Stage main

	EventStageChange ev;
	ev.setTick(CFG_GAME_STAGE_MAIN);
	ev.stage = RpgStream::GameConfig::StageMain;

	eventStore(std::move(ev));

	// Stage last

	EventStageChange ev2;
	ev2.setTick(CFG_GAME_STAGE_LAST);
	ev2.stage = RpgStream::GameConfig::StageLast;

	eventStore(std::move(ev2));


	// Heat initiliazie

	d->loadHeat(0);

}


/**
 * @brief RpgLogic::checkState
 * @param state
 */

void RpgLogic::checkState(const RpgStream::GameState &state)
{
	Q_UNUSED(state);
}




/**
 * @brief RpgLogic::initializeChests
 * @return
 */

ChestList RpgLogic::initializeChests()
{
	QMutexLocker locker(&m_mutex);

	const int numChests = m_registry.ctx().get<HeatList>().size()-1;

	if (numChests <= 0) {
		ELOG_INFO << "Heat list empty, skip chests";
		return {};
	};

	// Chest

	ELOG_DEBUG << "Randomize chests";

	ChestList list = m_registry.ctx().get<ChestList>();

	std::shuffle(list.begin(), list.end(), m_rnd);

	if ((int) list.size() > numChests) {
		list.erase(list.cbegin()+numChests, list.cend());
	}

	return list;
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

	ChestList chest;

	for (const RpgStream::PlayerPosition &p : data.chestPositionList()) {
		Chest c;
		c.pos.x = p.posXAsFloat();
		c.pos.y = p.posYAsFloat();
		chest.emplace_back(std::move(c));
	}

	HeatList heat = data.heat();

	m_registry.ctx().insert_or_assign<ChunkGrid>(std::move(grid));
	m_registry.ctx().insert_or_assign<PlayerPositionList>(std::move(l));
	m_registry.ctx().insert_or_assign<ChestList>(std::move(chest));
	m_registry.ctx().insert_or_assign<HeatList>(std::move(heat));

	d->loadMpEmitters(data.mpEmitterList());
	d->loadTower(data.towerList());
}





/**
 * @brief RpgLogic::reloadMapData
 * @param data
 */

void RpgLogic::reloadMapData(const RpgStream::MapData &data)
{
	QMutexLocker locker(&m_mutex);

	RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	if (cfg.flags().testFlag(RpgStream::GameConfig::FlagDataReloaded))
		return;

	if (!cfg.flags().testFlag(RpgStream::GameConfig::FlagDataPrepared)) {
		ELOG_ERROR << "Reload map data error, map isn't prepared";
		return;
	}


	ELOG_DEBUG << "Reload map data (chunk grid)";

	ChunkGrid grid = ChunkGrid::fromRpgStream(data.chunkGrid());

	m_registry.ctx().insert_or_assign<ChunkGrid>(std::move(grid));

	cfg.flags().setFlag(RpgStream::GameConfig::FlagDataReloaded);
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
		ELOG_ERROR << "Missing ChunkGrid";
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
 * @brief RpgLogic::getMapData
 * @return
 */

std::optional<RpgStream::MapData> RpgLogic::getMapData() const
{
	QMutexLocker locker(&m_mutex);

	const ChunkGrid *chunk = m_registry.ctx().find<ChunkGrid>();
	const PlayerPositionList *pList = m_registry.ctx().find<PlayerPositionList>();

	if (!chunk || !pList)
		return std::nullopt;

	RpgStream::MapData map;

	map.setChunkGrid(chunk->toRpgStream());
	map.setPlayerPositionList(*pList);

	// Skip emitters
	// Skip towers

	return map;
}











/**
 * @brief RpgLogic::playerAdd
 * @param team
 * @return
 */

entt::entity RpgLogic::playerAdd(const RpgStream::PlayerData &data, quint32 *idPtr, quint32 *tagIdPtr)
{
	QMutexLocker locker(&m_mutex);

	auto view = m_registry.view<Player>();

	RpgStream::PlayerData playerData = data;

	ELOG_INFO << "Add player" << data.playerId();

	// Akkor adunk neki új sorszámot, ha kértünk idPtr-t (tehát multiplayerben csak a szerver adja, a kliens nem!)

	if (idPtr) {
		quint32 next = 1;

		for (const auto &e : view) {
			const auto &player = view.get<Player>(e);
			if (player.playerData.playerId() >= next)
				next = player.playerData.playerId() + 1;
		}

		*idPtr = next;

		playerData.setPlayerId(next);
	}

	auto entity = m_registry.create();

	m_registry.emplace<PlayerPrivate>(entity).load(data.config(), playerData, idPtr ? true : false);
	m_registry.emplace<PlayerStateInput>(entity);
	m_registry.emplace<PlayerStateOutput>(entity);

	const Player &pp = m_registry.emplace<Player>(entity, std::move(playerData), data.team());

	entitySetIdTag(entity, pp.idTag());

	if (tagIdPtr)
		*tagIdPtr = pp.idTag();

	return entity;
}




/**
 * @brief RpgLogic::npcAdd
 * @param data
 * @param owner
 * @param tagIdPtr
 * @return
 */

entt::entity RpgLogic::npcAdd(const RpgStream::NpcData &data, entt::entity owner, const cpVect &pos, quint32 *tagIdPtr)
{
	QMutexLocker locker(&m_mutex);

	Player *player = nullptr;
	PlayerPrivate *priv = nullptr;

	if (m_registry.valid(owner)) {
		player = m_registry.try_get<Player>(owner);
		priv = m_registry.try_get<PlayerPrivate>(owner);
	}

	return d->generateNpc(player, priv, data, pos, tagIdPtr);
}





/**
 * @brief RpgLogic::npcAdd
 * @param heatNpc
 */

void RpgLogic::npcAdd(const RpgStream::HeatNpc &heatNpc)
{
	QMutexLocker locker(&m_mutex);

	quint32 tick = lastAuthTick() + 1;

	std::optional<std::uniform_int_distribution<int> > dptr;

	if (heatNpc.positionList().size() > 1)
		dptr = std::uniform_int_distribution<int>(0, heatNpc.positionList().size()-1);

	for (quint32 i=0; i<heatNpc.num(); ++i) {
		EventNpcCreate evc;

		evc.setTick(tick + i*heatNpc.delay());
		evc.data = heatNpc.data();
		evc.data.setTeam(RpgStream::TeamNone);

		if (dptr) {
			int idx = (*dptr)(m_rnd);

			evc.pos.x = heatNpc.positionList().at(idx).posXAsFloat();
			evc.pos.y = heatNpc.positionList().at(idx).posYAsFloat();

		} else if (!heatNpc.positionList().empty()) {
			evc.pos.x = heatNpc.positionList().front().posXAsFloat();
			evc.pos.y = heatNpc.positionList().front().posYAsFloat();
		}

		ELOG_DEBUG << "NPC create event for" << evc.tick();

		eventStore(std::move(evc));
	}
}


/**
 * @brief RpgLogic::onNpcCreated
 * @param entity
 * @param idTag
 * @param player
 */

void RpgLogic::onNpcCreated(entt::entity entity, const quint32 &idTag, Player *player)
{
	Q_UNUSED(entity);
	Q_UNUSED(idTag);
	Q_UNUSED(player);
}






/**
 * @brief RpgLogic::onControlStateChange
 * @param entity
 * @param control
 * @param isAlive
 * @param state
 */

bool RpgLogic::onControlStateChange(entt::entity entity, Control *control, const bool &isAlive, const quint32 &state)
{
	Q_UNUSED(entity)
	Q_UNUSED(control)
	Q_UNUSED(isAlive)
	Q_UNUSED(state)

	return true;
}



/**
 * @brief RpgLogic::getResult
 * @return
 */

RpgStream::Result RpgLogic::getResult()
{
	return getResultByTeam(RpgStream::TeamNone);
}


/**
 * @brief RpgLogic::getQuestList
 * @return
 */

QuestList RpgLogic::getQuestList() const
{
	return QuestList{};
}






/**
 * @brief RpgLogic::getResultByTeam
 * @param team
 * @return
 */

RpgStream::Result RpgLogic::getResultByTeam(const RpgStream::Team &team)
{
	QMutexLocker locker(&m_mutex);

	RpgStream::Result res;

	const RpgStream::GameState &state = m_registry.ctx().get<RpgStream::GameState>();

	for (auto e : m_registry.view<Player>()) {
		const Player &p = m_registry.get<Player>(e);
		const RpgStream::PlayerState *last = getLastState<RpgStream::PlayerState>(e);

		if (!last) {
			ELOG_ERROR << "Invalid PlayerState" << p.playerData.playerId();
			continue;
		}

		RpgStream::PlayerResult r;

		r.setPlayerId(p.idTag());
		r.setTeam(p.playerData.team());
		r.setQuest(p.playerData.quest());

		if (team == RpgStream::TeamNone)
			r.setHeat(state.heat());

		r.result().setQuestion(last->question());
		r.result().setStreak(last->streak());


		bool success = last->question() >= p.playerData.quest().question()  &&
					   last->streak() >= p.playerData.quest().streak();

		if (team == RpgStream::TeamNone)
			success &= state.ptsA() >= p.playerData.quest().pts();
		else
			success &= p.playerData.team() == team;


		r.setSuccess(success);


		float factor = 0.;

		float fq = p.playerData.quest().question() > 0 ?
					   ((float) last->question() / (float) p.playerData.quest().question()) :
					   0.f;

		float fp = p.playerData.quest().pts() > 0 ?
					   ((float) state.ptsA() / (float) p.playerData.quest().pts()) :
					   0.f;

		const float fs = last->streak() >= p.playerData.quest().streak() ? 1.f : 0.f;


		if (!success) {
			if (fq > 1.f) fq = 1.f;
			if (fp > 1.f) fp = 1.f;
		}


		float weight = CFG_RESULT_WEIGHT_QUESTION + CFG_RESULT_WEIGHT_STREAK;

		factor += fq * CFG_RESULT_WEIGHT_QUESTION;
		factor += fs * CFG_RESULT_WEIGHT_STREAK;

		if (team == RpgStream::TeamNone) {
			factor += fp * CFG_RESULT_WEIGHT_PTS;
			weight += CFG_RESULT_WEIGHT_PTS;
		}

		factor /= weight;


		if (team == RpgStream::TeamNone && success)
			factor *= (1. + state.heat() * CFG_RESULT_HEAT_RATIO);


		r.result().setToken(p.playerData.quest().token() * factor);
		r.result().setXp(p.playerData.quest().xp() * factor);

		if (team == RpgStream::TeamNone)
			r.result().setPts(state.ptsA() * factor);
		else if (p.playerData.team() == RpgStream::TeamA)
			r.result().setPts(state.ptsA() * factor);
		else
			r.result().setPts(state.ptsB() * factor);


		ELOG_DEBUG << "[RESULT] Player" << p.playerData.playerId() << p.idTag() << success << factor << state.heat() << "---->"
				   << r.result().token()
				   << r.result().xp()
				   << r.result().pts();

		res.players().emplace_back(std::move(r));
	}

	return res;
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

	if (distSq < 0.001f)
		return knockback;

	if (dist > attacker.pushDist())
		return knockback;

	float power = attacker.push() * distanceFalloff(dist, attacker.pushDist())
				  - target.resist()
				  ;

	if (power <= 0)
		return knockback;

	cpVect dir = cpvnormalize(delta);

	cpVect knock = cpvmult(dir, power);

	knockback = cpvclamp(cpvadd(knockback, knock), CFG_MAX_KNOCKBACK);

	targetState->setSlideXAsFloat(knockback.x);
	targetState->setSlideYAsFloat(knockback.y);

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
 * @brief RpgLogic::oppositeTeam
 * @param team
 * @return
 */

RpgStream::Team RpgLogic::oppositeTeam(const RpgStream::Team &team)
{
	switch (team) {
		case RpgStream::TeamA:
			return RpgStream::TeamB;
		case RpgStream::TeamB:
			return RpgStream::TeamA;
		default:
			return RpgStream::TeamNone;
	}

	return RpgStream::TeamNone;
}





/**
 * @brief RpgLogic::checkInFog
 * @param pos
 * @param team
 * @return
 */

bool RpgLogic::checkInFog(const cpVect &pos, const RpgStream::Team &team) const
{
	return d->checkInFog(pos, team);
}








/**
 * @brief RpgLogic::eventRealizedDefault
 * @param entity
 */

void RpgLogic::eventRealizedDefault(entt::entity entity)
{
	QMutexLocker locker(&m_mutex);

	const quint32 tick = lastAuthTick();

	if (RpgStream::EventStageChanged *ev = m_registry.try_get<RpgStream::EventStageChanged>(entity)) {
		d->onEventStageChanged(*ev);
		return;
	}

	if (Rpg::EventMpEmitterEmpty *ev = m_registry.try_get<Rpg::EventMpEmitterEmpty>(entity)) {
		if (!m_registry.valid(ev->emitter)) {
			ELOG_ERROR << "Invalid emitter";
			return;
		}

		ELOG_DEBUG << "Emitter empty" << m_registry.get<Rpg::MpEmitter>(ev->emitter).idTag << "at" << tick;

		Rpg::EventMpCreate evc = Rpg::EventMpCreate::createMp(m_registry.ctx().get<RpgStream::GameConfig>().stage(), tick);
		evc.emitter = ev->emitter;

		ELOG_DEBUG << "Register MP create event for" << evc.tick();

		eventStore(std::move(evc));

		return;
	}
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
		ELOG_ERROR << "Invalid player";
		return;
	}

	input->load(list, q->lastAuthTick(), q->m_serverTick + MAX_FUTURE_TICK);
}




/**
 * @brief RpgLogic::playerInputLoad
 * @param list
 */

void RpgLogicPrivate::playerInputLoad(const std::vector<RpgStream::PlayerStateList> &list,
									  EventWindowHash *acceptedInputList, const bool &delta)
{
	for (const RpgStream::PlayerStateList &ps : list) {
		if (!acceptedInputList || acceptedInputList->contains(ps.tagId()))
			playerInputLoad(ps.tagId(), delta ? ps.extractStateVector() : ps.state());
		else
			ELOG_ERROR << "Unacceptable input" << ps.tagId();
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
		ELOG_ERROR << "Invalid player";
		return 0;
	}

	Player *p = q->m_registry.try_get<Player>(player);

	if (!p) {
		ELOG_ERROR << "Invalid player";
		return 0;
	}

	if (q->m_registry.all_of<LockTag>(player)) {
		ELOG_WARNING << "Player already locked" << p->playerData.playerId();
		return 0;
	}


	const quint32 id = nextLockId();
	const quint32 expire = q->lastAuthTick() + CFG_QUESTION_MAX_DURATION;

	q->m_registry.emplace<LockTag>(player, id, expire, penalty);

	ELOG_DEBUG << "Player" << p->playerData.playerId() << "lock start" << id << "at" << q->lastAuthTick();

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
		ELOG_ERROR << "Invalid player entity";
		return false;
	}

	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (!pLock) {
		ELOG_ERROR << "LockTag missing";
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


	PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	pp.resetStreak();

	st.setStreak(pp.passedQuestStreak);


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
		ELOG_ERROR << "Invalid player entity";
		return false;
	}

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);


	if (st.hp() == 0) {
		ELOG_WARNING << "Player isn't alive";
		return 0;
	}

	quint32 nextHp = st.hp() - std::min(count, st.hp());

	st.setHp(nextHp);


	if (nextHp > 0)
		return nextHp;


	quint32 nextTick = q->lastAuthTick() + CFG_PLAYER_RESPAWN;


	// Register respawn event

	const Player &p = q->m_registry.get<Player>(player);

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventRespawn);
	e.setTagId(p.idTag());
	e.setAt(nextTick);

	eventRealStore(std::move(e));


	ELOG_INFO << "Player" << p.playerData.playerId() << "back at" << nextTick;

	EventPlayerRespawn ev;
	ev.setTick(nextTick);
	ev.player = player;

	q->eventStore(std::move(ev));


	// Register mp create event

	if (st.mp() < 2)
		return nextHp;

	quint32 eCount = std::floor((float) st.mp() / 2.);

	st.setMp(st.mp() - eCount);

	EventMpCreate evc;
	evc.setTick(q->lastAuthTick()+2);
	evc.parent = player;
	evc.mpCount = eCount;
	evc.pos.x = st.entityState().posXAsFloat();
	evc.pos.y = st.entityState().posYAsFloat();

	q->eventStore(std::move(evc));

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
		ELOG_ERROR << "Invalid player entity";
		return 0;
	}


	Player *p = q->m_registry.try_get<Player>(player);

	Q_ASSERT(p);

	PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	pp.increasePassedQuestions(skip);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	st.setQuestion(pp.passedQuestions);
	st.setStreak(pp.passedQuestStreak);



	const auto it = cfgRewardStreak.find(pp.passedStreak);

	if (it == cfgRewardStreak.constEnd())
		return pp.passedStreak;




	if (st.hp() == 0) {
		ELOG_WARNING << "Player" << p->playerData.playerId() << "isn't alive";
		return pp.passedStreak;
	}

	st.setHp(st.hp() + it->hp);




	// Register respawn event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventStreak);
	e.setTagId(p->idTag());
	e.setAt(pp.passedStreak);

	eventRealStore(std::move(e));


	RpgStream::GameState &state = q->m_registry.ctx().get<RpgStream::GameState>();

	quint32 point = it->point * pp.power;

	if (p->team == RpgStream::TeamA)
		state.setPtsA(state.ptsA() + point);
	else if (p->team == RpgStream::TeamB)
		state.setPtsB(state.ptsB() + point);

	ELOG_DEBUG << "Player" << p->playerData.playerId() << "streak" << pp.passedStreak;


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

		q->entitySetIdTag(entity, emitter.idTag);

		q->m_registry.emplace<MpEmitter>(entity, std::move(emitter));

	}
}




/**
 * @brief RpgLogicPrivate::mpEmitterUpdate
 * @param list
 */

void RpgLogicPrivate::mpEmitterUpdate(const std::vector<RpgStream::MpEmitter> &list)
{
	QMutexLocker locker(&q->m_mutex);

	std::unordered_map<quint32, entt::entity> entities;
	entities.reserve(list.size());

	for (auto e : q->m_registry.view<MpEmitter>()) {
		if (!q->m_registry.valid(e))
			continue;

		entities[q->m_registry.get<MpEmitter>(e).idTag] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());

	std::vector<RpgStream::MpEmitter> createList;

	for (const RpgStream::MpEmitter &p : list) {
		ids.insert(p.tagId());

		auto it = entities.find(p.tagId());

		if (it == entities.end()) {
			createList.push_back(p);
		} else {
			q->m_registry.get<MpEmitter>(it->second).active = p.active();
		}
	}

	if (!createList.empty())
		loadMpEmitters(createList);

	for (const auto &[id, e] : entities) {
		if (ids.contains(id))
			continue;

		q->m_registry.emplace<DeleteTag>(e);
	}
}





/**
 * @brief RpgLogicPrivate::generateMp
 */

void RpgLogicPrivate::preRenderEventMpCreate(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent)) {
		ELOG_ERROR << "Missing entity";
		return;
	}

	const EventMpCreate &event = q->m_registry.get<EventMpCreate>(ent);

	quint32 eventIdTag = 0;

	int num = 0;
	float radius = 100.;
	cpVect center = cpvzero;

	MpEmitter *emitter = nullptr;

	ChunkGrid *grid = nullptr;

	if (q->m_registry.valid(event.emitter)) {
		emitter = q->m_registry.try_get<MpEmitter>(event.emitter);

		if (!emitter) {
			ELOG_ERROR << "Invalid emitter";
			return;
		}

		if (!emitter->active) {
			ELOG_ERROR << "Inactive emitter";
			return;
		}

		num = event.mpCount > 0 ? event.mpCount :
								  (emitter->capacity > 0 ? emitter->capacity : 5) * event.capacityRatio;
		radius = emitter->radius > 0 ? emitter->radius : 75.;

		center = emitter->pos;

		eventIdTag = emitter->idTag;

	} else if (q->m_registry.valid(event.parent)) {
		if (Player *parent = q->m_registry.try_get<Player>(event.parent)) {
			eventIdTag = parent->idTag();
		} else if (Npc *parent = q->m_registry.try_get<Npc>(event.parent)) {
			eventIdTag = parent->idTag;
		} else {
			ELOG_ERROR << "Invalid MP parent";
			return;
		}

		num = event.mpCount;
		radius = 125.;
		center = event.pos;

		grid = q->m_registry.ctx().find<ChunkGrid>();			// Ellenőrizni kell, hogy ne oda tegyük, ahol nem lehet elérni

	} else {
		ELOG_ERROR << "Missing emitter";
		return;
	}



	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	const float startRadian = dist(q->m_rnd) * M_PI;
	const float step = 2.*M_PI / num;

	int generated = 0;

	for (int i=0; generated<num && i<1000; ++i) {
		const float rad = startRadian + i*step;

		float r = radius * (0.5 + dist(q->m_rnd)*0.5);

		cpVect pos = cpvadd(center, cpvmult(cpvforangle(rad), r));

		if (grid && !grid->isAccessible(pos.x, pos.y)) {
			continue;
		}

		auto entity = q->m_registry.create();

		Mp &mp = q->m_registry.emplace<Mp>(entity);
		mp.idTag = nextIdTag(IdMp);
		mp.emitter = emitter ? event.emitter : entt::null;
		mp.pos = pos;
		mp.origin = center;

		if (emitter)
			emitter->mpList.push_back(entity);

		q->entitySetIdTag(entity, mp.idTag);

		++generated;

		ELOG_DEBUG << "Add MP" << mp.idTag << "at" << mp.pos.x << mp.pos.y ;
	}

	if (generated < num) {
		ELOG_ERROR << "Couldn't generate enough MP" << num << "->" << generated;
	}

	// Register event

	if (eventIdTag > 0) {
		RpgStream::EventMpEmitter ev;
		ev.setTagId(eventIdTag);

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

	const ChunkGrid &grid = q->m_registry.ctx().get<ChunkGrid>();

	for (const RpgStream::Tower &e : list) {
		auto entity = q->m_registry.create();

		ELOG_DEBUG << "Load tower" << e.tagId();

		Tower tower = Tower::fromRpgStream(e);

		q->entitySetIdTag(entity, tower.idTag);


		loadDefender(entity, tower, e.defenders());

		QPair<quint32, quint32> chunk = grid.getChunk(e.posXAsFloat(), e.posYAsFloat());

		tower.adjacentChunks.emplace_back(chunk.first, chunk.second);

		for (int x=std::max(0, (int)(chunk.first)-2); x < std::min((qint32) chunk.first+3, grid.gridWidth); ++x) {
			for (int y=std::max(0, (int)(chunk.second)-2); y < std::min((qint32) chunk.second+3, grid.gridHeight); ++y) {
				Chunk ch;
				ch.x = x;
				ch.y = y;

				if (grid.isAccessible(ch))
					tower.adjacentChunks.emplace_back(std::move(ch));
			}
		}



		q->m_registry.emplace<Tower>(entity, std::move(tower));

		TowerStateOutput &out = q->m_registry.emplace<TowerStateOutput>(entity);

		RpgStream::TowerState state;

		state.setTick(q->lastAuthTick());
		state.setTeam(RpgStream::TeamNone);
		state.setLoad(0);
		state.setLockedUntil(0);
		state.setActive(false);
		//state.setHasDefender(false);

		out.append(std::move(state));
	}
}



/**
 * @brief RpgLogicPrivate::loadDefender
 * @param towerEntity
 * @param list
 */

void RpgLogicPrivate::loadDefender(entt::entity towerEntity, Tower &tower, const std::vector<RpgStream::Defender> &list)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(towerEntity)) {
		ELOG_ERROR << "Invalid entity";
		return;
	}


	for (const RpgStream::Defender &e : list) {
		auto entity = q->m_registry.create();

		Defender defender = Defender::fromRpgStream(e);

		defender.tower = towerEntity;
		tower.defenderList.push_back(entity);

		ELOG_DEBUG << "Load defender" << defender.idTag << "for tower" << tower.idTag;

		q->entitySetIdTag(entity, defender.idTag);

		q->m_registry.emplace<Defender>(entity, std::move(defender));
	}
}





/**
 * @brief RpgLogicPrivate::towerUpdate
 * @param list
 */

void RpgLogicPrivate::towerUpdate(const std::vector<RpgStream::Tower> &list)
{
	QMutexLocker locker(&q->m_mutex);

	std::unordered_map<quint32, entt::entity> entities;
	entities.reserve(list.size());

	for (auto e : q->m_registry.view<Tower>()) {
		if (!q->m_registry.valid(e))
			continue;

		entities[q->m_registry.get<Tower>(e).idTag] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());

	std::vector<RpgStream::Tower> createList;

	for (const RpgStream::Tower &p : list) {
		ids.insert(p.tagId());

		auto it = entities.find(p.tagId());

		if (it == entities.end()) {
			createList.push_back(p);
		} else {
			q->m_registry.patch<Tower>(it->second, [&p](Tower &t) {
				t.active = p.active();
			});
		}
	}

	if (!createList.empty())
		loadTower(createList);

	/*for (const auto &[id, e] : entities) {
		if (ids.contains(id))
			continue;

		q->m_registry.emplace<DeleteTag>(e);
	}*/
}




/**
 * @brief RpgLogicPrivate::defenderUpdate
 * @param list
 */


void RpgLogicPrivate::defenderUpdate(const std::vector<RpgStream::BaseDefenderObject> &list)
{
	QMutexLocker locker(&q->m_mutex);

	std::unordered_map<quint32, entt::entity> entities;
	entities.reserve(list.size());

	for (auto e : q->m_registry.view<DefenderObject>()) {
		if (!q->m_registry.valid(e))
			continue;

		entities[q->m_registry.get<DefenderObject>(e).idTag] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());

	for (const RpgStream::BaseDefenderObject &p : list) {
		ids.insert(p.tagId());

		auto it = entities.find(p.tagId());

		if (it == entities.end()) {
			auto entity = q->m_registry.create();

			DefenderObject defender = DefenderObject::fromRpgStream(p);

			q->entitySetIdTag(entity, defender.idTag);

			DefenderStateOutput &out = q->m_registry.emplace<DefenderStateOutput>(entity);

			RpgStream::DefenderState state;

			state.setTick(q->lastAuthTick());
			state.setType(defender.type);
			state.setHp(p.maxHp());

			defenderUpdate(entity, p, &defender, &state);

			out.append(std::move(state));

			q->m_registry.emplace<DefenderObject>(entity, std::move(defender));
		}
	}

	for (const auto &[id, e] : entities) {
		if (ids.contains(id))
			continue;

		q->m_registry.emplace<DeleteTag>(e);
	}
}



/**
 * @brief RpgLogicPrivate::defenderUpdate
 * @param entity
 * @param stream
 */

void RpgLogicPrivate::defenderUpdate(entt::entity entity, const RpgStream::BaseDefenderObject &stream,
									 DefenderObject *object, RpgStream::DefenderState *state)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(entity)) {
		ELOG_ERROR << "Invalid entity";
		return;
	}

	RpgStream::BaseDefenderObject::Type t = object ? object->type : stream.type();

	/*if (t == RpgStream::BaseDefenderObject::Dummy) {
		DefenderDummyObject &dummy = q->m_registry.emplace<DefenderDummyObject>(entity);
		dummy.dummy = stream.dummy();

		if (state) {
			state->setDummy(125);
		}
	}*/


	if (state && object && q->m_registry.valid(object->defender))
		state->setVisible(true);


	const CfgDefenderBase *cfg = nullptr;

	switch (t) {
		case RpgStream::BaseDefenderObject::Fog:
			cfg = &cfgDefenderFog;
			break;

		case RpgStream::BaseDefenderObject::Multiplier1:
			cfg = &cfgDefenderMultiplier;
			break;

		case RpgStream::BaseDefenderObject::Pulse:
			cfg = &cfgDefenderPulse.base;
			break;

		case RpgStream::BaseDefenderObject::None:
			ELOG_ERROR << "Invalid defender type" << t;
			break;
	}

	if (cfg) {
		if (object) object->fromDefenderConfigBase(*cfg);
		if (state) {
			state->setHp(cfg->maxHp);
			if (cfg->alwaysVisible)
				state->setVisible(true);
		}
	}
}




/**
 * @brief RpgLogicPrivate::defenderRender
 * @param defender
 * @param state
 */

entt::entity RpgLogicPrivate::defenderRender(DefenderObject *defender, RpgStream::DefenderState &state)
{
	Q_ASSERT(defender);

	static const std::unordered_set<RpgStream::BaseDefenderObject::Type> skip = {
		RpgStream::BaseDefenderObject::Fog,
		RpgStream::BaseDefenderObject::Multiplier1,
	};

	if (skip.contains(defender->type))
		return entt::null;


	QMutexLocker locker(&q->m_mutex);

	entt::entity tg = defenderNextTarget(*defender);

	if (tg == entt::null)
		return entt::null;

	if (defender->type == RpgStream::BaseDefenderObject::Pulse) {
		defenderRenderPulse(defender, tg);
	}

	return tg;
}



/**
 * @brief RpgLogicPrivate::defenderRenderPulse
 * @param defender
 * @param target
 * @return
 */

void RpgLogicPrivate::defenderRenderPulse(DefenderObject *defender, entt::entity target)
{
	Q_ASSERT(defender);

	QMutexLocker locker(&q->m_mutex);

	RpgStream::EntityState dState;
	dState.setPosXAsFloat(defender->pos.x);
	dState.setPosYAsFloat(defender->pos.y);


	const RpgStream::EntityConfig dConfig = cfgDefenderPulse.toEntityConfig();

	cpVect knockback = cpvzero;

	Player *p = q->m_registry.try_get<Player>(target);
	PlayerPrivate *pCfg = q->m_registry.try_get<PlayerPrivate>(target);
	Npc *n = q->m_registry.try_get<Npc>(target);



	if (p && pCfg) {
		RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(target);
		knockback = RpgLogic::addKnockbackImpulse(&st.entityState(), dState, dConfig, pCfg->toEntityConfig());
	} else if (n) {
		RpgStream::NpcState &st = getEditableCurrentState<RpgStream::NpcState>(target);
		knockback = RpgLogic::addKnockbackImpulse(&st.entityState(), dState, dConfig, n->data.entity());
	}

	if (!cpveql(knockback, cpvzero))
		q->m_registry.emplace_or_replace<KnockbackTag>(target, q->lastAuthTick());
}



/**
 * @brief RpgLogicPrivate::checkAttackerInFog
 * @return
 */

bool RpgLogicPrivate::checkInFog(entt::entity entity, const bool &isAttacker) const
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(entity))
		return false;


	Player *p = q->m_registry.try_get<Player>(entity);
	Npc *n = q->m_registry.try_get<Npc>(entity);


	if (p) {
		const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(entity);

		if (!st)
			return false;

		return checkInFog(cpv(st->entityState().posXAsFloat(), st->entityState().posYAsFloat()),
						  isAttacker ? RpgLogic::oppositeTeam(p->team) : p->team);

	} else if (n) {
		const RpgStream::NpcState *st = q->getCurrentState<RpgStream::NpcState>(entity);

		if (!st)
			return false;

		if (n->data.team() == RpgStream::TeamNone && !isAttacker)
			return false;

		return checkInFog(cpv(st->entityState().posXAsFloat(), st->entityState().posYAsFloat()),
						  isAttacker ? RpgLogic::oppositeTeam(n->data.team()) : n->data.team());
	}

	return false;
}





/**
 * @brief RpgLogicPrivate::checkInFog
 * @param pos
 * @param team
 * @return
 */

bool RpgLogicPrivate::checkInFog(const cpVect &pos, const RpgStream::Team &team) const
{
	QMutexLocker locker(&q->m_mutex);

	for (auto e : q->m_registry.view<DefenderObject>()) {
		const DefenderObject &def = q->m_registry.get<DefenderObject>(e);

		if (def.type != RpgStream::BaseDefenderObject::Fog)
			continue;

		if (team != RpgStream::TeamNone && def.team != team)
			continue;

		if (def.isNear(pos))
			return true;
	}

	return false;
}



/**
 * @brief RpgLogicPrivate::defenderNextTarget
 * @param defender
 * @return
 */

entt::entity RpgLogicPrivate::defenderNextTarget(DefenderObject &defender) const
{
	QMutexLocker locker(&q->m_mutex);

	if (defender.nearTargets.empty())
		return entt::null;

	if (defender.nearTargets.size() == 1)
		return *defender.nearTargets.cbegin();

	for (entt::entity e : defender.nearTargets) {
		if (!defender.lastTargets.contains(e))
			return e;
	}

	defender.lastTargets.clear();

	return *defender.nearTargets.cbegin();
}










/**
 * @brief RpgLogicPrivate::generateNpc
 * @param player
 * @param priv
 * @param data
 * @param pos
 * @return
 */

entt::entity RpgLogicPrivate::generateNpc(Player *player, PlayerPrivate *priv, const RpgStream::NpcData &data,
										  const cpVect &pos, quint32 *idTagPtr)
{
	QMutexLocker locker(&q->m_mutex);

	auto entity = q->m_registry.create();

	m_requireFull = true;

	Npc &d = q->m_registry.emplace<Npc>(entity);

	if (player && priv)
		d.idTag = nextIdTag(player, priv);
	else
		d.idTag = nextIdTag(IdNpc);


	d.data = data;
	d.data.setTagId(d.idTag);

	//m_registry.emplace<PlayerPrivate>(entity).load(data.config());
	q->m_registry.emplace<NpcStateInput>(entity);
	q->m_registry.emplace<NpcStateOutput>(entity);


	if (!cpveql(pos, cpvzero)) {
		q->m_registry.emplace<cpVect>(entity) = pos;
	}

	npcUpdate(entity, RpgStream::NpcData{}, &d, nullptr);


	q->entitySetIdTag(entity, d.idTag);

	if (idTagPtr)
		*idTagPtr = d.idTag;

	ELOG_DEBUG << "Add NPC" << d.idTag << "type" << d.data.type() << "team" << d.data.team() << "at" << q->lastAuthTick();

	return entity;
}



/**
 * @brief RpgLogicPrivate::npcInitialize
 * @param ent
 * @return
 */

bool RpgLogicPrivate::npcInitialize(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent)) {
		ELOG_ERROR << "Invalid NPC entity";
		return false;
	}


	Npc *npc = q->m_registry.try_get<Npc>(ent);

	if (!npc) {
		ELOG_ERROR << "Invalid NPC";
		return false;
	}

	/// State

	RpgStream::NpcState &state = getEditableCurrentState<RpgStream::NpcState>(ent);

	state.setTick(q->lastAuthTick());
	state.setType(npc->data.type());
	state.setHp(npc->data.entity().maxHp());
	state.entityState().setVelSq(0);

	if (cpVect *pos = q->m_registry.try_get<cpVect>(ent)) {
		state.entityState().setPosXAsFloat(pos->x);
		state.entityState().setPosYAsFloat(pos->y);

		q->m_registry.erase<cpVect>(ent);
	} else {
		ChunkGrid *grid = q->m_registry.ctx().find<ChunkGrid>();
		Q_ASSERT(grid);

		const auto p = grid->getRandomChunk(q->m_rnd);

		if (!p) {
			ELOG_ERROR << "Random chunk generate error";
		} else {
			cpVect ppos = grid->chunkCenter(p.value());

			state.entityState().setPosXAsFloat(ppos.x);
			state.entityState().setPosYAsFloat(ppos.y);
		}
	}


	npcUpdate(ent, *npc, state);

	return true;
}





/**
 * @brief RpgLogicPrivate::npcUpdate
 * @param list
 */

void RpgLogicPrivate::npcUpdate(const std::vector<RpgStream::NpcData> &list)
{
	QMutexLocker locker(&q->m_mutex);

	std::unordered_map<quint32, entt::entity> entities;
	entities.reserve(list.size());

	for (auto e : q->m_registry.view<Npc>()) {
		if (!q->m_registry.valid(e))
			continue;

		entities[q->m_registry.get<Npc>(e).idTag] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());


	for (const RpgStream::NpcData &p : list) {
		ids.insert(p.tagId());

		auto it = entities.find(p.tagId());

		if (it != entities.end()) {
			q->m_registry.patch<Npc>(it->second, [&p](Npc &t) {
				t.data.entity().setMaxHp(p.entity().maxHp());
				t.data.setTeam(p.team());
			});
		} else {
			auto entity = q->m_registry.create();

			Npc npc;
			npc.idTag = p.tagId();
			npc.data = p;

			q->entitySetIdTag(entity, npc.idTag);

			NpcStateOutput &out = q->m_registry.emplace<NpcStateOutput>(entity);

			RpgStream::NpcState state;

			state.setType(p.type());
			state.setTick(q->lastAuthTick());
			state.setHp(p.entity().maxHp());

			npcUpdate(entity, p, &npc, &state);

			out.append(std::move(state));

			q->m_registry.emplace<Npc>(entity, std::move(npc));
		}
	}

	for (const auto &[id, e] : entities) {
		if (ids.contains(id))
			continue;

		q->m_registry.emplace<DeleteTag>(e);
	}
}





/**
 * @brief RpgLogicPrivate::npcUpdate
 * @param entity
 * @param data
 * @param object
 * @param state
 */

void RpgLogicPrivate::npcUpdate(entt::entity entity, const RpgStream::NpcData &stream, Npc *object, RpgStream::NpcState *state)
{
	Q_UNUSED(entity);
	Q_UNUSED(stream);
	Q_UNUSED(object);
	Q_UNUSED(state);

	/*QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(entity)) {
		ELOG_ERROR << "Invalid entity";
		return;
	}

	RpgStream::NpcData::Type t = object ? object->data.type() : stream.type();

	switch (t) {
		case RpgStream::NpcData::TowerAttacker:
			q->m_registry.emplace<NpcTowerAttacker>(entity).force = std::min(1u, stream.force());
			break;

		case RpgStream::NpcData::MpLeecher:
			q->m_registry.emplace<NpcMpLeecher>(entity).force = std::min(1u, stream.force());
			break;

		case RpgStream::NpcData::None:
			ELOG_DEBUG << "Invalid NPC type" << t;
			break;
	}*/

}



/**
 * @brief RpgLogicPrivate::npcUpdate
 * @param entity
 * @param object
 * @param state
 */

void RpgLogicPrivate::npcUpdate(entt::entity entity, const Npc &object, RpgStream::NpcState &state)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(entity)) {
		ELOG_ERROR << "Invalid entity";
		return;
	}

	if (object.data.type() == RpgStream::NpcData::TowerAttacker) {
		state.setDestinationTower(0);
		state.setDestinationX(0);
		state.setDestinationY(0);
	} else if (object.data.type() == RpgStream::NpcData::MpLeecher) {
		state.setDestinationX(0);
		state.setDestinationY(0);
	}
}



/**
 * @brief RpgLogicPrivate::npcInputLoad
 * @param list
 * @param acceptedInputList
 * @param delta
 */

void RpgLogicPrivate::npcInputLoad(const std::vector<RpgStream::NpcStateList> &list, EventWindowHash *acceptedInputList, const bool &delta)
{
	for (const RpgStream::NpcStateList &ps : list) {
		if (!acceptedInputList || acceptedInputList->contains(ps.tagId()))
			npcInputLoad(ps.tagId(), delta ? ps.extractStateVector() : ps.state());
		else
			ELOG_ERROR << "Unacceptable input" << ps.tagId();
	}
}



/**
 * @brief RpgLogicPrivate::npcInputLoad
 * @param ent
 * @param list
 */

void RpgLogicPrivate::npcInputLoad(entt::entity ent, const std::vector<RpgStream::NpcState> &list)
{
	QMutexLocker locker(&q->m_mutex);

	Npc *p = q->m_registry.try_get<Npc>(ent);
	NpcStateInput *input = q->m_registry.try_get<NpcStateInput>(ent);

	if (!p || !input) {
		ELOG_ERROR << "Invalid NPC";
		return;
	}

	input->load(list, q->lastAuthTick(), q->m_serverTick + MAX_FUTURE_TICK);
}




/**
 * @brief RpgLogicPrivate::npcRenderInput
 * @param npc
 * @param input
 * @param dest
 */

void RpgLogicPrivate::npcRenderInput(const Npc &npc, const RpgStream::NpcState &input, RpgStream::NpcState &dest)
{
	switch (npc.data.type()) {
		case RpgStream::NpcData::TowerAttacker:
			dest.setTarget(input.target());
			dest.setDestinationTower(input.destinationTower());
			dest.setDestinationX(input.destinationX());
			dest.setDestinationY(input.destinationY());
			break;

		case RpgStream::NpcData::MpLeecher:
			dest.setTarget(input.target());
			dest.setDestinationX(input.destinationX());
			dest.setDestinationY(input.destinationY());
			break;

		case RpgStream::NpcData::None:
			break;
	}
}



/**
 * @brief RpgLogicPrivate::npcFullState
 * @param npc
 * @param state
 * @return
 */

QString RpgLogicPrivate::npcFullState(const Npc &npc, const RpgStream::NpcState &state) const
{
	switch (npc.data.type()) {
		case RpgStream::NpcData::TowerAttacker:
			return QStringLiteral("%1 (%2, %3)")
					.arg(state.destinationTower())
					.arg(state.destinationXAsFloat())
					.arg(state.destinationYAsFloat())
					;
			break;

		case RpgStream::NpcData::MpLeecher:
			return QStringLiteral("(%1, %2)")
					.arg(state.destinationXAsFloat())
					.arg(state.destinationYAsFloat())
					;
			break;

		case RpgStream::NpcData::None:
			return QStringLiteral("INVALID");
	}

	return QString();
}




/**
 * @brief RpgLogicPrivate::npcDecreaseHp
 * @param npc
 * @param count
 * @return
 */

quint32 RpgLogicPrivate::npcDecreaseHp(entt::entity npc, const quint32 &count)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(npc)) {
		ELOG_ERROR << "Invalid NPC entity";
		return false;
	}

	RpgStream::NpcState &st = getEditableCurrentState<RpgStream::NpcState>(npc);

	if (st.hp() == 0) {
		ELOG_WARNING << "Npc isn't alive";
		return 0;
	}

	quint32 nextHp = st.hp() - std::min(count, st.hp());

	st.setHp(nextHp);

	return nextHp;
}



/**
 * @brief RpgLogicPrivate::utility
 * @param player
 * @param priv
 * @param type
 * @param target
 * @return
 */

bool RpgLogicPrivate::utility(Player *player, PlayerPrivate *priv, const RpgStream::PlayerConfig::Utility &type, entt::entity target)
{
	Q_ASSERT(player);
	Q_ASSERT(priv);

	QMutexLocker locker(&q->m_mutex);

	bool success = false;

	Utility d;

	d.type = type;
	d.team = player->team;
	d.target = target;
	d.destroyAt = q->lastAuthTick() + 1;

	switch (type) {
		case RpgStream::PlayerConfig::UtilityMissionary:
			success = utilityMissionary(d, target);
			break;

		case RpgStream::PlayerConfig::UtilitySniper:
			success = utilitySniper(d, target);
			break;

		case RpgStream::PlayerConfig::UtilityNone:
			ELOG_WARNING << "Invalid utility";
			break;
	}

	if (!success)
		return false;

	return true;
}







/**
 * @brief RpgLogicPrivate::utilityMissionary
 * @param entity
 * @param utility
 */

bool RpgLogicPrivate::utilityMissionary(Utility &utility, entt::entity target)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(target)) {
		ELOG_WARNING << "Invalid target";
		return false;
	}

	Npc *npc = q->m_registry.try_get<Npc>(utility.target);

	if (!npc) {
		ELOG_WARNING << "Invalid NPC";
		return false;
	}

	if (npc->data.team() != RpgStream::TeamNone) {
		ELOG_WARNING << "NPC" << npc->idTag << "already in team" << npc->data.team();
		return false;
	}

	m_requireFull = true;

	npc->data.setTeam(utility.team);

	ELOG_INFO << "NPC" << npc->idTag << "replace team to" << npc->data.team() << "at" << q->lastAuthTick();


	/*
	auto entity = q->m_registry.create();

	q->m_registry.emplace<Utility>(entity, std::move(utility));*/

	return true;
}



/**
 * @brief RpgLogicPrivate::utilitySniper
 * @param utility
 * @param target
 * @return
 */

bool RpgLogicPrivate::utilitySniper(Utility &utility, entt::entity target)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(target)) {
		ELOG_WARNING << "Invalid target";
		return false;
	}

	Npc *npc = q->m_registry.try_get<Npc>(utility.target);
	DefenderObject *defender = q->m_registry.try_get<DefenderObject>(utility.target);

	if (!npc && !defender) {
		ELOG_WARNING << "Invalid NPC or defender";
		return false;
	}

	if (npc && npc->data.team() == utility.team) {
		ELOG_WARNING << "NPC" << npc->idTag << "team matches" << npc->data.team();
		return false;
	}

	if (defender && defender->team == utility.team) {
		ELOG_WARNING << "Defender" << defender->idTag << "team matches" << defender->team;
		return false;
	}

	if (npc) {
		RpgStream::NpcState &state = getEditableCurrentState<RpgStream::NpcState>(target);
		state.setHp(0);

		ELOG_INFO << "NPC" << npc->idTag << "killed at" << q->lastAuthTick();
	}

	if (defender) {
		RpgStream::DefenderState &state = getEditableCurrentState<RpgStream::DefenderState>(target);

		state.setHp(0);

		ELOG_INFO << "Defender" << defender->idTag << "killed at" << q->lastAuthTick();
	}

	return true;
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

	m_requireFull = true;

	DefenderObject &d = q->m_registry.emplace<DefenderObject>(entity);
	d.idTag = nextIdTag(player, priv);


	q->m_registry.emplace<DefenderStateOutput>(entity);

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

	RpgStream::DefenderState &state = getEditableCurrentState<RpgStream::DefenderState>(entity);

	state.setTick(q->lastAuthTick());
	state.setTagId(d.idTag);
	state.setType(d.type);

	defenderUpdate(entity, RpgStream::BaseDefenderObject{}, &d, &state);

	//out.append(std::move(state));

	q->entitySetIdTag(entity, d.idTag);

	ELOG_DEBUG << "Add defender" << d.idTag << "type" << d.type << "HP:" << state.hp() << "/" << d.maxHp;

	return entity;
}




/**
 * @brief RpgLogic::eventLoad
 * @param list
 */

void RpgLogicPrivate::eventInputLoad(const std::vector<RpgStream::Events> &list, EventWindowHash *acceptedInputList)
{
	const quint32 &minTick = q->lastAuthTick();
	const quint32 &maxTick = q->m_serverTick + MAX_FUTURE_TICK;

	for (const RpgStream::Events &e : list) {
		if (e.tick() < minTick || e.tick() > maxTick) {
			ELOG_WARNING << "Unacceptable input at" << e.tick() << "valid range:" << minTick << "-" << maxTick;
		} else {
			eventInputLoad(e.player(), acceptedInputList);
			eventInputLoad(e.npc(), acceptedInputList);
		}
	}

}


/**
 * @brief RpgLogic::eventPlayerInputLoad
 * @param list
 */

void RpgLogicPrivate::eventInputLoad(const std::vector<RpgStream::EventPlayer> &list, EventWindowHash *acceptedInputList)
{
	if (list.empty())
		return;

	for (RpgStream::EventPlayer l : list) {
		if (acceptedInputList && !acceptedInputList->accept(l.tagId(), l)) {
			ELOG_TRACE << "Unacceptable event" << l.tagId() << l.type() << l.tick() << l.seq();
		} else {
			ELOG_TRACE << "Accept event" << l.tagId() << l.type() << l.tick() << l.seq();
			q->eventStore(std::move(l));
		}
	}
}




/**
 * @brief RpgLogicPrivate::eventInputLoad
 * @param list
 * @param acceptedInputList
 */

void RpgLogicPrivate::eventInputLoad(const std::vector<RpgStream::EventNpc> &list, EventWindowHash *acceptedInputList)
{
	if (list.empty())
		return;

	for (RpgStream::EventNpc l : list) {
		if (acceptedInputList && !acceptedInputList->accept(l.tagId(), l)) {
			ELOG_TRACE << "Unacceptable event" << l.tagId() << l.type() << l.tick() << l.seq();
		} else {
			ELOG_TRACE << "Accept event" << l.tagId() << l.type() << l.tick() << l.seq();
			q->eventStore(std::move(l));
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
		ELOG_ERROR << "Invalid entity";
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

	if (RpgStream::EventNpc *e = q->m_registry.try_get<RpgStream::EventNpc>(entity)) {
		dst->flags().setFlag(RpgStream::Events::Npc);
		dst->npc().push_back(*e);
		return true;
	}

	if (RpgStream::EventControl *e = q->m_registry.try_get<RpgStream::EventControl>(entity)) {
		dst->flags().setFlag(RpgStream::Events::Control);
		dst->control().push_back(*e);
		return true;
	}

	if (RpgStream::EventDefender *e = q->m_registry.try_get<RpgStream::EventDefender>(entity)) {
		dst->flags().setFlag(RpgStream::Events::Defender);
		dst->defender().push_back(*e);
		return true;
	}

	if (RpgStream::EventStageChanged *e = q->m_registry.try_get<RpgStream::EventStageChanged>(entity)) {
		dst->flags().setFlag(RpgStream::Events::Stage);
		dst->stage().push_back(*e);
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

		case RpgStream::EventPlayer::EventChangeUtility:
			preRenderEventChangeUtility(e);
			break;


		case RpgStream::EventPlayer::EventReplaceDefender:
			preRenderEventReplaceDefender(e);
			break;

		case RpgStream::EventPlayer::EventReplaceUtility:
			preRenderEventReplaceUtility(e);
			break;


		case RpgStream::EventPlayer::EventUseUtility:
			preRenderEventUseUtility(e);
			break;

		case RpgStream::EventPlayer::EventUseControl:
			preRenderEventUseControl(e);
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
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity mp = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	if (!q->m_registry.valid(mp)) {
		ELOG_WARNING << "Mp entity not found" << event.target();
		return;
	}

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);
	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);

	if (st->hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << st->tick() << st->hp();
		return;
	}

	if (st->mp() >= pp.maxMp) {
		ELOG_WARNING << "Player reached max mp" << st->tick() << st->mp();
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
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity tower = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	if (!q->m_registry.valid(tower)) {
		ELOG_WARNING << "Tower entity not found" << event.target();
		return;
	}

	if (!q->m_registry.get<Tower>(tower).active) {
		ELOG_WARNING << "Tower inactive" << event.target();
		return;
	}

	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);


	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			ELOG_WARNING << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			ELOG_WARNING << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const quint32 tick = q->lastAuthTick()+1;
	const Player &pp = q->m_registry.get<Player>(player);
	const PlayerPrivate &ppp = q->m_registry.get<PlayerPrivate>(player);

	if (PenaltyTag *pen = q->m_registry.try_get<PenaltyTag>(player);
			pen && pen->expire > tick) {
		ELOG_ERROR << "Player has penalty" << pp.playerData.playerId();
		return;
	}

	//const Tower &tt = q->m_registry.get<Tower>(tower);
	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);
	const RpgStream::TowerState *t = q->getCurrentState<RpgStream::TowerState>(tower);

	if (st->hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << st->tick() << st->hp();
		return;
	}


	if (t->lockedUntil() > tick) {
		ELOG_WARNING << "Tower can't be attacked" << st->tick() << event.target() << t->lockedUntil();
		return;
	}

	if (t->hasDefender() && t->team() != pp.team) {
		ELOG_WARNING << "Tower has defender" << st->tick() << event.target();
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
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity defender = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}


	// Player state

	const Player &pp = q->m_registry.get<Player>(player);

	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);

	if (st->hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << st->tick() << st->hp();
		return;
	}

	if (st->defender() == RpgStream::BaseDefenderObject::None || !st->hasDefender()) {
		ELOG_WARNING << "Player hasn't defender" << st->tick();
		return;
	}




	if (!q->m_registry.valid(defender)) {										// Emplace on chunk
		if (!RpgStream::BaseDefenderObject::placementFlags(st->defender()).testFlag(RpgStream::BaseDefenderObject::PlacementChunk)) {
			ELOG_WARNING << "Defender type" << st->defender() << "can't be placed onto chunk";
			return;
		}

		const Chunk &chunk = Chunk::fromRpgStream(event.chunk());

		if (!q->isChunkEmpty(chunk)) {
			ELOG_WARNING << "Chunk is not accessible" << st->tick() << chunk.x << chunk.y;
			return;
		}


		EventDefenderAdd final;
		final.player = player;
		final.chunk = chunk;
		final.type = st->defender();

		eventFinalStore(std::move(final));


	} else {
		if (!RpgStream::BaseDefenderObject::placementFlags(st->defender()).testFlag(RpgStream::BaseDefenderObject::PlacementTower)) {
			ELOG_WARNING << "Defender type" << st->defender() << "can't be placed onto tower";
			return;
		}

		const Defender &dd = q->m_registry.get<Defender>(defender);

		if (!q->m_registry.valid(dd.tower)) {
			ELOG_WARNING << "Tower entity not found" << event.target();
			return;
		}

		if (q->m_registry.valid(dd.object)) {
			ELOG_WARNING << "The defender already contains an object" << event.target();
			return;
		}

		//const Tower &tt = q->m_registry.get<Tower>(dd.tower);
		const RpgStream::TowerState *t = q->getCurrentState<RpgStream::TowerState>(dd.tower);

		if (!t->active()) {
			ELOG_WARNING << "Tower isn't active" << st->tick();
			return;
		}

		if (t->team() != pp.team) {
			ELOG_WARNING << "Tower doesn't belong to player's team" << st->tick();
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
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	const Player &pp = q->m_registry.get<Player>(player);
	Player *tt = q->m_registry.try_get<Player>(target);
	Npc *nn = q->m_registry.try_get<Npc>(target);

	if (!tt && !nn) {
		ELOG_WARNING << "Target entity not found" << event.target();
		return;
	}


	RpgStream::PlayerState &stPlayer = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (stPlayer.hp() == 0) {
		ELOG_WARNING << "Player isn't alive" << stPlayer.tick() << stPlayer.hp();
		return;
	}

	if (stPlayer.bullet() == 0) {
		ELOG_WARNING << "Player hasn't enough bullet" << stPlayer.tick() << stPlayer.bullet();
		return;
	}


	const bool inFog = checkInFog(cpv(stPlayer.entityState().posXAsFloat(),
									  stPlayer.entityState().posYAsFloat()),
								  RpgLogic::oppositeTeam(pp.team));

	if (tt) {
		RpgStream::PlayerState &stTarget = getEditableCurrentState<RpgStream::PlayerState>(target);

		if (stTarget.hp() == 0) {
			ELOG_WARNING << "Target isn't alive" << stPlayer.tick() << stTarget.hp();
			return;
		}


		EventAttackPlayer final;
		final.player = player;
		final.target = target;
		final.withHurt = stTarget.lock() == 0;
		final.withSuccess = !inFog && !checkInFog(cpv(stTarget.entityState().posXAsFloat(),
													  stTarget.entityState().posYAsFloat()),
												  tt->team);

		eventFinalStore(std::move(final));

	} else {
		RpgStream::NpcState &stTarget = getEditableCurrentState<RpgStream::NpcState>(target);

		if (stTarget.hp() == 0) {
			ELOG_WARNING << "Target isn't alive" << stPlayer.tick() << stTarget.hp();
			return;
		}


		EventAttackNpc final;
		final.player = player;
		final.target = target;

		if (nn->data.team() == RpgStream::TeamNone) {
			final.withSuccess = !inFog;
		} else {
			final.withSuccess = !inFog && !checkInFog(cpv(stTarget.entityState().posXAsFloat(),
														  stTarget.entityState().posYAsFloat()),
													  nn->data.team());
		}

		eventFinalStore(std::move(final));
	}
}





/**
 * @brief RpgLogicPrivate::preRenderEventAttackDefender
 * @param event
 */

void RpgLogicPrivate::preRenderEventAttackDefender(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventAttackDefender) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	Player *pp = q->m_registry.try_get<Player>(player);
	DefenderObject *tt = q->m_registry.try_get<DefenderObject>(target);

	if (!pp) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	if (!tt) {
		ELOG_WARNING << "DefenderObject entity not found" << event.target();
		return;
	}

	if (pp->team == tt->team) {
		ELOG_WARNING << "DefenderObject's team matches" << event.target();
		return;
	}

	const PlayerPrivate &ppp = q->m_registry.get<PlayerPrivate>(player);

	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);


	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			ELOG_WARNING << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			ELOG_WARNING << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const quint32 tick = q->lastAuthTick()+1;

	if (PenaltyTag *pen = q->m_registry.try_get<PenaltyTag>(player);
			pen && pen->expire > tick) {
		ELOG_ERROR << "Player has penalty" << pp->playerData.playerId();
		return;
	}




	RpgStream::PlayerState &stPlayer = getEditableCurrentState<RpgStream::PlayerState>(player);
	RpgStream::DefenderState &stTarget = getEditableCurrentState<RpgStream::DefenderState>(target);

	if (stPlayer.hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << stPlayer.tick() << stPlayer.hp();
		return;
	}

	if (stTarget.hp() <= 0) {
		ELOG_WARNING << "Target isn't alive" << stPlayer.tick() << stTarget.hp();
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
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	quint32 penalty = 0;

	PlayerPrivate *pp = q->m_registry.try_get<PlayerPrivate>(player);

	Q_ASSERT(pp);

	if (entt::entity tg = q->entityFromIdTag(event.target()); q->m_registry.valid(tg)) {
		if (q->m_registry.try_get<Tower>(tg)) {
			penalty = pp->penaltyMsec;
		} else if (q->m_registry.try_get<Chest>(tg)) {
			penalty = pp->penaltyMsec;
		}
	}

	ELOG_DEBUG << "Player" << event.tagId() << "unlock with penalty" << penalty << "at" << q->lastAuthTick();

	playerUnlock(player, penalty);
}




/**
 * @brief RpgLogicPrivate::preRenderEventChangeBullet
 * @param event
 */

void RpgLogicPrivate::preRenderEventChangeBullet(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventChangeBullet) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}


	QMutexLocker locker(&q->m_mutex);


	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}


	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			ELOG_WARNING << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			ELOG_WARNING << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}


	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);
	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (st.hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << st.tick() << st.hp();
		return;
	}

	if (st.mp() < CFG_MP_CHANGE_BULLET) {
		ELOG_WARNING << "Player hasn't enough MP" << st.tick() << st.mp();
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
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}


	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			ELOG_WARNING << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			ELOG_WARNING << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (st.hp() <= 0) {
		ELOG_WARNING << "Player" << event.tagId() << "isn't alive" << st.tick() << st.hp();
		return;
	}

	const RpgStream::BaseDefenderObject::Type dType = st.defender();

	if (dType == RpgStream::BaseDefenderObject::None) {
		ELOG_WARNING << "Player" << event.tagId() << "didn't select defender" << st.tick() << st.defender();
		return;
	}

	if (!pp.defenders.contains(dType)) {
		ELOG_WARNING << "Player" << event.tagId() << "can't have this defender" << st.tick() << st.defender();
		return;
	}

	if (st.hasDefender()) {
		ELOG_WARNING << "Player" << event.tagId() << "already have a defender" << st.tick() << st.defender();
		return;
	}

	if (st.mp() < (quint32) cfgRequiredMpDefender.value(dType)) {
		ELOG_WARNING << "Player" << event.tagId() << "hasn't enough MP" << st.tick() << st.mp();
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
 * @brief RpgLogicPrivate::preRenderEventChangeUtility
 * @param event
 */

void RpgLogicPrivate::preRenderEventChangeUtility(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventChangeUtility) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}


	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);

	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			ELOG_WARNING << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			ELOG_WARNING << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	if (st.hp() <= 0) {
		ELOG_WARNING << "Player" << event.tagId() << "isn't alive" << st.tick() << st.hp();
		return;
	}

	const RpgStream::PlayerConfig::Utility dType = st.utility();

	if (dType == RpgStream::PlayerConfig::UtilityNone) {
		ELOG_WARNING << "Player" << event.tagId() << "didn't select utility" << st.tick() << st.utility();
		return;
	}

	if (!pp.utilities.contains(dType)) {
		ELOG_WARNING << "Player" << event.tagId() << "can't have this utility" << st.tick() << st.utility();
		return;
	}

	if (st.hasUtility()) {
		ELOG_WARNING << "Player" << event.tagId() << "already have an utility" << st.tick() << st.utility();
		return;
	}

	if (st.mp() < (quint32) cfgRequiredMpUtility.value(dType)) {
		ELOG_WARNING << "Player" << event.tagId() << "hasn't enough MP" << st.tick() << st.mp();
		return;
	}


	EventChangeUtility final;
	final.player = player;
	final.type = dType;
	final.lock = (event.lockId() == 0);
	if (final.lock) final.skipLock = pp.canSkipLock();

	eventFinalStore(std::move(final));
}



/**
 * @brief RpgLogicPrivate::preRenderEventReplaceDefender
 * @param event
 */

void RpgLogicPrivate::preRenderEventReplaceDefender(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventReplaceDefender) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	const RpgStream::BaseDefenderObject::Type defender = RpgStream::BaseDefenderObject::Type(event.at());

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	if (!pp.defenders.contains(defender)) {
		ELOG_WARNING << "Player" << event.tagId() << "don't have defender" << defender;
		return;
	}

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	st.setDefender(defender);
}





/**
 * @brief RpgLogicPrivate::preRenderEventReplaceUtility
 * @param event
 */

void RpgLogicPrivate::preRenderEventReplaceUtility(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventReplaceUtility) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	const RpgStream::PlayerConfig::Utility utility = RpgStream::PlayerConfig::Utility(event.at());

	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(player);

	if (!pp.utilities.contains(utility)) {
		ELOG_WARNING << "Player" << event.tagId() << "don't have utility" << utility;
		return;
	}

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	st.setUtility(utility);
}



/**
 * @brief RpgLogicPrivate::preRenderEventUseUtility
 * @param event
 */

void RpgLogicPrivate::preRenderEventUseUtility(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventUseUtility) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}


	// Player state

	//const Player &pp = q->m_registry.get<Player>(player);

	const RpgStream::PlayerState *st = q->getCurrentState<RpgStream::PlayerState>(player);

	if (st->hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << st->tick() << st->hp();
		return;
	}

	if (st->utility() == RpgStream::PlayerConfig::UtilityNone || !st->hasUtility()) {
		ELOG_WARNING << "Player hasn't utility" << st->tick();
		return;
	}


	EventUtility final;
	final.player = player;
	final.target = target;
	final.type = st->utility();

	eventFinalStore(std::move(final));
}




/**
 * @brief RpgLogicPrivate::preRenderEventUseControl
 * @param event
 */

void RpgLogicPrivate::preRenderEventUseControl(const RpgStream::EventPlayer &event)
{
	if (event.type() != RpgStream::EventPlayer::EventUseControl) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.target());

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	Player *pp = q->m_registry.try_get<Player>(player);
	Control *tt = q->m_registry.try_get<Control>(target);

	if (!pp) {
		ELOG_WARNING << "Player entity not found" << event.tagId();
		return;
	}

	if (!tt) {
		ELOG_WARNING << "Control entity not found" << event.target();
		return;
	}

	const PlayerPrivate &ppp = q->m_registry.get<PlayerPrivate>(player);

	const LockTag *pLock = q->m_registry.try_get<LockTag>(player);


	if (event.lockId() == 0) {				// Most jön a kérés a zárolásra
		if (pLock) {
			ELOG_WARNING << "Player already locked" << event.tagId();
			return;
		}
	} else {
		if (pLock && pLock->id != event.lockId()) {
			ELOG_WARNING << "Player lockid mismatch" << event.tagId() << event.lockId() << pLock->id;
			return;
		}
	}

	const quint32 tick = q->lastAuthTick()+1;

	if (PenaltyTag *pen = q->m_registry.try_get<PenaltyTag>(player);
			pen && pen->expire > tick) {
		ELOG_ERROR << "Player has penalty" << pp->playerData.playerId();
		return;
	}


	RpgStream::PlayerState &stPlayer = getEditableCurrentState<RpgStream::PlayerState>(player);
	RpgStream::ControlState &stTarget = getEditableCurrentState<RpgStream::ControlState>(target);

	if (stPlayer.hp() <= 0) {
		ELOG_WARNING << "Player isn't alive" << stPlayer.tick() << stPlayer.hp();
		return;
	}

	// Ha már zárolva van, akkor nem bántjuk

	if (!pLock) {
		if (!stTarget.isAlive()) {
			ELOG_WARNING << "Control isn't alive" << stPlayer.tick() << stTarget.isAlive();
			return;
		}


		if (!preRenderControlCheck(tt, stTarget, pp))
			return;

	}


	EventControl final;
	final.player = player;
	final.control = target;
	final.lock = (event.lockId() == 0);
	if (final.lock) final.skipLock = ppp.canSkipLock();

	eventFinalStore(std::move(final));
}






/**
 * @brief RpgLogicPrivate::preRenderEventNpcCreate
 * @param ent
 */

void RpgLogicPrivate::preRenderEventNpcCreate(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent)) {
		ELOG_ERROR << "Missing entity";
		return;
	}

	const EventNpcCreate &event = q->m_registry.get<EventNpcCreate>(ent);

	Player *player = nullptr;
	PlayerPrivate *priv = nullptr;

	if (q->m_registry.valid(event.owner)) {
		player = q->m_registry.try_get<Player>(event.owner);
		priv = q->m_registry.try_get<PlayerPrivate>(event.owner);
	}

	quint32 idTag = 0;

	auto e = generateNpc(player, priv, event.data, event.pos, &idTag);
	npcInitialize(e);

	q->onNpcCreated(e, idTag, player);

	// Register event

	if (idTag > 0) {
		RpgStream::EventNpc ev;
		ev.setTagId(idTag);
		ev.setType(RpgStream::EventNpc::EventCreated);

		eventRealStore(std::move(ev));
	}
}



/**
 * @brief RpgLogicPrivate::preRenderEventNpc
 * @param event
 */

void RpgLogicPrivate::preRenderEventNpc(entt::entity event)
{
	QMutexLocker locker(&q->m_mutex);

	RpgStream::EventNpc &e = q->m_registry.get<RpgStream::EventNpc>(event);

	switch (e.type()) {
		case RpgStream::EventNpc::EventAttack:
			preRenderEventNpcAttack(e);
			break;

		case RpgStream::EventNpc::EventCreated:
		case RpgStream::EventNpc::EventNone:
			break;
	}
}



/**
 * @brief RpgLogicPrivate::preRenderEventNpcAttack
 * @param event
 */

void RpgLogicPrivate::preRenderEventNpcAttack(const RpgStream::EventNpc &event)
{
	if (event.type() != RpgStream::EventNpc::EventAttack) {
		ELOG_ERROR << "Invalid event" << event.type();
		return;
	}

	QMutexLocker locker(&q->m_mutex);

	entt::entity entity = q->entityFromIdTag(event.tagId());
	entt::entity target = q->entityFromIdTag(event.targetId());

	if (!q->m_registry.valid(entity)) {
		ELOG_WARNING << "Npc entity not found" << event.tagId();
		return;
	}

	if (!q->m_registry.valid(target)) {
		ELOG_WARNING << "Npc target entity not found" << event.targetId();
		return;
	}

	Npc *npc = q->m_registry.try_get<Npc>(entity);

	if (!npc) {
		ELOG_WARNING << "Invalid npc entity" << event.tagId();
		return;
	}

	if (DefenderObject *tt = q->m_registry.try_get<DefenderObject>(target))
		return preRenderEventNpcAttack(entity, target, npc, tt, event);

	if (Tower *tt = q->m_registry.try_get<Tower>(target))
		return preRenderEventNpcAttack(entity, target, npc, tt, event);

	if (Player *tt = q->m_registry.try_get<Player>(target))
		return preRenderEventNpcAttack(entity, target, npc, tt, event);

	ELOG_WARNING << "Invalid npc target" << event.targetId();
}





/**
 * @brief RpgLogicPrivate::preRenderEventNpcAttack
 * @param entity
 * @param target
 * @param npc
 * @param object
 * @param event
 */

void RpgLogicPrivate::preRenderEventNpcAttack(entt::entity entity, entt::entity target, Npc *npc,
											  DefenderObject *object, const RpgStream::EventNpc &event)
{
	Q_ASSERT(npc);
	Q_ASSERT(object);

	if (npc->data.team() == object->team) {
		ELOG_WARNING << "DefenderObject's team matches" << event.targetId();
		return;
	}

	RpgStream::NpcState &stNpc = getEditableCurrentState<RpgStream::NpcState>(entity);
	RpgStream::DefenderState &stTarget = getEditableCurrentState<RpgStream::DefenderState>(target);

	if (stNpc.hp() <= 0) {
		ELOG_WARNING << "Npc isn't alive" << stNpc.tick() << stNpc.hp();
		return;
	}

	if (stTarget.hp() <= 0) {
		ELOG_WARNING << "Target defender isn't alive" << stNpc.tick() << stTarget.hp();
		return;
	}


	EventNpcAttackDefender final;
	final.npc = entity;
	final.target = target;
	final.operation = event.operation();

	eventFinalStore(std::move(final));
}



/**
 * @brief RpgLogicPrivate::preRenderEventNpcAttack
 * @param entity
 * @param target
 * @param npc
 * @param object
 * @param event
 */

void RpgLogicPrivate::preRenderEventNpcAttack(entt::entity entity, entt::entity target, Npc *npc,
											  Tower *object, const RpgStream::EventNpc &event)
{
	Q_ASSERT(npc);
	Q_ASSERT(object);

	if (!object->active) {
		ELOG_WARNING << "Tower inactive" << event.targetId();
		return;
	}

	RpgStream::NpcState &stNpc = getEditableCurrentState<RpgStream::NpcState>(entity);
	RpgStream::TowerState &stTarget = getEditableCurrentState<RpgStream::TowerState>(target);

	if (npc->data.team() == stTarget.team()) {
		ELOG_WARNING << "Tower's team matches" << event.targetId();
		return;
	}

	if (stNpc.hp() <= 0) {
		ELOG_WARNING << "Npc isn't alive" << stNpc.tick() << stNpc.hp();
		return;
	}

	if (stTarget.load() == 0) {
		ELOG_WARNING << "Tower isn't active" << stNpc.tick() << stTarget.load();
		return;
	}


	EventNpcAttackTower final;
	final.npc = entity;
	final.target = target;

	eventFinalStore(std::move(final));
}



/**
 * @brief RpgLogicPrivate::preRenderEventNpcAttack
 * @param entity
 * @param target
 * @param npc
 * @param object
 * @param event
 */

void RpgLogicPrivate::preRenderEventNpcAttack(entt::entity entity, entt::entity target, Npc *npc,
											  Player *object, const RpgStream::EventNpc &event)
{
	Q_ASSERT(npc);
	Q_ASSERT(object);

	RpgStream::NpcState &stNpc = getEditableCurrentState<RpgStream::NpcState>(entity);
	RpgStream::PlayerState &stTarget = getEditableCurrentState<RpgStream::PlayerState>(target);

	if (npc->data.team() == object->team) {
		ELOG_WARNING << "Player's team matches" << event.targetId();
		return;
	}

	if (stNpc.hp() <= 0) {
		ELOG_WARNING << "Npc isn't alive" << stNpc.tick() << stNpc.hp();
		return;
	}

	if (stTarget.hp() == 0) {
		ELOG_WARNING << "Player isn't active" << stNpc.tick() << object->playerData.playerId();
		return;
	}

	if (stTarget.lock() > 0) {
		ELOG_WARNING << "Player locked" << stNpc.tick() << object->playerData.playerId();
		return;
	}


	EventNpcAttackPlayer final;
	final.npc = entity;
	final.target = target;

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
		ELOG_ERROR << "Missing entity";
		return;
	}

	const EventDefenderDestroy &event = q->m_registry.get<EventDefenderDestroy>(ent);

	if (!q->m_registry.valid(event.defenderObject)) {
		ELOG_ERROR << "Missing Defender entity";
		return;
	}

	q->m_registry.emplace_or_replace<DeleteTag>(event.defenderObject);

	m_requireFull = true;

	if (!q->m_registry.valid(event.container))
		return;

	Defender *def = q->m_registry.try_get<Defender>(event.container);

	if (!def) {
		ELOG_ERROR << "Invalid defender entity";
		return;
	}

	def->object = entt::null;
}



/**
 * @brief RpgLogicPrivate::preRenderDefenders
 */

void RpgLogicPrivate::preRenderDefenders()
{
	QMutexLocker locker(&q->m_mutex);

	auto player = q->m_registry.view<Player>();
	auto npc = q->m_registry.view<Npc>();

	for (auto e : q->m_registry.view<DefenderObject>()) {
		const DefenderObject &defender = q->m_registry.get<DefenderObject>(e);

		static const std::unordered_set<RpgStream::BaseDefenderObject::Type> skip = {
			RpgStream::BaseDefenderObject::Multiplier1,
		};

		if (skip.contains(defender.type))
			continue;


		const RpgStream::DefenderState *current = q->getCurrentState<RpgStream::DefenderState>(e);

		if (!current) {
			ELOG_ERROR << "Invalid current state";
			continue;
		}

		if (current->hp() == 0)
			continue;

		std::unordered_set<entt::entity> list;

		for (auto p : player) {
			const Player &pp = q->m_registry.get<Player>(p);

			if (pp.team == defender.team)
				continue;

			const RpgStream::PlayerState *current = q->getCurrentState<RpgStream::PlayerState>(p);

			if (!current) {
				ELOG_ERROR << "Invalid player state";
				continue;
			}

			if (current->hp() == 0)
				continue;

			if (defender.isNear(cpv(current->entityState().posXAsFloat(),
									current->entityState().posYAsFloat() )))
				list.insert(p);
		}

		for (auto p : npc) {
			const Npc &pp = q->m_registry.get<Npc>(p);

			if (pp.data.team() == defender.team)
				continue;

			const RpgStream::NpcState *current = q->getCurrentState<RpgStream::NpcState>(p);

			if (!current) {
				ELOG_ERROR << "Invalid npc state";
				continue;
			}

			if (current->hp() == 0)
				continue;

			if (defender.isNear(cpv(current->entityState().posXAsFloat(),
									current->entityState().posYAsFloat() )))
				list.insert(p);
		}

		if (list.empty() && defender.nearTargets.empty())
			continue;

		EventDefenderStep final;
		final.defender = e;
		final.targets = std::move(list);

		eventFinalStore(std::move(final));
	}

}



/**
 * @brief RpgLogicPrivate::preRenderUtilities
 */

void RpgLogicPrivate::preRenderUtilities()
{
	QMutexLocker locker(&q->m_mutex);

	for (auto e : q->m_registry.view<Utility>()) {
		Utility &utility = q->m_registry.get<Utility>(e);

		if (utility.destroyAt >= q->lastAuthTick()) {
			ELOG_DEBUG << "Remove utility" << utility.type;
			q->m_registry.emplace_or_replace<DeleteTag>(e);
		} else {
			///utilityMissionary()			... TODO
		}
	}
}



/**
 * @brief RpgLogicPrivate::preRenderEventControlStateChange
 * @param ent
 */

void RpgLogicPrivate::preRenderEventControlStateChange(entt::entity ent)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent)) {
		ELOG_ERROR << "Missing entity";
		return;
	}

	const EventControlStateChange &event = q->m_registry.get<EventControlStateChange>(ent);

	Control *control = nullptr;

	if (q->m_registry.valid(event.control)) {
		control = q->m_registry.try_get<Control>(event.control);
	}

	if (!control) {
		ELOG_ERROR << "Invalid control";
		return;
	}

	if (!q->onControlStateChange(ent, control, event.isAlive, event.state))
		return;

	RpgStream::ControlState &stTarget = getEditableCurrentState<RpgStream::ControlState>(event.control);

	stTarget.setIsAlive(event.isAlive);
	stTarget.setState(event.state);

	// Register event

	RpgStream::EventControl ev;
	ev.setTagId(control->idTag);
	ev.control().setType(control->type);
	ev.control().setIsAlive(event.isAlive);
	ev.control().setState(event.state);

	eventRealStore(std::move(ev));

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
	std::map<entt::entity, std::vector<EventControl*> > listControl;
	std::map<entt::entity, std::vector<EventDefenderPut*> > listDefenderPut;


	for (auto e : view) {
		if (EventMpPick *event = q->m_registry.try_get<EventMpPick>(e))
			listMpPick[event->mp].push_back(event);
		else if (EventTower *event = q->m_registry.try_get<EventTower>(e))
			listTower[event->tower].push_back(event);
		else if (EventControl *event = q->m_registry.try_get<EventControl>(e))
			listControl[event->control].push_back(event);
		else if (EventDefenderPut *event = q->m_registry.try_get<EventDefenderPut>(e))
			listDefenderPut[event->defender].push_back(event);
		else if (EventDefenderAdd *event = q->m_registry.try_get<EventDefenderAdd>(e))
			renderEvents(event);
		else if (EventAttackPlayer *event = q->m_registry.try_get<EventAttackPlayer>(e))
			renderEvents(event);
		else if (EventAttackNpc *event = q->m_registry.try_get<EventAttackNpc>(e))
			renderEvents(event);
		else if (EventAttackDefender *event = q->m_registry.try_get<EventAttackDefender>(e))
			renderEvents(event);
		else if (EventChangeBullet *event = q->m_registry.try_get<EventChangeBullet>(e))
			renderEvents(event);
		else if (EventChangeDefender *event = q->m_registry.try_get<EventChangeDefender>(e))
			renderEvents(event);
		else if (EventChangeUtility *event = q->m_registry.try_get<EventChangeUtility>(e))
			renderEvents(event);
		else if (EventUtility *event = q->m_registry.try_get<EventUtility>(e))
			renderEvents(event);

		else if (EventNpcAttackDefender *event = q->m_registry.try_get<EventNpcAttackDefender>(e))
			renderEvents(event);
		else if (EventNpcAttackTower *event = q->m_registry.try_get<EventNpcAttackTower>(e))
			renderEvents(event);
		else if (EventNpcAttackPlayer *event = q->m_registry.try_get<EventNpcAttackPlayer>(e))
			renderEvents(event);

		else if (EventDefenderStep *event = q->m_registry.try_get<EventDefenderStep>(e))
			renderEvents(event);

		else if (EventPatchPlayer *event = q->m_registry.try_get<EventPatchPlayer>(e))
			renderEvents(event);

		else
			ELOG_ERROR << "Invalid event";
	}



	for (const auto &[mp, list] : listMpPick)
		renderEvents(mp, list);

	for (const auto &[e, list] : listTower)
		renderEvents(e, list);

	for (const auto &[mp, list] : listDefenderPut)
		renderEvents(mp, list);

	for (const auto &[e, list] : listControl)
		renderEvents(e, list);


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





	// Register mp state (delete

	q->m_registry.emplace_or_replace<DeleteTag>(mpent);




	// Register event

	const Player &p = q->m_registry.get<Player>(final->player);

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventMpPick);
	e.setTagId(p.idTag());
	e.setTarget(mp->idTag);

	eventRealStore(std::move(e));

	ELOG_DEBUG << "Player" << p.playerData.playerId() << "picked MP" << mp->idTag << "at" << st.tick();


	// Next emitter event

	if (MpEmitter *emitter = q->m_registry.try_get<MpEmitter>(mp->emitter)) {
		std::erase(emitter->mpList, mpent);

		if (emitter->mpList.empty()) {
			EventMpEmitterEmpty e;
			e.emitter = mp->emitter;
			eventRealStore(std::move(e));
		}
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
		PlayerPrivate *pp = q->m_registry.try_get<PlayerPrivate>(e->player);

		Q_ASSERT(player);
		Q_ASSERT(pp);

		RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(e->player);



		if (e->lock) {
			if (e->skipLock) {
				ELOG_DEBUG << "Player" << player->playerData.playerId() << "lock skip at" << q->lastAuthTick();

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
			load = pp->towerPlus;
		} else if (player->team == st.team()) {
			load += pp->towerPlus;
		} else {
			load -= pp->towerMinus;
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
		if (q->m_registry.ctx().get<RpgStream::GameConfig>().stage() == RpgStream::GameConfig::StageLast) {
			if (CFG_TOWER_LOCK_STAGE_L > 0)
				st.setLockedUntil(q->lastAuthTick()+1 + CFG_TOWER_LOCK_STAGE_L);
		} else {
			st.setLockedUntil(q->lastAuthTick()+1 + CFG_TOWER_LOCK);
		}
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
	//RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(d->tower);


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
 * @param list
 */

void RpgLogicPrivate::renderEvents(entt::entity ent, const std::vector<EventControl *> &list)
{
	QMutexLocker locker(&q->m_mutex);

	if (list.empty() || !q->m_registry.valid(ent))
		return;

	Control *d = q->m_registry.try_get<Control>(ent);

	Q_ASSERT(d);

	EventControl *final = list.front();			// Arbitrary select - TODO: earlier locked


	Player *p = q->m_registry.try_get<Player>(final->player);
	PlayerPrivate *pp = q->m_registry.try_get<PlayerPrivate>(final->player);

	Q_ASSERT(p);
	Q_ASSERT(pp);


	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(final->player);

	if (final->lock) {
		if (final->skipLock) {
			ELOG_DEBUG << "Player" << p->playerData.playerId() << "lock skip at" << q->lastAuthTick();

			pst.setLock(0);
		} else {
			const quint32 lockId = playerLock(final->player);

			if (lockId == 0) {
				return;
			}

			pst.setLock(lockId);

			return;
		}
	} else {
		pst.setLock(0);

		q->m_registry.remove<LockTag>(final->player);
	}


	playerIncreaseStreak(final->player, final->skipLock);

	controlUse(ent, d, p, pp);


	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventUseControl);
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

	ELOG_INFO << "Add Defender type" << event->type << "to chunk" << event->chunk.x << event->chunk.y;

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


	ELOG_DEBUG << "Player" << p->playerData.playerId() << "attacks" << t->playerData.playerId() << "at" << q->lastAuthTick();

	if (event->withSuccess) {
		if (event->withHurt)
			playerDecreaseHp(event->target, 1);


		cpVect knockback = RpgLogic::addKnockbackImpulse(&tg.entityState(), st.entityState(), pCfg->toEntityConfig(), tCfg->toEntityConfig());
		if (!cpveql(knockback, cpvzero))
			q->m_registry.emplace_or_replace<KnockbackTag>(event->target, q->lastAuthTick());
	} else {
		ELOG_DEBUG << "Player" << p->playerData.playerId() << "attack" << t->playerData.playerId() << "failed";
	}

	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackPlayer);
	e.setTagId(p->idTag());
	e.setTarget(t->idTag());
	e.setSuccess(event->withSuccess);

	eventRealStore(std::move(e));
}



/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventAttackNpc *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	Player *p = q->m_registry.try_get<Player>(event->player);
	Npc *t = q->m_registry.try_get<Npc>(event->target);

	PlayerPrivate *pCfg = q->m_registry.try_get<PlayerPrivate>(event->player);

	Q_ASSERT(p);
	Q_ASSERT(t);
	Q_ASSERT(pCfg);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(event->player);
	RpgStream::NpcState &tg = getEditableCurrentState<RpgStream::NpcState>(event->target);

	if (st.bullet() > 0)
		st.setBullet(st.bullet()-1);


	ELOG_DEBUG << "Player" << p->playerData.playerId() << "attacks NPC" << t->idTag << "at" << q->lastAuthTick();

	if (event->withSuccess) {
		const quint32 npcHp = npcDecreaseHp(event->target, 1);

		cpVect knockback = RpgLogic::addKnockbackImpulse(&tg.entityState(), st.entityState(), pCfg->toEntityConfig(), t->data.entity());
		if (!cpveql(knockback, cpvzero))
			q->m_registry.emplace_or_replace<KnockbackTag>(event->target, q->lastAuthTick());


		// On NPC dead

		if (npcHp == 0 && t->data.mp() > 0) {
			int mp = t->data.mp();
			int pMp = std::min((int) pCfg->maxMp - (int) st.mp(), mp);

			if (pMp > 0) {
				ELOG_DEBUG << "Player" << p->playerData.playerId() << "gained" << pMp << "MP from NPC" << t->idTag << "at" << q->lastAuthTick();
				st.setMp(st.mp() + pMp);
			}

			mp -= pMp;

			if (mp > 0) {
				EventMpCreate evc;
				evc.setTick(q->lastAuthTick()+2);
				evc.parent = event->target;
				evc.mpCount = mp;
				evc.pos.x = tg.entityState().posXAsFloat();
				evc.pos.y = tg.entityState().posYAsFloat();

				q->eventStore(std::move(evc));
			}
		}

	} else {
		ELOG_DEBUG << "Player" << p->playerData.playerId() << "attack NPC" << t->idTag << "failed";
	}


	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventAttackPlayer);
	e.setTagId(p->idTag());
	e.setTarget(t->idTag);
	e.setSuccess(event->withSuccess);


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

	Player *p = q->m_registry.try_get<Player>(event->player);
	DefenderObject *t = q->m_registry.try_get<DefenderObject>(event->target);

	Q_ASSERT(p);
	Q_ASSERT(t);


	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);
	RpgStream::DefenderState &st = getEditableCurrentState<RpgStream::DefenderState>(event->target);

	if (event->lock) {
		if (event->skipLock) {
			ELOG_DEBUG << "Player" << p->playerData.playerId() << "lock skip" << "at" << q->lastAuthTick();

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


	ELOG_DEBUG << "Player" << p->playerData.playerId() << "attacks defender" << t->idTag << "at" << q->lastAuthTick();


	playerIncreaseStreak(event->player, event->skipLock);

	/*
	if (t->type == RpgStream::BaseDefenderObject::Dummy) {
		//// TODO
		///

		DefenderDummyObject *dummy = q->m_registry.try_get<DefenderDummyObject>(event->target);

		if (!dummy) {
			ELOG_WARNING << "DefenderDummyObject entity not found";
			return;
		}

		st.setDummy(18);
	}
*/


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

	Player *p = q->m_registry.try_get<Player>(event->player);

	Q_ASSERT(p);


	const PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(event->player);

	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);

	if (event->lock) {
		if (event->skipLock) {
			ELOG_DEBUG << "Player" << p->playerData.playerId() << "lock skip" << "at" << q->lastAuthTick();
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

	ELOG_DEBUG << "Player" << p->playerData.playerId() << "load magazine at" << q->lastAuthTick();

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

	Player *p = q->m_registry.try_get<Player>(event->player);

	Q_ASSERT(p);


	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);

	if (event->lock) {
		if (event->skipLock) {
			ELOG_DEBUG << "Player" << p->playerData.playerId() << "lock skip at" << q->lastAuthTick();
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

	ELOG_DEBUG << "Player" << p->playerData.playerId() << "load defender type" << event->type << "at" << q->lastAuthTick();

	playerIncreaseStreak(event->player, event->skipLock);


	const quint32 mp = cfgRequiredMpDefender.value(event->type);

	if (pst.mp() > mp)
		pst.setMp(pst.mp()-mp);
	else
		pst.setMp(0);

	pst.setHasDefender(true);



	// Register event

	RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventChangeDefender);
	ev.setTagId(p->idTag());

	eventRealStore(std::move(ev));
}




/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventChangeUtility *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	Player *p = q->m_registry.try_get<Player>(event->player);

	Q_ASSERT(p);


	RpgStream::PlayerState &pst = getEditableCurrentState<RpgStream::PlayerState>(event->player);

	if (event->lock) {
		if (event->skipLock) {
			ELOG_DEBUG << "Player" << p->playerData.playerId() << "lock skip at" << q->lastAuthTick();
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

	ELOG_DEBUG << "Player" << p->playerData.playerId() << "load utility type" << event->type << "at" << q->lastAuthTick();

	playerIncreaseStreak(event->player, event->skipLock);


	const quint32 mp = cfgRequiredMpUtility.value(event->type);

	if (pst.mp() > mp)
		pst.setMp(pst.mp()-mp);
	else
		pst.setMp(0);

	pst.setHasUtility(true);



	// Register event

	RpgStream::EventPlayer ev(RpgStream::EventPlayer::EventChangeUtility);
	ev.setTagId(p->idTag());

	eventRealStore(std::move(ev));
}




/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventUtility *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	Player *p = q->m_registry.try_get<Player>(event->player);
	PlayerPrivate *pp = q->m_registry.try_get<PlayerPrivate>(event->player);

	Q_ASSERT(p);
	Q_ASSERT(pp);

	if (!utility(p, pp, event->type, event->target)) {
		ELOG_INFO << "Player" << p->playerData.playerId() << "use utility" << event->type << "failed";
		return;
	}

	ELOG_INFO << "Player" << p->playerData.playerId() << "use utility" << event->type;

	RpgStream::PlayerState &sp = getEditableCurrentState<RpgStream::PlayerState>(event->player);

	sp.setHasUtility(false);

	// Register event

	RpgStream::EventPlayer e(RpgStream::EventPlayer::EventUseUtility);
	e.setTagId(q->m_registry.get<Player>(event->player).idTag());

	if (q->m_registry.valid(event->target)) {
		if (IdTag *id = q->m_registry.try_get<IdTag>(event->target)) {
			e.setTarget(id->id);
		}
	}

	eventRealStore(std::move(e));
}




/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventNpcAttackDefender *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	Npc *p = q->m_registry.try_get<Npc>(event->npc);
	DefenderObject *t = q->m_registry.try_get<DefenderObject>(event->target);

	Q_ASSERT(p);
	Q_ASSERT(t);


	RpgStream::DefenderState &st = getEditableCurrentState<RpgStream::DefenderState>(event->target);

	ELOG_DEBUG << "NPC" << p->idTag << "attacks defender" << t->idTag << "at" << q->lastAuthTick();


	/*if (t->type == RpgStream::BaseDefenderObject::Dummy) {
		//// TODO
		///

		DefenderDummyObject *dummy = q->m_registry.try_get<DefenderDummyObject>(event->target);

		if (!dummy) {
			ELOG_WARNING << "DefenderDummyObject entity not found";
			return;
		}

		st.setDummy(18);
	}*/


	if (st.hp() > 0)
		st.setHp(st.hp()-1);


	// Register event

	RpgStream::EventNpc ev(RpgStream::EventNpc::EventAttack);
	ev.setTagId(p->idTag);
	ev.setTargetId(t->idTag);

	eventRealStore(std::move(ev));
}


/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventNpcAttackTower *event)
{
	QMutexLocker locker(&q->m_mutex);

	Npc *p = q->m_registry.try_get<Npc>(event->npc);
	Tower *tower = q->m_registry.try_get<Tower>(event->target);

	Q_ASSERT(p);
	Q_ASSERT(tower);

	const RpgStream::TowerState *last = q->getLastState<RpgStream::TowerState>(event->target);
	RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(event->target);

	int load = st.load();

	load -= std::max(1u, p->data.force());

	if (load <= 0) {
		st.setTeam(RpgStream::TeamNone);
		st.setLoad(0);
	} else {
		st.setLoad(load);
	}

	ELOG_DEBUG << "NPC" << p->idTag << "attacks tower" << tower->idTag << "at" << q->lastAuthTick();

	// Register event

	RpgStream::EventNpc ev(RpgStream::EventNpc::EventAttack);
	ev.setTagId(p->idTag);
	ev.setTargetId(tower->idTag);

	eventRealStore(std::move(ev));


	if (load < CFG_TOWER_INACTIVE)
		st.setActive(false);


	// Register event

	if (last && (last->active() != st.active() || last->team() != st.team())) {
		EventTowerActiveChanged ev;
		ev.tower = event->target;
		ev.active = st.active();
		ev.team = st.team();

		eventRealStore(std::move(ev));
	}

}



/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventNpcAttackPlayer *event)
{
	QMutexLocker locker(&q->m_mutex);

	Npc *p = q->m_registry.try_get<Npc>(event->npc);
	Player *player = q->m_registry.try_get<Player>(event->target);

	Q_ASSERT(p);
	Q_ASSERT(player);

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(event->target);

	ELOG_DEBUG << "NPC" << p->idTag << "attacks player" << player->playerData.playerId() << "at" << q->lastAuthTick();


	switch (p->data.type()) {
		case RpgStream::NpcData::MpLeecher:
			if (st.mp() > 0) {
				const quint32 diff = std::min(st.mp(), std::max(1u, p->data.force()));
				st.setMp(st.mp() - diff);
			} else {
				return;
			}
			break;

		case RpgStream::NpcData::TowerAttacker:
		case RpgStream::NpcData::None:
			ELOG_WARNING << "NPC" << p->idTag << "can't hurt player";
			return;

	}

	// Register event

	RpgStream::EventNpc ev(RpgStream::EventNpc::EventAttack);
	ev.setTagId(p->idTag);
	ev.setTargetId(player->idTag());

	eventRealStore(std::move(ev));
}




/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventDefenderStep *event)
{
	QMutexLocker locker(&q->m_mutex);

	DefenderObject *d = q->m_registry.try_get<DefenderObject>(event->defender);

	Q_ASSERT(d);



	// Remove missing targets

	for (auto it = d->nearTargets.cbegin(); it != d->nearTargets.cend(); ) {
		if (!event->targets.contains(*it)) {
			it = d->nearTargets.erase(it);
		} else {
			++it;
		}
	}

	// Add new targets

	for (auto e : event->targets) {
		if (!d->nearTargets.contains(e)) {
			d->nearTargets.insert(e);
		}
	}


	RpgStream::DefenderState &st = getEditableCurrentState<RpgStream::DefenderState>(event->defender);


	if (d->nearTargets.empty()) {
		st.setTargetId(0);
		return;
	}

	if (!st.visible()) {
		st.setVisible(true);
		ELOG_DEBUG << "Defender" << d->idTag << "became visible at" << q->lastAuthTick();
	}


	// Skip action

	if (d->repeaterDelay > 0 && d->lastAction > 0 &&
			d->lastAction + d->repeaterDelay > q->lastAuthTick()) {
		return;
	}

	entt::entity tg = defenderRender(d, st);


	// Save action

	if (tg == entt::null) {
		st.setTargetId(0);
		return;
	}

	d->lastAction = q->lastAuthTick();
	++d->actionCounter;
	d->lastTargets.insert(tg);

	if (d->actionsToHpLoss > 0 && d->actionCounter >= d->actionsToHpLoss) {
		d->actionCounter = 0;

		if (st.hp() > 0)
			st.setHp(st.hp()-1);

		if (st.hp() == 0)
			ELOG_DEBUG << "Defender" << d->idTag << "out";
	}




	// Register event

	RpgStream::EventDefender ev;
	ev.setTagId(d->idTag);

	if (IdTag *t = q->m_registry.try_get<IdTag>(tg)) {
		ev.setTargetId(t->id);
		st.setTargetId(t->id);

		ELOG_DEBUG << "Defender" << d->idTag << "attacks" << t->id << "at" << q->lastAuthTick();
	} else {
		ELOG_DEBUG << "Defender" << d->idTag << "attacks at" << q->lastAuthTick();

		st.setTargetId(0);
	}

	eventRealStore(std::move(ev));
}



/**
 * @brief RpgLogicPrivate::renderEvents
 * @param event
 */

void RpgLogicPrivate::renderEvents(EventPatchPlayer *event)
{
	if (!event)
		return;

	QMutexLocker locker(&q->m_mutex);

	entt::entity player = q->entityFromIdTag(event->tagId);

	if (!q->m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << event->tagId;
		return;
	}

	Player *p = q->m_registry.try_get<Player>(player);

	if (!p) {
		ELOG_WARNING << "Invalid player entity" << event->tagId;
		return;
	}

	RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(player);

	ELOG_DEBUG << "Player" << p->playerData.playerId() << "patch state" << "at" << q->lastAuthTick();

	st.loadFromDelta(event->deltaState, true);
}




/**
 * @brief RpgLogicPrivate::renderEntityKnockbacks
 */

void RpgLogicPrivate::renderEntityKnockbacks()
{
	QMutexLocker locker(&q->m_mutex);

	if (q->lastAuthTick() < 1) {
		//LOG_CINFO << "SKIP";
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

			if (cpveql(knock, cpvzero))
				q->m_registry.remove<KnockbackTag>(e);


		} else if (q->m_registry.try_get<Npc>(e)) {
			RpgStream::NpcState &st = getEditableCurrentState<RpgStream::NpcState>(e);

			const cpVect knock = RpgLogic::decayKnockback(&st.entityState());

			if (cpveql(knock, cpvzero))
				q->m_registry.remove<KnockbackTag>(e);

		} else {
			ELOG_ERROR << "Invalid knockback entity";
		}
	}

}






/**
 * @brief RpgLogicPrivate::renderPlayerInputs
 */

void RpgLogicPrivate::renderPlayerInputs(const bool &first)
{
	QMutexLocker locker(&q->m_mutex);

	if (!first && q->lastAuthTick() < 1)
		return;

	const quint32 tick = q->lastAuthTick();

	auto view = q->m_registry.view<Player>();

	for (auto e : view) {
		auto [player, input] =
				q->m_registry.try_get<Player, PlayerStateInput>(e);

		const RpgStream::PlayerState *lastInput = input ? input->at(tick) : nullptr;
		const RpgStream::PlayerState *current = q->getCurrentState<RpgStream::PlayerState>(e);

		if (!current && !lastInput)
			continue;

		if (!current) {
			ELOG_DEBUG << "Create first state for player" << player->idTag() << "at" << tick;
			continue;
		}


		RpgStream::PlayerState state = *current;

		state.setTick(tick);

		// Ha van input, akkor csak az engedélyezett mezőket írjuk felül

		if ((first || current->hp() > 0) && lastInput) {
			state.entityState().setPosX(lastInput->entityState().posX());
			state.entityState().setPosY(lastInput->entityState().posY());
			state.entityState().setVelSq(lastInput->entityState().velSq());
			state.entityState().setAngle(lastInput->entityState().angle());
			state.entityState().setFacing(lastInput->entityState().facing());
		}

		renderFinal(e, std::move(state));

		if (input)
			input->clear(tick);
	}

}



/**
 * @brief RpgLogicPrivate::renderNpcInputs
 * @param first
 */

void RpgLogicPrivate::renderNpcInputs(const bool &first)
{
	QMutexLocker locker(&q->m_mutex);

	if (!first && q->lastAuthTick() < 1)
		return;

	const quint32 tick = q->lastAuthTick();

	auto view = q->m_registry.view<Npc>();

	for (auto e : view) {
		auto [npc, input] =
				q->m_registry.try_get<Npc, NpcStateInput>(e);

		const RpgStream::NpcState *lastInput = input ? input->at(tick) : nullptr;
		const RpgStream::NpcState *current = q->getCurrentState<RpgStream::NpcState>(e);

		if (!current && !lastInput)
			continue;

		if (!current) {
			ELOG_DEBUG << "Create first state for NPC" << npc->idTag << "at" << tick;
			continue;
		}


		RpgStream::NpcState state = *current;

		state.setTick(tick);

		// Ha van input, akkor csak az engedélyezett mezőket írjuk felül

		if ((first || current->hp() > 0) && lastInput) {
			state.entityState().setPosX(lastInput->entityState().posX());
			state.entityState().setPosY(lastInput->entityState().posY());
			state.entityState().setVelSq(lastInput->entityState().velSq());
			state.entityState().setAngle(lastInput->entityState().angle());
			state.entityState().setFacing(lastInput->entityState().facing());

			npcRenderInput(*npc, *lastInput, state);
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
	renderFinalControls();
	renderFinalStage();
}




/**
 * @brief RpgLogicPrivate::renderFinalTowers
 */

void RpgLogicPrivate::renderFinalTowers()
{
	QMutexLocker locker(&q->m_mutex);

	auto view = q->m_registry.view<Tower>();

	for (auto e : view) {
		Tower &tower = q->m_registry.get<Tower>(e);

		RpgStream::TowerState &st = getEditableCurrentState<RpgStream::TowerState>(e);

		quint32 mult = 0;

		std::vector<RpgStream::FullMapTag> dList;

		for (auto e : tower.defenderList) {
			if (!q->m_registry.valid(e))
				continue;

			Defender *td = q->m_registry.try_get<Defender>(e);

			if (!td) {
				ELOG_ERROR << "Invalid defender";
				continue;
			}

			if (!q->m_registry.valid(td->object))
				continue;

			const DefenderObject &obj = q->m_registry.get<DefenderObject>(td->object);

			RpgStream::FullMapTag mt;
			mt.setTagId(obj.idTag);
			dList.emplace_back(std::move(mt));

			if (const RpgStream::DefenderState *dst = q->getCurrentState<RpgStream::DefenderState>(td->object)) {
				if (dst->hp() > 0) {
					if (dst->type() == RpgStream::BaseDefenderObject::Multiplier1)
						++mult;
				}
			} else {
				ELOG_ERROR << "Invalid DefenderObject state";
			}
		}

		st.setMultiply(mult);
		st.setDefenders(std::move(dList));


		RpgStream::TowerState state = st;

		renderFinal(e, std::move(state));
	}



	if (q->lastAuthTick() % 60 == 0) {
		RpgStream::GameState &state = q->m_registry.ctx().get<RpgStream::GameState>();

		const quint32 pts = q->m_registry.ctx().get<RpgStream::GameConfig>().stage() == RpgStream::GameConfig::StageLast ?
								CFG_POINT_STAGE_L :
								CFG_POINT;


		for (auto e : view) {
			const RpgStream::TowerState *current = q->getCurrentState<RpgStream::TowerState>(e);

			if (!current) {
				ELOG_ERROR << "Missing state" << q->lastAuthTick();
				continue;
			}

			const quint32 tpts = pts * (1+current->multiply());

			if (current->active()) {
				if (current->team() == RpgStream::TeamA)
					state.setPtsA(state.ptsA() + tpts);
				else if (current->team() == RpgStream::TeamB)
					state.setPtsB(state.ptsB() + tpts);
			}
		}

		q->checkState(state);
	}
}




/**
 * @brief RpgLogicPrivate::renderFinalDefenders
 */

void RpgLogicPrivate::renderFinalDefenders()
{
	QMutexLocker locker(&q->m_mutex);

	const RpgStream::GameConfig::Stage currentStage = q->m_registry.ctx().get<RpgStream::GameConfig>().stage();


	for (auto e : q->m_registry.view<DefenderObject>()) {
		const RpgStream::DefenderState *current = q->m_registry.try_get<RpgStream::DefenderState>(e);

		if (!current) {
			//ELOG_ERROR << "MISSING STATE";
			continue;
		}

		RpgStream::DefenderState state = *current;
		const DefenderObject &def = q->m_registry.get<DefenderObject>(e);


		/*entt::entity towerEntity = entt::null;
		Tower *tower = nullptr;

		if (q->m_registry.valid(def.defender)) {
			if (Defender *dd = q->m_registry.try_get<Defender>(def.defender)) {
				if (!q->m_registry.valid(dd->tower)) {
					ELOG_ERROR << "Invalid tower of defender";
				} else {
					towerEntity = dd->tower;
					tower = q->m_registry.try_get<Tower>(towerEntity);
				}
			}
		}*/



		if (state.hp() == 0) {
			EventDefenderDestroy ev;
			ev.defenderObject = e;
			ev.container = def.defender;
			ev.setTick(q->lastAuthTick()+
					   (def.defender == entt::null ||
						(currentStage == RpgStream::GameConfig::StageLast) ? 1 : CFG_DEFENDER_DESTROY));

			q->eventStore(std::move(ev));

			ELOG_INFO << "Destroy defender" << def.idTag << "at" << q->lastAuthTick() << "->" << ev.tick();
		}

		renderFinal(e, std::move(state));
	}
}




/**
 * @brief RpgLogicPrivate::renderFinalStage
 */

void RpgLogicPrivate::renderFinalStage()
{
	QMutexLocker locker(&q->m_mutex);

	RpgStream::GameConfig &cfg = q->m_registry.ctx().get<RpgStream::GameConfig>();

	if (q->m_serverTick >= cfg.duration()) {
		ELOG_INFO << "Game finished at" << q->m_serverTick;

		q->m_registry.ctx().insert_or_assign<RpgStream::Result>(q->getResult());

		changeStage(RpgStream::GameConfig::StageFinished);

		cfg.flags().setFlag(RpgStream::GameConfig::FlagFinished);
	}
}



/**
 * @brief RpgLogicPrivate::renderFinalControls
 */

void RpgLogicPrivate::renderFinalControls()
{
	QMutexLocker locker(&q->m_mutex);

	for (auto e : q->m_registry.view<Control>()) {
		const RpgStream::ControlState *current = q->m_registry.try_get<RpgStream::ControlState>(e);

		if (!current)
			continue;

		RpgStream::ControlState state = *current;

		renderFinal(e, std::move(state));
	}
}






/**
 * @brief RpgLogicPrivate::removeDeleteTags
 */

void RpgLogicPrivate::removeDeleteTags()
{
	QMutexLocker locker(&q->m_mutex);

	// Remove DeleteTag entities

	IdTagMapper &mapper = q->m_registry.ctx().get<IdTagMapper>();
	auto view = q->m_registry.view<DeleteTag>();

	for (auto e : view) {
		if (IdTag *tag = q->m_registry.try_get<IdTag>(e)) {
			mapper.map.remove(tag->id);
			q->m_registry.remove<IdTag>(e);
		}
	}

	q->m_registry.destroy(view.begin(), view.end());

}





/**
 * @brief RpgLogicPrivate::changeStage
 * @param stage
 */

void RpgLogicPrivate::changeStage(const RpgStream::GameConfig::Stage &stage)
{
	QMutexLocker locker(&q->m_mutex);

	RpgStream::GameConfig &cfg = q->m_registry.ctx().get<RpgStream::GameConfig>();

	if (stage <= cfg.stage()) {
		ELOG_ERROR << "Can't change stage from" << cfg.stage() << "to" << stage;
		return;
	}

	ELOG_INFO << "Change stage to" << stage;

	m_oldStage = cfg.stage();

	cfg.setStage(stage);

	// Register event

	RpgStream::EventStageChanged e;
	e.setConfig(cfg);

	eventRealStore(std::move(e));
}





/**
 * @brief RpgLogicPrivate::autoSelectInventory
 */

void RpgLogicPrivate::autoSelectInventory()
{
	QMutexLocker locker(&q->m_mutex);

	for (auto e : q->m_registry.view<Player>()) {
		Player &p = q->m_registry.get<Player>(e);
		PlayerPrivate &pp = q->m_registry.get<PlayerPrivate>(e);

		if (p.playerData.config().defenders().empty() &&
				p.playerData.config().utilities().empty()) {
			ELOG_DEBUG << "Player" << p.playerData.playerId() << "inventory empty!";
			continue;
		}

		RpgStream::PlayerState &st = getEditableCurrentState<RpgStream::PlayerState>(e);

		if (p.playerData.quest().question() == 0) {
			const QuestList *list = q->m_registry.ctx().find<QuestList>();

			if (!list || list->empty()) {
				ELOG_ERROR << "Missing QuestList";
			} else {
				p.playerData.setQuest(list->front());
				pp.requiredStreak = list->front().streak();

				ELOG_DEBUG << "Player" << p.playerData.playerId() << "auto select quest";
			}

		}

		if (!p.playerData.config().defenders().empty() &&
				st.defender() == RpgStream::BaseDefenderObject::None) {
			st.setDefender(p.playerData.config().defenders().front());
			ELOG_DEBUG << "Player" << p.playerData.playerId() << "auto select defender:" << st.defender();
		}

		if (!p.playerData.config().utilities().empty() &&
				st.utility() == RpgStream::PlayerConfig::UtilityNone) {
			st.setUtility(p.playerData.config().utilities().front());
			ELOG_DEBUG << "Player" << p.playerData.playerId() << "auto select utility:" << st.utility();
		}
	}

	m_requireFull = true;
}



/**
 * @brief RpgLogicPrivate::onEventStageChanged
 * @param event
 */

void RpgLogicPrivate::onEventStageChanged(const RpgStream::EventStageChanged &event)
{
	QMutexLocker locker(&q->m_mutex);

	if (event.config().stage() == RpgStream::GameConfig::StageWarmingUp) {
		for (auto e : q->m_registry.view<MpEmitter>()) {
			if (!q->m_registry.get<MpEmitter>(e).active)
				continue;

			EventMpCreate ev = EventMpCreate::createMp(q->m_registry.ctx().get<RpgStream::GameConfig>().stage(), q->lastAuthTick());
			ev.emitter = e;

			ELOG_DEBUG << "Register WarmingUp MP create event for" << ev.tick();

			q->eventStore(std::move(ev));
		}
	} else if (event.config().stage() == RpgStream::GameConfig::StageLast) {
		for (auto e : q->m_registry.view<MpEmitter>()) {
			if (!q->m_registry.get<MpEmitter>(e).active)
				continue;

			EventMpCreate ev = EventMpCreate::createMp(q->m_registry.ctx().get<RpgStream::GameConfig>().stage(), q->lastAuthTick());
			ev.emitter = e;

			ELOG_DEBUG << "Register Last MP create event for" << ev.tick();

			q->eventStore(std::move(ev));
		}
	}
}







/**
 * @brief RpgLogicPrivate::controlUpdate
 * @param list
 */

void RpgLogicPrivate::controlUpdate(const std::vector<RpgStream::ControlData> &list)
{
	QMutexLocker locker(&q->m_mutex);

	std::unordered_map<quint32, entt::entity> entities;
	entities.reserve(list.size());

	for (auto e : q->m_registry.view<Control>()) {
		if (!q->m_registry.valid(e))
			continue;

		entities[q->m_registry.get<Control>(e).idTag] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());


	for (const RpgStream::ControlData &p : list) {
		ids.insert(p.tagId());

		auto it = entities.find(p.tagId());

		if (it != entities.end()) {
			/*q->m_registry.patch<Npc>(it->second, [&p](Npc &t) {
				t.data.entity().setMaxHp(p.entity().maxHp());
				t.data.setTeam(p.team());
			});*/
		} else {
			auto entity = q->m_registry.create();

			Control control;
			control.idTag = p.tagId();
			control.type = p.type();
			control.pos.x = p.posXAsFloat();
			control.pos.y = p.posYAsFloat();
			control.data = p.data();

			q->entitySetIdTag(entity, control.idTag);

			ControlStateOutput &out = q->m_registry.emplace<ControlStateOutput>(entity);

			RpgStream::ControlState state;

			state.setType(p.type());
			state.setTick(q->lastAuthTick());
			state.setIsAlive(false);

			controlUpdate(entity, p, &control, &state);

			out.append(std::move(state));

			q->m_registry.emplace<Control>(entity, std::move(control));
		}
	}

	for (const auto &[id, e] : entities) {
		if (ids.contains(id))
			continue;

		q->m_registry.emplace<DeleteTag>(e);
	}
}





/**
 * @brief RpgLogicPrivate::controlUpdate
 * @param entity
 * @param stream
 * @param object
 * @param state
 */

void RpgLogicPrivate::controlUpdate(entt::entity entity, const RpgStream::ControlData &stream, Control *object, RpgStream::ControlState *state)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(entity)) {
		ELOG_ERROR << "Invalid entity";
		return;
	}

	RpgStream::ControlData::Type t = object ? object->type : stream.type();

	if (t == RpgStream::ControlData::Chest) {
		q->m_registry.emplace<Chest>(entity);

		/*if (object)
			object->data.entity().setMaxHp(5);

		if (state) {
			state->setDummy(125);
			state->setHp(1);
		}*/

	}
}



/**
 * @brief RpgLogicPrivate::preRenderControlCheck
 * @param control
 * @param state
 * @param player
 * @return
 */

bool RpgLogicPrivate::preRenderControlCheck(Control *control, const RpgStream::ControlState &state, Player *player)
{
	Q_ASSERT(control);

	QMutexLocker locker(&q->m_mutex);

	if (control->type == RpgStream::ControlData::Chest) {
		if (state.state() > Chest::StateNormal) {
			ELOG_ERROR << "Chest already activated" << control->idTag;
			return false;
		}
	}

	return true;
}




/**
 * @brief RpgLogicPrivate::controlUse
 * @param control
 * @param player
 * @param priv
 */

void RpgLogicPrivate::controlUse(entt::entity entity, Control *control, Player *player, PlayerPrivate *priv)
{
	Q_ASSERT(control);
	Q_ASSERT(player);
	Q_ASSERT(priv);

	QMutexLocker locker(&q->m_mutex);


	RpgStream::ControlState &st = getEditableCurrentState<RpgStream::ControlState>(entity);

	if (control->type == RpgStream::ControlData::Chest) {
		ELOG_INFO << "Player" << player->playerData.playerId() << "opens chest" << control->idTag << "at" << q->lastAuthTick();

		if (st.state() == Chest::StateNormal)
			st.setState(Chest::StateActivated);

		q->increaseHeat();
	}

	/*RpgStream::PlayerState &sp = getEditableCurrentState<RpgStream::PlayerState>(final->player);


	//entt::entity object =
	generateDefender(p, pp, final->type, ent, d);

	sp.setHasDefender(false);
*/
}








/**
 * @brief RpgLogicPrivate::nextIdTag
 * @return
 */

quint32 RpgLogicPrivate::nextIdTag(const LogicIdOwner &ownerId, const quint32 &scene)
{
	QMutexLocker locker(&q->m_mutex);

	return q->packId(scene, ownerId, ++m_lastObjectId[ownerId]);
}


/**
 * @brief RpgLogicPrivate::nextIdTag
 * @param player
 * @return
 */

quint32 RpgLogicPrivate::nextIdTag(Player *player, PlayerPrivate *p, const quint32 &scene)
{
	Q_ASSERT(player);
	Q_ASSERT(p);

	QMutexLocker locker(&q->m_mutex);

	return q->packId(scene, player->playerData.playerId(), ++p->lastObjectId);
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
 * @brief RpgLogicPrivate::mapInitialize
 */

void RpgLogicPrivate::mapInitialize()
{
	QMutexLocker locker(&q->m_mutex);

	// Towers

	const std::unordered_set<entt::entity> &towers = q->initializeTowers();

	for (entt::entity e : q->m_registry.view<Tower>()) {
		Tower &t = q->m_registry.get<Tower>(e);

		t.active = towers.contains(e);

		ELOG_DEBUG << "Tower" << t.idTag << "active:" << t.active;
	}


	// Mp emitters

	const std::unordered_set<entt::entity> &emitters = q->initializeEmitters();

	for (entt::entity e : q->m_registry.view<MpEmitter>()) {
		MpEmitter &t = q->m_registry.get<MpEmitter>(e);

		t.active = emitters.contains(e);

		ELOG_DEBUG << "MP emitter" << t.idTag << "active:" << t.active;
	}



	// Chest

	const ChestList &list = q->initializeChests();

	for (const Chest &chest : list) {
		RpgStream::ControlData d;
		d.setPosXAsFloat(chest.pos.x);
		d.setPosYAsFloat(chest.pos.y);
		d.setType(RpgStream::ControlData::Chest);

		std::bernoulli_distribution dist(0.5);

		d.setData(dist(q->m_rnd) ? 1 : 0);



		quint32 id = 0;

		auto entity = controlAdd(d, chest, &id);

		RpgStream::ControlState &state = getEditableCurrentState<RpgStream::ControlState>(entity);
		state.setTick(0);
		state.setType(RpgStream::ControlData::Chest);
		state.setIsAlive(true);

		ELOG_DEBUG << "Load chest" << id;
	}
}



/**
 * @brief RpgLogicPrivate::loadHeat
 * @param heat
 */

void RpgLogicPrivate::loadHeat(const quint8 &heat)
{
	QMutexLocker locker(&q->m_mutex);

	const HeatList &heatList = q->m_registry.ctx().get<HeatList>();
	RpgStream::GameState &state = q->m_registry.ctx().get<RpgStream::GameState>();

	if (heat >= heatList.size()) {
		ELOG_ERROR << "Invalid heat" << heat;
		return;
	}

	ELOG_DEBUG << "Load heat" << heat;

	state.setHeat(heat);

	const RpgStream::Heat &data = heatList.at(heat);

	for (const RpgStream::HeatNpc &n : data.npc()) {
		q->npcAdd(n);
	}
}





/**
 * @brief RpgLogicPrivate::playerUpdate
 * @param ent
 * @param data
 */

void RpgLogicPrivate::playerUpdate(entt::entity ent, const RpgStream::PlayerData &data)
{
	QMutexLocker locker(&q->m_mutex);

	if (!q->m_registry.valid(ent))
		return;

	Player *p = q->m_registry.try_get<Player>(ent);

	if (!p) {
		ELOG_ERROR << "Invalid player";
		return;
	}

	p->playerData = data;
	p->playerData.setTeam(p->team);
}





/**
 * @brief RpgLogicPrivate::playerUpdate
 * @param list
 */

void RpgLogicPrivate::playerUpdate(const std::vector<RpgStream::PlayerData> &list)
{
	QMutexLocker locker(&q->m_mutex);

	std::unordered_map<quint32, entt::entity> players;
	players.reserve(list.size());

	for (auto e : q->m_registry.view<Player>()) {
		if (!q->m_registry.valid(e))
			continue;

		players[q->m_registry.get<Player>(e).playerData.playerId()] = e;
	}

	std::unordered_set<quint32> ids;

	ids.reserve(list.size());

	for (const RpgStream::PlayerData &p : list) {
		ids.insert(p.playerId());

		auto it = players.find(p.playerId());

		if (it != players.end()) {
			playerUpdate(it->second, p);
		} else {
			entt::entity ent = q->playerAdd(p);
			playerInitialize(ent);
		}
	}

	for (const auto &[id, e] : players) {
		if (ids.contains(id))
			continue;

		q->m_registry.emplace<DeleteTag>(e);
	}
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
		ELOG_ERROR << "Empty player position list";
		return false;
	}

	std::shuffle(list->begin(), list->end(), q->m_rnd);

	QHash<RpgStream::Team, std::vector<RpgStream::PlayerPosition> > map;

	map.insert(RpgStream::TeamNone, {});
	map.insert(RpgStream::TeamA, {});
	map.insert(RpgStream::TeamB, {});

	std::bernoulli_distribution dist(0.5);

	const bool inverse = dist(q->m_rnd);

	for (const RpgStream::PlayerPosition &p : *list) {
		RpgStream::Team team = inverse ? RpgLogic::oppositeTeam(p.team()) : p.team();
		map[team].emplace_back(p);
	}

	bool success = true;

	auto view = q->m_registry.view<Player>(entt::exclude<RpgStream::PlayerPosition>);

	for (auto &player : view) {
		const Player &p = q->m_registry.get<Player>(player);

		std::vector<RpgStream::PlayerPosition> &plist = map[p.team];

		if (plist.empty()) {
			ELOG_ERROR << "Not enough player position for" << p.team;
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
		ELOG_ERROR << "Invalid player";
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
	state.entityState().setVelSq(0);

	priv.modSkipLock = 3;

	ELOG_INFO << "Player" << player->playerData.playerId() << "initialized at" << q->lastAuthTick();

	return true;
}





/**
 * @brief RpgLogic::fullStateLoad (to InputPull)
 * @param full
 */

void RpgLogic::fullStateLoad(const RpgStream::FullState &full, EventWindowHash *acceptedInputList)
{
	if (full.flags().testFlag(RpgStream::FullState::Player))
		d->playerInputLoad(full.players(), acceptedInputList, full.isDeltaMode());

	if (full.flags().testFlag(RpgStream::FullState::Event))
		d->eventInputLoad(full.events(), acceptedInputList);

	if (full.flags().testFlag(RpgStream::FullState::Npc))
		d->npcInputLoad(full.npcs(), acceptedInputList, full.isDeltaMode());
}



/**
 * @brief RpgLogic::getFullState
 * @param maxTick
 * @return
 */

RpgStream::FullState RpgLogic::getFullState(const int &maxTick, QString *textPtr)
{
	RpgStream::FullState full;

	QMutexLocker locker(&m_mutex);

	const RpgStream::GameState &state = m_registry.ctx().get<RpgStream::GameState>();
	const RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	full.setServerTick(m_serverTick);
	full.setState(state);

	if (textPtr) {
		*textPtr += QStringLiteral("TICK %1 / %2 | flags: %3\n")
					.arg(m_serverTick, 5)
					.arg(cfg.duration())
					.arg(cfg.flags().toInt())
					;

		*textPtr += QStringLiteral("==================================================================\n \n");

		*textPtr += QStringLiteral("TEAM 1: %1 points | TEAM 2: %2 points\n \n")
					.arg(state.ptsA(), 5)
					.arg(state.ptsB(), 5)
					;


		*textPtr += QStringLiteral("PLAYERS\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");
	}

	for (auto e : m_registry.view<Player, PlayerStateOutput>()) {
		const auto &[player, output] = m_registry.get<Player, PlayerStateOutput>(e);

		RpgStream::PlayerStateList pl;
		pl.setTagId(player.idTag());
		////pl.setIsDeltaMode(true);
		const std::vector<RpgStream::PlayerState> &list = output.extract(maxTick);
		///pl.compressStateVector(list);

		pl.setIsDeltaMode(false);
		pl.setState(list);

		if (textPtr) {
			*textPtr += QStringLiteral("Player %1 %2 (T%3) | %4\n")
						.arg(player.playerData.playerId())
						.arg(player.playerData.userName())
						.arg(player.team)
						.arg(player.playerData.flags().toInt())
						;

			for (const RpgStream::PlayerState &st : list) {
				*textPtr += QStringLiteral("   [%1] %2 HP %3 MP %4 BULLET (%5,%6) | ")
							.arg(st.tick(), 5)
							.arg(st.hp(), 2)
							.arg(st.mp(), 4)
							.arg(st.bullet(), 3)
							.arg(st.entityState().posXAsFloat())
							.arg(st.entityState().posYAsFloat())
							;

				*textPtr += st.hasDefender() ? QStringLiteral("D:[%1] ").arg(st.defender()) :
											   QStringLiteral("D: %1 ").arg(st.defender());

				*textPtr += st.hasUtility() ? QStringLiteral("U:[%1]").arg(st.utility()) :
											  QStringLiteral("U: %1").arg(st.utility());

				*textPtr += QStringLiteral("\n");
			}

			*textPtr += QStringLiteral(" \n");
		}

		full.players().emplace_back(std::move(pl));
	}

	full.flags().setFlag(RpgStream::FullState::Player);




	for (auto e : m_registry.view<Mp>()) {
		const Mp &mp = m_registry.get<Mp>(e);

		RpgStream::MpData d;
		d.setTagId(mp.idTag);
		d.setPosXAsFloat(mp.pos.x);
		d.setPosYAsFloat(mp.pos.y);
		d.setOrigXAsFloat(mp.origin.x);
		d.setOrigYAsFloat(mp.origin.y);

		full.mps().emplace_back(std::move(d));
	}

	full.flags().setFlag(RpgStream::FullState::Mp);



	if (textPtr) {
		*textPtr += QStringLiteral(" \nMP COUNT: %1\n").arg(full.mps().size());

		*textPtr += QStringLiteral(" \nTOWERS\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");
	}



	for (auto e : m_registry.view<Tower, TowerStateOutput>()) {
		const auto &[tower, output] = m_registry.get<Tower, TowerStateOutput>(e);

		if (!tower.active) {
			if (textPtr) {
				*textPtr += QStringLiteral("Tower %1 - inactive\n").arg(tower.idTag);
			}

			continue;
		}

		const RpgStream::TowerState *last = output.last();

		if (!last) {
			ELOG_ERROR << "Internal error";
			continue;
		}

		RpgStream::TowerState st = *last;

		st.setTagId(tower.idTag);


		if (textPtr) {
			*textPtr += QStringLiteral("Tower %1 (T%2 %3) %4")
						.arg(tower.idTag)
						.arg(st.team())
						.arg(st.load(), 3)
						.arg(st.active() ? QStringLiteral("[*]") : QStringLiteral("[ ]"))
						;

			if (st.multiply() > 0)
				*textPtr += QStringLiteral(" x%1").arg(1+st.multiply());


			for (const RpgStream::FullMapTag &tag : st.defenders())
				*textPtr += QStringLiteral(" ") + QString::number(tag.tagId());

			*textPtr += QStringLiteral("\n");
		}

		full.towers().emplace_back(std::move(st));
	}

	full.flags().setFlag(RpgStream::FullState::Tower);




	if (textPtr) {
		*textPtr += QStringLiteral(" \nDEFENDERS\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");
	}

	for (auto e : m_registry.view<DefenderObject>()) {
		const auto &[defender, output] = m_registry.get<DefenderObject, DefenderStateOutput>(e);

		const RpgStream::DefenderState *last = output.last();

		if (!last) {
			ELOG_ERROR << "Internal error";
			continue;
		}

		RpgStream::DefenderState st = *last;

		st.setTagId(defender.idTag);
		st.setType(defender.type);

		if (textPtr) {
			*textPtr += QStringLiteral("Defender %1 [%2] T%3 (%4,%5) %6/%7 HP %8\n")
						.arg(defender.idTag)
						.arg(defender.type)
						.arg(defender.team)
						.arg(defender.pos.x)
						.arg(defender.pos.y)
						.arg(st.hp())
						.arg(defender.maxHp)
						.arg(st.visible() ? (st.targetId() > 0 ? QStringLiteral("**") : QStringLiteral("()")) :
											QStringLiteral("__"))
						;

			//*textPtr += QStringLiteral(" \n");
		}

		full.defenders().emplace_back(std::move(st));
	}

	full.flags().setFlag(RpgStream::FullState::Defender);





	if (textPtr) {
		*textPtr += QStringLiteral(" \nNPC\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");
	}

	for (auto e : m_registry.view<Npc, NpcStateOutput>()) {
		const auto &[npc, output] = m_registry.get<Npc, NpcStateOutput>(e);

		RpgStream::NpcStateList pl;
		pl.setTagId(npc.idTag);
		////pl.setIsDeltaMode(true);
		const std::vector<RpgStream::NpcState> &list = output.extract(maxTick);
		///pl.compressStateVector(list);

		pl.setIsDeltaMode(false);
		pl.setState(list);

		if (textPtr) {
			*textPtr += QStringLiteral("Npc %1 %2 (T%3)\n")
						.arg(npc.idTag, 6)
						.arg(npc.data.type(), 2)
						.arg(npc.data.team())
						;

			for (const RpgStream::NpcState &st : list) {
				*textPtr += QStringLiteral("   [%1] %2 HP (%3,%4) | ")
							.arg(st.tick(), 5)
							.arg(st.hp(), 2)
							.arg(st.entityState().posXAsFloat())
							.arg(st.entityState().posYAsFloat())
							;

				*textPtr += d->npcFullState(npc, st);
				*textPtr += QStringLiteral("\n");
			}

			*textPtr += QStringLiteral(" \n");
		}

		full.npcs().emplace_back(std::move(pl));
	}

	full.flags().setFlag(RpgStream::FullState::Npc);






	if (textPtr) {
		*textPtr += QStringLiteral(" \nCONTROLS\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");
	}

	for (auto e : m_registry.view<Control, ControlStateOutput>()) {
		const auto &[control, output] = m_registry.get<Control, ControlStateOutput>(e);

		RpgStream::ControlStateList pl;
		pl.setTagId(control.idTag);
		const std::vector<RpgStream::ControlState> &list = output.extract(maxTick);

		pl.setState(list);

		if (textPtr) {
			*textPtr += QStringLiteral("Control %1 %2 (%3)\n")
						.arg(control.idTag, 6)
						.arg(control.type, 2)
						.arg(control.data)
						;

			for (const RpgStream::ControlState &st : list) {
				*textPtr += QStringLiteral("   [%1] %2 | %3")
							.arg(st.tick(), 5)
							.arg(st.isAlive() ? QStringLiteral("*") : QStringLiteral(" "))
							.arg(st.state())
							;

				/**textPtr += d->npcFullState(npc, st);*/
				*textPtr += QStringLiteral("\n");
			}

			*textPtr += QStringLiteral(" \n");
		}

		full.controls().emplace_back(std::move(pl));
	}

	full.flags().setFlag(RpgStream::FullState::Control);






	if (textPtr) {
		*textPtr += QStringLiteral(" \nUTILITIES\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");

		for (auto e : m_registry.view<Utility>()) {
			const Utility &utility = m_registry.get<Utility>(e);

			*textPtr += QStringLiteral("Utility %1 (T%2) - >%3\n")
						.arg(utility.type)
						.arg(utility.type)
						.arg(utility.destroyAt)
						;

		}
	}




	std::vector<RpgStream::Events> eventList = m_registry.ctx().get<EventsOutput>()
											   .extractAtLeast(lastAuthTick() >= 3 ? lastAuthTick()-3 : 0, maxTick);

	if (textPtr) {
		*textPtr += QStringLiteral(" \nEVENTS\n");
		*textPtr += QStringLiteral("------------------------------------------------------------------\n");

		for (const RpgStream::Events &event : eventList) {
			for (const RpgStream::EventPlayer &e : event.player()) {
				*textPtr += QStringLiteral("   [%1] PLAYER %2 - %3 TARGET: %4 LOCK: %5\n")
							.arg(e.tick(), 5)
							.arg(e.tagId())
							.arg(e.type())
							.arg(e.target())
							.arg(e.lockId())
							;
			}

			for (const RpgStream::EventMpEmitter &e : event.emitter()) {
				*textPtr += QStringLiteral("   [%1] MP %2\n")
							.arg(e.tick(), 5)
							.arg(e.tagId())
							;
			}

			for (const RpgStream::EventStageChanged &e : event.stage()) {
				*textPtr += QStringLiteral("   [%1] STAGE %2 [%3]\n")
							.arg(e.tick(), 5)
							.arg(e.config().stage())
							.arg(e.config().flags().toInt())
							;
			}
		}
	}

	if (!eventList.empty()) {
		full.setEvents(std::move(eventList));
		full.flags().setFlag(RpgStream::FullState::Event);
	}





	return full;
}





/**
 * @brief RpgLogic::fullLoad
 * @param full
 */

void RpgLogic::fullLoad(const RpgStream::Full &full)
{
	if (d->m_lastFullLoadTick >= 0 && (qint64) full.serverTick() <= d->m_lastFullLoadTick)
		return;

	if (full.config().flags() == RpgStream::GameConfig::FlagNull)
		return;


	QMutexLocker locker(&m_mutex);


	m_registry.ctx().get<RpgStream::GameConfig>() = full.config();

	d->playerUpdate(full.players());
	d->mpEmitterUpdate(full.mpEmitters());
	d->towerUpdate(full.towers());
	d->defenderUpdate(full.defenders());
	d->npcUpdate(full.npcs());
	d->controlUpdate(full.controls());


	d->m_lastFullLoadTick = full.serverTick();
}




/**
 * @brief RpgLogic::getFull
 * @return
 */

RpgStream::Full RpgLogic::getFull(QString *textPtr)
{
	RpgStream::Full full;

	QMutexLocker locker(&m_mutex);

	const RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	full.setServerTick(m_serverTick);
	full.setConfig(cfg);


	// Players

	for (auto entity : m_registry.view<Player>()) {
		const Player &p = m_registry.get<Player>(entity);
		full.players().push_back(p.playerData);
	}


	// Towers

	for (auto entity : m_registry.view<Tower>()) {
		const Tower &p = m_registry.get<Tower>(entity);

		RpgStream::Tower stream = p.toRpgStream();

		stream.defenders().reserve(p.defenderList.size());

		for (entt::entity e : p.defenderList) {
			if (!m_registry.valid(e))
				continue;

			Defender *d = m_registry.try_get<Defender>(e);

			if (!d)
				continue;

			stream.defenders().emplace_back(d->toRpgStream());
		}


		full.towers().emplace_back(std::move(stream));
	}


	// MP emitters

	for (auto entity : m_registry.view<MpEmitter>()) {
		const MpEmitter &p = m_registry.get<MpEmitter>(entity);

		full.mpEmitters().emplace_back(p.toRpgStream());
	}



	// DefenderObjects

	for (auto entity : m_registry.view<DefenderObject>()) {
		const DefenderObject &p = m_registry.get<DefenderObject>(entity);

		RpgStream::BaseDefenderObject stream = p.toRpgStream();

		/*if (p.type == RpgStream::BaseDefenderObject::Dummy) {
			if (DefenderDummyObject *dummy = m_registry.try_get<DefenderDummyObject>(entity))
				dummy->toRpgStream(stream);
		}*/

		full.defenders().emplace_back(std::move(stream));
	}


	// Npc

	for (auto entity : m_registry.view<Npc>()) {
		const Npc &p = m_registry.get<Npc>(entity);
		full.npcs().push_back(p.data);
	}


	// Controls

	for (auto entity : m_registry.view<Control>()) {
		const Control &p = m_registry.get<Control>(entity);
		full.controls().push_back(p.toRpgStream());
	}


	full.setFullState(getFullState(1, textPtr));

	return full;
}




/**
 * @brief RpgLogic::initialize
 */

bool RpgLogic::initialize()
{
	QMutexLocker locker(&m_mutex);

	RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	if (!cfg.flags().testFlag(RpgStream::GameConfig::FlagDataCompleted)) {
		ELOG_ERROR << "Incomplete data";
		return false;
	}

	m_registry.ctx().get<RpgStream::GameState>().setHeat(0);

	cfg.setDuration(CFG_GAME_DURATION);

	// Stages, heat initialize

	initializeStages();

	// Map initialize

	d->mapInitialize();

	// Emplace players

	d->emplacePlayers();


	// Initialize NPC

	for (auto e : m_registry.view<Npc>()) {
		d->npcInitialize(e);
	}

	render(true);

	cfg.flags().setFlag(RpgStream::GameConfig::FlagDataPrepared);

	return true;
}





/**
 * @brief RpgLogic::startStageSelect
 * @return
 */

bool RpgLogic::startStageSelect()
{
	QMutexLocker locker(&m_mutex);

	RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	if (!cfg.flags().testFlag(RpgStream::GameConfig::FlagPlaying)) {
		ELOG_ERROR << "Incomplete data";
		return false;
	}

	///d->changeStage(RpgStream::GameConfig::StageSelect);

	if (cfg.stage() >= RpgStream::GameConfig::StageSelect) {
		ELOG_ERROR << "Can't change stage from" << cfg.stage() << "to" << RpgStream::GameConfig::StageSelect;
		return false;
	}

	ELOG_INFO << "Change stage to" << RpgStream::GameConfig::StageSelect;


	// Load quests

	m_registry.ctx().insert_or_assign<QuestList>(getQuestList());


	cfg.setStage(RpgStream::GameConfig::StageSelect);

	return true;
}



/**
 * @brief RpgLogic::increaseHeat
 * @return
 */

bool RpgLogic::increaseHeat()
{
	QMutexLocker locker(&m_mutex);

	RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	if (!cfg.flags().testFlag(RpgStream::GameConfig::FlagPlaying)) {
		ELOG_ERROR << "Invalid game state";
		return false;
	}


	const HeatList &heatList = m_registry.ctx().get<HeatList>();
	RpgStream::GameState &state = m_registry.ctx().get<RpgStream::GameState>();

	quint8 heat = state.heat();

	if (heat >= heatList.size()) {
		ELOG_ERROR << "Can't increase heat, max:" << heatList.size();
		return false;
	}

	d->loadHeat(heat+1);

	return true;
}





/**
 * @brief RpgLogic::selectQuest
 * @param data
 */

void RpgLogic::selectQuest(const RpgStream::QuestSelect &data)
{
	QMutexLocker locker(&m_mutex);

	RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	if (cfg.stage() != RpgStream::GameConfig::StageSelect) {
		ELOG_WARNING << "Invalid stage" << cfg.stage();
		return;
	}

	entt::entity player = entityFromIdTag(data.tagId());

	if (!m_registry.valid(player)) {
		ELOG_WARNING << "Player entity not found" << data.tagId();
		return;
	}

	Player &p = m_registry.get<Player>(player);
	PlayerPrivate &pp = m_registry.get<PlayerPrivate>(player);

	RpgStream::PlayerState &st = d->getEditableCurrentState<RpgStream::PlayerState>(player);



	if (data.defender() == RpgStream::BaseDefenderObject::None) {
		ELOG_WARNING << "Player" << data.tagId() << "didn't select defender";
	} else if (!pp.defenders.contains(data.defender())) {
		ELOG_WARNING << "Player" << data.tagId() << "can't have this defender" << data.defender();
	} else {
		st.setDefender(data.defender());
	}


	if (data.utility() == RpgStream::PlayerConfig::UtilityNone) {
		ELOG_WARNING << "Player" << data.tagId() << "didn't select utility";
	} else if (!pp.utilities.contains(data.utility())) {
		ELOG_WARNING << "Player" << data.tagId() << "can't have this utility" << data.utility();
	} else {
		st.setUtility(data.utility());
	}


	const QuestList &questList = m_registry.ctx().get<QuestList>();

	if (data.quest() < questList.size()) {
		p.playerData.setQuest(questList.at(data.quest()));
		pp.requiredStreak = questList.at(data.quest()).streak();
	} else {
		ELOG_WARNING << "Player" << data.tagId() << "select invalid quest" << data.quest();
	}

	ELOG_DEBUG << "Player" << data.tagId() << "select quest completed" << data.quest();
}



/**
 * @brief RpgLogic::render
 */

bool RpgLogic::render(const bool &first)
{
	QMutexLocker locker(&m_mutex);

	d->m_requireFull = false;

	if (!first) {
		++m_serverTick;

		RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

		if (cfg.stage() < RpgStream::GameConfig::StageWarmingUp) {
			ELOG_DEBUG << "Set StageWarmingUp at" << m_serverTick;

			d->autoSelectInventory();
			d->changeStage(RpgStream::GameConfig::StageWarmingUp);

			if (d->m_oldStage) {
				rewindStage(d->m_oldStage.value());
				d->m_oldStage = std::nullopt;
			}
		}

		if (m_serverTick <= m_lastAuthTickDiff)
			return false;
	} else {
		if (m_serverTick > 0) {
			ELOG_ERROR << "Invalid first render on tick" << m_serverTick;
			return false;
		}
	}


	d->preRenderDefenders();
	d->preRenderUtilities();
	d->preRenderEvents();
	d->renderEvents();
	d->renderEntityKnockbacks();
	d->renderPlayerInputs(first);
	d->renderNpcInputs(first);
	d->renderFinal();

	d->storeRealEvents();

	d->removeDeleteTags();

	if (d->m_oldStage) {
		rewindStage(d->m_oldStage.value());
		d->m_oldStage = std::nullopt;
	}

	return d->m_requireFull;
}



/**
 * @brief RpgLogic::renderStageSelect
 */

void RpgLogic::renderStageSelect()
{
	QMutexLocker locker(&m_mutex);

	const RpgStream::GameConfig &cfg = m_registry.ctx().get<RpgStream::GameConfig>();

	if (cfg.stage() != RpgStream::GameConfig::StageSelect) {
		ELOG_ERROR << "Invalid stage";
		return;
	}


	d->storeRealEvents();

	d->removeDeleteTags();
}




/**
 * @brief RpgLogic::renderUpdate
 */

void RpgLogic::renderUpdate()
{
	// Ezt csak a client logic-ban használjuk: a full state load után törli a delete tag-eket


	QMutexLocker locker(&m_mutex);


	d->removeDeleteTags();
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
		} else if (q->m_registry.all_of<RpgStream::EventNpc>(e)) {
			preRenderEventNpc(e);
		} else if (q->m_registry.all_of<EventMpCreate>(e)) {
			preRenderEventMpCreate(e);
		} else if (q->m_registry.all_of<EventNpcCreate>(e)) {
			preRenderEventNpcCreate(e);
		} else if (q->m_registry.all_of<EventControlStateChange>(e)) {
			preRenderEventControlStateChange(e);
		} else if (q->m_registry.all_of<EventDefenderDestroy>(e)) {
			preRenderEventDefenderDestroy(e);
		} else if (EventPlayerRespawn *ev = q->m_registry.try_get<EventPlayerRespawn>(e)) {
			playerInitialize(ev->player);
		} else if (EventStageChange *event = q->m_registry.try_get<EventStageChange>(e)) {
			changeStage(event->stage);
		} else {
			q->m_registry.emplace<EventProcessingTag>(e);

			continue;			// Nem teszünk DeleteTag-et, majd a renderEvents();
		}

		q->m_registry.emplace<DeleteTag>(e);
	}


	// Remove penalties

	for (auto e : q->m_registry.view<PenaltyTag>()) {
		if (q->m_registry.get<PenaltyTag>(e).expire < tick) {
			q->m_registry.remove<PenaltyTag>(e);
		}
	}


	// Remove outdated locks

	for (auto e : q->m_registry.view<Player, LockTag>()) {
		if (q->m_registry.get<LockTag>(e).expire < tick) {
			playerUnlock(e, CFG_PENALTY_AUTO_UNLOCK);
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

	if (ch.chunkSize.width() > 0)
		ch.gridWidth = std::floor(ch.viewport.width() / ch.chunkSize.width());

	if (ch.chunkSize.height() > 0)
		ch.gridHeight = std::floor(ch.viewport.height() / ch.chunkSize.height());

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
	e.active = stream.active();

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
	stream.setActive(active);

	return stream;
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
	e.pos.x = stream.posXAsFloat();
	e.pos.y = stream.posYAsFloat();
	e.active = stream.active();

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
	stream.setPosXAsFloat(pos.x);
	stream.setPosYAsFloat(pos.y);
	stream.setActive(active);

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

void PlayerPrivate::load(const RpgStream::PlayerConfig &cfg, RpgStream::PlayerData &dst, const bool &isHosted)
{
	if (!isHosted) {
		power = cfg.power();
		push = cfg.entity().push();
		pushDist = cfg.entity().pushDist();
		resist = cfg.entity().resist();

		maxHp = cfg.entity().maxHp();
		maxMp = cfg.maxMp();
		maxBullet = cfg.maxBullet();

		towerPlus = cfg.towerPlus();
		towerMinus = cfg.towerMinus();

		for (const RpgStream::BaseDefenderObject::Type &d : cfg.defenders())
			defenders.insert(d);

		for (const RpgStream::PlayerConfig::Utility &d : cfg.utilities())
			utilities.insert(d);
	} else {
		power = std::max((quint8) 1, cfg.power());

		const CfgPowerLevel pData = CfgPowerLevel::fromPlayerConfig(cfg);

		push = pData.push;
		pushDist = pData.pushDist;
		resist = pData.pushRest;

		maxHp = pData.hp;
		maxMp = pData.mp;
		maxBullet = pData.bullet;

		towerPlus = pData.towerPlus;
		towerMinus = pData.towerMinus;


		// Defenders

		for (const RpgStream::BaseDefenderObject::Type &d : cfg.defenders()) {
			if (defenders.size() >= pData.defenderCount)
				break;

			defenders.insert(d);
		}


		// Utilites

		for (const RpgStream::PlayerConfig::Utility &d : cfg.utilities()) {
			if (utilities.size() >= pData.utilityCount)
				break;

			utilities.insert(d);
		}

		// Streak

		modSkipLock = pData.skipLock;
		penaltyMsec = pData.penalty * 60.;

	}

	dst.config().setPower(power);
	dst.config().setEntity(this->toEntityConfig());
	dst.config().setMaxBullet(maxBullet);
	dst.config().setMaxMp(maxMp);
	dst.config().utilities().assign(utilities.cbegin(), utilities.cend());
	dst.config().defenders().assign(defenders.cbegin(), defenders.cend());
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
	cfg.setMaxHp(maxHp);

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




/**
 * @brief DefenderObject::fromRpgStream
 * @param stream
 * @return
 */

DefenderObject DefenderObject::fromRpgStream(const RpgStream::BaseDefenderObject &stream)
{
	DefenderObject obj;

	obj.idTag = stream.tagId();
	obj.type = stream.type();
	obj.team = stream.team();
	obj.maxHp = stream.maxHp();
	obj.pos = cpv(stream.posXAsFloat(), stream.posYAsFloat());

	return obj;
}


/**
 * @brief DefenderObject::toRpgStream
 * @return
 */

RpgStream::BaseDefenderObject DefenderObject::toRpgStream() const
{
	RpgStream::BaseDefenderObject stream;

	stream.setTagId(idTag);
	stream.setType(type);
	stream.setTeam(team);
	stream.setMaxHp(maxHp);
	stream.setPosXAsFloat(pos.x);
	stream.setPosYAsFloat(pos.y);

	return stream;
}


/**
 * @brief DefenderObject::isNear
 * @param pos
 * @return
 */

bool DefenderObject::isNear(const cpVect &pos) const
{
	return cpvdistsq(this->pos, pos) <= radius*radius;
}


/**
 * @brief DefenderObject::fromDefenderConfigBase
 * @param cfg
 */

void DefenderObject::fromDefenderConfigBase(const CfgDefenderBase &cfg)
{
	maxHp = cfg.maxHp;
	radius = cfg.radius;
	repeaterDelay = cfg.repeaterDelay;
	actionsToHpLoss = cfg.actionsToHpLoss;
}



/**
 * @brief DefenderDummyObject::fromRpgStream
 * @param stream
 * @return
 */

/*DefenderDummyObject DefenderDummyObject::fromRpgStream(const RpgStream::BaseDefenderObject &stream)
{
	DefenderDummyObject obj;

	obj.dummy = stream.dummy();

	return obj;
}


void DefenderDummyObject::toRpgStream(RpgStream::BaseDefenderObject &stream) const
{
	stream.setDummy(dummy);
}
*/





/**
 * @brief EventMpCreate::createMp
 * @param stage
 * @return
 */

EventMpCreate EventMpCreate::createMp(const RpgStream::GameConfig::Stage &stage, const quint32 &tickNow)
{
	EventMpCreate ev;

	switch (stage) {
		case RpgStream::GameConfig::StageWarmingUp:
			ev.capacityRatio = CFG_EMITTER_CAPACITY_STAGE_WU;
			ev.setTick(tickNow + CFG_EMITTER_DELAY_STAGE_WU);
			break;

		case RpgStream::GameConfig::StageMain:
			ev.capacityRatio = CFG_EMITTER_CAPACITY_STAGE_M;
			ev.setTick(tickNow + CFG_EMITTER_DELAY_STAGE_M);
			break;

		case RpgStream::GameConfig::StageLast:
			ev.capacityRatio = CFG_EMITTER_CAPACITY_STAGE_L;
			ev.setTick(tickNow + CFG_EMITTER_DELAY_STAGE_L);
			break;

		case RpgStream::GameConfig::StageInit:
		case RpgStream::GameConfig::StageSelect:
		case RpgStream::GameConfig::StageFinished:
			ev.capacityRatio = 0.;
			ev.setTick(tickNow);
			break;
	}

	return ev;
}















/**
 * @brief RpgLogicPrivate::controlAdd
 * @param data
 * @param tagIdPtr
 * @return
 */

template<class T>
entt::entity RpgLogicPrivate::controlAdd(const RpgStream::ControlData &data, const T &controlData, quint32 *tagIdPtr)
{
	QMutexLocker locker(&q->m_mutex);

	auto entity = q->m_registry.create();

	Control &control = q->m_registry.emplace<Control>(entity);
	q->m_registry.emplace<T>(entity, controlData);

	control.idTag = nextIdTag(IdControl);
	control.type = data.type();
	control.pos.x = data.posXAsFloat();
	control.pos.y = data.posYAsFloat();
	control.data = data.data();

	q->entitySetIdTag(entity, control.idTag);
	q->m_registry.emplace<ControlStateOutput>(entity);

	if (tagIdPtr)
		*tagIdPtr = control.idTag;

	return entity;

}


/**
 * @brief Control::toRpgStream
 * @return
 */

RpgStream::ControlData Control::toRpgStream() const
{
	RpgStream::ControlData stream;

	stream.setTagId(idTag);
	stream.setType(type);
	stream.setPosXAsFloat(pos.x);
	stream.setPosYAsFloat(pos.y);
	stream.setData(data);

	return stream;
}





}		// end namespace




/**
 * @brief RpgUserData::setQuests
 * @param list
 */

void RpgUserData::setQuests(const std::vector<RpgStream::Quest> &list)
{
	quests.clear();
	quests.reserve(list.size());

	for (const RpgStream::Quest &q : list) {
		RpgQuestData d;
		d.question = q.question();
		d.streak = q.streak();
		d.pts = q.pts();
		d.xp = q.xp();
		d.token = q.token();
		quests.emplaceBack(std::move(d));
	}
}


/**
 * @brief RpgUserData::getQuests
 * @return
 */

std::vector<RpgStream::Quest> RpgUserData::getQuests() const
{
	std::vector<RpgStream::Quest> list;

	list.reserve(quests.size());

	for (const RpgQuestData &q : quests) {
		RpgStream::Quest d;
		d.setQuestion(q.question);
		d.setStreak(q.streak);
		d.setPts(q.pts);
		d.setXp(q.xp);
		d.setToken(q.token);
		list.emplace_back(std::move(d));
	}

	return list;
}


