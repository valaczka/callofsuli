/*
 * ---- Call of Suli ----
 *
 * rpgengine.cpp
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

#include "rpgengine.h"
#include "Logger.h"
#include "serverservice.h"
#include "udpserver.h"
#include <QCborArray>
#include <QCborMap>


int RpgEngine::m_nextId = 1;



/**
 * @brief The RpgEnginePrivate class
 */

class RpgEnginePrivate
{
private:
	RpgEnginePrivate(RpgEngine *engine)
		: q(engine)
	{}

	RpgEnginePlayer* getPlayer(UdpServerPeer *peer) const;

	void dataReceived(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPrepare(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPlay(RpgEnginePlayer *player, const QByteArray &data);

	void dataSendChrSel();
	void dataSendPrepare();
	void dataSendPlay();

	void insertBaseMapData(QCborMap *dst, RpgEnginePlayer *player);

	void updateState();



	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEvent, T>::value>::type>
	void eventAdd(Args &&...args);

	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEvent, T>::value>::type>
	void eventAddLater(Args &&...args);


	RpgGameData::CurrentSnapshot processEvents(const qint64 &tick);


	RpgGameData::GameConfig m_gameConfig;

	QElapsedTimer m_elapsedTimer;
	qint64 m_lastSentTick = -1;

	std::vector<std::unique_ptr<RpgEvent>> m_events;
	std::vector<std::unique_ptr<RpgEvent>> m_eventsLater;
	bool m_eventsProcessing = false;

	RpgEngine *q;

	friend class RpgEngine;
};






/**
 * @brief RpgEngine::RpgEngine
 * @param handler
 * @param parent
 */

RpgEngine::RpgEngine(EngineHandler *handler, QObject *parent)
	: UdpEngine(EngineRpg, handler, parent)
	, d(new RpgEnginePrivate(this))
	, m_snapshots(this)
{
	m_config.gameState = RpgConfig::StateCharacterSelect;
}


/**
 * @brief RpgEngine::~RpgEngine
 */

RpgEngine::~RpgEngine()
{
	QMutexLocker locker(&m_engineMutex);

	delete d;
	d = nullptr;
}


/**
 * @brief RpgEngine::engineCreate
 * @param handler
 * @return
 */

std::shared_ptr<RpgEngine> RpgEngine::engineCreate(EngineHandler *handler, UdpServer *server)
{
	if (!handler)
		return {};

	LOG_CDEBUG("engine") << "Create RpgEngine" << m_nextId << server;


	auto ptr = std::make_shared<RpgEngine>(handler);
	ptr->setId(m_nextId);
	increaseNextId();

	ptr->setUdpServer(server);
	return ptr;
}


/**
 * @brief RpgEngine::timerTick
 */

void RpgEngine::timerTick()
{
	if (!m_udpServer) {
		LOG_CERROR("engine") << "Missing UdpServer";
		return;
	}

	QMutexLocker locker(&m_engineMutex);

	d->updateState();

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		d->dataSendChrSel();
	else if (m_config.gameState == RpgConfig::StatePrepare)
		d->dataSendPrepare();
	else if (m_config.gameState == RpgConfig::StatePlay)
		d->dataSendPlay();

}



/**
 * @brief RpgEngine::binaryDataReceived
 * @param peer
 * @param data
 */

void RpgEngine::binaryDataReceived(UdpServerPeer *peer, const QByteArray &data)
{
	Q_ASSERT(peer);

	QMutexLocker locker(&m_engineMutex);

	RpgEnginePlayer *player = d->getPlayer(peer);

	if (!player) {
		LOG_CERROR("engine") << "Player not found" << peer;
		return;
	}

	d->dataReceived(player, data);
}


/**
 * @brief RpgEngine::udpPeerAdd
 * @param peer
 */

void RpgEngine::udpPeerAdd(UdpServerPeer *peer)
{
	QMutexLocker locker(&m_engineMutex);

	bool isHost = m_player.empty();			// TODO

	const auto &ptr = m_player.emplace_back(std::make_unique<RpgEnginePlayer>(peer, false));
	ptr->setPlayerId(m_nextPlayerId++);
	ptr->id = 1;		// Minden player azonosítója 1
	ptr->m_config.nickname = QStringLiteral("PLAYER #%1").arg(ptr->playerId());

	if (isHost)
		setHostPlayer(ptr.get());

	LOG_CDEBUG("engine") << "Add player" << ptr->o << peer << isHost << ptr->isHost();

}



/**
 * @brief RpgEngine::udpPeerRemove
 * @param peer
 */

void RpgEngine::udpPeerRemove(UdpServerPeer *peer)
{
	QMutexLocker locker(&m_engineMutex);

	LOG_CDEBUG("engine") << "Remove player" << peer;

	std::erase_if(m_player, [peer](const auto &ptr) { return ptr->udpPeer() == peer; });

	// TODO: next host
}



/**
 * @brief RpgEngine::setHostPlayer
 * @param newHostPlayer
 */

void RpgEngine::setHostPlayer(RpgEnginePlayer *newHostPlayer)
{
	QMutexLocker locker(&m_engineMutex);

	for (const auto &ptr : m_player) {
		ptr->setIsHost(false);
	}

	if (newHostPlayer)
		newHostPlayer->setIsHost(true);

	m_hostPlayer = newHostPlayer;
}




/**
 * @brief RpgEngine::currentTick
 * @return
 */

qint64 RpgEngine::currentTick()
{
	QMutexLocker locker(&m_engineMutex);

	return m_currentTick;
}






/**
 * @brief RpgEngine::createEvents
 * @param tick
 * @param data
 * @param snap
 * @param prev
 * @return
 */

int RpgEngine::createEvents(const qint64 &tick, const RpgGameData::EnemyBaseData &data,
							const RpgGameData::Enemy &snap, const std::optional<RpgGameData::Enemy> &prev)
{
	QMutexLocker locker(&m_engineMutex);

	if (!prev)
		return 0;

	if (prev->hp > 0 && snap.hp <= 0) {
		LOG_CINFO("engine") << "Enemy died" << data.o << data.id;
		eventAdd<RpgEventEnemyDied>(tick, data);
		return 1;
	}

	return 0;
}



/**
 * @brief RpgEngine::createEvents
 * @param tick
 * @param data
 * @param snap
 * @param prev
 * @return
 */

int RpgEngine::createEvents(const qint64 &tick, const RpgGameData::PlayerBaseData &data, const RpgGameData::Player &snap, const std::optional<RpgGameData::Player> &prev)
{
	return -1;
}



/**
 * @brief RpgEngine::createEvents
 * @param tick
 * @param data
 * @param snap
 * @param prev
 * @return
 */

int RpgEngine::createEvents(const qint64 &tick, const RpgGameData::BulletBaseData &data, const RpgGameData::Bullet &snap, const std::optional<RpgGameData::Bullet> &prev)
{
	return -1;
}



/**
 * @brief RpgEngine::processEvents
 * @return
 */

RpgGameData::CurrentSnapshot RpgEngine::processEvents(const qint64 &tick)
{
	return d->processEvents(tick);
}








/**
 * @brief RpgEngine::players
 * @return
 */

RpgGameData::SnapshotList<RpgGameData::Player, RpgGameData::PlayerBaseData> RpgEngine::players()
{
	return m_snapshots.players();
}

RpgGameData::SnapshotList<RpgGameData::Enemy, RpgGameData::EnemyBaseData> RpgEngine::enemies()
{
	return m_snapshots.enemies();
}

RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData> RpgEngine::bullets()
{
	return m_snapshots.bullets();
}






/**
 * @brief RpgEngine::preparePlayers
 */

void RpgEngine::preparePlayers()
{
	QMutexLocker locker(&m_engineMutex);

	if (d->m_gameConfig.positionList.isEmpty())
		return;

	int idx = 0;

	for (auto &ptr : m_player) {
		if (ptr->s != -1)
			continue;

		LOG_CDEBUG("engine") << "Prepare player" << (idx+1);

		RpgGameData::PlayerPosition pos;

		if (idx >= d->m_gameConfig.positionList.size()) {
			LOG_CERROR("engine") << "Missing player positions" << id();
			/*pos.x = 0;
			pos.y = 0;
			pos.scene = 0;*/

			pos.x = d->m_gameConfig.positionList.at(0).x - idx*30;
			pos.y = d->m_gameConfig.positionList.at(0).y - idx*30;
			pos.scene = d->m_gameConfig.positionList.at(0).scene;

		} else {
			pos = d->m_gameConfig.positionList.at(idx);
			++idx;
			/*if (idx >= d->m_gameConfig.positionList.size())
				idx = 0; */
		}

		// Scene = 0 minden player esetében
		ptr->s = 0;

		RpgGameData::Player pdata;
		pdata.p = {pos.x, pos.y};
		pdata.f = 0;
		pdata.sc = pos.scene;
		pdata.hp = 13;			// TODO: from m_config
		pdata.mhp = 13;
		pdata.arm.wl.append(RpgGameData::Weapon(RpgGameData::Weapon::WeaponLongsword, 85));
		pdata.arm.wl.append(RpgGameData::Weapon(RpgGameData::Weapon::WeaponShortbow, 125));
		pdata.arm.cw = RpgGameData::Weapon::WeaponShortbow;

		m_snapshots.playerAdd(*ptr, pdata);

		LOG_CINFO("engine") << "SET PLAYER" << ptr->id << pdata.p << ptr->s << pdata.sc;
	}
}



/**
 * @brief RpgEngine::nextTick
 * @return
 */

qint64 RpgEngine::nextTick()
{
	QMutexLocker locker(&m_engineMutex);

	return ++m_currentTick;
}
















/**
 * @brief RpgEnginePrivate::getPlayer
 * @param peer
 * @return
 */

RpgEnginePlayer *RpgEnginePrivate::getPlayer(UdpServerPeer *peer) const
{
	QMutexLocker locker(&q->m_engineMutex);

	auto it = std::find_if(q->m_player.begin(), q->m_player.end(), [peer](const auto &ptr) { return ptr->udpPeer() == peer; });

	if (it == q->m_player.end())
		return nullptr;
	else
		return it->get();
}







/**
 * @brief RpgEnginePrivate::dataReceived
 * @param player
 * @param data
 */

void RpgEnginePrivate::dataReceived(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	QMutexLocker locker(&q->m_engineMutex);

	if (q->m_config.gameState == RpgConfig::StateCharacterSelect)
		dataReceivedChrSel(player, data);
	else if (q->m_config.gameState == RpgConfig::StatePrepare)
		dataReceivedPrepare(player, data);
	else if (q->m_config.gameState == RpgConfig::StatePlay)
		dataReceivedPlay(player, data);


}


/**
 * @brief RpgEnginePrivate::dataReceivedChrSel
 * @param player
 * @param cdata
 */

void RpgEnginePrivate::dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	QMutexLocker locker(&q->m_engineMutex);

	QCborMap m = QCborValue::fromCbor(data).toMap();

	if (const auto &it = m.find(QStringLiteral("chr")); it != m.cend()) {
		RpgGameData::CharacterSelect c;
		c.fromCbor(it.value());
		c.playerId = player->playerId();			// Ő nem változtathat rajta
		player->setConfig(c);
	}

	if (!player->isHost())
		return;

	if (const auto &it = m.find(QStringLiteral("cfg")); it != m.cend()) {
		m_gameConfig.fromCbor(it.value());
	}

	/*if (const auto &it = m.find(QStringLiteral("t")); it != m.cend()) {
		q->m_currentTick = it->toInteger();
	}*/

	/*if (const auto &it = m.find(QStringLiteral("ee")); it != m.cend()) {
		const QCborArray enemies = it->toArray();

		m_enemies.clear();

		for (const QCborValue &v : enemies) {
			RpgGameData::Enemy enemy;
			enemy.fromCbor(v);
			m_enemies.push_back(enemy);
		}
	}*/

	/*if (const auto &it = m.find(QStringLiteral("pp")); it != m.cend()) {
		const QCborArray players = it->toArray();

		for (const QCborValue &v : players) {
			QCborMap m = v.toMap();

			const int sceneId = m.value(QStringLiteral("s")).toInteger();
			const int id = m.value(QStringLiteral("id")).toInteger();

			if (sceneId <= 0 || id <= 0) {
				LOG_CWARNING("engine") << "Invalid id" << sceneId << id;
				continue;
			}

			if (player->s != sceneId || player->id != id)
				continue;

			player->fromCbor(v);
		}
	}*/
}



/**
 * @brief RpgEnginePrivate::dataReceivedPrepare
 * @param player
 * @param data
 */

void RpgEnginePrivate::dataReceivedPrepare(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	QMutexLocker locker(&q->m_engineMutex);

	QCborMap m = QCborValue::fromCbor(data).toMap();

	if (const auto &it = m.find(QStringLiteral("pr")); it != m.cend()) {
		RpgGameData::Prepare c;
		c.fromCbor(it.value());
		if (!player->isPrepared()) {
			player->setIsPrepared(c.prepared);
			LOG_CINFO("game") << "PREPARED" << player->playerId() << c.prepared;
		}
	}

	if (!player->isHost())
		return;

	if (const auto &it = m.find(QStringLiteral("cfg")); it != m.cend()) {
		m_gameConfig.fromCbor(it.value());
	}

	if (const auto &it = m.find(QStringLiteral("ee")); it != m.cend()) {
		QCborArray eList = it->toArray();

		for (const QCborValue &v : eList) {
			RpgGameData::EnemyBaseData enemy;
			enemy.fromCbor(v);

			if (enemy.t == RpgGameData::EnemyBaseData::EnemyInvalid) {
				LOG_CERROR("engine") << "Invalid enemy data" << q->id() << v;
				continue;
			}

			const auto &enemies = q->m_snapshots.enemies();

			const auto it = std::find_if(enemies.cbegin(), enemies.cend(),
										 [&enemy](const auto &e) {
				return e.data == enemy;
			});

			if (it == enemies.cend()) {
				RpgGameData::Enemy edata;
				edata.f = 0;
				edata.hp = 32;
				edata.mhp = 32;
				edata.arm.wl.append(RpgGameData::Weapon(RpgGameData::Weapon::WeaponLongsword, -1));
				edata.arm.cw = RpgGameData::Weapon::WeaponLongsword;

				q->m_snapshots.enemyAdd(enemy, edata);

				LOG_CINFO("eninge") << "CREATE ENEMY" << enemy.t << enemy.s << enemy.id;
			}
		}
	}

}


/**
 * @brief RpgEnginePrivate::dataReceivedPlay
 * @param player
 * @param data
 */

void RpgEnginePrivate::dataReceivedPlay(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	QMutexLocker locker(&q->m_engineMutex);

	QCborMap m = QCborValue::fromCbor(data).toMap();

	if (q->m_currentTick <= 0) {
		if (m.value(QStringLiteral("full")).toBool(false)) {
			player->setIsFullyPrepared(true);
			return;
		}
	}


	q->m_snapshots.registerSnapshot(player, m);
}




/**
 * @brief RpgEnginePrivate::dataSendChrSel
 */

void RpgEnginePrivate::dataSendChrSel()
{
	QMutexLocker locker(&q->m_engineMutex);

	QCborArray players;

	for (const auto &ptr : q->m_player) {
		players.append(ptr->config().toCborMap(true));
	}

	QCborMap gConfig = m_gameConfig.toCborMap(true);

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map;

		insertBaseMapData(&map, it->get());

		QCborMap configMap = it->get()->config().toCborMap(true);
		map.insert(QStringLiteral("cfg"), configMap);

		map.insert(QStringLiteral("g"), gConfig);
		map.insert(QStringLiteral("pp"), players);

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), false);

	}
}





