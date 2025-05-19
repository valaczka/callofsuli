/*
 * ---- Call of Suli ----
 *
 * actionrpgmultiplayergame.cpp
 *
 * Created on: 2025. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionRpgMultiplayerGame
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

#include "actionrpgmultiplayergame.h"
#include "rpgudpengine.h"
#include "client.h"
#include "rpgplayer.h"
#include "utils_.h"

#include <chipmunk/chipmunk.h>



/**
 * Private namespace
 */

class ActionRpgMultiplayerGamePrivate
{
private:
	ActionRpgMultiplayerGamePrivate(ActionRpgMultiplayerGame *game) : d(game) {}

	void updateBody(TiledObjectBody *body, const bool &isHosted);

private:
	template <typename T, typename T2, typename T3,
			  typename = std::enable_if<std::is_base_of<RpgGameDataInterface<T2, T3>, T>::value>::type
			  >
	void update(T *body, const bool &isHosted)
	{
		const auto &ptr = m_currentSnapshot.getSnapshot(body->baseData());

		if (!ptr) {
			if (isHosted)
				body->worldStep();
			return;
		} else {
			if (isHosted) {
				if (ptr->s1.f >= 0)
					body->updateFromLastSnapshot(ptr->s1);
				body->worldStep();
			} else {
				body->updateFromSnapshot(ptr.value());
			}
		}
	}

	RpgGameData::FullSnapshot m_currentSnapshot;

	qint64 m_lastSentTick = -1;
	ClientStorage m_toSend;

	class TimeSync {
	public:
		TimeSync() = default;

		void set(const qint64 &tick) {
#ifndef Q_OS_WASM
			QMutexLocker locker(&m_mutex);
#endif
			m_tick = tick;
			if (m_timestamp.isValid())
				m_timestamp.restart();
			else
				m_timestamp.start();
		}


		qint64 get() {
#ifndef Q_OS_WASM
			QMutexLocker locker(&m_mutex);
#endif
			if (!m_timestamp.isValid())
				return -1;

			return m_tick + AbstractGame::TickTimer::msecToTick(m_timestamp.elapsed() + getLatency());
		}


		void addLatency(const qint64 &latency) {
#ifndef Q_OS_WASM
			QMutexLocker locker(&m_mutex);
#endif
			m_latencies.append(latency);

			const int &s = m_latencies.size();

			if (const qint64 max=2*m_latencySize; (s > max)) {
				m_latencies.erase(m_latencies.cbegin(), m_latencies.cbegin()+(s-max));
			}
		}


		qint64 getLatency() {
#ifndef Q_OS_WASM
			QMutexLocker locker(&m_mutex);
#endif
			if (m_latencies.size() < m_latencySize)
				return 0;

			const qint64 &sum = std::accumulate(m_latencies.cbegin(), m_latencies.cend(), 0);

			return sum/m_latencies.size();
		}

	private:
		QElapsedTimer m_timestamp;
		qint64 m_tick = 0;
		QList<qint64> m_latencies;
		inline static const qint64 m_latencySize = 10;

#ifndef Q_OS_WASM
		QRecursiveMutex m_mutex;
#endif

	};

	TimeSync m_timeSync;

	ActionRpgMultiplayerGame *const d;



	//// TMP

	/*enum Measure {
		Invalid,
		SGsync,
		SGrender,
		Synchronize
	};

	struct MeasureData {
		qint64 min = -1;
		qint64 max = -1;
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
	}*/


	friend class ActionRpgMultiplayerGame;
};





/**
 * @brief ActionRpgMultiplayerGame::ActionRpgMultiplayerGame
 * @param missionLevel
 * @param client
 */

ActionRpgMultiplayerGame::ActionRpgMultiplayerGame(GameMapMissionLevel *missionLevel, Client *client)
	: ActionRpgGame(missionLevel, client)
	, m_playersModel(std::make_unique<QSListModel>())
	, m_engine(new RpgUdpEngine(this))
	, q(new ActionRpgMultiplayerGamePrivate(this))
{
	LOG_CDEBUG("game") << "ActionRpgMultiplayerGame constructed";

	setGameMode(MultiPlayerGuest);

	m_playersModel->setRoleNames(Utils::getRolesFromObject(RpgGameData::CharacterSelect().metaObject()));


	if (m_client->server()) {
		if (User *u = m_client->server()->user()) {
			m_playerConfig.username = u->username();
			m_playerConfig.nickname = u->fullNickName();
		}
	}

	connect(m_engine, &RpgUdpEngine::gameDataDownload, this, &ActionRpgMultiplayerGame::downloadGameData);
	connect(m_engine, &RpgUdpEngine::gameError, this, &ActionRpgMultiplayerGame::setError);

	m_keepAliveTimer.start(1000, this);
}




/**
 * @brief ActionRpgMultiplayerGame::~ActionRpgMultiplayerGame
 */

ActionRpgMultiplayerGame::~ActionRpgMultiplayerGame()
{
	delete q;
	q = nullptr;
	LOG_CDEBUG("game") << "ActionRpgMultiplayerGame destroyed";
}



/**
 * @brief ActionRpgMultiplayerGame::setRpgGame
 * @param newRpgGame
 */

void ActionRpgMultiplayerGame::setRpgGame(RpgGame *newRpgGame)
{
	if (m_rpgGame == newRpgGame)
		return;

	if (m_rpgGame) {
		setGameQuestion(nullptr);

		m_rpgGame->disconnect(this);
		this->disconnect(m_rpgGame);

		m_rpgGame->setActionRpgGame(nullptr);
	}

	m_rpgGame = newRpgGame;
	emit rpgGameChanged();

	if (m_rpgGame) {
		setGameQuestion(m_rpgGame->gameQuestion());
		m_rpgGame->setRpgQuestion(m_rpgQuestion.get());
		m_rpgGame->setActionRpgGame(this);
		connect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgMultiplayerGame::onGameSuccess, Qt::QueuedConnection);		// Azért kell, mert különbön az utolsó fegyverhasználatot nem számolja el a szerveren
		connect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgMultiplayerGame::onPlayerDead);
		connect(m_rpgGame, &RpgGame::gameLoadFailed, this, &ActionRpgMultiplayerGame::onGameLoadFailed);
	}
}



