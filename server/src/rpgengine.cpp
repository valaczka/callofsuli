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
#include "rpgstream.h"
#include "FileAppender.h"
#include "Logger.h"
#include "udpserver.h"
#include "serverservice.h"
#include <QCborArray>
#include <QCborMap>

#include "rpgengine_p.h"


/**
 * @brief RpgEngine::RpgEngine
 * @param server
 * @param parent
 */

RpgEngine::RpgEngine(UdpServer *server, UdpRoom *room, QObject *parent)
	: UdpEngine(server, room, parent)
	, d(new RpgEnginePrivate(this))
	, m_logic(this)
	, m_id(RpgEnginePrivate::m_engineId++)
{
	Q_ASSERT(room);
	Q_ASSERT(server);

	LOG_CDEBUG("engine") << "Create new RpgEngine" << m_id << m_readableId;

	room->setType(EngineRpg);

	if (const QString &dir = server->service()->logDir(); !dir.isEmpty()) {
		const QString fname = dir+QStringLiteral("/rpg-%1.log").arg(m_id, 3, 10, '0');

		if (QFile::exists(fname))
			QFile::remove(fname);

		setLoggerFile(fname);
	}
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
 * @brief RpgEngine::peerWithoutRoomHandle
 * @param data
 * @param peer
 * @param engines
 */

void RpgEngine::peerWithoutRoomHandle(std::unique_ptr<UdpBitStream> &&data, UdpServerPeer *peer, const QSet<RpgEngine *> &engines)
{
	if (!peer || !peer->server())
		return;

	if (peer->room())
		return;


	if (data) {
		RpgStream::EngineStream stream(data);

		if (stream.operation() == RpgStream::EngineStream::OperationCreate) {

			RpgEngine *engine = peer->server()->createEngine<RpgEngine>();

			if (!engine) {
				LOG_CERROR("engine") << "ENGINE CREATE ERROR";
			} else {
				peer->server()->sendRoomList(EngineRpg, true);

				peerConnectToEngine(peer, engine);
			}

			return;
		} else if (stream.operation() == RpgStream::EngineStream::OperationConnect) {
			RpgStream::Room r;
			r << stream;

			if (RpgEngine *e = findEngine(peer->server(), r.id())) {

				peerConnectToEngine(peer, e);

				return;
			} else {
				LOG_CWARNING("engine") << "Room not found" << r.id() << r.readableId();
			}
		}
	}


	peer->send(toRoomList(engines).toStream().data(), false);

}



/**
 * @brief RpgEngine::sendRoomList
 * @param server
 * @param engines
 */

void RpgEngine::sendRoomList(UdpServer *server, const QSet<RpgEngine *> &engines, const bool &reliable)
{
	Q_ASSERT(server);

	const std::vector<uint8_t> d = toRoomList(engines).toStream().data();

	server->sendAll(d, [](UdpServerPeer *peer) {
		return peer && peer->peerData().type == EngineRpg && !peer->room();
	}, reliable);
}



/**
 * @brief RpgEngine::toRoomList
 * @param engines
 * @return
 */

RpgStream::RoomList RpgEngine::toRoomList(const QSet<RpgEngine *> &engines)
{
	QMap<quint32, RpgEngine *> sorted;

	for (RpgEngine *e : engines)
		sorted.insert(e->internalId(), e);

	RpgStream::RoomList list;

	for (RpgEngine *e : sorted)
		list.rooms().emplace_back(e->toRoom());

	list.setCanCreate(true);

	return list;
}


/**
 * @brief RpgEngine::findEngine
 * @param server
 * @param id
 * @return
 */

RpgEngine *RpgEngine::findEngine(UdpServer *server, const quint32 &id)
{
	if (!server)
		return nullptr;

	return server->findEngine<RpgEngine>([id](const UdpRoom *r) {
		if (r->type() != EngineRpg)
			return false;

		RpgEngine *e = qobject_cast<RpgEngine*>(r->engine());

		if (!e)
			return false;

		return e->m_id == id;
	});

}




/**
 * @brief RpgEngine::peerConnectToEngine
 * @param peer
 * @param engine
 * @return
 */

bool RpgEngine::peerConnectToEngine(UdpServerPeer *peer, RpgEngine *engine)
{
	if (!peer || !engine)
		return false;

	LOG_CDEBUG("engine") << "Peer connnect to engine" << engine->readableId() << qPrintable(peer->address());

	// TODO: can connect

	engine->room()->peerAdd(peer);

	RpgStream::EngineStream st(RpgStream::EngineStream::OperationConnect);

	engine->toRoom() >> st;

	peer->send(st.data(), true);

	return true;
}




/**
 * @brief RpgEngine::toRoom
 * @return
 */

RpgStream::Room RpgEngine::toRoom() const
{
	RpgStream::Room r;

	r.setId(m_id);
	r.setReadableId(m_readableId);
	r.setHostId(d->m_host);


	for (const RpgEnginePrivate::RpgPeerData &p : d->m_players) {
		RpgStream::PlayerData pd = p.data;

		pd.setNickName(pd.nickName() +
					   (pd.flags().testFlag(RpgStream::PlayerData::FlagPlayerOnline) ? " ON" : " off"));

		r.players().emplace_back(std::move(pd));
	}


	return r;
}




/**
 * @brief RpgEngine::binaryDataReceived
 * @param data
 */

void RpgEngine::binaryDataReceived(UdpServerPeerReceivedList &data)
{
	for (auto &pair : data)
		binaryDataReceived(pair);
}






/**
 * @brief RpgEngine::binaryDataReceived
 * @param peer
 * @param data
 */

void RpgEngine::binaryDataReceived(UdpPacketRcv &recv)
{
	Q_ASSERT(recv.peer);

	RpgStream::EngineDataStream stream(recv.data);


	if (stream.operation() == RpgStream::EngineStream::OperationConnect ||
			stream.operation() == RpgStream::EngineStream::OperationList) {
		RpgStream::EngineStream r(RpgStream::EngineStream::OperationConnect);
		toRoom() >> r;
		recv.peer->send(r.data(), true);

		return;
	}

	// Todo: disconnect,...


	RpgEnginePrivate::RpgPeerData *player = d->getPlayer(recv.peer);

	if (!player) {
		ELOG_ERROR << "Player not found" << recv.peer;
		return;
	}


	if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationCharacterSelect)
		d->receiveCharacterSelect(player, std::move(stream));
	else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationMapData)
		d->receiveWaitingData(player, std::move(stream));
}



