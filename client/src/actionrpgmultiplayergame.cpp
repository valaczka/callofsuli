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
#include "rpgcontrolcontainer.h"
#include "rpgcontrolgate.h"
#include "rpgcontrollight.h"
#include "rpgcontrolcollection.h"
#include "rpgcontrolrandomizer.h"
#include "rpgcontrolteleport.h"
#include "rpgpickable.h"
#include "rpgquestion.h"
#include "rpgudpengine.h"
#include "client.h"
#include "rpgplayer.h"
#include "utils_.h"

#include <chipmunk/chipmunk.h>



#define CONNECTION_LOST_TIMEOUT				1000		// Ennyi ideig próbál újracsatlakozni (az ENet 5 mp után dobja ki)


/**
 * Private namespace
 */

class ActionRpgMultiplayerGamePrivate
{
private:
	ActionRpgMultiplayerGamePrivate(ActionRpgMultiplayerGame *game)
		: d(game)
		, m_connectionLostTimer(-1)
	{}

	void updateBody(TiledObjectBody *body, const bool &isHosted);
	void resetEngine();

	RpgGameData::Armory getArmory() const;

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

	bool m_canAddEngine = false;
	bool m_requireEngine = false;
	int m_selectedEngineId = -1;
	int m_engineId = -1;

	bool m_isReconnecting = false;
	bool m_hasReconnected = false;

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

	QDeadlineTimer m_connectionLostTimer;



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
	, m_enginesModel(std::make_unique<QSListModel>())
	, m_engine(new RpgUdpEngine(this))
	, q(new ActionRpgMultiplayerGamePrivate(this))
{
	LOG_CDEBUG("game") << "ActionRpgMultiplayerGame constructed";

	setGameMode(MultiPlayerGuest);

	QStringList roles = Utils::getRolesFromObject(RpgGameData::CharacterSelect().metaObject());
	roles.append(QStringLiteral("player"));
	m_playersModel->setRoleNames(roles);

	m_enginesModel->setRoleNames(Utils::getRolesFromObject(RpgGameData::Engine().metaObject()));

	if (m_client->server()) {
		if (User *u = m_client->server()->user()) {
			m_playerConfig.username = u->username();
			m_playerConfig.nickname = u->fullNickName();
		}
	}

	connect(m_engine, &RpgUdpEngine::gameDataDownload, this, &ActionRpgMultiplayerGame::downloadGameData);
	connect(m_engine, &RpgUdpEngine::gameError, this, &ActionRpgMultiplayerGame::setUnknownError);
	connect(m_engine, &RpgUdpEngine::serverConnectFailed, this, &ActionRpgMultiplayerGame::setError);
	connect(m_engine, &RpgUdpEngine::serverConnectionLost, this, &ActionRpgMultiplayerGame::onConnectionLost);
	connect(m_engine, &RpgUdpEngine::serverConnected, this, &ActionRpgMultiplayerGame::onConnected);
	connect(m_engine, &RpgUdpEngine::serverDisconnected, this, &ActionRpgMultiplayerGame::onDisconnected);

	m_keepAliveTimer.start(200, this);
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
		m_rpgGame->setMessageEnabled(true);
		connect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgMultiplayerGame::onGameSuccess, Qt::QueuedConnection);		// Azért kell, mert különbön az utolsó fegyverhasználatot nem számolja el a szerveren
		//connect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgMultiplayerGame::onPlayerDead);
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
 * @brief ActionRpgMultiplayerGame::selectTerrain
 * @param terrain
 */

void ActionRpgMultiplayerGame::selectTerrain(const QString &terrain)
{
	if (terrain == m_playerConfig.terrain)
		return;

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

	m_playerConfig.character = character;
}




/**
 * @brief ActionRpgMultiplayerGame::banOutPlayer
 * @param playerId
 */

void ActionRpgMultiplayerGame::banOutPlayer(const int &playerId)
{
	if (m_gameMode != MultiPlayerHost || m_config.gameState != RpgConfig::StateCharacterSelect)
		return;

	sendDataChrSel(playerId);
}



/**
 * @brief ActionRpgMultiplayerGame::lockEngine
 */

void ActionRpgMultiplayerGame::lockEngine()
{
	if (m_gameMode != MultiPlayerHost || m_config.gameState != RpgConfig::StateCharacterSelect)
		return;

	sendDataChrSel(-1, true);
}



/**
 * @brief ActionRpgMultiplayerGame::connectToEngine
 * @param id
 */

void ActionRpgMultiplayerGame::connectToEngine(const int &id)
{
	if (id <= 0) {
		LOG_CDEBUG("game") << "Require new engine";
		q->m_requireEngine = true;
		sendDataConnect();
	} else {
		LOG_CDEBUG("game") << "Connect to engine" << id;
		q->m_requireEngine = false;
		q->m_selectedEngineId = id;
		sendDataConnect();
	}
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
		LOG_CERROR("game") << "***** ERROR -> DISCONNECT";
		disconnectFromHost();
		return;
	}

	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (m_config.gameState == RpgConfig::StateConnect) {
		if (m_engine->connectionToken().isEmpty()) {
			LOG_CERROR("game") << "Missing connection token";
			setError(tr("Internal error"));
		} else {
			m_engine->connectToServer(m_client->server());
		}
		return;
	}

