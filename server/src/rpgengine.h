/*
 * ---- Call of Suli ----
 *
 * rpgengine.h
 *
 * Created on: 2025. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEngine
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

#ifndef RPGENGINE_H
#define RPGENGINE_H

#include "abstractengine.h"
#include "rpgconfig.h"
#include "rpgsnapshotstorage.h"


#define SERVER_OID	-2


/**
 * @brief The RpgEnginePlayer class
 */

class RpgEnginePlayer : public RpgGameData::PlayerBaseData
{
public:
	RpgEnginePlayer(UdpServerPeer *peer, const bool &isHost = false)
		: RpgGameData::PlayerBaseData()
		, m_udpPeer(peer)
		, m_isHost(isHost)
	{}

	RpgEnginePlayer(UdpServerPeer *peer = nullptr)
		: RpgEnginePlayer(peer, false)
	{}

	bool isHost() const { return m_isHost; }
	void setIsHost(bool newIsHost) { m_isHost = newIsHost; }

	UdpServerPeer *udpPeer() const { return m_udpPeer; }
	void setUdpPeer(UdpServerPeer *newUdpPeer) { m_udpPeer = newUdpPeer; }

	const RpgGameData::CharacterSelect &config() const { return m_config; }
	void setConfig(const RpgGameData::CharacterSelect &newConfig) { m_config = newConfig; }

	int playerId() const { return o; }
	void setPlayerId(int newPlayerId) {
		o = newPlayerId;
		m_config.playerId = newPlayerId;
	}

	bool isPrepared() const { return m_isPrepared; }
	void setIsPrepared(bool newIsPrepared) { m_isPrepared = newIsPrepared; }

	bool isFullyPrepared() const { return m_isFullyPrepared; }
	void setIsFullyPrepared(bool newIsPrepared) { m_isFullyPrepared = newIsPrepared; }

private:
	UdpServerPeer *m_udpPeer = nullptr;
	bool m_isHost = false;
	bool m_isPrepared = false;
	bool m_isFullyPrepared = false;
	RpgGameData::CharacterSelect m_config;

	friend class RpgEngine;
	friend class RpgEnginePrivate;

};



class RpgEnginePrivate;





/**
 * @brief The RpgEvent class
 */

class RpgEventBase
{
public:
	RpgEventBase(RpgEngine *engine, const qint64 &tick, const bool &unique = true);
	virtual ~RpgEventBase();

	virtual bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) = 0;							// Return true = delete

	virtual bool isEqual(RpgEventBase *other) const = 0;	// Ezalapján döntjük el, hogy egyedi-e (nem ismételhető)

	const qint64 &tick() const { return m_tick; }
	void setTick(const qint64 &tick) { m_tick = tick; }

	bool isUnique() const { return m_unique; }

protected:
	RpgEngine *const m_engine;
	qint64 m_tick = -1;					// Mikor történt / fog történni
	const bool m_unique = true;			// Nem lehet ismételni
};




template <typename T>
class RpgEvent : public RpgEventBase
{
public:
	RpgEvent(RpgEngine *engine, const qint64 &tick, const T &data, const bool &unique = true)
		: RpgEventBase(engine, tick, unique)
		, m_data(data)
	{ }

	virtual ~RpgEvent() = default;

	const T &baseData() const { return m_data; }

	bool isBaseEqual(RpgEventBase *other) const {
		if (RpgEvent<T> *d = dynamic_cast<RpgEvent<T>*>(other); d &&
				d->m_tick == m_tick &&
				d->m_data == m_data
				)
			return true;

		return false;
	}


protected:
	const T m_data;
};



#define ADD_EQUAL(T)		virtual bool isEqual(RpgEventBase *other) const override { \
return dynamic_cast<T*>(other) && isBaseEqual(other); \
}


/**
 * @brief The RpgEventEnemyDied class
 */

class RpgEventEnemyDied : public RpgEvent<RpgGameData::EnemyBaseData>
{
public:
	RpgEventEnemyDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::EnemyBaseData &data)
		: RpgEvent<RpgGameData::EnemyBaseData>(engine, tick, data)
	{
		LOG_CDEBUG("engine") << "Enemy died" << m_data.o << m_data.id << "@" << tick;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventEnemyDied);
};



/**
 * @brief The RpgEventEnemyResurrect class
 */

