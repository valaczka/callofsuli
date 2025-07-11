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
#include "udpserver_p.h"
#include "serverservice.h"
#include "Logger.h"
#include <QJsonObject>
#include <QCborMap>
#include <QJsonDocument>
#include "rpgengine.h"


#define SERVER_ENET_SPEED			1000./240.


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
	m_worker->getThread()->requestInterruption();
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

void UdpServer::send(UdpServerPeer *peer, const QByteArray &data, const bool &reliable)
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
		LOG_CINFO("engine") << "Remove engine" << engine->type() << engine->id();

		QMutexLocker peerLocker(&d->m_peerMutex);
		d->m_peerHash.removeIf([engine](const auto &ptr){
			return ptr.value().engine.lock().get() == engine;
		});

		ret.resolve();
	});

	QDefer::await(ret);
}





/**
 * @brief UdpServer::addPlayer
 * @param username
 * @return
 */

quint32 UdpServer::addPeer(const QString &username)
{
	QMutexLocker peerLocker(&d->m_peerMutex);

	for (int i=0; i<15; ++i) {
		quint32 newId = QRandomGenerator::global()->generate();

		if (newId > 0 && !d->m_peerHash.contains(newId)) {
			LOG_CDEBUG("engine") << "Udp server add player" << newId << username;

			d->m_peerHash.emplace(newId, username);

			return newId;
		}
	}

	LOG_CERROR("engine") << "Udp server add player failed:" << username;

	return 0;
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
	return d->peerRemoveEngine(peer);
}



/**
 * @brief UdpServer::dumpPeers
 * @return
 */

QString UdpServer::dumpPeers() const
{
	return d->dumpPeers();
}







/**
 * @brief UdpServerPrivate::UdpServerPrivate
 * @param engine
 */

UdpServerPrivate::UdpServerPrivate(UdpServer *engine)
	: QObject()
	, q(engine)
{
	unsigned char publicKey[crypto_box_PUBLICKEYBYTES];
	unsigned char secretKey[crypto_box_SECRETKEYBYTES];

	crypto_box_keypair(publicKey, secretKey);

	m_keyPair.publicKey = QByteArray(reinterpret_cast<char*>(publicKey), crypto_box_PUBLICKEYBYTES);
	m_keyPair.secretKey = QByteArray(reinterpret_cast<char*>(secretKey), crypto_box_SECRETKEYBYTES);
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

	m_maxPeers = std::max(4, q->m_service->settings()->udpMaxSeats());

	// Channel:
	// 0: unsigned
	// 1: signed

	m_enet_server = enet_host_create(&address,
									 2*m_maxPeers,
									 2,
									 0, 0);

	if (m_enet_server == NULL) {
		LOG_CERROR("engine") << "UDP server run error";
		return;
	} else {
		LOG_CDEBUG("engine") << "UPD server started:" << m_maxPeers << "seats";
	}


	ENetEvent event;

	while (int r = enet_host_service (m_enet_server, &event, SERVER_ENET_SPEED) >= 0) {
		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

		if (QThread::currentThread()->isInterruptionRequested())
			break;

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
						enet_peer_disconnect_later(event.peer, 1);
					}
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}


		QMutexLocker locker(&m_inOutChache.mutex);

		for (const auto &b : m_inOutChache.sendList) {
			QByteArray out;

			QDataStream stream(&out, QIODevice::WriteOnly);

			stream.setVersion(QDataStream::Qt_6_7);

			stream << (quint32) 0x434F53;			// COS
			stream << Utils::versionCode();
			stream << b.data;

			ENetPacket *packet = enet_packet_create(out.data(), out.size(),
													b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
			enet_peer_send(b.peer, 0, packet);
		}

		m_inOutChache.sendList.clear();

		locker.unlock();

		deliverReceived();

		disconnectUnusedPeers();

	}

	LOG_CTRACE("engine") << "UDP server stopped";
}



/**
 * @brief UdpServerPrivate::peerConnect
 * @param peer
 */