	if (m_config.gameState == RpgConfig::StatePrepare) {
		if (m_oldGameState != m_config.gameState)
			m_rpgQuestion->initialize();

		ActionRpgGame::onConfigChanged();
		return;
	}

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
			m_rpgGame->setMessageEnabled(true);
			//m_rpgGame->message(tr("LEVEL %1").arg(level()));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);

			m_fullyPrepared = true;

			gameStart();
			//m_deadlineTick = AbstractGame::TickTimer::msecToTick(m_config.duration*1000);
			//m_elapsedTick = 0;
			m_rpgGame->tickTimer()->start(this, m_elapsedTick);
			///m_timerLeft.start();
		}

		if (!m_fullyPrepared || q->m_isReconnecting) {
			LOG_CINFO("game") << "-----> SEND FULL";
			QCborMap map;
			map.insert(QStringLiteral("full"), true);
			sendData(map.toCborValue().toCbor(), true);
		}

	}


	if (m_config.gameState == RpgConfig::StateFinished && m_oldGameState == RpgConfig::StatePlay) {
		emit m_rpgGame->gameSuccess();
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

#ifdef WITH_FTXUI
	if (DesktopApplication *a = dynamic_cast<DesktopApplication*>(Application::instance())) {
		QCborMap map;
		QString txt;


		map.insert(QStringLiteral("mode"), QStringLiteral("RCV"));

		txt = QStringLiteral("CURRENT TICK %1 - server %2 - deadline %3\n\n").arg(tick).arg(snapshots.serverTick()).arg(m_deadlineTick);


		for (const auto &ptr : snapshots.controls().gates) {
			txt += QStringLiteral("STORAGE GATE %1 %2 %3\n=============================================\n")
				   .arg(ptr.data.o)
				   .arg(ptr.data.s)
				   .arg(ptr.data.id)
				   ;

			for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
				txt += QStringLiteral("%1: %2 (%3) %4")
					   .arg(it->second.f)
					   .arg(it->second.st)
					   .arg(it->second.a)
					   .arg(it->second.lck)
					   ;


				if (it->second.u.isValid())
					txt += QStringLiteral(" holder %1")
						   .arg(it->second.u.o)
						   ;
			}

			txt += QStringLiteral("\n \n");
		}


		for (const auto &ptr : snapshots.controls().collections) {
			txt += QStringLiteral("STORAGE COLLECTION %1 %2 %3   [%4]\n==============================================\n")
				   .arg(ptr.data.o)
				   .arg(ptr.data.s)
				   .arg(ptr.data.id)
				   .arg(ptr.data.gid)
				   ;

			for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
				txt += QStringLiteral("%1: (%2) %3  @%4")
					   .arg(it->second.f)
					   .arg(it->second.a ? "+" : "-")
					   .arg(it->second.lck)
					   .arg(it->second.idx)
					   ;

				if (it->second.p.size() > 1) {
					txt += QStringLiteral(" [%1,%2]")
						   .arg(it->second.p.at(0))
						   .arg(it->second.p.at(1))
						   ;
				}

				if (it->second.u.isValid())
					txt += QStringLiteral(" holder %1")
						   .arg(it->second.u.o)
						   ;

				if (it->second.own.isValid())
					txt += QStringLiteral(" owner %1")
						   .arg(it->second.own.o)
						   ;

				txt += '\n';
			}

			txt += QStringLiteral("\n \n");
		}


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


		for (const auto &ptr : snapshots.controls().containers) {
			txt += QStringLiteral("STORAGE CONTAINER %1 %2 %3\n==============================================\n")
				   .arg(ptr.data.o)
				   .arg(ptr.data.s)
				   .arg(ptr.data.id);

			for (auto it = ptr.list.crbegin(); it != ptr.list.crend(); ++it) {
				txt += QStringLiteral("%1: %2 (%3) %4\n")
					   .arg(it->second.f)
					   .arg(it->second.st)
					   .arg(it->second.a)
					   .arg(it->second.lck)
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

	if (!q->m_connectionLostTimer.isForever() &&
			q->m_connectionLostTimer.hasExpired()) {
		LOG_CERROR("game") << "Connection lost timeout" << q->m_connectionLostTimer.isForever() << q->m_connectionLostTimer.hasExpired()
						   << q->m_connectionLostTimer.remainingTime();

		if (m_config.gameState == RpgConfig::StatePlay) {
			LOG_CINFO("game") << "Start reconnecting";
			m_config.gameState = RpgConfig::StateDownloadContent;
			changeGameState(RpgConfig::StatePlay);
			//updateConfig();

		} else {
			setError(tr("Connection lost"));
		}

		q->m_connectionLostTimer.setRemainingTime(-1);

		return;
	}

	if (m_engine->gameState() == RpgConfig::StateInvalid)
		return;

	changeGameState(m_engine->gameState());

	setGameMode(m_engine->isHost() ? ActionRpgGame::MultiPlayerHost : ActionRpgGame::MultiPlayerGuest);
	setPlayerId(m_engine->playerId());

	if (q->m_isReconnecting || (!m_engine->isHost() && !m_engine->gameConfig().terrain.isEmpty()))
		worldTerrainSelect(m_engine->gameConfig().terrain, true);

	updatePlayersModel();

	if (m_config.gameState == RpgConfig::StateError) {
		//disconnectFromHost();
		return;
	}

	if (m_config.gameState == RpgConfig::StateInvalid)
		return;


	if (q->m_isReconnecting)
		return;

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		sendDataChrSel();
	else if (m_config.gameState == RpgConfig::StatePrepare)
		sendDataPrepare();
	else if (m_config.gameState == RpgConfig::StateConnect)
		sendDataConnect();


	if (tick > 0)
		m_engine->zapSnapshots(tick - 3*RPG_UDP_DELTA_TICK);

	if (snapshots.deadlineTick() > 0)
		m_deadlineTick = snapshots.deadlineTick();

	const QList<RpgGameData::Message> &messageList = m_engine->takeMessageList();

	for (const RpgGameData::Message &msg : messageList) {
		m_rpgGame->message(msg.m, msg.p);
	}

}



/**
 * @brief ActionRpgMultiplayerGame::changeGameState
 * @param state
 */

void ActionRpgMultiplayerGame::changeGameState(const RpgConfig::GameState &state)
{
	if (!q->m_isReconnecting && state == RpgConfig::StatePlay && m_config.gameState < RpgConfig::StatePrepare) {
		LOG_CINFO("game") << "Start reconnecting" << m_config.gameState << "->" << state;
		q->m_isReconnecting = true;
		emit isReconnectingChanged();
		if (m_rpgGame)
			m_rpgGame->setPaused(true);
	}

	if (q->m_isReconnecting) {
		if (m_config.gameState < RpgConfig::StateDownloadContent &&
				!m_playerConfig.terrain.isEmpty()) {
			m_config.gameState = RpgConfig::StateDownloadContent;
			LOG_CINFO("game") << "New reconnecting state" << m_config.gameState << m_playerConfig.terrain;
			updateConfig();
		} else if (m_config.gameState < RpgConfig::StateCharacterSelect) {
			m_config.gameState = RpgConfig::StateCharacterSelect;
			LOG_CINFO("game") << "New reconnecting state" << m_config.gameState << m_playerConfig.terrain;
			updateConfig();
		} else if (m_config.gameState < RpgConfig::StateDownloadContent) {
			m_config.gameState = RpgConfig::StateDownloadContent;
			LOG_CINFO("game") << "New reconnecting state" << m_config.gameState;
			updateConfig();
		} else if (m_config.gameState >= RpgConfig::StatePrepare) {
			updateConfig();
			q->m_isReconnecting = false;
			q->m_hasReconnected = true;
			if (m_rpgGame) {
				m_rpgGame->setMessageEnabled(true);
				m_rpgGame->setPaused(false);
			}
			emit isReconnectingChanged();
			LOG_CINFO("game") << "Reconnecting finished" << m_config.gameState;
		}

	} else {
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
				canSwitch = m_config.gameState <= RpgConfig::StateCharacterSelect;		// Itt még vissza tud váltani (pl. ha bannolnak)
				break;

			case RpgConfig::StateDownloadStatic:
			case RpgConfig::StateDownloadContent:
				canSwitch = m_config.gameState < RpgConfig::StatePrepare;
				break;

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
			setError(tr("Internal error"));
		}
	}
}

