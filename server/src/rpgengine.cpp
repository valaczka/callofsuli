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
#include "userapi.h"
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

	if (!d->loadRpgConfig(server->service()->rpgConfig())) {
		ELOG_ERROR << "RPG config load failed";
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
				LOG_CERROR("engine") << "Engine create error";
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


	peer->send(toRoomList(engines, peer->server()->availablePeerCount()).toStream().data(), false);

}



/**
 * @brief RpgEngine::sendRoomList
 * @param server
 * @param engines
 */

void RpgEngine::sendRoomList(UdpServer *server, const QSet<RpgEngine *> &engines, const bool &reliable)
{
	Q_ASSERT(server);

	const std::vector<uint8_t> d = toRoomList(engines, server->availablePeerCount()).toStream().data();

	server->sendAll(d, [](UdpServerPeer *peer) {
		return peer && peer->peerData().type == EngineRpg && !peer->room();
	}, reliable);
}





/**
 * @brief RpgEngine::toRoomList
 * @param engines
 * @return
 */

RpgStream::RoomList RpgEngine::toRoomList(const QSet<RpgEngine *> &engines, const int &availablePeerCount)
{
	QMap<quint32, RpgEngine *> sorted;

	for (RpgEngine *e : engines)
		sorted.insert(e->internalId(), e);

	RpgStream::RoomList list;

	for (RpgEngine *e : sorted) {
		if (e->d->m_boardingCompleted)
			continue;

		list.rooms().emplace_back(e->toRoom());
	}

	list.setCanCreate(availablePeerCount > 1);		// Azért >1, mert multiplayer...

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

	if (engine->d->m_boardingCompleted) {
		LOG_CWARNING("engine") << "Engine already completed" << engine->readableId() << qPrintable(peer->address());
		peer->send(UdpBitStream(UdpBitStream::MessageRejected).data(), true);
		return false;
	}

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
	r.setCompleted(d->m_boardingCompleted);


	for (const RpgEnginePrivate::RpgPeerData &p : d->m_players) {
		RpgStream::PlayerData pd = p.data;

		pd.setNickName(pd.nickName() + QByteArray::number(pd.team()));

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
	else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationQuestSelect)
		d->receiveQuestSelect(player, std::move(stream));
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
	stream.setCharactersA(m_characters.value(RpgStream::TeamA));
	stream.setCharactersB(m_characters.value(RpgStream::TeamB));

	if (!m_selector.value(RpgStream::TeamA).empty())
		stream.setSelectA(m_selector.value(RpgStream::TeamA).front());
	if (!m_selector.value(RpgStream::TeamB).empty())
		stream.setSelectB(m_selector.value(RpgStream::TeamB).front());

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

	ELOG_DEBUG << "Create game records";


	// Create SQL game records

	for (RpgPeerData &p : m_players) {
		if (p.gameId == -1) {
			UserAPI::UserGame g;
			g.campaign = p.token.campaign;
			g.map = p.token.mapUuid;
			g.mission = p.token.missionUuid;
			g.mode = GameMap::Rpg;
			g.level = p.token.missionLevel;

			ServerService *service = q->m_udpServer->service();

			UserAPI::gameCreateRpg(q->udpServer()->service()->databaseMain(),
								   p.username, g.campaign, g,
								   service->rpgConfig()->characterHash().value(p.data.character()),
								   cfg->terrain(),
								   &p.gameId);

			if (p.gameId <= 0) {
				LOG_CERROR("engine") << "Game create error" << p.username;
				p.gameId = 0;
			} else {
				ELOG_DEBUG << "Set gameid" << p.gameId << "for" << qPrintable(p.username);
			}
		}

		if (p.gameId <= 0) {
			LOG_CERROR("engine") << "Game id error";
			return;
		}
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
		scope.logic()->playerAdd(p.data, &p.rpgId, &p.playerTag);
		p.acceptedTags.insert(p.playerTag, {});

		ELOG_DEBUG << "Add player" << p.rpgId << p.peerId << p.playerTag << p.data.userName() << p.token.mapUuid << p.token.missionUuid << p.token.missionLevel;
	}

	ELOG_INFO << "All player completed";

	cfg->flags().setFlag(RpgStream::GameConfig::FlagWaitingData);
}





