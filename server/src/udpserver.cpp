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
#include "rpgengine.h"
#include <QJsonObject>
#include <QCborMap>
#include <QJsonDocument>








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
	LOG_CDEBUG("engine") << "Destroy UDP server";

	if (m_worker->getThread()->isFinished())
		return;

	QMetaObject::invokeMethod(d, &UdpServerPrivate::stop, Qt::BlockingQueuedConnection);

	m_worker->quitThread();
	m_worker->getThread()->wait();

	delete d;
	d = nullptr;

	LOG_CTRACE("engine") << "UDP server destroyed";
}




/**
 * @brief UdpServer::sendAll
 * @param data
 * @param fn
 * @param reliable
 */

void UdpServer::sendAll(const std::vector<uint8_t> &data, const std::function<bool (UdpServerPeer *)> &fn, const bool &reliable)
{
	for (const auto &ptr : m_peerList) {
		if (fn && !fn(ptr.get()))
			continue;

		send(ptr.get(), data, reliable);
	}
}


/**
 * @brief UdpServer::sendRoomList
 * @param type
 * @param reliable
 */

void UdpServer::sendRoomList(const UdpType &type, const bool &reliable)
{
	switch (type) {
		case EngineRpg:
			RpgEngine::sendRoomList(this, d->m_lobby->engines<RpgEngine>(EngineRpg), reliable);
			break;

		case EngineInvalid:
			LOG_CERROR("engine") << "Invalid type" << type;
			break;
	}
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

	if (peer->peer()) {
		if (m_worker->getThread() == QThread::currentThread())
			d->sendPacket(peer->peer(), data, reliable);
		else
			m_worker->execInThread([this, peer, data, reliable]() {
				d->sendPacket(peer->peer(), data, reliable);
			});
	}

	if (peer->socket()) {
		if (m_worker->getThread() == QThread::currentThread())
			d->sendPacket(peer->socket(), data);
		else
			m_worker->execInThread([this, peer, data]() {
				d->sendPacket(peer->socket(), data);
			});
	}
}




/**
 * @brief UdpServer::removeEngine
 * @param engine
 */
/*
void UdpServer::removeEngine(UdpEngine *engine)
{
	if (m_worker->getThread() == QThread::currentThread()) {
		if (d->m_lobby)
			d->m_lobby->removeEngine(engine);
	} else {
		QDefer ret;

		m_worker->execInThread([this, engine, ret]() mutable {
			if (d->m_lobby)
				d->m_lobby->removeEngine(engine);

			ret.resolve();
		});

		QDefer::await(ret);
	}
}
*/




/**
 * @brief UdpServer::addPlayer
 * @param username
 * @return
 */

quint32 UdpServer::addPeer(const QString &username, const QDateTime &expired)
{
	if (!d->m_lobby) {
		LOG_CERROR("engine") << "Udp server add peer failed:" << username << "no lobby";
		return 0;
	}

	if (auto ptr = d->m_lobby->add(username, expired)) {
		LOG_CDEBUG("engine") << "Udp server add peer" << ptr->peerId << ptr->username;
		return ptr->peerId;
	}

	LOG_CERROR("engine") << "Udp server add peer failed:" << username;

	return 0;
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
 * @brief UdpServer::websocketAdd
 * @param socket
 * @return
 */

void UdpServer::websocketAdd(QWebSocket *socket)
{
	LOG_CDEBUG("service") << "Add WebSocket" << socket;

	QMetaObject::invokeMethod(d, std::bind(&UdpServerPrivate::websocketAdd, d, socket), Qt::QueuedConnection);
}



/**
 * @brief UdpServer::service
 * @return
 */

ServerService *UdpServer::service() const
{
	return m_service;
}


/**
 * @brief UdpServer::createRoom
 * @param type
 * @return
 */

UdpRoom *UdpServer::createRoom()
{
	return d->m_lobby ? d->m_lobby->createRoom() : nullptr;
}


/**
 * @brief UdpServer::setRoom
 * @param index
 * @param room
 */

void UdpServer::setRoom(const quint32 &index, UdpRoom *room)
{
	if (d->m_lobby)
		d->m_lobby->setRoom(index, room);
}


/**
 * @brief UdpServer::findRoom
 * @param fn
 * @return
 */

const UdpRoom *UdpServer::findRoom(const std::function<bool (const UdpRoom *)> &fn) const
{
	return d->m_lobby && fn ? d->m_lobby->findRoom(fn) : nullptr;
}


/**
 * @brief UdpServer::findEngine
 * @param fn
 * @return
 */

UdpEngine *UdpServer::findEngine(const std::function<bool (const UdpRoom *)> &fn) const
{
	const UdpRoom *room = findRoom(fn);

	return room ? room->engine() : nullptr;
}



/**
 * @brief UdpServer::removeEngine
 * @param engine
 */

void UdpServer::removeEngine(UdpEngine *engine)
{
	LOG_CDEBUG("engine") << "REMOVE ENGINE" << engine;

	Q_ASSERT(engine);

	UdpRoom *room = engine->room();

	Q_ASSERT(room);

	d->m_lobby->removeRoom(room);
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
									 2,
									 0, 0);

	if (m_enet_server == NULL) {
		LOG_CERROR("engine") << "UDP server run error";
		return;
	} else {
		LOG_CDEBUG("engine") << "UPD server started:" << m_lobby->size() << "seats";
	}


	m_running.storeRelease(1);

	m_timer.start();

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
						enet_peer_disconnect_later(event.peer, 1);
					}
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}

		// Read websocket events

		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);


		deliverPackets();

		disconnectUnusedPeers();


		// Main tick (120 fps)

		if (qint64 dt = m_timer.elapsed(); dt >= 1000./120.) {
#ifdef WITH_FTXUI
			QString txt = q->dumpPeers();
#endif

			for (UdpEngine *engine : m_lobby->engines()) {
				engine->udpTimerEvent(dt);

#ifdef WITH_FTXUI
				txt += engine->dumpEngine();
#endif
			}

#ifdef WITH_FTXUI
			QCborMap m;
			m.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
			m.insert(QStringLiteral("txt"), txt);
			q->m_service->writeToSocket(m.toCborValue());
#endif

			m_timer.restart();
		}


		QThread::msleep(1);

	}

	m_lobby.reset();

	enet_host_flush(m_enet_server);

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
 * @brief UdpServerPrivate::sendHello
 * @param socket
 */