bool ActionRpgMultiplayerGame::locked() const
{
	return m_locked;
}

void ActionRpgMultiplayerGame::setLocked(bool newLocked)
{
	if (m_locked == newLocked)
		return;
	m_locked = newLocked;
	emit lockedChanged();
}

int ActionRpgMultiplayerGame::maxPlayers() const
{
	return m_maxPlayers;
}

void ActionRpgMultiplayerGame::setMaxPlayers(int newMaxPlayers)
{
	if (m_maxPlayers == newMaxPlayers)
		return;
	m_maxPlayers = newMaxPlayers;
	emit maxPlayersChanged();
}





/**
 * @brief ActionRpgMultiplayerGame::setConnectionToken
 * @param newConnectionToken
 */

void ActionRpgMultiplayerGame::setConnectionToken(const QByteArray &newConnectionToken)
{
	if (m_engine)
		m_engine->setConnectionToken(newConnectionToken);
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

void ActionRpgMultiplayerGame::updatePlayersModel()
{
	const QList<RpgGameData::CharacterSelect> &plist = m_engine->playerData();

	QVariantList list;

	for (const RpgGameData::CharacterSelect &p : plist) {
		QVariantMap m = p.toJson().toVariantMap();

		RpgPlayer *player = nullptr;

		if (m_rpgGame) {
			const auto &it = std::find_if(m_rpgGame->m_players.cbegin(),
										  m_rpgGame->m_players.cend(),
										  [&p](RpgPlayer *player) {
							 return player && player->baseData().o == p.playerId;
		});

			if (it != m_rpgGame->m_players.cend()) {
				player = *it;

				player->setIsGameCompleted(p.finished);

				if (player == m_rpgGame->m_controlledPlayer) {
					setXp(p.xp);
					m_rpgGame->setCurrency(p.cur);
				}
			}
		}

		m.insert(QStringLiteral("player"), QVariant::fromValue(player));
		list.append(m);
	}


	Utils::patchSListModel(m_playersModel.get(), list, QStringLiteral("playerId"));
}



/**
 * @brief ActionRpgMultiplayerGame::updateEnginesModel
 * @param selector
 */

void ActionRpgMultiplayerGame::updateEnginesModel(const RpgGameData::EngineSelector &selector)
{
	if (selector.operation == RpgGameData::EngineSelector::Connect) {
		LOG_CINFO("game") << "Connected to engine" << selector.engine;
		q->m_selectedEngineId = selector.engine;
		m_engine->setGameState(RpgConfig::StateConnect);
		return;
	}

	if (selector.operation == RpgGameData::EngineSelector::Reset) {
		q->resetEngine();
		Utils::patchSListModel(m_enginesModel.get(), QVariantList(), QStringLiteral("id"));
		return;
	}

	if (selector.operation != RpgGameData::EngineSelector::List)
		return;

	QVariantList list;

	for (const RpgGameData::Engine &e : selector.engines) {
		list << e.toVariantMap();
	}

	Utils::patchSListModel(m_enginesModel.get(), list, QStringLiteral("id"));

	setCanAddEngine(selector.add);
}




/**
 * @brief ActionRpgMultiplayerGame::syncEnemyList
 * @param list
 */


void ActionRpgMultiplayerGame::syncEnemyList(const ClientStorage &storage)
{
	if (!m_rpgGame || storage.enemies().empty() || !m_tiledGameLoaded)
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

	m_enemiesSynced = onlyServerEnemy.isEmpty() && localIndices.isEmpty();
}





/**
 * @brief ActionRpgMultiplayerGame::syncPlayerList
 * @param list
 */

void ActionRpgMultiplayerGame::syncPlayerList(const ClientStorage &storage)
{
	if (!m_rpgGame || storage.players().empty() || !m_tiledGameLoaded)
		return;

	if (m_playersSynced && (qsizetype) storage.players().size() == m_rpgGame->m_players.size())
		return;

	m_playersSynced = false;

	QList<RpgPlayer*> playerList;

	for (const auto &pl : storage.players()) {
		const auto it = std::find_if(m_rpgGame->m_players.begin(),
									 m_rpgGame->m_players.end(),
									 [&pl](const RpgPlayer *p){
			return p->baseData().isBaseEqual(pl.data);
		});

		if (it == m_rpgGame->m_players.end()) {
			LOG_CWARNING("game") << "Missing player" << pl.data.s << pl.data.id << pl.data.o << "RQ" << pl.data.rq;

			RpgGameData::Player pd;

			if (pl.list.size() > 0) {
				pd = std::prev(pl.list.cend())->second;
			} else {
				LOG_CERROR("game") << "Player data missing" << pl.data.o;
			}

			TiledScene *scene = m_rpgGame->findScene(pd.sc);
			if (!scene) {
				LOG_CERROR("game") << "Invalid scene" << pd.sc;
				return;
			}

			RpgPlayer *rpgPlayer = createPlayer(scene, pl.data, pd);

			if (!rpgPlayer) {
				LOG_CERROR("game") << "Missing RpgPlayer";
				return;
			}

			if (pl.data.o == m_playerId) {
				m_rpgGame->setFollowedItem(rpgPlayer);
				m_rpgGame->setControlledPlayer(rpgPlayer);
			}

			if (q->m_hasReconnected) {
				updateLastObjectId(rpgPlayer);
			}

			playerList.append(rpgPlayer);

		}
	}

	if (!playerList.isEmpty()) {
		playerList.append(m_rpgGame->players());
		m_rpgGame->setPlayers(playerList);
	}

	m_playersSynced = true;
}




/**
 * @brief ActionRpgMultiplayerGame::syncBulletList
 */

void ActionRpgMultiplayerGame::syncBulletList(const ClientStorage &storage)
{
	if (!m_rpgGame || !m_tiledGameLoaded)
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

			newBullet->baseData() = b.data;
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
 * @brief ActionRpgMultiplayerGame::syncCollectionList
 * @param storage
 */

void ActionRpgMultiplayerGame::syncCollectionList(const ClientStorage &storage)
{
	if (!m_rpgGame || storage.controls().collections.empty() || !m_tiledGameLoaded)
		return;

	for (const auto &pe : storage.controls().collections) {
		if (m_rpgGame->controlFind<RpgControlCollection>(pe.data)) {
			continue;
		}

		TiledScene *scene = m_rpgGame->findScene(pe.data.s);

		if (!scene) {
			LOG_CERROR("scene") << "Invalid scene" << pe.data.t << pe.data.o << pe.data.s << pe.data.id;
			continue;
		}

		if (pe.list.empty()) {
			LOG_CERROR("scene") << "Missing list" << pe.data.t << pe.data.o << pe.data.s << pe.data.id;
			continue;
		}

		QList<float> pData = pe.list.cbegin()->second.p;

		if (pData.size() < 2) {
			LOG_CERROR("scene") << "Invalid position" << pe.data.t << pe.data.o << pe.data.s << pe.data.id;
			continue;
		}

		QPointF pos(pData.at(0), pData.at(1));

		m_rpgGame->controlAdd<RpgControlCollection>(m_rpgGame, scene, pe.data, pos);
	}

}


/**
 * @brief ActionRpgMultiplayerGame::syncPickableList
 * @param storage
 */

void ActionRpgMultiplayerGame::syncPickableList(const ClientStorage &storage)
{
	if (!m_rpgGame || !m_tiledGameLoaded)
		return;

	QList<RpgControlBase*> bList;

	// Load all controls

	for (const auto &ptr : m_rpgGame->m_controls) {
		if (RpgPickable *p = dynamic_cast<RpgPickable*>(ptr.get())) {
			bList.append(p);
		}
	}

	// Sync pickables

	for (const auto &b : storage.controls().pickables) {
		if (RpgPickable *pickable = m_rpgGame->controlFind<RpgPickable>(b.data)) {
			// Remove existing pickable
			bList.removeAll(pickable);
		} else {

			// Create new pickable

			TiledScene *scene = m_rpgGame->findScene(b.data.s);

			if (!scene) {
				LOG_CERROR("game") << "Invalid scene" << b.data.s;
				continue;
			}

			RpgPickable *newPickable = m_rpgGame->controlAdd<RpgPickable>(m_rpgGame, scene, b.data);

			if (!newPickable) {
				LOG_CERROR("game") << "Pickable create error" << b.data.o << b.data.s << b.data.id;
			}
		}
	}



	// Delete missing pickables

	m_rpgGame->controlRemove(bList);

}







/**
 * @brief ActionRpgMultiplayerGame::updateLastObjectIds
 */

void ActionRpgMultiplayerGame::updateLastObjectId(RpgPlayer *player)
{
	if (!player)
		return;

	const QList<RpgGameData::CharacterSelect> &data = m_engine->playerData();
	const int playerId = player->baseData().o;

	const auto it = std::find_if(data.cbegin(),
								 data.cend(),
								 [&playerId](const RpgGameData::CharacterSelect &d) {
		return d.playerId == playerId;
	});

	if (it != data.cend())
		player->m_lastObjectId = std::max(player->m_lastObjectId, it->lastObjectId);
}



/**
 * @brief ActionRpgMultiplayerGame::onGameTimeout
 */

void ActionRpgMultiplayerGame::onGameTimeout()
{

}



/**
 * @brief ActionRpgMultiplayerGame::onGameSuccess
 */

void ActionRpgMultiplayerGame::onGameSuccess()
{
	Sound *sound = m_client->sound();

	////m_rpgGame->checkFinalQuests();

	setFinishState(Success);
	gameFinish();

	sound->stopMusic();
	sound->stopMusic2();
	sound->playSound(QStringLiteral("qrc:/sound/sfx/win.mp3"), Sound::SfxChannel);

	sound->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	sound->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);

	emit finishDialogRequest(m_isFlawless ? tr("Mission completed\nHibátlan győzelem!")
										  : tr("Mission completed"),
							 QStringLiteral("qrc:/Qaterial/Icons/trophy.svg"),
							 true);
}



/**
 * @brief ActionRpgMultiplayerGame::onGameFailed
 */

void ActionRpgMultiplayerGame::onGameFailed()
{

}






/**
 * @brief ActionRpgMultiplayerGame::playersModel
 * @return
 */

QSListModel *ActionRpgMultiplayerGame::playersModel() const
{
	return m_playersModel.get();
}


/**
 * @brief ActionRpgMultiplayerGame::enginesModel
 * @return
 */

QSListModel *ActionRpgMultiplayerGame::enginesModel() const
{
	return m_enginesModel.get();
}

bool ActionRpgMultiplayerGame::isReconnecting() const
{
	return q->m_isReconnecting;
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

	player->baseData() = config;
	player->setCollectionRq(config.rq);
	player->setCollection(0);
	player->setMaxHp(playerData.mhp);
	player->setHp(playerData.hp);
	player->setMaxMp(characterPtr->mpMax);
	player->setMp(characterPtr->mpStart);
	player->armory()->updateFromSnapshot(playerData.arm);

	loadInventory(player);


	// Set user name

	if (Server *s = m_client->server()) {
		player->setDisplayName(s->user()->fullNickName());
	}


	if (playerData.p.size() > 1) {
		player->emplace(playerData.p.at(0), playerData.p.at(1));
		LOG_CINFO("scene") << "PLAYER EMPLACED AT" << playerData.p << player;
	} else
		LOG_CWARNING("scene") << "Player position missing! ########################";

	player->setCurrentAngle(playerData.a);


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
	syncCollectionList(storage);
	syncPickableList(storage);

	if (const RpgGameData::Randomizer &randomizer = m_engine->gameConfig().randomizer; randomizer.groups.isEmpty()) {
		if (m_playersSynced && m_rpgGame->randomizer().groups.isEmpty()) {
			m_randomizerSynced = true;
		}
	} else if (!m_randomizerSynced) {
		updateRandomizer(randomizer);
		m_randomizerSynced = true;
	}


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
		LOG_CWARNING("game") << "Time skew -1 frame" << curr << "->" << tick;
		m_rpgGame->tickTimer()->start(this, curr-1);
	}


	m_rpgGame->iterateOverBodies([this](TiledObjectBody *b){
		if (RpgGameData::LifeCycle *iface = dynamic_cast<RpgGameData::LifeCycle*> (b)) {
			if (iface->stage() == RpgGameData::LifeCycle::StageDestroy) {
				onLifeCycleDelete(b);
			}
		}
	});

	emit msecLeftChanged();
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

	if (m_fullyPrepared) {
		m_rpgGame->iterateOverBodies([this, &tick, hasSnap, forceKeyFrame](TiledObjectBody *b) {
			if (RpgPlayer *iface = dynamic_cast<RpgPlayer*> (b); iface && m_rpgGame->controlledPlayer() == iface) {
				q->m_toSend.appendSnapshot(iface, tick, forceKeyFrame || hasSnap);
			}
		});
	}


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

	txt = QJsonDocument(snapshot.toCbor().toJsonObject()).toJson();

	m.insert(QStringLiteral("txt"), txt);
	a->writeToSocket(m.toCborValue());
#endif
*/

}






