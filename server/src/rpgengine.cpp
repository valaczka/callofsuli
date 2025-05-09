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



	/// ---- MEASURE ----


	enum Measure {
		Invalid,
		Received,
		Render,
		RenderFull,
		TimerTick,
		TimerUpd,
		BinaryRcv
	};

	struct MeasureData {
		qint64 min = -1;
		qint64 max = -1;
		qint64 med = -1;
		QList<qint64> data;

		qint64 avg() const {
			if (data.size() > 0) {
				qint64 sum = std::accumulate(data.constBegin(), data.constEnd(), 0);
				return sum/data.size();
			} else {
				return 0;
			}
		}

		void add(const qint64 &ms) {
			if (data.size() > limit)
				data.erase(data.constBegin(), data.constBegin()+(data.size()-limit-1));
			data.append(ms);

			QList<qint64> tmp = data;

			std::sort(tmp.begin(), tmp.end());

			if (const auto &s = tmp.size(); s % 2 == 0)
				med = (tmp.at(s / 2 - 1) + tmp.at(s / 2)) / 2;
			else
				med = tmp.at(s / 2);

			if (min < 0 || ms < min)
				min = ms;

			if (max < 0 || ms > max)
				max = ms;
		}

		int limit = 120;
	};

	QHash<Measure, MeasureData> m_renderData;

	QElapsedTimer m_renderTimer;

	void renderTimerStart() {
		if (m_renderTimer.isValid())
			m_renderTimer.restart();
		else
			m_renderTimer.start();
	}

	void renderTimerMeausure(const Measure &measure) {
		m_renderData[measure].add(m_renderTimer.restart());
	}

	void renderTimerMeausure(const Measure &measure, const qint64 &msec) {
		m_renderData[measure].add(msec);
	}

	QString renderTimerDump() const;
	QString engineDump() const;

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
 * @brief RpgEngine::binaryDataReceived
 * @param data
 */

void RpgEngine::binaryDataReceived(const UdpServerPeerReceivedList &data)
{
	for (const auto &pair : data) {
		binaryDataReceived(pair.first, pair.second);
	}

	QElapsedTimer t2;
	t2.start();

	d->updateState();

	d->renderTimerMeausure(RpgEnginePrivate::TimerUpd, t2.elapsed());

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		d->dataSendChrSel();
	else if (m_config.gameState == RpgConfig::StatePrepare)
		d->dataSendPrepare();
	else if (m_config.gameState == RpgConfig::StatePlay)
		d->dataSendPlay();

	d->renderTimerMeausure(RpgEnginePrivate::TimerTick, t2.elapsed());
}



/**
 * @brief RpgEngine::binaryDataReceived
 * @param peer
 * @param data
 */