/**
 * @brief RpgEngine::sendCharacterSelect
 */

void RpgEnginePrivate::sendCharacterSelect(const bool reliable)
{
	RpgStream::CharacterSelectServer stream;

	stream.setRoom(q->toRoom());

	{
		Rpg::RpgLogicScope scope = q->m_logic.getScope();
		RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
		Q_ASSERT(cfg);

		stream.setGameConfig(*cfg);
	}

	const std::vector<uint8_t> data = stream.toDataStream().data();

	for (const RpgPeerData &p : m_players) {
		if (p.peer)
			p.peer->send(data, reliable);
	}
}



/**
 * @brief RpgEnginePrivate::checkCompleted
 */

void RpgEnginePrivate::checkCompleted()
{
	if (m_players.isEmpty())
		return;

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	if (cfg->flags().testFlag(RpgStream::GameConfig::FlagSelected))
		return;

	bool cmpltd = true;

	for (const RpgPeerData &p : m_players) {
		if (!p.data.flags().testFlag(RpgStream::PlayerData::FlagCompleted)) {
			cmpltd = false;
			break;
		}
	}

	if (!cmpltd)
		return;


	if (cfg->terrain() == 0) {
		LOG_CERROR("engine") << "No terrain";
		return;
	}


	cfg->flags().setFlag(RpgStream::GameConfig::FlagSelected);

	onAllCompleted();
}



/**
 * @brief RpgEnginePrivate::onAllCompleted
 */

void RpgEnginePrivate::onAllCompleted()
{
	LOG_CWARNING("engine") << "ALL COMPLETED";

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);


	for (RpgPeerData &p : m_players) {
		scope.logic()->playerAdd(p.data, &p.rpgId, &p.playerTag);

		ELOG_DEBUG << "Add player" << p.rpgId << p.peerId << p.playerTag << p.data.userName() << p.token.mapUuid << p.token.missionUuid << p.token.missionLevel;
	}

	ELOG_INFO << "All completed";
	ELOG_INFO << engineDump().toUtf8().constData();

	cfg->flags().setFlag(RpgStream::GameConfig::FlagWaitingData);
}



