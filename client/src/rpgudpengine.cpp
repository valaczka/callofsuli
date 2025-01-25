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

	for (const QCborValue &v : pList) {
		RpgGameData::Player p;
		p.fromCbor(v.toMap());
		updateSnapshot(p, 0);
	}

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


	updateSnapshotPlayerList(playerList, 0);
	updateSnapshotEnemyList(enemyList, 0);

}


/**
 * @brief RpgUdpEngine::packetReceivedPlay
 * @param data
 */

void RpgUdpEngine::packetReceivedPlay(const QCborMap &data)
{
	if (!m_game)
		return;

	if (const qint64 tick = data.value(QStringLiteral("t")).toInteger(-1); tick > -1)
		m_game->setTickTimer(tick);

	QCborArray enemyList = data.value(QStringLiteral("ee")).toArray();
	QCborArray playerList = data.value(QStringLiteral("pp")).toArray();

	updateSnapshotPlayerList(playerList, 0);
	updateSnapshotEnemyList(enemyList, 0);
}

RpgConfig::GameState RpgUdpEngine::gameState() const
{
	return m_gameState;
}

void RpgUdpEngine::setGameState(const RpgConfig::GameState &newGameState)
{
	m_gameState = newGameState;
}







/**
 * @brief RpgUdpEngine::updateSnapshot
 * @param player
 */

void RpgUdpEngine::updateSnapshot(const RpgGameData::CharacterSelect &player)
{
	QMutexLocker locker(&m_mutex);

	if (!m_snapshots.empty() && m_snapshots.cbegin()->first != 0)
		m_snapshots.clear();

	const auto it = std::find_if(m_playerData.begin(),
								 m_playerData.end(),
								 [&player](const RpgGameData::CharacterSelect &ch){
		return ch.playerId == player.playerId;
	});

	if (it == m_playerData.cend())
		m_playerData.emplace_back(player);
	else
		*it = player;

	RpgGameData::Player pData;

	pData.o = player.playerId;

	if (m_snapshots.empty())
		m_snapshots.insert_or_assign(0, SnapshotPrivate{
								  .isAuth = true,
								  .players = { pData },
								  .enemies = {},
							  });
	else{
		updateSnapshot(m_snapshots.begin()->second, pData);
	}
}





/**
 * @brief RpgUdpEngine::updateSnapshot
 * @param snapshot
 * @param player
 */

void RpgUdpEngine::updateSnapshot(SnapshotPrivate &snapshot, const RpgGameData::Player &player)
{
	QMutexLocker locker(&m_mutex);

	auto it = std::find_if(snapshot.players.begin(),
						   snapshot.players.end(),
						   [&player](const RpgGameData::Player &p) {
		return p.o == player.o;
	});

	if (it == snapshot.players.end()) {
		LOG_CERROR("game") << "ADD PLAYER" << player.o << player.s;
		snapshot.players.emplace_back(player);
	} else
		*it = player;
}





/**
 * @brief RpgUdpEngine::updateSnapshot
 * @param snapshot
 * @param enemy
 */

void RpgUdpEngine::updateSnapshot(SnapshotPrivate &snapshot, const RpgGameData::Enemy &enemy)
{
	QMutexLocker locker(&m_mutex);

	auto it = std::find_if(snapshot.enemies.begin(),
						   snapshot.enemies.end(),
						   [&enemy](const RpgGameData::Enemy &p) {
		return p.RpgGameData::Body::isEqual(enemy);
	});

	if (it == snapshot.enemies.end())
		snapshot.enemies.emplace_back(enemy);
	else
		*it = enemy;
}





/**
 * @brief RpgUdpEngine::updateSnapshot
 * @param player
 * @param tick
 */

void RpgUdpEngine::updateSnapshot(const RpgGameData::Player &player, const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	const auto it = m_snapshots.find(tick);

	if (it == m_snapshots.end()) {
		LOG_CERROR("game") << "ADD TICK" << tick;
		m_snapshots.insert_or_assign(tick, SnapshotPrivate{
								  .isAuth = true,
								  .players = { player },
								  .enemies = {},
							  });
	} else {
		updateSnapshot(it->second, player);
	}
}



