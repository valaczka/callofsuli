/*
 * ---- Call of Suli ----
 *
 * udpengine.cpp
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


#include "udpserver.h"
#include <sodium/randombytes.h>
#include <sodium/crypto_generichash.h>
#include "udpbitstream.hpp"
#include "udpserver_p.h"
#include "serverservice.h"
#include "Logger.h"
#include <QJsonObject>
#include <QCborMap>
#include <QJsonDocument>
#include "rpgengine.h"




/**
 * @brief UdpEngine::dispatch
 * @param handler
 * @param type
 * @param server
 * @param data
 * @return
 */

std::shared_ptr<UdpEngine> UdpEngine::dispatch(EngineHandler *handler, const AbstractEngine::Type &type,
											   const QJsonObject &connectionToken,
											   const QByteArray &content, UdpServerPeer *peer)
{
	Q_ASSERT(handler);

	switch (type) {
		case AbstractEngine::EngineRpg:
			return RpgEngine::engineDispatch(handler, connectionToken, content, peer);
			break;

		case AbstractEngine::EngineInvalid:
			break;
	}

	return nullptr;
}


/**
 * @brief UdpEngine::onRemoveRequest
 */

void UdpEngine::onRemoveRequest()
{
	if (m_udpServer)
		m_udpServer->removeEngine(this);
}



/**
 * @brief UdpServer::UdpServer
 * @param handler
 * @param parent
 */

UdpServer::UdpServer(ServerService *service)
	: m_worker(new QLambdaThreadWorker)
	, m_service(service)
{
	LOG_CDEBUG("engine") << "Start UDP server";

	QDefer ret;
	m_worker->execInThread([this, ret]() mutable {
		d = new UdpServerPrivate(this);
		ret.resolve();
	});

	QDefer::await(ret);

	m_worker->execInThread(std::bind(&UdpServerPrivate::run, d));
}



/**
 * @brief UdpServer::~UdpServer
 */

UdpServer::~UdpServer()
{
	d->stop();
	m_worker->quitThread();
	m_worker->getThread()->wait();

	delete d;

	LOG_CTRACE("engine") << "UDP server destroyed";
}



/**
 * @brief UdpServer::send
 * @param peer
 * @param data
 * @param reliable
 */

void UdpServer::send(UdpServerPeer *peer, const std::vector<std::uint8_t> &data, const bool &reliable)
{
	if (!peer)
		return;

	m_worker->execInThread([this, peer, data, reliable]() {
		d->sendPacket(peer->peer(), data, reliable);
	});
}




/**
 * @brief UdpServer::removeEngine
 * @param engine
 */

void UdpServer::removeEngine(UdpEngine *engine)
{
	QDefer ret;

	m_worker->execInThread([this, engine, ret]() mutable {
		if (d->m_lobby)
			d->m_lobby->removeEngine(engine);

		ret.resolve();
	});

	QDefer::await(ret);
}





/**
 * @brief UdpServer::addPlayer
 * @param username
 * @return
 */

quint32 UdpServer::addPeer(const QString &username, const QDateTime &expired)
{
	if (!d->m_lobby) {
		LOG_CERROR("engine") << "Udp server add player failed:" << username << "no lobby";
		return 0;
	}

	if (auto ptr = d->m_lobby->add(username, expired)) {
		LOG_CDEBUG("engine") << "Udp server add player" << ptr->peerId << ptr->username;
		return ptr->peerId;
	}

	LOG_CERROR("engine") << "Udp server add player failed:" << username;

	return 0;
}




/**
 * @brief UdpServer::resetPeer
 * @param id
 * @param expired
 * @return
 */

quint32 UdpServer::resetPeer(const quint32 &id, const QString &username, const QDateTime &expired)
{
	if (!d->m_lobby) {
		LOG_CERROR("engine") << "Udp server player update failed:" << id << "no lobby";
		return 0;
	}

	auto ptr = d->m_lobby->reset(id, username, expired);

	return ptr ? ptr->peerId : 0;
}


/**
 * @brief UdpServer::removePeer
 * @param id
 * @return
 */

bool UdpServer::removePeer(const quint32 &id, UdpServerPeer *peer)
{
	return d->peerReject(id, peer);
}


/**
 * @brief UdpServer::peerConnectToEngine
 * @param peer
 * @param engine
 * @return
 */

bool UdpServer::peerConnectToEngine(UdpServerPeer *peer, const std::shared_ptr<UdpEngine> &engine)
{
	return d->peerConnectToEngine(peer, engine);
}


/**
 * @brief UdpServer::peerRemoveEngine
 * @param peer
 * @return
 */

bool UdpServer::peerRemoveEngine(UdpServerPeer *peer)
{
	if (d->m_lobby)
		return d->m_lobby->peerRemoveEngine(peer);
	else
		return false;
}