/**
 * @brief RpgEnginePrivate::sendQuestSelect
 */

void RpgEnginePrivate::sendQuestSelect()
{
	if (!m_selectTimer.isValid()) {
		ELOG_ERROR << "Invalid select timer";
		return;
	}
	const int msec = CFG_GAME_STAGE_SELECT - m_selectTimer.elapsed();

	if (msec <= 0) {
		ELOG_WARNING << "Invalid remaingin select time";
		return;
	}

	Rpg::RpgLogicScope scope = q->m_logic.getScope();
	Rpg::QuestList *list = scope.getCtx<Rpg::QuestList>();
	Q_ASSERT(list);

	RpgStream::QuestSelect stream;
	stream.setMsecLeft(msec);
	stream.setQuestList(*list);

	const std::vector<uint8_t> data = stream.toDataStream().data();

	for (const RpgPeerData &p : m_players) {
		if (p.peer)
			p.peer->send(data, false);
	}
}







/**
 * @brief RpgEnginePrivate::receiveQuestSelect
 * @param player
 * @param stream
 */

void RpgEnginePrivate::receiveQuestSelect(RpgPeerData *player, RpgStream::EngineDataStream &&stream)
{
	Q_ASSERT(player);

	if (player->data.flags().testFlag(RpgStream::PlayerData::FlagQuestSelected)) {
		LOG_CWARNING("engine") << "Engine" << m_engineId << "player" << player->rpgId << "already selected";
		return;
	}

	RpgStream::QuestSelect s;
	s << stream;

	q->m_logic.selectQuest(s);

	player->data.flags().setFlag(RpgStream::PlayerData::FlagQuestSelected);


	bool cmpltd = true;

	for (const RpgPeerData &p : m_players) {
		if (!p.data.flags().testFlag(RpgStream::PlayerData::FlagQuestSelected)) {
			cmpltd = false;
			break;
		}
	}

	if (!cmpltd)
		return;

	ELOG_INFO << "All players' quest selected";

	onSelectFinished();
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

	bool mapReload = false;

	if (cfg->flags().testFlag(RpgStream::GameConfig::FlagDataCompleted)) {
		if (cfg->flags().testFlag(RpgStream::GameConfig::FlagDataReloaded)) {
			//LOG_CWARNING("engine") << "Engine data already completed";
			//ELOG_WARNING << "Engine selection already completed";
			return;
		}

		mapReload = true;
	}

	RpgStream::MapData d;
	d << stream;

	if (mapReload) {
		if (!d.forceReload()) {
			//LOG_CWARNING("engine") << "Expecting reloaded data";
			return;
		}

		q->m_logic.reloadMapData(d);
		return;
	}

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


	if (flags.testFlag(RpgStream::GameConfig::FlagDataPrepared) &&
			!flags.testFlags(RpgStream::GameConfig::FlagDataReloaded)) {
		RpgStream::MapData stream;
		stream.setForceReload(true);
		for (const RpgPeerData &p : m_players) {
			if (p.peer && p.peerId == m_host)
				p.peer->send(stream.toDataStream().data(), true);
		}
	}

	RpgStream::MapData stream;

	if (flags.testFlags(RpgStream::GameConfig::FlagDataCompleted)) {
		if (const auto &ptr = q->m_logic.getMapData())
			stream = ptr.value();
	}

	if (flags.testFlags(RpgStream::GameConfig::FlagDataReloaded))
		stream.setForceReload(true);

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

	addMapTagsToStream(stream);

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
 * @brief RpgEnginePrivate::sendResult
 */

void RpgEnginePrivate::sendResult()
{
	const auto flags = q->configFlags();

	if (!flags.testFlags(RpgStream::GameConfig::FlagFinished))
		return;


	Rpg::RpgLogicScope scope = q->m_logic.getScope();

	const RpgStream::Result *result = scope.getCtx<RpgStream::Result>();

	if (!result) {
		ELOG_ERROR << "Missing Result";
		return;
	}

	const std::vector<uint8_t> data = result->toDataStream().data();

	ServerService *service = q->m_udpServer->service();
	const auto &ptr = service->webServer().lock().get();

	UserAPI *api = ptr->handler()->api<UserAPI>("user");

	if (!api) {
		ELOG_ERROR << "Invalid UserAPI";
	}

	const int duration = q->m_logic.lastAuthTick()*1000./60.;

	for (RpgPeerData &p : m_players) {
		if (p.gameId > 0 && api) {
			ELOG_DEBUG << "Store game result:" << p.username;

			Credential c;
			c.setUsername(p.username);

			QJsonObject res = getResultForPlayer(*result, p.playerTag);
			res[QStringLiteral("duration")] = duration;

			api->gameFinish(c, p.gameId, res, &p.result);

			if (p.result.empty()) {
				ELOG_WARNING << "Store game error" << p.username;
			} else {
				p.gameId = 0;

				if (p.peer) {
					RpgStream::JsonResult json;
					json.setJson(QJsonDocument(p.result).toJson(QJsonDocument::Compact));

					const std::vector<uint8_t> data = json.toDataStream().data();
					p.peer->send(data, true);
				}
			}
		}

		if (p.peer)
			p.peer->send(data, false);
	}

}



/**
 * @brief RpgEnginePrivate::getResultForPlayer
 * @param result
 * @param playerTag
 * @return
 */

QJsonObject RpgEnginePrivate::getResultForPlayer(const RpgStream::Result &result, const quint32 &playerTag) const
{
	for (const RpgStream::PlayerResult &r : result.players()) {
		if (r.playerId() != playerTag)
			continue;

		QJsonObject o;

		o[QStringLiteral("point")] = (int) r.result().pts();
		o[QStringLiteral("token")] = (int) r.result().token();
		o[QStringLiteral("xp")] = (int) r.result().xp();
		o[QStringLiteral("success")] = (int) r.success();

		return o;
	}

	return {};
}





/**
 * @brief RpgEnginePrivate::addMapTagsToStream
 * @param stream
 */

void RpgEnginePrivate::addMapTagsToStream(RpgStream::Full &stream) const
{
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
		//LOG_CDEBUG("engine") << "NOT PREPARED";
		return;
	}

	bool cmpltd = true;

	for (const RpgPeerData &p : m_players) {
		if (!p.data.flags().testFlag(RpgStream::PlayerData::FlagGamePrepared)) {
			cmpltd = false;
			break;
		}
	}

	if (!cmpltd)
		return;


	if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagDataReloaded)) {
		//LOG_CDEBUG("engine") << "WAIT FOR RELOAD";
		return;
	}

	onAllPrepared();
}