void UdpServerPrivate::sendHello(QWebSocket *socket)
{
	Q_ASSERT(socket);

	static const QJsonObject data{
		{ QStringLiteral("versionMajor"), ServerService::versionMajor() },
		{ QStringLiteral("versionMinor"), ServerService::versionMinor() },
	};

	QMetaObject::invokeMethod(socket, std::bind(&QWebSocket::sendTextMessage, socket,
												QString::fromUtf8(QJsonDocument(data).toJson())), Qt::QueuedConnection);
}



/**
 * @brief UdpServerPrivate::closeSocket
 * @param socket
 * @param error
 */

void UdpServerPrivate::closeSocket(QWebSocket *socket, const QString &error)
{
	if (!socket)
		return;

	socket->close(QWebSocketProtocol::CloseCodeProtocolError, error);
}



/**
 * @brief UdpServerPrivate::sendPacket
 * @param socket
 * @param data
 */

void UdpServerPrivate::sendPacket(QWebSocket *socket, const std::vector<uint8_t> &data)
{
	if (!socket)
		return;

	QByteArray d = QByteArray(reinterpret_cast<const char*>(data.data()), data.size());
	QMetaObject::invokeMethod(socket, std::bind(&QWebSocket::sendBinaryMessage, socket, d), Qt::QueuedConnection);
}




/**
 * @brief UdpServerPrivate::sendPacket
 * @param peer
 * @param data
 * @param isReliable
 */

void UdpServerPrivate::sendPacket(UdpServerPeer *peer, const std::vector<uint8_t> &data, const bool isReliable)
{
	if (!peer)
		return;

	UdpPacketSnd packet;

	packet.peer = peer->peer();
	packet.reliable = isReliable;
	packet.data = data;

	m_cacheSndPeer[peer].push(std::move(packet));
}




/**
 * @brief UdpServerPrivate::peerConnect
 * @param peer
 */

void UdpServerPrivate::peerConnect(ENetPeer *peer)
{
	Q_ASSERT(m_lobby);

	LOG_CDEBUG("engine") << "Peer connection start:" << qPrintable(UdpServerPeer::address(peer));

	if (q->m_peerList.size() >= m_lobby->size()) {
		LOG_CWARNING("engine") << "Reject connection" << qPrintable(UdpServerPeer::address(peer)) << "seats:" << m_lobby->size();

		sendPacket(peer, UdpBitStream(UdpBitStream::MessageServerFull).data(), true);

		enet_peer_disconnect_later(peer, 1);
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

	if (p && p->room())
		p->room()->peerRemove(p);

	m_cacheSnd.clearPeer(peer);
	m_cacheRcv.clearPeer(peer);

	if (const auto it = m_cacheSndPeer.find(p); it != m_cacheSndPeer.cend())
		m_cacheSndPeer.erase(it);


	std::erase_if(q->m_peerList, [p](const std::unique_ptr<UdpServerPeer> &ptr) {
		return ptr.get() == p;
	});

	peer->data = nullptr;
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


	std::unique_ptr<UdpBitStream> stream = std::make_unique<UdpBitStream>(event);

	if (!stream->validate()) {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid data from peer";
		return false;
	}


	if (stream->type() == UdpBitStream::MessageConnect) {
		return packetConnectReceived(std::move(stream), event);

	} else if (stream->type() == UdpBitStream::MessageChallenge) {
		return packetChallengeReceived(std::move(stream), event);

	} else if (stream->type() >= UdpBitStream::MessageUser) {
		return packetUserReceived(std::move(stream), event);
	} else {
		LOG_CWARNING("engine") << qPrintable(peerAddress) << "Invalid message type from peer" << stream->type();
		return false;
	}

}



/**
 * @brief UdpServerPrivate::packetConnectReceived
 * @param data
 * @return
 */

bool UdpServerPrivate::packetConnectReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event)
{
	auto ptr = data->readByteArray();

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

	UdpType engineType = static_cast<UdpType>(usertoken.type);

	if (engineType == UdpType::EngineInvalid) {
		LOG_CWARNING("engine") << "Invalid engine type" << usertoken.type << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}

	auto stream = m_lobby->updateConnection(usertoken, engineType, *connectionToken, nullptr);

	if (!stream.has_value()) {
		LOG_CWARNING("engine") << "PeerId not found" << usertoken.peer << usertoken.user << UdpServerPeer::address(event.peer);
		return false;
	}

	m_pendingClient.insert(hash, event.peer->address);

	sendPacket(event.peer, (*stream).data(), false);

	return true;
}




