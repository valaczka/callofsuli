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

#include "qmutex.h"
#include "qurl.h"
#include <QObject>
#include <QMap>
#include <QElapsedTimer>
#include "credential.h"

#ifndef Q_OS_WASM
#include <enet/enet.h>
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

	void sendMessage(QByteArray data, const bool &reliable = true, const bool &sign = true);
	void setUrl(const QUrl &url);

	const int &currentRtt() const { return m_speed.currentRtt; }
	void setCurrentRtt(const int &rtt) { m_speed.addRtt(rtt); }

	QByteArray connectionToken() const;
	void setConnectionToken(const QByteArray &newConnectionToken);


private:
	void updateChallenge();
	void destroyHostAndPeer();

	AbstractUdpEngine *q = nullptr;

	QUrl m_url;

#ifndef Q_OS_WASM
	void deliverReceived();
	void packetReceived(const ENetEvent &event);

	ENetHost *m_enet_host = nullptr;
	ENetPeer *m_enet_peer = nullptr;
#endif


	struct Speed {
		void addRtt(const int &rtt);

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

		inline static constexpr int maxFps = 60;
		int fps = maxFps;

		// min rtt -> max fps
		inline static const std::map<int, int> limit = {
			{ 45,	30 },
			{ 75,	20 },
			{ 150,	15 },
			{ 200,	10 }
		};


		QElapsedTimer lastSent;
		QElapsedTimer lastBad;
		QElapsedTimer lastGood;
		QDeadlineTimer nextGood;
		int delay = 2000;
		int currentRtt = 0;
	};


	Speed m_speed;

#ifndef Q_OS_WASM
	struct InOutCache {
		struct Packet {
			Packet(const QByteArray &d, const bool r, const bool &s, const qint64 &_tick = -1)
				: data(d)
				, reliable(r)
				, sign(s)
				, tick(_tick)
			{}

			QByteArray data;
			bool reliable = false;
			bool sign = true;
			qint64 tick = -1;
		};


		struct PacketRcv {
			PacketRcv(const QByteArray &d, const enet_uint8 _ch, const unsigned int &_rtt)
				: data(d)
				, channel(_ch)
				, rtt(_rtt)
			{}

			QByteArray data;
			enet_uint8 channel = 0;
			unsigned int rtt = 0;
		};


		QMutex mutex;
		std::vector<Packet> sendList;
		std::vector<PacketRcv> rcvList;
	};

	InOutCache m_inOutChache;
#endif

	QByteArray m_secretKey;
	QByteArray m_connectionToken;
	UdpChallengeRequest m_challenge;
	UdpServerResponse::State m_udpState = UdpServerResponse::StateInvalid;
	quint32 m_peerID = 0;

	friend class AbstractUdpEngineThread;
};



#endif // ABSTRACTUDPENGINE_P_H