/**
 * @brief RpgEnginePrivate::onAllPrepared
 */

void RpgEnginePrivate::onAllPrepared()
{
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

bool RpgEnginePrivate::loadRpgConfig(RpgLogicServerConfig *config)
{
	if (!config)
		return false;

	if (config->characters().empty())
		return false;

	m_characters.clear();

	std::vector<RpgStream::Character> list;
	list.reserve(config->characters().size());

	for (const QString &s : config->characters().keys()) {
		RpgStream::Character ch;
		ch.setCharacterResolved(s);
		ch.setDisabled(false);
		ch.setPicked(false);
		list.emplace_back(std::move(ch));
	}

	m_characters[RpgStream::TeamA] = list;
	m_characters[RpgStream::TeamB] = list;

	m_selector[RpgStream::TeamA] = {};
	m_selector[RpgStream::TeamB] = {};

	return true;
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
			ELOG_ERROR << "No host";
			onAborted();
		}

		return;
	}


	if (m_selectTimer.isValid()) {
		if (m_selectTimer.hasExpired(CFG_GAME_STAGE_SELECT)) {
			ELOG_INFO << "Select timer expired";
			onSelectFinished();
			return;
		} else {
			sendQuestSelect();
		}

	}


	const bool isStageSelect = (!running() && q->configStage() == RpgStream::GameConfig::StageSelect);

	quint32 t = isStageSelect ? 1 : tick();

	quint32 st = isStageSelect ? 0 : q->m_logic.serverTick();

	for (; st<t ; ++st) {
		bool requireFull = isStageSelect;

		if (isStageSelect)
			q->m_logic.renderStageSelect();
		else
			requireFull |= q->m_logic.render(false);

		if (st > 0)
			requireFull |= (st % 120 == 0);


		// SEND_STATE_COUNT db FullState-et gyártunk

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
				if (p.peer && !p.peer->socket())
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
			if (!p.peer)
				continue;

			// A websocketet nem akarjuk túltelíteni, ezért 60/SEND_STATE_COUNT fps-sel egyben kapja meg a
			// SEND_STATE_COUNT db FullState-et

			if (!p.peer->socket() || m_wsCounter == 0) {
				p.peer->send(data, requireFull && !isStageSelect);
			}
		}

		++m_wsCounter;
		if (m_wsCounter >= SEND_STATE_COUNT)
			m_wsCounter = 0;
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

		if (m_host == 0) {
			ELOG_INFO << "Close engine";
			m_closeTimer.setRemainingTime(5000);
		}

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
		LOG_CWARNING("engine") << "Engine" << m_engineId << "player" << player->rpgId << "already completed";
		ELOG_WARNING << "Player already completed" << player->rpgId;
		return;
	}

	bool toFill = false;

	RpgStream::CharacterSelectClient s;
	s << stream;

	{
		Rpg::RpgLogicScope scope = q->m_logic.getScope();
		RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();
		Q_ASSERT(cfg);

		if (cfg->flags().testFlag(RpgStream::GameConfig::FlagSelected)) {
			LOG_CWARNING("engine") << "Engine" << m_engineId << "selection already completed";
			ELOG_WARNING << "Engine selection already completed";
			return;
		}

		if (player->peerId == m_host && !m_boardingCompleted) {
			cfg->setTerrain(s.gameConfig().terrain());
			if (s.data().flags().testFlag(RpgStream::PlayerData::FlagOnboard) && cfg->terrain() > 0) {

				bool cmpltd = true;

				bool hasTeamA = false;
				bool hasTeamB = false;

				for (const RpgPeerData &p : m_players) {
					if (p.team == RpgStream::TeamA)
						hasTeamA = true;
					else if (p.team == RpgStream::TeamB)
						hasTeamB = true;

					if (p.peerId == player->peerId)
						continue;

					if (!p.data.flags().testFlag(RpgStream::PlayerData::FlagOnboard) ||
							!p.data.flags().testFlag(RpgStream::PlayerData::FlagPlayerOnline)) {
						cmpltd = false;
						break;
					}
				}

				if (cmpltd && hasTeamA && hasTeamB) {
					m_boardingCompleted = true;
					toFill = true;
				}
			}
		}
	}

	if (s.data().flags().testFlag(RpgStream::PlayerData::FlagOnboard) && !player->data.flags().testFlag(RpgStream::PlayerData::FlagOnboard)) {
		if (player->peerId != m_host || m_boardingCompleted) {
			ELOG_DEBUG << "Player onboard" << player->peerId;
			player->data.flags().setFlag(RpgStream::PlayerData::FlagOnboard);
		}
	}

	if (toFill) {
		ELOG_DEBUG << "Fill character selector queue";

		m_selector[player->team].push(player->peerId);

		for (const RpgPeerData &p : m_players) {
			if (p.peerId == m_host)
				continue;

			m_selector[p.team].push(p.peerId);
		}

		if (!m_selector.value(RpgStream::TeamA).empty())
			ELOG_DEBUG << "Wait for player" << m_selector.value(RpgStream::TeamA).front() << "in team A";
		if (!m_selector.value(RpgStream::TeamB).empty())
			ELOG_DEBUG << "Wait for player" << m_selector.value(RpgStream::TeamB).front() << "in team B";
	}

	if (!m_boardingCompleted && !player->data.flags().testFlag(RpgStream::PlayerData::FlagOnboard)) {
		if (s.data().team() != RpgStream::TeamNone) {
			player->team = s.data().team();
			player->data.setTeam(player->team);
		}
	}

	player->data.setNickName(s.data().nickName());
	player->data.setConfig(s.data().config());


	if (s.data().flags().testFlag(RpgStream::PlayerData::FlagCompleted)) {
		std::queue<quint32> &selector = m_selector[player->team];

		if (selector.empty() || selector.front() != player->peerId) {
			ELOG_WARNING << "Player can't select character" << player->peerId << "in team" << player->team;
		} else {
			const quint64 character = s.data().character();

			auto it = std::find_if(m_characters[player->team].begin(),
								   m_characters[player->team].end(),
								   [character](const RpgStream::Character &ch) {
				return (ch.character() == character);
			});

			if (it == m_characters[player->team].end()) {
				ELOG_ERROR << "Invalid character" << character << "from player" << player->peerId;
			} else if (it->disabled() || it->picked()) {
				ELOG_WARNING << "Character unavailable" << character << "for player" << player->peerId;
			} else {
				it->setPicked(true);
				selector.pop();

				ELOG_INFO << "Player" << player->peerId << "selected character" << character;

				if (!selector.empty())
					ELOG_DEBUG << "Wait for player" << selector.front() << "in team" << player->team;

				player->data.setCharacter(character);
				player->data.flags().setFlag(RpgStream::PlayerData::FlagCompleted);

				checkCompleted();
			}
		}
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

	if (!d->m_boardingCompleted)
		d->m_players.remove(id);

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
			d->m_closeTimer.setRemainingTime(5000);
		} else if (d->m_dtAcc < 100) {
			return;
		}

		d->sendFull();
		d->sendResult();

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
 * @brief RpgEngine::addTagToPlayer
 * @param idTag
 * @param player
 * @return
 */

