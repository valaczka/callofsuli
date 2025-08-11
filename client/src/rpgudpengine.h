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
#include "rpgconfig.h"
#include "rpggamedataiface.h"



/**
 * @brief The ClientStorage class
 */

class ClientStorage : public RpgGameData::SnapshotStorage
{
public:

	ClientStorage() = default;

	// Incoming snapshot
	void updateSnapshot(const QList<RpgGameData::CharacterSelect> &players);
	void updateSnapshot(const RpgGameData::CharacterSelect &player);
	void updateSnapshot(const RpgGameData::PlayerBaseData &playerData, const RpgGameData::Player &player);
	void updateSnapshot(const RpgGameData::EnemyBaseData &enemyData, const RpgGameData::Enemy &enemy);
	void updateSnapshot(const RpgGameData::BulletBaseData &bulletData, const RpgGameData::Bullet &bullet);

	void updateSnapshot(const RpgGameData::ControlBaseData &lightData, const RpgGameData::ControlLight &light);
	void updateSnapshot(const RpgGameData::ControlContainerBaseData &containerData, const RpgGameData::ControlContainer &container);
	void updateSnapshot(const RpgGameData::ControlCollectionBaseData &collectionData, const RpgGameData::ControlCollection &collection);
	void updateSnapshot(const RpgGameData::PickableBaseData &pickableData, const RpgGameData::Pickable &pickable);
	void updateSnapshot(const RpgGameData::ControlGateBaseData &baseData, const RpgGameData::ControlGate &data);
	void updateSnapshot(const RpgGameData::ControlTeleportBaseData &baseData, const RpgGameData::ControlTeleport &data);


	// Outgoing snapshot

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	bool appendSnapshot(RpgGameDataInterface<T, T2> *iface, const qint64 &tick = -1, const bool &forced = false);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	void appendSnapshot(const T2 &baseData, const T &data)
	{
		Q_UNUSED(baseData);
		Q_UNUSED(data);
		LOG_CERROR("game") << "Missing specialization";
	}

	void appendSnapshot(const RpgGameData::PlayerBaseData &baseData, const RpgGameData::Player &data) {
		appendSnapshotToList(baseData, data, &m_players, m_lastPlayerTick);
	}

	void appendSnapshot(const RpgGameData::EnemyBaseData &baseData, const RpgGameData::Enemy &data) {
		appendSnapshotToList(baseData, data, &m_enemies, m_lastEnemyTick);
	}

	void appendSnapshot(const RpgGameData::BulletBaseData &baseData, const RpgGameData::Bullet &data) {
		appendSnapshotToList(baseData, data, &m_bullets, m_lastBulletTick);
	}


	bool hasSnapshot();

	RpgGameData::CurrentSnapshot renderCurrentSnapshot();

	// Remove snapshot
	void removeMissingSnapshots(const std::vector<RpgGameData::BulletBaseData> &bulletList);
	void removeMissingSnapshots(const std::vector<RpgGameData::PickableBaseData> &pickableList);

	void clear();

	QByteArray dump();

	const qint64 &serverTick() const;
	void setServerTick(const qint64 &newServerTick);

	const qint64 &deadlineTick() const;
	void setDeadlineTick(const qint64 &newDeadlineTick);

private:
	template <typename T, typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type>
	qint64 insert(std::map<qint64, T> *dst, const T &snap, const qint64 &lastSentTick);

	void updateLastTick(const RpgGameData::CurrentSnapshot &snapshot);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	void removeMissing(RpgGameData::SnapshotList<T, T2> &snapshot, const std::vector<T2> &list);


	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgGameData::Body, T>::value>::type,
			  typename = std::enable_if<std::is_base_of<RpgGameData::BaseData, T2>::value>::type>
	void appendSnapshotToList(const T2 &baseData, const T &data,
							  RpgGameData::SnapshotList<T, T2> *list, const qint64 &lastTick);

	qint64 m_lastPlayerTick = -1;
	qint64 m_lastEnemyTick = -1;
	qint64 m_lastBulletTick = -1;

	qint64 m_serverTick = -1;
	qint64 m_deadlineTick = -1;
};






/**
 * @brief ClientStorage::appendSnapshot
 * @param iface
 */