/**
 * @brief ActionRpgMultiplayerGame::onPlayerAttackEnemy
 * @param player
 * @param enemy
 * @param weaponType
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy,
												   const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype)
{
	if (!player || !enemy)
		return false;

	if (!player || player->baseData().o != m_playerId)
		return false;

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerAttack;
	p.tg = enemy->baseData();
	p.arm.cw = weaponType;
	p.arm.s = weaponSubtype;

	q->m_toSend.appendSnapshot(player->baseData(), p);

	return true;
}



/**
 * @brief ActionRpgMultiplayerGame::onPlayerUseControl
 * @param player
 * @param control
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerUseControl(RpgPlayer *player, RpgActiveIface *control)
{
	if (!player || player->baseData().o != m_playerId)
		return false;

	if (!control)
		return false;

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());

	if (control->activeType() == RpgConfig::ControlExit) {
		p.st = RpgGameData::Player::PlayerExit;

		// Ezt ne tegyük be last snaphostnak

	} else {
		p.st = !control->questionLock() || m_rpgQuestion->emptyQuestions() ? RpgGameData::Player::PlayerUseControl : RpgGameData::Player::PlayerLockControl;
		p.tg = control->pureBaseData();

		player->setLastSnapshot(p);
	}

	q->m_toSend.appendSnapshot(player->baseData(), p);

	return true;
}


/**
 * @brief ActionRpgMultiplayerGame::onPlayerUseCast
 * @param player
 * @return
 */

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
	if (!player || player->baseData().o != m_playerId)
		return false;

	if (!ActionRpgGame::onPlayerHit(player, enemy, weapon))
		return false;

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerHit;
	p.arm.cw = weapon->weaponType();

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);

	LOG_CINFO("game") << "----LAST" << p.ft;

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
	if (!player || player->baseData().o != m_playerId)
		return false;

	if (!ActionRpgGame::onPlayerShot(player, weapon, angle))
		return false;

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerShot;
	p.a = angle;
	p.arm.cw = weapon->weaponType();

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);

	return true;
}



