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

	LOG_CDEBUG("engine") << "START UDP ENGINE";

	QDefer ret;
	m_worker->execInThread([this, ret]() mutable {
		d = new AbstractUdpEnginePrivate(this);
		ret.resolve();
	});

	QDefer::await(ret);

	m_worker->execInThread(std::bind(&AbstractUdpEnginePrivate::run, d));

	LOG_CDEBUG("engine") << "UDP ENGINE started";

#ifndef Q_OS_WASM


#endif
}



/**
 * @brief AbstractUdpEngine::~AbstractUdpEngine
 */

AbstractUdpEngine::~AbstractUdpEngine()
{
	LOG_CINFO("client") << "ABSTRACT UPD ENGINE DESTROY START";

#ifndef Q_OS_WASM
	m_worker->getThread()->requestInterruption();
	m_worker->quitThread();
	m_worker->getThread()->wait();

	delete d;
#endif

	LOG_CINFO("client") << "ABSTRACT UPD ENGINE DESTROYED";
}



/**
 * @brief AbstractUdpEngine::sendMessage
 * @param data
 * @param reliable
 * @param channel
 */

void AbstractUdpEngine::sendMessage(const QByteArray &data, const bool &reliable, const int &channel)
{
	m_worker->execInThread([this, data, reliable, channel](){
		d->sendMessage(data, reliable, channel);
	});
}



/**
 * @brief AbstractUdpEngine::setUrl
 * @param url
 */

void AbstractUdpEngine::setUrl(const QUrl &url)
{
	m_worker->execInThread([this, url](){
		d->setUrl(url);
	});
}




/**
 * @brief AbstractUdpEngine::currentRtt
 * @return
 */

int AbstractUdpEngine::currentRtt() const
{
	QDefer ret;
	int rtt = 0;
	m_worker->execInThread([this, &rtt](){
		rtt = d->currentRtt();
	});

	QDefer::await(ret);

	return rtt;
}




/**
 * @brief AbstractUdpEngine::setCurrentRtt
 * @param rtt
 */

void AbstractUdpEngine::setCurrentRtt(const int &rtt)
{
	m_worker->execInThread([this, rtt](){
		d->setCurrentRtt(rtt);
	});
}





/**
 * @brief RpgUdpEnginePrivate::run
 */

void AbstractUdpEnginePrivate::run()
{
#ifndef Q_OS_WASM
	LOG_CINFO("client") << "RUN";

	while (!QThread::currentThread()->isInterruptionRequested()) {
		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

		if (!m_enet_host) {
			if (m_url.isEmpty())
				continue;

			ENetHost *client = enet_host_create(NULL, 1, 2, 0, 0);

			if (!client) {
				LOG_CERROR("client") << "Connection refused" << qPrintable(m_url.toDisplayString());
				emit q->serverConnectFailed();
				QThread::msleep(1000);
				continue;
			}

			ENetAddress address;
			ENetEvent event;
			ENetPeer *peer;

			enet_address_set_host(&address, m_url.host().toLatin1());
			address.port = m_url.port();

			peer = enet_host_connect (client, &address, 2, 0);

			if (!peer) {
				LOG_CWARNING("game") << "Connection refused" << qPrintable(m_url.toDisplayString());
				enet_host_destroy(client);

				emit q->serverConnectFailed();
				QThread::msleep(1000);
				continue;
			}

			if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
				LOG_CINFO("game") << "Connected to host" << qPrintable(m_url.toDisplayString());
				emit q->serverConnected();
			} else {
				LOG_CWARNING("game") << "Connection failed" << qPrintable(m_url.toDisplayString());
				enet_peer_reset(peer);
				enet_host_destroy(client);

				emit q->serverConnectFailed();
				QThread::msleep(1000);
				continue;
			}

			m_enet_host = client;
			m_enet_peer = peer;
		} else {
			if (m_url.isEmpty()) {
				if (m_enet_peer) {
					enet_peer_disconnect(m_enet_peer, 0);
					enet_host_service(m_enet_host, nullptr, 200);
					enet_peer_reset(m_enet_peer);
					m_enet_peer = nullptr;
				}
				enet_host_destroy(m_enet_host);

				LOG_CDEBUG("client") << "Disconnected from host" << qPrintable(m_url.toDisplayString());
				emit q->serverDisconnected();

				m_enet_host = nullptr;
				continue;
			}
		}


		ENetEvent event;

		int r = enet_host_service (m_enet_host, &event, 1000./240.);

		if (r < 0) {
			LOG_CERROR("engine") << "ENet host service error";
		}

		if (QThread::currentThread()->isInterruptionRequested())
			break;

		if (r > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					LOG_CINFO("engine") << "CONNECT" << event.peer->address.host << event.peer->address.port;
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					LOG_CINFO("engine") << "DISCONNECT" << event.peer->address.host << event.peer->address.port; //<< event.peer->data;
					emit q->serverDisconnected();
					QThread::msleep(1000);
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

		if (m_speed.readyToSend()) {
			QMutexLocker locker(&m_inOutChache.mutex);

			for (const auto &b : m_inOutChache.sendList) {
				ENetPacket *packet = enet_packet_create(b.data.data(), b.data.size(),
														b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																	 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
				if (enet_peer_send(m_enet_peer, 0, packet) < 0) {
					LOG_CERROR("client") << "ENet peer send error";
					enet_packet_destroy(packet);
				}
			}

			m_inOutChache.sendList.clear();

			locker.unlock();
		}

		deliverReceived();
	}

	LOG_CINFO("client") << "UPD ENGINE RUN FINISHED";


	if (m_enet_host) {
		if (m_enet_peer) {
			enet_peer_disconnect(m_enet_peer, 0);
			enet_host_service(m_enet_host, nullptr, 200);
			enet_peer_reset(m_enet_peer);
			m_enet_peer = nullptr;
		}
		enet_host_destroy(m_enet_host);

		LOG_CDEBUG("client") << "Disconnected from host" << qPrintable(m_url.toDisplayString());
		emit q->serverDisconnected();
	}

	m_enet_host = nullptr;
#endif
}



/**
 * @brief AbstractUdpEnginePrivate::sendMessage
 * @param data
 * @param reliable
 * @param channel
 */

void AbstractUdpEnginePrivate::sendMessage(QByteArray data, const bool &reliable, const int &channel)
{
	Q_UNUSED(channel);

	QMutexLocker locker(&m_inOutChache.mutex);
	m_inOutChache.sendList.emplace_back( data, reliable );
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

	LOG_CDEBUG("client") << "SET URL" << newUrl;

	m_url = newUrl;
}



/**
 * @brief AbstractUdpEnginePrivate::deliverReceived
 */

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



/**
 * @brief AbstractUdpEnginePrivate::packetReceived
 * @param event
 */

void AbstractUdpEnginePrivate::packetReceived(const ENetEvent &event)
{
	QByteArray data((char*) event.packet->data, event.packet->dataLength);

	QCborMap map = QCborValue::fromCbor(data).toMap();
	const unsigned int rtt = m_enet_peer->roundTripTime;

	m_speed.addRtt(rtt);

	QMutexLocker locker(&m_inOutChache.mutex);
	m_inOutChache.rcvList.emplace_back(data, 0, rtt);			// timestamp
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