/**
 * @brief UdpServerPrivate::packetChallengeReceived
 * @param data
 * @param event
 * @return
 */

bool UdpServerPrivate::packetChallengeReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event)
{
	if (!data)
		return false;

	if (data->type() != UdpBitStream::MessageChallenge)
		return false;

	auto ptrToken = data->readByteArray();

	if (!ptrToken)
		return false;

	auto ptrSignedContent = data->readByteArray();

	if (!ptrSignedContent)
		return false;


	QByteArray hash;

	auto connectionToken = verifyToken(*ptrToken, &hash);

	if (!connectionToken) {
		LOG_CWARNING("engine") << "Invalid token" << UdpServerPeer::address(event.peer);
		return false;
	}

	static constexpr ENetAddress empty{.host = 0, .port = 0};

	const ENetAddress address = m_pendingClient.value(hash, empty);

	if (address.host != event.peer->address.host || address.port != event.peer->address.port) {
		LOG_CWARNING("engine") << "Invalid response (pending not found)" << UdpServerPeer::address(event.peer);
		return false;
	}

	UdpConnectionToken usertoken;
	usertoken.fromJson(*connectionToken);

	auto stream = m_lobby->updateChallenge(usertoken, *ptrSignedContent, event.peer);

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

	return true;
}




/**
 * @brief UdpServerPrivate::packetUserReceived
 * @param data
 * @param event
 * @return
 */

bool UdpServerPrivate::packetUserReceived(std::unique_ptr<UdpBitStream> &&data, const ENetEvent &event)
{
	const auto &index = data->readPeerIndex();

	if (!index) {
		LOG_CWARNING("engine") << "Missing peerIndex" << UdpServerPeer::address(event.peer);
		return false;
	}

	const auto &peerData = m_lobby->at(*index);

	if (!peerData) {
		LOG_CWARNING("engine") << "Invalid peerIndex" << UdpServerPeer::address(event.peer);
		return false;
	}

	if (!peerData->signer.has_value()) {
		LOG_CWARNING("engine") << "Missing public key" << peerData->peerId << peerData->username << UdpServerPeer::address(event.peer);
		return false;
	}

	const auto &lastPos = data->verifyBuffer(peerData->signer.value());

	if (!lastPos) {
		LOG_CWARNING("engine") << "Authentication error" << peerData->peerId << peerData->username << UdpServerPeer::address(event.peer);
		return false;
	}

	data->setAuthLastPosition(*lastPos);

	UdpServerPeer *peer = static_cast<UdpServerPeer*>(event.peer->data);

	if (!peer) {
		LOG_CINFO("engine") << "Create UdpServerPeer" << peerData->peerId << peerData->username << UdpServerPeer::address(event.peer);

		const std::unique_ptr<UdpServerPeer> &p = q->m_peerList.emplace_back(std::make_unique<UdpServerPeer>(index.value(), peerData.value(), q, event.peer));
		peer = p.get();
		p->peer()->data = peer;
	}


	peer->addRtt(event.peer->roundTripTime);


	if (!peer->room() || !peer->room()->engine()) {
		peerWithoutRoomHandle(std::move(data), peer);
		return true;
	}

	UdpPacketRcv packet(std::move(data));
	packet.peer = peer;

	m_cacheRcv.push(std::move(packet));

	return true;
}




/**
 * @brief UdpServerPrivate::websocketAdd
 * @param socket
 */

void UdpServerPrivate::websocketAdd(QPointer<QWebSocket> socket)
{
	if (!socket)
		return;

	Q_ASSERT(m_lobby);

	LOG_CDEBUG("engine") << "Websocket connection start:" << qPrintable(UdpServerPeer::address(socket)) << socket;

	if (q->m_peerList.size() >= m_lobby->size()) {
		LOG_CWARNING("engine") << "Reject connection" << qPrintable(UdpServerPeer::address(socket)) << "seats:" << m_lobby->size();

		sendPacket(socket, UdpBitStream(UdpBitStream::MessageServerFull).data());

		socket->close(QWebSocketProtocol::CloseCodeNormal, QStringLiteral("server full"));
		socket->deleteLater();
		return;
	}

	auto &ptr = m_pendingSocket.emplace_back(std::move(socket));


	LOG_CDEBUG("engine") << "TO PENDING:" << qPrintable(UdpServerPeer::address(ptr.get())) << ptr.get();

	connect(ptr.get(), &QWebSocket::disconnected, this, &UdpServerPrivate::websocketDisconnected, Qt::QueuedConnection);
	connect(ptr.get(), &QWebSocket::textMessageReceived, this, &UdpServerPrivate::websocketTextReceived, Qt::QueuedConnection);
	connect(ptr.get(), &QWebSocket::binaryMessageReceived, this, &UdpServerPrivate::websocketBinaryReceived, Qt::QueuedConnection);
}



