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
#include "rpgplayer.h"
#include "tiledobject.h"

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
									 ::TiledObjectBody::vectorFromAngle(angle,
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
 * @brief RpgLogicClient::addNpc
 * @param data
 * @param owner
 * @return
 */

entt::entity RpgLogicClient::addNpc(const RpgStream::NpcData &data, entt::entity owner)
{
	return npcAdd(data, owner);
}











/**
 * @brief RpgLogicClientSingle::RpgLogicClientSingle
 */

RpgLogicClientSingle::RpgLogicClientSingle(RpgGame *game)
	: RpgLogicClient(0, 0)
	, m_game(game)
{
	Q_ASSERT(m_game);

	QObject::connect(m_game, &RpgGame::heatChanged, m_game, [this]() {
		m_game->gameItem()->messageColor(QObject::tr("Heat upgraded: %1").arg(m_game->heat()),
										 QColorConstants::Svg::orangered);
	});
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

	// Load quests

	m_registry.ctx().insert_or_assign<QuestList>(getQuestList());


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

	// Rewind stage (workaround)

	RpgStream::GameConfig real = *cfg;

	cfg->setStage(RpgStream::GameConfig::StageSelect);

	return real;
}



/**
 * @brief RpgLogicClientSingle::overrideMapData
 * @param data
 */

void RpgLogicClientSingle::overrideMapData(RpgStream::MapData &data)
{
	Q_UNUSED(data);
}



/**
 * @brief RpgLogicClientSingle::getResult
 * @return
 */

RpgStream::Result RpgLogicClientSingle::getResult()
{
	return getResultByTeam(RpgStream::TeamNone);
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
				if (!scope.get<MpEmitter>(e).active)
					continue;

				EventMpCreate evc = EventMpCreate::createMp(RpgStream::GameConfig::StageMain, tick);
				evc.emitter = e;

				ELOG_DEBUG << "Register WarmingUp MP create event for" << evc.tick();

				eventStore(std::move(evc));
			}

		} else if (ev->config().stage() == RpgStream::GameConfig::StageLast) {
			// Disable chests

			for (auto e : scope.view<Control>()) {
				const Control &c = scope.get<Control>(e);
				if (c.type != RpgStream::ControlData::Chest)
					continue;

				const RpgStream::ControlState *st = getCurrentState<RpgStream::ControlState>(e);

				if (!st || !st->isAlive())
					continue;

				EventControlStateChange ev;
				ev.setTick(tick);
				ev.control = e;
				ev.isAlive = false;
				ev.state = Chest::StateDisabled;

				ELOG_DEBUG << "Register chest disable event for" << c.idTag << "at" << ev.tick();

				eventStore(std::move(ev));
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
 * @brief RpgLogicClientSingle::rewindStage
 * @param oldStage
 */

void RpgLogicClientSingle::rewindStage(const RpgStream::GameConfig::Stage &oldStage)
{
	if (oldStage <= RpgStream::GameConfig::StageSelect)
		return;

	RpgLogicScope scope = getScope();

	scope.getCtx<RpgStream::GameConfig>()->setStage(oldStage);
}





/**
 * @brief RpgLogicClientSingle::getQuestList
 * @return
 */

QuestList RpgLogicClientSingle::getQuestList() const
{
	QuestList list;

	static const std::vector<std::array<int, 5> > data = {
		{ 4,	3,	200,	540,	15 },
		{ 6,	4,	650,	580,	25 },
		{ 13,	6,	900,	1540,	215 },
	};


	for (const auto &a : data) {
		RpgStream::Quest q;
		q.setQuestion(a.at(0));
		q.setStreak(a.at(1));
		q.setPts(a.at(2));

		q.setXp(a.at(3));
		q.setToken(a.at(4));

		list.emplace_back(std::move(q));
	}

	return list;
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
		ELOG_ERROR << "Missing engine";
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
		ELOG_ERROR << "Missing engine";
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

	if (full.flags().testFlag(RpgStream::FullState::Npc))
		loadNpc(full.npcs());

	if (full.flags().testFlag(RpgStream::FullState::Control))
		loadControls(full.controls());
}








/**
 * @brief RpgLogicClientMulti::loadResult
 * @param result
 */

void RpgLogicClientMulti::loadResult(RpgStream::Result &&result)
{
	Rpg::RpgLogicScope scope = getScope();

	m_registry.ctx().insert_or_assign<RpgStream::Result>(std::move(result));

	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	cfg->flags().setFlag(RpgStream::GameConfig::FlagFinished);
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
			ELOG_ERROR << "Invalid player" << s.tagId();
			continue;
		}

		PlayerStateOutput *out = scope.try_get<PlayerStateOutput>(player);

		if (!out) {
			ELOG_ERROR << "Invalid player" << s.tagId();
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
			ELOG_ERROR << "Invalid tower" << s.tagId();
			continue;
		}

		TowerStateOutput *out = scope.try_get<TowerStateOutput>(tower) ;

		if (!out) {
			ELOG_ERROR << "Invalid tower" << s.tagId();
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
			ELOG_ERROR << "Invalid defender" << s.tagId();
			continue;
		}

		DefenderStateOutput *out = scope.try_get<DefenderStateOutput>(defender);

		if (!out) {
			ELOG_ERROR << "Invalid defender" << s.tagId();
			continue;
		}

		out->insert(s);
	}
}




