/*
 * ---- Call of Suli ----
 *
 * abstractudpengine.cpp
 *
 * Created on: 2025. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractUdpEngine
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

#include "abstractudpengine.h"
#include "Logger.h"
#include "abstractudpengine_p.h"
#include "qabstracteventdispatcher.h"
#include "application.h"
#include <sodium/crypto_sign.h>
#include <credential.h>
#include <QDataStream>
#include <QIODevice>
#include <BMLib/BinaryStream.hpp>



/**
 * @brief AbstractUdpEngine::AbstractUdpEngine
 * @param parent
 */

AbstractUdpEngine::AbstractUdpEngine(QObject *parent)
	: QObject(parent)
	#ifndef Q_OS_WASM
	, m_worker(new QLambdaThreadWorker)
	#endif
{
	LOG_CDEBUG("client") << "Start udp engine";


#ifndef Q_OS_WASM
	m_worker->getThread()->setPriority(QThread::TimeCriticalPriority);

	QDefer ret;
	m_worker->execInThread([this, ret]() mutable {
		d = new AbstractUdpEnginePrivate(this);
		ret.resolve();
	});

	QDefer::await(ret);

	m_worker->execInThread(std::bind(&AbstractUdpEnginePrivate::run, d));

#else
	d = new AbstractUdpEnginePrivate(this);
	d->runWebSocket();
#endif


}



/**
 * @brief AbstractUdpEngine::~AbstractUdpEngine
 */

AbstractUdpEngine::~AbstractUdpEngine()
{
#ifndef Q_OS_WASM
	d->stop();

	m_worker->quitThread();
	m_worker->getThread()->wait();
#endif

	delete d;

	LOG_CDEBUG("client") << "Udp engine destroyed";
}



/**
 * @brief AbstractUdpEngine::peerIndex
 * @return
 */

const quint32 &AbstractUdpEngine::peerIndex() const
{
	return d->m_peerIndex;
}



/**
 * @brief AbstractUdpEngine::sendMessage
 * @param data
 * @param reliable
 * @param sign
 */

void AbstractUdpEngine::sendMessage(const std::vector<uint8_t> &data, const bool &reliable)
{
#ifndef Q_OS_WASM
	m_worker->execInThread([this, data, reliable](){
		d->sendMessage(data, reliable);
	});
#else
	d->sendMessage(data, reliable);
#endif
}



/**
 * @brief AbstractUdpEngine::setUrl
 * @param url
 */

void AbstractUdpEngine::setUrl(const QUrl &url)
{
#ifndef Q_OS_WASM
	m_worker->execInThread([this, url](){
		d->setUrl(url);
	});
#else
	d->setUrl(url);
#endif
}


/**
 * @brief AbstractUdpEngine::connectionToken
 * @return
 */

QByteArray AbstractUdpEngine::connectionToken() const
{
#ifndef Q_OS_WASM
	QByteArray token;
	QDefer ret;
	m_worker->execInThread([this, &token, ret]() mutable {
		token = d->connectionToken();
		ret.resolve();
	});

	QDefer::await(ret);
	return token;
#else
	return d->connectionToken();
#endif
}




/**
 * @brief AbstractUdpEngine::setConnectionToken
 * @param token
 */

void AbstractUdpEngine::setConnectionToken(const QByteArray &token)
{
#ifndef Q_OS_WASM
	m_worker->execInThread([this, token](){
		d->setConnectionToken(token);
	});
#else
	d->setConnectionToken(token);
#endif
}







/**
 * @brief AbstractUdpEnginePrivate::AbstractUdpEnginePrivate
 * @param engine
 */

AbstractUdpEnginePrivate::AbstractUdpEnginePrivate(AbstractUdpEngine *engine)
	: q(engine)
{

}


/**
 * @brief RpgUdpEnginePrivate::run
 */


