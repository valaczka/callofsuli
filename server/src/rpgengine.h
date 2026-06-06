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

#include "abstractengine.h"
#include "rpgstream.h"
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
	explicit RpgEngine(EngineHandler *handler, QObject *parent = nullptr);
	virtual ~RpgEngine();


	static std::shared_ptr<RpgEngine> engineCreate(EngineHandler *handler, UdpServer *server);
	static std::shared_ptr<RpgEngine> engineDispatch(EngineHandler *handler, const QJsonObject &connectionToken, UdpPacketRcv &&data);


	static std::shared_ptr<RpgEngine> peerFind(UdpServer *server, const QString &username, quint32 *idPtr = nullptr);
	bool peerAbort(const quint32 &peerId);

	virtual bool canDelete(const int &useCount) override;

	virtual void binaryDataReceived(const UdpServerPeerReceivedList &data) override;
	virtual void udpPeerAdd(UdpServerPeer *peer) override;
	virtual void udpPeerRemove(UdpServerPeer *peer) override;
	virtual void disconnectUnusedPeer(UdpServerPeer *peer) override;
	virtual bool isPeerValid(const quint32 &peerId) const override;

	virtual QString dumpEngine() const override;


	RpgStream::Engine toStream() const;

	Logger *_logger() const;

private:
	void setLoggerFile(const QString &fname);

	void binaryDataReceived(const UdpPacketRcv &recv);

	int m_nextPlayerId = 1;
	int m_readableId = -1;

	RpgEnginePrivate *d;

	friend class RpgEnginePrivate;
};












#endif // RPGENGINE_H