/**
 * @brief ActionRpgMultiplayerGame::rpgGameActivated
 */

void ActionRpgMultiplayerGame::rpgGameActivated()
{
	if (!m_rpgGame)
		return;

	QMetaObject::invokeMethod(this, &ActionRpgMultiplayerGame::onRpgGameActivated, Qt::QueuedConnection);


	// TMP

	/*q->renderTimerMeausure(ActionRpgMultiplayerGamePrivate::SGsync, 0);

	connect(m_client->mainWindow(), &QQuickWindow::beforeSynchronizing, this, [this](){
		q->renderTimerStart();
	}, Qt::DirectConnection);

	connect(m_client->mainWindow(), &QQuickWindow::beforeRendering, this, [this](){
		q->renderTimerMeausure(ActionRpgMultiplayerGamePrivate::SGsync);
	}, Qt::DirectConnection);

	connect(m_client->mainWindow(), &QQuickWindow::afterRendering, this, [this](){
		q->renderTimerMeausure(ActionRpgMultiplayerGamePrivate::SGrender);
	}, Qt::DirectConnection); */

	connect(qApp, &QCoreApplication::aboutToQuit, this, [this](){
		if (m_client->mainWindow())
			m_client->mainWindow()->disconnect(this);
	});
}



/**
 * @brief ActionRpgMultiplayerGame::gamePrepared
 */

void ActionRpgMultiplayerGame::gamePrepared()
{
	LOG_CWARNING("game") << "GAME PREPARED";

	m_gamePrepared = true;
}


/**
 * @brief ActionRpgMultiplayerGame::gameAbort
 */

void ActionRpgMultiplayerGame::gameAbort()
{
	disconnectFromHost();
	ActionRpgGame::gameAbort();
}


/**
 * @brief ActionRpgMultiplayerGame::msecLeft
 * @return
 */

int ActionRpgMultiplayerGame::msecLeft() const
{
	return m_rpgGame ? m_rpgGame->tickTimer()->currentTick() : 0;
}


/**
 * @brief ActionRpgMultiplayerGame::selectTerrain
 * @param terrain
 */

void ActionRpgMultiplayerGame::selectTerrain(const QString &terrain)
{
	if (terrain == m_playerConfig.terrain)
		return;

	LOG_CINFO("game") << "SELECT TERRAIN" << terrain;
	m_playerConfig.terrain = terrain;

}


/**
 * @brief ActionRpgMultiplayerGame::selectCharacter
 * @param character
 */

void ActionRpgMultiplayerGame::selectCharacter(const QString &character)
{
	if (character == m_playerConfig.character)
		return;

	LOG_CINFO("game") << "SELECT CHARACTER" << character;
	m_playerConfig.character = character;
}


/**
 * @brief ActionRpgMultiplayerGame::selectWeapons
 * @param weaponList
 */

void ActionRpgMultiplayerGame::selectWeapons(const QStringList &weaponList)
{
	if (weaponList == m_playerConfig.weapons)
		return;

	LOG_CINFO("game") << "SELECT WEAPONS" << weaponList;
	m_playerConfig.weapons = weaponList;
}



/**
 * @brief ActionRpgMultiplayerGame::disconnect
 */

void ActionRpgMultiplayerGame::disconnectFromHost()
{
	LOG_CDEBUG("game") << "Disconnect from host";
	m_engine->disconnect();
}





/**
 * @brief ActionRpgMultiplayerGame::onConfigChanged
 */

void ActionRpgMultiplayerGame::onConfigChanged()
{
	if (m_config.gameState == RpgConfig::StateError) {
		disconnectFromHost();
		return;
	}

	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (m_config.gameState == RpgConfig::StateConnect) {
		LOG_CINFO("game") << "CONNECT TO SERVER";
		m_engine->connectToServer(m_client->server());
		return;
	}

	if (m_config.gameState == RpgConfig::StatePrepare) {
		ActionRpgGame::onConfigChanged();
		return;
	}

	/*if (m_config.gameState == RpgConfig::StatePlay) {
		LOG_CINFO("game") << "!!!! stop keepalive timer";
		m_keepAliveTimer.stop();
	}

	ActionRpgGame::onConfigChanged();*/

	if (m_config.gameState == RpgConfig::StatePlay) {

		if (!m_rpgGame) {
			m_config.gameState = RpgConfig::StateError;
			updateConfig();
			return;
		}

		if (m_oldGameState != RpgConfig::StatePlay) {
			m_keepAliveTimer.stop();

			m_client->sound()->setVolumeSfx(m_tmpSoundSfxVolume);

			if (!m_rpgGame->m_gameDefinition.music.isEmpty())
				m_client->sound()->playSound(m_rpgGame->m_gameDefinition.music, Sound::MusicChannel);
		}

		if (m_othersPrepared && !m_fullyPrepared) {
			m_rpgGame->message(tr("LEVEL %1").arg(level()));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);

			m_fullyPrepared = true;

			gameStart();
			m_deadlineTick = AbstractGame::TickTimer::msecToTick(m_config.duration*1000);
			m_elapsedTick = 0;
			m_rpgGame->tickTimer()->start(this, 0);
			m_timerLeft.start();
		}

		if (!m_fullyPrepared) {
			QCborMap map;
			map.insert(QStringLiteral("full"), true);
			sendData(map.toCborValue().toCbor(), true);
		}

	}

	m_oldGameState = m_config.gameState;
}