class RpgEventEnemyResurrect : public RpgEvent<bool>
{
public:
	RpgEventEnemyResurrect(RpgEngine *engine, const qint64 &tick)
		: RpgEvent<bool>(engine, tick, false)
	{
		LOG_CDEBUG("engine") << "RESURRECT created" << tick << m_unique;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventEnemyResurrect);
};







/**
 * @brief The RpgEventPlayerDied class
 */

class RpgEventPlayerDied : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventPlayerDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data)
	{
		LOG_CDEBUG("engine") << "Player died" << m_data.o << m_data.id << "@" << tick;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventPlayerDied);
};





/**
 * @brief The RpgEventPlayerResurrect class
 */

class RpgEventPlayerResurrect : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventPlayerResurrect(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data)
	{
		LOG_CDEBUG("engine") << "PLAYER RESURRECT created" << tick << m_unique << data.o;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventPlayerResurrect);
};





/**
 * @brief The RpgEventContainerUnlock class
 */

class RpgEventControlUnlock : public RpgEvent<RpgGameData::PlayerBaseData>
{
public:
	RpgEventControlUnlock(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
		: RpgEvent<RpgGameData::PlayerBaseData>(engine, tick, data, true)
	{
		LOG_CDEBUG("engine") << "CONTROL UNLOCK created" << tick << m_unique << data.o;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventControlUnlock);
};





/**
 * @brief The RpgEventContainerUnlock class
 */

class RpgEventCollectionRelocate : public RpgEvent<RpgGameData::ControlCollectionBaseData>
{
public:
	RpgEventCollectionRelocate(RpgEngine *engine, const qint64 &tick, const RpgGameData::ControlCollectionBaseData &data,
							   const bool &success, const RpgGameData::BaseData &player)
		: RpgEvent<RpgGameData::ControlCollectionBaseData>(engine, tick, data, true)
		, m_success(success)
		, m_player(player)
	{
		LOG_CDEBUG("engine") << "COLLECTION RELOCATE created" << tick << m_unique << data.o << success << player.o;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventCollectionRelocate);

private:
	bool m_success = false;
	const RpgGameData::BaseData m_player;
};








/**
 * @brief The RpgEventContainerOpened class
 */

class RpgEventContainerOpened : public RpgEvent<RpgGameData::ControlContainerBaseData>
{
public:
	RpgEventContainerOpened(RpgEngine *engine, const qint64 &tick, const RpgGameData::ControlContainerBaseData &data,
							   const RpgGameData::BaseData &player, const QList<RpgGameData::PickableBaseData> &pickables)
		: RpgEvent<RpgGameData::ControlContainerBaseData>(engine, tick, data, true)
		, m_player(player)
		, m_pickables(pickables)
	{
		LOG_CDEBUG("engine") << "CONTAINER OPENED created" << tick << m_unique << data.o << player.o << pickables.size();
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventCollectionRelocate);

private:
	bool m_success = false;
	const RpgGameData::BaseData m_player;
	const QList<RpgGameData::PickableBaseData> m_pickables;
};






/**
 * @brief The RpgEventPickablePicked class
 */

class RpgEventPickablePicked : public RpgEvent<RpgGameData::PickableBaseData>
{
public:
	RpgEventPickablePicked(RpgEngine *engine, const qint64 &tick, const RpgGameData::PickableBaseData &data,
								RendererObject<RpgGameData::PlayerBaseData> *player)
		: RpgEvent<RpgGameData::PickableBaseData>(engine, tick, data, true)
		, m_player(player)
	{
		Q_ASSERT(player);
		LOG_CDEBUG("engine") << "PICKABLE PICKED created" << tick << m_unique << data.o << player->baseData.o;
	}

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;

	ADD_EQUAL(RpgEventPickablePicked);

private:
	 RendererObject<RpgGameData::PlayerBaseData> *const m_player;
};








/**
 * @brief The RpgEngine class
 */

class RpgEngine : public UdpEngine
{
	Q_OBJECT

public:
	explicit RpgEngine(EngineHandler *handler, QObject *parent = nullptr);
	virtual ~RpgEngine();


	static std::shared_ptr<RpgEngine> engineCreate(EngineHandler *handler, UdpServer *server);

	static int increaseNextId() { return ++m_nextId; }
	static int setNextId(const int &id) { m_nextId = id+1; return m_nextId; }

	virtual bool canDelete(const int &useCount) override;

