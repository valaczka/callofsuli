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
#include "udpserver.h"


#define ELOG_TRACE            CuteMessageLogger(_logger(), Logger::Trace,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_DEBUG            CuteMessageLogger(_logger(), Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_INFO             CuteMessageLogger(_logger(), Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_WARNING          CuteMessageLogger(_logger(), Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_ERROR            CuteMessageLogger(_logger(), Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_FATAL            CuteMessageLogger(_logger(), Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO).write()


#define SLOG_TRACE(logger)            CuteMessageLogger(logger->_logger(), Logger::Trace,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define SLOG_DEBUG(logger)            CuteMessageLogger(logger->_logger(), Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define SLOG_INFO(logger)             CuteMessageLogger(logger->_logger(), Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO).write()
#define SLOG_WARNING(logger)          CuteMessageLogger(logger->_logger(), Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO).write()
#define SLOG_ERROR(logger)            CuteMessageLogger(logger->_logger(), Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define SLOG_FATAL(logger)            CuteMessageLogger(logger->_logger(), Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO).write()


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

	quint32 peerID() const;
	void setPeerID(quint32 newPeerID);

	const RpgGameData::PlayerPosition &startPosition() const;
	void setStartPosition(const RpgGameData::PlayerPosition &newStartPosition);

	bool isLost() const;
	void setIsLost(bool newIsLost);

private:
	UdpServerPeer *m_udpPeer = nullptr;
	bool m_isHost = false;
	bool m_isPrepared = false;
	bool m_isFullyPrepared = false;
	bool m_isLost = false;
	RpgGameData::CharacterSelect m_config;
	RpgGameData::PlayerPosition m_startPosition;

	bool m_finalSuccess = false;
	int m_gameId = -1;
	bool m_isFinishing = false;

	quint32 m_peerID = 0;

	QJsonObject m_final;

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
	Logger *_logger() const;

	RpgEngine *const m_engine;
	qint64 m_tick = -1;					// Mikor történt / fog történni
	const bool m_unique = true;			// Nem lehet ismételni
};




/**
 * @brief The RpgEngine class
 */

class RpgEngine : public UdpEngine
{
	Q_OBJECT

public:
	explicit RpgEngine(EngineHandler *handler, const RpgConfigBase &config, QObject *parent = nullptr);
	virtual ~RpgEngine();


	static std::shared_ptr<RpgEngine> engineCreate(EngineHandler *handler, const RpgConfigBase &config, UdpServer *server);
	static std::shared_ptr<RpgEngine> engineDispatch(EngineHandler *handler, const QJsonObject &connectionToken, UdpPacketRcv &&data);


	static std::shared_ptr<RpgEngine> peerFind(UdpServer *server, const QString &username, quint32 *idPtr = nullptr);
	bool peerAbort(const quint32 &peerId);

	virtual bool canDelete(const int &useCount) override;

	virtual void binaryDataReceived(const UdpServerPeerReceivedList &data) override;
	virtual void udpPeerAdd(UdpServerPeer *peer) override;
	virtual void udpPeerRemove(UdpServerPeer *peer) override;
	virtual void disconnectUnusedPeer(UdpServerPeer *peer) override;
	virtual bool isPeerValid(const quint32 &peerId) const override;

	virtual QString dumpEngine() const override;

	enum SentMessageTypes {
		MessageCollectAllRemaining
	};

	Q_ENUM(SentMessageTypes);

	const RpgConfig &config() const { return m_config; }

	RpgEnginePlayer *player(const RpgGameData::PlayerBaseData &base) const;
	RpgEnginePlayer *player(const quint32 &peerID) const;
	RpgEnginePlayer *playerSetGameCompleted(const RpgGameData::PlayerBaseData &base);
	RpgEnginePlayer *playerAddXp(const RpgGameData::PlayerBaseData &base, const int &xp, const bool &hasKill = false);

	bool playerSetFinal(const int &gameId, const QJsonObject &data);

	QCborArray getPlayerData(const bool &forced = false);

	const std::vector<std::unique_ptr<RpgEnginePlayer>> &playerList() const { return m_player; }

	RpgEnginePlayer *hostPlayer() const { return m_hostPlayer; }
	void setHostPlayer(RpgEnginePlayer *newHostPlayer);

	qint64 currentTick();
	int nextObjectId() const;

	void addMsec(const int &msec);

	void messageAdd(const RpgGameData::Message &msg, const QList<int> &players = {}, const bool &inverse = false);


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

