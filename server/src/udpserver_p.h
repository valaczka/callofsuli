/*
 * ---- Call of Suli ----
 *
 * udpengine_h.h
 *
 * Created on: 2025. 01. 02.
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

#ifndef UDPSERVER_P_H
#define UDPSERVER_P_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <enet/enet.h>
#include "udpserver.h"

/**
 * @brief The UdpServerPrivate class
 */

typedef QHash<UdpEngine*, UdpServerPeerReceivedList> UdpEngineReceived;



/**
 * @brief The UdpServerPrivate class
 */

class UdpServerPrivate : public QObject
{
	Q_OBJECT

public:
	UdpServerPrivate(UdpServer *engine)
		: QObject()
		, q(engine)
	{}

	virtual ~UdpServerPrivate();

	void run();

	void sendPacket(ENetPeer *peer, const QByteArray &data, const bool isReliable);

	QList<QByteArray> takePackets(UdpServerPeer *peer);
	QList<QByteArray> takePackets(UdpEngine *engine);
	UdpEngineReceived takePackets();


private:
	void peerConnect(ENetPeer *peer);
	void peerDisconnect(ENetPeer *peer);
	void udpPeerRemove(UdpServerPeer *peer);

	void packetReceived(const ENetEvent &event);
	void deliverReceived();
	void disconnectUnusedPeers();

	UdpServer *q;
	ENetHost *m_enet_server = nullptr;

	struct InOutCache {
		struct Packet {
			Packet(ENetPeer *p, const QByteArray &d, const bool r)
				: peer(p)
				, data(d)
				, reliable(r)
			{}

			ENetPeer *peer = nullptr;
			QByteArray data;
			bool reliable = false;
		};


		struct PacketRcv {
			PacketRcv(UdpServerPeer *p, const QByteArray &d, const qint64 _ts)
				: peer(p)
				, data(d)
				, ts(_ts)
			{}

			UdpServerPeer *peer = nullptr;
			QByteArray data;
			qint64 ts = -0;
		};


		QMutex mutex;
		std::vector<Packet> sendList;
		std::vector<PacketRcv> rcvList;
	};

	InOutCache m_inOutChache;

	friend class UdpServer;
};



#endif // UDPSERVER_P_H
