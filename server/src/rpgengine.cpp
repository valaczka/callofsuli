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
	else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationFull)
		d->receiveFull(player, std::move(stream));
	else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationPlayerData)
		d->receivePlayerData(player, std::move(stream));
	else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationState)
		d->receiveState(player, std::move(stream));
	else
		LOG_CWARNING("engine") << "Invalid data" << stream.operation() << stream.dataOperation();
}



/**
 * @brief RpgEngine::sendCharacterSelect
 */

void RpgEnginePrivate::sendCharacterSelect(const bool reliable)
{
	RpgStream::CharacterSelectServer stream;

	{
		Rpg::RpgLogicScope scope = q->m_logic.getScope();
		RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
		Q_ASSERT(cfg);

		if (cfg->flags().testFlag(RpgStream::GameConfig::FlagFinished))
			return;

		stream.setGameConfig(*cfg);
	}

	stream.setRoom(q->toRoom());

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
	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	for (RpgPeerData &p : m_players) {
		auto player = scope.logic()->playerAdd(p.data, &p.rpgId, &p.playerTag);
		p.acceptedTags.insert(p.playerTag, {});

		ELOG_DEBUG << "Add player" << p.rpgId << p.peerId << p.playerTag << p.data.userName() << p.token.mapUuid << p.token.missionUuid << p.token.missionLevel;



		////////////////////////////////////////////////////////////////////////
		LOG_CERROR("game") << "REMOVE<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";

		const QString character = "soldier04";

		RpgStream::NpcData d;
		d.setCharacterResolved(character);
		d.setTeam(RpgStream::TeamNone);
		d.setType(RpgStream::NpcData::Dummy);
		d.entity().setMaxHp(7);

		quint32 idTag = 0;

		q->m_logic.npcAdd(d, player, &idTag);
		p.acceptedTags.insert(idTag, {});

		ELOG_DEBUG << "Add NPC" << idTag;

		////////////////////////////////////////////////////////////////////////
	}

	ELOG_INFO << "All completed";
	ELOG_INFO << engineDump().toUtf8().constData();

	cfg->flags().setFlag(RpgStream::GameConfig::FlagWaitingData);
}




/**
 * @brief RpgEnginePrivate::receivePlayerData
 * @param player
 * @param stream
 */

