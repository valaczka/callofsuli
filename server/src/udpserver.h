/*
 * ---- Call of Suli ----
 *
 * udpengine.h
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

#ifndef UDPSERVER_H
#define UDPSERVER_H

#include "qlambdathreadworker.h"
#include "qwebsocket.h"
#include "udpbitstream.hpp"
#include "udphelper.h"
#include <enet/enet.h>
#include <QPointer>
#include <QThread>
#include <QElapsedTimer>

class ServerService;
class UdpServer;
class UdpServerPrivate;
class UdpEngine;
class UdpRoom;
class UdpServerPeer;






/**
 * @brief The UdpType enum
 */

enum UdpType {
	EngineInvalid = 0,
	EngineRpg
};






/**
 * @brief The UdpPeerData class
 */

struct UdpPeerData
{
	quint32 peerId = 0;

	UdpType type = UdpType::EngineInvalid;
	QJsonObject connectionToken;			// azért QJsonObject, hogy pl. RpgConnectionToken is lehessen
	QString username;
	QByteArray session;
};



/**
 * @brief The UdpServer class
 */

class UdpServer
{
public:
	explicit UdpServer(ServerService *service);
	virtual ~UdpServer();

	void send(UdpServerPeer *peer, const std::vector<uint8_t> &data, const bool &reliable);

	void sendAll(const std::vector<uint8_t> &data, const std::function<bool(UdpServerPeer*)> &fn, const bool &reliable);
	void sendAll(const std::vector<uint8_t> &data, const bool &reliable) {
		sendAll(data, nullptr, reliable);
	}

	void sendRoomList(const UdpType &type, const bool &reliable = false);

	quint32 addPeer(const QString &username, const QDateTime &expired);

	QString dumpPeers() const;
	void removeExpiredPeers();

	void websocketAdd(QWebSocket *socket);

	ServerService *service() const;

	UdpRoom *createRoom();

	template <class T, typename = std::enable_if<std::is_base_of<UdpEngine, T>::value>::type>
	T* createEngine();


	const UdpRoom *findRoom(const std::function<bool (const UdpRoom *)> &fn) const;
	UdpEngine *findEngine(const std::function<bool (const UdpRoom *)> &fn) const;

	template <class T, typename = std::enable_if<std::is_base_of<UdpEngine, T>::value>::type>
	T* findEngine(const std::function<bool(const UdpRoom*)> &fn) const {
		return qobject_cast<T*>(findEngine(fn));
	}

private:
	UdpServerPrivate *d = nullptr;
	std::unique_ptr<QLambdaThreadWorker> m_worker;
	ServerService *const m_service;

	std::vector<std::unique_ptr<UdpServerPeer> > m_peerList;
	QHash<QWebSocket*, UdpServerPeer*> m_websocketHash;

	friend class UdpServerPrivate;
};







/**
 * @brief The UdpServerPeer class
 */

class UdpServerPeer {

public:
	UdpServerPeer(const quint32 &peerIndex, const UdpPeerData &data, UdpServer *server, ENetPeer *peer = nullptr);
	~UdpServerPeer();

	const quint32 &peerIndex() const { return m_peerIndex; }
	const UdpPeerData &peerData() const { return m_peerData; }

	ENetPeer *peer() const { return m_peer; }
	void setPeer(ENetPeer *newPeer) { m_peer = newPeer; }

	UdpServer *server() const { return m_server; }

	QString host() const { return host(m_peer); }
	static QString host(ENetPeer *peer);

	int port() const { return port(m_peer); }
	static int port(ENetPeer *peer);

	QString address() const;
	static QString address(ENetPeer *peer);
	static QString address(QWebSocket *socket);

	void send(const std::vector<std::uint8_t> &data, const bool &reliable);

	bool readyToSend(const int &maxFps = 0);
	void addRtt(const int &rtt) { m_speed.addRtt(rtt); }

	const int &currentRtt() const { return m_speed.currentRtt; }
	const int &currentFps() const { return m_speed.fps; }
	const int &peerFps() const { return m_speed.peerFps; }