void AbstractUdpEnginePrivate::run()
{
#ifndef Q_OS_WASM
	m_running.storeRelease(1);

	while (m_running.loadAcquire() >= 1) {
		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

		if (!m_enet_host) {
			if (m_url.isEmpty())
				continue;

			if (m_connectionToken.isEmpty()) {
				LOG_CERROR("client") << "Connection token missing";
				continue;
			}

			ENetHost *client = enet_host_create(NULL, 1, 2, 0, 0);

			if (!client) {
				LOG_CERROR("client") << "Connection refused" << qPrintable(m_url.toDisplayString());
				emit q->serverConnectFailed(tr("Connection error"));
				QThread::msleep(500);
				continue;
			}

			ENetAddress address;
			ENetEvent event;
			ENetPeer *peer;

			enet_address_set_host(&address, m_url.host().toLatin1());
			address.port = m_url.port();

			peer = enet_host_connect (client, &address, 2, 0);

			if (!peer) {
				LOG_CWARNING("client") << "Connection refused" << qPrintable(m_url.toDisplayString());
				enet_host_destroy(client);

				if (m_udpState == UdpBitStream::MessageConnected) {
					emit q->serverConnectionLost();
				} else {
					emit q->serverConnectFailed(tr("Connection refused"));
				}
				QThread::msleep(500);
				continue;
			}

			if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
				LOG_CINFO("client") << "Connected to host" << qPrintable(m_url.toDisplayString());

				sendConnectionToken();

				if (m_udpState == UdpBitStream::MessageConnected)		// Reconnected
					emit q->serverConnected();
			} else {
				LOG_CWARNING("client") << "Connection failed" << qPrintable(m_url.toDisplayString());
				enet_peer_reset(peer);
				enet_host_destroy(client);

				if (m_udpState == UdpBitStream::MessageConnected) {
					emit q->serverConnectionLost();
				} else {
					emit q->serverConnectFailed(tr("Connection failed"));
				}

				QThread::msleep(500);
				continue;
			}

			m_enet_host = client;
			m_enet_peer = peer;
		} else {
			if (m_url.isEmpty()) {
				destroyHostAndPeer();
				continue;
			}
		}


		ENetEvent event;

		int r = enet_host_service (m_enet_host, &event, 1);

		if (r < 0) {
			LOG_CERROR("client") << "ENet host service error";

			if (m_udpState == UdpBitStream::MessageConnected)
				emit q->serverConnectionLost();
		}

		if (m_running.loadAcquire() < 1)
			break;

		if (r > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					sendConnectionToken();
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					if (m_udpState == UdpBitStream::MessageConnected) {
						emit q->serverConnectionLost();
						destroyHostAndPeer();
						QThread::msleep(1000);
					} else {
						destroyHostAndPeer();
						emit q->serverConnectFailed(tr("Connection rejected"));
						QThread::msleep(1000);
					}
					continue;
					break;

				case ENET_EVENT_TYPE_RECEIVE: {
					packetReceived(event);
					enet_packet_destroy(event.packet);
					break;
				}

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}

		deliverPackets();

		QThread::msleep(1);
	}


	destroyHostAndPeer();

#endif
}



/**
 * @brief AbstractUdpEnginePrivate::stop
 */

void AbstractUdpEnginePrivate::stop()
{
	LOG_CDEBUG("client") << "Stop UDP server";
	m_running.storeRelease(0);
}



/**
 * @brief AbstractUdpEnginePrivate::runWebSocket
 */

void AbstractUdpEnginePrivate::runWebSocket()
{
	m_webSocket.reset(new QWebSocket);

	connect(m_webSocket.get(), &QWebSocket::connected, this, [this](){
		sendConnectionToken();

		if (m_udpState == UdpBitStream::MessageConnected)		// Reconnected
			emit q->serverConnected();
	});

	connect(m_webSocket.get(), &QWebSocket::disconnected, this, [this](){
		if (m_udpState == UdpBitStream::MessageConnected) {
			emit q->serverConnectionLost();
			destroyHostAndPeer();
		} else {
			destroyHostAndPeer();
			emit q->serverConnectFailed(tr("Connection rejected"));
		}
	});


	connect(m_webSocket.get(), &QWebSocket::sslErrors, this, [this](const QList<QSslError> &errors) {
		LOG_CWARNING("client") << "Ignore SSL errors" << errors;
		m_webSocket->ignoreSslErrors();
	});

	connect(m_webSocket.get(), &QWebSocket::binaryMessageReceived, this, &AbstractUdpEnginePrivate::messageReceived);
	/*connect(m_webSocket.get(), &QWebSocket::textMessageReceived, this, [](const QString &text) {
		LOG_CDEBUG("client") << "***" << text;
	});*/
}



/**
 * @brief AbstractUdpEnginePrivate::sendMessage
 * @param data
 * @param reliable
 * @param sign
 */

void AbstractUdpEnginePrivate::sendMessage(const std::vector<uint8_t> &data, const bool &reliable)
{
	UdpPacketSnd packet;

	packet.reliable = reliable;
	packet.data = data;

	m_cacheSnd.push(std::move(packet));


#ifdef Q_OS_WASM
	deliverPackets();
#endif
}





/**
 * @brief AbstractUdpEnginePrivate::setUrl
 * @param newUrl
 */

