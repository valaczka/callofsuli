/*
 * ---- Call of Suli ----
 *
 * rpgudpengine.cpp
 *
 * Created on: 2025. 01. 19.
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


#include "rpgudpengine.h"



/**
 * @brief RpgUdpEngine::~RpgUdpEngine
 */

RpgUdpEngine::RpgUdpEngine(ActionRpgMultiplayerGame *game)
	: AbstractUdpEngine(game)
	, m_game(game)
{

}




/**
 * @brief RpgUdpEngine::~RpgUdpEngine
 */

RpgUdpEngine::~RpgUdpEngine()
{

}


/**
 * @brief RpgUdpEngine::packetReceived
 * @param data
 */

void RpgUdpEngine::packetReceived(const QCborMap &data, const unsigned int rtt)
{
	if (!m_game)
		return;

	if (m_game->rpgGame())
		m_game->rpgGame()->tickTimer()->setLatency(rtt);

	QMutexLocker locker(&m_mutex);

	if (m_game->config().gameState == RpgConfig::StateError || m_gameState == RpgConfig::StateError)
		return;

	updateState(data);

	if (m_gameState == RpgConfig::StateConnect)
		return;
	else if (m_gameState == RpgConfig::StateCharacterSelect)
		packetReceivedChrSel(data);
	else if (m_gameState == RpgConfig::StatePrepare) {
		if (m_game->config().gameState == RpgConfig::StateDownloadContent)
			packetReceivedDownload(data);
		else if (m_game->config().gameState == RpgConfig::StatePrepare)
			packetReceivedPrepare(data);
	} else if (m_gameState == RpgConfig::StatePlay) {
		packetReceivedPlay(data);
	}
}






/**
 * @brief RpgUdpEngine::disconnect
 */

void RpgUdpEngine::disconnect()
{
	LOG_CWARNING("game") << "DISCONNECT";
	setUrl({});
}





/**
 * @brief RpgUdpEngine::updateState
 * @param data
 */

void RpgUdpEngine::updateState(const QCborMap &data)
{
	QMutexLocker locker(&m_mutex);

	if (auto it = data.find(QStringLiteral("t")); it != data.cend()) {
		m_lastTick = it->toInteger(-1);
	}

	if (auto it = data.find(QStringLiteral("hst")); it != data.cend()) {
		m_isHost = it->toBool();
	}

	if (auto it = data.find(QStringLiteral("ply")); it != data.cend()) {
		m_playerId = it->toInteger(-1);
	}

	if (auto it = data.find(QStringLiteral("st")); it != data.cend()) {
		m_gameState = QVariant(it->toInteger()).value<RpgConfig::GameState>();
	}

}



/**
 * @brief RpgUdpEngine::updateSnapshot
 * @param player
 */

void RpgUdpEngine::updateSnapshot(const RpgGameData::CharacterSelect &player)
{
	QMutexLocker locker(&m_mutex);

	const auto it = std::find_if(m_playerData.begin(),
								 m_playerData.end(),
								 [&player](const RpgGameData::CharacterSelect &ch){
		return ch.playerId == player.playerId;
	});

	if (it == m_playerData.cend())
		m_playerData.emplace_back(player);
	else
		*it = player;


	m_snapshots.updateSnapshot(player);
}




/**
 * @brief RpgUdpEngine::packetReceivedChrSel
 * @param data
 */

void RpgUdpEngine::packetReceivedChrSel(const QCborMap &data)
{
	if (!m_game)
		return;

	QCborArray pList = data.value(QStringLiteral("pp")).toArray();

	for (const QCborValue &v : pList) {
		RpgGameData::CharacterSelect ch;
		ch.fromCbor(v.toMap());
		updateSnapshot(ch);
	}

	if (m_game->gameMode() == ActionRpgGame::MultiPlayerGuest) {
		QCborMap cfgGame = data.value(QStringLiteral("g")).toMap();

		if (!cfgGame.isEmpty()) {
			RpgGameData::GameConfig config;
			config.fromCbor(cfgGame);
			m_gameConfig = config;
		}
	}

}




/**
 * @brief RpgUdpEngine::packetReceivedDownload
 * @param data
 */

void RpgUdpEngine::packetReceivedDownload(const QCborMap &data)
{
	if (!m_game)
		return;

	QMutexLocker locker(&m_mutex);

	if (m_downloadContentStarted)
		return;

	RpgGameData::GameConfig config;

	LOG_CDEBUG("game") << data;

	QCborMap cfgGame = data.value(QStringLiteral("g")).toMap();
	config.fromCbor(cfgGame);

	if (config.terrain.isEmpty()) {
		LOG_CERROR("game") << "Missing game config terrain";
		emit gameError();
		return;
	}

	m_gameConfig = config;

	QCborArray pList = data.value(QStringLiteral("pp")).toArray();

	LOG_CWARNING("game") << "#####" << pList;

	updateSnapshotPlayerList(pList);

	emit gameDataDownload(config.terrain, m_playerData);

	m_downloadContentStarted = true;
}





