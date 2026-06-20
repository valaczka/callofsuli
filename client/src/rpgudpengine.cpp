/*
 * ---- Call of Suli ----
 *
 * rpgudpengine.cpp
 *
 * Created on: 2026. 06. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgUdpEngine
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

#include "rpgudpengine.h"
#include "rpggame_p.h"
#include "client.h"
#include "utils_.h"


RpgUdpEngine::RpgUdpEngine(RpgGamePrivate *game, const PublicKeySigner &signer, QObject *parent)
	: AbstractUdpEngine{parent}
	, m_signer(signer)
	, m_gamePrivate(game)
	, m_game(m_gamePrivate->q)
{
	Q_ASSERT(m_gamePrivate);
}


/**
 * @brief RpgUdpEngine::~RpgUdpEngine
 */

RpgUdpEngine::~RpgUdpEngine()
{
	stop();
}



/**
 * @brief RpgUdpEngine::binaryDataReceived
 * @param list
 */

void RpgUdpEngine::binaryDataReceived(std::vector<UdpPacketRcv> &&list, const int &currentRtt)
{
	if (list.empty())
		return;

	if (Rpg::RpgLogicClientMulti *l = logic())
		l->setServerRtt(currentRtt);

	for (UdpPacketRcv &p : list)
		onDataReceived(std::move(p.data));
}


/**
 * @brief RpgUdpEngine::logic
 * @return
 */

Rpg::RpgLogicClientMulti *RpgUdpEngine::logic() const
{
	return dynamic_cast<Rpg::RpgLogicClientMulti*>(m_gamePrivate->m_logic.get());
}



/**
 * @brief RpgUdpEngine::onDataReceived
 * @param data
 */

void RpgUdpEngine::onDataReceived(std::unique_ptr<UdpBitStream> data)
{
	RpgStream::EngineDataStream stream(data);

	if (stream.operation() == RpgStream::EngineStream::OperationList) {
		RpgStream::RoomList st;
		st << stream;

		lobbyLoad(st);
	} else if (stream.operation() == RpgStream::EngineStream::OperationConnect ||
			   stream.dataOperation() == RpgStream::EngineDataStream::DataOperationCharacterSelect) {

		if (stream.operation() == RpgStream::EngineStream::OperationConnect) {
			RpgStream::Room r;
			r << stream;

			m_room = std::move(r);

		} else {
			RpgStream::CharacterSelectServer r;
			r << stream;

			m_room = r.room();

			m_gamePrivate->m_characterSelect.setGameConfig(r.gameConfig());
			m_gamePrivate->updateCharacterSelect();
		}

		setIsHost(m_room->hostId() == peerId());


		updateRoom();

		if (m_game->gameState() < RpgGame::GameStateCharacterSelect)
			m_game->setGameState(RpgGame::GameStateCharacterSelect);

	} else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationMapData) {
		if (m_game->gameState() < RpgGame::GameStatePrepare)
			m_game->setGameState(RpgGame::GameStatePrepare);

		updateMapData(std::move(stream));

		if (m_gameFlags == RpgStream::PlayerData::FlagNull)
			return;
	} else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationFull) {
		if (m_game->gameState() < RpgGame::GameStatePrepare)
			m_game->setGameState(RpgGame::GameStatePrepare);

		updateFull(std::move(stream));
	} else if (stream.dataOperation() == RpgStream::EngineDataStream::DataOperationState) {
		updateState(std::move(stream));
	}
}



/**
 * @brief RpgUdpEngine::updateRoom
 */