#ifdef WITH_FTXUI
#include "desktopapplication.h"
#endif


/**
 * @brief ActionRpgMultiplayerGame::timerEvent
 */

void ActionRpgMultiplayerGame::timerEvent(QTimerEvent *)
{
	const ClientStorage &snapshots = m_engine->snapshots();

	const qint64 tick = m_rpgGame ? m_rpgGame->tickTimer()->currentTick() : -2;

	/*LOG_CDEBUG("game") << "[Benchmark] CURRENT TICK" << tick << "SERVER TICK" << snapshots.serverTick() << "--->"
					   << snapshots.serverTick()+AbstractGame::TickTimer::msecToTick(q->m_timeSync.getLatency())
					   << "Q" << q->m_timeSync.get() << "LAT" << q->m_timeSync.getLatency();*/

#ifdef WITH_FTXUI
	if (DesktopApplication *a = dynamic_cast<DesktopApplication*>(Application::instance())) {
		QCborMap map;
		QString txt;


		map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));

		txt = QStringLiteral("CURRENT TICK %1 - server %2\n\n").arg(tick).arg(snapshots.serverTick());

		for (const auto &ptr : snapshots.players()) {
			if (ptr.data.o == m_playerId)
				txt += QStringLiteral("***** [STORAGE PLAYER %1] ******\n==============================================\n").arg(ptr.data.o);
			else
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

			txt += QStringLiteral("\n \n");
		}



		for (const auto &ptr : snapshots.enemies()) {
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

			txt += QStringLiteral("\n \n");
		}


		for (const auto &ptr : snapshots.bullets()) {
			txt += QStringLiteral("STORAGE BULLET %1 %2 %3\n==============================================\n")
				   .arg(ptr.data.o)
				   .arg(ptr.data.s)
				   .arg(ptr.data.id);

			for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
				txt += QStringLiteral("%1: %2 (%3, %4) %5 %6 %7\n")
					   .arg(it->second.f)
					   .arg(it->second.st)
					   .arg(it->second.p)
					   .arg(it->second.tg.o)
					   .arg(it->second.tg.s)
					   .arg(it->second.tg.id)
					   ;
			}

			txt += QStringLiteral("\n \n");
		}

		map.insert(QStringLiteral("txt"), txt);
		a->writeToSocket(map.toCborValue());
	}
#endif


	if (m_engine->gameState() == RpgConfig::StateInvalid)
		return;

	changeGameState(m_engine->gameState());

	setGameMode(m_engine->isHost() ? ActionRpgGame::MultiPlayerHost : ActionRpgGame::MultiPlayerGuest);
	setPlayerId(m_engine->playerId());

	if (!m_engine->isHost() && !m_engine->gameConfig().terrain.isEmpty())
		worldTerrainSelect(m_engine->gameConfig().terrain, true);

	updatePlayersModel(m_engine->getPlayerList());

	if (m_config.gameState == RpgConfig::StateError) {
		//disconnectFromHost();
		return;
	}

	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		sendDataChrSel();
	else if (m_config.gameState == RpgConfig::StatePrepare)
		sendDataPrepare();


	if (tick > 0)
		m_engine->zapSnapshots(tick - 3*RPG_UDP_DELTA_TICK);

}



/**
 * @brief ActionRpgMultiplayerGame::changeGameState
 * @param state
 */

void ActionRpgMultiplayerGame::changeGameState(const RpgConfig::GameState &state)
{
	if (state == m_config.gameState)
		return;

	bool canSwitch = true;

	switch (state) {
		case RpgConfig::StateCharacterSelect:
			canSwitch = m_config.gameState == RpgConfig::StateConnect;
			break;

		case RpgConfig::StatePrepare:
			if (m_config.gameState == RpgConfig::StateCharacterSelect) {
				m_config.gameState = RpgConfig::StateDownloadContent;
				updateConfig();
				return;
			} else if (m_config.gameState == RpgConfig::StateDownloadContent) {
				return;
			} else {
				canSwitch = false;
			}
			break;

		case RpgConfig::StatePlay:
			canSwitch = m_config.gameState == RpgConfig::StatePrepare;
			break;

		case RpgConfig::StateConnect:
		case RpgConfig::StateDownloadStatic:
		case RpgConfig::StateDownloadContent:
		case RpgConfig::StateFinished:
		case RpgConfig::StateError:
		case RpgConfig::StateInvalid:
			break;
	}


	if (canSwitch) {
		LOG_CINFO("game") << "New state" << state;
		m_config.gameState = state;
		updateConfig();
	} else {
		LOG_CERROR("game") << "State conflict" << m_config.gameState << "->" << state;
		setError();
	}
}


/**
 * @brief ActionRpgMultiplayerGame::worldTerrainSelect
 * @param map
 * @param forced
 */

void ActionRpgMultiplayerGame::worldTerrainSelect(QString map, const bool forced)
{
	if (m_playerConfig.terrain == map)
		return;

	m_playerConfig.terrain = map;

	RpgUserWorld *world = m_client->server() && m_client->server()->user() ? m_client->server()->user()->wallet()->world() : nullptr;

	if (!world)
		return;

	if (RpgWorldLandData *land = m_client->server()->user()->wallet()->world()->selectedLand()) {
		if (land->bindedMap() == map)
			return;
	}

	world->select(map, forced);
}



/**
 * @brief ActionRpgMultiplayerGame::updatePlayersModel
 * @param list
 */

void ActionRpgMultiplayerGame::updatePlayersModel(const QVariantList &list)
{
	Utils::patchSListModel(m_playersModel.get(), list, QStringLiteral("playerId"));
}




/**
 * @brief ActionRpgMultiplayerGame::syncEnemyList
 * @param list
 */


