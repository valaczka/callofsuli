/*
 * ---- Call of Suli ----
 *
 * actionrpgmultiplayergame_p.h
 *
 * Created on: 2025. 01. 02.
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

#ifndef ACTIONRPGMULTIPLAYERGAME_P_H
#define ACTIONRPGMULTIPLAYERGAME_P_H


#include <QObject>
#include <QThread>
#include <QElapsedTimer>
#include <enet/enet.h>
#include "actionrpgmultiplayergame.h"

/**
 * @brief The UdpEnginePrivate class
 */

class RpgUdpEnginePrivate : public QObject
{
	Q_OBJECT

public:
	RpgUdpEnginePrivate(ActionRpgMultiplayerGame *game);
	virtual ~RpgUdpEnginePrivate();

	void stop() { m_canRun = false; }
	void worldStep(TiledObjectBody *body);
	void timeStepped();

private:
	bool connectToHost(const char *host, const int &port);
	void connectToServer();
	void disconnect();

	void packetReceived(ENetPacket *packet);
	void packetReceivedChrSel(const QCborMap &data);

	void run();

	void sendMessage(QByteArray data, const bool &reliable);
	bool forceKeyFrame();

	ActionRpgMultiplayerGame *q;
	ENetHost *m_enet_host = nullptr;
	ENetPeer *m_enet_peer = nullptr;
	bool m_canRun = true;

	QRecursiveMutex m_mutex;

	struct Packet {
		QByteArray data;
		bool reliable = false;
	};

	QList<Packet> m_sendList;

	QElapsedTimer m_lastKeyFrame;



	/**
	 * @brief The PlayerData class
	 */

	class PlayerData : public RpgGameData::Player {
	public:
		PlayerData() : RpgGameData::Player() {}
		PlayerData(const RpgGameData::Player &pdata)
			: RpgGameData::Player(pdata)
		{}

		RpgPlayer *player = nullptr;
	};


	std::vector<PlayerData> m_playerData;

	/// --- tmp
	qint64 m_lastTick = 0;

	friend class ActionRpgMultiplayerGame;
};


/**
 * @brief RpgUdpEnginePrivate::forceKeyFrame
 * @return
 */

inline bool RpgUdpEnginePrivate::forceKeyFrame()
{
	if (m_lastKeyFrame.isValid()) {
		if (m_lastKeyFrame.elapsed() > 250) {
			m_lastKeyFrame.restart();
			return true;
		}
	} else {
		m_lastKeyFrame.start();
		return true;
	}

	return false;
}





#endif // ACTIONRPGMULTIPLAYERGAME_P_H
