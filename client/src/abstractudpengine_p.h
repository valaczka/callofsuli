/*
 * ---- Call of Suli ----
 *
 * rpgudpengine_p.h
 *
 * Created on: 2025. 01. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * %{Cpp:License:ClassName}
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

#ifndef ABSTRACTUDPENGINE_P_H
#define ABSTRACTUDPENGINE_P_H

#include "qurl.h"
#include "abstractudpengine.h"
#include <QObject>
#include <QMap>
#include <QElapsedTimer>

#ifndef Q_OS_WASM
#include <enet/enet.h>
#include "udphelper.h"
#endif

class AbstractUdpEngine;


/**
 * @brief The RpgUdpEnginePrivate class
 */

class AbstractUdpEnginePrivate : public QObject
{
	Q_OBJECT

public:
	AbstractUdpEnginePrivate(AbstractUdpEngine *engine);

	void run();
	void stop();

	void sendMessage(const std::vector<uint8_t> &data, const bool &reliable = true);
	void setUrl(const QUrl &url);

	const int &currentRtt() const { return m_speed.currentRtt; }
	void setCurrentRtt(const int &rtt) { m_speed.addRtt(rtt); }

	QByteArray connectionToken() const;
	void setConnectionToken(const QByteArray &newConnectionToken);


private:
	void sendConnectionToken();
	void deliverPackets();
	void destroyHostAndPeer();

	bool packetChallengeReceived(const std::unique_ptr<UdpBitStream> &data);

	AbstractUdpEngine *q = nullptr;

	QUrl m_url;

#ifndef Q_OS_WASM
	bool packetReceived(const ENetEvent &event);

	ENetHost *m_enet_host = nullptr;
	ENetPeer *m_enet_peer = nullptr;
#endif


	struct UdpSpeedClient : public UdpSpeed
	{
		bool readyToSend() {
			if (!lastSent.isValid()) {
				lastSent.start();
				return true;
			}

			if (lastSent.hasExpired(1000./(float) fps)) {
				lastSent.restart();
				return true;
			}

			return false;
		}
	};

	UdpSpeedClient m_speed;


	QAtomicInt m_running{0};

	UdpCacheQueue<UdpPacketRcv> m_cacheRcv;
	UdpCacheQueue<UdpPacketSnd> m_cacheSnd;

	UdpAuthKey m_secretKey;

	QByteArray m_connectionToken;

	quint32 m_peerId = 0;
	quint32 m_peerIndex = 0;

	UdpBitStream::MessageType m_udpState = UdpBitStream::MessageInvalid;

	friend class AbstractUdpEngine;
	friend class AbstractUdpEngineThread;
};



#endif // ABSTRACTUDPENGINE_P_H