void RpgEngine::binaryDataReceived(UdpServerPeer *peer, const QByteArray &data)
{
	Q_ASSERT(peer);

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

int RpgEngine::createEvents(const qint64 &tick, const RpgGameData::PlayerBaseData &data,
							const RpgGameData::Player &snap, const std::optional<RpgGameData::Player> &prev)
{
	if (!prev)
		return 0;

	if (prev->hp > 0 && snap.hp <= 0) {
		LOG_CINFO("engine") << "Player died" << data.o << data.id;
		eventAdd<RpgEventPlayerDied>(tick, data);
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

const RpgGameData::SnapshotList<RpgGameData::Player, RpgGameData::PlayerBaseData> &RpgEngine::players()
{
	return m_snapshots.players();
}

const RpgGameData::SnapshotList<RpgGameData::Enemy, RpgGameData::EnemyBaseData> &RpgEngine::enemies()
{
	return m_snapshots.enemies();
}

const RpgGameData::SnapshotList<RpgGameData::Bullet, RpgGameData::BulletBaseData> &RpgEngine::bullets()
{
	return m_snapshots.bullets();
}


/**
 * @brief RpgEngine::renderTimerLog
 * @param msec
 */

void RpgEngine::renderTimerLog(const qint64 &msec)
{
	d->renderTimerMeausure(RpgEnginePrivate::Render, msec);
}






/**
 * @brief RpgEngine::preparePlayers
 */

void RpgEngine::preparePlayers()
{
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
	return ++m_currentTick;
}
















/**
 * @brief RpgEnginePrivate::getPlayer
 * @param peer
 * @return
 */

RpgEnginePlayer *RpgEnginePrivate::getPlayer(UdpServerPeer *peer) const
{
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

	QCborMap m = QCborValue::fromCbor(data).toMap();

	RpgGameData::CharacterSelect c;
	c.fromCbor(m);


	c.playerId = player->playerId();			// Ő nem változtathat rajta
	player->setConfig(c);

	if (!player->isHost())
		return;

	m_gameConfig = c.gameConfig;


#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"), QString::fromUtf8(QJsonDocument(m.toJsonObject()).toJson()));
	q->service()->writeToSocket(map.toCborValue());
#endif
}





/**
 * @brief RpgEnginePrivate::dataReceivedPrepare
 * @param player
 * @param data
 */

void RpgEnginePrivate::dataReceivedPrepare(RpgEnginePlayer *player, const QByteArray &data)
{
	Q_ASSERT(player);

	QCborMap m = QCborValue::fromCbor(data).toMap();

	RpgGameData::Prepare config;

	config.fromCbor(m);

	if (!player->isPrepared()) {
		player->setIsPrepared(config.prepared);
		LOG_CINFO("game") << "PREPARED" << player->playerId() << config.prepared;
	}

	if (!player->isHost())
		return;

	m_gameConfig = config.gameConfig;

	RpgGameData::CurrentSnapshot snapshot;
	snapshot.fromCbor(m);

#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"),
			   QString::fromUtf8(QJsonDocument(m.toJsonObject()).toJson()) +
			   QStringLiteral("-------------------------------------------------\n") +
			   QString::fromUtf8(QJsonDocument(snapshot.toCbor().toJsonObject()).toJson())
			   );
	q->service()->writeToSocket(map.toCborValue());
#endif



	for (const auto &ptr : snapshot.enemies) {
		const RpgGameData::EnemyBaseData &enemy = ptr.data;

		if (enemy.t == RpgGameData::EnemyBaseData::EnemyInvalid) {
			LOG_CERROR("engine") << "Invalid enemy data" << q->id();
			continue;
		}

		const auto &enemies = q->m_snapshots.enemies();

		const auto it = std::find_if(enemies.cbegin(), enemies.cend(),
									 [&enemy](const auto &e) {
			return e.data == enemy;
		});

		if (it == enemies.cend()) {
			RpgGameData::Enemy edata;

			if (!ptr.list.empty())
				edata = ptr.list.cbegin()->second;

			edata.f = 0;
			edata.hp = 32;
			edata.mhp = 32;
			edata.arm.wl.append(RpgGameData::Weapon(RpgGameData::Weapon::WeaponLongsword, -1));
			edata.arm.cw = RpgGameData::Weapon::WeaponLongsword;

			q->m_snapshots.enemyAdd(enemy, edata);

			LOG_CINFO("eninge") << "CREATE ENEMY" << enemy.t << enemy.s << enemy.id << edata.p;
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

	QElapsedTimer timer2;
	timer2.start();

	QCborMap m = QCborValue::fromCbor(data).toMap();

	if (q->m_currentTick <= 0) {
		if (m.value(QStringLiteral("full")).toBool(false)) {
			LOG_CINFO("engine") << "FULLY PREPARED" << player->playerId();
			player->setIsFullyPrepared(true);
			return;
		}
	}


	q->m_snapshots.registerSnapshot(player, m);

	renderTimerMeausure(Received, timer2.elapsed());
}




/**
 * @brief RpgEnginePrivate::dataSendChrSel
 */

void RpgEnginePrivate::dataSendChrSel()
{
	RpgGameData::CharacterSelectServer config;
	config.gameConfig = m_gameConfig;

	for (const auto &ptr : q->m_player)
		config.players.append(ptr->config());

	QCborMap baseMap = config.toCborMap();

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = baseMap;

		insertBaseMapData(&map, it->get());

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), false);
	}


#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	map.insert(QStringLiteral("txt"), QString::fromUtf8(QJsonDocument(baseMap.toJsonObject()).toJson()));
	q->service()->writeToSocket(map.toCborValue());
#endif
}





/**
 * @brief RpgEnginePrivate::dataSendPrepare
 */

void RpgEnginePrivate::dataSendPrepare()
{
	RpgGameData::Prepare config;
	config.gameConfig = m_gameConfig;

	QCborMap baseMap = config.toCborMap();
	QCborMap sm = q->m_snapshots.getCurrentSnapshot().toCbor();

	for (auto it = sm.cbegin(); it != sm.cend(); ++it)
		baseMap.insert(it.key(), it.value());


	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = baseMap;
		insertBaseMapData(&map, it->get());

		if (it->get()->udpPeer())
			it->get()->udpPeer()->send(map.toCborValue().toCbor(), false);
	}

#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	map.insert(QStringLiteral("txt"), QString::fromUtf8(QJsonDocument(baseMap.toJsonObject()).toJson()));
	q->service()->writeToSocket(map.toCborValue());
#endif
}






