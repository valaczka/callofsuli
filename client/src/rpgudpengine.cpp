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

	m_game->addLatency(rtt/2);

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



#ifdef WITH_FTXUI
#include "desktopapplication.h"
#endif


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

	QMutexLocker locker(&m_mutex);

	const qint64 tick = data.value(QStringLiteral("t")).toInteger(-1);

	if (tick > -1)
		m_game->setTickTimer(tick);

	QCborArray enemyList = data.value(QStringLiteral("ee")).toArray();
	QCborArray playerList = data.value(QStringLiteral("pp")).toArray();
	QCborArray bulletList = data.value(QStringLiteral("bb")).toArray();

	updateSnapshotPlayerList(playerList);
	updateSnapshotEnemyList(enemyList);
	updateSnapshotBulletList(bulletList);

#ifdef WITH_FTXUI
	if (DesktopApplication *a = dynamic_cast<DesktopApplication*>(Application::instance())) {
		QCborMap map;
		QString txt;

		/*		map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));


		for (const QCborValue &v : playerList) {
			const QCborMap &m = v.toMap();
			const QCborValue &pd = m.value(QStringLiteral("pd"));
			const QCborArray &p = m.value(QStringLiteral("p")).toArray();

			RpgGameData::PlayerBaseData playerData;
			playerData.fromCbor(pd);

			if (playerData.o < 0 ) {		// || playerData.o == m_playerId
				continue;
			}

			txt += QStringLiteral("PLAYER %1\n==============================================\n").arg(playerData.o);


			RpgGameData::Player player;
			QList<RpgGameData::Player> pl;

			for (const QCborValue &v : p) {
				player.fromCbor(v);

				pl.append(player);
			}

			for (auto it = pl.crbegin(); it != pl.crend(); ++it) {
				txt += QStringLiteral("%1: %8 %2 (%3, %4) -> (%5, %6) - %7  || %9\n")
					   .arg(it->f)
					   .arg(it->st)
					   .arg(it->p.size() > 1 ? it->p.at(0) : -1)
					   .arg(it->p.size() > 1 ? it->p.at(1) : -1)
					   .arg(it->cv.size() > 1 ? it->cv.at(0) : -1)
					   .arg(it->cv.size() > 1 ? it->cv.at(1) : -1)
					   .arg(it->a)
					   .arg(it->f)
					   .arg(it->arm.cw)
					   ;
			}

			txt += QStringLiteral("\n\n");
		}

		map.insert(QStringLiteral("txt"), txt);
		a->writeToSocket(map.toCborValue());




		txt.clear();
		map.clear();*/

		map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));

		txt = QStringLiteral("CURRENT TICK %1 - server %2\n\n").arg(m_game->rpgGame()->tickTimer()->currentTick()).arg(tick);

		for (const auto &ptr : m_snapshots.players()) {
			txt += QStringLiteral("STORAGE PLAYER %1\n==============================================\n").arg(ptr.data.o);

			for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
				txt += QStringLiteral("%1: %2 (%3, %4) %5  || %6\n")
					   .arg(it->second.f)
					   .arg(it->second.st)
					   .arg(it->second.p.size() > 1 ? it->second.p.at(0) : -1)
					   .arg(it->second.p.size() > 1 ? it->second.p.at(1) : -1)
					   .arg(it->second.a)
					   .arg(it->second.arm.cw)
					   ;
			}

			txt += QStringLiteral("\n\n");
		}



		for (const auto &ptr : m_snapshots.enemies()) {
			txt += QStringLiteral("STORAGE ENEMY %1 %2 %3\n==============================================\n")
				   .arg(ptr.data.o)
				   .arg(ptr.data.s)
				   .arg(ptr.data.id);

			for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
				txt += QStringLiteral("%1: %2 (%3, %4) %5  || %6\n")
					   .arg(it->second.f)
					   .arg(it->second.st)
					   .arg(it->second.p.size() > 1 ? it->second.p.at(0) : -1)
					   .arg(it->second.p.size() > 1 ? it->second.p.at(1) : -1)
					   .arg(it->second.a)
					   .arg(it->second.arm.cw)
					   ;
			}

			txt += QStringLiteral("\n\n");
		}

		map.insert(QStringLiteral("txt"), txt);
		a->writeToSocket(map.toCborValue());
	}