/**
 * @brief RpgEnginePrivate::dataSendPrepare
 */

void RpgEnginePrivate::dataSendPrepare()
{
	QMutexLocker locker(&q->m_engineMutex);

	QCborMap gConfig = m_gameConfig.toCborMap(true);

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = q->m_snapshots.getCurrentSnapshot().toCbor();

		insertBaseMapData(&map, it->get());

		map.insert(QStringLiteral("g"), gConfig);

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), false);
	}
}






/**
 * @brief RpgEnginePrivate::dataSendPlay
 */

void RpgEnginePrivate::dataSendPlay()
{
	QMutexLocker locker(&q->m_engineMutex);

	const qint64 tick = q->currentTick();

	if (tick > 0) {
		if (tick <= m_lastSentTick)
			return;

		m_lastSentTick = tick;
	}

	QCborMap current = q->m_snapshots.getCurrentSnapshot().toCbor();
	current.insert(QStringLiteral("t"), tick);

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = current;

		insertBaseMapData(&map, it->get());

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), false);
	}


#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"), QString::fromUtf8(QJsonDocument(current.toJsonObject()).toJson()));
	q->service()->writeToSocket(map.toCborValue());

#endif
}




/**
 * @brief RpgEnginePrivate::insertBaseMapData
 * @param dst
 * @param player
 */