/**
 * @brief ActionRpgMultiplayerGame::onQuestionSuccess
 * @param player
 * @param enemy
 * @param control
 * @param xp
 */

void ActionRpgMultiplayerGame::onQuestionSuccess(RpgPlayer *player, RpgActiveIface *control, int xp)
{
	if (!control)
		return;

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerUseControl;
	p.tg = control->pureBaseData();
	p.x = xp;

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);
}


/**
 * @brief ActionRpgMultiplayerGame::onQuestionFailed
 * @param player
 * @param enemy
 * @param control
 */

void ActionRpgMultiplayerGame::onQuestionFailed(RpgPlayer *player, RpgActiveIface *control)
{
	setIsFlawless(false);

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerUnlockControl;
	p.tg = control->pureBaseData();

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->setLastSnapshot(p);
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

	if (player->baseData().o != m_playerId)
		return;

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
	if (!enemy || !weapon)
		return false;

	if (m_gameMode != MultiPlayerHost)
		return false;

	///LOG_CWARNING("game") << "ENEMY" << enemy << "HIT" << player->objectId().ownerId << weapon << m_rpgGame->tickTimer()->currentTick();

	LOG_CWARNING("game") << "ENEMY" << enemy << "SHOT" << weapon << angle;

	if (!ActionRpgGame::onEnemyShot(enemy, weapon, angle))
		return false;

	RpgGameData::Enemy e = enemy->serialize(m_rpgGame->tickTimer()->currentTick());
	e.st = RpgGameData::Enemy::EnemyShot;
	e.arm.cw = weapon->weaponType();

	q->m_toSend.appendSnapshot(enemy->baseData(), e);
	enemy->setLastSnapshot(e);

	return true;
}




