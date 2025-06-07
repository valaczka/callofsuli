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

	enum SendMode {
		SendNone = 0,
		SendChrSel,
		SendPrepare,
		SendReconnect
	};

	void dataSend(const SendMode &mode, RpgEnginePlayer *player = nullptr);
	void dataSendPlay();

	void insertBaseMapData(QCborMap *dst, RpgEnginePlayer *player);

	void updateState();

	void updatePeers();
	bool reconnectPeer(UdpServerPeer *peer);



	void createEnemies(const RpgGameData::CurrentSnapshot &snapshot);
	void createControls(const RpgGameData::CurrentSnapshot &snapshot);
	void createCollection();
	int relocateCollection(const RpgGameData::ControlCollectionBaseData &base, const qint64 &tick, QPointF *ptr);
	bool finishCollection(const RpgGameData::ControlCollectionBaseData &base, const int &idx);

	void createRandomizer();


	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	void eventAdd(Args &&...args);

	template <typename T, typename ...Args,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	void eventAddLater(Args &&...args);

	template <typename T, typename T2,
			  typename = std::enable_if<std::is_base_of<RpgEventBase, T>::value>::type>
	T* eventFind(const T2 &baseData);

	void eventRemove(RpgEventBase *event);


	RpgGameData::CurrentSnapshot processEvents(const qint64 &tick);


	RpgGameData::GameConfig m_gameConfig;
	std::optional<RpgGameData::Randomizer> m_randomizer;

	QElapsedTimer m_elapsedTimer;
	qint64 m_elapsedTimerReference = 0;
	qint64 m_lastSentTick = -1;
	int m_lastMyObjectId = 0;

	std::vector<std::unique_ptr<RpgEventBase>> m_events;
	std::vector<std::unique_ptr<RpgEventBase>> m_eventsLater;
	QList<RpgEventBase*> m_eventsRemove;
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
 * @brief RpgEngine::canDelete
 * @param useCount
 * @return
 */

bool RpgEngine::canDelete(const int &useCount)
{
	if (m_config.gameState == RpgConfig::StatePlay)
		return false;
	else
		return AbstractEngine::canDelete(useCount);
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
		d->dataSend(RpgEnginePrivate::SendChrSel);
	else if (m_config.gameState == RpgConfig::StatePrepare)
		d->dataSend(RpgEnginePrivate::SendPrepare);
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
	if (d->reconnectPeer(peer))
		return;

	bool isHost = m_player.empty();			// TODO

	const auto &ptr = m_player.emplace_back(std::make_unique<RpgEnginePlayer>(peer, false));
	ptr->setPlayerId(m_nextPlayerId++);
	ptr->id = 1;		// Minden player azonosítója 1
	ptr->m_config.nickname = QStringLiteral("PLAYER #%1").arg(ptr->playerId());

	if (isHost)
		setHostPlayer(ptr.get());

	LOG_CDEBUG("engine") << "Add player" << ptr->o << peer << isHost << ptr->isHost();

	d->updatePeers();
}



/**
 * @brief RpgEngine::udpPeerRemove
 * @param peer
 */

