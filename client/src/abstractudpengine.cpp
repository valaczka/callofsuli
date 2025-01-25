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
	, m_dThread(new AbstractUdpEngineThread(this))
	, d(new AbstractUdpEnginePrivate(this))
{
	d->moveToThread(m_dThread);
	QObject::connect(m_dThread, &QThread::finished, d, &QObject::deleteLater);
	QObject::connect(m_dThread, &QThread::finished, m_dThread, &QObject::deleteLater);
	m_dThread->start();
}



/**
 * @brief AbstractUdpEngine::~AbstractUdpEngine
 */

AbstractUdpEngine::~AbstractUdpEngine()
{
	LOG_CINFO("client") << "ABSTRACT UPD ENGINE DESTROY START";

	m_dThread->requestInterruption();
	m_dThread->quit();
	m_dThread->wait();
	m_dThread = nullptr;

	LOG_CINFO("client") << "ABSTRACT UPD ENGINE DESTROYED";
}



/**
 * @brief AbstractUdpEngine::sendMessage
 * @param data
 * @param reliable
 * @param channel
 */

void AbstractUdpEngine::sendMessage(QByteArray data, const bool &reliable, const int &channel)
{
	QMetaObject::invokeMethod(d, &AbstractUdpEnginePrivate::sendMessage, Qt::QueuedConnection, data, reliable, channel);
}



/**
 * @brief AbstractUdpEngine::setUrl
 * @param url
 */

void AbstractUdpEngine::setUrl(const QUrl &url)
{
	QMetaObject::invokeMethod(d, &AbstractUdpEnginePrivate::setUrl, Qt::QueuedConnection, url);
}




/**
 * @brief AbstractUdpEngineThread::run
 */

void AbstractUdpEngineThread::run()
{
	q->d->run();
}



/**
 * @brief RpgUdpEnginePrivate::run
 */

void AbstractUdpEnginePrivate::run()
{
	LOG_CINFO("client") << "RUN";

	while (!QThread::currentThread()->isInterruptionRequested()) {
		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

		QMutexLocker locker(&m_mutex);


		if (!m_enet_host) {
			if (m_url.isEmpty())
				continue;

			ENetHost *client = enet_host_create(NULL, 1, 2, 0, 0);

			if (!client) {
				LOG_CERROR("client") << "Connection refused" << qPrintable(m_url.toDisplayString());
				emit serverConnectFailed();
				locker.unlock();
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

				emit serverConnectFailed();
				locker.unlock();
				QThread::msleep(1000);
				continue;
			}

			if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
				LOG_CINFO("game") << "Connected to host" << qPrintable(m_url.toDisplayString());
				emit serverConnected();
			} else {
				LOG_CWARNING("game") << "Connection failed" << qPrintable(m_url.toDisplayString());
				enet_peer_reset(peer);
				enet_host_destroy(client);

				emit serverConnectFailed();
				locker.unlock();
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
				emit serverDisconnected();

				m_enet_host = nullptr;
				continue;
			}
		}


		ENetEvent event;

		int r = enet_host_service (m_enet_host, &event, 1000/120);

		if (QThread::currentThread()->isInterruptionRequested())
			break;

		if (r > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					LOG_CINFO("engine") << "CONNECT" << event.peer->address.host << event.peer->address.port;
					//event.peer->data = "mydata";
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					LOG_CINFO("engine") << "DISCONNECT" << event.peer->address.host << event.peer->address.port; //<< event.peer->data;
					emit serverDisconnected();
					locker.unlock();
					QThread::msleep(1000);
					continue;
					break;

				case ENET_EVENT_TYPE_RECEIVE: {
					QByteArray data((char*)event.packet->data, event.packet->dataLength);
					QCborMap map = QCborValue::fromCbor(data).toMap();
					const unsigned int rtt = m_enet_peer->roundTripTime;
					QMetaObject::invokeMethod(q, &AbstractUdpEngine::packetReceived, Qt::QueuedConnection, map, rtt);
					enet_packet_destroy(event.packet);
					break;
				}

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}

		for (const auto &b : m_sendList) {
			ENetPacket *packet = enet_packet_create(b.data.data(), b.data.size(),
													b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
			enet_peer_send(m_enet_peer, 0, packet);
		}

		m_sendList.clear();

	}

	LOG_CINFO("client") << "UPD ENGINE RUN FINISHED";

	QMutexLocker locker(&m_mutex);

	if (m_enet_host) {
		if (m_enet_peer) {
			enet_peer_disconnect(m_enet_peer, 0);
			enet_host_service(m_enet_host, nullptr, 200);
			enet_peer_reset(m_enet_peer);
			m_enet_peer = nullptr;
		}
		enet_host_destroy(m_enet_host);

		LOG_CDEBUG("client") << "Disconnected from host" << qPrintable(m_url.toDisplayString());
		emit serverDisconnected();
	}

	m_enet_host = nullptr;

}



/**
 * @brief AbstractUdpEnginePrivate::sendMessage
 * @param data
 * @param reliable
 * @param channel
 */

void AbstractUdpEnginePrivate::sendMessage(QByteArray data, const bool &reliable, const int &channel)
{
	QMutexLocker locker(&m_mutex);
	m_sendList.append({
						  .data = data,
						  .channel = channel,
						  .reliable = reliable
					  });
}





/**
 * @brief AbstractUdpEnginePrivate::setUrl
 * @param newUrl
 */

void AbstractUdpEnginePrivate::setUrl(const QUrl &newUrl)
{
	if (m_url == newUrl)
		return;

	QMutexLocker locker(&m_mutex);

	if (m_enet_host && !newUrl.isEmpty()) {
		LOG_CERROR("client") << "Can't change server url";
		return;
	}

	LOG_CDEBUG("client") << "SET URL" << newUrl;

	m_url = newUrl;
}
