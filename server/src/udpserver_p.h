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
#include "qjsonobject.h"
#include "udpbitstream.hpp"
#include "udpserver.h"
#include "udphelper.h"
#include <QPointer>

#include <entt/entity/registry.hpp>

#include <QRandomGenerator>

class UdpServerPrivate;




struct PeerData;



/**
 * @brief The PeerData class
 */

struct PeerData : public UdpPeerData
{
	UdpChallenge challenge;
	std::optional<PublicKeySigner> signer = std::nullopt;

	bool hasChallenge = false;

	QDeadlineTimer deadline;
	QByteArray publicKey;
	UdpRoom *room = nullptr;

	void reset(const bool &holdRoom = false) {
		peerId = 0;
		challenge.fill(0);
		signer = std::nullopt;
		hasChallenge = false;
		session.clear();
		publicKey.clear();
		connectionToken = QJsonObject();
		username.clear();
		deadline.setRemainingTime(-1);

		// Ha nem vesszük ki a szobából (csak peer-t cserélünk pl. kapcsolatmegszakadás miatt)

		if (!holdRoom) {
			type = UdpType::EngineInvalid;
			room = nullptr;
		}
	}
};







/**
 * @brief The Lobby class
 */

class Lobby
{
public:
	Lobby(UdpServerPrivate *server, const quint32 &size);

	quint32 size() { return m_size; }

	std::optional<PeerData> at(const quint32 &index) const;
	std::optional<PeerData> get(const quint32 &peerId) const;

	std::optional<PeerData> add(const quint32 &peerId);
	std::optional<PeerData> add(const QString &username, const QDateTime &expired);
	std::optional<PeerData> updateExpiry(const quint32 &peerId, const QDateTime &expired);
	std::optional<PeerData> reset(const quint32 &peerId, const QString &username, const QDateTime &expired);

	std::optional<quint32> index(const quint32 &peerId) const;

	UdpRoom *createRoom();
	const UdpRoom *findRoom(const std::function<bool (const UdpRoom *)> &fn) const;

	bool removePeer(const quint32 &peerId);
	bool removeIndex(const quint32 &idx);

	std::optional<UdpBitStream> updateConnection(const UdpConnectionToken &token, const UdpType &type,
												 const QJsonObject &tokenObj, QWebSocket *socket);
	std::optional<UdpBitStream> updateChallenge(const UdpConnectionToken &connToken, const QByteArray &content, ENetPeer *peer);

	void removeExpiredPeers();

	QString dumpPeers() const;

	QSet<UdpEngine*> engines() const;
	QSet<UdpEngine*> engines(const UdpType &type) const;

	template <class T, typename = std::enable_if<std::is_base_of<UdpEngine, T>::value>::type>
	QSet<T*> engines(const UdpType &type) const {
		QSet<UdpEngine*> list = engines(type);
		QSet<T*> ret;
		ret.reserve(list.size());
		for (UdpEngine *e : list) {
			if (T* ee = qobject_cast<T*>(e))
				ret.insert(ee);
		}
		return ret;
	}

private:
	std::optional<quint32> _nextUnusedIndex() const;
	std::optional<quint32> _index(const quint32 &peerId) const;

	std::array<PeerData, UdpBitStream::peerCapacity()> m_data;
	QHash<quint32, quint32> m_indexMap;

	std::array<UdpRoom, UdpBitStream::peerCapacity()> m_rooms;

	UdpServer *m_server = nullptr;
	UdpServerPrivate *m_serverPrivate = nullptr;

	const quint32 m_size;

	mutable QMutex m_mutex;
};





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
	void stop();

	void sendPacket(ENetPeer *peer, const std::vector<std::uint8_t> &data, const bool isReliable);
	void sendPacket(UdpServerPeer *peer, const std::vector<std::uint8_t> &data, const bool isReliable);

	static void sendPacket(QWebSocket *socket, const std::vector<std::uint8_t> &data);
	static void sendHello(QWebSocket *socket);
	static void closeSocket(QWebSocket *socket, const QString &error = QString());

private:
	void peerConnect(ENetPeer *peer);
	void peerDisconnect(ENetPeer *peer);
	void udpPeerRemove(ENetPeer *peer);

	bool packetReceived(const ENetEvent &event);
	bool packetConnectReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event);
	bool packetChallengeReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event);
	bool packetUserReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event);

	void websocketAdd(QPointer<QWebSocket> socket);
	void websocketRemove(QWebSocket *socket);
	void websocketBinaryReceived(const QByteArray &data);
	void websocketTextReceived(const QString &text);
	void websocketDisconnected();

	void packetConnectReceived(std::unique_ptr<UdpBitStream> &&data, QWebSocket *socket);
	void packetUserReceived(std::unique_ptr<UdpBitStream> &&data, QWebSocket *socket);

	void peerWithoutRoomHandle(std::unique_ptr<UdpBitStream> &&data, UdpServerPeer *peer);

	static QByteArray hashToken(const QByteArray &token);
	static QByteArray hashToken(const uint8_t *data, const std::size_t &size);

	std::optional<QJsonObject> verifyToken(const QByteArray &token, QByteArray *hashPtr = nullptr);


	void deliverPackets();
	void disconnectUnusedPeers();

	UdpServer *q;
	ENetHost *m_enet_server = nullptr;

	std::unique_ptr<Lobby> m_lobby;

	QElapsedTimer m_timer;

	QHash<QByteArray, qint64> m_connectTokenHash;
	QHash<QByteArray, ENetAddress> m_pendingClient;
	std::vector<std::unique_ptr<QWebSocket> > m_pendingSocket;

	struct KeyPair {
		std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> publicKey;
		std::array<std::uint8_t, crypto_box_SECRETKEYBYTES> secretKey;
	};

	KeyPair m_keyPair;

	QAtomicInt m_running{0};

	UdpCacheQueue<UdpPacketRcv> m_cacheRcv;
	UdpCacheQueue<UdpPacketSnd> m_cacheSnd;
	std::unordered_map<UdpServerPeer*, UdpCacheQueue<UdpPacketSnd> > m_cacheSndPeer;


	friend class UdpServer;
	friend class Lobby;
};



#endif // UDPSERVER_P_H