/**
 * @brief RpgLogicClientMulti::loadNpc
 * @param list
 */

void RpgLogicClientMulti::loadNpc(const std::vector<RpgStream::NpcStateList> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	IdTagMapper *mapper = scope.getCtx<IdTagMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::NpcStateList &s : list) {
		entt::entity npc = mapper->get(s.tagId());

		if (!scope.valid(npc)) {
			ELOG_ERROR << "Invalid NPC" << s.tagId();
			continue;
		}

		NpcStateOutput *out = scope.try_get<NpcStateOutput>(npc);

		if (!out) {
			ELOG_ERROR << "Invalid NPC" << s.tagId();
			continue;
		}

		for (const RpgStream::NpcState &state : s.state()) {
			out->insert(state);
		}
	}
}




/**
 * @brief RpgLogicClientMulti::loadControls
 * @param list
 */

void RpgLogicClientMulti::loadControls(const std::vector<RpgStream::ControlStateList> &list)
{
	Rpg::RpgLogicScope scope = getScope();

	IdTagMapper *mapper = scope.getCtx<IdTagMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::ControlStateList &s : list) {
		entt::entity control = mapper->get(s.tagId());

		if (!scope.valid(control)) {
			ELOG_ERROR << "Invalid control" << s.tagId();
			continue;
		}

		ControlStateOutput *out = scope.try_get<ControlStateOutput>(control);

		if (!out) {
			ELOG_ERROR << "Invalid control" << s.tagId();
			continue;
		}

		for (const RpgStream::ControlState &state : s.state()) {
			out->insert(state);
		}
	}
}
















/////////////////////////////////////////////
/// TUTORIAL
/////////////////////////////////////////////


template<typename T, typename T2>
bool RpgLogicClientTutorial::compareEvent(const RpgLogicScope &scope, entt::entity entity, const RpgStream::BaseTickState *state) {
	const T* ev = scope.try_get<T>(entity);
	if (!ev)
		return false;

	const T* s = dynamic_cast<const T*>(state);

	if (!s)
		return false;

	return compareEvent(*s, *ev);
}




template<typename T, typename T2>
bool RpgLogicClientTutorial::compareEvent(const T &, const T &) {
	ELOG_WARNING << "Missing implementation";
	return false;
}















/**
 * @brief RpgLogicClientTutorial::RpgLogicClientTutorial
 */

RpgLogicClientTutorial::RpgLogicClientTutorial(RpgGame *game, std::unique_ptr<Tutorial> tutorial)
	: RpgLogicClientSingle(game)
	, m_tutorial(std::move(tutorial))
{
	Q_ASSERT(m_game);

	m_messageTimer.setInterval(15000);
	QObject::connect(&m_messageTimer, &QTimer::timeout, m_game, [this]() { onTimerTimeout(); });
	QObject::connect(m_game, &RpgGame::controlledPlayerChanged, m_game, [this]() { onControlledPlayerChanged(); });
}


/**
 * @brief RpgLogicClientTutorial::~RpgLogicClientTutorial
 */

RpgLogicClientTutorial::~RpgLogicClientTutorial()
{

}





/**
 * @brief RpgLogicClientTutorial::overrideMapData
 * @param data
 */

void RpgLogicClientTutorial::overrideMapData(RpgStream::MapData &data)
{
	data.heat().clear();
}



/**
 * @brief RpgLogicClientTutorial::loadGameData
 * @param dest
 * @return
 */

bool RpgLogicClientTutorial::loadGameData(RpgStream::CharacterSelectClient *dest)
{
	Q_ASSERT(dest);

	if (!m_tutorial) {
		ELOG_ERROR << "Missing tutorial";
		return false;
	}

	RpgPlayerDefinition def = RpgGame::characters().value(m_tutorial->character);

	if (def.name.isEmpty()) {
		ELOG_ERROR << "Invalid character" << m_tutorial->character;
		return false;
	}

	dest->data().setConfig(def.toPlayerConfig());
	dest->data().config().setPower(m_tutorial->power);
	dest->data().setCharacterResolved(m_tutorial->character);

	//m_characterSelect.data().setNickName(data.value(QStringLiteral("nickname")).toString().toUtf8());

	dest->gameConfig().setTerrainResolved(m_tutorial->terrain);

	return true;
}