	/*int createEvents(const qint64 &tick, const RpgGameData::BulletBaseData &data,
					 const RpgGameData::Bullet &snap, const std::optional<RpgGameData::Bullet> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlBaseData &data,
					 const RpgGameData::ControlLight &snap, const std::optional<RpgGameData::ControlLight> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlContainerBaseData &data,
					 const RpgGameData::ControlContainer &snap, const std::optional<RpgGameData::ControlContainer> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlCollectionBaseData &data,
					 const RpgGameData::ControlCollection &snap, const std::optional<RpgGameData::ControlCollection> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::PickableBaseData &data,
					 const RpgGameData::Pickable &snap, const std::optional<RpgGameData::Pickable> &prev);

	int createEvents(const qint64 &tick, const RpgGameData::ControlGateBaseData &data,
					 const RpgGameData::ControlGate &snap, const std::optional<RpgGameData::ControlGate> &prev);*/


	RpgGameData::CurrentSnapshot processEvents(const qint64 &tick);


	const RpgGameData::SnapshotList<RpgGameData::Player, RpgGameData::PlayerBaseData> &players();
	const RpgGameData::SnapshotList<RpgGameData::Enemy, RpgGameData::EnemyBaseData> &enemies();
	const RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData> &bullets();

	const RpgGameData::SnapshotList<RpgGameData::ControlLight, RpgGameData::ControlBaseData> &controlLights();
	const RpgGameData::SnapshotList<RpgGameData::ControlContainer, RpgGameData::ControlContainerBaseData> &controlContainers();
	const RpgGameData::SnapshotList<RpgGameData::ControlCollection, RpgGameData::ControlCollectionBaseData> &controlCollections();
	const RpgGameData::SnapshotList<RpgGameData::ControlGate, RpgGameData::ControlGateBaseData> &controlGates();
	const RpgGameData::SnapshotList<RpgGameData::Pickable, RpgGameData::PickableBaseData> &pickables();
	const RpgGameData::SnapshotList<RpgGameData::ControlTeleport, RpgGameData::ControlTeleportBaseData> &controlTeleports();

	void addRelocateCollection(const qint64 &tick, const RpgGameData::ControlCollectionBaseData &base,
							   const RpgGameData::PlayerBaseData &player, const bool &success);
	int relocateCollection(const RpgGameData::ControlCollectionBaseData &base, const qint64 &tick, QPointF *ptr = nullptr);
	bool finishCollection(const RpgGameData::ControlCollectionBaseData &base, const int &idx);
	void addPickablePicked(const qint64 &tick, const RpgGameData::PickableBaseData &base,
						   const RpgGameData::PlayerBaseData &player);
	void addContainerUsed(const qint64 &tick, const RpgGameData::ControlContainerBaseData &base,
						  const RpgGameData::PlayerBaseData &player, const bool &success);
	void addTeleportUsed(const qint64 &tick, const RpgGameData::ControlTeleportBaseData &base,
						 const RpgGameData::PlayerBaseData &player);

	void renderTimerLog(const qint64 &msec);

	int getCollected(const qint64 &tick, const RpgGameData::PlayerBaseData &player, int *leftPtr = nullptr);
	void checkPlayersCompleted();

	bool hasMessageSent(const SentMessageTypes &type) const { return m_sentMessages.contains(type); }
	void setMessageSent(const SentMessageTypes &type, const bool &on = true) {
		if (on)
			m_sentMessages.append(type);
		else
			m_sentMessages.removeAll(type);
	}

	Logger *_logger() const;

private:
	void setLoggerFile(const QString &fname);

	void binaryDataReceived(const UdpPacketRcv &recv);
	void preparePlayers();
	qint64 nextTick();

	int m_nextPlayerId = 1;
	int m_readableId = -1;

	RpgEnginePrivate *d;

	RpgConfig m_config;
	std::vector<std::unique_ptr<RpgEnginePlayer>> m_player;
	RpgEnginePlayer *m_hostPlayer = nullptr;

	RpgSnapshotStorage m_snapshots;

	qint64 m_currentTick = -1;
	qint64 m_deadlineTick = -1;

	QList<SentMessageTypes> m_sentMessages;

	friend class RpgEnginePrivate;
	friend class RpgSnapshotStorage;
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
	//LOG_CERROR("engine") << "Missing specialization";
	return -1;
}











#endif // RPGENGINE_H