void ActionRpgMultiplayerGame::syncEnemyList(const ClientStorage &storage)
{
	if (!m_rpgGame || storage.enemies().empty())
		return;

	if (m_enemiesSynced && (qsizetype) storage.enemies().size() == m_rpgGame->m_enemyDataList.size())
		return;

	m_enemiesSynced = false;

	QList<RpgGameData::EnemyBaseData> onlyServerEnemy;

	QList<int> localIndices;

	for (int i=0; i<m_rpgGame->m_enemyDataList.size(); ++i)
		localIndices.append(i);

	for (const auto &pe : storage.enemies()) {
		const auto it = std::find_if(m_rpgGame->m_enemyDataList.cbegin(),
									 m_rpgGame->m_enemyDataList.cend(),
									 [&pe](const RpgGame::EnemyData &d){
			return d.enemy && d.objectId.sceneId == pe.data.s && d.objectId.id == pe.data.id;
		});

		if (it == m_rpgGame->m_enemyDataList.cend()) {
			LOG_CERROR("game") << "Missing enemy" << pe.data.s << pe.data.id;
			onlyServerEnemy.append(pe.data);
			// TODO: create enemy...
		} else {
			localIndices.removeAll(it - m_rpgGame->m_enemyDataList.cbegin());
		}
	}

	if (!localIndices.empty())
		LOG_CWARNING("game") << "Only local enemies" << localIndices;

	LOG_CDEBUG("game") << "ENEMIES SYNCED";

	m_enemiesSynced = onlyServerEnemy.isEmpty() && localIndices.isEmpty();
}





/**
 * @brief ActionRpgMultiplayerGame::syncPlayerList
 * @param list
 */

void ActionRpgMultiplayerGame::syncPlayerList(const ClientStorage &storage)
{
	if (!m_rpgGame)
		return;

	if (m_playersSynced && (qsizetype) storage.players().size() == m_rpgGame->m_players.size())
		return;

	m_playersSynced = false;

	QList<RpgPlayer*> playerList;

	for (const auto &pl : storage.players()) {
		const auto it = std::find_if(m_rpgGame->m_players.constBegin(),
									 m_rpgGame->m_players.constEnd(),
									 [&pl](const RpgPlayer *p){
			return p->objectId().sceneId == pl.data.s && p->objectId().id == pl.data.id;
		});

		if (it == m_rpgGame->m_players.constEnd()) {
			LOG_CWARNING("game") << "Missing player" << pl.data.s << pl.data.id << pl.data.o;

			RpgGameData::Player pd;

			if (pl.list.size() > 0) {
				LOG_CWARNING("game") << "LAST KEY" << std::prev(pl.list.cend())->first;
				pd = std::prev(pl.list.cend())->second;
			} else {
				LOG_CERROR("game") << "PLAYER DATA MISSING" << pl.data.o;
			}

			TiledScene *scene = m_rpgGame->findScene(pd.sc);
			if (!scene) {
				LOG_CERROR("game") << "Invalid scene" << pd.sc;
				return;
			}

			RpgPlayer *rpgPlayer = createPlayer(scene, pl.data, pd);

			if (!rpgPlayer) {
				LOG_CERROR("game") << "ERROR";
				return;
			}

			rpgPlayer->setObjectId(pl.data.o, pl.data.s, pl.data.id);

			if (pl.data.o == m_playerId) {
				m_rpgGame->setFollowedItem(rpgPlayer);
				m_rpgGame->setControlledPlayer(rpgPlayer);
				LOG_CINFO("game") << "SET CONTROLLED" << pl.data.o;
			}


			playerList.append(rpgPlayer);

		}
	}

	if (!playerList.isEmpty()) {
		playerList.append(m_rpgGame->players());
		m_rpgGame->setPlayers(playerList);
	}

	LOG_CDEBUG("game") << "SYNCED";

	m_playersSynced = !storage.players().empty();
}



/**
 * @brief ActionRpgMultiplayerGame::syncBulletList
 */

void ActionRpgMultiplayerGame::syncBulletList(const ClientStorage &storage)
{
	if (!m_rpgGame)
		return;

	QList<QPointer<RpgBullet>> bList;


	// Load all bodies

	m_rpgGame->iterateOverBodies([&bList](TiledObjectBody *b){
		if (RpgBullet *bullet = dynamic_cast<RpgBullet*>(b)) {
			bList.append(bullet);
		}
	});


	// Sync bullets

	for (const auto &b : storage.bullets()) {
		if (RpgBullet *bullet = dynamic_cast<RpgBullet*>(m_rpgGame->findBody(TiledObjectBody::ObjectId{
																			 .ownerId = b.data.o,
																			 .sceneId = b.data.s,
																			 .id = b.data.id
	}))) {
			// Remove existing bullet
			bList.removeAll(bullet);
		} else {

			// Skip my bullet

			if (b.data.o == m_playerId) {
				continue;
			}

			// Create new bullet

			TiledScene *scene = m_rpgGame->findScene(b.data.s);

			if (!scene) {
				LOG_CERROR("game") << "Invalid scene" << b.data.s;
				continue;
			}


			RpgBullet *newBullet = m_rpgGame->createBullet(b.data.t, scene, b.data.id, b.data.o, false);

			if (!newBullet) {
				LOG_CERROR("game") << "Bullet create error" << b.data.o << b.data.s << b.data.id;
				continue;
			}

			newBullet->setOwner(b.data.own);
			newBullet->setOwnerId(b.data.ownId);
			newBullet->setTargets(b.data.tar);
			newBullet->setMaxDistance(b.data.t);

			if (b.data.pth.size() > 3) {
				newBullet->shot(b.data);
			} else {
				LOG_CERROR("game") << "Bullet path missing" << b.data.o << b.data.s << b.data.id;
				continue;
			}

			continue;
		}
	}



	// Delete missing bullets

	for (RpgBullet *b : bList) {
		if (!b)
			continue;

		// Ki kell hagyni a sajátunkat, különben azonnal kitörli

		if (b->objectId().ownerId != m_playerId || b->stage() == RpgGameData::LifeCycle::StageDead) {
			b->m_stage = RpgGameData::LifeCycle::StageDestroy;
		}
	}

}