/**
 * @brief RpgLogicClientTutorial::initialize
 * @return
 */

void RpgLogicClientTutorial::initialize()
{
	if (m_tutorial->fnInit)
		m_tutorial->fnInit(this);
}


/**
 * @brief RpgLogicClientTutorial::player
 * @return
 */

RpgPlayer *RpgLogicClientTutorial::player() const
{
	return m_game->controlledPlayer();
}


/**
 * @brief RpgLogicClientTutorial::npcAddToPoint
 * @param character
 * @param entryPoint
 * @param num
 * @param delay
 */

void RpgLogicClientTutorial::npcAddToPoint(const QString &character, const QStringList &entryPoint, const int &num, const int &delay)
{
	const auto &ptr = RpgGame::readNpcDefinition(character);

	if (!ptr) {
		LOG_CERROR("game") << "Invalid NPC type" << character;
		return;
	}

	RpgStream::HeatNpc n;

	n.setData(ptr->toNpcData());

	n.data().setCharacterResolved(character);

	n.setNum(num);
	n.setDelay(delay);

	for (const QString &entry : entryPoint) {
		const auto pos = m_game->entryPoint(entry);

		if (!pos) {
			LOG_CERROR("game") << "Invalid entry point" << entry;
			continue;
		}

		RpgStream::PlayerPosition p;
		p.setPosXAsFloat(pos->x());
		p.setPosYAsFloat(pos->y());

		n.positionList().emplace_back(std::move(p));
	}

	this->npcAdd(n);
}


/**
 * @brief RpgLogicClientTutorial::eventRealized
 * @param entity
 */

void RpgLogicClientTutorial::eventRealized(entt::entity entity)
{
	/*RpgLogicScope scope = getScope();

	if (!scope.valid(entity))
		return;

	if (RpgStream::EventStageChanged *ev = scope.try_get<RpgStream::EventStageChanged>(entity)) {
		if (ev->config().stage() == RpgStream::GameConfig::StageWarmingUp) {
			stepForward();
		}
	}

	if (!m_currentStep)
		return;*/

	checkEvent(entity);
}



/**
 * @brief RpgLogicClientTutorial::onTargetEntityChanged
 */

void RpgLogicClientTutorial::onTargetEntityChanged()
{

}


/**
 * @brief RpgLogicClientTutorial::onTargetControlChanged
 */

void RpgLogicClientTutorial::onTargetControlChanged()
{
	TiledObjectBody *obj = m_game->controlledPlayer()->targetControl();

	if (!m_currentStep)
		return;

	if (m_currentStep->inputEvents.empty()) {
		ELOG_DEBUG << "All input completed";
		stepForward();
		return;
	}

	for (auto it = m_currentStep->inputEvents.cbegin(); it != m_currentStep->inputEvents.cend(); ) {
		const EventTargetControlChanged *t = dynamic_cast<const EventTargetControlChanged*>(it->get());

		if (t && t->fnCmp && t->fnCmp(obj)) {
			it = m_currentStep->inputEvents.erase(it);
			continue;
		}

		++it;
	}

	if (m_currentStep->inputEvents.empty()) {
		ELOG_DEBUG << "All input completed";
		stepForward();
	}
}



/**
 * @brief RpgLogicClientTutorial::initializeTowers
 * @return
 */

std::unordered_set<entt::entity> RpgLogicClientTutorial::initializeTowers()
{
	if (!m_tutorial || !m_tutorial->towers)
		return RpgLogic::initializeTowers();

	std::unordered_set<entt::entity> r;

	if (m_tutorial->towers->empty())
		return r;

	RpgLogicScope scope = getScope();

	for (entt::entity e : scope.view<Tower>()) {
		if (m_tutorial->towers->contains(scope.get<Tower>(e).idTag))
			r.insert(e);
	}

	return r;
}


/**
 * @brief RpgLogicClientTutorial::initializeChests
 * @return
 */

std::vector<Chest> RpgLogicClientTutorial::initializeChests()
{
	if (!m_tutorial || !m_tutorial->chests)
		return RpgLogic::initializeChests();

	std::vector<Chest> r;

	if (m_tutorial->chests->empty())
		return r;

	for (const QString &e : m_tutorial->chests.value()) {
		auto ptr = m_game->entryPoint(e);

		if (ptr) {
			Chest c;
			c.pos.x = ptr->x();
			c.pos.y = ptr->y();
			r.emplace_back(std::move(c));
		}
	}

	return r;
}



/**
 * @brief RpgLogicClientTutorial::getQuestList
 * @return
 */

