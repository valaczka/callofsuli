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

#ifndef RPGUDPENGINE_H
#define RPGUDPENGINE_H


#include <QObject>
#include <QThread>
#include <QElapsedTimer>
#include <QCborArray>
#include "abstractudpengine.h"
#include "actionrpgmultiplayergame.h"



/**
 * @brief The ClientStorage class
 */

class ClientStorage : public RpgGameData::SnapshotStorage
{
public:

	ClientStorage() = default;

	void updateSnapshot(const RpgGameData::CharacterSelect &player);
	void updateSnapshot(const RpgGameData::BaseData &playerData, const RpgGameData::Player &player);
};






/**
 * @brief The UdpEnginePrivate class
 */

class RpgUdpEngine : public AbstractUdpEngine
{
	Q_OBJECT

public:
	RpgUdpEngine(ActionRpgMultiplayerGame *game);
	virtual ~RpgUdpEngine();

	void connectToServer(Server *server);
	void disconnect();

	RpgConfig::GameState gameState() const;
	void setGameState(const RpgConfig::GameState &newGameState);


signals:
	void gameError();
	void gameDataDownload(QString map, QList<RpgGameData::CharacterSelect> list);


protected:
	virtual void packetReceived(const QCborMap &data, const unsigned int rtt) override;

private:
	void updateState(const QCborMap &data);
	void updateSnapshot(const RpgGameData::CharacterSelect &player);

	void packetReceivedChrSel(const QCborMap &data);
	void packetReceivedDownload(const QCborMap &data);
	void packetReceivedPrepare(const QCborMap &data);
	void packetReceivedPlay(const QCborMap &data);

	bool forceKeyFrame();

	QRecursiveMutex m_mutex;


	QPointer<ActionRpgMultiplayerGame> m_game;
	bool m_downloadContentStarted = false;


	QList<RpgGameData::CharacterSelect> m_playerData;

	ClientStorage m_snapshots;

	RpgGameData::GameConfig m_gameConfig;
	RpgConfig::GameState m_gameState = RpgConfig::StateInvalid;
	int m_playerId = -1;
	bool m_isHost = false;

	QVariantList getPlayerList();

	void updateSnapshotEnemyList(const QCborArray &list);
	void updateSnapshotPlayerList(const QCborArray &list);


	/// --- tmp
	qint64 m_lastTick = 0;
	QElapsedTimer m_lastKeyFrame;

	friend class ActionRpgMultiplayerGame;
};






/**
 * @brief RpgUdpEngine::forceKeyFrame
 * @return
 */

inline bool RpgUdpEngine::forceKeyFrame()
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





#endif // RPGUDPENGINE_H