void AbstractUdpEnginePrivate::setUrl(const QUrl &newUrl)
{
	if (m_url == newUrl)
		return;

	m_url = newUrl;

#ifndef Q_OS_WASM
	if (m_enet_host && !newUrl.isEmpty()) {
		LOG_CERROR("client") << "Can't change server url";
		return;
	}
#else
	if (!m_webSocket) {
		LOG_CERROR("client") << "Missing websocket";
		return;
	}

	QUrl ws;
	ws.setScheme(m_url.scheme() == QStringLiteral("https") ? QStringLiteral("wss") : QStringLiteral("ws"));
	ws.setHost(m_url.host());
	ws.setPort(m_url.port());
	ws.setPath(QStringLiteral("/ws"));

	m_webSocket->open(ws);

#endif


}




/**
 * @brief AbstractUdpEnginePrivate::deliverPackets
 */

void AbstractUdpEnginePrivate::deliverPackets()
{
	// Send outgoing packets (channel 0 = normal, channel 1 = reliable)

	if (m_speed.readyToSend()) {
		std::vector<UdpPacketSnd> out = m_cacheSnd.take();

		for (const UdpPacketSnd &p : out) {
#ifndef Q_OS_WASM
			ENetPacket *packet = enet_packet_create(p.data.data(), p.data.size(),
													p.reliable ? ENET_PACKET_FLAG_RELIABLE :
																 0);

			if (enet_peer_send(m_enet_peer, p.reliable ? 1 : 0, packet) < 0) {
				LOG_CERROR("client") << "ENet peer send error";
				enet_packet_destroy(packet);

				if (m_udpState == UdpBitStream::MessageConnected) {
					LOG_CDEBUG("client") << "Udp connection lost";

					emit q->serverConnectionLost();

				} else {
					LOG_CDEBUG("client") << "Udp connection failed";
					emit q->serverConnectFailed(tr("Connection lost"));
				}

				destroyHostAndPeer();
			}

#endif

			if (m_webSocket) {
				QByteArray d = QByteArray::fromRawData(reinterpret_cast<const char*>(p.data.data()), p.data.size());
				m_webSocket->sendBinaryMessage(d);
			}
		}
	}


	// Deliver received packets

	std::vector<UdpPacketRcv> list = m_cacheRcv.take();

	q->binaryDataReceived(list);
}







/**
 * @brief AbstractUdpEnginePrivate::destroyHostAndPeer
 */

void AbstractUdpEnginePrivate::destroyHostAndPeer()
{
#ifndef Q_OS_WASM
	if (m_enet_peer) {
		enet_peer_disconnect(m_enet_peer, 0);
		if (m_enet_host)
			enet_host_service(m_enet_host, nullptr, 200);
		enet_peer_reset(m_enet_peer);
	}

	if (m_enet_host)
		enet_host_destroy(m_enet_host);

	m_enet_peer = nullptr;
	m_enet_host = nullptr;

#else
	if (m_webSocket) {
		m_webSocket->close();
		m_webSocket.reset();
	}
#endif
	LOG_CDEBUG("client") << "Disconnected from host" << qPrintable(m_url.toDisplayString());
	emit q->serverDisconnected();

}





/**
 * @brief AbstractUdpEnginePrivate::packetChallengeReceived
 * @param data
 * @return
 */

bool AbstractUdpEnginePrivate::packetChallengeReceived(const std::unique_ptr<UdpBitStream> &data)
{
	if (m_connectionToken.isEmpty()) {
		LOG_CERROR("client") << "Connection token missing";
		return false;
	}

	const auto &ptr = data->readChallenge();

	if (!ptr) {
		LOG_CWARNING("client") << "Challenge error";
		return false;
	}

	const QByteArray challenge = QByteArray::fromRawData(reinterpret_cast<const char*>(ptr->data()), ptr->size());

	const QByteArray sign = Application::instance()->sign(challenge);

	if (sign.isEmpty()) {
		LOG_CERROR("client") << "Missing public key";
		return false;
	}

	UdpBitStream msg(m_connectionToken, sign);

	sendMessage(*msg);

	return true;
}




/**
 * @brief AbstractUdpEnginePrivate::packetReceived
 * @param data
 */