/**
 * @brief RpgUdpEngine::updateSnapshot
 * @param player
 * @param tick
 */

void RpgUdpEngine::updateSnapshot(const RpgGameData::Enemy &enemy, const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	const auto it = m_snapshots.find(tick);

	if (it == m_snapshots.end()) {
		LOG_CERROR("game") << "ADD TICK" << tick;

		m_snapshots.insert_or_assign(tick, SnapshotPrivate{
								  .isAuth = true,
								  .players = { },
								  .enemies = { enemy },
							  });
	} else {
		updateSnapshot(it->second, enemy);
	}
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

void RpgUdpEngine::updateSnapshotEnemyList(const QCborArray &list, const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	for (const QCborValue &v : list) {
		RpgGameData::Enemy enemy;
		enemy.fromCbor(v);

		if (enemy.s < 0 || enemy.id < 0) {
			LOG_CERROR("game") << "Invalid enemy id" << enemy.s << enemy.id;
			continue;
		}

		updateSnapshot(enemy, tick);
	}
}



/**
 * @brief RpgUdpEngine::updateSnapshotPlayerList
 * @param list
 * @param tick
 */

void RpgUdpEngine::updateSnapshotPlayerList(const QCborArray &list, const qint64 &tick)
{
	QMutexLocker locker(&m_mutex);

	for (const QCborValue &v : list) {
		RpgGameData::Player player;
		player.fromCbor(v);

		if (player.o < 0) {
			LOG_CERROR("game") << "Invalid player id" << v;
			continue;
		}

		updateSnapshot(player, tick);
	}
}


/**
 * @brief RpgUdpEngine::updatePlayer
 * @param snapshot
 * @param player
 */

void RpgUdpEngine::updatePlayer(const Snapshot &snapshot, RpgPlayer *player, const int &owner)
{
	Q_ASSERT(player);

	QMutexLocker locker(&m_mutex);

	const auto it = std::find_if(snapshot.players.cbegin(),
								 snapshot.players.cend(),
								 [player, owner](const RpgGameData::Player &p) {
		return (p.o == owner && p.s == player->objectId().sceneId && p.id == player->objectId().id);
	});

	if (it == snapshot.players.cend()) {
		LOG_CERROR("game") << "Invalid player" << player;
		return;
	}

	if (it->p.size() > 1)
		player->emplace(it->p.at(0), it->p.at(1));

	player->setCurrentAngle(it->a);
}


/**
 * @brief RpgUdpEngine::updateEnemy
 * @param snapshot
 * @param enemy
 */

void RpgUdpEngine::updateEnemy(const Snapshot &snapshot, IsometricEnemy *enemy)
{
	Q_ASSERT(enemy);

	QMutexLocker locker(&m_mutex);

	const auto it = std::find_if(snapshot.enemies.cbegin(),
								 snapshot.enemies.cend(),
								 [enemy](const RpgGameData::Enemy &p) {
		return (p.s == enemy->objectId().sceneId && p.id == enemy->objectId().id);
	});

	if (it == snapshot.enemies.cend()) {
		LOG_CERROR("game") << "Invalid enemy" << enemy;
		return;
	}

	if (it->p.size() > 1)
		enemy->emplace(it->p.at(0), it->p.at(1));

	enemy->setCurrentAngle(it->a);
}


/**
 * @brief RpgUdpEngine::getCurrentSnapshot
 * @return
 */

RpgUdpEngine::Snapshot RpgUdpEngine::getCurrentSnapshot()
{
	QMutexLocker locker(&m_mutex);

	Snapshot s;

	auto it = m_snapshots.cbegin();

	if (it == m_snapshots.cend())
		return s;

	s.tick = it->first;
	s.enemies = it->second.enemies;
	s.players = it->second.players;

	return s;
}







/**
 * @brief ActionRpgMultiplayerGame::connectToHost
 */

void RpgUdpEngine::connectToServer(Server *server)
{
	Q_ASSERT(server);

	setUrl(server->url());
}

