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
#include "qcborarray.h"
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
	RpgGameData::FullSnapshot m_currentSnapshot;

	qint64 m_lastSentTick = -1;
	ClientStorage m_toSend;

	ActionRpgMultiplayerGame *const d;

	friend class ActionRpgMultiplayerGame;

	cpSpace *m_space = nullptr;
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

	q->m_space = cpSpaceNew();
}




/**
 * @brief ActionRpgMultiplayerGame::~ActionRpgMultiplayerGame
 */

ActionRpgMultiplayerGame::~ActionRpgMultiplayerGame()
{
	cpSpaceFree(q->m_space);
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
		connect(m_rpgGame, &RpgGame::marketRequest, this, &ActionRpgMultiplayerGame::marketRequest);
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


	if (m_config.gameState == RpgConfig::StatePlay) {
		LOG_CINFO("game") << "!!!! stop keepalive timer";
		m_keepAliveTimer.stop();
	}

	ActionRpgGame::onConfigChanged();
}


/**
 * @brief ActionRpgMultiplayerGame::timerEvent
 */

void ActionRpgMultiplayerGame::timerEvent(QTimerEvent *)
{
	QMutexLocker locker (&m_engine->m_mutex);

	const qint64 tick = m_rpgGame ? m_rpgGame->tickTimer()->currentTick() : -2;

	if (m_engine->m_gameState == RpgConfig::StateInvalid)
		return;

	changeGameState(m_engine->m_gameState);
	setGameMode(m_engine->m_isHost ? ActionRpgGame::MultiPlayerHost : ActionRpgGame::MultiPlayerGuest);
	setPlayerId(m_engine->m_playerId);

	if (!m_engine->m_isHost && !m_engine->m_gameConfig.terrain.isEmpty())
		worldTerrainSelect(m_engine->m_gameConfig.terrain, true);

	updatePlayersModel(m_engine->getPlayerList());

	locker.unlock();

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


	//locker.relock();

	if (tick > 0)
		m_engine->m_snapshots.zapSnapshots(tick - 3*RPG_UDP_DELTA_TICK);

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


void ActionRpgMultiplayerGame::syncEnemyList()
{
	QMutexLocker locker (&m_engine->m_mutex);

	if (!m_rpgGame || m_engine->m_snapshots.enemies().empty())
		return;

	if (m_enemiesSynced && (qsizetype) m_engine->m_snapshots.enemies().size() == m_rpgGame->m_enemyDataList.size())
		return;

	m_enemiesSynced = false;

	QList<RpgGameData::EnemyBaseData> onlyServerEnemy;

	QList<int> localIndices;

	for (int i=0; i<m_rpgGame->m_enemyDataList.size(); ++i)
		localIndices.append(i);

	for (const auto &pe : m_engine->m_snapshots.enemies()) {
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

void ActionRpgMultiplayerGame::syncPlayerList()
{
	QMutexLocker locker (&m_engine->m_mutex);

	if (!m_rpgGame)
		return;

	if (m_playersSynced && (qsizetype) m_engine->m_snapshots.players().size() == m_rpgGame->m_players.size())
		return;

	m_playersSynced = false;

	QList<RpgPlayer*> playerList;

	for (const auto &pl : m_engine->m_snapshots.players()) {
		const auto it = std::find_if(m_rpgGame->m_players.constBegin(),
									 m_rpgGame->m_players.constEnd(),
									 [&pl](const RpgPlayer *p){
			return p->objectId().sceneId == pl.data.s && p->objectId().id == pl.data.id;
		});

		if (it == m_rpgGame->m_players.constEnd()) {
			LOG_CWARNING("game") << "Missing player" << pl.data.s << pl.data.id << pl.data.o;

			RpgGameData::Player pd;

			if (pl.list.size() > 0) {
				LOG_CWARNING("game") << "FIRST KEY" << pl.list.cbegin()->first;
				pd = pl.list.cbegin()->second;
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

			const int hp = m_missionLevel->startHP();

			rpgPlayer->setHp(hp);
			rpgPlayer->setMaxHp(hp);

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

	m_playersSynced = !m_engine->m_snapshots.players().empty();
}



/**
 * @brief ActionRpgMultiplayerGame::syncBulletList
 */

void ActionRpgMultiplayerGame::syncBulletList()
{
	QMutexLocker locker (&m_engine->m_mutex);

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

	for (const auto &b : m_engine->m_snapshots.bullets()) {
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

	QMutexLocker locker (&m_engine->m_mutex);
	const auto it = std::find_if(m_engine->m_playerData.cbegin(),
								 m_engine->m_playerData.cend(),
								 [&config](const RpgGameData::CharacterSelect &ch){
		return (ch.playerId == config.o);
	});

	if (it == m_engine->m_playerData.cend()) {
		LOG_CERROR("game") << "Invalid player id" << config.o;
		return nullptr;
	}

	const RpgGameData::CharacterSelect chData = *it;

	locker.unlock();

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

	if (characterPtr->cast != RpgPlayerCharacterConfig::CastInvalid) {
		loadWeapon(player, RpgGameData::Weapon::WeaponMageStaff);
	}


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



	if (playerData.p.size() > 1)
		player->emplace(playerData.p.at(0), playerData.p.at(1));

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
	QMutexLocker locker (&m_engine->m_mutex);

	syncPlayerList();
	syncEnemyList();
	syncBulletList();
}



/**
 * @brief ActionRpgMultiplayerGame::onTimeStepped
 */

void ActionRpgMultiplayerGame::onTimeStepped()
{
	QMutexLocker locker (&m_engine->m_mutex);

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

	q->m_currentSnapshot = m_engine->m_snapshots.getFullSnapshot(tick);
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

	const bool forceKeyFrame = q->m_lastSentTick < 0 || (tick - q->m_lastSentTick >= 2) || q->m_toSend.hasSnapshot();

	if (!forceKeyFrame)
		return;

	m_rpgGame->iterateOverBodies([this, &tick](TiledObjectBody *b){
		if (RpgPlayer *iface = dynamic_cast<RpgPlayer*> (b)) {
			if (m_rpgGame->controlledPlayer() == iface) {
				q->m_toSend.appendSnapshot(iface->baseData(), iface->serialize(tick));
			}
		} else if (RpgBullet *iface = dynamic_cast<RpgBullet*> (b);
				   iface && b->objectId().ownerId == m_playerId &&
				   iface->stage() != RpgGameData::LifeCycle::StageDestroy) {
			q->m_toSend.appendSnapshot(iface->baseData(), iface->serialize(tick));
		}
	});


	if (m_gameMode == MultiPlayerHost) {
		m_rpgGame->iterateOverBodies([this, &tick](TiledObjectBody *b){
			RpgEnemy *iface = dynamic_cast<RpgEnemy*> (b);
			if (iface) {
				q->m_toSend.appendSnapshot(iface->baseData(), iface->serialize(tick));
			}
		});
	}




	RpgGameData::CurrentSnapshot snapshot = q->m_toSend.renderCurrentSnapshot();

	if (QCborMap map = snapshot.toCbor(); !map.isEmpty()) {


#ifdef WITH_FTXUI
		if (DesktopApplication *a = dynamic_cast<DesktopApplication*>(Application::instance())) {
			static QStringList txt;
			static qint64 lastF = -1;

			QCborArray pp = map.value(QStringLiteral("pp")).toArray();

			for (const QCborValue &v : pp) {
				const QCborMap &m = v.toMap();

				QCborArray p = m.value(QStringLiteral("p")).toArray();

				for (const QCborValue &v : p) {
					const QCborMap &m = v.toMap();

					qint64 f = m.value("f").toInteger(-1);

					const QCborArray &pos = m.value("p").toArray();
					const QCborArray &cv = m.value("cv").toArray();

					txt += QStringLiteral("%1: %2 (%3,%4) (%5,%6) %7  || %8")
						   .arg(m.value("f").toInteger(-1))
						   .arg(m.value("st").toInteger(-1))
						   .arg(pos.size() > 1 ? pos.at(0).toDouble(-1) : -1)
						   .arg(pos.size() > 1 ? pos.at(1).toDouble(-1) : -1)
						   .arg(cv.size() > 1 ? cv.at(0).toDouble(-1) : -1)
						   .arg(cv.size() > 1 ? cv.at(1).toDouble(-1) : -1)
						   .arg(m.value("a").toDouble(0))
						   .arg(m.value("arm").toMap().value("cw").toInteger(-1))
						   + (f <= lastF ? QString("****") : "")
						   ;

					lastF = f;
				}
			}

			txt += "----------------------------------";

			for (int i=txt.size() - 480; i>0; --i)
				txt.removeFirst();


			QCborMap map;
			map.insert(QStringLiteral("mode"), QStringLiteral("SND"));

			QString t;

			for (auto it = txt.crbegin(); it != txt.crend(); ++it)
				t += *it + QStringLiteral("\n");

			map.insert(QStringLiteral("txt"), t);
			a->writeToSocket(map.toCborValue());

		}
#endif



		sendData(map.toCborValue().toCbor(), true);
	}

	q->m_lastSentTick = tick;

	q->m_toSend.clear();
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

	/*RpgGameData::Enemy e = enemy->serialize();

	e.attacked(enemy->baseData(), weaponType, player->baseData());

	int xp = std::max(0, e.hp-enemy->hp());
	setXp(m_xp+xp);

	enemy->attackedByPlayer(player, weaponType);

	enemy->updateFromSnapshot(e);

	return true;*/


	if (!player || player->objectId().ownerId != m_playerId)
		return false;

	LOG_CWARNING("game") << "PLAYER" << m_playerId << "ATTACK" << enemy << m_rpgGame->tickTimer()->currentTick();

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerAttack;
	p.tg = enemy->baseData();

	q->m_toSend.appendSnapshot(player->baseData(), p);

	return true;
}




/**
 * @brief ActionRpgMultiplayerGame::onPlayerUseContainer
 * @param player
 * @param container
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerUseContainer(RpgPlayer *player, RpgContainer *container)
{
	return false;
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
	player->m_lastSnapshot = p;

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
	player->m_lastSnapshot = p;

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

	q->m_toSend.appendSnapshot(player->baseData(), p);
	player->m_lastSnapshot = p;

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

	LOG_CWARNING("game") << "ENEMY" << enemy << "HIT" << player->objectId().ownerId << weapon << m_rpgGame->tickTimer()->currentTick();

	if (!ActionRpgGame::onEnemyHit(enemy, player, weapon))
		return false;

	RpgGameData::Enemy e = enemy->serialize(m_rpgGame->tickTimer()->currentTick());
	e.st = RpgGameData::Enemy::EnemyHit;

	q->m_toSend.appendSnapshot(enemy->baseData(), e);
	enemy->m_lastSnapshot = e;

	return true;
}




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

	/*RpgGameData::Player p = player->serialize();
	p.attacked(player->baseData(), weaponType, enemy->baseData());

	const bool prot = p.hp == player->hp();

	player->updateFromSnapshot(p);
	player->attackedByEnemy(enemy, weaponType, prot);*/

	LOG_CWARNING("game") << "ENEMY" << enemy << "ATTACK" << player->objectId().ownerId << m_rpgGame->tickTimer()->currentTick();

	RpgGameData::Enemy e = enemy->serialize(m_rpgGame->tickTimer()->currentTick());
	e.st = RpgGameData::Enemy::EnemyAttack;
	e.tg = player->baseData();

	q->m_toSend.appendSnapshot(enemy->baseData(), e);

	///player->attackedByEnemy(enemy, weaponType, prot);

	return true;

}



/**
 * @brief ActionRpgMultiplayerGame::onBulletImpact
 * @param bullet
 * @param other
 * @return
 */

bool ActionRpgMultiplayerGame::onBulletImpact(RpgBullet *bullet, TiledObjectBody *other)
{
	if (!bullet)
		return false;

	if (bullet->ownerId().o != m_playerId)
		return false;

	if (!ActionRpgGame::onBulletImpact(bullet, other))
		return false;

	RpgGameData::Bullet p = bullet->serialize(m_rpgGame->tickTimer()->currentTick());
	bullet->m_lastSnapshot = p;

	return true;
}







/**
 * @brief ActionRpgMultiplayerGame::worldStep
 * @param body
 */

bool ActionRpgMultiplayerGame::onBodyStep(TiledObjectBody *body)
{
	if (m_config.gameState != RpgConfig::StatePlay || !body)
		return true;

	q->updateBody(body,
				  body->objectId().ownerId == m_playerId || (body->objectId().ownerId == -1 && m_engine->m_isHost)
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

void ActionRpgMultiplayerGame::sendData(const QByteArray &data, const bool &reliable)
{
	m_engine->sendMessage(data, reliable);
}


/**
 * @brief ActionRpgMultiplayerGame::sendDataChrSel
 */

void ActionRpgMultiplayerGame::sendDataChrSel()
{
	QCborMap map;

	if (m_gameMode == MultiPlayerHost) {
		RpgGameData::GameConfig gameConfig;
		gameConfig.terrain = m_playerConfig.terrain;

		QCborMap cfgMap = gameConfig.toCborMap(true);
		map.insert(QStringLiteral("cfg"), cfgMap);
	}

	RpgGameData::CharacterSelect config(m_playerConfig);
	config.completed = m_selectionCompleted;

	QCborMap cfgMap = config.toCborMap(true);
	map.insert(QStringLiteral("chr"), cfgMap);

	sendData(map.toCborValue().toCbor(), true);
}



/**
 * @brief ActionRpgMultiplayerGame::sendDataPrepare
 */

void ActionRpgMultiplayerGame::sendDataPrepare()
{
	QCborMap map;

	RpgGameData::Prepare config;

	if (m_gameMode == MultiPlayerHost) {
		RpgGameData::GameConfig gameConfig;

		if (m_rpgGame) {
			for (const RpgGame::PlayerPosition &p : m_rpgGame->m_playerPositionList) {
				if (p.scene)
					gameConfig.positionList.emplaceBack(p.scene->sceneId(), p.position.x(), p.position.y());
				else
					LOG_CERROR("game") << "Missing scene" << p.position;
			}
		}

		gameConfig.terrain = m_playerConfig.terrain;

		QCborMap cfgMap = gameConfig.toCborMap(true);
		map.insert(QStringLiteral("cfg"), cfgMap);


		QCborArray eList;

		for (RpgGame::EnemyData &e : m_rpgGame->m_enemyDataList) {
			RpgGameData::EnemyBaseData enemy(e.type, -1, e.objectId.sceneId, e.objectId.id);
			eList.append(enemy.toCborMap(true));
		}

		map.insert(QStringLiteral("ee"), eList);

	}

	config.prepared = m_gamePrepared && m_playersSynced && m_enemiesSynced;


	QCborMap cfgMap = config.toCborMap(true);
	map.insert(QStringLiteral("pr"), cfgMap);

	sendData(map.toCborValue().toCbor(), true);
}



/**
 * @brief ActionRpgMultiplayerGame::setTickTimer
 * @param tick
 */

void ActionRpgMultiplayerGame::setTickTimer(const qint64 &tick)
{
	if (m_rpgGame) {
		const qint64 curr = m_rpgGame->tickTimer()->currentTick();
		const qint64 diff = curr - tick;
		if (diff < -2*RPG_UDP_DELTA_TICK) {
			LOG_CWARNING("game") << "Tick timer difference error" << curr << tick << m_rpgGame->tickTimer()->startTick();
			m_rpgGame->tickTimer()->start(this, tick);
		} else if (diff > RPG_UDP_DELTA_TICK * 0.5) {
			LOG_CERROR("game") << "Tick timer difference error" << curr << tick << m_rpgGame->tickTimer()->startTick();
			m_rpgGame->tickTimer()->start(this, tick);
		}
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

	if (RpgPlayer *p = dynamic_cast<RpgPlayer*>(body)) {
		const auto &ptr = m_currentSnapshot.getPlayer(p->baseData());

		if (!ptr) {
			if (isHosted)
				p->worldStep();
			else
				LOG_CERROR("game") << "INVALID ptr";
			return;
		} else {
			if (isHosted) {
				if (ptr->last.f >= 0)
					p->updateFromLastSnapshot(ptr->last);
				p->worldStep();
			} else {
				p->updateFromSnapshot(ptr.value());

			}
		}
	} /*else if (RpgEnemy *p = dynamic_cast<RpgEnemy*>(body)) {
		const auto &ptr = m_currentSnapshot.getEnemy(p->baseData());

		if (!ptr) {
			if (isHosted)
				body->worldStep();
			return;
		} else {
			if (isHosted) {
				if (ptr->last.f >= 0)
					p->updateFromLastSnapshot(ptr->last);
				p->worldStep();
			} else {
				p->updateFromSnapshot(ptr.value());
			}
		}
	} else if (RpgBullet *p = dynamic_cast<RpgBullet*>(body)) {
		const auto &ptr = m_currentSnapshot.getBullet(p->baseData());

		if (!ptr) {
			if (isHosted)
				body->worldStep();
			return;
		} else {
			if (isHosted) {
				if (ptr->last.f >= 0)
					p->updateFromLastSnapshot(ptr->last);
				p->worldStep();
			} else {
				p->updateFromSnapshot(ptr.value());
			}
		}
	} else if (isHosted) {
		body->worldStep();
	} */

}