/**
 * @brief ActionRpgMultiplayerGame::onEnemyAttackPlayer
 * @param enemy
 * @param player
 * @param weaponType
 * @return
 */

bool ActionRpgMultiplayerGame::onEnemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player,
												   const RpgGameData::Weapon::WeaponType &weaponType, const int &weaponSubtype)
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
	e.arm.s = weaponSubtype;

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

	if (bullet->baseData().o != m_playerId)				// Csak az általunk gyártott (player vagy enemy) lövedékekkel foglalkozunk
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

	if (q->m_isReconnecting) {
		body->stop();
		return true;
	}

	q->updateBody(body,
				  body->objectId().ownerId == m_playerId || (body->objectId().ownerId == -1 && m_engine->isHost())
				  );

	return true;
}



/**
 * @brief ActionRpgMultiplayerGame::onWorldStep
 */

void ActionRpgMultiplayerGame::onWorldStep()
{
	for (const auto &ptr : m_rpgGame->m_controls) {
		if (RpgControlLight *c = dynamic_cast<RpgControlLight*>(ptr.get())) {
			if (const auto &s = q->m_currentSnapshot.getSnapshot<RpgGameData::ControlBaseData, RpgGameData::ControlLight>(
						c->baseData(), q->m_currentSnapshot.controls.lights))
				c->updateFromSnapshot(s.value());

		} else if (RpgControlContainer *c = dynamic_cast<RpgControlContainer*>(ptr.get())) {
			if (const auto &s = q->m_currentSnapshot.getSnapshot<RpgGameData::ControlContainerBaseData, RpgGameData::ControlContainer>(
						c->baseData(), q->m_currentSnapshot.controls.containers))
				c->updateFromSnapshot(s.value());

		} else if (RpgControlCollection *c = dynamic_cast<RpgControlCollection*>(ptr.get())) {
			if (const auto &s = q->m_currentSnapshot.getSnapshot<RpgGameData::ControlCollectionBaseData, RpgGameData::ControlCollection>(
						c->baseData(), q->m_currentSnapshot.controls.collections))
				c->updateFromSnapshot(s.value());

		} else if (RpgPickable *c = dynamic_cast<RpgPickable*>(ptr.get())) {
			if (const auto &s = q->m_currentSnapshot.getSnapshot<RpgGameData::PickableBaseData, RpgGameData::Pickable>(
						c->baseData(), q->m_currentSnapshot.controls.pickables))
				c->updateFromSnapshot(s.value());

		} else if (RpgControlGate *c = dynamic_cast<RpgControlGate*>(ptr.get())) {
			if (const auto &s = q->m_currentSnapshot.getSnapshot<RpgGameData::ControlGateBaseData, RpgGameData::ControlGate>(
						c->baseData(), q->m_currentSnapshot.controls.gates))
				c->updateFromSnapshot(s.value());

		} else if (RpgControlTeleport *c = dynamic_cast<RpgControlTeleport*>(ptr.get())) {
			if (const auto &s = q->m_currentSnapshot.getSnapshot<RpgGameData::ControlTeleportBaseData, RpgGameData::ControlTeleport>(
						c->baseData(), q->m_currentSnapshot.controls.teleports))
				c->updateFromSnapshot(s.value());

		} else if (dynamic_cast<RpgControlRandomizer*>(ptr.get())) {

		} else {
			LOG_CERROR("game") << "Invalid control" << ptr.get();
		}
	}
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

	const int players = m_engine->playerData().size();

	if (!m_rpgGame->load(ptr.value(), players)) {
		LOG_CERROR("game") << "Game load error";
		return;
	}

	TiledScene *firstScene = m_rpgGame->findScene(ptr->firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Game load error: missing first scene";
		return;
	}

	m_tiledGameLoaded = true;

	emit m_rpgGame->gameLoaded();
}



