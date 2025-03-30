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
	: d(new UdpServerPrivate(this))
	, m_dThread(this)
	, m_service(service)
{
	LOG_CDEBUG("engine") << "START UDP ENGINE";

	d->moveToThread(&m_dThread);
	m_dThread.start();
}



/**
 * @brief UdpServer::~UdpServer
 */

UdpServer::~UdpServer()
{
	d = nullptr;
	m_dThread.requestInterruption();
	m_dThread.quit();
	m_dThread.wait();

	LOG_CDEBUG("engine") << "END UDP ENGINE";
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

	QMetaObject::invokeMethod(d, &UdpServerPrivate::sendPacket, Qt::QueuedConnection, peer->peer(), data, reliable);
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

	LOG_CINFO("engine") << "UPD SERVER FINISHED";
}



/**
 * @brief UdpServerPrivate::run
 */

void UdpServerPrivate::run()
{
	LOG_CINFO("engine") << "UPD ENGINE RUN";

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 10101;

	m_enet_server = enet_host_create(&address,
									 32,
									 1,
									 0, 0);

	if (m_enet_server == NULL) {
		LOG_CERROR("engine") << "UPD SERVER START ERROR";
		return;
	} else {
		LOG_CINFO("engine") << "UPD SERVER STARTED";
	}


	ENetEvent event;

	while (int r = enet_host_service (m_enet_server, &event, SERVER_ENET_SPEED) >= 0) {
		QThread::currentThread()->eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::AllEvents);

		if (QThread::currentThread()->isInterruptionRequested()) {
			LOG_CWARNING("engine") << "STOP";
			break;
		}

		if (r > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					peerConnect(event.peer);
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					peerDisconnect(event.peer);
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					packetReceived(event);
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}


		QMutexLocker locker(&m_mutex);

		for (const auto &b : m_sendList) {
			ENetPacket *packet = enet_packet_create(b.data.data(), b.data.size(),
													b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
			enet_peer_send(b.peer, 0, packet);
		}

		m_sendList.clear();

	}

	LOG_CINFO("engine") << "UPD ENGINE RUN FINISHED";
}



/**
 * @brief UdpServerPrivate::peerConnect
 * @param peer
 */

void UdpServerPrivate::peerConnect(ENetPeer *peer)
{
	if (!peer)
		return;

	bool isFirst = q->m_peerList.empty();

	const std::unique_ptr<UdpServerPeer> &p = q->m_peerList.emplace_back(std::make_unique<UdpServerPeer>(q, peer));
	p->peer()->data = p.get();

	LOG_CDEBUG("engine") << "Peer connected:" << p->host() << p->port();

	if (isFirst) {
		std::shared_ptr<RpgEngine> e = RpgEngine::engineCreate(q->m_service->engineHandler(), q);
		q->m_service->engineHandler()->engineAdd(e);
		p->setEngine(e);
	} else {
		const auto &engines= q->m_service->engineHandler()->engines();

		auto it = std::find_if(engines.constBegin(), engines.constEnd(), [](const std::shared_ptr<AbstractEngine> &e) {
			return e->type() == AbstractEngine::EngineRpg;
		});

		Q_ASSERT(it != engines.constEnd());

		p->setEngine(std::dynamic_pointer_cast<UdpEngine>(*it));
	}

	p->engine()->udpPeerAdd(p.get());
}


/**
 * @brief UdpServerPrivate::peerDisconnect
 * @param peer
 */

void UdpServerPrivate::peerDisconnect(ENetPeer *peer)
{
	if (!peer)
		return;

	UdpServerPeer *p = static_cast<UdpServerPeer*>(peer->data);

	if (!p) {
		LOG_CERROR("engine") << "Invalid UpdServerPeer";
		return;
	}

	LOG_CDEBUG("engine") << "Peer disconnected:" << p->host() << p->port();

	udpPeerRemove(p);
}


/**
 * @brief UdpServerPrivate::udpPeerRemove
 * @param peer
 */

void UdpServerPrivate::udpPeerRemove(UdpServerPeer *peer)
{
	Q_ASSERT(peer);

	if (peer->engine())
		peer->engine()->udpPeerRemove(peer);

	QMutexLocker locker(&m_mutex);

	std::erase_if(m_sendList, [peer](const Packet &p) { return p.peer == peer->peer(); });

	std::erase_if(q->m_peerList, [peer](const std::unique_ptr<UdpServerPeer> &ptr) {
		return peer == ptr.get();
	});

}


/**
 * @brief UdpServerPrivate::packetReceived
 * @param event
 */

void UdpServerPrivate::packetReceived(const ENetEvent &event)
{
	UdpServerPeer *peer = static_cast<UdpServerPeer*>(event.peer->data);

	if (!peer) {
		LOG_CWARNING("engine") << "Peer not found";
		return;
	}

	peer->packetReceived(event.channelID, event.packet);
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

	QMutexLocker locker(&m_mutex);

	m_sendList.emplace_back(peer, data, isReliable);
}



void UdpServerThread::run()
{
	q->d->run();
}





/**
 * @brief UdpServerPeer::host
 * @return
 */

UdpServerPeer::UdpServerPeer(UdpServer *server, ENetPeer *peer)
	: m_server(server)
	, m_peer(peer)
{
	LOG_CDEBUG("engine") << "New peer" << this;
}


/**
 * @brief UdpServerPeer::~UdpServerPeer
 */

UdpServerPeer::~UdpServerPeer()
{
	LOG_CDEBUG("engine") << "Delete peer" << this;
}


/**
 * @brief UdpServerPeer::packetReceived
 * @param channel
 */

void UdpServerPeer::packetReceived(const int &/*channel*/, ENetPacket *packet)
{
	Q_ASSERT(m_engine);
	QByteArray data((char*) packet->data, packet->dataLength);

	m_engine->binaryDataReceived(this, data);
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





