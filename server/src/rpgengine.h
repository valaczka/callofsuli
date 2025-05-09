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

class RpgEvent
{
public:
	RpgEvent(RpgEngine *engine, const qint64 &tick, const bool &unique = true);
	virtual ~RpgEvent();

	virtual bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) = 0;							// Return true = delete
	virtual bool isEqual(RpgEvent *other) const = 0;	// Ezalapján döntjük el, hogy egyedi-e (nem ismételhető)

	const qint64 &tick() const { return m_tick; }
	bool isUnique() const { return m_unique; }

protected:
	RpgEngine *const m_engine;
	const qint64 m_tick;				// Mikor történt / fog történni
	const bool m_unique = true;			// Nem lehet ismételni
};





/**
 * @brief The RpgEventEnemyDied class
 */

class RpgEventEnemyDied : public RpgEvent
{
public:
	RpgEventEnemyDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::EnemyBaseData &data);

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;
	bool isEqual(RpgEvent *other) const override;

private:
	const RpgGameData::EnemyBaseData m_data;
};



/**
 * @brief The RpgEventEnemyResurrect class
 */

class RpgEventEnemyResurrect : public RpgEvent
{
public:
	RpgEventEnemyResurrect(RpgEngine *engine, const qint64 &tick);

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;
	bool isEqual(RpgEvent *other) const override;
};







/**
 * @brief The RpgEventPlayerDied class
 */

class RpgEventPlayerDied : public RpgEvent
{
public:
	RpgEventPlayerDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data);

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;
	bool isEqual(RpgEvent *other) const override;

private:
	const RpgGameData::PlayerBaseData m_data;
};



/**
 * @brief The RpgEventPlayerResurrect class
 */

class RpgEventPlayerResurrect : public RpgEvent
{
public:
	RpgEventPlayerResurrect(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data);

	bool process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst) override;
	bool isEqual(RpgEvent *other) const override;

private:
	const RpgGameData::PlayerBaseData m_data;
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

	virtual void binaryDataReceived(const UdpServerPeerReceivedList &data) override;
	virtual void udpPeerAdd(UdpServerPeer *peer) override;
	virtual void udpPeerRemove(UdpServerPeer *peer) override;

	RpgConfig config() const { return m_config; }
	void setConfig(const RpgConfig &newConfig) { m_config = newConfig; }

	RpgEnginePlayer *hostPlayer() const { return m_hostPlayer; }
	void setHostPlayer(RpgEnginePlayer *newHostPlayer);

	qint64 currentTick();



	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEvent, T>::value>::type>
	void eventAdd(Args &&...args);

	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEvent, T>::value>::type>
	void eventAddLater(Args &&...args);


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


	RpgGameData::CurrentSnapshot processEvents(const qint64 &tick);


	const RpgGameData::SnapshotList<RpgGameData::Player, RpgGameData::PlayerBaseData> &players();
	const RpgGameData::SnapshotList<RpgGameData::Enemy, RpgGameData::EnemyBaseData> &enemies();
	const RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData> &bullets();



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