/**
 * @brief RpgEnginePrivate::receiveWaitingData
 * @param player
 * @param stream
 */

void RpgEnginePrivate::receiveWaitingData(RpgPeerData *player, RpgStream::EngineDataStream &&stream)
{
	Q_ASSERT(player);

	if (player->peerId != m_host) {
		LOG_CWARNING("engine") << "Data without host permission";
		ELOG_WARNING << "Data without host permission" << qPrintable(player->peer->address()) << player->peerId;
		return;
	}

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	if (cfg->flags().testFlag(RpgStream::GameConfig::FlagDataCompleted)) {
		//LOG_CWARNING("engine") << "Engine data already completed";
		//ELOG_WARNING << "Engine selection already completed";
		return;
	}

	RpgStream::MapData d;
	d << stream;

	if (!d.playerPositionList().empty() && d.chunkGrid().chunkHeight() > 0 && d.chunkGrid().chunkWidth() > 0) {
		q->m_logic.loadMapData(d);
		cfg->flags().setFlag(RpgStream::GameConfig::FlagWaitingData, false);
		cfg->flags().setFlag(RpgStream::GameConfig::FlagDataCompleted);

		onDataReceived();
	}
}



/**
 * @brief RpgEnginePrivate::sendWaitingData
 */

void RpgEnginePrivate::sendWaitingData()
{
	const auto flags = q->configFlags();

	RpgStream::MapData stream;

	if (flags.testFlags(RpgStream::GameConfig::FlagDataCompleted)) {
		if (const auto &ptr = q->m_logic.getMapData())
			stream = ptr.value();
	}


	const std::vector<uint8_t> data = stream.toDataStream().data();

	for (const RpgPeerData &p : m_players) {
		if (p.peer)
			p.peer->send(data, false);
	}
}



/**
 * @brief RpgEnginePrivate::onDataReceived
 */

void RpgEnginePrivate::onDataReceived()
{
	LOG_CWARNING("engine") << "ALL DATA RECEIVED";

	ELOG_INFO << "All data received";
	ELOG_INFO << engineDump().toUtf8().constData();

	if (q->m_logic.initialize())
		ELOG_INFO << "Game initialized";
	else
		ELOG_ERROR << "Game initialize error";
}



/**
 * @brief RpgEnginePrivate::sendFull
 */

void RpgEnginePrivate::sendFull()
{
	const auto flags = q->configFlags();

	if (!flags.testFlags(RpgStream::GameConfig::FlagDataPrepared))
		return;


#ifdef WITH_FTXUI
	QString txt;
	RpgStream::Full stream = q->m_logic.getFull(&txt);
#else
	RpgStream::Full stream = q->m_logic.getFull();
#endif


	stream.map().reserve(m_players.size());

	for (const RpgPeerData &p : m_players) {
		RpgStream::FullPlayerMap m;

		m.setPeerId(p.peerId);
		m.setPlayer(p.playerTag);
		// TODO: set entities

		stream.map().emplace_back(std::move(m));
	}

	const std::vector<uint8_t> data = stream.toDataStream().data();

	for (const RpgPeerData &p : m_players) {
		if (p.peer)
			p.peer->send(data, false);
	}

#ifdef WITH_FTXUI
	QCborMap m;
	m.insert(QStringLiteral("mode"), QStringLiteral("SND"));
	m.insert(QStringLiteral("txt"), txt);
	q->m_udpServer->service()->writeToSocket(m.toCborValue());
#endif

}



/**
 * @brief RpgEnginePrivate::getPlayer
 * @param peer
 * @return
 */

RpgEnginePrivate::RpgPeerData *RpgEnginePrivate::getPlayer(UdpServerPeer *peer)
{
	auto it = std::find_if(m_players.begin(),
						   m_players.end(),
						   [peer](const RpgPeerData &d) {
		return d.peer == peer;
	});

	if (it == m_players.end())
		return nullptr;

	return &(it.value());
}




/**
 * @brief RpgEnginePrivate::receiveCharacterSelect
 * @param stream
 */

RpgStream::Team RpgEnginePrivate::nextTeam() const
{
	if (m_players.isEmpty())
		return RpgStream::TeamA;

	int numA = 0;
	int numB = 0;

	for (const RpgPeerData &p : m_players) {
		switch (p.team) {
			case RpgStream::TeamA:
				++numA;
				break;
			case RpgStream::TeamB:
				++numB;
				break;
			case RpgStream::TeamNone:
				break;
		}
	}


	return numA > numB ? RpgStream::TeamB : RpgStream::TeamA;
}