void UdpServerPrivate::peerConnect(ENetPeer *peer)
{
	LOG_CDEBUG("engine") << "Peer connection start:" << qPrintable(UdpServerPeer::address(peer));

	if (m_peerHash.size() >= m_maxPeers) {
		LOG_CDEBUG("engine") << "Reject connection" << qPrintable(UdpServerPeer::address(peer)) << "seats:" << m_peerHash.size();
		enet_peer_disconnect_later(peer, 1);

		sendPacket(peer, UdpServerResponse(UdpServerResponse::StateFull).toCborMap().toCborValue().toCbor(), false);
		return;
	}
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

	QMutexLocker locker(&m_inOutChache.mutex);

	std::erase_if(m_inOutChache.sendList, [peer](const auto &p) { return p.peer == peer; });

	if (p)
		std::erase_if(m_inOutChache.rcvList, [p](const auto &ptr) { return ptr.peer == p; });

	locker.unlock();

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
 * @brief UdpServerPrivate::peerRemoveEngine
 * @param peer
 * @return
 */

bool UdpServerPrivate::peerRemoveEngine(UdpServerPeer *peer)
{
	if (!peer)
		return false;

	LOG_CDEBUG("engine") << "Peer remove from engine" << qPrintable(peer->address()) << "<-" << peer->engine()->id();

	QMutexLocker peerLocker(&m_peerMutex);
	auto it = m_peerHash.find(peer->peerID());

	if (it == m_peerHash.end()) {
		LOG_CERROR("engine") << "PeerID not found" << peer->peerID();
	} else {
		it->engine.reset();
	}

	peerLocker.unlock();

	if (peer->engine()) {
		LOG_CDEBUG("engine") << "Peer remove from engine" << qPrintable(peer->address()) << "<-" << peer->engine()->id();
		peer->engine()->udpPeerRemove(peer);
		peer->setEngine(nullptr);

		return true;
	}



	return false;
}




/**
 * @brief UdpServerPrivate::packetReceived
 * @param event
 */

bool UdpServerPrivate::packetReceived(const ENetEvent &event)
{
	const QString peerAddress = '[' + (event.peer ? UdpServerPeer::address(event.peer) : QStringLiteral("invalid peer")) + ']';

	if (event.packet->dataLength <= 0) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid data from peer";
		return false;
	}

	QByteArray data = QByteArray::fromRawData((char*) event.packet->data, event.packet->dataLength);

	QDataStream stream(data);
	stream.setVersion(QDataStream::Qt_6_7);

	quint32 magic = 0;
	quint32 version = 0;

	stream >> magic >> version;

	if (magic != 0x434F53 || version == 0) {			// COS
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid data";
		return false;
	}


	stream.startTransaction();

	quint32 peerID = 0;

	stream >> peerID;

	if (!stream.commitTransaction()) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid stream";
		return false;
	}

	QByteArray mac;

	if (event.channelID == 1) {
		stream.startTransaction();

		stream >> mac;

		if (!stream.commitTransaction()) {
			LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid mac" << peerID;
			return false;
		}

	}

	UdpServerPeer *peer = static_cast<UdpServerPeer*>(event.peer->data);

	QByteArray content;

	stream >> content;


	QMutexLocker peerLocker(&m_peerMutex);
	QHash<quint32, PeerData>::iterator ptr = m_peerHash.find(peerID);

	if (ptr == m_peerHash.end()) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid peerID" << peerID;
		return false;
	}

	if (event.channelID == 1) {
		if (ptr->privateKey.isEmpty()) {
			LOG_CWARNING("engine") << qPrintable(peerAddress) << "Peer secret key missing" << peerID;
			return false;
		}

		if (!Token::verify(content, mac, ptr->privateKey)) {
			LOG_CWARNING("engine") << qPrintable(peerAddress) << "Peer sign error" << peerID;
			return false;
		}

		if (std::shared_ptr<UdpEngine> engine = ptr->engine.lock(); engine && !peer) {
			LOG_CINFO("engine") << qPrintable(peerAddress) << "Create UdpServerPeer for" << peerID << "to engine" << engine->id();
			const std::unique_ptr<UdpServerPeer> &p = q->m_peerList.emplace_back(std::make_unique<UdpServerPeer>(ptr.key(), q, event.peer));
			peer = p.get();
			p->peer()->data = peer;

			peer->server()->peerConnectToEngine(peer, engine);
		}


		peer->addRtt(event.peer->roundTripTime);

		if (!ptr->engine.lock()) {
			updateEngine(ptr, content, peer);
		} else {
			// Ha a peer be van állítva, akkor az már "connected"

			peerLocker.unlock();

			QMutexLocker locker(&m_inOutChache.mutex);
			m_inOutChache.rcvList.emplace_back(peer, content, event.channelID);
		}

		return true;
	}


	if (peer && event.peer != peer->peer()) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Peer mismatch" << peerID;
		return false;
	}

	return updateChallenge(ptr, event.peer, content);
}