/**
 * @brief ActionRpgMultiplayerGame::playersModel
 * @return
 */

QSListModel *ActionRpgMultiplayerGame::playersModel() const
{
	return m_playersModel.get();
}


int ActionRpgMultiplayerGame::playerId() const
{
	return m_playerId;
}

void ActionRpgMultiplayerGame::setPlayerId(int newPlayerId)
{
	if (m_playerId == newPlayerId)
		return;
	m_playerId = newPlayerId;
	emit playerIdChanged();
}





/**
 * @brief ActionRpgMultiplayerGame::createPlayer
 * @param scene
 * @param config
 */

RpgPlayer* ActionRpgMultiplayerGame::createPlayer(TiledScene *scene,
												  const RpgGameData::PlayerBaseData &config,
												  const RpgGameData::Player &playerData)
{
	LOG_CINFO("game") << "CREATE PLAYER" << config.o << "vs" << m_playerId;

	if (!m_rpgGame) {
		LOG_CERROR("game") << "Missing RpgGame";
		return nullptr;
	}

	const QList<RpgGameData::CharacterSelect> pDataList = m_engine->playerData();

	const auto it = std::find_if(pDataList.cbegin(),
								 pDataList.cend(),
								 [&config](const RpgGameData::CharacterSelect &ch){
		return (ch.playerId == config.o);
	});

	if (it == pDataList.cend()) {
		LOG_CERROR("game") << "Invalid player id" << config.o;
		return nullptr;
	}

	const RpgGameData::CharacterSelect chData = *it;


	const auto characterPtr = RpgGame::characters().find(chData.character);

	if (characterPtr == RpgGame::characters().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid character" << chData.character;
		return nullptr;
	}


	RpgPlayer *player = m_rpgGame->createPlayer(scene, *characterPtr, config.o, config.o == m_playerId);

	if (!player) {
		LOG_CERROR("game") << "Player create error";
		return nullptr;
	}


	player->setMaxHp(playerData.mhp);
	player->setHp(playerData.hp);
	player->setMaxMp(characterPtr->mpMax);
	player->setMp(characterPtr->mpStart);
	loadInventory(player);


	// Set user name

	if (Server *s = m_client->server()) {
		player->setDisplayName(s->user()->fullNickName());
	}


	// From wallet

	RpgUserWalletList* wallet = m_client->server() ? m_client->server()->user()->wallet() : nullptr;

	if (wallet) {
		for (const QString &s : chData.weapons) {
			const auto it = std::find_if(wallet->constBegin(), wallet->constEnd(), [&s](RpgUserWallet *w) {
							return w->market().type == RpgMarket::Weapon && w->market().name == s;
		});
			if (it == wallet->constEnd()) {
				LOG_CERROR("game") << "Missing weapon" << s;
				continue;
			}

			if (characterPtr->disabledWeapons.contains(s)) {
				LOG_CWARNING("game") << "Weapon" << s << "disabled for character" << characterPtr->name;
				continue;
			}

			loadWeapon(player,
					   RpgArmory::weaponHash().key(s, RpgGameData::Weapon::WeaponInvalid),
					   /*(*it)->bullet() ? (*it)->bullet()->amount() : 0*/
					   (*it)->market().cost == 0 ? -1 : (*it)->amount());
		}
	}



	if (playerData.p.size() > 1) {
		player->emplace(playerData.p.at(0), playerData.p.at(1));
		LOG_CINFO("scene") << "PLAYER EMPLACED AT" << playerData.p << player;
	} else
		LOG_CWARNING("scene") << "Player position missing! ########################";

	player->setCurrentAngle(playerData.a);
	//player->setCurrentSceneStartPosition(d->m_playerPosition.value_or(QPointF{0,0}));


	connect(player->armory(), &RpgArmory::currentWeaponChanged, this, &ActionRpgMultiplayerGame::onPlayerWeaponChanged, Qt::DirectConnection);

	return player;
}



/**
 * @brief ActionRpgMultiplayerGame::onTimeStepPrepare
 */

void ActionRpgMultiplayerGame::onTimeStepPrepare()
{
	const ClientStorage &storage = m_engine->snapshots();

	syncPlayerList(storage);
	syncEnemyList(storage);
	syncBulletList(storage);


	if (m_config.gameState == RpgConfig::StatePlay && !m_fullyPrepared) {
		if (q->m_timeSync.get() > 0) {
			m_othersPrepared = true;
		}

		updateConfig();
	}


}



/**
 * @brief ActionRpgMultiplayerGame::onTimeStepped
 */

void ActionRpgMultiplayerGame::onTimeStepped()
{
	static const qint64 delta = 3;

	const qint64 tick = q->m_timeSync.get();
	const qint64 curr = m_rpgGame->tickTimer()->currentTick();
	const qint64 diff = tick-curr;

	if (diff > 2*delta || diff < -3*delta) {
		LOG_CERROR("game") << "Time reset" << curr << "->" << tick;
		m_rpgGame->tickTimer()->start(this, tick);
	} else if (diff > delta) {
		LOG_CWARNING("game") << "Time skew +1 frame" << curr << "->" << tick;
		m_rpgGame->tickTimer()->start(this, curr+1);
	} else if (diff < -2*delta) {
		LOG_CERROR("game") << "Time skew -1 frame" << curr << "->" << tick;
		m_rpgGame->tickTimer()->start(this, curr-1);
	}


	m_rpgGame->iterateOverBodies([this](TiledObjectBody *b){
		if (RpgBullet *iface = dynamic_cast<RpgBullet*> (b)) {
			if (iface->stage() == RpgGameData::LifeCycle::StageDestroy) {
				onBulletDelete(iface);
			}
		}
	});
}