void RpgEnginePrivate::receivePlayerData(RpgPeerData *player, RpgStream::EngineDataStream &&stream)
{
	Q_ASSERT(player);

	if (!player->data.flags().setFlag(RpgStream::PlayerData::FlagCompleted)) {
		LOG_CWARNING("engine") << "Flag mismatch";
		return;
	}

	RpgStream::PlayerData d;
	d << stream;

	/*{
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
	}*/


	if (d.flags().testFlag(RpgStream::PlayerData::FlagLoadStarted)) {
		if (!player->data.flags().testFlag(RpgStream::PlayerData::FlagLoadStarted))
			ELOG_DEBUG << "Player" << player->data.playerId() << "load started...";

		player->data.flags().setFlag(RpgStream::PlayerData::FlagLoadStarted);
	}

	if (d.flags().testFlag(RpgStream::PlayerData::FlagLoadCompleted)) {
		if (player->data.flags().testFlag(RpgStream::PlayerData::FlagLoadStarted)) {

			if (!player->data.flags().testFlag(RpgStream::PlayerData::FlagLoadCompleted))
				ELOG_DEBUG << "Player" << player->data.playerId() << "load completed.";

			player->data.flags().setFlag(RpgStream::PlayerData::FlagLoadCompleted);
		}
	}

	if (d.flags().testFlag(RpgStream::PlayerData::FlagGamePrepared)) {
		if (player->data.flags().testFlag(RpgStream::PlayerData::FlagLoadCompleted)) {

			if (!player->data.flags().testFlag(RpgStream::PlayerData::FlagGamePrepared))
				ELOG_DEBUG << "Player" << player->data.playerId() << "game prepared.";

			player->data.flags().setFlag(RpgStream::PlayerData::FlagGamePrepared);

			checkPrepared();
		}
	}

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

	if (flags.testFlags(RpgStream::GameConfig::FlagFinished))
		return;

	if (flags.testFlags(RpgStream::GameConfig::FlagPlaying))
		return;

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
 * @brief RpgEnginePrivate::receiveFull
 * @param player
 * @param stream
 */

void RpgEnginePrivate::receiveFull(RpgPeerData *player, RpgStream::EngineDataStream &&stream)
{
	Q_ASSERT(player);

	LOG_CDEBUG("engine") << "TODO: receiveFull...";

	return;

	/*Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);*/

	/*if (cfg->flags().testFlag(RpgStream::GameConfig::FlagDataCompleted)) {
		//LOG_CWARNING("engine") << "Engine data already completed";
		//ELOG_WARNING << "Engine selection already completed";
		return;
	}*/


	// TODO: full request handling

	/*RpgStream::Full d;
	d << stream;


	if (d.config().flags().testFlag(RpgStream::PlayerData::FlagCompleted)) {
		player->data.flags().setFlag(RpgStream::PlayerData::FlagCompleted);

		checkCompleted();
	}

	if (!d.playerPositionList().empty() && d.chunkGrid().chunkHeight() > 0 && d.chunkGrid().chunkWidth() > 0) {
		q->m_logic.loadMapData(d);
		cfg->flags().setFlag(RpgStream::GameConfig::FlagWaitingData, false);
		cfg->flags().setFlag(RpgStream::GameConfig::FlagDataCompleted);

		onDataReceived();
	}*/
}



/**
 * @brief RpgEnginePrivate::onDataReceived
 */

void RpgEnginePrivate::onDataReceived()
{
	LOG_CWARNING("engine") << "ALL DATA RECEIVED";

	ELOG_INFO << "All data received";

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
		for (const quint32 &k : p.acceptedTags.keys()) {
			if (p.playerTag != k) {
				RpgStream::FullMapTag t;
				t.setTagId(k);
				m.entities().emplace_back(std::move(t));
			}
		}

		stream.map().emplace_back(std::move(m));
	}

	const std::vector<uint8_t> data = stream.toDataStream().data();

	for (const RpgPeerData &p : m_players) {
		// TODO: full request handling

		if (p.data.flags().testFlags(RpgStream::PlayerData::FlagGamePrepared) ||
				!p.data.flags().testFlags(RpgStream::PlayerData::FlagLoadCompleted))
			continue;

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
 * @brief RpgEnginePrivate::checkPrepared
 */

void RpgEnginePrivate::checkPrepared()
{
	if (m_players.isEmpty())
		return;

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	if (cfg->flags().testFlag(RpgStream::GameConfig::FlagPlaying) ||
			cfg->flags().testFlag(RpgStream::GameConfig::FlagFinished)) {
		return;
	}

	if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagDataPrepared)) {
		LOG_CDEBUG("engine") << "NOT PREPARED";
		return;
	}

	bool cmpltd = true;

	for (const RpgPeerData &p : m_players) {
		if (!p.data.flags().testFlag(RpgStream::PlayerData::FlagGamePrepared)) {
			cmpltd = false;
			break;
		}
	}

	LOG_CDEBUG("engine") << "COMPLETED" << cmpltd;

	if (!cmpltd)
		return;


	//cfg->flags().setFlag(RpgStream::GameConfig::FlagSelected);

	onAllPrepared();
}




/**
 * @brief RpgEnginePrivate::onAllPrepared
 */

void RpgEnginePrivate::onAllPrepared()
{
	LOG_CWARNING("engine") << "ALL PREPARED";

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	ELOG_INFO << "All prepared";

	cfg->flags().setFlag(RpgStream::GameConfig::FlagPlaying);

	scope.logic()->startStageSelect();
	m_selectTimer.start();
}



/**
 * @brief RpgEnginePrivate::receiveState
 * @param player
 * @param stream
 */

void RpgEnginePrivate::receiveState(RpgPeerData *player, RpgStream::EngineDataStream &&stream)
{
	Q_ASSERT(player);

	/*if (player->data.flags().testFlag(RpgStream::PlayerData::FlagCompleted)) {
		LOG_CWARNING("engine") << "Player already completed" << player->peerId;
		ELOG_WARNING << "Player already completed" << player->peerId;
		return;
	}*/

	if (!q->configFlags().testFlag(RpgStream::GameConfig::FlagPlaying)) {
		LOG_CWARNING("engine") << "NOT PLAYING";
		//ELOG_WARNING << "Engine selection already completed";
		return;
	}


	RpgStream::FullState s;
	s << stream;

	//LOG_CDEBUG("engine") << "***" << s.serverTick() << s.flags() << s.events().size();

	/*for (const RpgStream::PlayerStateList &l : s.players()) {
		LOG_CDEBUG("engine") << "   " << l.tagId();
	}*/

	q->m_logic.fullStateLoad(s, &player->acceptedTags);
}



/**
 * @brief RpgEnginePrivate::onSelectFinished
 */