/**
 * @brief UdpServerPrivate::updateChallenge
 * @param iterator
 * @param content
 * @return
 */

bool UdpServerPrivate::updateChallenge(const QHash<quint32, PeerData>::iterator &iterator, ENetPeer *peer, const QByteArray &content)
{
	const QCborMap cbor = QCborValue::fromCbor(content).toMap();

	if (iterator->privateKey.isEmpty()) {
		if (iterator->challenge.isEmpty()) {
			// Még nem volt challenge request

			UdpConnectRequest rq;
			rq.fromCbor(cbor);

			if (rq.token.isEmpty()) {
				LOG_CWARNING("engine") << "Empty token" << iterator.key();
				return false;
			}

			const QByteArray &hash = hashToken(rq.token);

			if (m_connectTokenHash.contains(hash)) {
				LOG_CWARNING("engine") << "Token already used" << iterator.key();
				return false;
			}

			Token jwt(rq.token);

			if (!jwt.verify(q->m_service->settings()->jwtSecret())) {
				LOG_CWARNING("engine") << "Invalid token" << iterator.key();
				return false;
			}

			QJsonObject connectionToken = jwt.payload();

			UdpToken usertoken;
			usertoken.fromJson(connectionToken);

			if (usertoken.exp <= QDateTime::currentSecsSinceEpoch()) {
				LOG_CWARNING("engine") << "Expired token" << iterator.key();
				return false;
			}

			if (usertoken.peerID != iterator.key() || usertoken.type == UdpToken::Invalid) {
				LOG_CWARNING("engine") << "Invalid token" << iterator.key() << usertoken.peerID << usertoken.type;
				return false;
			}


			const std::pair<enet_uint32, enet_uint16> address(peer->address.host, peer->address.port);

			m_pendingClient.insert(hash, address);

			static constexpr size_t size = 32;
			char buf[size];

			randombytes_buf(buf, size);

			iterator->challenge = QByteArray(buf, size);
			iterator->type = usertoken.type;
			iterator->connectionToken = connectionToken;

		} else {
			// Már volt challenge request

			UdpChallengeResponse rsp;
			rsp.fromCbor(cbor);

			if (rsp.token.isEmpty()) {
				LOG_CWARNING("engine") << "Empty token" << iterator.key();
				return false;
			}

			if (rsp.response.size() > crypto_box_SEALBYTES) {
				const qsizetype len = rsp.response.size() - crypto_box_SEALBYTES;
				unsigned char *msg = (unsigned char*) malloc(len);

				if (crypto_box_seal_open(msg,
										 reinterpret_cast<const unsigned char*>(rsp.response.constData()),
										 rsp.response.size(),
										 reinterpret_cast<const unsigned char*>(m_keyPair.publicKey.constData()),
										 reinterpret_cast<const unsigned char*>(m_keyPair.secretKey.constData())) == 0) {

					const QByteArray content = QByteArray::fromRawData(reinterpret_cast<const char*>(msg), len);

					UdpChallengeResponseContent rc;
					rc.fromCbor(QCborValue::fromCbor(content).toMap());

					free(msg);

					if (rc.challenge == iterator->challenge && !rc.key.isEmpty()) {
						static constexpr std::pair<enet_uint32, enet_uint16> empty(0, 0);
						const std::pair<enet_uint32, enet_uint16> address(peer->address.host, peer->address.port);

						const QByteArray &hash = hashToken(rsp.token);

						if (m_pendingClient.value(hash, empty) != address) {
							LOG_CWARNING("engine") << "Invalid response (pending not found)" << iterator.key();
							return false;
						}

						m_pendingClient.remove(hash);
						m_connectTokenHash.insert(hash, QDateTime::currentMSecsSinceEpoch());

						m_connectTokenHash.removeIf([](const auto &ptr) {
							return ptr.value() < QDateTime::currentMSecsSinceEpoch() - 1000*60*240;
						});

						iterator->challenge.clear();
						iterator->privateKey = rc.key;

						return true;

					} else {
						LOG_CWARNING("engine") << "Invalid response" << iterator.key();
					}

				} else {
					LOG_CWARNING("engine") << "Invalid response" << iterator.key();

					free(msg);

					return false;
				}


				// free(msg); ^^^

			}

			// Nem jött válasz, küljdük újra
		}


		// Ha nem dolgoztuk fel korábban, akkor küldjük a challenget

		UdpChallengeRequest chr(iterator->challenge, m_keyPair.publicKey);

		sendPacket(peer, chr.toCborMap().toCborValue().toCbor(), false);

	} else {
		if (peer->data == nullptr) {
			// Már megvan a private key, beállítjuk a peert

			const std::unique_ptr<UdpServerPeer> &p = q->m_peerList.emplace_back(std::make_unique<UdpServerPeer>(iterator.key(), q, peer));
			p->peer()->data = p.get();

			iterator->challenge.clear();

			LOG_CDEBUG("engine") << "Peer connected" << qPrintable(p->address()) << "to id" << iterator.key() << "user:" << qPrintable(iterator->username);

		}

		sendPacket(peer, UdpServerResponse(UdpServerResponse::StateConnected).toCborMap().toCborValue().toCbor(), false);
	}

	return true;
}