void RpgUdpEngine::updateRoom()
{
	emit m_game->readableRoomChanged();

	if (!m_room) {
		m_game->modelPlayer()->clear();
		return;
	}

	QVariantList l;

	for (const RpgStream::PlayerData &d : m_room->players()) {
		l.append(QVariantMap{
					 { QStringLiteral("playerId"),				d.playerId() },
					 { QStringLiteral("username"),				QString::fromUtf8(d.userName()) },
					 { QStringLiteral("nickname"),				QString::fromUtf8(d.nickName())+(m_isHost ? " HOST" : "") },
					 { QStringLiteral("character"),				d.characterResolved(m_gamePrivate->m_characterHash) },
					 { QStringLiteral("power"),					d.config().power() },
					 { QStringLiteral("team"),					d.team() },
				 });
	}


	Utils::patchSListModel(m_game->modelPlayer(), l, QStringLiteral("playerId"));
}



/**
 * @brief RpgUdpEngine::onBeforeWorldStep
 * @param tick
 */

void RpgUdpEngine::onBeforeWorldStep(const qint64 &tick)
{
	if (tick >= 0)
		return;

	if (m_gameFlags == RpgStream::PlayerData::FlagNull)
		return;

	if (m_fullReceived) {
		if (!m_gameFlags.testFlag(RpgStream::PlayerData::FlagGamePrepared)) {
			LOG_CINFO("game") << "******************* PREAPRED *********************";
		}

		m_gameFlags.setFlag(RpgStream::PlayerData::FlagGamePrepared);
	}

	RpgStream::EngineDataStream stream = getDataStream(RpgStream::EngineDataStream::DataOperationPlayerData);

	RpgStream::PlayerData d;

	d.setFlags(m_gameFlags);

	d >> stream;

	sendMessage(stream.data());
}



/**
 * @brief RpgUdpEngine::lobbyReload
 */

void RpgUdpEngine::lobbyReload()
{
	if (m_game->gameState() != RpgGame::GameStateLobby)
		return;

	sendMessage(getStream(RpgStream::EngineStream::OperationList).data());
}


/**
 * @brief RpgUdpEngine::lobbyCreate
 */

void RpgUdpEngine::lobbyCreate()
{
	if (m_game->gameState() != RpgGame::GameStateLobby)
		return;

	sendMessage(getStream(RpgStream::EngineStream::OperationCreate).data(), true);
}



/**
 * @brief RpgUdpEngine::lobbyConnect
 * @param id
 */

void RpgUdpEngine::lobbyConnect(const int &id)
{
	if (m_game->gameState() != RpgGame::GameStateLobby)
		return;

	RpgStream::Room r;
	r.setId(id);

	RpgStream::EngineStream stream = getStream(RpgStream::EngineStream::OperationConnect);
	r >> stream;

	sendMessage(stream.data(), true);
}





/**
 * @brief RpgUdpEngine::lobbyLoad
 * @param list
 */

void RpgUdpEngine::lobbyLoad(const RpgStream::RoomList &list)
{
	QVariantList l;

	//LOG_CDEBUG("client") << "......." << st.canCreate() << st.rooms().size();

	for (const RpgStream::Room &r : list.rooms()) {
		l.append(QVariantMap{
					 { QStringLiteral("roomId"), r.id() },
					 { QStringLiteral("readableId"), m_gamePrivate->toReadableRoomId(r) }
				 });
	}

	Utils::patchSListModel(m_game->modelLobby(), l, QStringLiteral("roomId"));
}



/**
 * @brief RpgUdpEngine::sendCharacterSelect
 * @param data
 */

void RpgUdpEngine::sendCharacterSelect(const RpgStream::CharacterSelectClient &data)
{
	RpgStream::EngineDataStream st = getDataStream(RpgStream::EngineDataStream::DataOperationCharacterSelect);

	data >> st;

	sendMessage(st.data(), true);

	if (data.data().flags().testFlags(RpgStream::PlayerData::FlagCompleted))
		m_gameFlags.setFlag(RpgStream::PlayerData::FlagCompleted);

}



/**
 * @brief RpgUdpEngine::updateMapData
 * @param stream
 */