/**
 * @brief UdpServer::findEngineForUser
 * @param type
 * @param username
 * @return
 */

std::shared_ptr<UdpEngine> UdpServer::findEngineForUser(const AbstractEngine::Type &type, const QString &username, quint32 *idPtr) const
{
	if (d->m_lobby)
		return d->m_lobby->getEngineForUser(type, username, idPtr);
	else
		return nullptr;
}



/**
 * @brief UdpServer::dumpPeers
 * @return
 */

QString UdpServer::dumpPeers() const
{
	if (d->m_lobby)
		return d->m_lobby->dumpPeers();
	else
		return {};
}


/**
 * @brief UdpServer::removeExpiredPeers
 */

void UdpServer::removeExpiredPeers()
{
	if (d->m_lobby)
		d->m_lobby->removeExpiredPeers();
}







/**
 * @brief UdpServerPrivate::UdpServerPrivate
 * @param engine
 */

UdpServerPrivate::UdpServerPrivate(UdpServer *engine)
	: QObject()
	, q(engine)
{
	crypto_box_keypair(m_keyPair.publicKey.data(), m_keyPair.secretKey.data());
}




/**
 * @brief UdpServerPrivate::~UdpServerPrivate
 */


UdpServerPrivate::~UdpServerPrivate()
{
	if (m_enet_server) {
		enet_host_destroy(m_enet_server);
		m_enet_server = nullptr;
	}
}


/**
 * @brief UdpServerPrivate::run
 */

void UdpServerPrivate::run()
{
	Q_ASSERT(q->m_service);

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = q->m_service->settings()->listenPort();


	m_lobby.reset(new Lobby(this, std::max(4, q->m_service->settings()->udpMaxSeats())));

	m_enet_server = enet_host_create(&address,
									 m_lobby->size(),
									 1,
									 0, 0);

	if (m_enet_server == NULL) {
		LOG_CERROR("engine") << "UDP server run error";
		return;
	} else {
		LOG_CDEBUG("engine") << "UPD server started:" << m_lobby->size() << "seats";
	}


	m_running.storeRelease(1);


	ENetEvent event;

	while (int r = enet_host_service (m_enet_server, &event, 1) >= 0) {
		if (m_running.loadAcquire() < 1)
			break;

		// Read incoming packets

		if (r > 0) {

			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					peerConnect(event.peer);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					peerDisconnect(event.peer);
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					if (!packetReceived(event)) {
						LOG_CDEBUG("engine") << "Reject connection" << qPrintable(UdpServerPeer::address(event.peer));
						////enet_peer_disconnect_later(event.peer, 1);
					}
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}


		deliverPackets();

		//enet_host_flush(m_enet_server);		// Ez nem vsz., hogy kellene

		disconnectUnusedPeers();

		QThread::msleep(1);

	}

	m_lobby.reset();

	enet_host_destroy(m_enet_server);
	m_enet_server = nullptr;

	LOG_CDEBUG("engine") << "UDP server stopped";
}



/**
 * @brief UdpServerPrivate::stop
 */

void UdpServerPrivate::stop()
{
	LOG_CDEBUG("engine") << "Stop UDP server";
	m_running.storeRelease(0);
}




/**
 * @brief UdpServerPrivate::peerConnect
 * @param peer
 */

void UdpServerPrivate::peerConnect(ENetPeer *peer)
{
	Q_ASSERT(m_lobby);

	LOG_CDEBUG("engine") << "Peer connection start:" << qPrintable(UdpServerPeer::address(peer));

	/*QMutexLocker locker(&m_peerMutex);
	const qsizetype size = m_peerHash.size();
	locker.unlock();

	if (size >= m_maxPeers) {
		LOG_CWARNING("engine") << "Reject connection" << qPrintable(UdpServerPeer::address(peer)) << "seats:" << m_peerHash.size();

		sendPacket(peer, UdpBitStream(UdpBitStream::MessageServerFull).data(), true);

		enet_peer_disconnect_later(peer, 1);
		return;
	}*/
}


/**
 * @brief UdpServerPrivate::peerDisconnect
 * @param peer
 */

void UdpServerPrivate::peerDisconnect(ENetPeer *peer)
{
	if (!peer)
		return;

	LOG_CDEBUG("engine") << "Peer disconnected:" << qPrintable(UdpServerPeer::address(peer));

	udpPeerRemove(peer);
}


/**
 * @brief UdpServerPrivate::udpPeerRemove
 * @param peer
 */

void UdpServerPrivate::udpPeerRemove(ENetPeer *peer)
{
	Q_ASSERT(peer);

	UdpServerPeer *p = static_cast<UdpServerPeer*>(peer->data);

	if (p && p->engine())
		p->engine()->udpPeerRemove(p);

	m_cacheSnd.clearPeer(peer);
	m_cacheRcv.clearPeer(peer);

	std::erase_if(q->m_peerList, [peer](const std::unique_ptr<UdpServerPeer> &ptr) {
		return peer == ptr->peer();
	});

	peer->data = nullptr;
}