/**
 * @brief UdpServerPrivate::updateEngine
 * @param iterator
 * @param content
 * @return
 */

bool UdpServerPrivate::updateEngine(const QHash<quint32, PeerData>::iterator &iterator, const QByteArray &content, UdpServerPeer *peer)
{
	std::shared_ptr<UdpEngine> engine = UdpEngine::dispatch(q->m_service->engineHandler(),
															iterator->type,
															iterator->connectionToken,
															content,
															peer);

	if (engine) {
		iterator->engine = engine;
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
	unsigned char hash[crypto_generichash_BYTES];

	crypto_generichash(hash, sizeof hash, (const unsigned char*) token.constData(), token.size(), NULL, 0);
	return QByteArray((const char*) hash, sizeof hash);
}



/**
 * @brief UdpServerPrivate::dumpPeers
 * @return
 */

QString UdpServerPrivate::dumpPeers() const
{
	QString txt;

	txt += QStringLiteral("SEATS\n");
	txt += QStringLiteral("==================================================================\n");


	for (const auto &[id, data] : m_peerHash.asKeyValueRange()) {
		UdpEngine *e = data.engine.lock().get();

		txt += QStringLiteral("%1: [%2] %3 (%4) %5\n")
			   .arg(id, 12)
			   .arg(!data.privateKey.isEmpty() ? '*' : (data.challenge.isEmpty() ? ' ' : '+'))
			   .arg(data.type)
			   .arg(e ? e->id() : 0, 3)
			   .arg(data.username)
			   ;

	}

	txt += QStringLiteral(" \n \n");

	return txt;
}





/**
 * @brief UdpServerPrivate::deliverReceived
 */

void UdpServerPrivate::deliverReceived()
{
	const UdpEngineReceived &p = takePackets();

	QList<UdpEngine*> engines;

	for (const auto &ptr : q->m_peerList) {
		auto *e = ptr->engine().get();
		if (!e)
			continue;

		if (!engines.contains(e))
			engines.append(e);
	}

	for (const auto &[e, list] : p.asKeyValueRange()) {
		e->binaryDataReceived(list);
		engines.removeAll(e);
	}

	for (UdpEngine *e : engines)
		e->binaryDataReceived({});
}



/**
 * @brief UdpServerPrivate::disconnectUnusedPeers
 */

void UdpServerPrivate::disconnectUnusedPeers()
{
	for (const auto &ptr : q->m_peerList) {
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

void UdpServerPrivate::sendPacket(ENetPeer *peer, const QByteArray &data, const bool isReliable)
{
	if (!peer)
		return;

	QMutexLocker locker(&m_inOutChache.mutex);

	m_inOutChache.sendList.emplace_back(peer, data, isReliable);
}




/**
 * @brief UdpServerPrivate::takePackets
 * @return
 */

UdpEngineReceived UdpServerPrivate::takePackets()
{
	QMutexLocker locker(&m_inOutChache.mutex);

	UdpEngineReceived r;

	for (const auto &it : m_inOutChache.rcvList) {
		if (!it.peer) {
			LOG_CERROR("engine") << "Invalid peer";
			continue;
		}

		UdpEngine *engine = it.peer->engine().get();

		if (!engine) {
			LOG_CERROR("engine") << "Invalid engine";
			continue;
		}

		r[engine].append(QPair<UdpServerPeer *, QByteArray>(it.peer, it.data));
	}

	m_inOutChache.rcvList.clear();

	return r;
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

void UdpServerPeer::send(const QByteArray &data, const bool &reliable)
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