void RpgEnginePrivate::insertBaseMapData(QCborMap *dst, RpgEnginePlayer *player)
{
	Q_ASSERT(dst);
	Q_ASSERT(player);

	dst->insert(QStringLiteral("st"), q->m_config.gameState);
	dst->insert(QStringLiteral("hst"), player->isHost());
	dst->insert(QStringLiteral("ply"), player->playerId());
}







/**
 * @brief RpgEnginePrivate::updateState
 */

void RpgEnginePrivate::updateState()
{
	QMutexLocker locker(&q->m_engineMutex);

	if (q->m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (q->m_config.gameState == RpgConfig::StateConnect)
		return;

	if (q->m_config.gameState == RpgConfig::StateCharacterSelect) {
		bool allCompleted = !q->m_player.empty();

		for (const auto &ptr : q->m_player) {
			if (!ptr->config().completed) {
				allCompleted = false;
				break;
			}
		}

		if (allCompleted) {
			LOG_CDEBUG("engine") << "All players completed" << q << q->id();
			q->m_config.gameState = RpgConfig::StatePrepare;
		}
		return;
	}

	if (q->m_config.gameState == RpgConfig::StatePrepare) {
		bool allPrepared = true;
		bool hasNoScene = false;

		if (m_gameConfig.positionList.empty())
			allPrepared = false;

		for (const auto &ptr : q->m_player) {
			if (!ptr->isPrepared())
				allPrepared = false;

			if (ptr->s == -1)
				hasNoScene = true;
		}

		if (hasNoScene) {
			q->preparePlayers();
		} else if (allPrepared) {
			LOG_CDEBUG("engine") << "All players prepared" << q << q->id();
			q->m_config.gameState = RpgConfig::StatePlay;
		}

		return;
	}


	if (q->m_config.gameState == RpgConfig::StatePlay) {
		if (!m_elapsedTimer.isValid()) {
			bool allFullyPrepared = true;

			for (const auto &ptr : q->m_player) {
				if (!ptr->isFullyPrepared()) {
					allFullyPrepared = false;
					break;
				}
			}

			if (allFullyPrepared)
				m_elapsedTimer.start();

		} else {

			const double tMs = q->currentTick() * 1000./60.;
			const double currMs = m_elapsedTimer.elapsed();
			int diff = (currMs-tMs) * 60./1000.;

			if (currMs-tMs >= 2*1000./60.) {
				LOG_CWARNING("engine") << "Engine" << q->id() << "render lag" << currMs-tMs;
			}

			for (; diff > 0; --diff) {
				q->m_snapshots.render(q->nextTick());
			}
		}


	}
}



/**
 * @brief RpgEnginePrivate::createEvents
 * @param tick
 * @param data
 * @param snap
 * @param prev
 * @return
 */









/**
 * @brief RpgEvent::RpgEvent
 * @param m_tick
 */

RpgEvent::RpgEvent(RpgEngine *engine, const qint64 &tick, const bool &unique)
	: m_engine(engine)
	, m_tick(tick)
	, m_unique(unique)
{

}



/**
 * @brief RpgEvent::~RpgEvent
 */

RpgEvent::~RpgEvent()
{

}



/**
 * @brief RpgEnginePrivate::eventAddLater
 * @param args
 */
template<typename T, typename ...Args, typename T3>
void RpgEnginePrivate::eventAddLater(Args &&...args)
{
	QMutexLocker locker(&q->m_engineMutex);
	std::unique_ptr<T> e(new T(q, std::forward<Args>(args)...));

	for (const auto &ptr : m_events) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			LOG_CWARNING("engine") << "Event unique constraint failed";
			return;
		}
	}

	for (const auto &ptr : m_eventsLater) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			LOG_CWARNING("engine") << "Event unique constraint failed";
			return;
		}
	}

	m_eventsLater.push_back(std::move(e));
}