/**
 * @brief UdpServerPrivate::peerConnectToEngine
 * @param peer
 * @param engine
 * @return
 */

UdpServerPeer *UdpServerPrivate::peerConnectToEngine(UdpServerPeer *peer, const std::shared_ptr<UdpEngine> &engine)
{
	LOG_CDEBUG("engine") << "Connect peer to engine" << peer->peerID() << qPrintable(peer->address()) << "->" << engine->id();

	if (!peer || !engine)
		return nullptr;

	peer->setEngine(engine);
	engine->udpPeerAdd(peer);

	return peer;
}




/**
 * @brief UdpServerPrivate::peerReject
 * @param id
 * @param peer
 * @return
 */

bool UdpServerPrivate::peerReject(const quint32 &id, UdpServerPeer *peer)
{
	LOG_CDEBUG("engine") << "Udp server peer rejected" << id;

	if (peer) {
		peer->m_isRejected = true;
		sendPacket(peer->peer(), UdpBitStream(UdpBitStream::MessageRejected).data(), true);
	}

	if (m_lobby)
		m_lobby->removePeer(id);
	else
		return false;


	return true;
}




/**
 * @brief UdpServerPrivate::packetReceived
 * @param event
 */

bool UdpServerPrivate::packetReceived(const ENetEvent &event)
{
	const QString peerAddress = '[' + (event.peer ? UdpServerPeer::address(event.peer) : QStringLiteral("invalid peer")) + ']';

	if (!event.peer) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid peer";
		return false;
	}

	if (event.packet->dataLength <= 0) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid data from peer";
		return false;
	}


	UdpBitStream stream(event);

	if (!stream.validate()) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid data from peer";
		return false;
	}



	if (stream.type() == UdpBitStream::MessageConnect) {
		return packetConnectReceived(stream, event);

	} else if (stream.type() == UdpBitStream::MessageChallenge) {
		return packetChallengeReceived(stream, event);

	} else if (stream.type() >= UdpBitStream::MessageUser) {
		return packetUserReceived(stream, event);
	} else {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid message type from peer" << stream.type();
		return false;
	}

}



/**
 * @brief UdpServerPrivate::packetConnectReceived
 * @param data
 * @return
 */

bool UdpServerPrivate::packetConnectReceived(UdpBitStream &data, const ENetEvent &event)
{
	auto ptr = data.readByteArray();

	if (!ptr) {
		LOG_CWARNING("engine") << "Invalid connect message" << UdpServerPeer::address(event.peer);
		return false;
	}

	QByteArray hash;

	auto connectionToken = verifyToken(*ptr, &hash);

	if (!connectionToken) {
		LOG_CWARNING("engine") << "Invalid token" << UdpServerPeer::address(event.peer);
		return false;
	}

	UdpConnectionToken usertoken;
	usertoken.fromJson(*connectionToken);

	if (usertoken.exp <= QDateTime::currentSecsSinceEpoch()) {
		LOG_CWARNING("engine") << "Expired token" << usertoken.exp << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}

	AbstractEngine::Type engineType = static_cast<AbstractEngine::Type>(usertoken.type);

	if (engineType == AbstractEngine::EngineInvalid) {
		LOG_CWARNING("engine") << "Invalid engine type" << usertoken.type << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}

	auto stream = m_lobby->updateConnection(usertoken.peer, engineType, *connectionToken);

	if (!stream.has_value()) {
		LOG_CWARNING("engine") << "PeerId not found" << usertoken.peer << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}

	m_pendingClient.insert(hash, event.peer->address);

	sendPacket(event.peer, stream->data(), false);
}




/**
 * @brief UdpServerPrivate::packetChallengeReceived
 * @param data
 * @param event
 * @return
 */