/**
 * @brief ActionRpgMultiplayerGame::onConnected
 */

void ActionRpgMultiplayerGame::onConnected()
{
	LOG_CINFO("game") << "Server connected";

	q->m_connectionLostTimer.setRemainingTime(-1);

	if (q->m_isReconnecting) {
		LOG_CINFO("game") << "FINISH RECONNECTING";
		m_oldGameState = RpgConfig::StatePlay;
		m_config.gameState = RpgConfig::StatePlay;
		changeGameState(RpgConfig::StatePlay);
	}

}



/**
 * @brief ActionRpgMultiplayerGame::onConnectionLost
 */

void ActionRpgMultiplayerGame::onConnectionLost()
{
	LOG_CWARNING("game") << "Connection lost";

	if (m_config.gameState != RpgConfig::StatePlay) {
		disconnectFromHost();
		return;
	}

	if (q->m_connectionLostTimer.isForever()) {
		m_client->snack(tr("Connection lost"));
		q->m_connectionLostTimer.setRemainingTime(CONNECTION_LOST_TIMEOUT);
	}


}


/**
 * @brief ActionRpgMultiplayerGame::onDisconnected
 */

void ActionRpgMultiplayerGame::onDisconnected()
{
	LOG_CWARNING("game") << "Server disconnected" << q->m_isReconnecting;

	if (m_config.gameState != RpgConfig::StatePlay) {
		disconnectFromHost();
	}
}


/**
 * @brief ActionRpgMultiplayerGame::sendData
 * @param data
 * @param reliable
 */

