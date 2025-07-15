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
#include "FileAppender.h"
#include "Logger.h"
#include "serverservice.h"
#include "udpserver.h"
#include "userapi.h"
#include <QCborArray>
#include <QCborMap>




/**
 * @brief The RpgEngineMessage class
 */

class RpgEngineMessage
{
public:
	RpgEngineMessage(const RpgGameData::Message &msg, const QList<int> &players = {}, const bool &inverse = false)
		: m_message(msg)
		, m_players(players)
		, m_inverse(inverse)
	{}

	const RpgGameData::Message &message() const { return m_message; }
	const QList<int> &players() const { return m_players; }
	const bool &inverse() const { return m_inverse; }

	const QSet<int> &sent() const { return m_sent; }
	void addSent(const int &id) { m_sent.insert(id); }

	bool canSend(const int &id) const {
		if (m_sent.contains(id))
			return false;

		const bool &ct = m_players.contains(id);

		return (m_players.isEmpty() ||
				(!m_inverse && ct) ||
				(m_inverse && !ct))
				;

	}

private:
	RpgGameData::Message m_message;
	QList<int> m_players;
	bool m_inverse;
	QSet<int> m_sent;
};







/**
 * @brief The RpgEnginePrivate class
 */

class RpgEnginePrivate
{
private:
	RpgEnginePrivate(RpgEngine *engine)
		: m_removeTimer(-1)
		, q(engine)
		, m_logger(new Logger(QStringLiteral("engineprivate"), false))
	{}


	static void sendEngineList(const RpgConfigBase &config, UdpServerPeer *peer, EngineHandler *handler);

	static bool canConnect(const qint64 &peerID, const RpgConfigBase &config, RpgEngine *engine);

	RpgEnginePlayer* getPlayer(UdpServerPeer *peer) const;
	RpgEnginePlayer* getPlayer(const int &playerId) const;

