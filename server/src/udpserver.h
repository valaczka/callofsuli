/*
 * ---- Call of Suli ----
 *
 * udpengine.h
 *
 * Created on: 2025. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UdpEngine
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

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include "qlambdathreadworker.h"
#include <enet/enet.h>
#include <QThread>

class ServerService;
class UdpServer;
class UdpServerPrivate;
class UdpEngine;



/**
 * @brief The UdpServerPeer class
 */

class UdpServerPeer {
public:
	UdpServerPeer(const quint32 &id, UdpServer *server, ENetPeer *peer = nullptr);
	~UdpServerPeer();

	const quint32 &peerID() const { return m_peerID; }

	ENetPeer *peer() const { return m_peer; }
	void setPeer(ENetPeer *newPeer) { m_peer = newPeer; }

	UdpServer *server() const { return m_server; }

	QString host() const { return host(m_peer); }
	static QString host(ENetPeer *peer);

	int port() const { return port(m_peer); }
	static int port(ENetPeer *peer);

	QString address() const { return address(m_peer); }
	static QString address(ENetPeer *peer);

	std::shared_ptr<UdpEngine> engine() const { return m_engine; }
	void setEngine(const std::shared_ptr<UdpEngine> &newEngine) { m_engine = newEngine; }

	void send(const QByteArray &data, const bool &reliable);

	const qint64 &lastSentTick() const { return m_lastSentTick; }
	void setLastSentTick(const qint64 &newLastSentTick) { m_lastSentTick = newLastSentTick; }

	bool readyToSend(const int &maxFps = 0);
	void addRtt(const int &rtt) { m_speed.addRtt(rtt); }

	const int &currentRtt() const { return m_speed.currentRtt; }
	const int &currentFps() const { return m_speed.fps; }
	const int &peerFps() const { return m_speed.peerFps; }

	bool isReconnecting() const { return m_isReconnecting; }
	void setIsReconnecting(bool newIsReconnecting) { m_isReconnecting = newIsReconnecting; }

private:
	const quint32 m_peerID;
	UdpServer *m_server = nullptr;
	ENetPeer *m_peer = nullptr;
	std::shared_ptr<UdpEngine> m_engine;
	qint64 m_lastSentTick = -1;
	bool m_isReconnecting = false;

	struct Speed {
		void addRtt(const int &rtt);

		inline static constexpr int maxFps = 30;
		int fps = maxFps;

		// min rtt -> max fps
		inline static const std::map<int, int> limit = {
			{ 30,	30 },
			{ 75,	24 },
			{ 150,	20 },
			{ 200,	15 },
		};


		QElapsedTimer lastSent;
		QElapsedTimer lastBad;
		QElapsedTimer lastGood;
		QDeadlineTimer nextGood;
		int delay = 2000;
		int currentRtt = 0;
		int peerFps = 0;

		std::vector<qint64> received;
	};


	Speed m_speed;

	friend class UdpServerPrivate;
};



struct UdpServerPeerReceived {
	UdpServerPeer *peer = nullptr;
	qint64 diff = 0;
	QByteArray data;
};

typedef QList<UdpServerPeerReceived> UdpServerPeerReceivedList;



/**
 * @brief The UdpServer class
 */

class UdpServer
{

public:
	explicit UdpServer(ServerService *service);
	virtual ~UdpServer();

	const std::vector<std::unique_ptr<UdpServerPeer>> &peerList() const { return m_peerList; }

	void send(UdpServerPeer *peer, const QByteArray &data, const bool &reliable);

	void removeEngine(UdpEngine *engine);

	quint32 addPeer(const QString &username, const QDateTime &expired);
	bool peerConnectToEngine(UdpServerPeer *peer, const std::shared_ptr<UdpEngine> &engine);
	bool peerRemoveEngine(UdpServerPeer *peer);

	QString dumpPeers() const;
	void removeExpiredPeers();

private:
	UdpServerPrivate *d = nullptr;
	std::unique_ptr<QLambdaThreadWorker> m_worker;
	ServerService *m_service = nullptr;

	std::vector<std::unique_ptr<UdpServerPeer>> m_peerList;

	friend class UdpServerPrivate;
};

#endif // UDPSERVER_H