void AbstractUdpEnginePrivate::messageReceived(const QByteArray &data)
{
	std::unique_ptr<UdpBitStream> stream = std::make_unique<UdpBitStream>(reinterpret_cast<const unsigned char*>(data.constData()),
																		  data.size());

	if (!stream->validate()) {
		LOG_CWARNING("client") << "Invalid data from peer";
		return;
	}


	if (stream->type() == UdpBitStream::MessageConnect) {
		LOG_CWARNING("client") << "MESSAGE CONNECT";

		sendConnectionToken();

		return;

	} else if (stream->type() == UdpBitStream::MessageConnected || stream->type() >= UdpBitStream::MessageUser) {

		if (stream->type() == UdpBitStream::MessageConnected) {
			LOG_CINFO("client") << "MESSAGE CONNECTED";

			stream->getConnected(&m_peerId, &m_peerIndex);

			LOG_CINFO("client") << "#####" << m_peerId << m_peerIndex;
		}

		m_cacheRcv.push(UdpPacketRcv(std::move(stream), -1));

		if (m_udpState != UdpBitStream::MessageConnected) {
			m_udpState = UdpBitStream::MessageConnected;
			emit q->serverConnected();
		}

		deliverPackets();

		return;

	} else if (stream->type() == UdpBitStream::MessageServerFull) {
		m_udpState = UdpBitStream::MessageRejected;
		LOG_CWARNING("client") << "MESSAGE SERVERFULL";
		emit q->serverConnectFailed(tr("Server full"));
		return;

	} else if (stream->type() == UdpBitStream::MessageRejected) {
		m_udpState = UdpBitStream::MessageRejected;

		emit q->serverConnectFailed(tr("Connection rejected"));
		return;

	} else {
		LOG_CWARNING("client") <<  "Invalid message type from peer" << stream->type();
	}

}



/**
 * @brief AbstractUdpEnginePrivate::packetReceived
 * @param event
 */

#ifndef Q_OS_WASM

bool AbstractUdpEnginePrivate::packetReceived(const ENetEvent &event)
{
	if (!event.peer) {
		LOG_CWARNING("client") << "Invalid peer";
		return false;
	}

	if (event.packet->dataLength <= 0) {
		LOG_CWARNING("client") << qPrintable(UdpAddress::address(event.peer->address)) << "Invalid data from peer";
		return false;
	}


	std::unique_ptr<UdpBitStream> stream = std::make_unique<UdpBitStream>(event);

	if (!stream->validate()) {
		LOG_CWARNING("client") << qPrintable(UdpAddress::address(event.peer->address)) << "Invalid data from peer";
		return false;
	}


	if (stream->type() == UdpBitStream::MessageConnect) {
		LOG_CWARNING("client") << "MESSAGE CONNECT";

		sendConnectionToken();

		return true;

	} else if (stream->type() == UdpBitStream::MessageChallenge) {

		packetChallengeReceived(stream);

		return true;


	} else if (stream->type() == UdpBitStream::MessageConnected || stream->type() >= UdpBitStream::MessageUser) {

		const unsigned int rtt = m_enet_peer->roundTripTime;

		m_speed.addRtt(rtt);

		if (stream->type() == UdpBitStream::MessageConnected) {
			LOG_CINFO("client") << "MESSAGE CONNECTED";

			stream->getConnected(&m_peerId, &m_peerIndex);

			LOG_CINFO("client") << "#####" << m_peerId << m_peerIndex;
		}

		m_cacheRcv.push(UdpPacketRcv(std::move(stream), rtt));

		if (m_udpState != UdpBitStream::MessageConnected) {
			m_udpState = UdpBitStream::MessageConnected;
			emit q->serverConnected();
		}

		return true;

	} else if (stream->type() == UdpBitStream::MessageServerFull) {
		m_udpState = UdpBitStream::MessageRejected;
		LOG_CWARNING("client") << "MESSAGE SERVERFULL";
		emit q->serverConnectFailed(tr("Server full"));
		return true;

	} else if (stream->type() == UdpBitStream::MessageRejected) {
		m_udpState = UdpBitStream::MessageRejected;

		emit q->serverConnectFailed(tr("Connection rejected"));
		return true;

	} else {
		LOG_CWARNING("client") << qPrintable(UdpAddress::address(event.peer->address)) << "Invalid message type from peer" << stream->type();
		return false;
	}

	return true;
}

#endif

/**
 * @brief AbstractUdpEnginePrivate::connectionToken
 * @return
 */

QByteArray AbstractUdpEnginePrivate::connectionToken() const
{
	return m_connectionToken;
}


/**
 * @brief AbstractUdpEnginePrivate::setConnectionToken
 * @param newConnectionToken
 */

void AbstractUdpEnginePrivate::setConnectionToken(const QByteArray &newConnectionToken)
{
	m_connectionToken = newConnectionToken;

	Token jwt(m_connectionToken);
	UdpConnectionToken u;
	u.fromJson(jwt.payload());

	if (u.peer > 0)
		m_peerId = u.peer;
}



/**
 * @brief AbstractUdpEnginePrivate::sendConnectionToken
 */

void AbstractUdpEnginePrivate::sendConnectionToken()
{
	if (m_connectionToken.isEmpty()) {
		LOG_CERROR("client") << "Emtpy connection token";
		return;
	}

	UdpBitStream stream(m_connectionToken);

	sendMessage(*stream);
}


