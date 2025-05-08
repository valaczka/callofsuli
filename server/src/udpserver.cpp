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
	: m_worker(new QLambdaThreadWorker)
	, m_service(service)
{
	LOG_CDEBUG("engine") << "START UDP ENGINE";

	QDefer ret;
	m_worker->execInThread([this, ret]() mutable {
		d = new UdpServerPrivate(this);
		ret.resolve();
	});

	QDefer::await(ret);

	m_worker->execInThread(std::bind(&UdpServerPrivate::run, d));

	LOG_CDEBUG("engine") << "UDP ENGINE started";
}



/**
 * @brief UdpServer::~UdpServer
 */

UdpServer::~UdpServer()
{
	LOG_CDEBUG("engine") << "STOPPING UDP ENGINE";

	m_worker->getThread()->requestInterruption();
	m_worker->quitThread();
	m_worker->getThread()->wait();

	delete d;

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

	m_worker->execInThread([this, peer, data, reliable]() {
		d->sendPacket(peer->peer(), data, reliable);
	});
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


		QMutexLocker locker(&m_inOutChache.mutex);

		for (const auto &b : m_inOutChache.sendList) {
			ENetPacket *packet = enet_packet_create(b.data.data(), b.data.size(),
													b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
			enet_peer_send(b.peer, 0, packet);
		}

		m_inOutChache.sendList.clear();

		locker.unlock();

		//if (m_timer.hasExpired(m_fps)) {
		deliverReceived();
		//	m_timer.restart();
		//}

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

	QMutexLocker locker(&m_inOutChache.mutex);

	std::erase_if(m_inOutChache.sendList, [peer](const auto &p) { return p.peer == peer->peer(); });
	std::erase_if(m_inOutChache.rcvList, [peer](const auto &p) { return p.peer == peer; });

	locker.unlock();

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

	if (event.packet->dataLength <= 0) {
		LOG_CWARNING("engine") << "Invalid data";
		return;
	}

	peer->addRtt(event.peer->roundTripTime);


	QMutexLocker locker(&m_inOutChache.mutex);

	QByteArray data((char*) event.packet->data, event.packet->dataLength);

	m_inOutChache.rcvList.emplace_back(peer, data, 0);			// timestamp

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
 * @param peer
 * @return
 */

QList<QByteArray> UdpServerPrivate::takePackets(UdpServerPeer *peer)
{
	if (!peer)
		return {};

	QMutexLocker locker(&m_inOutChache.mutex);

	QList<QByteArray> list;

	list.reserve(m_inOutChache.rcvList.size());

	for (auto it = m_inOutChache.rcvList.cbegin(); it != m_inOutChache.rcvList.cend(); ) {
		if (it->peer == peer) {
			list.append(it->data);
			it = m_inOutChache.rcvList.erase(it);
		} else {
			++it;
		}
	}

	return list;
}



/**
 * @brief UdpServerPrivate::takePackets
 * @param engine
 * @return
 */

QList<QByteArray> UdpServerPrivate::takePackets(UdpEngine *engine)
{
	if (!engine)
		return {};

	QMutexLocker locker(&m_inOutChache.mutex);

	QList<QByteArray> list;

	list.reserve(m_inOutChache.rcvList.size());

	for (auto it = m_inOutChache.rcvList.cbegin(); it != m_inOutChache.rcvList.cend(); ) {
		if (it->peer->engine().get() == engine) {
			list.append(it->data);
			it = m_inOutChache.rcvList.erase(it);
		} else {
			++it;
		}
	}

	return list;
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
			LOG_CDEBUG("game") << "RTT=" << rtt << "SET FPS" << fps << "->" << f;
			fps = f;
		}

		return;
	}


	// Ha jó a helyzet

	if (!nextGood.hasExpired())
		return;

	if (fps != maxFps) {
		LOG_CDEBUG("game") << "RTT=" << rtt << "SET FPS" << fps << "->" << maxFps;

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