/**
 * @brief RpgEnginePrivate::changeHost
 * @return
 */

quint32 RpgEnginePrivate::changeHost()
{
	if (m_host != 0 && m_players.contains(m_host) && m_players[m_host].data.flags().testFlags(RpgStream::PlayerData::FlagPlayerOnline)) {
		return m_host;

	} else {
		quint32 h = 0;


		for (const RpgPeerData &p : m_players) {
			if (p.data.flags().testFlag(RpgStream::PlayerData::FlagPlayerOnline)) {
				h = p.peerId;
				break;
			}
		}

		LOG_CWARNING("engine") << "Next host" << h;

		m_host = h;

		return h;
	}
}




/**
 * @brief RpgEnginePrivate::receiveCharacterSelect
 * @param player
 * @param stream
 */

void RpgEnginePrivate::receiveCharacterSelect(RpgPeerData *player, RpgStream::EngineDataStream &&stream)
{
	Q_ASSERT(player);


	if (player->data.flags().testFlag(RpgStream::PlayerData::FlagCompleted)) {
		LOG_CWARNING("engine") << "Player already completed" << player->peerId;
		ELOG_WARNING << "Player already completed" << player->peerId;
		return;
	}

	RpgStream::CharacterSelectClient s;
	s << stream;

	{
		Rpg::RpgLogicScope scope = q->m_logic.getScope();
		RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
		Q_ASSERT(cfg);

		if (cfg->flags().testFlag(RpgStream::GameConfig::FlagSelected)) {
			LOG_CWARNING("engine") << "Engine selection already completed";
			ELOG_WARNING << "Engine selection already completed";
			return;
		}

		if (player->peerId == m_host)
			cfg->setTerrain(s.gameConfig().terrain());
	}


	player->data.setCharacter(s.data().character());
	player->data.setNickName(s.data().nickName());
	player->data.setConfig(s.data().config());

	if (s.data().team() != RpgStream::TeamNone)
		player->team = s.data().team();

	if (s.data().flags().testFlag(RpgStream::PlayerData::FlagCompleted)) {
		player->data.flags().setFlag(RpgStream::PlayerData::FlagCompleted);

		checkCompleted();
	}


	sendCharacterSelect(true);
}




/**
 * @brief RpgEngine::udpPeerAdd
 * @param peer
 */

void RpgEngine::udpPeerAdd(UdpServerPeer *peer)
{
	if (!peer)
		return;

	const quint32 id = peer->peerData().peerId;

	ELOG_DEBUG << "Peer add" << id << qPrintable(peer->peerData().username) << qPrintable(peer->address());


	if (d->m_players.contains(id)) {
		ELOG_WARNING << "Player peerId already exists" << id << peer->peerData().username;

		RpgEnginePrivate::RpgPeerData &pd = d->m_players[id];

		if (pd.peer) {
			ELOG_ERROR << "Peer already connected" << id << qPrintable(pd.peer->address());
			return;
		}

		pd.peer = peer;
		pd.data.flags().setFlag(RpgStream::PlayerData::FlagPlayerOnline);

		d->changeHost();

		d->sendCharacterSelect(true);

		return;
	}

	RpgEnginePrivate::RpgPeerData pd(peer->peerData());

	pd.loadToken();
	pd.peer = peer;
	pd.team = d->nextTeam();

	pd.data.setPlayerId(pd.peerId);
	pd.data.setUserName(pd.token.user.toUtf8());
	pd.data.setTeam(pd.team);
	pd.data.flags().setFlag(RpgStream::PlayerData::FlagPlayerOnline);

	d->m_players.insert(id, pd);

	d->changeHost();

	d->sendCharacterSelect(true);
}



/**
 * @brief RpgEngine::udpPeerRemove
 * @param peer
 */

void RpgEngine::udpPeerRemove(UdpServerPeer *peer)
{
	const quint32 id = peer->peerData().peerId;

	ELOG_DEBUG << "Peer remove" << id << qPrintable(peer->peerData().username) << qPrintable(peer->address());

	if (!d->m_players.contains(id)) {
		ELOG_WARNING << "Player peerId not found" << id;
		return;
	}

	RpgEnginePrivate::RpgPeerData &pd = d->m_players[id];

	if (pd.peer && pd.peer != peer) {
		ELOG_ERROR << "Peer mismatch" << id << qPrintable(pd.peer->address());
	}

	pd.peer = nullptr;
	pd.data.flags().setFlag(RpgStream::PlayerData::FlagPlayerOnline, false);

	d->changeHost();

	d->sendCharacterSelect(true);
}





