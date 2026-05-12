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
 * @brief RpgLogic::RpgLogic
 */

RpgLogic::RpgLogic()
{
	registerCtx<RpgStream::PlayerPositionList>();
	registerCtx<RpgStream::GameConfig>();
	registerCtx<ChunkGrid>();
	registerCtx<IdTagMapper>();
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
 * @brief RpgLogic::loadChunkGrid
 * @param grid
 * @return
 */

ChunkGrid &RpgLogic::loadChunkGrid(ChunkGrid &&grid)
{
	QMutexLocker locker(&m_mutex);
	return m_registry.ctx().insert_or_assign<ChunkGrid>(std::move(grid));
}


/**
 * @brief RpgLogic::loadChunkGrid
 * @param grid
 * @return
 */

ChunkGrid &RpgLogic::loadChunkGrid(const RpgStream::ChunkGrid &grid)
{
	QMutexLocker locker(&m_mutex);
	return m_registry.ctx().insert_or_assign<ChunkGrid>(ChunkGrid::fromRpgStream(grid));
}




/**
 * @brief RpgLogic::playerPositionAdd
 * @param pos
 * @param team
 */

void RpgLogic::playerPositionAdd(const QPointF &pos, const TeamTag::Team &team)
{
	RpgLogicScope scope = getScope();
	RpgStream::PlayerPositionList *list = scope.getCtx<RpgStream::PlayerPositionList>();

	Q_ASSERT(list);

	RpgStream::PlayerPosition p;
	p.setPosXAsFloat(pos.x());
	p.setPosYAsFloat(pos.y());
	p.setTeam(team);

	list->list().emplace_back(std::move(p));
}



/**
 * @brief RpgLogic::playerPositionListSet
 * @param list
 */

void RpgLogic::playerPositionListSet(RpgStream::PlayerPositionList &&list)
{
	QMutexLocker locker(&m_mutex);
	m_registry.ctx().insert_or_assign<RpgStream::PlayerPositionList>(std::move(list));
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


	m_registry.emplace<Player>(entity, std::move(playerData), (quint32) 0);
	m_registry.emplace<TeamTag>(entity, team);

	return entity;
}





/**
 * @brief RpgLogic::emplacePlayers - elhelyezzük a játékosokat a kezdőpontjaikra
 * @return
 */

bool RpgLogic::emplacePlayers()
{
	RpgLogicScope scope = getScope();
	RpgStream::PlayerPositionList *list = scope.getCtx<RpgStream::PlayerPositionList>();

	Q_ASSERT(list);

	if (list->list().empty()) {
		LOG_CWARNING("game") << "Empty player position list";
		return false;
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(list->list().begin(), list->list().end(), g);

	QHash<TeamTag::Team, std::vector<RpgStream::PlayerPosition> > map;

	map.insert(TeamTag::TeamNone, {});
	map.insert(TeamTag::TeamA, {});
	map.insert(TeamTag::TeamB, {});

	for (const RpgStream::PlayerPosition &p : list->list()) {
		map[QVariant(p.team()).value<TeamTag::Team>()].emplace_back(p);
	}

	bool success = true;

	auto view = m_registry.view<Player, TeamTag>(entt::exclude<RpgStream::PlayerPosition>);

	for (auto &player : view) {
		const TeamTag &t = m_registry.get<TeamTag>(player);

		std::vector<RpgStream::PlayerPosition> &plist = map[t.team];

		if (plist.empty()) {
			LOG_CERROR("game") << "Not enough player position for" << t.team;
			success = false;
			continue;
		}

		m_registry.emplace<RpgStream::PlayerPosition>(player, plist.back());

		plist.pop_back();
	}


	auto v = m_registry.view<Player>();

	for (auto &player : v) {
		auto p = m_registry.get<Player>(player);

		LOG_CINFO("game") << "***************PLAYER" << p.playerData.playerId() << p.playerData.character();

		if (TeamTag *t = m_registry.try_get<TeamTag>(player)) {
			LOG_CDEBUG("game") << "   - team:" << t->team;
		}

		if (RpgStream::PlayerPosition *p = m_registry.try_get<RpgStream::PlayerPosition>(player)) {
			LOG_CDEBUG("game") << "   - pos:" << p->posXAsFloat() << p->posYAsFloat();
		}
	}


	return success;
}



/**
 * @brief RpgLogic::initializePlayers
 * @return
 */

bool RpgLogic::initializePlayers()
{
	QMutexLocker locker(&m_mutex);

	auto view = m_registry.view<Player>();

	for (auto &player : view) {
		RpgStream::PlayerState &state = m_registry.emplace_or_replace<RpgStream::PlayerState>(player);
		RpgStream::PlayerPosition *p = m_registry.try_get<RpgStream::PlayerPosition>(player);

		state.setHp(12);

		state.entityState().setPosX(p ? p->posX() : 0.f);
		state.entityState().setPosY(p ? p->posY() : 0.f);
		state.entityState().setVelX(0);
		state.entityState().setVelY(0);
	}

	return true;
}


/**
 * @brief RpgLogic::lastAuthTick
 * @return
 */

quint32 RpgLogic::lastAuthTick() const
{
	QMutexLocker locker(&m_mutex);
	return m_lastAuthTick;
}


/**
 * @brief RpgLogic::setLastAuthTick
 * @param newLastAuthTick
 */

void RpgLogic::setLastAuthTick(quint32 newLastAuthTick)
{
	QMutexLocker locker(&m_mutex);
	m_lastAuthTick = newLastAuthTick;
}



/**
 * @brief RpgLogic::render
 */

void RpgLogic::render()
{
	QMutexLocker locker(&m_mutex);

	++m_lastAuthTick;


	/*entt::entity p1 = entityFromIdTag(RpgLogic::packId(0, 1, 0));

	auto [player, tick] = m_registry.try_get<Rpg::Player, Rpg::PlayerTickMap>(p1);

	if (!player || !tick) {
		LOG_CERROR("game") << "ERR";
		return;
	}

	const quint32 fromTick = m_lastAuthTick-1;

	auto fromIt = tick->map.lowerBound(fromTick);
	auto toIt = tick->map.lowerBound(m_lastAuthTick);

	if (fromIt == tick->map.end() || toIt == tick->map.end()) {
		return;
	}

	if (fromIt.key() > fromTick || toIt.key() > m_lastAuthTick) {
		return;
	}

	qint32 dx = (qint32) toIt->entityState().posX() - (qint32) fromIt->entityState().posX();
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

	for (const QPair<quint32, quint32> &ch : excludeSet)
		list.emplace_back(ch.first, ch.second);

	return grid;
}


/**
 * @brief ChunkGrid::getChunk
 * @param x
 * @param y
 * @return
 */

QPair<quint32, quint32> ChunkGrid::getChunk(const float &x, const float &y) const
{
	QPair<quint32, quint32> pos(0, 0);

	if (chunkSize.isNull() || !chunkSize.isValid())
		return pos;

	pos.first = std::floor(std::clamp(x, 0.f, (float) viewport.width()) / chunkSize.width());
	pos.second = std::floor(std::clamp(y, 0.f, (float) viewport.height()) / chunkSize.height());

	return pos;
}












}		// end namespace
