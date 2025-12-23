/*
 * ---- Call of Suli ----
 *
 * udphelper.h
 *
 * Created on: 2025. 12. 22.
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

#ifndef UDPHELPER_H
#define UDPHELPER_H


#include "qdeadlinetimer.h"
#include "qelapsedtimer.h"
#include "udpbitstream.hpp"
#include <deque>
#include <map>
#include <vector>

#ifndef Q_OS_WASM
#include "qmutex.h"
#include <enet/enet.h>
#endif

typedef std::array<std::uint8_t, CHALLENGE_BYTES>				UdpChallenge;
typedef std::array<std::uint8_t, crypto_auth_KEYBYTES>			UdpAuthKey;
typedef std::array<std::uint8_t, crypto_box_PUBLICKEYBYTES>		UdpPublicKey;




/**
 * @brief The Speed class
 */

struct UdpSpeed {
	void addRtt(const int &rtt);

	inline static constexpr int maxFps = 30;
	int fps = maxFps;

	// min rtt -> max fps
	inline static const std::map<int, int> limit = {
		{ 30,	30 },
		{ 75,	24 },
		{ 150,	20 },
		{ 200,	15 },
	};


	QElapsedTimer lastSent;
	QElapsedTimer lastBad;
	QElapsedTimer lastGood;
	QDeadlineTimer nextGood;
	int delay = 2000;
	int currentRtt = 0;
	int peerFps = 0;

	std::vector<qint64> received;
};





/**
 * @brief The CacheQueue class
 */

template <typename T>
class UdpCacheQueue {
public:
	UdpCacheQueue() = default;

	void push(T &&value) {
#ifndef Q_OS_WASM
		QMutexLocker locker(&m_mutex);
#endif
		m_queue.emplace_back(std::move(value));
	}

	std::vector<T> take() {
#ifndef Q_OS_WASM
		QMutexLocker locker(&m_mutex);
#endif
		std::vector<T> out;

		out.reserve(m_queue.size());
		while (!m_queue.empty()) {
			out.push_back(std::move(m_queue.front()));
			m_queue.pop_front();
		}

		return out;
	}


#ifndef Q_OS_WASM
	void clearPeer(ENetPeer *peer) {
		if (!peer)
			return;

		QMutexLocker locker(&m_mutex);

		std::erase_if(m_queue, [peer](const T &data) {
			return data.getENetPeer() == peer;
		});
	}
#endif

private:
#ifndef Q_OS_WASM
	QMutex m_mutex;
#endif
	std::deque<T> m_queue;
};




/**
 * @brief The UdpAddress class
 */
#ifndef Q_OS_WASM
struct UdpAddress
{
	static QString host(const ENetAddress &address)
	{
		return QStringLiteral("%1.%2.%3.%4")
				.arg((address.host)&0xFF)
				.arg(((address.host)>>8)&0xFF)
				.arg(((address.host)>>16)&0xFF)
				.arg(((address.host)>>24)&0xFF);
	}


	static int port(const ENetAddress &address)
	{
		return address.port;
	}


	static QString address(const ENetAddress &address)
	{
		return host(address).append(':').append(QString::number(port(address)));
	}
};
#endif


/// ------------------------------------------------------------------------- ///



inline void UdpSpeed::addRtt(const int &rtt)
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
			//LOG_CDEBUG("game") << "RTT=" << rtt << "SET FPS" << fps << "->" << f;
			fps = f;
		}

		return;
	}


	// Ha jó a helyzet

	if (!nextGood.hasExpired())
		return;

	if (fps != maxFps) {
		//LOG_CDEBUG("game") << "RTT=" << rtt << "SET FPS" << fps << "->" << maxFps;
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



#endif // UDPHELPER_H