void RpgEngine::udpPeerRemove(UdpServerPeer *peer)
{
	RpgEnginePlayer *player = d->getPlayer(peer);

	LOG_CDEBUG("engine") << "Remove player" << peer << player;

	if (m_config.gameState == RpgConfig::StatePlay) {
		if (player) {
			LOG_CDEBUG("engine") << "Remove UDP PEER" << peer << "from player" << player;
			player->setUdpPeer(nullptr);
			eventAdd<RpgEventControlUnlock>(m_currentTick+600, *player);
		}
	} else {
		std::erase_if(m_player, [peer](const auto &ptr) { return ptr->udpPeer() == peer; });
	}

	if (player && m_hostPlayer == player) {
		LOG_CERROR("engine") << "No host";
		setHostPlayer(nullptr);
	}

	d->updatePeers();
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

	if (newHostPlayer) {
		newHostPlayer->setIsHost(true);
		LOG_CDEBUG("engine") << "New host player:" << newHostPlayer->o;
	}

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
 * @brief RpgEngine::nextObjectId
 * @return
 */

int RpgEngine::nextObjectId() const
{
	return ++d->m_lastMyObjectId;
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

const RpgGameData::SnapshotList<RpgGameData::ControlLight, RpgGameData::ControlBaseData> &RpgEngine::controlLights()
{
	return m_snapshots.controls().lights;
}

const RpgGameData::SnapshotList<RpgGameData::ControlContainer, RpgGameData::ControlContainerBaseData> &RpgEngine::controlContainers()
{
	return m_snapshots.controls().containers;
}

const RpgGameData::SnapshotList<RpgGameData::ControlCollection, RpgGameData::ControlCollectionBaseData> &RpgEngine::controlCollections()
{
	return m_snapshots.controls().collections;
}

const RpgGameData::SnapshotList<RpgGameData::ControlGate, RpgGameData::ControlGateBaseData> &RpgEngine::controlGates()
{
	return m_snapshots.controls().gates;
}

const RpgGameData::SnapshotList<RpgGameData::Pickable, RpgGameData::PickableBaseData> &RpgEngine::pickables()
{
	return m_snapshots.controls().pickables;
}









/**
 * @brief RpgEngine::addRelocateCollection
 * @param base
 * @param tick
 */

void RpgEngine::addRelocateCollection(const qint64 &tick, const RpgGameData::ControlCollectionBaseData &base,
									  const RpgGameData::PlayerBaseData &player, const bool &success)
{
	eventAdd<RpgEventCollectionRelocate>(success ? tick : tick+180, base, success, player);
}




/**
 * @brief RpgEngine::relocateCollection
 * @param base
 * @param ptr
 * @return
 */

int RpgEngine::relocateCollection(const RpgGameData::ControlCollectionBaseData &base, const qint64 &tick, QPointF *ptr)
{
	return d->relocateCollection(base, tick, ptr);
}


/**
 * @brief RpgEngine::finishCollection
 * @param base
 * @param idx
 * @return
 */

bool RpgEngine::finishCollection(const RpgGameData::ControlCollectionBaseData &base, const int &idx)
{
	return d->finishCollection(base, idx);
}


/**
 * @brief RpgEngine::addPickablePicked
 * @param tick
 * @param player
 */

void RpgEngine::addPickablePicked(const qint64 &tick, const RpgGameData::PickableBaseData &base,
								  const RpgGameData::PlayerBaseData &player)
{
	eventAdd<RpgEventPickablePicked>(tick, base, player);
}



/**
 * @brief RpgEngine::addContainerOpened
 * @param tick
 * @param base
 * @param player
 */

void RpgEngine::addContainerUsed(const qint64 &tick,
								 const RpgGameData::ControlContainerBaseData &base,
								 const RpgGameData::PlayerBaseData &player,
								 const bool &success)
{
	QList<RpgGameData::PickableBaseData> list;

	if (!success) {
		eventAdd<RpgEventContainerUsed>(tick, base, player, list, false);
		return;
	}

	for (const RpgGameData::InventoryItem &it : base.inv.l) {
		const RpgGameData::PickableBaseData &data = m_snapshots.pickableAdd(it.t, base.s, QPointF(base.x, base.y));

		if (!data.isValid()) {
			LOG_CERROR("engine") << "Pickable creation failed";
			continue;
		}

		list.append(data);
	}

	eventAdd<RpgEventContainerUsed>(tick, base, player, list, success);
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

		LOG_CINFO("engine") << "SET PLAYER" << ptr->id << pdata.p << ptr->s << pdata.sc << "RQ:" << ptr->rq;
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


	createEnemies(snapshot);
	createControls(snapshot);
	createCollection();
	createRandomizer();


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

	if (q->m_currentTick <= 0 || player->udpPeer()->isReconnecting()) {
		if (m.value(QStringLiteral("full")).toBool(false)) {
			LOG_CINFO("engine") << "FULLY PREPARED" << player->playerId();
			player->setIsFullyPrepared(true);
			player->udpPeer()->setIsReconnecting(false);
			return;
		}
	}


	if (player->udpPeer()->isReconnecting())
		return;

	q->m_snapshots.registerSnapshot(player, m);

	renderTimerMeausure(Received, timer2.elapsed());
}




/**
 * @brief RpgEnginePrivate::dataSend
 * @param flags
 */

void RpgEnginePrivate::dataSend(const SendMode &mode, RpgEnginePlayer *player)
{
	if (mode == SendNone)
		return;


	QCborMap baseMap;

	switch (mode) {
		case SendChrSel: {
			RpgGameData::CharacterSelectServer config;
			config.gameConfig = m_gameConfig;
			if (m_randomizer.has_value())
				config.gameConfig.randomizer = m_randomizer.value();

			for (const auto &ptr : q->m_player)
				config.players.append(ptr->config());

			baseMap = config.toCborMap();

			break;
		}

		case SendPrepare: {
			RpgGameData::Prepare config;
			config.gameConfig = m_gameConfig;
			if (m_randomizer.has_value())
				config.gameConfig.randomizer = m_randomizer.value();

			baseMap = config.toCborMap();
			QCborMap sm = q->m_snapshots.getCurrentSnapshot().toCbor();

			for (auto it = sm.cbegin(); it != sm.cend(); ++it)
				baseMap.insert(it.key(), it.value());

			break;
		}

		case SendReconnect: {
			RpgGameData::CharacterSelectServer config;
			config.gameConfig = m_gameConfig;
			if (m_randomizer.has_value())
				config.gameConfig.randomizer = m_randomizer.value();

			for (const auto &ptr : q->m_player) {
				RpgGameData::CharacterSelect c = ptr->config();
				c.lastObjectId = q->m_snapshots.lastLifeCycleId(ptr->o);
				config.players.append(c);
			}

			baseMap = config.toCborMap();

			QCborMap sm = q->m_snapshots.getCurrentSnapshot().toCbor();

			for (auto it = sm.cbegin(); it != sm.cend(); ++it)
				baseMap.insert(it.key(), it.value());

			break;
		}

		case SendNone:
			break;
	}


	if (player) {
		QCborMap map = baseMap;

		insertBaseMapData(&map, player);

		if (auto *peer = player->udpPeer())
			peer->send(map.toCborValue().toCbor(), false);

		return;
	}

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		QCborMap map = baseMap;

		insertBaseMapData(&map, it->get());

		if (auto *peer = it->get()->udpPeer())
			peer->send(map.toCborValue().toCbor(), false);
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
		UdpServerPeer *peer = it->get()->udpPeer();

		if (!peer)
			continue;

		if (peer->isReconnecting()) {
			dataSend(SendReconnect, it->get());
			continue;
		}

		if (!peer->readyToSend())
			continue;

		QCborMap map = current;

		insertBaseMapData(&map, it->get());

		peer->send(map.toCborValue().toCbor(), false);
		peer->setLastSentTick(tick);
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
			const double currMs = m_elapsedTimerReference + m_elapsedTimer.elapsed();
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
 * @brief RpgEnginePrivate::updatePeers
 */

void RpgEnginePrivate::updatePeers()
{
	bool hasPeer = false;

	for (const auto &ptr : q->m_player) {
		if (ptr->udpPeer()) {
			hasPeer = true;

			if (!q->m_hostPlayer)
				q->setHostPlayer(ptr.get());

			break;
		}
	}

	if (!hasPeer) {
		LOG_CERROR("engine") << "All peers disconnected from engine" << q->m_id;

		if (m_elapsedTimer.isValid()) {
			m_elapsedTimerReference = q->currentTick() * 1000./60.;
			LOG_CDEBUG("engine") << "Set elapsed reference:" << m_elapsedTimerReference;
			m_elapsedTimer.invalidate();
		}
	}
}



/**
 * @brief RpgEnginePrivate::reconnectPeer
 * @param peer
 * @return
 */

bool RpgEnginePrivate::reconnectPeer(UdpServerPeer *peer)
{
	if (q->m_config.gameState != RpgConfig::StatePlay)
		return false;

	peer->setIsReconnecting(true);

	for (const auto &ptr : q->m_player) {
		if (!ptr->udpPeer()) {
			LOG_CINFO("engine") << "Reconnect peer" << peer;
			ptr->setUdpPeer(peer);
			if (!q->m_hostPlayer)
				q->setHostPlayer(ptr.get());


			if (RpgEventControlUnlock *e = q->eventFind<RpgEventControlUnlock>(*ptr.get())) {
				eventRemove(e);
			}

			return true;
		}
	}

	return false;
}






/**
 * @brief RpgEnginePrivate::createEnemies
 * @param snapshot
 */

void RpgEnginePrivate::createEnemies(const RpgGameData::CurrentSnapshot &snapshot)
{
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
 * @brief RpgEnginePrivate::createControls
 * @param snapshot
 */

void RpgEnginePrivate::createControls(const RpgGameData::CurrentSnapshot &snapshot)
{
	for (const auto &ptr : snapshot.controls.lights) {
		const RpgGameData::ControlBaseData &cd = ptr.data;

		if (cd.t != RpgConfig::ControlLight) {
			LOG_CERROR("engine") << "Invalid control light data" << q->id();
			continue;
		}

		const auto &controls = q->m_snapshots.controls().lights;

		const auto it = std::find_if(controls.cbegin(), controls.cend(),
									 [&cd](const auto &e) {
			return e.data == cd;
		});

		if (it == controls.cend() && !ptr.list.empty()) {
			RpgGameData::ControlLight data = ptr.list.cbegin()->second;

			data.f = 0;

			q->m_snapshots.lightAdd(cd, data);

			LOG_CINFO("eninge") << "CREATE LIGHT" << cd.t << cd.s << cd.id << data.st;
		}
	}



	for (const auto &ptr : snapshot.controls.containers) {
		const RpgGameData::ControlContainerBaseData &cd = ptr.data;

		if (cd.t != RpgConfig::ControlContainer) {
			LOG_CERROR("engine") << "Invalid control container data" << q->id();
			continue;
		}

		const auto &controls = q->m_snapshots.controls().containers;

		const auto it = std::find_if(controls.cbegin(), controls.cend(),
									 [&cd](const auto &e) {
			return e.data == cd;
		});

		if (it == controls.cend() && !ptr.list.empty()) {
			RpgGameData::ControlContainer data = ptr.list.cbegin()->second;

			data.f = 0;

			q->m_snapshots.containerAdd(cd, data);

			LOG_CINFO("eninge") << "CREATE CONTAINER" << cd.t << cd.s << cd.id << data.st << data.lck << data.a;
		}
	}



	for (const auto &ptr : snapshot.controls.gates) {
		const RpgGameData::ControlGateBaseData &cd = ptr.data;

		if (cd.t != RpgConfig::ControlGate) {
			LOG_CERROR("engine") << "Invalid control gate data" << q->id();
			continue;
		}

		const auto &controls = q->m_snapshots.controls().gates;

		const auto it = std::find_if(controls.cbegin(), controls.cend(),
									 [&cd](const auto &e) {
			return e.data == cd;
		});

		if (it == controls.cend() && !ptr.list.empty()) {
			RpgGameData::ControlGate data = ptr.list.cbegin()->second;

			data.f = 0;

			q->m_snapshots.gateAdd(cd, data);

			LOG_CINFO("eninge") << "CREATE GATE" << cd.t << cd.s << cd.id << data.st << data.lck << data.a;
		}
	}


}



/**
 * @brief RpgEnginePrivate::createCollection
 */

void RpgEnginePrivate::createCollection()
{
	if (!q->m_snapshots.controls().collections.empty())
		return;

	const int req = QRandomGenerator::global()->bounded(5, 30);

	LOG_CINFO("engine") << "----- GENERATE" << req << "COLL";

	int generated = 0;
	const QHash<int, QList<int> > &pos = m_gameConfig.collection.allocate(req, &generated);

	for (const auto &[gid, list] : pos.asKeyValueRange()) {
		const auto &it = m_gameConfig.collection.find(gid);

		if (it == m_gameConfig.collection.groups.cend()) {
			LOG_CERROR("engine") << "Invalid GID" << gid;
			continue;
		}

		RpgGameData::ControlCollectionBaseData base;
		base.o = SERVER_OID;
		base.s = it->scene;
		base.gid = gid;


		for (const int &idx : list) {
			RpgGameData::ControlCollectionBaseData cd = base;
			cd.id = q->nextObjectId();

			RpgGameData::ControlCollection data;
			data.f = 0;
			data.sc = base.s;
			data.a = true;
			data.idx = idx;

			const auto &ptr = it->pos.at(idx);

			data.p = QList<float>{ptr.x, ptr.y};

			q->m_snapshots.collectionAdd(cd, data);

			LOG_CDEBUG("engine") << "--- create collection" << cd.id << gid << "scene" << data.sc << "IDX" << data.idx;
		}
	}


	if (q->m_player.size() == 1) {
		q->m_player.front()->rq = generated;

		LOG_CDEBUG("engine") << "   >>> SINGLE" << generated;

		return;
	}


	QList<RpgGameData::PlayerBaseData *> pList;

	pList.reserve(q->m_player.size());

	for (const auto &ptr : q->m_player) {
		pList.append(ptr.get());
	}

	RpgGameData::PlayerBaseData::assign(pList, generated);

	for (const auto &ptr : q->m_player) {
		LOG_CDEBUG("engine") << "   >>>" << ptr->o << "->" << ptr->rq;
	}
}



/**
 * @brief RpgEnginePrivate::relocateCollection
 * @param base
 * @param ptr
 */

int RpgEnginePrivate::relocateCollection(const RpgGameData::ControlCollectionBaseData &base, const qint64 &tick, QPointF *ptr)
{
	const auto &it = m_gameConfig.collection.find(base.gid);

	if (it == m_gameConfig.collection.groups.cend() || it->pos.empty()) {
		LOG_CERROR("engine") << "Missing places for group" << base.gid;
		return -1;
	}

	LOG_CINFO("engine") << "------ COLLECTION -------";

	for (const auto &g : m_gameConfig.collection.groups) {
		int idx = 0;
		for (const auto &p : g.pos) {
			LOG_CDEBUG("engine") << "   *" << g.id << idx++ << p.x << p.y << "done?" << p.done;
		}
	}

	LOG_CDEBUG("engine") << "   -----------------------------------------";

	QList<int> freeIndices;
	freeIndices.reserve(it->pos.size());

	for (int i=0; i<it->pos.size(); ++i) {
		LOG_CDEBUG("engine") << "   @" << i << it->pos.at(i).done;
		if (!it->pos.at(i).done)
			freeIndices.append(i);
	}


	LOG_CDEBUG("engine") << "INDICES" << freeIndices;

	for (const auto &ptr : q->controlCollections()) {
		if (ptr.list.empty()) {
			LOG_CERROR("engine") << "NO COLLECTION";
			continue;
		}


		if (ptr.data.gid != base.gid)
			continue;

		LOG_CDEBUG("entine") << "   -rm" << ptr.data.gid << ptr.get(tick)->idx;

		freeIndices.removeAll(ptr.get(tick)->idx);
	}

	LOG_CDEBUG("engine") << "INDICES" << freeIndices;

	if (freeIndices.isEmpty())
		return -1;

	const int idx = freeIndices.at(QRandomGenerator::global()->bounded(freeIndices.size()));

	if (ptr) {
		const auto &p = it->pos.at(idx);
		*ptr = QPointF(p.x, p.y);
	}

	return idx;
}



/**
 * @brief RpgEnginePrivate::finishCollection
 * @param base
 * @param idx
 * @return
 */

bool RpgEnginePrivate::finishCollection(const RpgGameData::ControlCollectionBaseData &base, const int &idx)
{
	if (idx < 0) {
		LOG_CERROR("engine") << "Invalid index" << idx;
		return false;
	}

	const auto &it = m_gameConfig.collection.find(base.gid);

	if (it == m_gameConfig.collection.groups.cend() || it->pos.empty()) {
		LOG_CERROR("engine") << "Missing places for group" << base.gid;
		return false;
	}

	if (idx >= it->pos.size()) {
		LOG_CERROR("engine") << "Invalid index" << idx << ">=" << it->pos.size();
		return false;
	}

	it->pos[idx].done = true;

	LOG_CINFO("engine") << "COLLECTION gid" << base.gid << "POS" << idx << "done";

	return true;
}




/**
 * @brief RpgEnginePrivate::createRandomizer
 */

void RpgEnginePrivate::createRandomizer()
{
	if (m_randomizer.has_value())
		return;

	LOG_CINFO("engine") << "Randomize";

	m_randomizer = m_gameConfig.randomizer;

	m_randomizer->randomize();

	for (const RpgGameData::RandomizerGroup &g : m_randomizer->groups) {
		LOG_CDEBUG("engine") << "---" << g.gid << "-" << g.current;
	}
}












/**
 * @brief RpgEvent::RpgEvent
 * @param m_tick
 */

RpgEventBase::RpgEventBase(RpgEngine *engine, const qint64 &tick, const bool &unique)
	: m_engine(engine)
	, m_tick(tick)
	, m_unique(unique)
{

}



/**
 * @brief RpgEvent::~RpgEvent
 */

RpgEventBase::~RpgEventBase()
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
 * @brief RpgEnginePrivate::eventFind
 * @param baseData
 * @return
 */

template<typename T, typename T2, typename T3>
T *RpgEnginePrivate::eventFind(const T2 &baseData)
{
	for (const auto &ptr : m_events) {
		if (T* e = dynamic_cast<T*>(ptr.get()); e && e->baseData() == baseData)
			return e;
	}

	return nullptr;
}




/**
 * @brief RpgEnginePrivate::eventRemove
 * @param event
 */

void RpgEnginePrivate::eventRemove(RpgEventBase *event)
{
	m_eventsRemove.append(event);
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



template<typename T, typename T2, typename T3>
T *RpgEngine::eventFind(const T2 &data)
{
	return d->eventFind<T>(data);
}


void RpgEngine::eventRemove(RpgEventBase *event)
{
	d->eventRemove(event);
}



/**
 * @brief RpgEnginePrivate::processEvents
 * @return
 */

RpgGameData::CurrentSnapshot RpgEnginePrivate::processEvents(const qint64 &tick)
{
	m_eventsProcessing = true;

	// Generate auth snaps for this snapshot -> saveRenderer overrides main storage

	RpgGameData::CurrentSnapshot snapshot;

	for (auto it = m_events.begin(); it != m_events.end(); ) {
		if (m_eventsRemove.contains(it->get())) {
			LOG_CWARNING("engine") << "REMOVE EVENT" << it->get();
			m_eventsRemove.removeAll(it->get());
			it = m_events.erase(it);
			continue;
		}

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
 * @brief RpgEventEnemyDied::process
 * @return
 */

bool RpgEventEnemyDied::process(const qint64 &/*tick*/, RpgGameData::CurrentSnapshot *dst)
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

		if (ptr.get(m_tick)->hp > 0) {
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

		RpgGameData::Enemy edata = ptr.get(m_tick).value();
		edata.f = m_tick;
		edata.hp = 32;

		LOG_CINFO("engine") << "###### RESURRECT" << m_tick << ptr.data.id;

		dst->assign(dst->enemies, ptr.data, edata);
	}

	return true;
}





/**
 * @brief RpgEventPlayerDied::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPlayerDied::process(const qint64 &/*tick*/, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	LOG_CDEBUG("engine") << "Player died processed" << m_data.o << m_data.id << "@" << m_tick;

	m_engine->eventAdd<RpgEventPlayerResurrect>(m_tick+300, m_data);

	return true;
}


/**
 * @brief RpgEventPlayerResurrect::RpgEventPlayerResurrect
 * @param engine
 * @param tick
 */




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
		player = it->get(m_tick).value();

	player.f = m_tick;
	player.hp = 45;

	LOG_CINFO("engine") << "###### RESURRECT PLAYER" << m_tick << m_data.id;

	dst->assign(dst->players, m_data, player);

	return true;
}



/**
 * @brief RpgEventCollectionRelocate::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventCollectionRelocate::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->controlCollections();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		LOG_CERROR("engine") << "Invalid collection" << m_data.o << m_data.s << m_data.id;
		return true;
	}

	RpgGameData::ControlCollection d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	d.f = m_tick;
	d.u = {};


	if (m_success) {
		if (!m_engine->finishCollection(it->data, d.idx)) {
			LOG_CERROR("engine") << "Finish colection failed" << it->data.o << it->data.s << it->data.id;
		}

		d.a = false;
		d.own = m_player;

		LOG_CINFO("engine") << "###### FINISH COLLECTION" << m_tick << m_data.id << "--->" << m_player.o;

	} else {
		const auto &pl = m_engine->players();

		const auto pit = std::find_if(pl.cbegin(),
									 pl.cend(),
									 [this](const auto &p){
			return p.data.isBaseEqual(m_player);
		});

		if (pit == pl.cend() || pit->list.empty()) {
			LOG_CERROR("engine") << "Invalid player" << m_data.o << m_data.s << m_data.id;
		} else {
			RpgGameData::Player playerData = pit->get(m_tick).value();
			playerData.controlFailed(RpgConfig::ControlCollection);

			dst->assign(dst->players, m_player, playerData);
		}


		d.a = true;
		d.own = {};

		QPointF p;
		const int idx = m_engine->relocateCollection(it->data, m_tick, &p);

		LOG_CINFO("engine") << "###### UNLOCK COLLECTION" << m_tick << m_data.id << "--->" << idx << p;

		if (idx >= 0) {
			d.idx = idx;
			d.p = QList<float>{(float) p.x(), (float) p.y()};
		}
	}

	dst->assign(dst->controls.collections, m_data, d);

	return true;
}






/**
 * @brief RpgEventControlUnlock::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventControlUnlock::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;


	const auto &pp = m_engine->players();

	if (const auto &it = std::find_if(pp.cbegin(),
									  pp.cend(),
									  [this](const auto &pd){
									  return pd.data == m_data;
}); it != pp.cend() && !it->list.empty()) {
		RpgGameData::Player d = it->get(m_tick).value();

		d.l = false;

		LOG_CINFO("engine") << "###### UNLOCK PLAYER" << m_tick << d.f << it->data.o;
		dst->assign(dst->players, m_data, d);
	}

	const auto &pl = m_engine->controlContainers();



	for (const auto &ptr : pl) {
		if (ptr.list.empty())
			continue;

		RpgGameData::ControlContainer d = ptr.get(m_tick).value();

		if (d.u.isBaseEqual(m_data) && d.st == RpgGameData::ControlContainer::ContainerClose) {
			d.u = {};
			d.a = true;
			LOG_CINFO("engine") << "###### UNLOCK CONTAINER" << m_tick << d.f << ptr.data.id;
			dst->assign(dst->controls.containers, ptr.data, d);
		}
	}



	const auto &pc = m_engine->controlCollections();

	for (const auto &ptr : pc) {
		if (ptr.list.empty())
			continue;

		RpgGameData::ControlCollection d = ptr.get(m_tick).value();

		if (d.u.isBaseEqual(m_data) && !d.own.isValid()) {
			d.u = {};
			d.a = true;
			LOG_CINFO("engine") << "###### UNLOCK COLLECTION" << m_tick << d.f << ptr.data.id;
			dst->assign(dst->controls.collections, ptr.data, d);
		}
	}


	return true;
}



/**
 * @brief RpgEventPickablePicked::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventPickablePicked::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->pickables();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		LOG_CERROR("engine") << "Invalid pickable" << m_data.o << m_data.s << m_data.id;
		return true;
	}


	RpgGameData::Player pData;

	const auto &pp = m_engine->players();

	if (const auto &it = std::find_if(pp.cbegin(),
									  pp.cend(),
									  [this](const auto &pd){
									  return pd.data == m_player;
}); it != pp.cend() && !it->list.empty()) {
		pData = it->get(m_tick).value();
	} else {
		LOG_CERROR("engine") << "Invalid player" << m_player.o << m_player.s << m_player.id;
		return true;
	}


	RpgGameData::Pickable d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	d.f = m_tick;
	d.u = {};
	d.a = false;
	d.own = m_player;
	d.st = RpgGameData::LifeCycle::StageDead;

	dst->assign(dst->controls.pickables, m_data, d);



	pData.f = m_tick;

	if (pData.pick(m_data.pt)) {
		LOG_CINFO("engine") << "###### FINISH PICKABLE" << m_tick << m_data.id << "--->" << m_player.o;
		LOG_CINFO("engine") << "!!!!! ADD HP" << pData.f << pData.hp;
		dst->assign(dst->players, m_player, pData);
	} else {
		LOG_CERROR("engine") << "###### PICKABLE PROCESS FAILED" << m_data.pt << m_tick << "--->" << m_player.o;
	}

	return true;
}





/**
 * @brief RpgEventContainerOpened::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventContainerUsed::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	if (tick < m_tick)
		return false;

	LOG_CDEBUG("engine") << "Container used processed" << m_data.o << m_data.id << "@" << m_tick << m_success;

	if (!m_success) {
		const auto &pl = m_engine->players();

		const auto it = std::find_if(pl.cbegin(),
									 pl.cend(),
									 [this](const auto &p){
			return p.data.isBaseEqual(m_player);
		});

		if (it == pl.cend() || it->list.empty()) {
			LOG_CERROR("engine") << "Invalid player" << m_data.o << m_data.s << m_data.id;
			return true;
		}

		RpgGameData::Player playerData = it->get(m_tick).value();
		playerData.controlFailed(RpgConfig::ControlContainer);

		dst->assign(dst->players, m_player, playerData);


	} else {
		for (const RpgGameData::PickableBaseData &base : m_pickables) {
			const auto &pl = m_engine->pickables();

			const auto it = std::find_if(pl.cbegin(),
										 pl.cend(),
										 [&base](const auto &p){
				return p.data == base;
			});

			if (it == pl.cend()) {
				LOG_CERROR("engine") << "Invalid pickable" << m_data.o << m_data.s << m_data.id;
				continue;
			}

			RpgGameData::Pickable d;

			if (!it->list.empty())
				d = it->get(m_tick).value();

			d.f = m_tick;
			d.u = {};
			d.a = true;
			d.own = {};
			d.st = RpgGameData::LifeCycle::StageLive;

			dst->assign(dst->controls.pickables, base, d);
		}
	}

	return true;
}

