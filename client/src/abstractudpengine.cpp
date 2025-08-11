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
#include <sodium/crypto_sign.h>
#include <credential.h>
#include <QDataStream>
#include <QIODevice>
#include "sodium/crypto_box.h"
#include "utils_.h"



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

#endif


}



/**
 * @brief AbstractUdpEngine::~AbstractUdpEngine
 */

AbstractUdpEngine::~AbstractUdpEngine()
{
#ifndef Q_OS_WASM
	m_worker->getThread()->requestInterruption();
	m_worker->quitThread();
	m_worker->getThread()->wait();
#endif

	delete d;

	LOG_CDEBUG("client") << "Udp engine destroyed";
}



/**
 * @brief AbstractUdpEngine::sendMessage
 * @param data
 * @param reliable
 * @param sign
 */

void AbstractUdpEngine::sendMessage(const QByteArray &data, const bool &reliable, const bool &sign)
{
#ifndef Q_OS_WASM
	m_worker->execInThread([this, data, reliable, sign](){
		d->sendMessage(data, reliable, sign);
	});
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
#endif
}


/**
 * @brief AbstractUdpEngine::connectionToken
 * @return
 */

QByteArray AbstractUdpEngine::connectionToken() const
{
	QByteArray token;
#ifndef Q_OS_WASM
	QDefer ret;
	m_worker->execInThread([this, &token, ret]() mutable {
		token = d->connectionToken();
		ret.resolve();
	});

	QDefer::await(ret);
#endif
	return token;
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
#endif
}




/**
 * @brief AbstractUdpEngine::currentRtt
 * @return
 */

int AbstractUdpEngine::currentRtt() const
{
	int rtt = 0;

#ifndef Q_OS_WASM
	QDefer ret;
	m_worker->execInThread([this, &rtt](){
		rtt = d->currentRtt();
	});

	QDefer::await(ret);
#endif

	return rtt;
}




/**
 * @brief AbstractUdpEngine::setCurrentRtt
 * @param rtt
 */

void AbstractUdpEngine::setCurrentRtt(const int &rtt)
{
#ifndef Q_OS_WASM
	m_worker->execInThread([this, rtt](){
		d->setCurrentRtt(rtt);
	});
#endif
}




/**
 * @brief AbstractUdpEnginePrivate::AbstractUdpEnginePrivate
 * @param engine
 */

AbstractUdpEnginePrivate::AbstractUdpEnginePrivate(AbstractUdpEngine *engine)
	: q(engine)
	, m_secretKey(Token::generateSecret())
{
	LOG_CINFO("client") << "secret generated" << m_secretKey.toBase64();
}


/**
 * @brief RpgUdpEnginePrivate::run
 */


