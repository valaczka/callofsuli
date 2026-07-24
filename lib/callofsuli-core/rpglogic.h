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
#include <random>
#include "qmutex.h"
#include "qpaintdevice.h"
#include "qpoint.h"
#include "rpgconfig.h"
#include "rpgstream.h"



#define DEFAULT_PULL_SIZE		12


#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
#define USE_LOGGER
#endif


#ifdef USE_LOGGER

#define ELOG_TRACE            CuteMessageLogger(_logger(), Logger::Trace,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_DEBUG            CuteMessageLogger(_logger(), Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_INFO             CuteMessageLogger(_logger(), Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_WARNING          CuteMessageLogger(_logger(), Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_ERROR            CuteMessageLogger(_logger(), Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_FATAL            CuteMessageLogger(_logger(), Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO).write()


#else

#define ELOG_TRACE            qDebug()
#define ELOG_DEBUG            qDebug()
#define ELOG_INFO             qInfo()
#define ELOG_WARNING          qWarning()
#define ELOG_ERROR            qWarning()
#define ELOG_FATAL            qCritical()

#endif

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
				//LOG_CTRACE("game") << "Tick dropped" << state.tick() << "min:" << minTick << "max:" << maxTick;
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
				//LOG_CTRACE("game") << "Tick dropped" << state.tick() << "min:" << minTick << "max:" << maxTick;
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
		m_maxTick = std::max(content.tick(), m_maxTick);
	}
	void append(T &&content) {
		if (m_head > 1 && m_list[(m_head-1) % PULL_SIZE] == content)
			return;

		m_list[m_head % PULL_SIZE] = std::move(content);
		++m_head;
		m_maxTick = std::max(content.tick(), m_maxTick);
	}

	void appendGreater(const T &content) {
		if (m_head == 0 || content.tick() > m_maxTick)
			append(content);
	}

	void appendGreater(T &&content) {
		if (m_head == 0 || content.tick() > m_maxTick)
			append(std::move(content));
	}

	void insert(const T &content) {
		for (quint32 i=0; i<PULL_SIZE && i<m_head; ++i) {
			if (m_list[i].tick() == content.tick()) {
				m_list[i] = content;
				return;
			}
		}

		append(content);
	}
	void insert(T &&content) {
		for (quint32 i=0; i<PULL_SIZE && i<m_head; ++i) {
			if (m_list[i].tick() == content.tick()) {
				m_list[i] = std::move(content);
				return;
			}
		}

		append(std::move(content));
	}

	std::vector<T> extract(const int &max = 0) const {
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

	std::vector<T> extractAtLeast(const quint32 &minTick, const int &max = 0) const {
		std::vector<T> list;

		if (m_head == 0)
			return list;

		const quint32 from = std::max(std::max((int) m_head - (int) PULL_SIZE, 0),
									  max > 0 && max <= (int) PULL_SIZE ? ((int) m_head - max) : 0);

		list.reserve(PULL_SIZE);

		for (quint32 i=from; i<m_head; ++i) {
			const T &d = m_list[i % PULL_SIZE];
			if (d.tick() >= minTick)
				list.emplace_back(d);
		}

		return list;
	}

	std::map<quint32, T> extractToMap(const quint32 &minTick = 0) const {
		std::map<quint32, T> list;

		if (m_head == 0)
			return list;

		for (quint32 i=0; i<m_head && i<PULL_SIZE; ++i) {
			const T &d = m_list[i % PULL_SIZE];
			if (d.tick() >= minTick)
				list[d.tick()] = d;
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



	const T* atMost(const quint32 &tick) const {
		if (m_head == 0)
			return nullptr;

		const T* r = nullptr;

		for (const T &t : m_list) {
			if (t.tick() <= tick && (!r || r->tick() < tick)) {
				r = &t;
			}
		}

		return r;
	}


	const T* last() const {
		if (m_head == 0)
			return nullptr;

		return &(m_list[(m_head-1) % PULL_SIZE]);
	}



	const T* latest() const {
		if (m_head == 0)
			return nullptr;

		const T* r = nullptr;

		for (const T &t : m_list) {
			if (!r || r->tick() < t.tick()) {
				r = &t;
			}
		}

		return r;
	}


protected:
	std::array<T, PULL_SIZE> m_list;
	quint32 m_head = 0;
	quint32 m_maxTick = 0;
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

struct EventTag {									// Előre rögzítjük (majd meg fog történni)
	quint32 tick = 0;
};

struct EventProcessingTag { };						// Az aktuális renderben dolgozzuk fel (ellenőrzések után az ütközések feloldása)
struct EventRealTag { };							// Az aktuális renderben ténylegesen megtörtént események

struct KnockbackTag {								// Folyamatban van még a hátracsúszás
	quint32 tick = 0;
};




// Játékos

struct Player
{
	RpgStream::PlayerData playerData;

	RpgStream::Team team = RpgStream::TeamNone;

	quint32 idTag() const;
};



// Npc

struct Npc
{
	quint32 idTag = 0;
	RpgStream::NpcData data;
};


// Npc (TowerAttacker)

struct NpcTowerAttacker
{
	quint32 destinationTower = 0;					// Ez nem itt kell...
	cpVect destination = cpvzero;
};



// A játékos kérdésre válaszol

struct LockTag {
	quint32 id = 0;
	quint32 expire = 0;								// Amikor lejár, töröljük automatikusan (pl. ha kilépett közben)
	quint32 penalty = 1;							// Hp csökkentés hibás válasz esetén
};


// Ameddig a játékos nem kérhet újabb zárolást

struct PenaltyTag {
	quint32 expire = 0;
};


// Mp kibocsátó

struct MpEmitter
{
	quint32 idTag = 0;
	cpVect pos = cpvzero;

	bool active = false;

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
	cpVect origin = cpvzero;
};



// Tower defender

struct Defender
{
	quint32 idTag = 0;

	cpVect pos = cpvzero;

	entt::entity tower = entt::null;
	entt::entity object = entt::null;				// A ráhelyezett DefenderObject

	static Defender fromRpgStream(const RpgStream::Defender &stream);
	RpgStream::Defender toRpgStream() const;
};


// Chunk

struct Chunk
{
	quint32 x = 0;
	quint32 y = 0;

	bool operator== (const Chunk &other) const {
		return other.x == x && other.y == y;
	}

	static Chunk fromRpgStream(const RpgStream::Chunk &stream);
	RpgStream::Chunk toRpgStream() const;
};



// Tower

struct Tower
{
	quint32 idTag = 0;
	bool active = false;

	cpVect pos = cpvzero;
	std::vector<entt::entity> defenderList;
	std::vector<Chunk> adjacentChunks;

	static Tower fromRpgStream(const RpgStream::Tower &stream);
	RpgStream::Tower toRpgStream() const;
};








/**
 * @brief The DefenderObject class
 */

struct DefenderObject
{
	quint32 idTag = 0;

	cpVect pos = cpvzero;

	entt::entity defender = entt::null;

	RpgStream::BaseDefenderObject::Type type = RpgStream::BaseDefenderObject::None;
	RpgStream::Team team = RpgStream::TeamNone;
	quint32 maxHp = 0;

	quint32 radius = 350;					// Ekkora körben hat

	quint32 repeaterDelay = 30;				// Ennyi tick kell két akció között
	quint32 lastAction = 0;					// Ekkor volt utoljára akció
	quint32 actionsToHpLoss = 3;			// Ennyi akció után veszít hp-t

	std::unordered_set<entt::entity> nearTargets;	// Ezek vannak jelen pillanatban a közelében

	std::unordered_set<entt::entity> lastTargets;
	quint32 actionCounter = 0;

	static DefenderObject fromRpgStream(const RpgStream::BaseDefenderObject &stream);
	RpgStream::BaseDefenderObject toRpgStream() const;

	bool isNear(const cpVect &pos) const;

	void fromDefenderConfigBase(const CfgDefenderBase &cfg);
};



/**
 * @brief The DefenderDummyObject class
 */
/*
struct DefenderDummyObject
{
	quint32 dummy = 0;

	static DefenderDummyObject fromRpgStream(const RpgStream::BaseDefenderObject &stream);
	void toRpgStream(RpgStream::BaseDefenderObject &stream) const;
};
*/





/**
 * @brief The Utility class
 */

struct Utility
{
	RpgStream::PlayerConfig::Utility type = RpgStream::PlayerConfig::UtilityNone;
	RpgStream::Team team = RpgStream::TeamNone;

	entt::entity target = entt::null;

	quint32 destroyAt = 0;
};





// Chunkgrid data

struct ChunkGrid
{
	QRectF viewport;
	QSet<QPair<qint32, qint32> > excludeSet;
	QSizeF chunkSize;

	qint32 gridWidth = 0;
	qint32 gridHeight = 0;

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
	bool isAccessible(const Chunk &chunk) const {
		return !excludeSet.contains(QPair<qint32, qint32>(chunk.x, chunk.y));
	}

	cpVect chunkCenter(const int &x, const int &y) const;
	cpVect chunkCenter(const QPoint &pos) const {
		return chunkCenter(pos.x(), pos.y());
	}
	cpVect chunkCenter(const Chunk &chunk) const {
		return chunkCenter(chunk.x, chunk.y);
	}

	std::optional<cpVect> accessibleChunkCenter(const int &x, const int &y) const {
		if (isAccessible(x, y))
			return chunkCenter(x, y);
		else
			return std::nullopt;
	}
	std::optional<cpVect> accessibleChunkCenter(const QPoint &pos) const {
		return accessibleChunkCenter(pos.x(), pos.y());
	}
	std::optional<cpVect> accessibleChunkCenter(const Chunk &chunk) const {
		return accessibleChunkCenter(chunk.x, chunk.y);
	}

	std::optional<Chunk> getRandomChunk(std::mt19937 &rnd) const {
		if (gridWidth == 0 && gridHeight == 0)
			return std::nullopt;

		Chunk c;
		std::uniform_int_distribution<int> dx(0, gridWidth);
		std::uniform_int_distribution<int> dy(0, gridHeight);

		for (int i=0; i<100; ++i) {
			c.x = dx(rnd);
			c.y = dy(rnd);

			if (isAccessible(c))
				return c;
		}

		return std::nullopt;
	}
};








// Control

struct Control
{
	quint32 idTag = 0;
	RpgStream::ControlData::Type type = RpgStream::ControlData::None;
	cpVect pos = cpvzero;

	quint32 data = 0;

	RpgStream::ControlData toRpgStream() const;
};




// Chest

struct Chest
{
	cpVect pos = cpvzero;

	enum State {
		StateNormal,
		StateActivated,
		StateDisabled
	};
};








typedef std::vector<RpgStream::PlayerPosition> PlayerPositionList;

typedef BaseStateMap<RpgStream::PlayerState> PlayerStateInput;
typedef BaseStatePull<RpgStream::PlayerState> PlayerStateOutput;

typedef BaseStatePull<RpgStream::TowerState> TowerStateOutput;
typedef BaseStatePull<RpgStream::DefenderState> DefenderStateOutput;

typedef BaseStatePull<RpgStream::Events> EventsOutput;

typedef BaseStateMap<RpgStream::NpcState> NpcStateInput;
typedef BaseStatePull<RpgStream::NpcState> NpcStateOutput;

typedef BaseStatePull<RpgStream::ControlState> ControlStateOutput;

typedef std::vector<Chest> ChestList;

typedef std::vector<RpgStream::Heat> HeatList;
typedef std::vector<RpgStream::Quest> QuestList;




///
/// Game events
///





// Egy event-et csak egyszer dolgozunk fel a szerveren

struct EventWindow {
	quint32 highestSeq = 0;
	quint64 mask = 0;
	bool initialized = false;

	bool accept(const quint32 &seq) {
		if (!initialized) {
			initialized = true;
			highestSeq = seq;
			mask = 1ull;
			return true;
		}

		if (seq > highestSeq) {
			uint32_t diff = seq - highestSeq;

			if (diff >= 64) {
				mask = 1ull;
			} else {
				mask <<= diff;
				mask |= 1ull;
			}

			highestSeq = seq;
			return true;
		}


		uint32_t diff = highestSeq - seq;

		if (diff >= 64)
			return false;

		uint64_t bit = 1ull << diff;

		if (mask & bit)
			return false;


		mask |= bit;
		return true;
	}
};




/**
 * @brief The EventWindowHash class
 */

class EventWindowHash : public QHash<quint32, EventWindow>
{
public:
	EventWindowHash() = default;
	~EventWindowHash() = default;

	bool accept(const quint32 &tag, const quint32 &seq) {
		auto it = this->find(tag);
		if (it == this->end())
			return false;
		return it->accept(seq);
	}

	bool accept(const quint32 &tag, const RpgStream::BaseEventState &eventState) {
		return accept(tag, eventState.seq());
	}
};




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
	bool lock = true;
	bool skipLock = false;
};




// Player use control

struct EventControl {
	entt::entity control;
	entt::entity player;
	bool lock = true;
	bool skipLock = false;
};



// Player put tower defender

struct EventDefenderPut {
	entt::entity defender;
	entt::entity player;
	RpgStream::BaseDefenderObject::Type type = RpgStream::BaseDefenderObject::None;
};



// Add defender to chunk

struct EventDefenderAdd {
	Chunk chunk;
	entt::entity player;
	RpgStream::BaseDefenderObject::Type type = RpgStream::BaseDefenderObject::None;
};


// Using utility

struct EventUtility {
	entt::entity player;
	entt::entity target;
	RpgStream::PlayerConfig::Utility type = RpgStream::PlayerConfig::UtilityNone;
};



// Player target player

struct EventAttackPlayer {
	entt::entity player;
	entt::entity target;
	bool withHurt = false;
	bool withSuccess = true;				// Ha pl. DefenderFog-ban van, akkor nem lehet eltalálni
};



// Player target NPC

struct EventAttackNpc {
	entt::entity player;
	entt::entity target;
	bool withSuccess = true;				// Ha pl. DefenderFog-ban van, akkor nem lehet eltalálni
};



// Player target defender

struct EventAttackDefender {
	entt::entity player;
	entt::entity target;
	bool lock = true;
	bool skipLock = false;
};




// Npc target defender

struct EventNpcAttackDefender {
	entt::entity npc;
	entt::entity target;
	quint32 operation = 0;
};



// Npc target tower

struct EventNpcAttackTower {
	entt::entity npc;
	entt::entity target;
};


// Tower activated

struct EventTowerActiveChanged {
	entt::entity tower;
	bool active = false;
	RpgStream::Team team = RpgStream::TeamNone;
};



// Change Mp to bullet

struct EventChangeBullet {
	entt::entity player;
	bool lock = true;
	bool skipLock = false;
};


// Change Mp to defender

struct EventChangeDefender {
	entt::entity player;
	bool lock = true;
	bool skipLock = false;
	RpgStream::BaseDefenderObject::Type type = RpgStream::BaseDefenderObject::None;
};


// Change Mp to utility

struct EventChangeUtility {
	entt::entity player;
	bool lock = true;
	bool skipLock = false;
	RpgStream::PlayerConfig::Utility type = RpgStream::PlayerConfig::UtilityNone;
};



class RpgLogicPrivate;
class RpgLogicScope;


/**
 * @brief The RpgLogic class
 */

class RpgLogic
{
public:
	RpgLogic(const quint32 &lastAuthDiff = 0);
	virtual ~RpgLogic();

	// Logger

#ifdef USE_LOGGER
	void setLogger(Logger *logger) { m_logger = logger; }
#endif

	// Get scope

	[[nodiscard]] RpgLogicScope getScope();

	// Set tick

	quint32 serverTick() const { return m_serverTick; }
	quint32 lastAuthTick() const { return m_serverTick > m_lastAuthTickDiff ? m_serverTick-m_lastAuthTickDiff : 0; }



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


	void fullStateLoad(const RpgStream::FullState &full, EventWindowHash *acceptedInputList);
	RpgStream::FullState getFullState(const int &maxTick, QString *textPtr = nullptr);

	void fullLoad(const RpgStream::Full &full);
	RpgStream::Full getFull(QString *textPtr = nullptr);

	bool initialize();
	bool startStageSelect();
	bool increaseHeat();
	void selectQuest(const RpgStream::QuestSelect &data);

	bool render(const bool &first = false);
	void renderStageSelect();
	void renderUpdate();

	// EnTT object id

	static quint32 packId(const quint32 &scene, const quint32 &owner, const quint32 &id);
	static void unpackId(const quint32 &from, quint32 &scene, quint32 &owner, quint32 &id);


	// Map Data

	void loadMapData(const RpgStream::MapData &data);
	void reloadMapData(const RpgStream::MapData &data);
	bool isChunkEmpty(const Chunk &chunk) const;
	std::optional<RpgStream::MapData> getMapData() const;


	// Player

	entt::entity playerAdd(const RpgStream::PlayerData &data, quint32 *idPtr = nullptr, quint32 *tagIdPtr = nullptr);

	// Common entity

	static cpVect addKnockbackImpulse(RpgStream::EntityState *targetState,
									  const RpgStream::EntityState &attackerState,
									  const RpgStream::EntityConfig &attacker,
									  const RpgStream::EntityConfig &target);

	static cpVect decayKnockback(RpgStream::EntityState *targetState);
	static cpVect decayKnockback(cpVect &knockback);

	static RpgStream::Team oppositeTeam(const RpgStream::Team &team);


	std::mt19937 &rnd() { return m_rnd; }

	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	void eventStore(T &&event);

protected:

	virtual void eventRealized(entt::entity entity) { Q_UNUSED(entity); }
	void eventRealizedDefault(entt::entity entity);

	void entitySetIdTag(entt::entity &entity, const quint32 &tag);
	entt::entity entityFromIdTag(const quint32 &tag) const;

	virtual void rewindStage(const RpgStream::GameConfig::Stage &oldStage);
	virtual std::unordered_set<entt::entity> initializeTowers();
	virtual std::unordered_set<entt::entity> initializeEmitters();
	virtual std::vector<Chest> initializeChests();
	virtual void checkState(const RpgStream::GameState &state);

	entt::entity npcAdd(const RpgStream::NpcData &data, entt::entity owner, const cpVect &pos = cpvzero, quint32 *tagIdPtr = nullptr);

	virtual void onNpcCreated(entt::entity entity, const quint32 &idTag, Player *player);
	virtual bool onControlStateChange(entt::entity entity, Control *control, const bool &isAlive, const quint32 &state);

	virtual RpgStream::Result getResult();
	virtual QuestList getQuestList() const;

	RpgStream::Result getResultByTeam(const RpgStream::Team &team);


private:
	RpgLogicPrivate *d = nullptr;

protected:
	quint32 m_serverTick = 0;
	const quint32 m_lastAuthTickDiff = 0;

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


	template <class T, typename = std::enable_if<std::is_base_of<RpgStream::BaseTickState, T>::value>::type>
	const T* getLastState(entt::entity ent) const
	{
		QMutexLocker locker(&m_mutex);

		auto pull = m_registry.try_get<BaseStatePull<T> >(ent);

		if (!pull)
			return nullptr;

		return pull->last();
	}


#ifdef USE_LOGGER
	Logger *_logger() const { return m_logger; };
	Logger *m_logger = cuteLoggerInstance();
#endif

	mutable QRecursiveMutex m_mutex;
	entt::registry m_registry;

	std::mt19937 m_rnd;

	friend class RpgLogicPrivate;
	friend class RpgLogicScope;
};









/**
 * @brief RpgLogic::eventStore
 * @param event
 */

template<class T, typename T2>
inline void RpgLogic::eventStore(T &&event)
{
	QMutexLocker locker(&m_mutex);

	auto entity = m_registry.create();

	m_registry.emplace<EventTag>(entity, event.tick());
	m_registry.emplace<T>(entity, std::move(event));
}







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

	bool valid(const entt::entity ent) const { return m_logic->m_registry.valid(ent); }

private:
	RpgLogic *const m_logic;
	const QMutexLocker<QRecursiveMutex> m_locker;
};






/// Common events


// Generate mp

class EventMpCreate : public RpgStream::BaseTickState
{
public:
	EventMpCreate() : RpgStream::BaseTickState() {}

	static EventMpCreate createMp(const RpgStream::GameConfig::Stage &stage, const quint32 &tickNow);

	entt::entity emitter = entt::null;
	entt::entity player = entt::null;

	float capacityRatio = 1.0;
	float mpCount = 0;
	cpVect pos = cpvzero;
};






// Emitter empty

class EventMpEmitterEmpty : public RpgStream::BaseTickState
{
public:
	EventMpEmitterEmpty() : RpgStream::BaseTickState() {}

	entt::entity emitter = entt::null;
};





// Create Npc

class EventNpcCreate : public RpgStream::BaseTickState
{
public:
	EventNpcCreate() : RpgStream::BaseTickState() {}

	entt::entity owner = entt::null;

	RpgStream::NpcData data;
	cpVect pos = cpvzero;
};





// Change control state

class EventControlStateChange : public RpgStream::BaseTickState
{
public:
	EventControlStateChange() : RpgStream::BaseTickState() {}

	entt::entity control = entt::null;

	bool isAlive = true;
	quint32 state = 0;

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