bool UdpServerPrivate::packetChallengeReceived(UdpBitStream &data, const ENetEvent &event)
{
	QByteArray token;
	QByteArray encrypted;

	if (!data.getChallengeResponse(&token, &encrypted)) {
		LOG_CWARNING("engine") << "Invalid connect message" << UdpServerPeer::address(event.peer);
		return false;
	}

	if (encrypted.size() <= crypto_box_SEALBYTES) {
		LOG_CWARNING("engine") << "Buffer size error" << UdpServerPeer::address(event.peer);
		return false;
	}


	QByteArray hash;

	auto connectionToken = verifyToken(token, &hash);

	if (!connectionToken) {
		LOG_CWARNING("engine") << "Invalid token" << UdpServerPeer::address(event.peer);
		return false;
	}

	UdpConnectionToken usertoken;
	usertoken.fromJson(*connectionToken);

	static constexpr ENetAddress empty{.host = 0, .port = 0};

	const ENetAddress address = m_pendingClient.value(hash, empty);

	if (address.host != event.peer->address.host || address.port != event.peer->address.port) {
		LOG_CWARNING("engine") << "Invalid response (pending not found)" << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}





	const qsizetype len = encrypted.size() - crypto_box_SEALBYTES;

	std::vector<unsigned char> msg(len);

	if (crypto_box_seal_open(msg.data(),
							 reinterpret_cast<const unsigned char*>(encrypted.constData()),
							 encrypted.size(),
							 m_keyPair.publicKey.data(),
							 m_keyPair.secretKey.data()) == 0) {

		UdpChallengeResponseStream decrypted(msg);

		auto stream = m_lobby->updateChallenge(usertoken.peer, decrypted);

		if (!stream.has_value()) {
			LOG_CWARNING("engine") << "Challenge error" << usertoken.peer << usertoken.user << UdpServerPeer::address(event.peer);
			return false;
		}

		m_pendingClient.remove(hash);
		m_connectTokenHash.insert(hash, QDateTime::currentMSecsSinceEpoch());

		// Régieket töröljük

		m_connectTokenHash.removeIf([](const auto &ptr) {
			return ptr.value() < QDateTime::currentMSecsSinceEpoch() - 1000*60*240;
		});

		sendPacket(event.peer, stream->data(), false);

	} else {
		LOG_CWARNING("engine") << "Seal open error" << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}

	return true;
}




/**
 * @brief UdpServerPrivate::packetUserReceived
 * @param data
 * @param event
 * @return
 */

bool UdpServerPrivate::packetUserReceived(UdpBitStream &data, const ENetEvent &event)
{
	const auto &index = data.readPeerIndex();

	if (!index) {
		LOG_CWARNING("engine") << "Missing peerIndex" << UdpServerPeer::address(event.peer);
		return false;
	}

	const auto &peerData = m_lobby->at(*index);

	if (!peerData) {
		LOG_CWARNING("engine") << "Invalid peerIndex" << UdpServerPeer::address(event.peer);
		return false;
	}

	if (!peerData->hasAuthKey) {
		LOG_CWARNING("engine") << "Missing auth key" << peerData->username << UdpServerPeer::address(event.peer);
		return false;
	}

	const auto &lastPos = data.verifyBuffer(peerData->authKey);

	if (!lastPos) {
		LOG_CWARNING("engine") << "Authentication error" << peerData->username << UdpServerPeer::address(event.peer);
		return false;
	}

	data.setAuthLastPosition(*lastPos);

	UdpServerPeer *peer = static_cast<UdpServerPeer*>(event.peer->data);
	std::shared_ptr<UdpEngine> engine = peerData->engine.lock();

	if (engine && !peer) {
		LOG_CINFO("engine") << "Create UdpServerPeer for" << peerData->peerId << "to engine" << engine->id() << peerData->username << UdpServerPeer::address(event.peer);

		const std::unique_ptr<UdpServerPeer> &p = q->m_peerList.emplace_back(std::make_unique<UdpServerPeer>(peerData->peerId, q, event.peer));
		peer = p.get();
		p->peer()->data = peer;

		peer->server()->peerConnectToEngine(peer, engine);
	}

	peer->addRtt(event.peer->roundTripTime);

	std::size_t len = 0;
	const auto &rmData = data.getRemainingData(&len);

	if (!rmData) {
		LOG_CWARNING("engine") << "Buffer read error" << peerData->username << UdpServerPeer::address(event.peer);
		return false;
	}

	if (!engine) {
		QByteArray content = QByteArray::fromRawData(reinterpret_cast<const char*>(*rmData), len);

		std::shared_ptr<UdpEngine> peerEngine = UdpEngine::dispatch(q->m_service->engineHandler(),
																	peerData->type,
																	peerData->connectionToken,
																	content,
																	peer);

		if (peerEngine)
			m_lobby->updateEngine(peerData->peerId, peerEngine);

	} else {
		UdpPacketRcv packet;

		packet.peer = peer;
		packet.data.assign(*rmData, *rmData + len);

		m_cacheRcv.push(std::move(packet));
	}

	return true;
}




/**
 * @brief UdpServerPrivate::hashToken
 * @param token
 * @return
 */

QByteArray UdpServerPrivate::hashToken(const QByteArray &token)
{
	return hashToken((const unsigned char*) token.constData(), token.size());
}


/**
 * @brief UdpServerPrivate::hashToken
 * @param data
 * @param size
 * @return
 */