/**
 * @brief RpgEnginePrivate::dataSendPlay
 */

void RpgEnginePrivate::dataSendPlay()
{
	const qint64 tick = q->currentTick();

	if (tick > 0) {
		if (tick <= m_lastSentTick+1)				// Clamp to 30 fps
			return;

		m_lastSentTick = tick;
	}

	QCborMap current = q->m_snapshots.getCurrentSnapshot().toCbor();
	current.insert(QStringLiteral("t"), tick);

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = current;

		insertBaseMapData(&map, it->get());

		if (UdpServerPeer *peer = it->get()->udpPeer()) {
			if (!peer->readyToSend())
				continue;

			peer->send(map.toCborValue().toCbor(), false);
			peer->setLastSentTick(tick);

		}
	}


#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));

	map.insert(QStringLiteral("txt"), renderTimerDump()+engineDump());

	//map.insert(QStringLiteral("txt"), QString::fromUtf8(QJsonDocument(current.toJsonObject()).toJson()));
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

			if (currMs-tMs >= 2.*1000./60.) {
				LOG_CWARNING("engine") << "Engine" << q->id() << "render lag" << currMs-tMs;
			}

			if (diff > 0) {

				QElapsedTimer t;

				t.start();

				QString txt;

				for (; diff > 0; --diff) {
					txt = q->m_snapshots.render(q->nextTick());
				}

				q->m_snapshots.renderEnd(txt);

				renderTimerMeausure(RenderFull, t.elapsed());
			}
		}


	}
}









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
 * @brief RpgEnginePrivate::renderTimerDump
 * @return
 */

QString RpgEnginePrivate::renderTimerDump() const
{
	QString txt;

	static const QHash<Measure, QString> hash = {
		{ Invalid, QStringLiteral("Invalid") },
		{ Received, QStringLiteral("Received") },
		{ Render, QStringLiteral("Render") },
		{ RenderFull, QStringLiteral("RenderFull") },
		{ TimerTick, QStringLiteral("TimerTick") },
		{ TimerUpd, QStringLiteral("TimerUpd") },
		{ BinaryRcv, QStringLiteral("BinaryRcv") },
	};

	for (const auto &[key, d] : m_renderData.asKeyValueRange()) {
		QString t;
		txt += QStringLiteral("[%1]: ").arg(hash.value(key, QStringLiteral("???")), 16);

		txt += QStringLiteral("avg: %1 med: %2 max: %3")
			   .arg(d.avg(), 3)
			   .arg(d.med, 3)
			   .arg(d.max, 3)
			   ;

		if (!d.data.isEmpty()) {
			const auto [min, max] = std::minmax_element(d.data.constBegin(), d.data.constEnd());
			txt += QStringLiteral(" [min: %1 max: %2]").arg(*min, 3).arg(*max, 3);
		}

		txt += '\n';
	}

	txt += QStringLiteral("-------------------------------------\n \n");

	/*txt += '\n';

	for (const auto &[key, d] : m_renderData.asKeyValueRange()) {
		txt += hash.value(key, QStringLiteral("???"));
		txt += QStringLiteral("\n-------------------------------------\n");

		for (int i=0; i<d.data.size() && i<40; ++i)
			txt += QStringLiteral("%1\n").arg(d.data.at(i), 5);

		txt += '\n';
	}*/

	return txt;
}