/**
 * @brief ActionRpgMultiplayerGame::onTimeBeforeWorldStep
 * @param tick
 */

void ActionRpgMultiplayerGame::onTimeBeforeWorldStep(const qint64 &tick)
{
	Q_ASSERT(q);

	q->m_currentSnapshot = m_engine->getNextFullSnapshot(tick);
}




#ifdef WITH_FTXUI
#include "desktopapplication.h"
#endif


/**
 * @brief ActionRpgMultiplayerGame::onTimeAfterWorldStep
 * @param tick
 */

void ActionRpgMultiplayerGame::onTimeAfterWorldStep(const qint64 &tick)
{
	Q_ASSERT(q);

	q->m_currentSnapshot.clear();

	if (m_config.gameState != RpgConfig::StatePlay)
		return;

	const bool forceKeyFrame = q->m_lastSentTick < 0 || (tick - q->m_lastSentTick >= 15);


	bool hasSnap = q->m_toSend.hasSnapshot();			// Ha nem tesszük be a playert, nem fogja a szerver feldolgozni

	m_rpgGame->iterateOverBodies([this, &tick, &hasSnap, forceKeyFrame](TiledObjectBody *b) {
		if (m_gameMode == MultiPlayerHost) {
			if (RpgEnemy *iface = dynamic_cast<RpgEnemy*> (b)) {
				if (q->m_toSend.appendSnapshot(iface, tick, forceKeyFrame))
					hasSnap = true;
			}
		};

		if (RpgBullet *iface = dynamic_cast<RpgBullet*> (b);
				iface && b->objectId().ownerId == m_playerId &&
				iface->stage() != RpgGameData::LifeCycle::StageDestroy) {
			if (q->m_toSend.appendSnapshot(iface, tick, forceKeyFrame))
				hasSnap = true;
		}

	});

	m_rpgGame->iterateOverBodies([this, &tick, hasSnap, forceKeyFrame](TiledObjectBody *b) {
		if (RpgPlayer *iface = dynamic_cast<RpgPlayer*> (b); iface && m_rpgGame->controlledPlayer() == iface) {
			q->m_toSend.appendSnapshot(iface, tick, forceKeyFrame || hasSnap);
		}
	});


	if (!q->m_toSend.hasSnapshot())
		return;


	RpgGameData::CurrentSnapshot snapshot = q->m_toSend.renderCurrentSnapshot();

	if (QCborMap map = snapshot.toCbor(); !map.isEmpty()) {
		sendData(map.toCborValue().toCbor(), true);
	}

	q->m_lastSentTick = tick;

	q->m_toSend.clear();

	/*
#ifdef WITH_FTXUI
	DesktopApplication *a = dynamic_cast<DesktopApplication*>(Application::instance());

	QCborMap m;
	m.insert(QStringLiteral("mode"), QStringLiteral("SND"));

	QString txt;

	for (const auto &[key, d] : q->m_renderData.asKeyValueRange()) {
		QString t;
		switch (key) {
			case ActionRpgMultiplayerGamePrivate::SGsync:
				t = QStringLiteral("SG sync");
				break;
			case ActionRpgMultiplayerGamePrivate::SGrender:
				t = QStringLiteral("SG render");
				break;
			default:
				t = QStringLiteral("???");
				break;
		}

		txt += QStringLiteral("[%1]: ").arg(t, 12);

		txt += QStringLiteral("avg: %1 min: %2 max: %3")
			   .arg(d.avg(), 3)
			   .arg(d.min, 3)
			   .arg(d.max, 3)
			   ;

		if (!d.data.isEmpty()) {
			const auto [min, max] = std::minmax_element(d.data.constBegin(), d.data.constEnd());
			txt += QStringLiteral(" [min: %1 max: %2]").arg(*min, 3).arg(*max, 3);
		}

		txt += '\n';
	}


	txt += '\n';

	for (const auto &[key, d] : q->m_renderData.asKeyValueRange()) {
		switch (key) {
			case ActionRpgMultiplayerGamePrivate::SGsync:
				txt += QStringLiteral("SG sync\n");
				break;
			case ActionRpgMultiplayerGamePrivate::SGrender:
				txt += QStringLiteral("SG render\n");
				break;
			default:
				txt += QStringLiteral("???\n");
				break;
		}

		txt += QStringLiteral("-------------------------------------\n");

		for (int i=0; i<d.data.size() && i<40; ++i)
			txt += QStringLiteral("%1\n").arg(d.data.at(i), 5);

		txt += '\n';
	}

	m.insert(QStringLiteral("txt"), txt);
	a->writeToSocket(m.toCborValue());
#endif
*/

}





bool ActionRpgMultiplayerGame::onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable)
{
	return false;
}