/**
 * @brief RpgEnginePrivate::eventAdd
 * @param args
 */

template<typename T, typename ...Args, typename T3>
void RpgEnginePrivate::eventAdd(Args &&...args)
{
	QMutexLocker locker(&q->m_engineMutex);

	if (m_eventsProcessing) {
		LOG_CDEBUG("engine") << "...........";
		eventAddLater<T>(std::forward<Args>(args)...);
		return;
	}

	std::unique_ptr<T> e(new T(q, std::forward<Args>(args)...));

	for (const auto &ptr : m_events) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			LOG_CWARNING("engine") << "Event unique constraint failed";
			return;
		}
	}

	m_events.push_back(std::move(e));
}





/**
 * @brief RpgEngine::eventAdd
 * @param args
 */

template<typename T, typename ...Args, typename T3>
void RpgEngine::eventAdd(Args &&...args)
{
	d->eventAdd<T>(std::forward<Args>(args)...);
}



template<typename T, typename ...Args, typename T3>
void RpgEngine::eventAddLater(Args &&...args)
{
	d->eventAddLater<T>(std::forward<Args>(args)...);
}



/**
 * @brief RpgEnginePrivate::processEvents
 * @return
 */

RpgGameData::CurrentSnapshot RpgEnginePrivate::processEvents(const qint64 &tick)
{
	QMutexLocker locker(&q->m_engineMutex);

	m_eventsProcessing = true;

	RpgGameData::CurrentSnapshot snapshot;

	for (auto it = m_events.begin(); it != m_events.end(); ) {
		if (it->get()->process(tick, &snapshot))
			it = m_events.erase(it);
		else
			++it;
	}

	for (auto &ptr : m_eventsLater)
		m_events.push_back(std::move(ptr));

	m_eventsLater.clear();

	m_eventsProcessing = false;

	return snapshot;
}





