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
#include <sodium/crypto_box.h>
#include "credential.h"
#include "udpserver.h"
#include <QPointer>

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
	UdpServerPrivate(UdpServer *engine);
	virtual ~UdpServerPrivate();

	void run();

	void sendPacket(ENetPeer *peer, const QByteArray &data, const bool isReliable);

	UdpEngineReceived takePackets();


private:
	struct PeerData {
		PeerData(const QString _user)
			: username(_user)
		{}

		std::weak_ptr<UdpEngine> engine;
		UdpToken::Type type = UdpToken::Invalid;
		QByteArray challenge;
		QByteArray privateKey;					// non emtpy = connected
		QJsonObject connectionToken;			// azért QJsonObject, hogy pl. RpgConnectionToken is lehessen
		QString username;

		QDeadlineTimer deadline;
	};

	void peerConnect(ENetPeer *peer);
	void peerDisconnect(ENetPeer *peer);
	void udpPeerRemove(ENetPeer *peer);
	UdpServerPeer* peerConnectToEngine(UdpServerPeer *peer, const std::shared_ptr<UdpEngine> &engine);
	bool peerRemoveEngine(UdpServerPeer *peer);

	bool packetReceived(const ENetEvent &event);
	bool updateChallenge(const QHash<quint32, PeerData>::iterator &iterator, ENetPeer *peer, const QByteArray &content);
	bool updateEngine(const QHash<quint32, PeerData>::iterator &iterator, const QByteArray &content, UdpServerPeer *peer);
	static QByteArray hashToken(const QByteArray &token);


	QString dumpPeers() const;
	void removeExpiredPeers();

	void deliverReceived();
	void disconnectUnusedPeers();

	UdpServer *q;
	ENetHost *m_enet_server = nullptr;

	int m_maxPeers = 0;

	mutable QMutex m_peerMutex;
	QHash<quint32, PeerData> m_peerHash;


	QHash<QByteArray, qint64> m_connectTokenHash;
	QHash<QByteArray, std::pair<enet_uint32, enet_uint16>> m_pendingClient;

	struct KeyPair {
		QByteArray publicKey;
		QByteArray secretKey;
	};

	KeyPair m_keyPair;


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
			PacketRcv(UdpServerPeer *p, const QByteArray &d, const enet_uint8 &_ch)
				: peer(p)
				, data(d)
				, channel(_ch)
			{}

			UdpServerPeer *peer = nullptr;
			QByteArray data;
			enet_uint8 channel = 0;
		};


		QMutex mutex;
		std::vector<Packet> sendList;
		std::vector<PacketRcv> rcvList;
	};

	InOutCache m_inOutChache;

	friend class UdpServer;
};



#endif // UDPSERVER_P_H