void RpgEnginePrivate::onSelectFinished()
{
	m_selectTimer.invalidate();

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	ELOG_INFO << "Select finished";

	LOG_CINFO("engine")	 << "SELECT FINISHED";

	m_deadlineTick = cfg->duration();

	start(0);
}





/**
 * @brief RpgEnginePrivate::onAborted
 */

void RpgEnginePrivate::onAborted()
{
	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
	Q_ASSERT(cfg);

	cfg->flags().setFlag(RpgStream::GameConfig::FlagFinished);

	ELOG_INFO << "Aborted";

	LOG_CINFO("engine")	 << "ABORTED";

	stop();
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
 * @brief RpgEnginePrivate::render
 */

void RpgEnginePrivate::render()
{
	if (m_deadlineTick > 0 && !running()) {
		return;
	}

	if (m_deadlineTick > 0 && m_host == 0) {
		if (running()) {
			LOG_CERROR("engine") << "NO HOST";
			onAborted();
		}

		return;
	}


	if (m_selectTimer.isValid() && m_selectTimer.hasExpired(2500)) {
		LOG_CINFO("engine") << "EXPIRED";
		onSelectFinished();

		return;
	}


	const bool isStageSelect = (!running() && q->configStage() == RpgStream::GameConfig::StageSelect);

	quint32 t = isStageSelect ? 1 : tick();

	/*if (t > m_deadlineTick) {
		LOG_CINFO("engine") << "STOP GAME";

		stop();

		return;
	}*/

	quint32 st = isStageSelect ? 0 : q->m_logic.serverTick();

	for (; st<t ; ++st) {
		bool requireFull = isStageSelect;

		if (isStageSelect)
			q->m_logic.renderStageSelect();
		else
			requireFull |= q->m_logic.render(false);

		const RpgStream::Full &full = q->m_logic.getRenderedState(requireFull);

		const RpgStream::FullState::Flags flags = full.fullState().flags();

		if (!requireFull && flags == RpgStream::FullState::Null)
			continue;

		RpgStream::FullState state = full.fullState();

		// Send only events (reliable)

		if (flags.testFlag(RpgStream::FullState::Event)) {
			state.setFlags(RpgStream::FullState::Null | RpgStream::FullState::Event);

			const std::vector<uint8_t> data = state.toDataStream().data();

			for (const RpgPeerData &p : m_players) {
				if (p.peer)
					p.peer->send(data, true);
			}

			state.setFlags(flags);
		}

		// Send full (not reliable)

		std::vector<uint8_t> data;

		if (requireFull)
			data = full.toDataStream().data();
		else
			data = state.toDataStream().data();


		for (const RpgPeerData &p : m_players) {
			if (p.peer)
				p.peer->send(data, requireFull && !isStageSelect);
		}
	}

}




/**
 * @brief RpgEnginePrivate::nextTeam
 * @return
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

		ELOG_INFO << "Set next host:" << h;

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
	UdpEngine::udpPeerAdd(peer);

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


	if (flags.testFlags(RpgStream::GameConfig::FlagFinished)) {
		if (d->m_closeTimer.isForever()) {
			LOG_CDEBUG("engine") << "FINISHED....";
			d->m_closeTimer.setRemainingTime(5000);
		} else if (d->m_dtAcc < 100) {
			return;
		}

		d->sendFull();

	} else if (flags.testFlags(RpgStream::GameConfig::FlagPlaying)) {
		d->render();

	} else if (!flags.testFlags(RpgStream::GameConfig::FlagSelected)) {
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
 * @brief RpgEngine::canRemove
 * @return
 */

bool RpgEngine::canRemove() const
{
	return !d->m_closeTimer.isForever() && d->m_closeTimer.hasExpired();
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
	appender->setDetailsLevel(Logger::Trace);
#else
	appender->setDetailsLevel(Logger::Debug);
#endif

	d->m_logger->registerAppender(appender);

	m_logic.setLogger(d->m_logger.get());



#ifndef QT_NO_DEBUG
	LOG_CERROR("engine") << "TEMPORARY APPENDER";

	ColorConsoleAppender *console = new ColorConsoleAppender;

	console->setDetailsLevel(Logger::Debug);

	console->setFormat(QString::fromStdString(
									 "%{time}{hh:mm:ss} %{category:-10} [%{TypeOne}] %{message} "+
									 ColorConsoleAppender::reset+ColorConsoleAppender::green+"<%{function} "+
									 ColorConsoleAppender::magenta+"%{file}:%{line}"+
									 ColorConsoleAppender::green+">\n"));

	d->m_logger->registerAppender(console);

#endif
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