QByteArray UdpServerPrivate::hashToken(const uint8_t *data, const std::size_t &size)
{
	unsigned char hash[crypto_generichash_BYTES];

	crypto_generichash(hash, sizeof hash, data, size, NULL, 0);
	return QByteArray((const char*) hash, sizeof hash);
}


/**
 * @brief UdpServerPrivate::verifyToken
 * @param token
 * @param hashPtr
 * @return
 */

std::optional<QJsonObject> UdpServerPrivate::verifyToken(const QByteArray &token, QByteArray *hashPtr)
{
	const QByteArray &hash = hashToken(token);

	if (hashPtr)
		*hashPtr = hash;

	if (m_connectTokenHash.contains(hash)) {
		LOG_CWARNING("engine") << "Token already used";
		return std::nullopt;
	}

	Token jwt(token);

	if (!jwt.verify(q->m_service->settings()->jwtSecret())) {
		LOG_CWARNING("engine") << "Invalid token";
		return std::nullopt;
	}

	return jwt.payload();
}






/**
 * @brief UdpServerPrivate::deliverPackets
 */

void UdpServerPrivate::deliverPackets()
{
	// Send outgoing packets (always on channel 0)

	std::vector<UdpPacketSnd> out = m_cacheSnd.take();

	for (const UdpPacketSnd &p : out) {
		ENetPacket *packet = enet_packet_create(p.data.data(), p.data.size(),
												p.reliable ? ENET_PACKET_FLAG_RELIABLE :
															 0);
		enet_peer_send(p.peer, 0, packet);

	}


	// Deliver received packets

	std::vector<UdpPacketRcv> in = m_cacheRcv.take();

	QHash<UdpEngine*, UdpServerPeerReceivedList> engines;


	for (const auto &ptr : q->m_peerList) {
		UdpEngine *engine = ptr->engine().get();

		if (!engine)
			continue;

		engines.tryEmplace(engine);
	}


	for (UdpPacketRcv &p : in) {
		UdpEngine *engine = p.peer->engine().get();

		if (!engine)
			continue;

		engines[engine].emplace_back(std::move(p));
	}

	for (const auto &[e, list] : engines.asKeyValueRange())
		e->binaryDataReceived(list);
}



/**
 * @brief UdpServerPrivate::disconnectUnusedPeers
 */

void UdpServerPrivate::disconnectUnusedPeers()
{
	for (const auto &ptr : q->m_peerList) {
		if (ptr->m_isRejected)
			sendPacket(ptr->peer(), UdpBitStream(UdpBitStream::MessageRejected).data(), true);

		UdpEngine *e = ptr->engine().get();
		if (!e)
			continue;

		e->disconnectUnusedPeer(ptr.get());
	}
}





/**
 * @brief UdpServerPrivate::sendPacket
 * @param data
 * @param isReliable
 */

void UdpServerPrivate::sendPacket(ENetPeer *peer, const std::vector<uint8_t> &data, const bool isReliable)
{
	if (!peer)
		return;

	UdpPacketSnd packet;

	packet.peer = peer;
	packet.reliable = isReliable;
	packet.data = data;

	m_cacheSnd.push(std::move(packet));
}









/**
 * @brief UdpServerPeer::host
 * @return
 */

UdpServerPeer::UdpServerPeer(const quint32 &id, UdpServer *server, ENetPeer *peer)
	: m_peerID(id)
	, m_server(server)
	, m_peer(peer)
{
	LOG_CTRACE("engine") << "New peer" << id << this;
}


/**
 * @brief UdpServerPeer::~UdpServerPeer
 */

UdpServerPeer::~UdpServerPeer()
{
	LOG_CTRACE("engine") << "Delete peer" << m_peerID << this;
}





/**
 * @brief UdpServerPeer::host
 * @param peer
 * @return
 */

QString UdpServerPeer::host(ENetPeer *peer)
{
	if (!peer)
		return {};

	const unsigned int &address = peer->address.host;

	return QStringLiteral("%1.%2.%3.%4")
			.arg((address)&0xFF)
			.arg(((address)>>8)&0xFF)
			.arg(((address)>>16)&0xFF)
			.arg(((address)>>24)&0xFF);
}


/**
 * @brief UdpServerPeer::port
 * @param peer
 * @return
 */

int UdpServerPeer::port(ENetPeer *peer)
{
	if (!peer)
		return -1;
	else
		return peer->address.port;
}


/**
 * @brief UdpServerPeer::address
 * @param peer
 * @return
 */

QString UdpServerPeer::address(ENetPeer *peer)
{
	return host(peer).append(':').append(QString::number(port(peer)));
}


/**
 * @brief UdpServerPeer::send
 * @param data
 * @param reliable
 */

void UdpServerPeer::send(const std::vector<uint8_t> &data, const bool &reliable)
{
	if (!m_server) {
		LOG_CERROR("engine") << "Missing UdpServer";
		return;
	}

	m_server->send(this, data, reliable);
}