/**
 * @brief UdpServerPrivate::websocketRemove
 * @param socket
 */

void UdpServerPrivate::websocketRemove(QWebSocket *socket)
{
	Q_ASSERT(socket);

	const auto it = std::find_if(q->m_peerList.cbegin(),
								 q->m_peerList.cend(),
								 [socket](const std::unique_ptr<UdpServerPeer> &ptr) {
		return ptr.get()->socket() == socket;
	});

	UdpServerPeer *p = (it == q->m_peerList.cend() ? nullptr : it->get());

	if (p && p->room())
		p->room()->peerRemove(p);

	m_cacheSnd.clearSocket(socket);
	m_cacheRcv.clearSocket(socket);

	q->m_websocketHash.remove(socket);

	if (const auto it = m_cacheSndPeer.find(p); it != m_cacheSndPeer.cend())
		m_cacheSndPeer.erase(it);

	std::erase_if(m_pendingSocket,
				  [socket](const auto &ptr) {
		return ptr.get() == socket;
	});


	if (it != q->m_peerList.cend()) {
		q->m_peerList.erase(it);
	}
}




/**
 * @brief UdpServerPrivate::websocketBinaryReceived
 * @param data
 */

void UdpServerPrivate::websocketBinaryReceived(const QByteArray &data)
{
	QWebSocket *ws = qobject_cast<QWebSocket*>(sender());

	if (!ws)
		return;

	std::unique_ptr<UdpBitStream> st = std::make_unique<UdpBitStream>(reinterpret_cast<const unsigned char*>(data.constData()),
																	  data.size());


	if (!st->validate()) {
		LOG_CWARNING("engine") << "Invalid data from peer" << qPrintable(UdpServerPeer::address(ws));
		closeSocket(ws, QStringLiteral("invalid data"));
		return;
	}


	if (st->type() == UdpBitStream::MessageConnect) {
		return packetConnectReceived(std::move(st), ws);
	} else if (st->type() >= UdpBitStream::MessageUser) {
		return packetUserReceived(std::move(st), ws);

	} else {
		LOG_CWARNING("engine") << "Invalid message type from peer" << qPrintable(UdpServerPeer::address(ws));
		closeSocket(ws, QStringLiteral("invalid nessage"));
		return;
	}
}



/**
 * @brief UdpServerPrivate::websocketTextReceived
 * @param text
 */

void UdpServerPrivate::websocketTextReceived(const QString &text)
{
	QWebSocket *ws = qobject_cast<QWebSocket*>(sender());

	LOG_CDEBUG("service") << "WebSocket received" << ws << text;

	if (!ws)
		return;

	sendHello(ws);
}





/**
 * @brief UdpServerPrivate::websocketDisconnected
 */

void UdpServerPrivate::websocketDisconnected()
{
	QWebSocket *ws = qobject_cast<QWebSocket*>(sender());

	LOG_CDEBUG("service") << "WebSocket disconnected:" << ws;

	if (!ws)
		return;

	websocketRemove(ws);
}






/**
 * @brief UdpServerPrivate::packetConnectReceived
 * @param data
 * @param socket
 */

