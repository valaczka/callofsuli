/*
 * ---- Call of Suli ----
 *
 * rpglogicserver.cpp
 *
 * Created on: 2026. 06. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgLogicServer
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

#include "rpglogicserver.h"
#include "rpgengine.h"
#include "serverservice.h"

#define LAST_AUTH_DIFF				6


RpgLogicServer::RpgLogicServer(RpgEngine *engine)
	: Rpg::RpgLogic(LAST_AUTH_DIFF)
	, m_engine(engine)
{

}


/**
 * @brief RpgLogicServer::getRenderedState
 * @return
 */

RpgStream::Full RpgLogicServer::getRenderedState(const bool &requireFull)
{
	RpgStream::Full f;

#ifdef WITH_FTXUI
	QString txt;

	if (requireFull) {
		f = getFull(&txt);
	} else {
		f.setFullState(getFullState(SEND_STATE_COUNT, &txt));
	}
#else
	if (requireFull) {
		f = getFull();
	} else {
		f.setFullState(getFullState(SEND_STATE_COUNT));
	}
#endif


	if (requireFull)
		m_engine->addMapTagsToStream(f);



#ifdef WITH_FTXUI
	QCborMap m;
	m.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	m.insert(QStringLiteral("txt"), txt);
	m_engine->udpServer()->service()->writeToSocket(m.toCborValue());
#endif

	return f;
}





/**
 * @brief RpgLogicServer::eventRealized
 * @param entity
 */

void RpgLogicServer::eventRealized(entt::entity entity)
{
	QMutexLocker locker(&m_mutex);

	if (RpgStream::EventStageChanged *ev = m_registry.try_get<RpgStream::EventStageChanged>(entity)) {
		ELOG_DEBUG << "Stage changed to" << ev->config().stage();

		if (!onStageChanged(ev->config().stage()))
			eventRealizedDefault(entity);

		return;
	}

	if (m_registry.try_get<Rpg::EventMpEmitterEmpty>(entity))
		eventRealizedDefault(entity);
}



/**
 * @brief RpgLogicServer::onNpcCreated
 * @param entity
 * @param idTag
 * @param player
 */

void RpgLogicServer::onNpcCreated(entt::entity entity, const quint32 &idTag, Rpg::Player *player)
{
	Q_UNUSED(entity);

	quint32 pid = m_engine->addTagToPlayer(idTag, player);

	ELOG_DEBUG << "NPC" << idTag << "owner set to player" << pid;
}



/**
 * @brief RpgLogicServer::initializeChests
 * @return
 */

std::vector<Rpg::Chest> RpgLogicServer::initializeChests()
{
	const int numChests = m_registry.ctx().get<Rpg::HeatList>().size();

	if (numChests <= 1) {
		ELOG_INFO << "Not enough heat, skip chests";
		return {};
	};

	int numTowers = 0;

	for (entt::entity e : m_registry.view<Rpg::Tower>()) {
		if (m_registry.get<Rpg::Tower>(e).active)
			++numTowers;
	}

	if (numTowers < 1) {
		ELOG_WARNING << "No tower";
		return {};
	};

	static constexpr int maxPoint =
			(((CFG_GAME_STAGE_LAST)/60. * CFG_POINT_STAGE_L)
			 +((CFG_GAME_DURATION-(CFG_GAME_STAGE_LAST))/60. * CFG_POINT_STAGE_L)) * CFG_MAX_POINT_FACTOR;

	const int step = maxPoint * numTowers / numChests;

	m_heatSteps.clear();

	for (int i=1; i<numChests; ++i) {
		m_heatSteps[step*i] = i;
	}

	return std::vector<Rpg::Chest>{};
}



/**
 * @brief RpgLogicServer::checkState
 * @param state
 */

void RpgLogicServer::checkState(const RpgStream::GameState &state)
{
	if (m_heatSteps.empty())
		return;

	int pts = state.ptsA() + state.ptsB();

	auto it = m_heatSteps.upper_bound(pts);

	if (it == m_heatSteps.begin())
		return;

	std::advance(it, -1);

	if (state.heat() < it->second) {
		increaseHeat();
	}
}



