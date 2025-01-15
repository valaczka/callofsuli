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
#include <QCborArray>
#include <enet/enet.h>
#include <deque>
#include <list>
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

	void connectToServer(Server *server);
	void disconnect();

	void worldStep(const TiledGame::Body &body);
	void timeStepped();

signals:			// REMOVE SIGNALS
	void serverConnected();
	void serverDisconnected();
	void serverConnectFailed();

	void gameError();
	void gameDataDownload(QString map, QList<RpgGameData::CharacterSelect> list);


private:
	void connectToHost(const char *host, const int &port);

	void updateState(const QCborMap &data);

	void packetReceived(ENetPacket *packet);
	void packetReceivedChrSel(const QCborMap &data);
	void packetReceivedDownload(const QCborMap &data);
	void packetReceivedPrepare(const QCborMap &data);
	void packetReceivedPlay(const QCborMap &data);

	void run();

	void sendMessage(QByteArray data, const bool &reliable);
	bool forceKeyFrame();





	QPointer<ActionRpgMultiplayerGame> q;
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
	bool m_downloadContentStarted = false;


	QList<RpgGameData::CharacterSelect> m_playerData;

	// Snapshots

	struct Snapshot {
		qint64 tick = -1;
		bool isAuth = false;
		std::list<RpgGameData::Player> players;
		std::list<RpgGameData::Enemy> enemies;
	};

	std::deque<Snapshot> m_snapshots;
	RpgGameData::GameConfig m_gameConfig;
	RpgConfig::GameState m_gameState = RpgConfig::StateInvalid;
	int m_playerId = -1;
	bool m_isHost = false;

	void updateSnapshot(const RpgGameData::CharacterSelect &player);
	void updateSnapshot(Snapshot &snapshot, const RpgGameData::Player &player);
	void updateSnapshot(Snapshot &snapshot, const RpgGameData::Enemy &enemy);
	void updateSnapshot(const RpgGameData::Player &player, const qint64 &tick);
	void updateSnapshot(const RpgGameData::Enemy &enemy, const qint64 &tick);

	QVariantList getPlayerList();

	void updateSnapshotEnemyList(const QCborArray &list, const qint64 &tick);
	void updateSnapshotPlayerList(const QCborArray &list, const qint64 &tick);

	Snapshot getCurrentSnapshot();

        void updatePlayer(const Snapshot &snapshot, RpgPlayer *player, const int &owner);
	void updateEnemy(const Snapshot &snapshot, IsometricEnemy *enemy);


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
	QMutexLocker locker(&m_mutex);

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