void UdpServerPrivate::packetConnectReceived(std::unique_ptr<UdpBitStream> &&data, QWebSocket *socket)
{
	if (!socket)
		return;

	auto ptr = data->readByteArray();

	if (!ptr) {
		LOG_CWARNING("engine") << "Invalid connect message" << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	QByteArray hash;

	auto connectionToken = verifyToken(*ptr, &hash);

	if (!connectionToken) {
		LOG_CWARNING("engine") << "Invalid token" << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	UdpConnectionToken usertoken;
	usertoken.fromJson(*connectionToken);

	if (usertoken.exp <= QDateTime::currentSecsSinceEpoch()) {
		LOG_CWARNING("engine") << "Expired token" << usertoken.exp << usertoken.user << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	UdpType engineType = static_cast<UdpType>(usertoken.type);

	if (engineType == UdpType::EngineInvalid) {
		LOG_CWARNING("engine") << "Invalid engine type" << usertoken.type << usertoken.user << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	auto s = m_lobby->updateConnection(usertoken, engineType, *connectionToken, socket);

	if (!s.has_value()) {
		LOG_CWARNING("engine") << "PeerId not found" << usertoken.peer << usertoken.user << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	sendPacket(socket, s->data());


	auto itP = std::find_if(m_pendingSocket.begin(),
							m_pendingSocket.end(),
							[socket](const auto &ptr) {
		return ptr.get() == socket;
	});


	auto itR = std::find_if(q->m_peerList.cbegin(),
							q->m_peerList.cend(),
							[socket](const auto &ptr) {
		return ptr->socket() == socket;
	});


	if (itP == m_pendingSocket.end() && itR == q->m_peerList.cend()) {
		LOG_CERROR("engine") << "Websocket storage error";
		closeSocket(socket);
		return;
	}


	if (itR == q->m_peerList.cend()) {
		const auto &index = m_lobby->index(usertoken.peer);

		if (!index) {
			LOG_CERROR("engine") << "Websocket storage error";
			closeSocket(socket);
			return;
		}

		const auto &peerData = m_lobby->at(index.value());

		QWebSocket *ws = itP->release();

		LOG_CDEBUG("engine") << "===" << ws;

		m_pendingSocket.erase(itP);

		LOG_CDEBUG("engine") << "===2" << ws;


		LOG_CINFO("engine") << "Create UdpServerPeer" << peerData->peerId << peerData->username << qPrintable(UdpServerPeer::address(ws));
		//LOG_CINFO("engine") << "Create UdpServerPeer engine:" << engine->id() << "peer:" << peerData->peerId << peerData->username << UdpServerPeer::address(event.peer);

		const std::unique_ptr<UdpServerPeer> &p = q->m_peerList.emplace_back(std::make_unique<UdpServerPeer>(index.value(), peerData.value(), q, nullptr));

		std::unique_ptr<QWebSocket> ptr(std::move(ws));

		p->setSocket(std::move(ptr));

		LOG_CDEBUG("engine") << "===3" << p->socket();

		q->m_websocketHash.insert(ws, p.get());

		/*if (peerData->room)
			p->server()->peerConnectToEngine(p.get(), peerData->room->engine.lock());*/

	}
}





/**
 * @brief UdpServerPrivate::packetUserReceived
 * @param data
 * @param socket
 */

void UdpServerPrivate::packetUserReceived(std::unique_ptr<UdpBitStream> &&data, QWebSocket *socket)
{
	if (!socket)
		return;

	const auto &index = data->readPeerIndex();

	if (!index) {
		LOG_CWARNING("engine") << "Missing peerIndex" << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	UdpServerPeer *peer = q->m_websocketHash.value(socket);

	if (!peer) {
		LOG_CERROR("engine") << "Missing websocket hash" << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	const auto &peerData = m_lobby->at(*index);

	if (!peerData) {
		LOG_CWARNING("engine") << "Invalid peerIndex" << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	if (!peerData->signer.has_value()) {
		LOG_CWARNING("engine") << "Missing public key" << peerData->peerId << peerData->username << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	const auto &lastPos = data->verifyBuffer(peerData->signer.value());

	if (!lastPos) {
		LOG_CWARNING("engine") << "Authentication error" << peerData->peerId << peerData->username << qPrintable(UdpServerPeer::address(socket));
		closeSocket(socket);
		return;
	}

	data->setAuthLastPosition(*lastPos);

	if (!peer->room() || !peer->room()->engine()) {
		peerWithoutRoomHandle(std::move(data), peer);
		return;
	}

	UdpPacketRcv packet(std::move(data));
	packet.peer = peer;

	m_cacheRcv.push(std::move(packet));
}



/**
 * @brief UdpServerPrivate::peerWithoutRoomHandle
 * @param data
 * @param peer
 */

void UdpServerPrivate::peerWithoutRoomHandle(std::unique_ptr<UdpBitStream> &&data, UdpServerPeer *peer)
{
	if (!peer)
		return;

	LOG_CDEBUG("engine") << "HANDLE" << data.get() << peer << peer->peerData().type;

	switch (peer->peerData().type) {
		case EngineRpg:
			RpgEngine::peerWithoutRoomHandle(std::move(data), peer, m_lobby->engines<RpgEngine>(EngineRpg));
			break;

		case EngineInvalid:
			LOG_CERROR("engine") << "Invalid engine type" << peer->peerData().type << qPrintable(peer->address());
			sendPacket(peer, UdpBitStream(UdpBitStream::MessageRejected).data(), false);
			break;
	}
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
	QByteArray hash(crypto_generichash_BYTES, Qt::Uninitialized);

	crypto_generichash(reinterpret_cast<unsigned char*>(hash.data()),
					   hash.size(),
					   data, size,
					   NULL, 0);
	return hash;
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
	// Send outgoing packets (channel 0 = normal, channel 1 = rliable)

	std::vector<UdpPacketSnd> out = m_cacheSnd.take();

	for (const UdpPacketSnd &p : out) {
		ENetPacket *packet = enet_packet_create(p.data.data(), p.data.size(),
												p.reliable ? ENET_PACKET_FLAG_RELIABLE :
															 0);
		enet_peer_send(p.peer, p.reliable ? 1 : 0, packet);

	}

	for (auto &[peer, cache] : m_cacheSndPeer) {
		if (!peer->readyToSend())
			continue;

		std::vector<UdpPacketSnd> out = cache.take();

		for (const UdpPacketSnd &p : out) {
			if (p.peer) {
				ENetPacket *packet = enet_packet_create(p.data.data(), p.data.size(),
														p.reliable ? ENET_PACKET_FLAG_RELIABLE :
																	 0);
				enet_peer_send(p.peer, p.reliable ? 1 : 0, packet);

			} else if (p.socket) {
				sendPacket(p.socket, p.data);
			} else {
				LOG_CERROR("engine") << "Missing peer/socket";
			}

		}
	}


	// Deliver received packets

	std::vector<UdpPacketRcv> in = m_cacheRcv.take();

	std::unordered_map<UdpEngine*, UdpServerPeerReceivedList> engines;
	engines.reserve(m_lobby->engines().size());


	for (UdpEngine *engine : m_lobby->engines()) {
		auto p = engines.try_emplace(engine);
		p.first->second.reserve(in.size());
	}


	for (UdpPacketRcv &p : in) {
		UdpEngine *engine = (p.peer && p.peer->room() ? p.peer->room()->engine() : nullptr);

		if (!engine) {
			LOG_CERROR("engine") << "Packet without engine";
			continue;
		}

		engines[engine].emplace_back(std::move(p));
	}

	for (auto &[e, list] : engines)
		e->binaryDataReceived(list);
}





/**
 * @brief UdpServerPrivate::disconnectUnusedPeers
 */

void UdpServerPrivate::disconnectUnusedPeers()
{
	/*for (const auto &ptr : q->m_peerList) {
		if (ptr->m_isRejected)
			sendPacket(ptr->peer(), UdpBitStream(UdpBitStream::MessageRejected).data(), true);

		UdpEngine *e = ptr->engine().get();
		if (!e)
			continue;

		e->disconnectUnusedPeer(ptr.get());
	}*/

	for (UdpEngine *engine : m_lobby->engines()) {
		if (engine->canRemove())
			q->removeEngine(engine);
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

UdpServerPeer::UdpServerPeer(const quint32 &peerIndex, const UdpPeerData &data, UdpServer *server, ENetPeer *peer)
	: m_peerIndex(peerIndex)
	, m_peerData(data)
	, m_server(server)
	, m_peer(peer)
{
	LOG_CTRACE("engine") << "New peer" << m_peerData.peerId << this;

	m_speed.maxFps = 30;
}


/**
 * @brief UdpServerPeer::~UdpServerPeer
 */

UdpServerPeer::~UdpServerPeer()
{
	LOG_CTRACE("engine") << "Delete peer" << m_peerData.peerId << this;
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

	return UdpAddress::host(peer->address);
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
		return UdpAddress::port(peer->address);
}


/**
 * @brief UdpServerPeer::address
 * @return
 */

QString UdpServerPeer::address() const
{
	if (m_peer)
		return address(m_peer);

	if (m_socket)
		return address(m_socket.get());

	return QString();
}


/**
 * @brief UdpServerPeer::address
 * @param peer
 * @return
 */

QString UdpServerPeer::address(ENetPeer *peer)
{
	if (!peer)
		return {};

	return UdpAddress::address(peer->address);
}


/**
 * @brief UdpServerPeer::address
 * @param socket
 * @return
 */

QString UdpServerPeer::address(QWebSocket *socket)
{
	if (!socket)
		return {};

	return socket->peerName().append(':').append(QString::number(socket->peerPort()));
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



QWebSocket* UdpServerPeer::socket() const
{
	return m_socket.get();
}

void UdpServerPeer::setSocket(std::unique_ptr<QWebSocket> newSocket)
{
	m_socket = std::move(newSocket);
}

UdpRoom *UdpServerPeer::room() const
{
	return m_room;
}

void UdpServerPeer::setRoom(UdpRoom *newRoom)
{
	m_room = newRoom;
}










/**
 * @brief Lobby::Lobby
 * @param size
 */

Lobby::Lobby(UdpServerPrivate *server, const quint32 &size)
	: m_server(server->q)
	, m_serverPrivate(server)
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
 * @brief Lobby::get
 * @param peerId
 * @return
 */

std::optional<PeerData> Lobby::get(const quint32 &peerId) const
{
	QMutexLocker l(&m_mutex);

	const auto idx = _index(peerId);

	if (!idx)
		return std::nullopt;

	if (*idx >= m_size)
		return std::nullopt;

	return m_data[*idx];
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
 * @brief Lobby::createRoom
 * @param type
 * @return
 */

UdpRoom *Lobby::createRoom()
{
	QMutexLocker l(&m_mutex);

	int idx = -1;

	for (uint i=0; i<m_rooms.size(); ++i) {
		if (!m_rooms.at(i).engine()) {
			idx = i;
			break;
		}
	}

	if (idx < 0) {
		LOG_CERROR("engine") << "Room create error";
		return nullptr;
	}

	UdpRoom &room = m_rooms.at(idx);

	return &room;
}


/**
 * @brief Lobby::findRoom
 * @param fn
 * @return
 */

const UdpRoom *Lobby::findRoom(const std::function<bool (const UdpRoom *)> &fn) const
{
	if (!fn)
		return nullptr;

	QMutexLocker l(&m_mutex);

	int idx = -1;

	for (uint i=0; i<m_rooms.size(); ++i) {
		if (fn(&m_rooms.at(i))) {
			idx = i;
			break;
		}
	}

	if (idx < 0) {
		LOG_CERROR("engine") << "Room not found";
		return nullptr;
	}

	const UdpRoom &room = m_rooms.at(idx);

	return &room;
}



/**
 * @brief Lobby::removeRoom
 * @param room
 */

void Lobby::removeRoom(UdpRoom *room)
{
	Q_ASSERT(room);

	QMutexLocker l(&m_mutex);

	QSet<quint32> indices;

	for (UdpServerPeer *p : room->peers()) {
		indices.insert(p->peerIndex());
	}

	room->reset();

	l.unlock();

	for (const quint32 i : indices)
		removeIndex(i);
}





/**
 * @brief Lobby::setRoom
 * @param index
 * @param room
 */

void Lobby::setRoom(const quint32 &index, UdpRoom *room)
{
	QMutexLocker l(&m_mutex);

	if (index >= m_size) {
		LOG_CERROR("engine") << "Invalid peer index" << index;
		return;
	}

	m_data[index].room = room;
	m_data[index].type = room ? room->type() : EngineInvalid;
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

std::optional<UdpBitStream> Lobby::updateConnection(const UdpConnectionToken &token, const UdpType &type,
													const QJsonObject &tokenObj, QWebSocket *socket)
{
	Q_ASSERT(m_server);

	if (token.peer == 0)
		return std::nullopt;

	QMutexLocker l(&m_mutex);


	auto idx = _index(token.peer);

	if (!idx.has_value())
		return std::nullopt;

	PeerData &d = m_data[idx.value()];

	if (d.signer.has_value()) {
		// Already connected

		return std::optional<UdpBitStream>(std::in_place, d.peerId, idx.value());

	} else {
		if (!d.hasChallenge) {
			randombytes_buf(d.challenge.data(), d.challenge.size());
			d.hasChallenge = true;
		}

		d.type = type;
		d.peerId = token.peer;
		d.session = QByteArray::fromBase64(token.ses.toLatin1());
		d.publicKey = QByteArray::fromBase64(token.pub.toLatin1());
		d.connectionToken = tokenObj;

		if (socket) {
			PublicKeySigner signer;
			signer.setPublicKey(d.publicKey);
			d.signer = std::move(signer);

			LOG_CINFO("engine") << "Peer connected:" << d.session << qPrintable(UdpServerPeer::address(socket));

			return std::optional<UdpBitStream>(std::in_place, d.peerId, idx.value());
		}

		return std::optional<UdpBitStream>(std::in_place, d.challenge);
	}
}



/**
 * @brief Lobby::updateChallenge
 * @param peerId
 * @param stream
 * @return
 */

std::optional<UdpBitStream> Lobby::updateChallenge(const UdpConnectionToken &connToken, const QByteArray &content,
												   ENetPeer *peer)
{
	Q_ASSERT(m_server);
	Q_ASSERT(peer);

	if (connToken.peer == 0)
		return std::nullopt;

	QMutexLocker l(&m_mutex);

	auto idx = _index(connToken.peer);

	if (!idx.has_value())
		return std::nullopt;

	PeerData &d = m_data[idx.value()];

	if (!d.signer.has_value()) {
		// Not yet connected

		if (!d.hasChallenge) {
			LOG_CWARNING("engine") << "Missing challenge" << connToken.peer << connToken.user << qPrintable(UdpServerPeer::address(peer));
			return std::nullopt;
		}

		if (d.publicKey.isEmpty()) {
			LOG_CWARNING("engine") << "Missing public key" << connToken.peer << connToken.user << qPrintable(UdpServerPeer::address(peer));
			return std::nullopt;
		}

		QByteArray challenge = QByteArray::fromRawData(reinterpret_cast<const char*>(d.challenge.data()), d.challenge.size());


		PublicKeySigner signer;
		signer.setPublicKey(d.publicKey);

		if (!signer.verifySign(challenge, content)) {
			LOG_CWARNING("engine") << "Invalid challenge" << connToken.peer << connToken.user << qPrintable(UdpServerPeer::address(peer));
			return std::nullopt;
		}


		d.signer = std::move(signer);

		LOG_CINFO("engine") << "Peer connected:" << connToken.peer << connToken.user << qPrintable(UdpServerPeer::address(peer));
	}

	return std::optional<UdpBitStream>(std::in_place, d.peerId, idx.value());
}






/**
 * @brief Lobby::removeExpiredPeers
 */

void Lobby::removeExpiredPeers()
{
	QMutexLocker l(&m_mutex);

	for (PeerData &d : m_data) {
		if ((!d.room || !d.room->engine()) && d.deadline.hasExpired()) {
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
	QMutexLocker l(&m_mutex);

	QString txt;

	txt += QStringLiteral("ROOMS\n");
	txt += QStringLiteral("==================================================================\n");

	int count = 0;

	const auto fnPrintSeats = [this, &count](const UdpRoom *room) -> QString {
		QString s;
		for (size_t i=0; i<m_data.size(); ++i) {
			const PeerData &d = m_data[i];

			if (d.peerId == 0)
				continue;

			if (d.room != room)
				continue;

			++count;

			UdpEngine *e = d.room ? d.room->engine() : nullptr;

			s += QStringLiteral("%1 [%2]: (%3%4) e%5 [%6] %7\n")
				 .arg(d.peerId, 12)
				 .arg(i, 4)
				 .arg(d.hasChallenge ? '*' : ' ')
				 .arg(d.signer.has_value() ? '*' : ' ')
				 .arg(d.room ? d.room->type() : 0)
				 .arg(e ? e->readableId() : 0)
				 .arg(d.username)
				 ;
		}

		return s;
	};


	txt += fnPrintSeats(nullptr);
	txt += QStringLiteral(" \n \n");


	for (const UdpRoom &r : m_rooms) {
		if (!r.engine())
			continue;

		txt += fnPrintSeats(&r);
		txt += QStringLiteral(" \n \n");
	}


	txt.prepend(QStringLiteral("PEERS %1/%2\n").arg(count).arg(m_size));

	return txt;
}


/**
 * @brief Lobby::engines
 * @return
 */

QSet<UdpEngine*> Lobby::engines() const
{
	QMutexLocker l(&m_mutex);

	QSet<UdpEngine*> list;
	list.reserve(m_rooms.size());

	for (const UdpRoom &r : m_rooms) {
		if (r.engine())
			list.insert(r.engine());
	}

	return list;
}




/**
 * @brief Lobby::engines
 * @param type
 * @return
 */

QSet<UdpEngine *> Lobby::engines(const UdpType &type) const
{
	QMutexLocker l(&m_mutex);

	QSet<UdpEngine*> list;
	list.reserve(m_rooms.size());

	for (const UdpRoom &r : m_rooms) {
		if (r.type() == type && r.engine())
			list.insert(r.engine());
	}

	return list;
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

	auto it = m_indexMap.find(peerId);

	if (it == m_indexMap.cend())
		return std::nullopt;
	else
		return it.value();
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

	LOG_CINFO("engine") << "RESET WITH ROOM" << m_data[*it].room;

	m_data[*it].reset(true);					// Hold room
	m_data[*it].peerId = peerId;
	m_data[*it].username = username;

	if (expired.isValid())
		m_data[*it].deadline.setRemainingTime(QDateTime::currentDateTime().msecsTo(expired));
	else
		m_data[*it].deadline.setRemainingTime(-1);

	return m_data[*it];
}



/**
 * @brief UdpEngine::UdpEngine
 * @param server
 * @param room
 * @param parent
 */

UdpEngine::UdpEngine(UdpServer *server, UdpRoom *room, QObject *parent)
	: QObject(parent)
	, m_room(room)
	, m_udpServer(server)
{
	Q_ASSERT(m_room);
	Q_ASSERT(m_udpServer);

	generateReadableId();

	LOG_CTRACE("engine") << "UdpEngine created" << this << m_readableId << m_room;

}



/**
 * @brief UdpEngine::~UdpEngine
 */

UdpEngine::~UdpEngine()
{
	LOG_CTRACE("engine") << "UdpEngine destroyed" << this;
}



/**
 * @brief UdpEngine::udpPeerAdd
 * @param peer
 */

void UdpEngine::udpPeerAdd(UdpServerPeer *peer)
{
	if (!peer)
		return;

	LOG_CDEBUG("engine") << "Add peer" << qPrintable(peer->address()) << "to engine" << m_readableId;

	m_udpServer->setRoom(peer->peerIndex(), m_room);
}



/**
 * @brief UdpEngine::udpPeerRemove
 * @param peer
 */

void UdpEngine::udpPeerRemove(UdpServerPeer *peer)
{
	if (!peer)
		return;

	LOG_CDEBUG("engine") << "Remove peer" << qPrintable(peer->address()) << "from engine" << m_readableId;

	m_udpServer->setRoom(peer->peerIndex(), nullptr);
}



/**
 * @brief UdpEngine::dumpEngine
 * @return
 */

QString UdpEngine::dumpEngine() const
{
	return QStringLiteral(">>>> UdpEngine ");
}


/**
 * @brief UdpEngine::generateReadableId
 */

void UdpEngine::generateReadableId()
{
	m_readableId = QRandomGenerator::global()->bounded(1,1000000);
}






/**
 * @brief UdpRoom::type
 * @return
 */

UdpType UdpRoom::type() const
{
	return m_type;
}


/**
 * @brief UdpRoom::setType
 * @param newType
 */

void UdpRoom::setType(const UdpType &newType)
{
	m_type = newType;
}


/**
 * @brief UdpRoom::engine
 * @return
 */

UdpEngine* UdpRoom::engine() const
{
	return m_engine.get();
}

void UdpRoom::setEngine(std::unique_ptr<UdpEngine> newEngine)
{
	m_engine = std::move(newEngine);
}



/**
 * @brief UdpRoom::peerAdd
 * @param peer
 * @param host
 */

void UdpRoom::peerAdd(UdpServerPeer *peer)
{
	if (!peer)
		return;

	if (peer->room())
		peer->room()->peerRemove(peer);

	m_peers.insert(peer);

	peer->setRoom(this);

	if (m_engine)
		m_engine->udpPeerAdd(peer);
}




/**
 * @brief UdpRoom::peerRemove
 * @param peer
 */

void UdpRoom::peerRemove(UdpServerPeer *peer)
{
	if (!peer)
		return;

	m_peers.remove(peer);

	if (peer->room() == this)
		peer->setRoom(nullptr);

	if (m_engine)
		m_engine->udpPeerRemove(peer);
}




/**
 * @brief UdpRoom::peerRemoveAll
 */

void UdpRoom::peerRemoveAll()
{
	for (UdpServerPeer *peer : m_peers) {
		if (peer->room() == this)
			peer->setRoom(nullptr);

		if (m_engine)
			m_engine->udpPeerRemove(peer);
	}

	m_peers.clear();
}



/**
 * @brief UdpRoom::reset
 */

void UdpRoom::reset()
{
	peerRemoveAll();
	m_engine.reset();
	setType(EngineInvalid);
}


