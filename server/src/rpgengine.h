/*
 * ---- Call of Suli ----
 *
 * rpgengine.h
 *
 * Created on: 2025. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgEngine
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

#ifndef RPGENGINE_H
#define RPGENGINE_H

#include "rpglogicserver.h"
#include "udpserver.h"


#define ELOG_TRACE            CuteMessageLogger(_logger(), Logger::Trace,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_DEBUG            CuteMessageLogger(_logger(), Logger::Debug,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_INFO             CuteMessageLogger(_logger(), Logger::Info,    __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_WARNING          CuteMessageLogger(_logger(), Logger::Warning, __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_ERROR            CuteMessageLogger(_logger(), Logger::Error,   __FILE__, __LINE__, Q_FUNC_INFO).write()
#define ELOG_FATAL            CuteMessageLogger(_logger(), Logger::Fatal,   __FILE__, __LINE__, Q_FUNC_INFO).write()


class RpgEnginePrivate;



/**
 * @brief The RpgEngine class
 */

class RpgEngine : public UdpEngine
{
	Q_OBJECT

public:
	explicit RpgEngine(UdpServer *server, UdpRoom *room, QObject *parent = nullptr);
	virtual ~RpgEngine();

	static void peerWithoutRoomHandle(std::unique_ptr<UdpBitStream> &&data, UdpServerPeer *peer, const QSet<RpgEngine*> &engines);
	static void sendRoomList(UdpServer *server, const QSet<RpgEngine*> &engines, const bool &reliable = false);
	static RpgStream::RoomList toRoomList(const QSet<RpgEngine*> &engines);
	static RpgEngine* findEngine(UdpServer *server, const quint32 &id);
	static bool peerConnectToEngine(UdpServerPeer *peer, RpgEngine *engine);

	RpgStream::Room toRoom() const;

	virtual void binaryDataReceived(UdpServerPeerReceivedList &data) override;
	virtual void udpPeerAdd(UdpServerPeer *peer) override;
	virtual void udpPeerRemove(UdpServerPeer *peer) override;
	virtual void disconnectUnusedPeer(UdpServerPeer *peer) override;
	virtual void udpTimerEvent(const qint64 &dt) override;

	virtual QString dumpEngine() const override;

	virtual bool canRemove() const override;

	Logger *_logger() const;

	quint32 internalId() const { return m_id; }

	RpgStream::GameConfig::Flags configFlags() const;
	RpgStream::GameConfig::Stage configStage() const;


private:
	void setLoggerFile(const QString &fname);
	void binaryDataReceived(UdpPacketRcv &recv);


private:
	RpgEnginePrivate *d;
	mutable RpgLogicServer m_logic;

	const quint32 m_id;

	quint32 m_pausedTick = 0;

	friend class RpgEnginePrivate;
};







#endif // RPGENGINE_H