/**
 * @brief RpgEventEnemyDied::RpgEventEnemyDied
 * @param tick
 * @param data
 */

RpgEventEnemyDied::RpgEventEnemyDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::EnemyBaseData &data)
	: RpgEvent(engine, tick)
	, m_data(data)
{
	LOG_CDEBUG("engine") << "Enemy died" << m_data.o << m_data.id << "@" << tick;
}



/**
 * @brief RpgEventEnemyDied::process
 * @return
 */

bool RpgEventEnemyDied::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	LOG_CDEBUG("engine") << "Enemy died processed" << m_data.o << m_data.id << "@" << m_tick;

	bool allDead = true;

	for (const auto &ptr : m_engine->enemies()) {
		if (ptr.data == m_data)
			continue;

		if (ptr.list.empty()) {
			allDead = false;
			break;
		}

		if (std::prev(ptr.list.cend())->second.hp > 0) {
			allDead = false;
			break;
		}
	}

	if (allDead) {
		LOG_CINFO("engine") << "ALL DEAD";
		m_engine->eventAdd<RpgEventEnemyResurrect>(m_tick+180);
	}

	return true;
}



/**
 * @brief RpgEventEnemyDied::isEqual
 * @param other
 * @return
 */

bool RpgEventEnemyDied::isEqual(RpgEvent *other) const
{
	if (RpgEventEnemyDied *d = dynamic_cast<RpgEventEnemyDied*>(other); d &&
			d->m_tick == m_tick &&
			d->m_data == m_data
			)
		return true;

	return false;
}