	void dataReceived(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff);
	void dataReceivedChrSel(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPrepare(RpgEnginePlayer *player, const QByteArray &data);
	void dataReceivedPlay(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff);

	enum SendMode {
		SendNone = 0,
		SendChrSel,
		SendPrepare,
		SendReconnect
	};

	void dataSend(const SendMode &mode, RpgEnginePlayer *player = nullptr);
	void dataSendPlay();
	void dataSendFinished();

	void insertBaseMapData(QCborMap *dst, RpgEnginePlayer *player);
	bool insertMessages(QCborMap *dst, RpgEnginePlayer *player);
	void clearMessages();

	void updateState();

	void updatePeers();
	bool reconnectPeer(UdpServerPeer *peer);
	bool banOutPlayer(RpgEnginePlayer *player);



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

	UserAPI::UserGame toUserGame() const;
	bool gameCreate(RpgEnginePlayer *player);
	bool gameFinish();


	RpgGameData::GameConfig m_gameConfig;
	std::optional<RpgGameData::Randomizer> m_randomizer;

	QElapsedTimer m_elapsedTimer;
	qint64 m_elapsedTimerReference = 0;
	qint64 m_lastSentTick = -1;
	qint64 m_lastReliable = -1;
	int m_lastMyObjectId = 0;

	float m_avgCollectionMsec = 0;
	int m_collectionGenerated = -1;

	bool m_playerDataModified = false;

	std::vector<std::unique_ptr<RpgEventBase>> m_events;
	std::vector<std::unique_ptr<RpgEventBase>> m_eventsLater;
	QList<RpgEventBase*> m_eventsRemove;
	bool m_eventsProcessing = false;

	QDeadlineTimer m_removeTimer;

	bool m_locked = false;
	QList<qint64> m_banList;


	QList<RpgEngineMessage> m_messages;

	RpgEngine *q;

	std::unique_ptr<Logger> m_logger;
	Logger *_logger() const { return m_logger.get(); }



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

RpgEngine::RpgEngine(EngineHandler *handler, const RpgConfigBase &config, QObject *parent)
	: UdpEngine(EngineRpg, handler, parent)
	, d(new RpgEnginePrivate(this))
	, m_config(config)
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

std::shared_ptr<RpgEngine> RpgEngine::engineCreate(EngineHandler *handler, const RpgConfigBase &config, UdpServer *server)
{
	if (!handler)
		return {};

	LOG_CDEBUG("engine") << "Create RpgEngine" << m_nextId << server;


	auto ptr = std::make_shared<RpgEngine>(handler, config);
	ptr->setId(m_nextId);
	increaseNextId();

	ptr->setUdpServer(server);

	ptr->setPlayerLimit(5);

	if (const QString &dir = handler->service()->logDir(); !dir.isEmpty()) {
		const QString fname = dir+QStringLiteral("/rpg-%1.log").arg(ptr->id(), 3, 10, '0');

		if (QFile::exists(fname))
			QFile::remove(fname);

		ptr->setLoggerFile(fname);
	}

	handler->engineAdd(ptr);

	return ptr;
}



/**
 * @brief RpgEngine::engineDispatch
 * @param handler
 * @param data
 * @param server
 * @return
 */

std::shared_ptr<RpgEngine> RpgEngine::engineDispatch(EngineHandler *handler, const QJsonObject &connectionToken,
													 const QByteArray &data, UdpServerPeer *peer)
{
	Q_ASSERT(handler);
	Q_ASSERT(peer);
	Q_ASSERT(peer->server());

	RpgGameData::EngineSelector selector;
	selector.fromCbor(QCborValue::fromCbor(data));

	RpgGameData::ConnectionToken cToken;
	cToken.fromJson(connectionToken);

	if (selector.operation == RpgGameData::EngineSelector::Invalid) {
		LOG_CDEBUG("engine") << "Invalid operation" << peer->peerID();

		peer->send(RpgGameData::EngineSelector(RpgGameData::EngineSelector::Reset).toCborMap().toCborValue().toCbor(), true);
		return {};
	}

	if (selector.operation == RpgGameData::EngineSelector::List) {
		RpgEnginePrivate::sendEngineList(cToken.config, peer, handler);
		return {};
	}

	if (selector.operation == RpgGameData::EngineSelector::Reset) {
		LOG_CWARNING("engine") << "Invalid RESET operation" << peer->peerID();
		return {};
	}



	// Create

	if (selector.operation == RpgGameData::EngineSelector::Create) {
		std::shared_ptr<RpgEngine> engine = engineCreate(handler, cToken.config, peer->server());

		LOG_CINFO("engine") << "Create engine" << peer->peerID() << "id:" << engine->id();

		peer->server()->peerConnectToEngine(peer, engine);

		return engine;
	}



	// Connect

	const auto &list = handler->engines();

	const auto it = std::find_if(list.constBegin(),
								 list.constEnd(),
								 [peer](const std::shared_ptr<AbstractEngine> &ptr){
		if (!ptr || ptr->type() != EngineRpg)
			return false;

		return std::dynamic_pointer_cast<RpgEngine>(ptr)->player(peer->peerID()) != nullptr;

	});

	if (it != list.constEnd()) {
		std::shared_ptr<RpgEngine> engine = std::dynamic_pointer_cast<RpgEngine>(*it);

		if (!engine) {
			LOG_CERROR("engine") << "Engine cast error";
			return {};
		}

		if (engine->config() == cToken.config) {
			if (selector.engine <= 0 || selector.engine == engine->id()) {
				peer->server()->peerConnectToEngine(peer, engine);
				return engine;
			} else {
				LOG_CWARNING("engine") << "Engine config mismatch" << peer->peerID() << selector.engine << "vs." << engine->id();
			}
		}
	}


	// Direct connect

	const auto eit = std::find_if(list.constBegin(),
								  list.constEnd(),
								  [&cToken, &selector, peerId = peer->peerID()](const std::shared_ptr<AbstractEngine> &ptr){
		if (!ptr || ptr->type() != AbstractEngine::EngineRpg)
			return false;

		if (!RpgEnginePrivate::canConnect(peerId, cToken.config, std::dynamic_pointer_cast<RpgEngine>(ptr).get()))
			return false;

		return ptr->id() == selector.engine;
	});


	if (eit == list.constEnd()) {
		LOG_CWARNING("engine") << "Invalid engine" << peer->peerID() << selector.engine;
		return {};
	}


	std::shared_ptr<RpgEngine> engine = std::dynamic_pointer_cast<RpgEngine>(*eit);

	peer->server()->peerConnectToEngine(peer, engine);
	return engine;
}



/**
 * @brief RpgEngine::canDelete
 * @param useCount
 * @return
 */

bool RpgEngine::canDelete(const int &useCount)
{
	if (!d->m_removeTimer.isForever()) {
		return d->m_removeTimer.hasExpired();
	} else if (m_config.gameState == RpgConfig::StatePlay) {
		// Ha van még, aki nincs kész, akkor nem zárjuk le

		const auto it = std::find_if(m_player.cbegin(),
									 m_player.cend(),
									 [](const auto &ptr) {
			return !ptr->config().finished;
		});

		const bool hasNoFinished = (it != m_player.cend());

		return !hasNoFinished;

	} else {
		return AbstractEngine::canDelete(useCount);
	}
}



/**
 * @brief RpgEngine::binaryDataReceived
 * @param data
 */

void RpgEngine::binaryDataReceived(const UdpServerPeerReceivedList &data)
{
	for (const auto &pair : data)
		binaryDataReceived(pair);

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
	else if (m_config.gameState == RpgConfig::StateFinished)
		d->dataSendFinished();

	d->renderTimerMeausure(RpgEnginePrivate::TimerTick, t2.elapsed());
}



/**
 * @brief RpgEngine::binaryDataReceived
 * @param peer
 * @param data
 */

void RpgEngine::binaryDataReceived(const UdpServerPeerReceived &recv)
{
	Q_ASSERT(recv.peer);

	RpgEnginePlayer *player = d->getPlayer(recv.peer);

	if (!player) {
		ELOG_ERROR << "Player not found" << recv.peer;
		return;
	}

	d->dataReceived(player, recv.data, recv.diff);
}


/**
 * @brief RpgEngine::udpPeerAdd
 * @param peer
 */

void RpgEngine::udpPeerAdd(UdpServerPeer *peer)
{
	if (!peer)
		return;

	if (d->m_banList.contains(peer->peerID())) {
		ELOG_DEBUG << "Peer already banned" << peer->peerID();
		disconnectUnusedPeer(peer);
		return;
	}

	if (d->reconnectPeer(peer))
		return;

	if (m_config.gameState >= RpgConfig::StatePrepare) {
		ELOG_ERROR << "Game has already begun";
		return;
	}

	bool isHost = m_player.empty();			// TODO

	const auto &ptr = m_player.emplace_back(std::make_unique<RpgEnginePlayer>(peer, false));
	ptr->setPlayerId(m_nextPlayerId++);
	ptr->setPeerID(peer->peerID());
	ptr->id = 1;		// Minden player azonosítója 1
	ptr->m_config.nickname = QStringLiteral("PLAYER #%1").arg(ptr->playerId());

	if (isHost)
		setHostPlayer(ptr.get());

	ELOG_DEBUG << "Add player" << ptr->o << peer << isHost << ptr->isHost();

	d->updatePeers();
}



/**
 * @brief RpgEngine::udpPeerRemove
 * @param peer
 */

void RpgEngine::udpPeerRemove(UdpServerPeer *peer)
{
	RpgEnginePlayer *player = d->getPlayer(peer);

	ELOG_DEBUG << "Remove player" << peer << player;

	if (player && d->m_banList.contains(player->peerID())) {
		ELOG_DEBUG << "Erase banned player" << player->peerID();
		std::erase_if(m_player, [player](const auto &ptr) { return ptr.get() == player; });

	} else if (m_config.gameState == RpgConfig::StatePlay) {
		if (player) {
			ELOG_DEBUG << "Remove UDP PEER" << peer << "from player" << player;
			player->setUdpPeer(nullptr);
			eventAdd<RpgEventControlUnlock>(m_currentTick+600, *player);
		}
	} else {
		std::erase_if(m_player, [peer](const auto &ptr) { return ptr->udpPeer() == peer; });
	}

	if (player && m_hostPlayer == player) {
		ELOG_WARNING << "No host";
		setHostPlayer(nullptr);
	}

	d->updatePeers();
}



/**
 * @brief RpgEngine::disconnectUnusedPeer
 * @param peer
 */

void RpgEngine::disconnectUnusedPeer(UdpServerPeer *peer)
{
	if (m_config.gameState == RpgConfig::StateFinished && !d->m_removeTimer.isForever() && d->m_removeTimer.hasExpired()) {
		ELOG_INFO << "Disconnect peer" << peer;

		if (peer->peer()) {
			enet_peer_disconnect_later(peer->peer(), 0);
		} else {
			LOG_CERROR("engine") << "Missing ENetPeer" << peer;
		}
	}
}



/**
 * @brief RpgEngine::dumpEngine
 * @return
 */

QString RpgEngine::dumpEngine() const
{
	return d->engineDump();
}






/**
 * @brief RpgEngine::player
 * @param base
 * @return
 */

RpgEnginePlayer *RpgEngine::player(const RpgGameData::PlayerBaseData &base) const
{
	auto it = std::find_if(m_player.begin(), m_player.end(), [&base](const auto &ptr) { return ptr->isBaseEqual(base); });

	if (it == m_player.end())
		return nullptr;
	else
		return it->get();
}


/**
 * @brief RpgEngine::player
 * @param peerID
 * @return
 */

RpgEnginePlayer *RpgEngine::player(const quint32 &peerID) const
{
	auto it = std::find_if(m_player.begin(), m_player.end(), [&peerID](const auto &ptr) { return ptr->peerID() == peerID; });

	if (it == m_player.end())
		return nullptr;
	else
		return it->get();
}


/**
 * @brief RpgEngine::playerSetGameCompleted
 * @param base
 * @return
 */

RpgEnginePlayer *RpgEngine::playerSetGameCompleted(const RpgGameData::PlayerBaseData &base)
{
	RpgEnginePlayer *p = player(base);

	if (!p) {
		LOG_CERROR("engine") << "Invalid player" << base.o;
		return nullptr;
	}

	ELOG_DEBUG << "Engine" << m_id << "player" << base.o << "game completed";

	p->m_config.finished = true;

	d->m_playerDataModified = true;

	return p;
}



/**
 * @brief RpgEngine::playerAddXp
 * @param base
 * @param xp
 * @return
 */

RpgEnginePlayer *RpgEngine::playerAddXp(const RpgGameData::PlayerBaseData &base, const int &xp)
{
	RpgEnginePlayer *p = player(base);

	if (!p) {
		LOG_CERROR("engine") << "Invalid player" << base.o;
		return nullptr;
	}

	ELOG_DEBUG << "Engine" << m_id << "player" << base.o << "add" << xp << "XP";

	p->m_config.xp += xp;

	d->m_playerDataModified = true;

	return p;
}



/**
 * @brief RpgEngine::getPlayerData
 * @param forced
 * @return
 */

QCborArray RpgEngine::getPlayerData(const bool &forced)
{
	QCborArray list;

	if (!d->m_playerDataModified && !forced)
		return list;

	if (m_player.empty())
		return list;

	for (const auto &ptr : m_player)
		list.append(ptr->config().toCborMap());

	d->m_playerDataModified = false;

	return list;
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
		ELOG_INFO << "New host player:" << newHostPlayer->o;
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
 * @brief RpgEngine::messageAdd
 * @param msg
 * @param players
 * @param inverse
 */

void RpgEngine::messageAdd(const RpgGameData::Message &msg, const QList<int> &players, const bool &inverse)
{
	d->m_messages.emplace_back(msg, players, inverse);
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
		ELOG_INFO << "Enemy died" << data.o << data.id;
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
		ELOG_INFO << "Player died" << data.o << data.id;
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

const RpgGameData::SnapshotList<RpgGameData::ControlTeleport, RpgGameData::ControlTeleportBaseData> &RpgEngine::controlTeleports()
{
	return m_snapshots.controls().teleports;
}









/**
 * @brief RpgEngine::addRelocateCollection
 * @param base
 * @param tick
 */

void RpgEngine::addRelocateCollection(const qint64 &tick, const RpgGameData::ControlCollectionBaseData &base,
									  const RpgGameData::PlayerBaseData &player, const bool &success)
{
	eventAdd<RpgEventCollectionUsed>(tick, base, success, player);
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
			ELOG_ERROR << "Pickable creation failed";
			continue;
		}

		list.append(data);
	}

	eventAdd<RpgEventContainerUsed>(tick, base, player, list, success);
}


/**
 * @brief RpgEngine::addTeleportUsed
 * @param tick
 * @param base
 * @param player
 */

void RpgEngine::addTeleportUsed(const qint64 &tick, const RpgGameData::ControlTeleportBaseData &base, const RpgGameData::PlayerBaseData &player)
{
	eventAdd<RpgEventTeleportUsed>(tick, base, player);
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
 * @brief RpgEngine::checkPlayersCompleted
 */

void RpgEngine::checkPlayersCompleted()
{
	if (m_config.gameState != RpgConfig::StatePlay)
		return;

	bool completed = true;

	if (m_deadlineTick > -1 && m_currentTick >= m_deadlineTick) {
		ELOG_INFO << "Game timeout";
	} else {
		for (const auto &pl : m_player) {
			if (!pl->config().finished) {
				completed = false;
				break;
			}
		}
	}

	if (!completed)
		return;

	ELOG_INFO << "All players completed!!!";

	m_config.gameState = RpgConfig::StateFinished;
	d->m_removeTimer.setRemainingTime(5000);

	d->gameFinish();
}


/**
 * @brief RpgEngine::_logger
 * @return
 */

Logger *RpgEngine::_logger() const
{
	return d->m_logger.get();
}



/**
 * @brief RpgEngine::setLoggerFile
 * @param fname
 */

void RpgEngine::setLoggerFile(const QString &fname)
{
	FileAppender* appender = new FileAppender(fname);

	appender->setFormat(QString::fromStdString( "%{time}{hh:mm:ss.zzz} [%{TypeOne}] %{message}\n"));

#ifndef QT_NO_DEBUG
	//appender->setFormat(QString::fromStdString( "%{time}{yyyy-MM-dd hh:mm:ss.zzz} [%{TypeOne}] %{message} <%{function} %{file}:%{line}>\n"));
	appender->setDetailsLevel(Logger::Trace);
#else
	//appender->setFormat(QString::fromStdString( "%{time}{hh:mm:ss.zzz} [%{TypeOne}] %{message}\n"));
	appender->setDetailsLevel(Logger::Debug);
#endif

	d->m_logger->registerAppender(appender);
}









/**
 * @brief RpgEngine::preparePlayers
 */

void RpgEngine::preparePlayers()
{
	if (d->m_gameConfig.duration > 0 &&
			m_deadlineTick <= 0) {

		qint64 ms = d->m_gameConfig.duration * 1000.;			// duration to msec

		ms = 300*1000;

		if (d->m_avgCollectionMsec > 0 && d->m_collectionGenerated > 0) {
			ms += (d->m_avgCollectionMsec * d->m_collectionGenerated * 2.0) / (m_player.size() > 0 ? m_player.size() : 1);
		}

		m_deadlineTick = ms * 60./1000.;						// msec to frame
	}

	if (d->m_gameConfig.positionList.isEmpty())
		return;

	int idx = 0;

	for (auto &ptr : m_player) {
		if (ptr->m_gameId < 0)
			d->gameCreate(ptr.get());

		if (ptr->s != -1)
			continue;

		ELOG_DEBUG << "Prepare player" << (idx+1);


		RpgGameData::PlayerPosition pos;

		if (idx >= d->m_gameConfig.positionList.size()) {
			ELOG_ERROR << "Missing player positions" << id();
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


		ptr->setStartPosition(pos);

		const RpgGameData::CharacterSelect &config = ptr->config();

		ptr->mmp = config.maxMp;

		RpgGameData::Player pdata;
		pdata.p = {pos.x, pos.y};
		pdata.f = 0;
		pdata.sc = pos.scene;
		pdata.hp = std::max(config.maxHp, 13);			// TODO: from m_config
		pdata.mhp = std::max(config.maxHp, 20);
		pdata.mp = config.mp;

		if (!config.armory.wl.isEmpty()) {
			pdata.arm = config.armory;

			LOG_CINFO("engine") << "SET ARMORY" << pdata.arm.cw << pdata.arm.s << pdata.arm.wl.size();

			for (auto w : pdata.arm.wl) {
				LOG_CDEBUG("engine") << "  -" << w.t << w.s;
			}
		} else {
			pdata.arm.add(RpgGameData::Weapon::WeaponLongsword, 0, 135);
			pdata.arm.cw = RpgGameData::Weapon::WeaponShortbow;
			pdata.arm.s = 0;

			LOG_CINFO("engine") << "SET AUTO ARMORY" << pdata.arm.cw << pdata.arm.s << pdata.arm.wl.size();
		}

		ELOG_INFO << "SET ARMORY" << pdata.arm.cw << pdata.arm.s << "size" <<  pdata.arm.wl.size() << *ptr;

		m_snapshots.playerAdd(*ptr, pdata);
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

void RpgEnginePrivate::sendEngineList(const RpgConfigBase &config, UdpServerPeer *peer, EngineHandler *handler)
{
	Q_ASSERT(handler);
	Q_ASSERT(peer);
	Q_ASSERT(peer->server());

	RpgGameData::EngineSelector selector(RpgGameData::EngineSelector::List);

	for (const auto &ptr : handler->engines()) {
		if (!ptr || ptr->type() != AbstractEngine::EngineRpg)
			continue;

		const auto &e = std::dynamic_pointer_cast<RpgEngine>(ptr);

		if (!canConnect(peer->peerID(), config, e.get()))
			continue;

		RpgGameData::Engine engine;
		engine.id = e->id();
		engine.count = e->playerLimit();

		RpgEnginePlayer *host = e->hostPlayer();

		if (host) {
			engine.owner.username = host->config().username;
			engine.owner.nickname = host->config().nickname;
		}

		for (const auto &p : e->playerList()) {
			if (!p.get())
				continue;

			if (host && p.get() == host)
				continue;

			engine.players.emplaceBack(p->config().username, p->config().nickname);
		}

		selector.engines.append(engine);
	}

	const int max = std::max(1, handler->service()->settings()->udpMaxEngines());

	selector.add = (handler->engines().size() < max);

	peer->send(selector.toCborMap().toCborValue().toCbor(), false);
}



/**
 * @brief RpgEnginePrivate::canConnect
 * @param config
 * @param engine
 * @return
 */


bool RpgEnginePrivate::canConnect(const qint64 &peerID, const RpgConfigBase &config, RpgEngine *engine)
{
	return (engine &&
			(engine->config().gameState == RpgConfig::StateConnect ||
			 engine->config().gameState == RpgConfig::StateCharacterSelect) &&
			engine->config() == config &&
			!engine->d->m_locked &&
			!engine->d->m_banList.contains(peerID) &&
			(engine->m_playerLimit <= 0 || engine->m_player.size() < engine->m_playerLimit)
			);
}






RpgEnginePlayer *RpgEnginePrivate::getPlayer(UdpServerPeer *peer) const
{
	auto it = std::find_if(q->m_player.begin(), q->m_player.end(), [peer](const auto &ptr) { return ptr->udpPeer() == peer; });

	if (it == q->m_player.end())
		return nullptr;
	else
		return it->get();
}



/**
 * @brief RpgEnginePrivate::getPlayer
 * @param playerId
 * @return
 */

RpgEnginePlayer *RpgEnginePrivate::getPlayer(const int &playerId) const
{
	auto it = std::find_if(q->m_player.begin(), q->m_player.end(), [playerId](const auto &ptr) { return ptr->playerId() == playerId; });

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

void RpgEnginePrivate::dataReceived(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff)
{
	Q_ASSERT(player);

	if (q->m_config.gameState == RpgConfig::StateCharacterSelect)
		dataReceivedChrSel(player, data);
	else if (q->m_config.gameState == RpgConfig::StatePrepare)
		dataReceivedPrepare(player, data);
	else if (q->m_config.gameState == RpgConfig::StatePlay)
		dataReceivedPlay(player, data, diff);


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

	RpgGameData::EngineSelector selector;
	selector.fromCbor(m);

	if (selector.operation != RpgGameData::EngineSelector::Invalid)
		return;

	RpgGameData::CharacterSelect c;
	c.fromCbor(m);


	c.playerId = player->playerId();			// Ő nem változtathat rajta
	player->setConfig(c);

	if (!player->isHost())
		return;

	m_gameConfig = c.gameConfig;

	if (c.locked) {
		ELOG_INFO << "Lock engine by peer" << player->peerID();
		m_locked = true;
	}

	if (m.contains(QStringLiteral("ban"))) {
		const int playerId = m.value(QStringLiteral("ban")).toInteger();

		RpgEnginePlayer *p = getPlayer(playerId);

		if (p) {
			ELOG_INFO << "Ban" << playerId << "peer:" << p->peerID();

			banOutPlayer(p);
		} else {
			ELOG_ERROR << "Ban" << playerId << "failed";
		}
	}

	if (m.value(QStringLiteral("lock")).toBool()) {
		m_locked = true;
	}

	/*#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"), QString::fromUtf8(QJsonDocument(m.toJsonObject()).toJson()));
	q->service()->writeToSocket(map.toCborValue());
#endif*/
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

	RpgGameData::EngineSelector selector;
	selector.fromCbor(m);

	if (selector.operation != RpgGameData::EngineSelector::Invalid) {
		ELOG_TRACE << "Skip engine selector for player" << player->peerID();
		return;
	}

	RpgGameData::Prepare config;

	config.fromCbor(m);

	if (!player->isPrepared()) {
		player->setIsPrepared(config.prepared);
	}

	if (!player->isHost() || !config.loaded)
		return;

	m_gameConfig = config.gameConfig;
	m_avgCollectionMsec = config.avg;

	RpgGameData::CurrentSnapshot snapshot;
	snapshot.fromCbor(m);

	/*#ifdef WITH_FTXUI
	QCborMap map;
	map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));
	map.insert(QStringLiteral("txt"),
			   QString::fromUtf8(QJsonDocument(m.toJsonObject()).toJson()) +
			   QStringLiteral("-------------------------------------------------\n") +
			   QString::fromUtf8(QJsonDocument(snapshot.toCbor().toJsonObject()).toJson())
			   );
	q->service()->writeToSocket(map.toCborValue());
#endif
*/

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

void RpgEnginePrivate::dataReceivedPlay(RpgEnginePlayer *player, const QByteArray &data, const qint64 &diff)
{
	Q_ASSERT(player);

	QElapsedTimer timer2;
	timer2.start();

	QCborMap m = QCborValue::fromCbor(data).toMap();

	RpgGameData::EngineSelector selector;
	selector.fromCbor(m);

	if (selector.operation != RpgGameData::EngineSelector::Invalid) {
		ELOG_TRACE << "Skip engine selector for player" << player->peerID();
		return;
	}

	if (q->m_currentTick <= 0 || player->udpPeer()->isReconnecting()) {
		if (m.value(QStringLiteral("full")).toBool(false)) {
			player->setIsFullyPrepared(true);
			player->udpPeer()->setIsReconnecting(false);
			return;
		}
	}


	if (player->udpPeer()->isReconnecting())
		return;

	q->m_snapshots.registerSnapshot(player, m, diff);

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
			config.locked = m_locked;
			config.max = q->m_playerLimit;
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

			for (const auto &ptr : q->m_player)
				config.players.append(ptr->config());

			baseMap = config.toCborMap();
			QCborMap sm = q->m_snapshots.getCurrentSnapshot().toCbor();

			for (auto it = sm.cbegin(); it != sm.cend(); ++it)
				baseMap.insert(it.key(), it.value());

			break;
		}

		case SendReconnect: {
			RpgGameData::CharacterSelectServer config;
			config.gameConfig = m_gameConfig;
			config.locked = m_locked;
			config.max = q->m_playerLimit;

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

	bool reliable = tick > m_lastReliable+10;

	QCborMap current = q->m_snapshots.getCurrentSnapshot().toCbor();
	current.insert(QStringLiteral("t"), tick);

	if (q->m_deadlineTick > 0)
		current.insert(QStringLiteral("d"), q->m_deadlineTick);

	if (const QCborArray &list = q->getPlayerData(reliable); !list.isEmpty()) {
		current.insert(QStringLiteral("pList"), list);
		reliable = true;
	}



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
		bool hasMsg = insertMessages(&map, it->get());

		// reliable when message is present

		peer->send(map.toCborValue().toCbor(), reliable || hasMsg);

		peer->setLastSentTick(tick);
	}

	if (reliable)
		m_lastReliable = tick;

	clearMessages();
}





/**
 * @brief RpgEnginePrivate::dataSendFinished
 */

void RpgEnginePrivate::dataSendFinished()
{
	const qint64 tick = q->currentTick();

	QCborMap current = q->m_snapshots.getCurrentSnapshot().toCbor();
	current.insert(QStringLiteral("t"), tick);

	if (q->m_deadlineTick > 0)
		current.insert(QStringLiteral("d"), q->m_deadlineTick);

	if (const QCborArray &list = q->getPlayerData(true); !list.isEmpty()) {
		current.insert(QStringLiteral("pList"), list);
	}

	for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
		UdpServerPeer *peer = it->get()->udpPeer();

		if (!peer)
			continue;

		if (peer->isReconnecting()) {
			continue;
		}

		QCborMap map = current;

		insertBaseMapData(&map, it->get());
		insertMessages(&map, it->get());

		peer->send(map.toCborValue().toCbor(), true);

		peer->setLastSentTick(tick);
	}

	clearMessages();
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
 * @brief RpgEnginePrivate::insertMessages
 * @param dst
 * @param player
 */

bool RpgEnginePrivate::insertMessages(QCborMap *dst, RpgEnginePlayer *player)
{
	Q_ASSERT(dst);
	Q_ASSERT(player);

	if (m_messages.empty())
		return false;

	for (RpgEngineMessage &message : m_messages) {
		if (!message.canSend(player->playerId()))
			continue;

		dst->insert(QStringLiteral("msg"), message.message().toCborMap());
		message.addSent(player->playerId());

		return true;
	}

	return false;

}


/**
 * @brief RpgEnginePrivate::clearMessages
 */

void RpgEnginePrivate::clearMessages()
{
	for (auto mIt = m_messages.cbegin(); mIt != m_messages.cend(); ) {
		bool canSend = false;
		for (auto it = q->m_player.cbegin(); it != q->m_player.cend(); ++it) {
			if (mIt->canSend(it->get()->playerId())) {
				canSend = true;
				break;
			}
		}

		if (canSend)
			++mIt;
		else
			mIt = m_messages.erase(mIt);
	}
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
			ELOG_DEBUG << "All players completed" << q << q->id();

			ELOG_INFO << "-------------------------------------------------------------";
			ELOG_INFO << "PLAYERS";
			ELOG_INFO << "-------------------------------------------------------------";


			for (const auto &ptr : q->m_player) {
				QByteArray txt;

				if (ptr->isHost())
					txt += QByteArrayLiteral("(*) ");
				else
					txt += QByteArrayLiteral("( ) ");

				UdpServerPeer *peer = ptr->udpPeer();

				txt += QStringLiteral("P%1 [%2] %3")
					   .arg(ptr->playerId())
					   .arg(ptr->peerID(), 12)
					   .arg(peer ? peer->address() : QStringLiteral("---"), 21)
					   .toLatin1();

				ELOG_INFO << txt.constData();
			}

			ELOG_INFO << "-------------------------------------------------------------";

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
			ELOG_DEBUG << "All players prepared";

			q->m_config.gameState = RpgConfig::StatePlay;

			for (const auto &ptr : q->m_player) {
				if (!m_gameConfig.collection.quest.isEmpty())
					q->messageAdd(RpgGameData::Message(m_gameConfig.collection.quest.arg(ptr->rq), true), QList<int>{ptr->playerId()});
				else if (ptr->rq > 1)
					q->messageAdd(RpgGameData::Message(QStringLiteral("Collect %1 items").arg(ptr->rq), true), QList<int>{ptr->playerId()});
				else if (ptr->rq > 0)
					q->messageAdd(RpgGameData::Message(QStringLiteral("Collect 1 item"), true), QList<int>{ptr->playerId()});
			}
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

			if (allFullyPrepared) {
				m_elapsedTimer.start();

				ELOG_DEBUG << "All players fully prepared";
			}

		} else {

			const double tMs = q->currentTick() * 1000./60.;
			const double currMs = m_elapsedTimerReference + m_elapsedTimer.elapsed();
			int diff = (currMs-tMs) * 60./1000.;

			if (currMs-tMs >= 2.*1000./60.) {
				ELOG_WARNING << "Engine render lag" << currMs-tMs;
			}

			if (diff > 0) {

				QElapsedTimer t;

				t.start();

				QString txt;

				for (; diff > 0; --diff) {
					txt = q->m_snapshots.render(q->nextTick());

					if (q->m_deadlineTick > -1 && q->m_currentTick >= q->m_deadlineTick)
						break;
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

			if (!q->m_hostPlayer && q->m_config.gameState < RpgConfig::StateFinished)
				q->setHostPlayer(ptr.get());

			break;
		}
	}

	if (!hasPeer) {
		ELOG_INFO << "All peers disconnected from engine";

		if (m_elapsedTimer.isValid()) {
			m_elapsedTimerReference = q->currentTick() * 1000./60.;
			ELOG_DEBUG << "Set elapsed reference:" << m_elapsedTimerReference;
			m_elapsedTimer.invalidate();
		}

		if (q->m_config.gameState == RpgConfig::StatePlay)
			m_removeTimer.setRemainingTime(90000);
		else
			m_removeTimer.setRemainingTime(10000);
	} else {
		if (q->m_config.gameState != RpgConfig::StateFinished)
			m_removeTimer.setRemainingTime(-1);
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

	ELOG_DEBUG << "Peer trying reconnect" << peer->peerID();

	for (const auto &ptr : q->m_player) {
		if (ptr->peerID() == peer->peerID()) {
			ELOG_INFO << "Reconnecting peer" << peer->peerID();
			if (!ptr->udpPeer()) {
				ptr->setUdpPeer(peer);

				if (!q->m_hostPlayer)
					q->setHostPlayer(ptr.get());

				if (RpgEventControlUnlock *e = q->eventFind<RpgEventControlUnlock>(*ptr.get())) {
					eventRemove(e);
				}

				return true;
			} else {
				ELOG_ERROR << "Reconnect peer error" << peer->peerID();
				LOG_CERROR("engine") << "Reconnect peer error" << peer->peerID();

				return false;
			}
		}
	}

	return false;
}


/**
 * @brief RpgEnginePrivate::banOutPlayer
 * @param player
 * @return
 */

bool RpgEnginePrivate::banOutPlayer(RpgEnginePlayer *player)
{
	if (!player)
		return false;

	m_banList.push_back(player->peerID());

	if (player->udpPeer())
		q->m_udpServer->peerRemoveEngine(player->udpPeer());

	return true;
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
			ELOG_ERROR << "Invalid enemy data" << enemy;
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

			q->m_snapshots.enemyAdd(enemy, edata);
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
			ELOG_ERROR << "Invalid control light data" << cd;
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
		}
	}



	for (const auto &ptr : snapshot.controls.containers) {
		const RpgGameData::ControlContainerBaseData &cd = ptr.data;

		if (cd.t != RpgConfig::ControlContainer) {
			ELOG_ERROR << "Invalid control container data" << cd;
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
		}
	}



	for (const auto &ptr : snapshot.controls.gates) {
		const RpgGameData::ControlGateBaseData &cd = ptr.data;

		if (cd.t != RpgConfig::ControlGate) {
			ELOG_ERROR << "Invalid control gate data" << cd;
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
		}
	}


	for (const auto &ptr : snapshot.controls.teleports) {
		const RpgGameData::ControlTeleportBaseData &cd = ptr.data;

		if (cd.t != RpgConfig::ControlTeleport) {
			ELOG_ERROR << "Invalid control teleport data" << cd;
			continue;
		}

		const auto &controls = q->m_snapshots.controls().teleports;

		const auto it = std::find_if(controls.cbegin(), controls.cend(),
									 [&cd](const auto &e) {
			return e.data == cd;
		});

		if (it == controls.cend() && !ptr.list.empty()) {
			RpgGameData::ControlTeleport data = ptr.list.cbegin()->second;

			data.f = 0;

			q->m_snapshots.teleportAdd(cd, data);
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

	/*int max = 0;

	for (const RpgGameData::CollectionGroup &g : m_gameConfig.collection.groups) {
		max += g.pos.size();
	}*/

	const int req = 3 * q->m_player.size();//QRandomGenerator::global()->bounded(5, 7);

	ELOG_INFO << "Generate" << req << "collection items with images" << m_gameConfig.collection.images;



	const QHash<int, QList<int> > &pos = m_gameConfig.collection.allocate(req, &m_collectionGenerated);

	for (const auto &[gid, list] : pos.asKeyValueRange()) {
		const auto &it = m_gameConfig.collection.find(gid);

		if (it == m_gameConfig.collection.groups.cend()) {
			ELOG_ERROR << "Invalid GID" << gid;
			continue;
		}

		RpgGameData::ControlCollectionBaseData base;
		base.o = SERVER_OID;
		base.s = it->scene;
		base.gid = gid;


		for (const int &idx : list) {
			RpgGameData::ControlCollectionBaseData cd = base;
			cd.id = q->nextObjectId();

			if (const auto &s = m_gameConfig.collection.images.size(); s > 1)
				cd.img = m_gameConfig.collection.images.at(QRandomGenerator::global()->bounded(s));
			else if (s > 0)
				cd.img = m_gameConfig.collection.images.first();

			RpgGameData::ControlCollection data;
			data.f = 0;
			data.sc = base.s;
			data.a = true;
			data.idx = idx;

			const auto &ptr = it->pos.at(idx);

			data.p = QList<float>{ptr.x, ptr.y};

			q->m_snapshots.collectionAdd(cd, data);
		}
	}


	if (q->m_player.size() == 1) {
		q->m_player.front()->rq = m_collectionGenerated;
		return;
	}


	QList<RpgGameData::PlayerBaseData *> pList;

	pList.reserve(q->m_player.size());

	for (const auto &ptr : q->m_player) {
		pList.append(ptr.get());
	}


	ELOG_DEBUG << "Assign" << (m_collectionGenerated - q->m_player.size()) << "items to" << pList.size() << "players";

	RpgGameData::PlayerBaseData::assign(pList, m_collectionGenerated - q->m_player.size());
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
		ELOG_ERROR << "Missing places for group" << base.gid << "in" << base;
		return -1;
	}


	QList<int> freeIndices;
	freeIndices.reserve(it->pos.size());

	for (int i=0; i<it->pos.size(); ++i) {
		if (!it->pos.at(i).done)
			freeIndices.append(i);
	}


	for (const auto &ptr : q->controlCollections()) {
		if (ptr.list.empty())
			continue;

		if (ptr.data.gid != base.gid)
			continue;

		freeIndices.removeAll(ptr.get(tick)->idx);
	}

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
		ELOG_ERROR << "Invalid index" << idx << "in" << base;
		return false;
	}

	const auto &it = m_gameConfig.collection.find(base.gid);

	if (it == m_gameConfig.collection.groups.cend() || it->pos.empty()) {
		ELOG_ERROR << "Missing places for group" << base.gid << "in" << base;
		return false;
	}

	if (idx >= it->pos.size()) {
		ELOG_ERROR << "Invalid index" << idx << ">=" << it->pos.size() << "in" << base;
		return false;
	}

	it->pos[idx].done = true;

	return true;
}




/**
 * @brief RpgEnginePrivate::createRandomizer
 */

void RpgEnginePrivate::createRandomizer()
{
	if (m_randomizer.has_value())
		return;

	m_randomizer = m_gameConfig.randomizer;
	m_randomizer->randomize();
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

Logger *RpgEventBase::_logger() const
{
	return m_engine->_logger();
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
			ELOG_WARNING << "Event unique constraint failed";
			return;
		}
	}

	for (const auto &ptr : m_eventsLater) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			ELOG_WARNING << "Event unique constraint failed";
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
		eventAddLater<T>(std::forward<Args>(args)...);
		return;
	}

	std::unique_ptr<T> e(new T(q, std::forward<Args>(args)...));

	for (const auto &ptr : m_events) {
		if (ptr->isUnique() && ptr->isEqual(e.get())) {
			ELOG_ERROR << "Event unique constraint failed";
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
 * @brief RpgEnginePrivate::toUserGame
 * @return
 */

UserAPI::UserGame RpgEnginePrivate::toUserGame() const
{
	UserAPI::UserGame game;

	game.map = q->m_config.mapUuid;
	game.mission  = q->m_config.missionUuid;
	game.level = q->m_config.missionLevel;
	game.mode = GameMap::Rpg;
	game.campaign = q->m_config.campaign;

	return game;
}




/**
 * @brief RpgEnginePrivate::gameCreate
 * @return
 */

bool RpgEnginePrivate::gameCreate(RpgEnginePlayer *player)
{
	if (!player || player->m_gameId > 0)
		return false;

	WebServer *server = q->m_service->webServer().lock().get();

	if (!server || !server->handler()) {
		LOG_CERROR("engine") << "WebServer not found";
		return false;
	}

	UserAPI::UserGame game = toUserGame();
	UserAPI *api = server->handler()->api<UserAPI>("user");

	if (!api) {
		LOG_CERROR("engine") << "UserAPI not found" << UserAPI::apiPath();
		return false;
	}


	bool r = true;

	int gameId = -1;
	api->gameCreate(player->config().username, game.campaign, game, {}, &gameId);

	if (gameId == -1) {
		LOG_CERROR("engine") << "Game create error for user" << player->config().username;
		r = false;
	} else {
		player->m_gameId = gameId;
	}

	return r;
}






/**
 * @brief RpgEnginePrivate::gameFinish
 * @return
 */

bool RpgEnginePrivate::gameFinish()
{
	WebServer *server = q->m_service->webServer().lock().get();

	if (!server || !server->handler()) {
		LOG_CERROR("engine") << "WebServer not found";
		return false;
	}

	UserAPI::UserGame game = toUserGame();
	UserAPI *api = server->handler()->api<UserAPI>("user");

	if (!api) {
		LOG_CERROR("engine") << "UserAPI not found";
		return false;
	}

	for (const auto &pl : q->m_player) {
		if (pl->config().finished) {
			pl->m_finalSuccess = true;
		}

		if (pl->m_gameId < 0) {
			ELOG_WARNING << "Invalid game id" << pl->config().username;
			continue;
		}

		api->gameFinish(pl->config().username, pl->m_gameId, game, {}, {},
						false, pl->config().xp, q->m_currentTick * 1000./60., nullptr);
	}

	return true;
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
		txt += QStringLiteral("%1: ").arg(hash.value(key, QStringLiteral("???")), 16);

		txt += QStringLiteral("avg: %1 med: %2 max: %3")
			   .arg(d.avg(), 3)
			   .arg(d.med, 3)
			   .arg(d.max, 3)
			   ;

		if (!d.data.isEmpty()) {
			const auto [min, max] = std::minmax_element(d.data.constBegin(), d.data.constEnd());
			txt += QStringLiteral(" (min: %1 max: %2)").arg(*min, 3).arg(*max, 3);
		}

		txt += '\n';
	}

	txt += QStringLiteral(" \n");

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
	txt += QStringLiteral("------------------------------------------------------------------\n");

	txt += renderTimerDump();

	txt += QStringLiteral("State: %1 | Players: %2 | Tick: %3\n")
		   .arg(q->m_config.gameState, 2).arg(q->m_player.size(), 2)
		   .arg(q->m_currentTick, 5)
		   ;

	txt += QStringLiteral("------------------------------------------------------------------\n");

	for (const auto &ptr : q->m_player) {
		if (ptr->isHost())
			txt += QStringLiteral("(*) ");
		else
			txt += QStringLiteral("( ) ");

		txt += QStringLiteral("P%1 [%2] ")
			   .arg(ptr->playerId())
			   .arg(ptr->peerID(), 12)
			   ;


		if (UdpServerPeer *peer = ptr->udpPeer()) {
			txt += QStringLiteral("%1 | ").arg(peer->address(), 21);
			txt += QStringLiteral("RTT %1 | FPS: %2 | Peer FPS: %3")
				   .arg(peer->currentRtt(), 2)
				   .arg(peer->currentFps(), 2)
				   .arg(peer->peerFps(), 2)
				   ;
		}

		txt += '\n';
	}

	txt += QStringLiteral(" \n \n");

	return txt;
}







/**
 * @brief RpgEventEnemyDied::process
 * @return
 */

bool RpgEventEnemyDied::process(const qint64 &/*tick*/, RpgGameData::CurrentSnapshot *dst)
{
	Q_UNUSED(dst);

	ELOG_DEBUG << "Enemy died processed" << m_data << "@" << m_tick;

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
		ELOG_INFO << "All enemies died";
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

	ELOG_DEBUG << "Player died processed" << m_data << "@" << m_tick;

	RpgGameData::Message msg;

	if (RpgEnginePlayer *player = m_engine->player(m_data)) {
		msg.m = QStringLiteral("%1 died").arg(player->config().nickname.isEmpty() ?
												  player->config().username :
												  player->config().nickname);
	} else {
		msg.m = QStringLiteral("Player %1 died").arg(m_data.o);
	}


	m_engine->eventAdd<RpgEventPlayerResurrect>(m_tick+300, m_data);
	m_engine->messageAdd(msg, QList<int>{m_data.o}, true);

	return true;
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
		ELOG_ERROR << "Invalid player" << m_data;
		return true;
	}

	RpgGameData::Player player;

	if (!it->list.empty())
		player = it->get(m_tick).value();

	player.f = m_tick;
	player.hp = 45;

	dst->assign(dst->players, m_data, player);

	return true;
}



/**
 * @brief RpgEventCollectionRelocate::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventCollectionUsed::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
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
		ELOG_ERROR << "Invalid collection" << m_data;
		return true;
	}

	RpgGameData::ControlCollection d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	d.f = m_tick;
	d.u = {};

	const auto &players = m_engine->players();

	const auto pit = std::find_if(players.cbegin(),
								  players.cend(),
								  [this](const auto &p){
		return p.data.isBaseEqual(m_player);
	});


	if (m_success) {
		d.a = false;
		d.own = m_player;

		if (pit == players.cend() || pit->list.empty()) {
			ELOG_ERROR << "Invalid player" << m_data;
		} else {
			RpgGameData::Player playerData = pit->get(m_tick).value();
			playerData.c++;
			dst->assign(dst->players, m_player, playerData);

			const int left = pit->data.rq - playerData.c;

			if (left > 1)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect %1 more items").arg(left), true),
									 QList<int>{m_player.o});
			else if (left > 0)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect 1 more item"), true),
									 QList<int>{m_player.o});
			else
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("All required items collected"), true),
									 QList<int>{m_player.o});
		}

		m_engine->eventAdd<RpgEventCollectionPost>(m_tick+1, m_data, true);

	} else {
		if (pit == players.cend() || pit->list.empty()) {
			ELOG_ERROR << "Invalid player" << m_data;
		} else {
			RpgGameData::Player playerData = pit->get(m_tick).value();
			playerData.controlFailed(RpgConfig::ControlCollection);

			dst->assign(dst->players, m_player, playerData);
		}

		m_engine->eventAdd<RpgEventCollectionPost>(m_tick+180, m_data, false);
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

		ELOG_TRACE << "Unlock player" << it->data;
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
			ELOG_TRACE << "Unlock container" << ptr.data;
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
			ELOG_TRACE << "Unlock collection" << ptr.data;
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
		ELOG_ERROR << "Invalid pickable" << m_data;
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
		ELOG_ERROR << "Invalid player" << m_player;
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
		ELOG_INFO << "###### FINISH PICKABLE" << m_tick << m_data.id << "--->" << m_player.o;
		ELOG_INFO << "!!!!! ADD HP" << pData.f << pData.hp;
		dst->assign(dst->players, m_player, pData);
	} else {
		ELOG_ERROR << "###### PICKABLE PROCESS FAILED" << m_data.pt << m_tick << "--->" << m_player.o;
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

	ELOG_DEBUG << "Container use processed" << m_data << m_success << "@" << m_tick;

	if (!m_success) {
		const auto &pl = m_engine->players();

		const auto it = std::find_if(pl.cbegin(),
									 pl.cend(),
									 [this](const auto &p){
			return p.data.isBaseEqual(m_player);
		});

		if (it == pl.cend() || it->list.empty()) {
			ELOG_ERROR << "Invalid player" << m_data;
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
				ELOG_ERROR << "Invalid pickable" << m_data;
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




/**
 * @brief RpgEventCollectionPost::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventCollectionPost::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
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
		ELOG_ERROR << "Invalid collection" << m_data;
		return true;
	}

	RpgGameData::ControlCollection d;

	if (!it->list.empty())
		d = it->get(m_tick).value();

	if (m_success) {
		if (!m_engine->finishCollection(it->data, d.idx)) {
			ELOG_ERROR << "Finish colection failed" << it->data;
		}

		int left = -1;
		bool hasMissing = false;		// van-e, akinek még hiányzik

		// Recalculate collections

		for (const auto &p : m_engine->players()) {
			if (p.list.empty()) {
				ELOG_ERROR << "Empty player snap list";
				continue;
			}

			RpgGameData::Player playerData = p.get(m_tick).value();

			int coll = 0;
			int l = 0;				// csak egy körben megszámoljuk, mennyi van még szabadon

			for (const auto &c : m_engine->controlCollections()) {
				if (const auto &ptr = c.get(m_tick)) {
					if (left < 0 && !ptr->own.isValid())
						++l;

					if (ptr->own.isBaseEqual(p.data))
						++coll;
				}
			}

			if (left < 0)
				left = l;

			playerData.c = coll;

			if (coll < p.data.rq)
				hasMissing = true;

			dst->assign(dst->players, p.data, playerData);
		}

		if (left == 0) {
			ELOG_INFO << "All items collected, open teleports";

			bool hasTeleport = false;

			for (const auto &p : m_engine->controlTeleports()) {
				// Nem final teleportok kihagyása
				if (p.data.dst.isValid())
					continue;

				if (p.list.empty()) {
					ELOG_ERROR << "Empty player snap list";
					continue;
				}

				hasTeleport = true;

				RpgGameData::ControlTeleport pData = p.get(m_tick).value();

				pData.a = true;

				dst->assign(dst->controls.teleports, p.data, pData);

			}

			if (hasTeleport)
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Escape through the teleporter"), true));

		} else if (!hasMissing && !m_engine->hasMessageSent(RpgEngine::MessageCollectAllRemaining)) {
			m_engine->messageAdd(RpgGameData::Message(QStringLiteral("Collect all the remaining items"), true));
			m_engine->setMessageSent(RpgEngine::MessageCollectAllRemaining);
		}


	} else {
		d.f = m_tick;
		d.u = {};
		d.a = true;
		d.own = {};

		QPointF p;
		const int idx = m_engine->relocateCollection(it->data, m_tick, &p);

		ELOG_DEBUG << "Relocate collection" << m_data << "->" << idx << p << "@" << m_tick;

		if (idx >= 0) {
			d.idx = idx;
			d.p = QList<float>{(float) p.x(), (float) p.y()};
		}

		dst->assign(dst->controls.collections, m_data, d);
	}

	return true;
}




/**
 * @brief RpgEventTeleportUsed::process
 * @param tick
 * @param dst
 * @return
 */

bool RpgEventTeleportUsed::process(const qint64 &tick, RpgGameData::CurrentSnapshot *dst)
{
	Q_ASSERT(dst);

	if (tick < m_tick)
		return false;

	const auto &pl = m_engine->controlTeleports();

	const auto it = std::find_if(pl.cbegin(),
								 pl.cend(),
								 [this](const auto &p){
		return p.data == m_data;
	});

	if (it == pl.cend()) {
		ELOG_ERROR << "Invalid teleport" << m_data;
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
		ELOG_ERROR << "Invalid player" << m_player;
		return true;
	}

	if (pData.useTeleport(m_data, m_player)) {
		ELOG_INFO << "Teleport used" << m_data << "by player" << m_player << "@" << m_tick;
		dst->assign(dst->players, m_player, pData);

		// Hideout

		if (m_data.hd)
			return true;

		// Final teleport

		if (!m_data.dst.isValid()) {
			if (RpgEnginePlayer *enginePlayer = m_engine->playerSetGameCompleted(m_player)) {
				m_engine->messageAdd(RpgGameData::Message(QStringLiteral("%1 mission completed").arg(enginePlayer->config().nickname.isEmpty() ?
																										 enginePlayer->config().username :
																										 enginePlayer->config().nickname),
														  true),
									 QList<int>{m_data.o}, true);
			} else {
				ELOG_ERROR << "Invalid player" << m_player.o;
			}
		}



	} else {
		ELOG_ERROR << "Player teleport" << m_data << "usage failed for player" << m_player;

		const int left = std::max(1, m_player.rq - pData.c);

		if (left > 1)
			m_engine->messageAdd(RpgGameData::Message(QStringLiteral("%1 items missing").arg(left), false),
								 QList<int>{m_player.o});
		else
			m_engine->messageAdd(RpgGameData::Message(QStringLiteral("1 item missing"), false),
								 QList<int>{m_player.o});
	}

	return true;
}


/**
 * @brief RpgEnginePlayer::peerID
 * @return
 */

quint32 RpgEnginePlayer::peerID() const
{
	return m_peerID;
}

void RpgEnginePlayer::setPeerID(quint32 newPeerID)
{
	m_peerID = newPeerID;
}


/**
 * @brief RpgEnginePlayer::startPosition
 * @return
 */

const RpgGameData::PlayerPosition &RpgEnginePlayer::startPosition() const
{
	return m_startPosition;
}

void RpgEnginePlayer::setStartPosition(const RpgGameData::PlayerPosition &newStartPosition)
{
	m_startPosition = newStartPosition;
}
