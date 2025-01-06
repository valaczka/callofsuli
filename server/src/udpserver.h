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
	UdpServerPeer(UdpServer *server, ENetPeer *peer = nullptr);
	~UdpServerPeer();

	void packetReceived(const int &channel, ENetPacket *packet);

	ENetPeer *peer() const { return m_peer; }
	void setPeer(ENetPeer *newPeer) { m_peer = newPeer; }

	QString host() const { return host(m_peer); }
	static QString host(ENetPeer *peer);

	int port() const { return port(m_peer); }
	static int port(ENetPeer *peer);

	std::shared_ptr<UdpEngine> engine() const { return m_engine; }
	void setEngine(const std::shared_ptr<UdpEngine> &newEngine) { m_engine = newEngine; }

	void send(const QByteArray &data, const bool &reliable);

private:
	UdpServer *m_server = nullptr;
	ENetPeer *m_peer = nullptr;
	std::shared_ptr<UdpEngine> m_engine;

	friend class UdpServerPrivate;
};







/**
 * @brief The UdpServerThread class
 */

class UdpServerThread : public QThread
{
	Q_OBJECT

public:
	UdpServerThread(UdpServer *server)
		: QThread()
		, q(server)
	{  }

	virtual ~UdpServerThread() {}
	void run() override;
	UdpServer *q;
};




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

private:
	UdpServerPrivate *d = nullptr;
	UdpServerThread m_dThread;
	ServerService *m_service = nullptr;

	std::vector<std::unique_ptr<UdpServerPeer>> m_peerList;

	friend class UdpServerPrivate;
	friend class UdpServerThread;
};

#endif // UDPSERVER_H