quint32 RpgEngine::addTagToPlayer(const quint32 &idTag, Rpg::Player *player)
{
	RpgEnginePrivate::RpgPeerData *dst = nullptr;


	// Azt keressük mindig, akinél a legkevesebb van

	for (RpgEnginePrivate::RpgPeerData &p : d->m_players) {
		if (player && p.rpgId == player->playerData.playerId()) {
			ELOG_DEBUG << "Add" << idTag << "directly to player" << player->playerData.playerId();
			dst = &p;
			break;
		}

		if (!dst) {
			dst = &p;
			continue;
		}

		if (p.acceptedTags.size() < dst->acceptedTags.size())
			dst = &p;
	}

	if (!dst) {
		ELOG_ERROR << "AddTagToPlayer failed" << idTag;
		return 0;
	}

	dst->acceptedTags.insert(idTag, {});

	return dst->rpgId;
}



/**
 * @brief RpgEngine::addMapTagsToStream
 * @param stream
 */

void RpgEngine::addMapTagsToStream(RpgStream::Full &stream) const
{
	d->addMapTagsToStream(stream);
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
	LOG_CERROR("engine") << "Set temporary console appender";

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

	if (!m_boardingCompleted) {
		txt += QStringLiteral("Boarding... | Players: %1 | Terrain: %2 | Host: %3\n")
			   .arg(m_players.size(), 2)
			   .arg(cfg->terrain())
			   .arg(m_host);
	} else if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagSelected)) {
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