/**
 * @brief RpgLogicServer::getQuestList
 * @return
 */

Rpg::QuestList RpgLogicServer::getQuestList() const
{
	Rpg::QuestList list;

	static const std::vector<std::array<int, 5> > data = {
		{ 4,	3,	200,	1540,	215 },
		{ 6,	4,	650,	2580,	325 },
		{ 13,	6,	900,	3540,	415 },
	};


	for (const auto &a : data) {
		RpgStream::Quest q;
		q.setQuestion(a.at(0));
		q.setStreak(a.at(1));
		q.setPts(0/*a.at(2)*/);

		q.setXp(a.at(3));
		q.setToken(a.at(4));

		list.emplace_back(std::move(q));
	}

	return list;
}





/**
 * @brief RpgLogicServer::getResult
 * @return
 */

RpgStream::Result RpgLogicServer::getResult()
{
	QMutexLocker locker(&m_mutex);

	const RpgStream::GameState &state = m_registry.ctx().get<RpgStream::GameState>();

	RpgStream::Team winnerTeam = RpgStream::TeamNone;

	if (state.ptsA() > state.ptsB())
		winnerTeam = RpgStream::TeamA;
	else if (state.ptsA() < state.ptsB())
		winnerTeam = RpgStream::TeamB;
	else {
		// Pontegyenlőség esetén
		//
		// 1. a helyes válaszok száma dönt
		// 2. a több streak dönt
		// 3. a maradék MP dönt
		// 4. random döntünk

		int questionA = 0, questionB = 0;
		int streakA = 0, streakB = 0;
		int mpA = 0, mpB = 0;


		for (auto e : m_registry.view<Rpg::Player>()) {
			const Rpg::Player &p = m_registry.get<Rpg::Player>(e);
			const RpgStream::PlayerState *last = getLastState<RpgStream::PlayerState>(e);

			if (!last) {
				ELOG_ERROR << "Invalid PlayerState" << p.playerData.playerId();
				continue;
			}

			if (p.playerData.team() == RpgStream::TeamA) {
				questionA += last->question();
				streakA += last->streak();
				mpA += last->mp();
			} else {
				questionB += last->question();
				streakB += last->streak();
				mpB += last->mp();
			}
		}

		if (questionA > questionB)
			winnerTeam = RpgStream::TeamA;
		else if (questionA < questionB)
			winnerTeam = RpgStream::TeamB;
		else {
			if (streakA > streakB)
				winnerTeam = RpgStream::TeamA;
			else if (streakA < streakB)
				winnerTeam = RpgStream::TeamB;
			else {
				if (mpA > mpB)
					winnerTeam = RpgStream::TeamA;
				else if (mpA < mpB)
					winnerTeam = RpgStream::TeamB;
				else {
					std::bernoulli_distribution dist(0.5);

					if (dist(m_rnd))
						winnerTeam = RpgStream::TeamA;
					else
						winnerTeam = RpgStream::TeamB;
				}
			}
		}
	}


	RpgStream::Result res = getResultByTeam(winnerTeam);

	res.setTeam(winnerTeam);

	ELOG_TRACE << "[RESULT] winner team:" << winnerTeam << state.ptsA() << state.ptsB();

	return res;
}



/**
 * @brief RpgLogicServer::onStageChanged
 * @param stage
 */

bool RpgLogicServer::onStageChanged(const RpgStream::GameConfig::Stage &stage)
{
	if (stage == RpgStream::GameConfig::StageWarmingUp)	{
		ELOG_DEBUG << "Stage: warming up";
	}

	// run default

	return false;
}


/**
 * @brief RpgLogicServer::_logger
 * @return
 */

Logger *RpgLogicServer::_logger() const
{
	Q_ASSERT (m_engine);
	return m_engine->_logger();
}



