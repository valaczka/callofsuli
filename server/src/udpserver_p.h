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
#include "qjsonobject.h"
#include "udpbitstream.hpp"
#include "udpserver.h"
#include "udphelper.h"
#include <QPointer>

#include <entt/entity/registry.hpp>

class UdpServerPrivate;



/**
 * @brief The PeerData class
 */

struct PeerData
{
	quint32 peerId = 0;
	std::weak_ptr<UdpEngine> engine;
	AbstractEngine::Type type = AbstractEngine::EngineInvalid;
	UdpChallenge challenge;
	UdpAuthKey authKey;

	bool hasChallenge = false;
	bool hasAuthKey = false;

	QJsonObject connectionToken;			// azért QJsonObject, hogy pl. RpgConnectionToken is lehessen
	QString username;
	QDeadlineTimer deadline;

	void reset() {
		peerId = 0;
		engine.reset();
		type = AbstractEngine::EngineInvalid;
		challenge.fill(0);
		authKey.fill(0);
		hasChallenge = false;
		hasAuthKey = false;
		connectionToken = QJsonObject();
		username.clear();
		deadline.setRemainingTime(-1);
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

	bool removePeer(const quint32 &peerId);
	bool removeIndex(const quint32 &idx);

	std::optional<UdpBitStream> updateConnection(const quint32 &peerId, const AbstractEngine::Type &type, const QJsonObject &token);
	std::optional<UdpBitStream> updateChallenge(const UdpConnectionToken &connToken, const UdpChallengeResponseStream &stream, ENetPeer *peer);
	bool updateEngine(const quint32 &peerId, const std::shared_ptr<UdpEngine> &engine);

	void removeEngine(UdpEngine *engine);
	std::shared_ptr<UdpEngine> getEngineForUser(const AbstractEngine::Type &type, const QString &username, quint32 *idPtr) const;
	bool peerRemoveEngine(UdpServerPeer *peer);

	void removeExpiredPeers();

	QString dumpPeers() const;

private:
	std::optional<quint32> _nextUnusedIndex() const;
	std::optional<quint32> _index(const quint32 &peerId) const;

	std::array<PeerData, UdpBitStream::peerCapacity()> m_data;
	QHash<quint32, quint32> m_indexMap;

	UdpServerPrivate *m_server = nullptr;

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

private:
	void peerConnect(ENetPeer *peer);
	void peerDisconnect(ENetPeer *peer);
	void udpPeerRemove(ENetPeer *peer);
	UdpServerPeer* peerConnectToEngine(UdpServerPeer *peer, const std::shared_ptr<UdpEngine> &engine);
	bool peerReject(const quint32 &id, UdpServerPeer *peer);

	bool packetReceived(const ENetEvent &event);
	bool packetConnectReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event);
	bool packetChallengeReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event);
	bool packetUserReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event);

	static QByteArray hashToken(const QByteArray &token);
	static QByteArray hashToken(const uint8_t *data, const std::size_t &size);

	std::optional<QJsonObject> verifyToken(const QByteArray &token, QByteArray *hashPtr = nullptr);


	void deliverPackets();
	void disconnectUnusedPeers();

	UdpServer *q;
	ENetHost *m_enet_server = nullptr;

	std::unique_ptr<Lobby> m_lobby;


	QHash<QByteArray, qint64> m_connectTokenHash;
	QHash<QByteArray, ENetAddress> m_pendingClient;

	struct KeyPair {
		std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES> publicKey;
		std::array<std::uint8_t, crypto_box_SECRETKEYBYTES> secretKey;
	};

	KeyPair m_keyPair;

	QAtomicInt m_running{0};

	UdpCacheQueue<UdpPacketRcv> m_cacheRcv;
	UdpCacheQueue<UdpPacketSnd> m_cacheSnd;


	friend class UdpServer;
	friend class Lobby;
};



#endif // UDPSERVER_P_H
