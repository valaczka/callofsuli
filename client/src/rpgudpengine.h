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

	// Incoming snapshot
	void updateSnapshot(const RpgGameData::CharacterSelect &player);
	void updateSnapshot(const RpgGameData::PlayerBaseData &playerData, const RpgGameData::Player &player);
	void updateSnapshot(const RpgGameData::EnemyBaseData &enemyData, const RpgGameData::Enemy &enemy);
	void updateSnapshot(const RpgGameData::BulletBaseData &bulletData, const RpgGameData::Bullet &bullet);


	// Outgoing snapshot
	void appendSnapshot(const RpgGameData::PlayerBaseData &playerData, const RpgGameData::Player &player);
	void appendSnapshot(const RpgGameData::EnemyBaseData &enemyData, const RpgGameData::Enemy &enemy);
	void appendSnapshot(const RpgGameData::BulletBaseData &bulletData, const RpgGameData::Bullet &bullet);


	// Remove snapshot
	void removeMissingSnapshots(const std::vector<RpgGameData::BulletBaseData> &bulletList);

	void clear();

	QByteArray dump();

private:
	template <typename T, typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type>
	qint64 insert(std::map<qint64, T> *dst, const T &snap);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	void removeMissing(RpgGameData::SnapshotList<T, T2> &snapshot, const std::vector<T2> &list);
};








/**
 * @brief ClientStorage::removeMissing
 * @param snapshot
 * @param list
 */

template<typename T, typename T2, typename T3, typename T4>
inline void ClientStorage::removeMissing(RpgGameData::SnapshotList<T, T2> &snapshot, const std::vector<T2> &list)
{
	QMutexLocker locker(&m_mutex);

	for (auto it = snapshot.cbegin(); it != snapshot.cend(); ) {
		if (std::find(list.cbegin(), list.cend(), it->data) == list.cend())
			it = snapshot.erase(it);
		else
			++it;
	}
}



/**
 * @brief ClientStorage::insert
 * @param dst
 * @param snap
 */

template<typename T, typename T2>
inline qint64 ClientStorage::insert(std::map<qint64, T> *dst, const T &snap)
{
	Q_ASSERT(dst);

	qint64 f = snap.f;

	while (dst->contains(f))
		++f;

	if (f != snap.f) {
		T s2 = snap;
		s2.f = f;
		dst->insert_or_assign(f, s2);
	} else {
		dst->insert_or_assign(f, snap);
	}

	return f;
}




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
	void updateSnapshotBulletList(const QCborArray &list);

	friend class ActionRpgMultiplayerGame;
};









#endif // RPGUDPENGINE_H
