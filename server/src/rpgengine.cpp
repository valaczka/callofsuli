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
	bool isHostPrepared() const;

	void dataReceived(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPrepare(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPlay(RpgEnginePlayer *player, const QByteArray &data);

	void dataSendChrSel();
	void dataSendPrepare();
	void dataSendPlay();

	void insertBaseMapData(QCborMap *dst, RpgEnginePlayer *player);

	void updateState();

	RpgGameData::GameConfig m_gameConfig;

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

	if (m_timer.isValid())
		return m_timer.elapsed() + m_startTick;
	else
		return -1;
}



/**
 * @brief RpgEngine::preparePlayers
 */

void RpgEngine::preparePlayers()
{
	QMutexLocker locker(&m_engineMutex);

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

		m_snapshots.playerAdd(*ptr, pdata);

		LOG_CINFO("engine") << "SET PLAYER" << ptr->id << pdata.p << ptr->s << pdata.sc;
	}
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
 * @brief RpgEnginePrivate::isHostPrepared
 * @return
 */

bool RpgEnginePrivate::isHostPrepared() const
{
	QMutexLocker locker(&q->m_engineMutex);

	return q->m_hostPlayer && q->m_hostPlayer->isPrepared();
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
			LOG_CINFO("game") << "PREPARED" << player->playerId();
		}
	}

	if (!player->isHost())
		return;

	if (const auto &it = m.find(QStringLiteral("cfg")); it != m.cend()) {
		m_gameConfig.fromCbor(it.value());
	}

	/*if (const auto &it = m.find(QStringLiteral("ee")); it != m.cend()) {
		QCborArray eList = it->toArray();

		for (const QCborValue &v : eList) {
			RpgGameData::Enemy enemy;
			enemy.fromCbor(v);

			if (enemy.t == RpgGameData::Enemy::EnemyInvalid) {
				LOG_CERROR("engine") << "Invalid enemy data" << q->id() << v;
				continue;
			}

			const auto it = std::find_if(q->m_enemies.cbegin(), q->m_enemies.cend(),
										 [&enemy](const RpgGameData::Enemy &e) {
				return e.RpgGameData::Body::isEqual(enemy);
			});

			if (it == q->m_enemies.cend()) {
				q->m_enemies.push_back(enemy);
				LOG_CINFO("eninge") << "CREATE ENEMY" << enemy.t << enemy.s << enemy.id;
			}
		}
	}*/

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
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), true);

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
		map.insert(QStringLiteral("hostPrepared"), isHostPrepared());

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), true);
	}
}






/**
 * @brief RpgEnginePrivate::dataSendPlay
 */

void RpgEnginePrivate::dataSendPlay()
{
	QMutexLocker locker(&q->m_engineMutex);

	const qint64 tick = q->currentTick();

	QCborMap current = q->m_snapshots.getCurrentSnapshot().toCbor();
	current.insert(QStringLiteral("t"), tick);

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = current;

		insertBaseMapData(&map, it->get());

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), true);


		/*static QElapsedTimer tmr;

		if (tmr.isValid()) {
			if (tmr.hasExpired(500)) {
				LOG_CDEBUG("engine") << "***" << QJsonDocument(map.toJsonObject()).toJson().constData();

				tmr.restart();
			}
		} else
			tmr.start();*/
	}

	q->m_snapshots.zapSnapshots(tick - 4000);
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
		bool hostPrepared = false;
		bool hasNoScene = false;

		if (m_gameConfig.positionList.empty())
			allPrepared = false;

		for (const auto &ptr : q->m_player) {
			if (!ptr->isPrepared())
				allPrepared = false;

			if (ptr->isHost())
				hostPrepared = ptr->isPrepared();

			if (ptr->s == -1)
				hasNoScene = true;
		}

		if (hostPrepared && hasNoScene) {
			q->preparePlayers();
		} else if (allPrepared) {
			LOG_CDEBUG("engine") << "All players prepared" << q << q->id();
			q->m_config.gameState = RpgConfig::StatePlay;
			q->m_timer.start();
		}

		return;
	}
}