template<typename T, typename T2, typename T3, typename T4>
inline bool ClientStorage::appendSnapshot(RpgGameDataInterface<T, T2> *iface, const qint64 &tick, const bool &forced)
{
	Q_ASSERT(iface);

	if (forced) {
		appendSnapshot(iface->baseData(), iface->serialize(tick));
		return true;
	}

	if (const auto &ptr = iface->serializeCmp(tick)) {
		appendSnapshot(iface->baseData(), ptr.value());
		return true;
	}

	return false;
}







/**
 * @brief ClientStorage::appendSnapshot
 * @param baseData
 * @param data
 * @param list
 * @param lastTick
 */

template<typename T, typename T2, typename T3, typename T4>
inline void ClientStorage::appendSnapshotToList(const T2 &baseData, const T &data,
												RpgGameData::SnapshotList<T, T2> *list, const qint64 &lastTick)
{
	Q_ASSERT(list);

	auto it = std::find_if(list->begin(),
						   list->end(),
						   [&baseData](const auto &p) {
		return (p.data == baseData);
	});

	if (it == list->end()) {
		RpgGameData::SnapshotData<T, T2> d;
		d.data = baseData;
		insert(&d.list, data, lastTick);
		list->push_back(d);
	} else {
		insert(&(it->list), data, lastTick);
	}
}









/**
 * @brief ClientStorage::removeMissing
 * @param snapshot
 * @param list
 */

template<typename T, typename T2, typename T3, typename T4>
inline void ClientStorage::removeMissing(RpgGameData::SnapshotList<T, T2> &snapshot, const std::vector<T2> &list)
{
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
inline qint64 ClientStorage::insert(std::map<qint64, T> *dst, const T &snap, const qint64 &lastSentTick)
{
	Q_ASSERT(dst);

	qint64 realF = snap.f * 10;
	qint64 f = std::max(lastSentTick+1, realF);

	while (dst->contains(f))
		++f;

	T s2 = snap;
	s2.f = f;
	dst->insert_or_assign(f, s2);

	return f;
}




class ActionRpgMultiplayerGame;
class Server;


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

	const RpgConfig::GameState &gameState() const;
	void setGameState(const RpgConfig::GameState &newGameState);

	ClientStorage snapshots();

	void zapSnapshots(const qint64 &tick);
	RpgGameData::FullSnapshot getFullSnapshot(const qint64 &tick, const bool &findLast = false);
	RpgGameData::FullSnapshot getNextFullSnapshot(const qint64 &tick);
	RpgGameData::CurrentSnapshot getCurrentSnapshot();
	QList<RpgGameData::Message> takeMessageList();

	const RpgGameData::GameConfig &gameConfig() const;

	int playerId() const;
	void setPlayerId(int newPlayerId);

	bool isHost() const;
	void setIsHost(bool newIsHost);

	QList<RpgGameData::CharacterSelect> playerData();

signals:
	void gameError();
	void gameDataDownload(QString map, QList<RpgGameData::CharacterSelect> list);


protected:
	virtual void binaryDataReceived(const QList<QPair<QByteArray, unsigned int> > &list) override;

private:
	void updateState(const QCborMap &data);
	void updateSnapshot(const QList<RpgGameData::CharacterSelect> &players);
	void updateSnapshot(const RpgGameData::CurrentSnapshot &snapshot);
	void messageAdd(const RpgGameData::Message &message);

	void packetReceivedConnect(const QCborMap &data);
	void packetReceivedChrSel(const QCborMap &data);
	void packetReceivedDownload(const QCborMap &data);
	void packetReceivedPrepare(const QCborMap &data);
	void packetReceivedPlay(const QCborMap &data);
	void packetReceivedFinished(const QCborMap &data);

	void onConnectedToServer();

	template <typename T, typename T2>
	void updateSnapshotRemoveMissing(const RpgGameData::SnapshotList<T, T2> &list);



	QPointer<ActionRpgMultiplayerGame> m_game;
	bool m_downloadContentStarted = false;

#ifndef Q_OS_WASM
	QRecursiveMutex m_snapshotMutex;
#endif

	QList<RpgGameData::CharacterSelect> m_playerData;
	ClientStorage m_snapshots;
	QList<RpgGameData::Message> m_messageList;

	RpgGameData::GameConfig m_gameConfig;
	RpgConfig::GameState m_gameState = RpgConfig::StateInvalid;
	int m_playerId = -1;
	bool m_isHost = false;
};









#endif // RPGUDPENGINE_H