	virtual void binaryDataReceived(const UdpServerPeerReceivedList &data) override;
	virtual void udpPeerAdd(UdpServerPeer *peer) override;
	virtual void udpPeerRemove(UdpServerPeer *peer) override;

	RpgConfig config() const { return m_config; }
	void setConfig(const RpgConfig &newConfig) { m_config = newConfig; }

	RpgEnginePlayer *hostPlayer() const { return m_hostPlayer; }
	void setHostPlayer(RpgEnginePlayer *newHostPlayer);

	qint64 currentTick();
	int nextObjectId() const;


	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	void eventAdd(Args &&...args);

	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	void eventAddLater(Args &&...args);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	T* eventFind(const T2 &data);

	void eventRemove(RpgEventBase *event);


	template <typename T, typename T2,
			  typename = std::enable_if< std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if< std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	int createEvents(const qint64 &tick, const T2 &data, const T &snap, const std::optional<T> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::EnemyBaseData &data,
					 const RpgGameData::Enemy &snap, const std::optional<RpgGameData::Enemy> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::PlayerBaseData &data,
					 const RpgGameData::Player &snap, const std::optional<RpgGameData::Player> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::BulletBaseData &data,
					 const RpgGameData::Bullet &snap, const std::optional<RpgGameData::Bullet> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlBaseData &data,
					 const RpgGameData::ControlLight &snap, const std::optional<RpgGameData::ControlLight> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlContainerBaseData &data,
					 const RpgGameData::ControlContainer &snap, const std::optional<RpgGameData::ControlContainer> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlCollectionBaseData &data,
					 const RpgGameData::ControlCollection &snap, const std::optional<RpgGameData::ControlCollection> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::PickableBaseData &data,
					 const RpgGameData::Pickable &snap, const std::optional<RpgGameData::Pickable> &prev);


	RpgGameData::CurrentSnapshot processEvents(const qint64 &tick);


	const RpgGameData::SnapshotList<RpgGameData::Player, RpgGameData::PlayerBaseData> &players();
	const RpgGameData::SnapshotList<RpgGameData::Enemy, RpgGameData::EnemyBaseData> &enemies();
	const RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData> &bullets();

	const RpgGameData::SnapshotList<RpgGameData::ControlLight, RpgGameData::ControlBaseData> &controlLights();
	const RpgGameData::SnapshotList<RpgGameData::ControlContainer, RpgGameData::ControlContainerBaseData> &controlContainers();
	const RpgGameData::SnapshotList<RpgGameData::ControlCollection, RpgGameData::ControlCollectionBaseData> &controlCollections();
	const RpgGameData::SnapshotList<RpgGameData::Pickable, RpgGameData::PickableBaseData> &pickables();

	void addRelocateCollection(const qint64 &tick, const RpgGameData::ControlCollectionBaseData &base,
							   const RpgGameData::BaseData &player, const bool &success);
	int relocateCollection(const RpgGameData::ControlCollectionBaseData &base, QPointF *ptr = nullptr);
	bool finishCollection(const RpgGameData::ControlCollectionBaseData &base, const int &idx);
	void addPickablePicked(const qint64 &tick, const RpgGameData::PickableBaseData &base,
						   RendererObject<RpgGameData::PlayerBaseData> *player);
	void addContainerOpened(const qint64 &tick, const RpgGameData::ControlContainerBaseData &base,
							   const RpgGameData::BaseData &player);


	void renderTimerLog(const qint64 &msec);


private:
	void binaryDataReceived(UdpServerPeer *peer, const QByteArray &data);
	void preparePlayers();
	qint64 nextTick();

	static int m_nextId;
	int m_nextPlayerId = 1;

	RpgEnginePrivate *d;

	RpgConfig m_config;
	std::vector<std::unique_ptr<RpgEnginePlayer>> m_player;
	RpgEnginePlayer *m_hostPlayer = nullptr;

	RpgSnapshotStorage m_snapshots;

	qint64 m_currentTick = -1;

	friend class RpgEnginePrivate;
};




/**
 * @brief RpgEngine::createEvents
 * @param tick
 * @param data
 * @param snap
 * @param prev
 * @return
 */

template<typename T, typename T2, typename T3, typename T4>
inline int RpgEngine::createEvents(const qint64 &, const T2 &, const T &, const std::optional<T> &)
{
	LOG_CERROR("engine") << "Missing specialization";
	return -1;
}











#endif // RPGENGINE_H