/**
 * @brief UdpServerPeer::readyToSend
 * @return
 */

bool UdpServerPeer::readyToSend(const int &maxFps)
{
	if (!m_speed.lastSent.isValid()) {
		m_speed.lastSent.start();
		return true;
	}

	if (m_speed.lastSent.hasExpired(1000./(float) m_speed.fps) &&
			(maxFps <= 0 || m_speed.lastSent.hasExpired(1000./(float) maxFps)) )
	{
		m_speed.lastSent.restart();
		return true;
	}

	return false;
}













/**
 * @brief UdpServerPeer::Speed::addRtt
 * @param rtt
 */

void UdpServerPeer::Speed::addRtt(const int &rtt)
{
	// Bejövö packet idejének rögzítése

	received.emplace_back(QDateTime::currentMSecsSinceEpoch());
	std::erase_if(received, [](const qint64 &ms) {
		return ms < QDateTime::currentMSecsSinceEpoch()-10000;
	});

	peerFps = received.size()/10.;

	currentRtt = rtt;

	const auto it = limit.upper_bound(rtt);

	// Ha túl nagy az rtt


	if (it != limit.cbegin()) {
		// Ha visszaestünk a rosszba (5 mp-en belül), duplázzuk a várakozási időt

		if (fps != maxFps && lastBad.isValid() && lastBad.elapsed() < 5000)
			delay = std::min(10000, delay*2);

		// Eddig nem váltunk vissza
		nextGood.setRemainingTime(delay);


		if (lastBad.isValid())
			lastBad.restart();
		else
			lastBad.start();

		lastGood.invalidate();

		if (const auto f = std::prev(it)->second; f < fps) {
			//LOG_CDEBUG("game") << "RTT=" << rtt << "SET FPS" << fps << "->" << f;
			fps = f;
		}

		return;
	}


	// Ha jó a helyzet

	if (!nextGood.hasExpired())
		return;

	if (fps != maxFps) {
		//LOG_CDEBUG("game") << "RTT=" << rtt << "SET FPS" << fps << "->" << maxFps;
		fps = maxFps;
	}

	// Mérjük, hogy mióta jó

	if (!lastGood.isValid()) {
		lastGood.start();
		return;
	}

	if (lastGood.hasExpired(5000)) {
		delay = std::max(1000, delay/2);
		lastGood.restart();
	}

}










/**
 * @brief Lobby::Lobby
 * @param size
 */

Lobby::Lobby(UdpServerPrivate *server, const quint32 &size)
	: m_server(server)
	, m_size(std::min(UdpBitStream::peerCapacity(), size))
{
	Q_ASSERT(size > 0);
}



/**
 * @brief Lobby::at
 * @param index
 * @return
 */

std::optional<PeerData> Lobby::at(const quint32 &index) const
{
	QMutexLocker l(&m_mutex);

	if (index >= m_size)
		return std::nullopt;

	return m_data[index];
}




/**
 * @brief Lobby::add
 * @param peerId
 * @return
 */

std::optional<PeerData> Lobby::add(const quint32 &peerId)
{
	if (peerId == 0)
		return std::nullopt;

	QMutexLocker l(&m_mutex);

	auto it = std::find_if(m_data.cbegin(),
						   m_data.cend(), [peerId](const PeerData &d){
		return d.peerId == peerId;
	});

	if (it != m_data.cend())
		return std::nullopt;

	auto idx = _nextUnusedIndex();

	if (!idx)
		return std::nullopt;

	PeerData &d = m_data[idx.value()];

	d.reset();
	d.peerId = peerId;

	m_indexMap[peerId] = idx.value();

	return d;
}


/**
 * @brief Lobby::index
 * @param peerId
 * @return
 */

std::optional<quint32> Lobby::index(const quint32 &peerId) const
{
	if (peerId == 0)
		return std::nullopt;

	QMutexLocker l(&m_mutex);

	return _index(peerId);
}



/**
 * @brief Lobby::removePeer
 * @param peerId
 * @return
 */

bool Lobby::removePeer(const quint32 &peerId)
{
	QMutexLocker l(&m_mutex);

	bool r = false;

	for (quint32 i=0; i<m_size; ++i) {
		if (m_data[i].peerId == peerId) {
			m_data[i].reset();
			r = true;
		}

	}

	m_indexMap.remove(peerId);

	return r;
}



/**
 * @brief Lobby::removeIndex
 * @param idx
 * @return
 */

bool Lobby::removeIndex(const quint32 &idx)
{
	QMutexLocker l(&m_mutex);

	if (idx >= m_size)
		return false;

	m_indexMap.remove(m_data[idx].peerId);
	m_data[idx].reset();

	return true;
}