void RpgUdpEngine::updateMapData(RpgStream::EngineDataStream &&stream)
{
	RpgStream::MapData m;

	m << stream;

	if (m_isHost && m_isMapReady) {
		RpgStream::EngineDataStream st = getDataStream(RpgStream::EngineDataStream::DataOperationMapData);

		m_gamePrivate->m_mapData >> st;

		sendMessage(st.data(), true);
	}


	if (m.playerPositionList().empty() || m.chunkGrid().chunkWidth() == 0 || m.chunkGrid().chunkHeight() == 0) {
		return;
	}

	if (!m_gamePrivate->m_isMapLoaded) {
		LOG_CDEBUG("game") << "Load map data";

		m_gamePrivate->m_logic->loadMapData(m);
		m_gamePrivate->m_isMapLoaded = true;
	}
}



/**
 * @brief RpgUdpEngine::updateFull
 * @param stream
 */

void RpgUdpEngine::updateFull(RpgStream::EngineDataStream &&stream)
{
	if (!m_gamePrivate->m_isMapLoaded) {
		LOG_CWARNING("game") << "Map not loaded";
		return;
	}

	RpgStream::Full full;

	full << stream;

	if (full.config().flags() == RpgStream::GameConfig::FlagNull) {
		LOG_CERROR("game") << "Invalid game data";
		return;
	}

	m_gamePrivate->m_logic->fullLoad(full);

	Rpg::RpgLogicClientMulti *l = logic();

	if (!l) {
		LOG_CERROR("game") << "Invalid logic" << m_gamePrivate->m_logic.get();
		return;
	}

	l->loadFull(full);

	m_fullReceived = true;
}



/**
 * @brief RpgUdpEngine::updateStage
 * @param config
 * @param tick
 */

void RpgUdpEngine::updateStage(const RpgStream::GameConfig &config, const quint32 &tick)
{
	m_gamePrivate->syncGameConfig(config, tick);
}



/**
 * @brief RpgUdpEngine::updateState
 * @param stream
 */

void RpgUdpEngine::updateState(RpgStream::EngineDataStream &&stream)
{
	if (m_game->gameState() < RpgGame::GameStatePrepare) {
		LOG_CERROR("game") << "Invalid state" << m_game->gameState();
	}

	RpgStream::FullState full;

	full << stream;

	if (full.flags() == RpgStream::FullState::Null) {
		LOG_CERROR("game") << "Invalid state data";
		return;
	}

	Rpg::RpgLogicClientMulti *l = logic();

	if (!l) {
		LOG_CERROR("game") << "Invalid logic" << m_gamePrivate->m_logic.get();
		return;
	}

	l->loadFullState(full);
}



/**
 * @brief RpgUdpEngine::sendState
 * @param data
 */

void RpgUdpEngine::sendState(const RpgStream::FullState &data)
{
	if (data.flags() == RpgStream::FullState::Null)
		return;

	RpgStream::EngineDataStream st = getDataStream(RpgStream::EngineDataStream::DataOperationState);

	data >> st;

	sendMessage(st.data(), false);


	// Send events realiable

	if (data.flags().testFlag(RpgStream::FullState::Event)) {
		RpgStream::FullState s2 = data;
		s2.setFlags(RpgStream::FullState::Null | RpgStream::FullState::Event);

		RpgStream::EngineDataStream st2 = getDataStream(RpgStream::EngineDataStream::DataOperationState);

		LOG_CINFO("game") << "SEND EVENT" << s2.flags() << s2.events().size();

		s2 >> st2;

		sendMessage(st2.data(), true);
	}
}



/**
 * @brief RpgUdpEngine::isHost
 * @return
 */

bool RpgUdpEngine::isHost() const
{
	return m_isHost;
}

void RpgUdpEngine::setIsHost(bool newIsHost)
{
	if (m_isHost == newIsHost)
		return;
	m_isHost = newIsHost;
	emit isHostChanged();

	if (m_isHost)
		m_game->client()->snack(tr("You are the host now"));
}



/**
 * @brief RpgUdpEngine::room
 * @return
 */

const std::optional<RpgStream::Room> &RpgUdpEngine::room() const
{
	return m_room;
}