/**
 * @brief RpgEventEnemyResurrect::RpgEventEnemyResurrect
 * @param engine
 * @param tick
 */

RpgEventEnemyResurrect::RpgEventEnemyResurrect(RpgEngine *engine, const qint64 &tick)
	: RpgEvent(engine, tick)
{
	LOG_CDEBUG("engine") << "RESURRECT created" << tick << m_unique;
}


/**
 * @brief RpgEventEnemyResurrect::process
 * @return
 */

bool RpgEventEnemyResurrect::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	for (const auto &ptr : m_engine->enemies()) {
		if (ptr.list.empty())
			continue;

		RpgGameData::Enemy edata = std::prev(ptr.list.cend())->second;
		edata.f = m_tick;
		edata.hp = 32;

		LOG_CINFO("engine") << "###### RESURRECT" << m_tick << ptr.data.id;

		dst->assign(dst->enemies, ptr.data, edata);
	}

	return true;
}


/**
 * @brief RpgEventEnemyResurrect::isEqual
 * @param other
 * @return
 */

bool RpgEventEnemyResurrect::isEqual(RpgEvent *other) const
{
	if (RpgEventEnemyResurrect *d = dynamic_cast<RpgEventEnemyResurrect*>(other); d &&
			d->m_tick == m_tick
			)
		return true;

	return false;
}




