/*
 * ---- Call of Suli ----
 *
 * rpgconfig.h
 *
 * Created on: 2024. 03. 24.
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

#ifndef RPGLOGIC_H
#define RPGLOGIC_H

#include "chipmunk/cpVect.h"
#include "qcborarray.h"
#include <QSerializer>
#include <QIODevice>
#include <QColor>
#include <entt/entt.hpp>
#include "qmutex.h"
#include "qpaintdevice.h"
#include "qpoint.h"
#include "rpgstream.h"



#define DEFAULT_PULL_SIZE		12


/**************************************************************
 * RPG WORLD
 **************************************************************/


/**
 * @brief The RpgWorldLandGeometry class
 */


class RpgWorldLandGeometry : public QSerializer
{
	Q_GADGET

public:
	RpgWorldLandGeometry()
		: QSerializer()
		, x(0.)
		, y(0.)
		, textX(0.)
		, textY(0.)
		, rotate(0.)
	{}

	QS_SERIALIZABLE

	QS_FIELD(qreal, x)
	QS_FIELD(qreal, y)
	QS_FIELD(qreal, textX)
	QS_FIELD(qreal, textY)
	QS_FIELD(qreal, rotate)
};



/**
 * @brief The RpgWorldOrig class
 */

class RpgWorldOrig : public QSerializer
{
	Q_GADGET

public:
	RpgWorldOrig()
		: QSerializer()
		, width(0.)
		, height(0.)
	{}

	QS_SERIALIZABLE

	QS_FIELD(qreal, width)
	QS_FIELD(qreal, height)
	QS_FIELD(QString, description)
	QS_FIELD(QString, background)
	QS_FIELD(QString, over)
	QS_QT_DICT(QMap, QString, QJsonArray, adjacency)

};





/**
 * @brief The RpgWorldLandMap class
 */

class RpgWorldMapBinding : public QSerializer
{
	Q_GADGET

public:
	RpgWorldMapBinding()
		: QSerializer()
		, free(false)
	{}

	QS_SERIALIZABLE

	QS_FIELD(QString, map)
	QS_FIELD(bool, free)					// Szabadon játszható
};



/**
 * @brief The RpgWorld class
 */


class RpgWorld : public QSerializer
{
	Q_GADGET

public:
	RpgWorld()
		: QSerializer()
	{}

	QS_SERIALIZABLE

	QS_OBJECT(RpgWorldOrig, orig)
	QS_QT_DICT_OBJECTS(QMap, QString, RpgWorldLandGeometry, lands)
	QS_QT_DICT_OBJECTS(QMap, QString, RpgWorldMapBinding, binding)
};






/**************************************************************
 * RPG LOGIC
 **************************************************************/

namespace Rpg {


template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
class BaseStateMap
{
public:
	BaseStateMap() = default;

	const std::map<quint32, T> &map() const { return m_map; }
	void setMap(const std::map<quint32, T> &newMap) { m_map = newMap; }

	void insert(const T &state) { m_map[state.tick()] = state; }
	void insert(T &&state) { m_map[state.tick()] = std::move(state); }

	void clear() { m_map.clear(); }
	void clear(const quint32 &minTick) { m_map.erase(m_map.cbegin(), m_map.lower_bound(minTick)); }

	T* at(const quint32 &tick) {
		if (m_map.empty())
			return nullptr;

		const auto it = m_map.find(tick);
		if (it == m_map.cend())
			return nullptr;
		else
			return &(it->second);
	}

	T* last(const quint32 &tick) {
		if (m_map.empty())
			return nullptr;

		auto it = m_map.upper_bound(tick);

		if (it != m_map.cbegin())
			it = std::prev(it);
		else
			return nullptr;

		return &(it->second);
	}

	bool extract(std::vector<T> &listPtr,
				 const std::map<quint32, T>::const_iterator &from) const {

		if (from == m_map.cend())
			return false;

		listPtr.clear();
		listPtr.reserve(m_map.size());

		for (auto it = from; it != m_map.cend(); ++it)
			listPtr.push_back(it->second);

		return true;
	}

	bool extract(std::vector<T> &listPtr) const {
		return extract(listPtr, m_map.cbegin());
	}

	bool extract(std::vector<T> &listPtr, const quint32 &minTick) const {
		return extract(listPtr, m_map.lower_bound(minTick));
	}