/**
 * @brief Lobby::updateConnection
 * @param peerId
 * @param type
 * @param token
 * @return
 */

std::optional<UdpBitStream> Lobby::updateConnection(const quint32 &peerId, const AbstractEngine::Type &type, const QJsonObject &token)
{
	Q_ASSERT(m_server);

	if (peerId == 0)
		return std::nullopt;

	QMutexLocker l(&m_mutex);

	auto idx = _index(peerId);

	if (!idx.has_value())
		return std::nullopt;

	PeerData &d = m_data[idx.value()];

	if (d.hasAuthKey) {
		// Already connected

		return UdpBitStream(d.peerId, idx.value());

	} else {
		if (!d.hasChallenge) {
			randombytes_buf(d.challenge.data(), d.challenge.size());
			d.hasChallenge = true;
		}

		d.type = type;
		d.connectionToken = token;

		return UdpBitStream(d.challenge, m_server->m_keyPair.publicKey);
	}
}



/**
 * @brief Lobby::updateChallenge
 * @param peerId
 * @param stream
 * @return
 */

std::optional<UdpBitStream> Lobby::updateChallenge(const quint32 &peerId, const UdpChallengeResponseStream &stream)
{
	Q_ASSERT(m_server);

	if (peerId == 0)
		return std::nullopt;

	QMutexLocker l(&m_mutex);

	auto idx = _index(peerId);

	if (!idx.has_value())
		return std::nullopt;

	PeerData &d = m_data[idx.value()];

	if (!d.hasAuthKey) {
		// Not yet connected

		if (!d.hasChallenge) {
			LOG_CWARNING("engine") << "Missing challenge";
			return std::nullopt;
		}

		std::array<std::uint8_t, CHALLENGE_BYTES> challenge;
		std::array<std::uint8_t, crypto_auth_KEYBYTES> authKey;

		if (stream.getResponse(&challenge, &authKey)) {
			LOG_CWARNING("engine") << "Invalid response";
			return std::nullopt;
		}

		if (challenge != d.challenge) {
			LOG_CWARNING("engine") << "Challenge mismatch";
			return std::nullopt;
		}


		d.authKey = std::move(authKey);
		d.hasAuthKey = true;
	}

	return UdpBitStream(d.peerId, idx.value());
}


/**
 * @brief Lobby::updateEngine
 * @param peerId
 * @param engine
 * @return
 */

bool Lobby::updateEngine(const quint32 &peerId, const std::shared_ptr<UdpEngine> &engine)
{
	if (peerId == 0)
		return false;

	QMutexLocker l(&m_mutex);

	auto idx = _index(peerId);

	if (!idx.has_value())
		return false;

	PeerData &d = m_data[idx.value()];

	d.engine = engine;

	return true;
}



/**
 * @brief Lobby::removeEngine
 * @param engine
 */

void Lobby::removeEngine(UdpEngine *engine)
{
	if (!engine)
		return;

	LOG_CINFO("engine") << "Remove engine" << engine->type() << engine->id();

	QMutexLocker l(&m_mutex);

	for (PeerData &d : m_data) {
		if (d.engine.lock().get() == engine) {
			LOG_CDEBUG("engine") << "--- remove engine from" << d.peerId << d.username;
			m_indexMap.remove(d.peerId);
			d.reset();
		}
	}
}



/**
 * @brief Lobby::getEngineForUser
 * @param type
 * @param username
 * @param idPtr
 * @return
 */

std::shared_ptr<UdpEngine> Lobby::getEngineForUser(const AbstractEngine::Type &type, const QString &username, quint32 *idPtr) const
{
	QMutexLocker l(&m_mutex);

	for (const PeerData &d : m_data) {
		if (d.peerId == 0)
			continue;

		if (d.type == type && d.username == username) {
			if (idPtr)
				*idPtr = d.peerId;

			auto ptr = d.engine.lock();

			if (ptr && !ptr->isPeerValid(d.peerId))
				continue;

			return ptr;
		}
	}

	if (idPtr)
		*idPtr = 0;

	return {};
}



/**
 * @brief Lobby::peerRemoveEngine
 * @param peer
 * @return
 */

bool Lobby::peerRemoveEngine(UdpServerPeer *peer)
{
	if (!peer)
		return false;

	{
		QMutexLocker l(&m_mutex);

		auto idx = _index(peer->peerID());

		if (!idx) {
			LOG_CERROR("engine") << "PeerID not found" << peer->peerID();
			return false;
		} else {
			m_data[*idx].engine.reset();
		}
	}

	if (peer->engine()) {
		LOG_CDEBUG("engine") << "Peer remove from engine" << qPrintable(peer->address()) << "<-" << peer->engine()->id();
		peer->engine()->udpPeerRemove(peer);
		peer->setEngine(nullptr);

		return true;
	}

	return false;
}