/**
 * @brief RpgEnginePrivate::engineDump
 * @return
 */

QString RpgEnginePrivate::engineDump() const
{
	QString txt;

	txt += QStringLiteral("[ENGINE %1]\n").arg(q->m_id);
	txt += QStringLiteral("-------------------------------------\n \n");
	txt += QStringLiteral("State: %1  |  Players: %2  |  Tick (current/last): %3/%4\n")
		   .arg(q->m_config.gameState, 2).arg(q->m_player.size(), 2)
		   .arg(q->m_currentTick, 5).arg(m_lastSentTick, 5);

	for (const auto &ptr: q->udpServer()->peerList()) {
		txt += QStringLiteral("Peer %1:%2 - ").arg(ptr->host()).arg(ptr->port());
		txt += QStringLiteral("RTT %1  |  FPS: %2  |  Peer FPS: %3\n")
			   .arg(ptr->currentRtt(), 4)
			   .arg(ptr->currentFps(), 2)
			   .arg(ptr->peerFps(), 2)
			   ;
	}

	txt += QStringLiteral(" \n");

	return txt;
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



/**
 * @brief RpgEventPlayerDied::RpgEventPlayerDied
 * @param engine
 * @param tick
 * @param data
 */

RpgEventPlayerDied::RpgEventPlayerDied(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
	: RpgEvent(engine, tick)
	, m_data(data)
{
	LOG_CDEBUG("engine") << "Player died" << m_data.o << m_data.id << "@" << tick;
}



/**
 * @brief RpgEventPlayerDied::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPlayerDied::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	LOG_CDEBUG("engine") << "Player died processed" << m_data.o << m_data.id << "@" << m_tick;

	m_engine->eventAdd<RpgEventPlayerResurrect>(m_tick+300, m_data);

	return true;
}



/**
 * @brief RpgEventPlayerDied::isEqual
 * @param other
 * @return
 */

bool RpgEventPlayerDied::isEqual(RpgEvent *other) const
{
	if (RpgEventPlayerDied *d = dynamic_cast<RpgEventPlayerDied*>(other); d &&
			d->m_tick == m_tick &&
			d->m_data == m_data
			)
		return true;

	return false;
}



/**
 * @brief RpgEventPlayerResurrect::RpgEventPlayerResurrect
 * @param engine
 * @param tick
 */

RpgEventPlayerResurrect::RpgEventPlayerResurrect(RpgEngine *engine, const qint64 &tick, const RpgGameData::PlayerBaseData &data)
	: RpgEvent(engine, tick)
	, m_data(data)
{
	LOG_CDEBUG("engine") << "PLAYER RESURRECT created" << tick << m_unique << data.o;
}


/**
 * @brief RpgEventPlayerResurrect::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPlayerResurrect::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->players();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		LOG_CERROR("engine") << "Invalid player" << m_data.o << m_data.id;
		return true;
	}

	RpgGameData::Player player;

	if (!it->list.empty())
		player = std::prev(it->list.cend())->second;

	player.f = m_tick;
	player.hp = 45;

	LOG_CINFO("engine") << "###### RESURRECT PLAYER" << m_tick << m_data.id;

	dst->assign(dst->players, m_data, player);

	return true;
}




/**
 * @brief RpgEventPlayerResurrect::isEqual
 * @param other
 * @return
 */

bool RpgEventPlayerResurrect::isEqual(RpgEvent *other) const
{
	if (RpgEventPlayerResurrect *d = dynamic_cast<RpgEventPlayerResurrect*>(other); d &&
			d->m_data == m_data &&
			d->m_tick == m_tick
			)
		return true;

	return false;
}