#endif

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
 * @brief RpgUdpEngine::getPlayerData
 * @return
 */

QList<RpgGameData::CharacterSelect> RpgUdpEngine::getPlayerData()
{
	QMutexLocker locker(&m_mutex);
	return m_playerData;
}





/**
 * @brief RpgUdpEngine::updateSnapshotEnemyList
 * @param list
 * @param tick
 */

void RpgUdpEngine::updateSnapshotEnemyList(const QCborArray &list)
{
	QMutexLocker locker(&m_mutex);

	for (const QCborValue &v : list) {
		const QCborMap &m = v.toMap();
		const QCborValue &ed = m.value(QStringLiteral("ed"));
		const QCborArray &e = m.value(QStringLiteral("e")).toArray();

		RpgGameData::EnemyBaseData enemyData;
		enemyData.fromCbor(ed);

		if (enemyData.s < 0 || enemyData.id < 0) {
			LOG_CERROR("game") << "Invalid enemy id" << enemyData.s << enemyData.id;
			continue;
		}

		RpgGameData::Enemy enemy;

		for (const QCborValue &v : e) {
			enemy.fromCbor(v);
			m_snapshots.updateSnapshot(enemyData, enemy);
		}
	}
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
		const QCborValue &pd = m.value(QStringLiteral("pd"));
		const QCborArray &p = m.value(QStringLiteral("p")).toArray();

		RpgGameData::PlayerBaseData playerData;
		playerData.fromCbor(pd);

		if (playerData.o < 0) {
			LOG_CERROR("game") << "Invalid player id" << v;
			continue;
		}

		RpgGameData::Player player;

		for (const QCborValue &v : p) {
			player.fromCbor(v);
			m_snapshots.updateSnapshot(playerData, player);
		}
	}
}





/**
 * @brief RpgUdpEngine::updateSnapshotBulletList
 * @param list
 */

void RpgUdpEngine::updateSnapshotBulletList(const QCborArray &list)
{
	QMutexLocker locker(&m_mutex);

	std::vector<RpgGameData::BulletBaseData> existingBullets;

	for (const QCborValue &v : list) {
		const QCborMap &m = v.toMap();
		const QCborValue &ed = m.value(QStringLiteral("bd"));
		const QCborArray &e = m.value(QStringLiteral("b")).toArray();

		RpgGameData::BulletBaseData bulletData;
		bulletData.fromCbor(ed);

		if (bulletData.s < 0 || bulletData.id < 0) {
			LOG_CERROR("game") << "Invalid bullet id" << bulletData.o << bulletData.s << bulletData.id;
			continue;
		}

		RpgGameData::Bullet bullet;

		for (const QCborValue &v : e) {
			bullet.fromCbor(v);
			m_snapshots.updateSnapshot(bulletData, bullet);
		}

		existingBullets.push_back(bulletData);
	}

	// Remove missing bullets

	m_snapshots.removeMissingSnapshots(existingBullets);

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
		RpgGameData::SnapshotData<RpgGameData::Player, RpgGameData::PlayerBaseData> d;
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

void ClientStorage::updateSnapshot(const RpgGameData::PlayerBaseData &playerData, const RpgGameData::Player &player)
{
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);
	return !m_players.empty() || !m_enemies.empty() || !m_bullets.empty();
}



/**
 * @brief ClientStorage::renderCurrentSnapshot
 * @return
 */

RpgGameData::CurrentSnapshot ClientStorage::renderCurrentSnapshot()
{
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);

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
	QMutexLocker locker(&m_mutex);

	QByteArray data;
	QBuffer buf(&data);
	buf.open(QIODevice::WriteOnly);

	dumpSnapshots(&buf, m_players);

	buf.close();
	return data;
}