	int load(const std::vector<T> &list, const quint32 &minTick = 0, const quint32 &maxTick = 0) {
		int n = 0;
		for (const T &state : list) {
			if (state.tick() < minTick || (maxTick > 0 && state.tick() > maxTick)) {
				LOG_CTRACE("game") << "Tick dropped" << state.tick();
				continue;
			}

			insert(state);

			++n;
		}
		return n;
	}



protected:
	std::map<quint32, T> m_map;
};






/**
 * @brief The BaseEventMap class
 */

template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
class BaseEventMap
{
public:
	BaseEventMap() = default;

	const std::map<quint32, std::vector<std::pair<quint32, T> > > &map() const { return m_map; }
	void setMap(const std::map<quint32, std::vector<std::pair<quint32, T> > > &newMap) { m_map = newMap; }

	void insert(const quint32 &id, const T &state) { m_map[state.tick()].push_back({id, state}); }
	void insert(const quint32 &id, T &&state) { m_map[state.tick()].emplace_back(id, std::move(state)); }

	void clear() { m_map.clear(); }
	void clear(const quint32 &minTick) { m_map.erase(m_map.cbegin(), m_map.lower_bound(minTick)); }

	std::vector<std::pair<quint32, T> >* at(const quint32 &tick) {
		if (m_map.empty())
			return nullptr;

		const auto it = m_map.find(tick);
		if (it == m_map.cend())
			return nullptr;
		else
			return &(it->second);
	}

	std::vector<std::pair<quint32, T> >* last(const quint32 &tick) {
		if (m_map.empty())
			return nullptr;

		auto it = m_map.upper_bound(tick);

		if (it != m_map.cbegin())
			it = std::prev(it);
		else
			return nullptr;

		return &(it->second);
	}


	int load(const quint32 &id, const std::vector<T> &list, const quint32 &minTick = 0, const quint32 &maxTick = 0) {
		int n = 0;
		for (const T &state : list) {
			if (state.tick() < minTick || (maxTick > 0 && state.tick() > maxTick)) {
				LOG_CTRACE("game") << "Tick dropped" << state.tick();
				continue;
			}

			insert(id, state);

			++n;
		}
		return n;
	}




protected:
	std::map<quint32, std::vector<std::pair<quint32, T> > > m_map;
};







/**
 * @brief The BaseStatePull class
 */

template <typename T, std::size_t PULL_SIZE = DEFAULT_PULL_SIZE,
		  typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
class BaseStatePull
{
public:
	BaseStatePull() = default;

	void reset() { m_head = 0; }
	void append(const T &content) {
		if (m_head > 1 && m_list[(m_head-1) % PULL_SIZE] == content)
			return;

		m_list[m_head % PULL_SIZE] = content;
		++m_head;
	}
	void append(T &&content) {
		if (m_head > 1 && m_list[(m_head-1) % PULL_SIZE] == content)
			return;

		m_list[m_head % PULL_SIZE] = std::move(content);
		++m_head;
	}

	std::vector<T> extract(const int &max = 0) {
		std::vector<T> list;

		if (m_head == 0)
			return list;

		const quint32 from = std::max(std::max((int) m_head - (int) PULL_SIZE, 0),
									  max > 0 && max <= (int) PULL_SIZE ? ((int) m_head - max) : 0);

		list.reserve(PULL_SIZE);

		for (quint32 i=from; i<m_head; ++i) {
			list.emplace_back(m_list[i % PULL_SIZE]);
		}

		return list;
	}


	const T* at(const quint32 &tick) const {
		if (m_head == 0)
			return nullptr;

		for (const T &t : m_list) {
			if (t.tick() == tick) {
				return &t;
			}
		}

		return nullptr;
	}


	const T* last() const {
		if (m_head == 0)
			return nullptr;

		return &(m_list[(m_head-1) % PULL_SIZE]);
	}


protected:
	std::array<T, PULL_SIZE> m_list;
	quint32 m_head = 0;
};















// Minden objektum közös azonosítója (packId, unpackId)
// Amelyik entity-nek van a kliensen, azt már szinkronizáltuk (létrehoztuk), tehát az exclued-del le tudjuk kérni, amit meg kell csinálni

struct IdTag
{
	quint32 id = 0;
};

struct IdTagMapper
{
	QHash<quint32, entt::entity> map;