/**
 * @brief ActionRpgMultiplayerGame::onPlayerAttackEnemy
 * @param player
 * @param enemy
 * @param weaponType
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;

	if (!player || player->objectId().ownerId != m_playerId)
		return false;

	LOG_CWARNING("game") << "PLAYER" << m_playerId << "ATTACK" << enemy << m_rpgGame->tickTimer()->currentTick();

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerAttack;
	p.tg = enemy->baseData();
	p.arm.cw = weaponType;

	q->m_toSend.appendSnapshot(player->baseData(), p);

	return true;
}



/**
 * @brief ActionRpgMultiplayerGame::onPlayerUseControl
 * @param player
 * @param control
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerUseControl(RpgPlayer *player, RpgActiveControlObject *control)
{
	return ActionRpgGame::onPlayerUseControl(player, control);
}

bool ActionRpgMultiplayerGame::onPlayerUseCast(RpgPlayer *player)
{
	return false;
}

bool ActionRpgMultiplayerGame::onPlayerCastTimeout(RpgPlayer *player)
{
	return false;
}

bool ActionRpgMultiplayerGame::onPlayerFinishCast(RpgPlayer *player)
{
	return false;
}


/**
 * @brief ActionRpgMultiplayerGame::onPlayerHit
 * @param player
 * @param enemy
 * @param weapon
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerHit(RpgPlayer *player, RpgEnemy *enemy, RpgWeapon *weapon)
{
	if (!player || player->objectId().ownerId != m_playerId)
		return false;

	LOG_CWARNING("game") << "PLAYER" << m_playerId << "HIT" << enemy << weapon << m_rpgGame->tickTimer()->currentTick();

	if (!ActionRpgGame::onPlayerHit(player, enemy, weapon))
		return false;

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerHit;
	p.arm.cw = weapon->weaponType();

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);

	return true;
}






/**
 * @brief ActionRpgMultiplayerGame::onPlayerShot
 * @param player
 * @param weaponType
 * @param scene
 * @param targets
 * @param angle
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerShot(RpgPlayer *player, RpgWeapon *weapon, const qreal &angle)
{
	if (!player || player->objectId().ownerId != m_playerId)
		return false;

	LOG_CWARNING("game") << "PLAYER" << player << "SHOT" << weapon << angle;

	if (!ActionRpgGame::onPlayerShot(player, weapon, angle))
		return false;

	LOG_CWARNING("game") << "SERIALIZE" << player << "SHOT";

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerShot;
	p.a = angle;
	p.arm.cw = weapon->weaponType();

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);

	return true;
}



/**
 * @brief ActionRpgMultiplayerGame::onPlayerWeaponChanged
 */

void ActionRpgMultiplayerGame::onPlayerWeaponChanged()
{
	RpgArmory *armory = qobject_cast<RpgArmory*>(sender());

	if (!armory) {
		LOG_CERROR("game") << "Invalid armory" << this << sender();
		return;
	}

	RpgPlayer *player = qobject_cast<RpgPlayer*>(armory->parentObject());

	if (!player) {
		LOG_CERROR("game") << "Invalid player" << armory << armory->parentObject();
		return;
	}

	if (player->objectId().ownerId != m_playerId)
		return;

	LOG_CWARNING("game") << "SERIALIZE" << player << "WEAPON CHANGE" << m_rpgGame->tickTimer()->currentTick();

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerWeaponChange;

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);

}








/**
 * @brief ActionRpgMultiplayerGame::onEnemyHit
 * @param enemy
 * @param player
 * @param weapon
 * @return
 */

bool ActionRpgMultiplayerGame::onEnemyHit(RpgEnemy *enemy, RpgPlayer *player, RpgWeapon *weapon)
{
	if (!enemy || !player || !weapon)
		return false;

	if (m_gameMode != MultiPlayerHost)
		return false;

	///LOG_CWARNING("game") << "ENEMY" << enemy << "HIT" << player->objectId().ownerId << weapon << m_rpgGame->tickTimer()->currentTick();

	if (!ActionRpgGame::onEnemyHit(enemy, player, weapon))
		return false;

	RpgGameData::Enemy e = enemy->serialize(m_rpgGame->tickTimer()->currentTick());
	e.st = RpgGameData::Enemy::EnemyHit;
	e.arm.cw = weapon->weaponType();

	q->m_toSend.appendSnapshot(enemy->baseData(), e);
	enemy->setLastSnapshot(e);

	return true;
}





/**
 * @brief ActionRpgMultiplayerGame::onEnemyShot
 * @param enemy
 * @param weapon
 * @param angle
 * @return
 */

bool ActionRpgMultiplayerGame::onEnemyShot(RpgEnemy *enemy, RpgWeapon *weapon, const qreal &angle)
{

	return false;
}




/**
 * @brief ActionRpgMultiplayerGame::onEnemyAttackPlayer
 * @param enemy
 * @param player
 * @param weaponType
 * @return
 */

bool ActionRpgMultiplayerGame::onEnemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType)
{
	if (m_gameMode != MultiPlayerHost)
		return false;

	if (!player || !enemy)
		return false;

	////LOG_CWARNING("game") << "ENEMY" << enemy << "ATTACK" << player->objectId().ownerId << m_rpgGame->tickTimer()->currentTick();

	RpgGameData::Enemy e = enemy->serialize(m_rpgGame->tickTimer()->currentTick());
	e.st = RpgGameData::Enemy::EnemyAttack;
	e.tg = player->baseData();
	e.arm.cw = weaponType;

	q->m_toSend.appendSnapshot(enemy->baseData(), e);

	return true;
}



/**
 * @brief ActionRpgMultiplayerGame::onBulletImpact
 * @param bullet
 * @param other
 * @return
 */

bool ActionRpgMultiplayerGame::onBulletImpact(RpgBullet *bullet, TiledObjectBody */*other*/)
{
	if (!bullet)
		return false;

	if (bullet->ownerId().o != m_playerId)
		return false;

	LOG_CWARNING("game") << "BULLET IMPACTED" << bullet->objectId().id
						 << "--->"
						 << bullet->m_impactedObject.o
							<< bullet->m_impactedObject.s
						 << bullet->m_impactedObject.id
						 << "at"
						 << m_rpgGame->tickTimer()->currentTick();

	bullet->setImpacted(true);
	bullet->disableBullet();
	bullet->setStage(RpgGameData::LifeCycle::StageDead);

	RpgGameData::Bullet b = bullet->serialize(m_rpgGame->tickTimer()->currentTick());
	q->m_toSend.appendSnapshot(bullet->baseData(), b);

	bullet->setLastSnapshot(b);

	return true;
}