/**
 * @brief RpgEngine::disconnectUnusedPeer
 * @param peer
 */

void RpgEngine::disconnectUnusedPeer(UdpServerPeer *peer)
{
	/*if (m_config.gameState == RpgConfig::StateFinished && !d->m_removeTimer.isForever() && d->m_removeTimer.hasExpired()) {
		ELOG_INFO << "Disconnect peer" << peer->peerID() << "from engine";

		if (peer->peer()) {
			enet_peer_disconnect_later(peer->peer(), 0);
		} else {
			LOG_CERROR("engine") << "Missing ENetPeer" << peer << "in peer" << peer->peerID();
		}
	}*/
}



/**
 * @brief RpgEngine::udpTimerEvent
 */

void RpgEngine::udpTimerEvent(const qint64 &dt)
{
	d->m_dtAcc += dt;

	const RpgStream::GameConfig::Flags flags = configFlags();


	if (!flags.testFlags(RpgStream::GameConfig::FlagSelected)) {
		if (d->m_dtAcc < 250)  return;

		d->sendCharacterSelect(false);

	} else if (flags.testFlags(RpgStream::GameConfig::FlagDataPrepared)) {
		if (d->m_dtAcc < 50)  return;

		d->sendWaitingData();
		d->sendFull();
	} else if (flags.testFlags(RpgStream::GameConfig::FlagWaitingData) &&
			!flags.testFlags(RpgStream::GameConfig::FlagDataCompleted)) {

		if (d->m_dtAcc < 50)  return;

		d->sendWaitingData();
	} else {
		return;
	}

	d->m_dtAcc = 0;
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
 * @brief RpgEngine::_logger
 * @return
 */

Logger *RpgEngine::_logger() const
{
	return d->m_logger.get();
}



/**
 * @brief RpgEngine::configFlags
 * @return
 */

RpgStream::GameConfig::Flags RpgEngine::configFlags() const
{
	Rpg::RpgLogicScope scope = m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	return cfg->flags();
}




/**
 * @brief RpgEngine::configStage
 * @return
 */

RpgStream::GameConfig::Stage RpgEngine::configStage() const
{
	Rpg::RpgLogicScope scope = m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	return cfg->stage();
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
	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	QString txt;

	txt += QStringLiteral("[ENGINE %1] - %2\n").arg(q->m_id).arg(q->m_readableId, 6, 10, '0');
	txt += QStringLiteral("------------------------------------------------------------------\n");

	txt += renderTimerDump();

	if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagSelected)) {
		txt += QStringLiteral("Character select... | Players: %1 | Terrain: %2 | Host: %3\n")
			   .arg(m_players.size(), 2)
			   .arg(cfg->terrain())
			   .arg(m_host);
	} else if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagPlaying)) {
		txt += QStringLiteral("Preparing... | Players: %1 | Terrain: %2 | Host: %3\n")
			   .arg(m_players.size(), 2)
			   .arg(cfg->terrain())
			   .arg(m_host);
	} else {
		txt += QStringLiteral("Stage: %1 | Players: %2 | Tick: %3 | Terrain: %4 | Host: %5\n")
			   .arg(cfg->stage(), 2)
			   .arg(m_players.size(), 2)
			   .arg(q->m_logic.serverTick(), 5)
			   .arg(cfg->terrain())
			   .arg(m_host)
			   ;
	}


	txt += QStringLiteral("------------------------------------------------------------------\n");

	for (const auto &[key, data] : m_players.asKeyValueRange()) {
		if (!data.peer)
			txt += QStringLiteral("    ");
		else if (key == m_host)
			txt += QStringLiteral("(*) ");
		else
			txt += QStringLiteral("( ) ");

		txt += QStringLiteral("%1 [%2] T%3 ")
			   .arg(data.rpgId, 2)
			   .arg(data.peerId, 12)
			   .arg(data.team)
			   ;

		txt += data.username + " " + data.data.nickName();


		if (UdpServerPeer *peer = data.peer) {
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