	entt::entity get(const quint32 &id) const { return map.value(id, entt::null); }
};



// Render után törlendő entity-k

struct DeleteTag { };


// Esemény

struct EventTag {
	quint32 tick = 0;
};

struct EventProcessingTag { };




// Csapatjelzés

struct TeamTag
{
	RpgStream::Team team = RpgStream::TeamNone;
};




// Játékos

struct Player
{
	RpgStream::PlayerData playerData;

	quint32 lastObjectId = 0;

	quint32 idTag() const;
};



// Mp kibocsátó

struct MpEmitter
{
	quint32 idTag = 0;
	cpVect pos = cpvzero;

	float radius = 0.;
	quint32 capacity = 0;

	std::vector<entt::entity> mpList;

	static MpEmitter fromRpgStream(const RpgStream::MpEmitter &stream);
	RpgStream::MpEmitter toRpgStream() const;
};



// Mp

struct Mp
{
	quint32 idTag = 0;
	entt::entity emitter = entt::null;

	cpVect pos = cpvzero;
};




// Tower

struct Tower
{
	quint32 idTag = 0;
	/*cpVect pos = cpvzero;

	float radius = 0.;
	quint32 capacity = 0;

	std::vector<entt::entity> mpList;*/

	static Tower fromRpgStream(const RpgStream::Tower &stream);
	RpgStream::Tower toRpgStream() const;
};



// Chunkgrid data

struct ChunkGrid
{
	QRectF viewport;
	QSet<QPair<qint32, qint32> > excludeSet;
	QSizeF chunkSize;

	static ChunkGrid fromRpgStream(const RpgStream::ChunkGrid &grid);
	RpgStream::ChunkGrid toRpgStream() const;

	QPair<qint32, qint32> getAccessibleChunk(const float &x, const float &y) const {
		if (isAccessible(x, y))
			return getChunk(x, y);
		else
			return {-1., -1.};
	}

	QPair<qint32, qint32> getAccessibleChunk(const QPointF &pos) const {
		return getAccessibleChunk(pos.x(), pos.y());
	}

	QPair<qint32, qint32> getChunk(const float &x, const float &y) const;
	QPair<qint32, qint32> getChunk(const QPointF &pos) const {
		return getChunk(pos.x(), pos.y());
	}

	bool isAccessible(const float &x, const float &y) const {
		return !excludeSet.contains(getChunk(x, y));
	}
	bool isAccessible(const QPointF &pos) const {
		return isAccessible(pos.x(), pos.y());
	}
};




typedef std::vector<RpgStream::PlayerPosition> PlayerPositionList;

typedef BaseStateMap<RpgStream::PlayerState> PlayerStateInput;
typedef BaseStatePull<RpgStream::PlayerState> PlayerStateOutput;

typedef BaseStatePull<RpgStream::TowerState> TowerStateOutput;

typedef BaseStatePull<RpgStream::Events> EventsOutput;










class RpgLogicPrivate;
class RpgLogicScope;


/**
 * @brief The RpgLogic class
 */

class RpgLogic
{
public:
	RpgLogic(const quint32 &lastAuthDiff = 0, const quint32 &jitterDiff = 0);
	virtual ~RpgLogic();

	// Get scope

	[[nodiscard]] RpgLogicScope getScope();

	// Set tick

	quint32 serverTick() const { return m_serverTick; }
	quint32 lastAuthTick() const { return m_serverTick > m_lastAuthTickDiff ? m_serverTick-m_lastAuthTickDiff : 0; }
	quint32 jitterTick() const { return m_serverTick > m_jitterDiff ? m_serverTick-m_jitterDiff : 0; }



	///
	/// Render logic:
	///
	/// MULTIPLAYER
	///
	/// Client
	///		worldStep() -> after() -> OutpuStatePull[] -> FullState -> logic:send
	///		logic:receive -> InputStatePull[] -> currentFullState(jittered) -> before() -> worldStep()
	///
	/// Server
	///		logic:receive -> InputStatePull[] -> render() -> OutputStatePull[] -> logic:send
	///
	///
	/// SINGLE PLAYER
	///
	/// Client
	///		worldStep() -> after() -> OutputStatePull[0] -> FullState -> InputStatePull[0] -> render() ->
	///		-> currentFullState(0) -> before() -> worldStep()
	///


	void fullStateLoad(const RpgStream::FullState &full, const QSet<quint32> &acceptedInputList);
	RpgStream::FullState getFullState(const int &maxTick);

	void render();

	// EnTT object id