/**
 * @brief ActionRpgMultiplayerGame::worldStep
 * @param body
 */

bool ActionRpgMultiplayerGame::onBodyStep(TiledObjectBody *body)
{
	if (m_config.gameState != RpgConfig::StatePlay || !body || !m_fullyPrepared)
		return true;

	q->updateBody(body,
				  body->objectId().ownerId == m_playerId || (body->objectId().ownerId == -1 && m_engine->isHost())
				  );

	return true;
}




/**
 * @brief ActionRpgMultiplayerGame::onRpgGameActivated
 */

void ActionRpgMultiplayerGame::onRpgGameActivated()
{
	if (!m_rpgGame)
		return;

	m_rpgGame->m_level = level();

	const auto &ptr = RpgGame::terrains().find(m_playerConfig.terrain);

	if (ptr == RpgGame::terrains().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid terrain" << m_playerConfig.terrain;
		return;
	}

	if (!m_rpgGame->load(ptr.value())) {
		LOG_CERROR("game") << "Game load error";
		return;
	}

	TiledScene *firstScene = m_rpgGame->findScene(ptr->firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Game load error: missing first scene";
		return;
	}

	emit m_rpgGame->gameLoaded();
}


/**
 * @brief ActionRpgMultiplayerGame::sendData
 * @param data
 * @param reliable
 */

void ActionRpgMultiplayerGame::sendData(const QSerializer &data, const bool &reliable)
{
	m_engine->sendMessage(data.toCborMap().toCborValue().toCbor(), reliable);
}


/**
 * @brief ActionRpgMultiplayerGame::sendData
 * @param data
 * @param reliable
 */

void ActionRpgMultiplayerGame::sendData(const QByteArray &data, const bool &reliable)
{
	m_engine->sendMessage(data, reliable);
}


/**
 * @brief ActionRpgMultiplayerGame::sendDataChrSel
 */

void ActionRpgMultiplayerGame::sendDataChrSel()
{
	RpgGameData::CharacterSelect config(m_playerConfig);

	if (m_gameMode == MultiPlayerHost) {
		config.gameConfig.terrain = m_playerConfig.terrain;
	}

	config.completed = m_selectionCompleted;

	sendData(config, true);
}



/**
 * @brief ActionRpgMultiplayerGame::sendDataPrepare
 */

void ActionRpgMultiplayerGame::sendDataPrepare()
{
	RpgGameData::Prepare config;

	QCborMap map;

	config.prepared = m_gamePrepared && m_playersSynced && m_enemiesSynced;

	if (m_gameMode == MultiPlayerHost) {
		if (m_rpgGame) {
			for (const RpgGame::PlayerPosition &p : m_rpgGame->m_playerPositionList) {
				if (p.scene)
					config.gameConfig.positionList.emplaceBack(p.scene->sceneId(), p.position.x(), p.position.y());
				else
					LOG_CERROR("game") << "Missing scene" << p.position;
			}
		}

		config.gameConfig.terrain = m_playerConfig.terrain;

		RpgGameData::CurrentSnapshot snap;

		for (RpgGame::EnemyData &e : m_rpgGame->m_enemyDataList) {
			RpgGameData::EnemyBaseData eData(e.type, -1, e.objectId.sceneId, e.objectId.id);

			if (RpgEnemy *enemy = dynamic_cast<RpgEnemy*>(e.enemy.get()))
				snap.assign(snap.enemies, eData, enemy->serialize(0));
			else
				snap.assign(snap.enemies, eData, RpgGameData::Enemy());

		}

		map = config.toCborMap();

		QCborMap sm = snap.toCbor();

		for (auto it = sm.cbegin(); it != sm.cend(); ++it)
			map.insert(it.key(), it.value());

	} else {
		map = config.toCborMap();
	}

	sendData(map.toCborValue().toCbor(), true);
}



/**
 * @brief ActionRpgMultiplayerGame::setTickTimer
 * @param tick
 */

void ActionRpgMultiplayerGame::setTickTimer(const qint64 &tick)
{
	q->m_timeSync.set(tick);
}


/**
 * @brief ActionRpgMultiplayerGame::addLatency
 * @param latency
 */

void ActionRpgMultiplayerGame::addLatency(const qint64 &latency)
{
	q->m_timeSync.addLatency(latency);
}




/**
 * @brief ActionRpgMultiplayerGame::selectionCompleted
 * @return
 */


bool ActionRpgMultiplayerGame::selectionCompleted() const
{
	return m_selectionCompleted;
}

void ActionRpgMultiplayerGame::setSelectionCompleted(bool newSelectionCompleted)
{
	if (m_selectionCompleted == newSelectionCompleted)
		return;
	m_selectionCompleted = newSelectionCompleted;
	emit selectionCompletedChanged();
}





/**
 * @brief ActionRpgMultiplayerGamePrivate::updateBody
 * @param body
 * @param game
 * @param snapshot
 */

void ActionRpgMultiplayerGamePrivate::updateBody(TiledObjectBody *body, const bool &isHosted)
{
	Q_ASSERT(d);
	Q_ASSERT(body);

	if (RpgPlayer *p = dynamic_cast<RpgPlayer*>(body))
		update<RpgPlayer, RpgGameData::Player, RpgGameData::PlayerBaseData>(p, isHosted);
	else if (RpgEnemy *p = dynamic_cast<RpgEnemy*>(body))
		update<RpgEnemy, RpgGameData::Enemy, RpgGameData::EnemyBaseData>(p, isHosted);
	else if (RpgBullet *p = dynamic_cast<RpgBullet*>(body))
		update<RpgBullet, RpgGameData::Bullet, RpgGameData::BulletBaseData>(p, isHosted);
	else if (isHosted)
		body->worldStep();


}