void ActionRpgMultiplayerGame::sendData(const QSerializer &data, const bool &reliable)
{
	sendData(data.toCborMap().toCborValue().toCbor(), reliable);
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

void ActionRpgMultiplayerGame::sendDataChrSel(const int &ban, const bool &lock)
{
	RpgGameData::CharacterSelect config(m_playerConfig);

	if (m_gameMode == MultiPlayerHost) {
		config.gameConfig.terrain = m_playerConfig.terrain;
	}

	config.completed = m_selectionCompleted;
	config.armory = q->getArmory();

	if (ban > 0 && m_gameMode == MultiPlayerHost) {
		QCborMap m = config.toCborMap();
		m.insert(QStringLiteral("ban"), ban);
		sendData(m.toCborValue().toCbor(), true);
		return;
	}

	if (lock && m_gameMode == MultiPlayerHost) {
		QCborMap m = config.toCborMap();
		m.insert(QStringLiteral("lock"), true);
		sendData(m.toCborValue().toCbor(), true);
		return;
	}

	sendData(config, true);
}



/**
 * @brief ActionRpgMultiplayerGame::sendDataPrepare
 */

void ActionRpgMultiplayerGame::sendDataPrepare()
{
	RpgGameData::Prepare config;

	QCborMap map;

	config.prepared = m_gamePrepared && m_playersSynced && m_enemiesSynced && m_randomizerSynced && m_tiledGameLoaded;
	config.loaded = m_tiledGameLoaded;

	if (m_gameMode == MultiPlayerHost)
		config.gameConfig.terrain = m_playerConfig.terrain;

	if (m_gameMode == MultiPlayerHost && m_tiledGameLoaded) {
		if (m_rpgGame) {
			config.gameConfig.positionList = m_rpgGame->playerPositions();
			config.gameConfig.collection = m_rpgGame->collection();
			config.gameConfig.randomizer = m_rpgGame->randomizer();

			config.avg = m_rpgQuestion->count() > 0 ? (float) m_rpgQuestion->duration() * 1000. / (float) m_rpgQuestion->count() : 0.;
			config.gameConfig.duration = m_config.duration + m_rpgGame->m_gameDefinition.duration;
		}

		RpgGameData::CurrentSnapshot snap;

		for (RpgGame::EnemyData &e : m_rpgGame->m_enemyDataList) {
			RpgGameData::EnemyBaseData eData(e.type, -1, e.objectId.sceneId, e.objectId.id);

			if (RpgEnemy *enemy = dynamic_cast<RpgEnemy*>(e.enemy.get()))
				snap.assign(snap.enemies, eData, enemy->serialize(0));
			else
				snap.assign(snap.enemies, eData, RpgGameData::Enemy());

		}


		for (const auto &ptr : m_rpgGame->m_controls) {
			if (RpgControlLight *c = dynamic_cast<RpgControlLight*>(ptr.get())) {
				snap.assign(snap.controls.lights, c->baseData(), c->serialize(0));
			} else if (RpgControlContainer *c = dynamic_cast<RpgControlContainer*>(ptr.get())) {
				snap.assign(snap.controls.containers, c->baseData(), c->serialize(0));
			} else if (RpgControlGate *c = dynamic_cast<RpgControlGate*>(ptr.get())) {
				snap.assign(snap.controls.gates, c->baseData(), c->serialize(0));
			} else if (RpgControlTeleport *c = dynamic_cast<RpgControlTeleport*>(ptr.get())) {
				snap.assign(snap.controls.teleports, c->baseData(), c->serialize(0));
			} else {
				//LOG_CERROR("game") << "Invalid control" << ptr.get();
			}
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
 * @brief ActionRpgMultiplayerGame::sendDataConnect
 */

void ActionRpgMultiplayerGame::sendDataConnect()
{
	RpgGameData::EngineSelector selector;

	if (q->m_requireEngine) {
		selector.operation = RpgGameData::EngineSelector::Create;
	} else if (q->m_selectedEngineId > 0){
		selector.operation = RpgGameData::EngineSelector::Connect;
		selector.engine = q->m_selectedEngineId;
	} else {
		selector.operation = RpgGameData::EngineSelector::List;
	}

	sendData(selector, false);
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
 * @brief ActionRpgMultiplayerGame::overrideCurrentFrame
 * @param tick
 */

void ActionRpgMultiplayerGame::overrideCurrentFrame(const qint64 &tick)
{
	if ((q->m_isReconnecting || q->m_hasReconnected) && m_rpgGame && m_rpgGame->currentFrame() <= 0) {
		m_rpgGame->overrideCurrentFrame(tick);
		m_elapsedTick = tick;
	}
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


/**
 * @brief ActionRpgMultiplayerGamePrivate::resetEngine
 */

void ActionRpgMultiplayerGamePrivate::resetEngine()
{
	LOG_CWARNING("game") << "Reset engine";

	m_engineId = -1;
	m_requireEngine = false;
	m_selectedEngineId = -1;

	d->m_engine->setGameState(RpgConfig::StateConnect);

	d->changeGameState(RpgConfig::StateConnect);
}




/**
 * @brief ActionRpgMultiplayerGamePrivate::getArmory
 * @return
 */

RpgGameData::Armory ActionRpgMultiplayerGamePrivate::getArmory() const
{
	RpgUserWalletList* wallet = d->m_client->server() ? d->m_client->server()->user()->wallet() : nullptr;

	if (!wallet)
		return RpgGameData::Armory();

	return wallet->getArmory(d->m_playerConfig.character);
}



/**
 * @brief ActionRpgMultiplayerGame::canAddEngine
 * @return
 */

bool ActionRpgMultiplayerGame::canAddEngine() const
{
	return q->m_canAddEngine;
}


/**
 * @brief ActionRpgMultiplayerGame::setCanAddEngine
 * @param newCanAddEngine
 */

void ActionRpgMultiplayerGame::setCanAddEngine(bool newCanAddEngine)
{
	if (q->m_canAddEngine == newCanAddEngine)
		return;
	q->m_canAddEngine = newCanAddEngine;
	emit canAddEngineChanged();
}