	QWebSocket *socket() const;
	void setSocket(std::unique_ptr<QWebSocket> newSocket);

	UdpRoom *room() const;
	void setRoom(UdpRoom *newRoom);

private:
	const quint32 m_peerIndex;
	const UdpPeerData m_peerData;
	UdpServer *m_server = nullptr;
	ENetPeer *m_peer = nullptr;
	std::unique_ptr<QWebSocket> m_socket;
	UdpRoom *m_room = nullptr;

	UdpSpeed m_speed;

	friend class UdpServerPrivate;
};




struct UdpPacketRcv {
	UdpPacketRcv(std::unique_ptr<UdpBitStream> &&_data)
		: data(std::move(_data))
	{}

	UdpServerPeer *peer = nullptr;
	std::unique_ptr<UdpBitStream> data;

	ENetPeer *getENetPeer() const { return peer ? peer->peer() : nullptr; }
	QWebSocket *getWebSocket() const { return peer ? peer->socket() : nullptr; }
};


struct UdpPacketSnd {
	ENetPeer *peer = nullptr;
	QPointer<QWebSocket> socket;
	std::vector<std::uint8_t> data;
	bool reliable = false;

	ENetPeer *getENetPeer() const { return peer; }
	QWebSocket *getWebSocket() const { return socket; }
};


typedef std::vector<UdpPacketRcv> UdpServerPeerReceivedList;





/**
 * @brief The UdpEngine class
 */

class UdpEngine : public QObject
{
	Q_OBJECT

public:
	explicit UdpEngine(UdpServer *server, UdpRoom *room, QObject *parent = nullptr);
	virtual ~UdpEngine();

	virtual void binaryDataReceived(UdpServerPeerReceivedList &data) { Q_UNUSED(data); }
	virtual void udpPeerAdd(UdpServerPeer *peer) { Q_UNUSED(peer); }
	virtual void udpPeerRemove(UdpServerPeer *peer) { Q_UNUSED(peer); }
	virtual void disconnectUnusedPeer(UdpServerPeer *peer) { Q_UNUSED(peer); }
	virtual void hostChanged(UdpServerPeer *peer) { Q_UNUSED(peer); }

	virtual void udpTimerEvent(const qint64 &dt) { Q_UNUSED(dt); }

	virtual QString dumpEngine() const;

	UdpServer *udpServer() const { return m_udpServer; }
	void setUdpServer(UdpServer *server) { m_udpServer = server; }

	UdpRoom *room() const { return m_room;}

	quint32 readableId() const { return m_readableId; }
	void setReadableId(quint32 newReadableId) { m_readableId = newReadableId; }

protected:
	void generateReadableId();

	UdpRoom *const m_room;
	quint32 m_readableId = 0;

	UdpServer *m_udpServer = nullptr;
};





/**
 * @brief The UdpRoom class
 */

class UdpRoom {

public:
	UdpRoom(const UdpType &type, std::unique_ptr<UdpEngine> engine)
		: m_type(type)
		, m_engine(std::move(engine))
	{  }

	UdpRoom() : UdpRoom(UdpType::EngineInvalid, nullptr) {}

	virtual ~UdpRoom() {  }


	UdpType type() const;
	void setType(const UdpType &newType);

	UdpEngine* engine() const;
	void setEngine(std::unique_ptr<UdpEngine> newEngine);

	const QSet<UdpServerPeer *> &peers() const { return m_peers; }
	QSet<UdpServerPeer *> &peers() { return m_peers; }

	void peerAdd(UdpServerPeer *peer);
	void peerRemove(UdpServerPeer *peer);

protected:
	UdpType m_type = UdpType::EngineInvalid;
	std::unique_ptr<UdpEngine> m_engine;
	QSet<UdpServerPeer*> m_peers;
};






/**
 * @brief UdpServer::createEngine
 * @param type
 * @return
 */

template<class T, typename T2>
inline T *UdpServer::createEngine()
{
	UdpRoom *room = createRoom();
	if (!room)
		return nullptr;

	room->setEngine(std::make_unique<T>(this, room));

	return qobject_cast<T*>(room->engine());
}





#endif // UDPSERVER_H