	static quint32 packId(const quint32 &scene, const quint32 &owner, const quint32 &id);
	static void unpackId(const quint32 &from, quint32 &scene, quint32 &owner, quint32 &id);

	void entitySetIdTag(entt::entity &entity, const quint32 &tag);
	entt::entity entityFromIdTag(const quint32 &tag) const;


	// Map Data

	void loadMapData(const RpgStream::MapData &data);


	// Player

	entt::entity playerAdd(const RpgStream::Team &team = RpgStream::TeamNone);

	void emplacePlayers();





protected:
	RpgLogicPrivate *d = nullptr;
	quint32 m_serverTick = 0;
	const quint32 m_lastAuthTickDiff = 0;
	const quint32 m_jitterDiff = 0;

	template <typename T>
	void registerCtx() {
		QMutexLocker locker(&m_mutex);
		if (!m_registry.ctx().contains<T>()) {
			m_registry.ctx().emplace<T>();
		}
	}


	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	const T* getCurrentState(entt::entity ent) const
	{
		QMutexLocker locker(&m_mutex);

		auto [pull, state] = m_registry.try_get<BaseStatePull<T>, T>(ent);

		if (state)
			return state;

		if (!pull)
			return nullptr;

		return pull->last();
	}



	mutable QRecursiveMutex m_mutex;
	entt::registry m_registry;
	quint32 m_lastObjectId = 0;

	friend class RpgLogicPrivate;
	friend class RpgLogicScope;
};







/**
 * @brief The RpgLogicScope class
 */

class RpgLogicScope
{
public:
	RpgLogicScope(RpgLogic *logic)
		: m_logic(logic)
		, m_locker(&logic->m_mutex)
	{}

	// EnTT Context

	template <typename T>
	void registerCtx();

	template<typename T>
	[[nodiscard]] const T *getCtx() const;

	template<typename T>
	[[nodiscard]] T *getCtx();



	template<typename... Ts, typename... Args>
	[[nodiscard]] auto view(Args&&... args) const;

	template<typename... Ts, typename... Args>
	[[nodiscard]] auto try_get(Args&&... args) const;

	template<typename... Ts, typename... Args>
	[[nodiscard]] auto get(Args&&... args) const;

	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	const T* getCurrentState(entt::entity ent) const;

	RpgLogic *logic() const { return m_logic; }

	entt::entity entityFromIdTag(const quint32 &tag) const { return m_logic->entityFromIdTag(tag); }

private:
	RpgLogic *const m_logic;
	const QMutexLocker<QRecursiveMutex> m_locker;
};














inline quint32 Player::idTag() const { return RpgLogic::packId(0, playerData.playerId(), 0); }



/**
 * @brief RpgLogic::registerCtx
 */

template<typename T>
inline void RpgLogicScope::registerCtx()
{
	m_logic->registerCtx<T>();
}


/**
 * @brief RpgLogic::getCtx
 * @return
 */

template<typename T>
inline T *RpgLogicScope::getCtx()
{
	return m_logic->m_registry.ctx().find<T>();
}


/**
 * @brief RpgLogic::getCtx
 * @return
 */

template<typename T>
inline const T *RpgLogicScope::getCtx() const
{
	return m_logic->m_registry.ctx().find<T>();
}




/**
 * @brief RpgLogic::view
 * @param args
 */

template<typename... Ts, typename... Args>
inline auto RpgLogicScope::view(Args&&... args) const
{
	return m_logic->m_registry.view<Ts...>(std::forward<Args>(args)...);
}


/**
 * @brief RpgLogic::try_get
 * @param args
 */

template<typename... Ts, typename... Args>
inline auto RpgLogicScope::try_get(Args&&... args) const
{
	return m_logic->m_registry.try_get<Ts...>(std::forward<Args>(args)...);
}



/**
 * @brief RpgLogic::try_get
 * @param args
 */

template<typename... Ts, typename... Args>
inline auto RpgLogicScope::get(Args&&... args) const
{
	return m_logic->m_registry.get<Ts...>(std::forward<Args>(args)...);
}


/**
 * @brief RpgLogicScope::getCurrentState
 * @param ent
 * @return
 */

template<class T, typename T2>
inline const T *RpgLogicScope::getCurrentState(entt::entity ent) const
{
	return m_logic->getCurrentState<T>(ent);
}



}			// end namespace
#endif // RPGLOGIC_H