void AbstractUdpEnginePrivate::run()
{
#ifndef Q_OS_WASM

	while (!QThread::currentThread()->isInterruptionRequested()) {
		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

		if (!m_enet_host) {
			if (m_url.isEmpty())
				continue;


			// Channel 0: unsigned
			// Channel 1: signed

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

				if (m_udpState == UdpServerResponse::StateConnected) {
					emit q->serverConnectionLost();
				} else {
					emit q->serverConnectFailed(tr("Connection refused"));
				}
				QThread::msleep(500);
				continue;
			}

			if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
				LOG_CINFO("client") << "Connected to host" << qPrintable(m_url.toDisplayString());
				if (m_udpState == UdpServerResponse::StateConnected)
					emit q->serverConnected();
			} else {
				LOG_CWARNING("client") << "Connection failed" << qPrintable(m_url.toDisplayString());
				enet_peer_reset(peer);
				enet_host_destroy(client);

				if (m_udpState == UdpServerResponse::StateConnected) {
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

		int r = enet_host_service (m_enet_host, &event, 1000./240.);

		if (r < 0) {
			LOG_CERROR("client") << "ENet host service error";

			if (m_udpState == UdpServerResponse::StateConnected)
				emit q->serverConnectionLost();
		}

		if (QThread::currentThread()->isInterruptionRequested())
			break;

		if (r > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					if (m_udpState == UdpServerResponse::StateConnected) {
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

		updateChallenge();			// send -> mutex lock

		if (m_speed.readyToSend()) {
			QMutexLocker locker(&m_inOutChache.mutex);

			for (const auto &b : m_inOutChache.sendList) {
				enet_uint8 channel = 0;

				QByteArray s;
				QDataStream stream(&s, QIODevice::WriteOnly);
				stream.setVersion(QDataStream::Qt_6_7);

				stream << (quint32) 0x434F53;			// COS
				stream << Utils::versionCode();
				stream << m_peerID;

				if (!m_secretKey.isEmpty() && b.sign) {
					stream << Token::sign(b.data, m_secretKey);
					channel = 1;
				}

				/*if (cur > -1 && b.tick > -1)
					stream << std::max((qint64) 0, cur-b.tick);
				else*/
				stream << (qint64) 0;

				stream << b.data;

				ENetPacket *packet = enet_packet_create(s.data(), s.size(),
														b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																	 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);

				if (enet_peer_send(m_enet_peer, channel, packet) < 0) {
					LOG_CERROR("client") << "ENet peer send error";
					enet_packet_destroy(packet);


					if (m_udpState == UdpServerResponse::StateConnected) {
						LOG_CDEBUG("engine") << "Udp connection lost";

						emit q->serverConnectionLost();

					} else {
						LOG_CDEBUG("engine") << "Udp connection failed";
						emit q->serverConnectFailed(tr("Connection lost"));
					}

					destroyHostAndPeer();
				}
			}

			m_inOutChache.sendList.clear();

			locker.unlock();
		}

		deliverReceived();
	}


	destroyHostAndPeer();
#endif
}



/**
 * @brief AbstractUdpEnginePrivate::sendMessage
 * @param data
 * @param reliable
 * @param sign
 */

void AbstractUdpEnginePrivate::sendMessage(QByteArray data, const bool &reliable, const bool &sign)
{
#ifndef Q_OS_WASM
	//const qint64 time = q->getTick();
	QMutexLocker locker(&m_inOutChache.mutex);
	m_inOutChache.sendList.emplace_back( data, reliable, sign, 0 );
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

#ifndef Q_OS_WASM
	if (m_enet_host && !newUrl.isEmpty()) {
		LOG_CERROR("client") << "Can't change server url";
		return;
	}

#endif

	m_url = newUrl;
}



/**
 * @brief AbstractUdpEnginePrivate::deliverReceived
 */

#ifndef Q_OS_WASM

void AbstractUdpEnginePrivate::deliverReceived()
{
	std::vector<InOutCache::PacketRcv> list;

	QMutexLocker locker(&m_inOutChache.mutex);

	m_inOutChache.rcvList.swap(list);

	locker.unlock();

	QList<QPair<QByteArray, unsigned int> > l;

	for (const auto &ptr : list)
		l.append(QPair<QByteArray, unsigned int>(ptr.data, ptr.rtt));

	q->binaryDataReceived(l);
}

#endif


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

#endif
	LOG_CDEBUG("client") << "Disconnected from host" << qPrintable(m_url.toDisplayString());
	emit q->serverDisconnected();

}



/**
 * @brief AbstractUdpEnginePrivate::packetReceived
 * @param event
 */

#ifndef Q_OS_WASM

void AbstractUdpEnginePrivate::packetReceived(const ENetEvent &event)
{
	const QByteArray msg(reinterpret_cast<char*>(event.packet->data), event.packet->dataLength);

	const unsigned int rtt = m_enet_peer->roundTripTime;

	m_speed.addRtt(rtt);


	QDataStream stream(msg);
	stream.setVersion(QDataStream::Qt_6_7);

	quint32 magic = 0;
	quint32 version = 0;

	stream >> magic >> version;

	if (magic != 0x434F53 || version == 0) {			// COS
		LOG_CERROR("game") << "Invalid stream";
		return;
	}


	QByteArray data;

	stream >> data;

	const QCborMap map = QCborValue::fromCbor(data).toMap();

	UdpServerResponse rsp;
	rsp.fromCbor(map);

	if (m_udpState != UdpServerResponse::StateConnected) {
		if (rsp.state == UdpServerResponse::StateRejected || rsp.state == UdpServerResponse::StateConnected) {
			if (m_udpState != rsp.state) {
				if (rsp.state == UdpServerResponse::StateConnected)
					emit q->serverConnected();
				else
					emit q->serverConnectFailed(tr("Connection rejected"));
			}

			QMutexLocker locker(&m_inOutChache.mutex);
			m_udpState = rsp.state;
			return;
		}

		if (m_udpState == UdpServerResponse::StateInvalid) {
			if (rsp.state == UdpServerResponse::StateChallenge) {
				UdpChallengeRequest rsp;
				rsp.fromCbor(map);

				if (rsp.key.size() != crypto_box_PUBLICKEYBYTES) {
					LOG_CERROR("client") << "Invalid key size";
					return;
				}

				m_challenge = rsp;
				m_udpState = UdpServerResponse::StateChallenge;
			}

		}

		return;
	}

	if (rsp.state == UdpServerResponse::StateRejected) {
		emit q->serverConnectFailed(tr("Connection rejected"));
		return;
	}

	QMutexLocker locker(&m_inOutChache.mutex);
	m_inOutChache.rcvList.emplace_back(data, event.channelID, rtt);			// timestamp
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
	UdpToken u;
	u.fromJson(jwt.payload());

	if (u.peerID > 0)
		m_peerID = u.peerID;
}



/**
 * @brief AbstractUdpEnginePrivate::updateChallenge
 */

void AbstractUdpEnginePrivate::updateChallenge()
{
	if (m_connectionToken.isEmpty())
		return;

	if (m_udpState == UdpServerResponse::StateConnected)
		return;

	if (m_udpState == UdpServerResponse::StateRejected) {
		LOG_CERROR("client") << "Udp connection rejected";
		return;
	}

	if (m_udpState == UdpServerResponse::StateInvalid) {
		UdpConnectRequest rq(m_connectionToken);
		QByteArray d = rq.toCborMap().toCborValue().toCbor();

#ifndef Q_OS_WASM
		m_inOutChache.sendList.emplace_back( d, false, false, 0 );
#endif

		return;
	}


	if (m_udpState == UdpServerResponse::StateChallenge) {
		UdpChallengeResponseContent rc;
		rc.challenge = m_challenge.challenge;
		rc.key = m_secretKey;

		const QByteArray content = rc.toCborMap().toCborValue().toCbor();
		const qsizetype len = crypto_box_SEALBYTES + content.size();

		unsigned char *dest = (unsigned char*) malloc(len);

		if (crypto_box_seal(dest, reinterpret_cast<const unsigned char*>(content.constData()), content.size(),
							reinterpret_cast<const unsigned char*> (m_challenge.key.constData())) != 0) {
			LOG_CERROR("client") << "Seal errror";
		} else {
			UdpChallengeResponse r;
			r.response = QByteArray(reinterpret_cast<const char *>(dest), len);
			r.token = m_connectionToken;
			sendMessage(r.toCborMap().toCborValue().toCbor(), false, false);
		}

		free(dest);

	}



}








/**
 * @brief AbstractUdpEnginePrivate::Speed::addRtt
 * @param rtt
 */

void AbstractUdpEnginePrivate::Speed::addRtt(const int &rtt)
{
	currentRtt = rtt;

	const auto it = limit.upper_bound(rtt);

	// Ha túl nagy az rtt

	if (it != limit.cbegin()) {
		// Ha visszaestünk a rosszba (10 mp-en belül), duplázzuk a várakozási időt

		if (fps != maxFps && lastBad.isValid() && lastBad.elapsed() < 10000)
			delay = std::min(60000, delay*2);

		// Eddig nem váltunk vissza
		nextGood.setRemainingTime(delay);


		if (lastBad.isValid())
			lastBad.restart();
		else
			lastBad.start();

		lastGood.invalidate();

		if (const auto &f = std::prev(it)->second; f < fps) {
			LOG_CDEBUG("game") << "[Benchmark] RTT=" << rtt << "SET FPS" << fps << "->" << f;
			fps = f;
		}

		return;
	}


	// Ha jó a helyzet

	if (!nextGood.hasExpired())
		return;


	if (fps != maxFps) {
		LOG_CDEBUG("game") << "[Benchmark] RTT=" << rtt << "SET FPS" << fps << "->" << maxFps;

		fps = maxFps;
	}

	// Mérjük, hogy mióta jó

	if (!lastGood.isValid()) {
		lastGood.start();
		return;
	}

	if (lastGood.hasExpired(10000)) {
		delay = std::max(1000, delay/2);
		lastGood.restart();
	}
}
