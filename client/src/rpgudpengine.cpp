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
#include "qbuffer.h"
#include "actionrpgmultiplayergame.h"
#include "server.h"



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
 * @brief RpgUdpEngine::updateSnapshotRemoveMissing
 * @param list
 */

template<typename T, typename T2>
void RpgUdpEngine::updateSnapshotRemoveMissing(const RpgGameData::SnapshotList<T, T2> &list)
{
	QMutexLocker locker(&m_snapshotMutex);

	std::vector<T2> existing;

	for (const auto &ptr : list) {
		for (const auto &p : ptr.list) {
			m_snapshots.updateSnapshot(ptr.data, p.second);
		}
		existing.push_back(ptr.data);
	}

	m_snapshots.removeMissingSnapshots(existing);
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
	QMutexLocker locker(&m_snapshotMutex);

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
 * @brief RpgUdpEngine::updateSnapshot
 * @param snapshot
 */

void RpgUdpEngine::updateSnapshot(const RpgGameData::CurrentSnapshot &snapshot)
{
	QMutexLocker locker(&m_snapshotMutex);

	for (const auto &ptr : snapshot.players) {
		for (const auto &p : ptr.list) {
			m_snapshots.updateSnapshot(ptr.data, p.second);
		}
	}

	for (const auto &ptr : snapshot.enemies) {
		for (const auto &p : ptr.list) {
			m_snapshots.updateSnapshot(ptr.data, p.second);
		}
	}

	updateSnapshotRemoveMissing(snapshot.bullets);

}






/**
 * @brief RpgUdpEngine::packetReceivedChrSel
 * @param data
 */

void RpgUdpEngine::packetReceivedChrSel(const QCborMap &data)
{
	if (!m_game)
		return;

	RpgGameData::CharacterSelectServer config;

	config.fromCbor(data);

	for (const RpgGameData::CharacterSelect &ch : config.players)
		updateSnapshot(ch);


	if (m_game->gameMode() == ActionRpgGame::MultiPlayerGuest)
		m_gameConfig = config.gameConfig;
}




/**
 * @brief RpgUdpEngine::packetReceivedDownload
 * @param data
 */

void RpgUdpEngine::packetReceivedDownload(const QCborMap &data)
{
	if (!m_game)
		return;

	if (m_downloadContentStarted)
		return;

	RpgGameData::Prepare config;
	config.fromCbor(data);

	if (config.gameConfig.terrain.isEmpty()) {
		LOG_CERROR("game") << "Missing game config terrain";
		emit gameError();
		return;
	}

	m_gameConfig = config.gameConfig;

	RpgGameData::CurrentSnapshot snapshot;
	snapshot.fromCbor(data);

	updateSnapshot(snapshot);

	QMutexLocker locker(&m_snapshotMutex);
	emit gameDataDownload(config.gameConfig.terrain, m_playerData);

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

	RpgGameData::Prepare config;
	config.fromCbor(data);

	RpgGameData::CurrentSnapshot snapshot;
	snapshot.fromCbor(data);

	updateSnapshot(snapshot);
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

	if (tick > -1)
		m_game->setTickTimer(tick);

	RpgGameData::CurrentSnapshot snapshot;
	snapshot.fromCbor(data);

	updateSnapshot(snapshot);
}




/**
 * @brief RpgUdpEngine::gameState
 * @return
 */

const RpgConfig::GameState &RpgUdpEngine::gameState() const
{
	return m_gameState;
}

void RpgUdpEngine::setGameState(const RpgConfig::GameState &newGameState)
{
	m_gameState = newGameState;
}


/**
 * @brief RpgUdpEngine::snapshots
 * @return
 */

ClientStorage RpgUdpEngine::snapshots()
{
	QMutexLocker locker(&m_snapshotMutex);
	return m_snapshots;
}



/**
 * @brief RpgUdpEngine::zapSnapshots
 * @param tick
 */

void RpgUdpEngine::zapSnapshots(const qint64 &tick)
{
	QMutexLocker locker(&m_snapshotMutex);
	m_snapshots.zapSnapshots(tick);
}


/**
 * @brief RpgUdpEngine::getFullSnapshot
 * @param tick
 * @param findLast
 * @return
 */

RpgGameData::FullSnapshot RpgUdpEngine::getFullSnapshot(const qint64 &tick, const bool &findLast)
{
	QMutexLocker locker(&m_snapshotMutex);
	return m_snapshots.getFullSnapshot(tick, findLast);
}



/**
 * @brief RpgUdpEngine::getNextFullSnapshot
 * @param tick
 * @return
 */

RpgGameData::FullSnapshot RpgUdpEngine::getNextFullSnapshot(const qint64 &tick)
{
	QMutexLocker locker(&m_snapshotMutex);
	return m_snapshots.getNextFullSnapshot(tick);
}



/**
 * @brief RpgUdpEngine::getCurrentSnapshot
 * @return
 */

RpgGameData::CurrentSnapshot RpgUdpEngine::getCurrentSnapshot()
{
	QMutexLocker locker(&m_snapshotMutex);
	return m_snapshots.getCurrentSnapshot();
}






/**
 * @brief RpgUdpEngine::binaryDataReceived
 * @param list
 * @param rtt
 */

void RpgUdpEngine::binaryDataReceived(const QList<QPair<QByteArray, unsigned int> > &list)
{
	if (!m_game)
		return;

	for (const auto &ptr : list) {
		m_game->addLatency(ptr.second/2);

		if (m_game->config().gameState == RpgConfig::StateError || m_gameState == RpgConfig::StateError)
			return;

		const QCborMap &cbor = QCborValue::fromCbor(ptr.first).toMap();

		updateState(cbor);

		if (m_gameState == RpgConfig::StateConnect)
			return;
		else if (m_gameState == RpgConfig::StateCharacterSelect)
			packetReceivedChrSel(cbor);
		else if (m_gameState == RpgConfig::StatePrepare) {
			if (m_game->config().gameState == RpgConfig::StateDownloadContent)
				packetReceivedDownload(cbor);
			else if (m_game->config().gameState == RpgConfig::StatePrepare)
				packetReceivedPrepare(cbor);
		} else if (m_gameState == RpgConfig::StatePlay) {

			if (const qint64 tick = cbor.value(QStringLiteral("t")).toInteger(-1); tick > -1) {
				QMutexLocker locker(&m_snapshotMutex);
				m_snapshots.setServerTick(tick);
			}

			packetReceivedPlay(cbor);
		}
	}
}










/**
 * @brief RpgUdpEngine::getPlayerList
 * @param tick
 * @return
 */

QVariantList RpgUdpEngine::getPlayerList()
{
	QMutexLocker locker(&m_snapshotMutex);

	QVariantList list;

	for (const RpgGameData::CharacterSelect &p : m_playerData) {
		list.append(p.toJson().toVariantMap());
	}

	return list;
}



/**
 * @brief RpgUdpEngine::playerData
 * @return
 */

QList<RpgGameData::CharacterSelect> RpgUdpEngine::playerData()
{
	QMutexLocker locker(&m_snapshotMutex);
	return m_playerData;
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
	m_isHost = newIsHost;
}

int RpgUdpEngine::playerId() const
{
	return m_playerId;
}

void RpgUdpEngine::setPlayerId(int newPlayerId)
{
	m_playerId = newPlayerId;
}

const RpgGameData::GameConfig &RpgUdpEngine::gameConfig() const
{
	return m_gameConfig;
}

void RpgUdpEngine::setGameConfig(const RpgGameData::GameConfig &newGameConfig)
{
	m_gameConfig = newGameConfig;
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
	const auto pIt = std::find_if(m_players.cbegin(),
								  m_players.cend(),
								  [&player](const auto &key) {
		return key.data.o == player.playerId;
	});


	RpgGameData::Player pData;

	if (pIt == m_players.cend()) {
		RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> d;
		d.data.o = player.playerId;
		m_players.push_back(d);
	}
}





/**
 * @brief ClientStorage::updateSnapshot
 * @param playerData
 * @param player
 */

void ClientStorage::updateSnapshot(const RpgGameData::PlayerBaseData &playerData, const RpgGameData::Player &player)
{
	auto it = std::find_if(m_players.begin(),
						   m_players.end(),
						   [&playerData](const auto &p) {
		return p.data.o == playerData.o;							// Csak ownert keresünk, mert a többi még nincs beállítva
	});

	if (it == m_players.end()) {
		LOG_CERROR("game") << "Invalid player" << playerData.o;
		return;
	}

	it->data.s = playerData.s;										// Itt állítjuk be
	it->data.id = playerData.id;


	if (player.f < 0)
		LOG_CDEBUG("game") << "SKIP FRAME" << player.f << player.p;
	else
		it->list.insert_or_assign(player.f, player);
}




/**
 * @brief ClientStorage::updateSnapshot
 * @param enemyData
 * @param enemy
 */

void ClientStorage::updateSnapshot(const RpgGameData::EnemyBaseData &enemyData, const RpgGameData::Enemy &enemy)
{
	auto it = std::find_if(m_enemies.begin(),
						   m_enemies.end(),
						   [&enemyData](const auto &p) {
		return (p.data.RpgGameData::BaseData::isEqual(enemyData));
	});

	if (it == m_enemies.end()) {
		LOG_CINFO("game") << "New enemy" << enemyData.o << enemyData.s << enemyData.id;
		m_enemies.push_back({
								.data = enemyData,
								.list = {}
							});
		it = m_enemies.end()-1;
	}

	it->data = enemyData;

	if (enemy.f < 0)
		LOG_CDEBUG("game") << "SKIP FRAME" << enemy.f << enemy.p;
	else
		it->list.insert_or_assign(enemy.f, enemy);
}





/**
 * @brief ClientStorage::updateSnapshot
 * @param bulletData
 * @param bullet
 */

void ClientStorage::updateSnapshot(const RpgGameData::BulletBaseData &bulletData, const RpgGameData::Bullet &bullet)
{
	auto it = std::find_if(m_bullets.begin(),
						   m_bullets.end(),
						   [&bulletData](const auto &p) {
		return (p.data.RpgGameData::BaseData::isEqual(bulletData));
	});

	if (it == m_bullets.end()) {
		LOG_CINFO("game") << "New bullet" << bulletData.o << bulletData.s << bulletData.id;
		m_bullets.push_back({
								.data = bulletData,
								.list = {}
							});
		it = m_bullets.end()-1;
	}

	it->data = bulletData;

	if (bullet.f < 0)
		LOG_CDEBUG("game") << "SKIP FRAME" << bullet.f << bullet.p;
	else
		it->list.insert_or_assign(bullet.f, bullet);
}



/**
 * @brief ClientStorage::hasSnapshot
 * @return
 */

bool ClientStorage::hasSnapshot()
{
	return !m_players.empty() || !m_enemies.empty() || !m_bullets.empty();
}



/**
 * @brief ClientStorage::renderCurrentSnapshot
 * @return
 */

RpgGameData::CurrentSnapshot ClientStorage::renderCurrentSnapshot()
{
	RpgGameData::CurrentSnapshot snapshot = getCurrentSnapshot();
	updateLastTick(snapshot);

	return snapshot;
}


/**
 * @brief ClientStorage::updateLastTick
 * @param snapshot
 */

void ClientStorage::updateLastTick(const RpgGameData::CurrentSnapshot &snapshot)
{
	for (const auto &ptr : snapshot.players) {
		for (const auto &l : ptr.list) {
			if (l.second.f > m_lastPlayerTick)
				m_lastPlayerTick = l.second.f;
		}
	}

	for (const auto &ptr : snapshot.enemies) {
		for (const auto &l : ptr.list) {
			if (l.second.f > m_lastEnemyTick)
				m_lastEnemyTick = l.second.f;
		}
	}

	for (const auto &ptr : snapshot.bullets) {
		for (const auto &l : ptr.list) {
			if (l.second.f > m_lastBulletTick)
				m_lastBulletTick = l.second.f;
		}
	}
}


/**
 * @brief ClientStorage::setServerTick
 * @param newServerTick
 */

void ClientStorage::setServerTick(const qint64 &newServerTick)
{
	m_serverTick = newServerTick;
}


/**
 * @brief ClientStorage::serverTick
 * @return
 */

const qint64 &ClientStorage::serverTick() const
{
	return m_serverTick;
}



/**
 * @brief ClientStorage::removeMissingSnapshots
 * @param bulletList
 */

void ClientStorage::removeMissingSnapshots(const std::vector<RpgGameData::BulletBaseData> &bulletList)
{
	removeMissing(m_bullets, bulletList);
}




/**
 * @brief ClientStorage::clear
 */

void ClientStorage::clear()
{
	m_players.clear();
	m_enemies.clear();
	m_bullets.clear();
}



/**
 * @brief ClientStorage::dump
 * @return
 */

QByteArray ClientStorage::dump()
{
	QByteArray data;
	QBuffer buf(&data);
	buf.open(QIODevice::WriteOnly);

	dumpSnapshots(&buf, m_players);

	buf.close();
	return data;
}