QuestList RpgLogicClientTutorial::getQuestList() const
{
	if (m_tutorial)
		return m_tutorial->questList;
	else
		return {};
}




/**
 * @brief RpgLogicClientTutorial::initializeEmitters
 * @return
 */

std::unordered_set<entt::entity> RpgLogicClientTutorial::initializeEmitters()
{
	if (!m_tutorial || !m_tutorial->emitters)
		return RpgLogic::initializeEmitters();

	std::unordered_set<entt::entity> r;

	if (m_tutorial->emitters->empty())
		return r;

	RpgLogicScope scope = getScope();

	for (entt::entity e : scope.view<MpEmitter>()) {
		if (m_tutorial->emitters->contains(scope.get<MpEmitter>(e).idTag))
			r.insert(e);
	}

	return r;
}


/**
 * @brief RpgLogicClientTutorial::stepForward
 * @return
 */

int RpgLogicClientTutorial::stepForward()
{
	if (!m_tutorial)
		return -1;

	if (m_currentStep) {
		if (m_currentStep->fnNext)
			m_currentStep->fnNext(this, lastAuthTick()+1);
	}

	++m_tutorial->currentStep;

	ELOG_DEBUG << "Tutorial next step:" << m_tutorial->currentStep;

	if (m_tutorial->currentStep >= (int) m_tutorial->steps.size()) {
		ELOG_INFO << "Tutorial finished";
		m_currentStep = nullptr;
		onTutorialFinished();
		return -1;
	}

	m_currentStep = &m_tutorial->steps[m_tutorial->currentStep];

	m_messageTimer.start();
	onTimerTimeout();

	return m_tutorial->currentStep;
}



/**
 * @brief RpgLogicClientTutorial::onTimerTimeout
 */

void RpgLogicClientTutorial::onTimerTimeout()
{
	if (!m_currentStep || m_currentStep->message.isEmpty())
		return;

	m_game->gameItem()->message(m_currentStep->message, true);

}



/**
 * @brief RpgLogicClientTutorial::onTutorialFinished
 */

void RpgLogicClientTutorial::onTutorialFinished()
{

}



/**
 * @brief RpgLogicClientTutorial::onControlledPlayerChanged
 */

void RpgLogicClientTutorial::onControlledPlayerChanged()
{
	if (!m_game->controlledPlayer())
		return;

	QObject::connect(m_game->controlledPlayer(), &RpgPlayer::targetEntityChanged, m_game, [this]() { onTargetEntityChanged(); });
	QObject::connect(m_game->controlledPlayer(), &RpgPlayer::targetControlChanged, m_game, [this]() { onTargetControlChanged(); });
}







/**
 * @brief RpgLogicClientTutorial::checkEvent
 * @param entity
 */

void RpgLogicClientTutorial::checkEvent(entt::entity entity)
{
	if (!m_currentStep)
		return;

	if (m_currentStep->inputEvents.empty()) {
		ELOG_INFO << "All input completed";
		stepForward();
		return;
	}

	RpgLogicScope scope = getScope();

	if (!scope.valid(entity))
		return;

	for (auto it = m_currentStep->inputEvents.cbegin(); it != m_currentStep->inputEvents.cend(); ) {
		if (
				compareEvent<RpgStream::EventStageChanged>(scope, entity, it->get())
				) {
			it = m_currentStep->inputEvents.erase(it);
			continue;
		}

		++it;
	}

	if (m_currentStep->inputEvents.empty()) {
		ELOG_INFO << "All input completed";
		stepForward();
	}
}


/**
 * @brief RpgLogicClientTutorial::compareEvent
 * @param step
 * @param event
 * @return
 */

bool RpgLogicClientTutorial::compareEvent(const RpgStream::EventStageChanged &step, const RpgStream::EventStageChanged &event)
{
	return step.config().stage() == event.config().stage();
}



/**
 * @brief RpgLogicClientTutorial::Tutorial::Step::addTargetEntityEvent
 * @param fn
 */

void RpgLogicClientTutorial::Tutorial::Step::addTargetEntityEvent(const std::function<bool (RpgEntity *)> &fn)
{
	auto ev = std::make_unique<EventTargetEntityChanged>();
	ev->fnCmp = fn;
	inputEvents.emplace_back(std::move(ev));
}



/**
 * @brief RpgLogicClientTutorial::Tutorial::Step::addTargetControlEvent
 * @param fn
 */

void RpgLogicClientTutorial::Tutorial::Step::addTargetControlEvent(const std::function<bool (TiledObjectBody *)> &fn)
{
	auto ev = std::make_unique<EventTargetControlChanged>();
	ev->fnCmp = fn;
	inputEvents.emplace_back(std::move(ev));
}



}		// end of namespace