/**
 * @brief Lobby::removeExpiredPeers
 */

void Lobby::removeExpiredPeers()
{
	QMutexLocker l(&m_mutex);

	for (PeerData &d : m_data) {
		if (!d.engine.lock() && d.deadline.hasExpired()) {
			m_indexMap.remove(d.peerId);
			d.reset();
		}
	}
}


/**
 * @brief Lobby::dumpPeers
 * @return
 */

QString Lobby::dumpPeers() const
{
	QString txt;

	///txt += QStringLiteral("SEATS\n");
	txt += QStringLiteral("==================================================================\n");

	QMutexLocker l(&m_mutex);

	int count = 0;

	for (const PeerData &d : m_data) {
		if (d.peerId == 0)
			continue;

		++count;

		UdpEngine *e = d.engine.lock().get();

		txt += QStringLiteral("%1: [%2] %3 (%4) %5\n")
			   .arg(d.peerId, 12)
			   .arg(d.hasAuthKey ? '*' : (d.hasChallenge ? '+' : ' '))
			   .arg(d.type)
			   .arg(e ? e->id() : 0, 3)
			   .arg(d.username)
			   ;
	}

	txt += QStringLiteral(" \n \n");

	txt.prepend(QStringLiteral("SEATS %1/%2\n").arg(count).arg(m_size));

	return txt;
}




/**
 * @brief Lobby::_nextUnusedIndex
 * @return
 */

std::optional<quint32> Lobby::_nextUnusedIndex() const
{
	/// Nincs mutex locker, azt a hívó metódusban kell megtenni!

	for (quint32 i=0; i<m_size; ++i) {
		if (m_data[i].peerId == 0)
			return i;
	}

	return std::nullopt;
}




/**
 * @brief Lobby::_index
 * @param peerId
 * @return
 */

std::optional<quint32> Lobby::_index(const quint32 &peerId) const
{
	/// Nincs mutex locker, azt a hívó metódusban kell megtenni!

	for (quint32 i=0; i<m_size; ++i) {
		if (m_data[i].peerId == peerId)
			return i;
	}

	return std::nullopt;
}





/**
 * @brief Lobby::peerAdd
 * @param username
 * @param expired
 * @return
 */

std::optional<PeerData> Lobby::add(const QString &username, const QDateTime &expired)
{
	QMutexLocker l(&m_mutex);

	auto idx = _nextUnusedIndex();

	if (!idx)
		return std::nullopt;

	std::set<quint32> used;

	for (const PeerData &p : m_data) {
		if (p.peerId > 0)
			used.insert(p.peerId);
	}

	for (int i=0; i<15; ++i) {
		quint32 newId = QRandomGenerator::global()->generate();

		if (newId > 0 && !used.contains(newId)) {
			LOG_CDEBUG("engine") << "Udp server add peer" << newId << username;


			PeerData &d = m_data[idx.value()];

			d.reset();
			d.peerId = newId;
			d.username = username;

			if (expired.isValid())
				d.deadline.setRemainingTime(QDateTime::currentDateTime().msecsTo(expired));
			else
				d.deadline.setRemainingTime(-1);

			m_indexMap[newId] = idx.value();

			return d;
		}
	}

	LOG_CERROR("engine") << "Udp server add peer failed:" << username;

	return std::nullopt;
}



/**
 * @brief Lobby::updateExpiry
 * @param peerId
 * @param expired
 * @return
 */

std::optional<PeerData> Lobby::updateExpiry(const quint32 &peerId, const QDateTime &expired)
{
	QMutexLocker l(&m_mutex);

	auto it = m_indexMap.find(peerId);

	if (it == m_indexMap.end())
		return std::nullopt;

	Q_ASSERT(it.value() < m_size);

	if (expired.isValid())
		m_data[*it].deadline.setRemainingTime(QDateTime::currentDateTime().msecsTo(expired));
	else
		m_data[*it].deadline.setRemainingTime(-1);

	return m_data[*it];
}


/**
 * @brief Lobby::reset
 * @param peerId
 * @return
 */

std::optional<PeerData> Lobby::reset(const quint32 &peerId, const QString &username, const QDateTime &expired)
{
	QMutexLocker l(&m_mutex);

	auto it = m_indexMap.find(peerId);

	if (it == m_indexMap.end())
		return std::nullopt;

	Q_ASSERT(it.value() < m_size);

	if (m_data[*it].engine.lock())
		return std::nullopt;

	m_data[*it].reset();
	m_data[*it].peerId = peerId;
	m_data[*it].username = username;

	if (expired.isValid())
		m_data[*it].deadline.setRemainingTime(QDateTime::currentDateTime().msecsTo(expired));
	else
		m_data[*it].deadline.setRemainingTime(-1);

	return m_data[*it];
}


