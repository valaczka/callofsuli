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


/**
 * @brief The RpgEnginePlayer class
 */

class RpgEnginePlayer : public RpgGameData::BaseData
{
public:
	RpgEnginePlayer(UdpServerPeer *peer, const bool &isHost = false)
		: RpgGameData::BaseData()
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

	RpgGameData::Player currentSnapshot() const;

private:
	UdpServerPeer *m_udpPeer = nullptr;
	bool m_isHost = false;
	bool m_isPrepared = false;
	RpgGameData::CharacterSelect m_config;

	std::vector<RpgGameData::Player> m_snapshots;

	friend class RpgEngine;
	friend class RpgEnginePrivate;

};



class RpgEnginePrivate;


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

	virtual void timerTick() override;

	virtual void binaryDataReceived(UdpServerPeer *peer, const QByteArray &data) override;
	virtual void udpPeerAdd(UdpServerPeer *peer) override;
	virtual void udpPeerRemove(UdpServerPeer *peer) override;

	RpgConfig config() const { return m_config; }
	void setConfig(const RpgConfig &newConfig) { m_config = newConfig; }

	RpgEnginePlayer *hostPlayer() const { return m_hostPlayer; }
	void setHostPlayer(RpgEnginePlayer *newHostPlayer);

	qint64 currentTick();

private:
	void preparePlayers();

	static int m_nextId;
	int m_nextPlayerId = 1;

	RpgEnginePrivate *d;

	RpgConfig m_config;
	std::vector<std::unique_ptr<RpgEnginePlayer>> m_player;
	std::vector<RpgGameData::Enemy> m_enemies;
	RpgEnginePlayer *m_hostPlayer = nullptr;

	qint64 m_startTick = 0;
	QElapsedTimer m_timer;

	friend class RpgEnginePrivate;
};

#endif // RPGENGINE_H