/**
 * @brief RpgUdpEngine::packetReceivedPrepare
 * @param data
 */

void RpgUdpEngine::packetReceivedPrepare(const QCborMap &data)
{
	if (!m_game)
		return;

	QCborArray enemyList = data.value(QStringLiteral("ee")).toArray();
	QCborArray playerList = data.value(QStringLiteral("pp")).toArray();


	updateSnapshotPlayerList(playerList);
	updateSnapshotEnemyList(enemyList);

}


/**
 * @brief RpgUdpEngine::packetReceivedPlay
 * @param data
 */

void RpgUdpEngine::packetReceivedPlay(const QCborMap &data)
{
	if (!m_game)
		return;

	const qint64 tick = data.value(QStringLiteral("t")).toInteger(-1);

	/*if (tick > -1)
		m_game->setTickTimer(tick);*/

	QCborArray enemyList = data.value(QStringLiteral("ee")).toArray();
	QCborArray playerList = data.value(QStringLiteral("pp")).toArray();

	updateSnapshotPlayerList(playerList);
	updateSnapshotEnemyList(enemyList);
}



/**
 * @brief RpgUdpEngine::gameState
 * @return
 */

RpgConfig::GameState RpgUdpEngine::gameState() const
{
	return m_gameState;
}

void RpgUdpEngine::setGameState(const RpgConfig::GameState &newGameState)
{
	m_gameState = newGameState;
}










/**
 * @brief RpgUdpEngine::getPlayerList
 * @param tick
 * @return
 */

QVariantList RpgUdpEngine::getPlayerList()
{
	QMutexLocker locker(&m_mutex);

	QVariantList list;

	for (const RpgGameData::CharacterSelect &p : m_playerData) {
		list.append(p.toJson().toVariantMap());
	}

	return list;
}





/**
 * @brief RpgUdpEngine::updateSnapshotEnemyList
 * @param list
 * @param tick
 */

void RpgUdpEngine::updateSnapshotEnemyList(const QCborArray &list)
{
	QMutexLocker locker(&m_mutex);

	/*for (const QCborValue &v : list) {
		RpgGameData::Enemy enemy;
		enemy.fromCbor(v);

		if (enemy.s < 0 || enemy.id < 0) {
			LOG_CERROR("game") << "Invalid enemy id" << enemy.s << enemy.id;
			continue;
		}

		updateSnapshot(enemy);
	}*/
}



/**
 * @brief RpgUdpEngine::updateSnapshotPlayerList
 * @param list
 * @param tick
 */

void RpgUdpEngine::updateSnapshotPlayerList(const QCborArray &list)
{
	QMutexLocker locker(&m_mutex);

	for (const QCborValue &v : list) {
		const QCborMap &m = v.toMap();
		const QCborValue &p = m.value(QStringLiteral("p"));
		const QCborValue &pd = m.value(QStringLiteral("pd"));

		RpgGameData::Player player;
		player.fromCbor(p);

		RpgGameData::BaseData playerData;
		playerData.fromCbor(pd);

		if (playerData.o < 0) {
			LOG_CERROR("game") << "Invalid player id" << v;
			continue;
		}

		m_snapshots.updateSnapshot(playerData, player);
	}
}






/**
 * @brief ActionRpgMultiplayerGame::connectToHost
 */

void RpgUdpEngine::connectToServer(Server *server)
{
	Q_ASSERT(server);

	setUrl(server->url());
}




/**
 * @brief ClientStorage::updateSnapshot
 * @param player
 */

void ClientStorage::updateSnapshot(const RpgGameData::CharacterSelect &player)
{
	QMutexLocker locker(&m_mutex);

	const auto pIt = std::find_if(m_players.cbegin(),
								  m_players.cend(),
								  [&player](const auto &key) {
		return key.data.o == player.playerId;
	});


	RpgGameData::Player pData;

	if (pIt == m_players.cend()) {
		RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::BaseData> d;
		d.data.o = player.playerId;
		m_players.push_back(d);
	} /*else {
		auto ptr = pIt->list;
		ptr[0] = pData;
	}*/
}





/**
 * @brief ClientStorage::updateSnapshot
 * @param playerData
 * @param player
 */

void ClientStorage::updateSnapshot(const RpgGameData::BaseData &playerData, const RpgGameData::Player &player)
{
	QMutexLocker locker(&m_mutex);

	auto it = std::find_if(m_players.begin(),
						   m_players.end(),
						   [&playerData](const auto &p) {
		return (p.data.o == playerData.o);
	});

	if (it == m_players.end()) {
		LOG_CERROR("game") << "Invalid player" << playerData.o;
		return;
	}

	it->data.s = playerData.s;
	it->data.id = playerData.id;

	if (player.f < 0)
		LOG_CDEBUG("game") << "SKIP FRAME" << player.f << player.p;
	else
		it->list.insert_or_assign(player.f, player);
}


