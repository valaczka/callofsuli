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



/**
 * Private namespace
 */

class ActionRpgMultiplayerGamePrivate
{
private:
	ActionRpgMultiplayerGamePrivate(ActionRpgMultiplayerGame *game) : d(game) {}

	void updateBody(TiledObjectBody *body);

private:
	RpgGameData::FullSnapshot m_currentSnapshot;

	qint64 m_lastSentTick = -1;
	ClientStorage m_toSend;

	ActionRpgMultiplayerGame *const d;

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
		disconnect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgMultiplayerGame::onGameSuccess);
		disconnect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgMultiplayerGame::onPlayerDead);
		disconnect(m_rpgGame, &RpgGame::gameLoadFailed, this, &ActionRpgMultiplayerGame::onGameLoadFailed);
		disconnect(m_rpgGame, &RpgGame::marketRequest, this, &ActionRpgMultiplayerGame::marketRequest);
		//disconnect(this, &ActionRpgGame::marketUnloaded, m_rpgGame, &RpgGame::onMarketUnloaded);
		//disconnect(this, &ActionRpgGame::marketLoaded, m_rpgGame, &RpgGame::onMarketLoaded);
		m_rpgGame->setRpgQuestion(nullptr);
		m_rpgGame->setFuncPlayerPick(nullptr);
		m_rpgGame->setFuncPlayerHit(nullptr);
		m_rpgGame->setFuncPlayerShot(nullptr);
		m_rpgGame->setFuncPlayerAttackEnemy(nullptr);
		m_rpgGame->setFuncPlayerUseContainer(nullptr);
		m_rpgGame->setFuncPlayerUseCast(nullptr);
		m_rpgGame->setFuncPlayerCastTimeout(nullptr);
		m_rpgGame->setFuncPlayerFinishCast(nullptr);
		m_rpgGame->setFuncEnemyAttackPlayer(nullptr);
	}

	m_rpgGame = newRpgGame;
	emit rpgGameChanged();

	if (m_rpgGame) {
		setGameQuestion(m_rpgGame->gameQuestion());
		m_rpgGame->setRpgQuestion(m_rpgQuestion.get());
		connect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgMultiplayerGame::onGameSuccess, Qt::QueuedConnection);		// Azért kell, mert különbön az utolsó fegyverhasználatot nem számolja el a szerveren
		connect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgMultiplayerGame::onPlayerDead);
		connect(m_rpgGame, &RpgGame::gameLoadFailed, this, &ActionRpgMultiplayerGame::onGameLoadFailed);
		connect(m_rpgGame, &RpgGame::marketRequest, this, &ActionRpgMultiplayerGame::marketRequest);
		//connect(this, &ActionRpgGame::marketUnloaded, m_rpgGame, &RpgGame::onMarketUnloaded);
		//connect(this, &ActionRpgGame::marketLoaded, m_rpgGame, &RpgGame::onMarketLoaded);

		m_rpgGame->m_funcBodyStep = std::bind(&ActionRpgMultiplayerGame::worldStep, this, std::placeholders::_1);
		m_rpgGame->m_funcBeforeWorldStep = std::bind(&ActionRpgMultiplayerGame::beforeWorldStep, this, std::placeholders::_1);
		m_rpgGame->m_funcAfterWorldStep = std::bind(&ActionRpgMultiplayerGame::afterWorldStep, this, std::placeholders::_1);

		m_rpgGame->setFuncPlayerHit(std::bind(&ActionRpgMultiplayerGame::onPlayerHit, this,
											  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		m_rpgGame->setFuncPlayerShot(std::bind(&ActionRpgMultiplayerGame::onPlayerShot, this,
											   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
											   std::placeholders::_4, std::placeholders::_5));

		//m_rpgGame->m_funcTimeStep = std::bind(&RpgUdpEngine::timeStepped, d);



		/*m_rpgGame->setFuncPlayerPick(std::bind(&ActionRpgMultiplayerGame::onPlayerPick, this, std::placeholders::_1, std::placeholders::_2));
		m_rpgGame->setFuncPlayerAttackEnemy(std::bind(&ActionRpgMultiplayerGame::onPlayerAttackEnemy, this,
													  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_rpgGame->setFuncPlayerUseContainer(std::bind(&ActionRpgMultiplayerGame::onPlayerUseContainer, this, std::placeholders::_1, std::placeholders::_2));
		m_rpgGame->setFuncPlayerUseCast(std::bind(&ActionRpgMultiplayerGame::onPlayerUseCast, this, std::placeholders::_1));
		m_rpgGame->setFuncPlayerCastTimeout(std::bind(&ActionRpgMultiplayerGame::onPlayerCastTimeout, this, std::placeholders::_1));
		m_rpgGame->setFuncPlayerFinishCast(std::bind(&ActionRpgMultiplayerGame::onPlayerFinishCast, this, std::placeholders::_1));*/
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

	syncPlayerList();
	syncEnemyList();

	locker.unlock();

	if (m_config.gameState == RpgConfig::StateError) {
		//disconnectFromHost();
		return;
	}

	//LOG_CDEBUG("game")	<< "timer override";
	if (m_config.gameState == RpgConfig::StateInvalid)
		return;


	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		sendDataChrSel();
	else if (m_config.gameState == RpgConfig::StatePrepare)
		sendDataPrepare();
	else if (m_config.gameState == RpgConfig::StatePlay)
		sendDataPlay();


	//locker.relock();

	if (tick > 0)
		m_engine->m_snapshots.zapSnapshots(tick - 2*RPG_UDP_DELTA_MSEC);

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
	/*if (!m_rpgGame || m_engine->m_snapshots.enemies.empty())
		return;*/


	m_enemiesSynced = true; return;

	/*
	if (m_enemiesSynced && (qsizetype) snapshot.enemies.size() == m_rpgGame->m_enemyDataList.size())
		return;

	m_enemiesSynced = false;

	QList<RpgGameData::Enemy> onlyServerEnemy;

	QList<int> localIndices;

	for (int i=0; i<m_rpgGame->m_enemyDataList.size(); ++i)
		localIndices.append(i);

	for (const RpgGameData::Enemy &enemy : snapshot.enemies) {
		const auto it = std::find_if(m_rpgGame->m_enemyDataList.cbegin(),
									 m_rpgGame->m_enemyDataList.cend(),
									 [&enemy](const RpgGame::EnemyData &d){
			return d.objectId.sceneId == enemy.s && d.objectId.id == enemy.id;
		});

		if (it == m_rpgGame->m_enemyDataList.cend()) {
			LOG_CWARNING("game") << "Missing enemy" << enemy.s << enemy.id;
			onlyServerEnemy.append(enemy);
		} else {
			localIndices.removeAll(it - m_rpgGame->m_enemyDataList.cbegin());
		}
	}

	if (!localIndices.empty())
		LOG_CWARNING("game") << "Only local enemies" << localIndices;

	m_enemiesSynced = onlyServerEnemy.isEmpty() && localIndices.isEmpty();*/
}



/**
 * @brief ActionRpgMultiplayerGame::syncPlayerList
 * @param list
 */

void ActionRpgMultiplayerGame::syncPlayerList()
{
	QMutexLocker locker (&m_engine->m_mutex);

	if (!m_rpgGame || m_engine->m_snapshots.players().empty())
		return;

	if (m_playersSynced && (qsizetype) m_engine->m_snapshots.players().size() == m_rpgGame->m_players.size())
		return;

	m_playersSynced = false;

	QList<int> localIndices;

	for (int i=0; i<m_rpgGame->m_players.size(); ++i)
		localIndices.append(i);

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

		} else {
			localIndices.removeAll(it - m_rpgGame->m_players.constBegin());
		}
	}

	if (!playerList.isEmpty()) {
		playerList.append(m_rpgGame->players());
		m_rpgGame->setPlayers(playerList);
	}

	if (!localIndices.empty())
		LOG_CWARNING("game") << "Only local players" << localIndices;

	LOG_CDEBUG("game") << "SYNCED";

	m_playersSynced = true; //localIndices.isEmpty();
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
	LOG_CINFO("game") << "MYID" << m_playerId;
}





/**
 * @brief ActionRpgMultiplayerGame::createPlayer
 * @param scene
 * @param config
 */

RpgPlayer* ActionRpgMultiplayerGame::createPlayer(TiledScene *scene,
												  const RpgGameData::BaseData &config,
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


	RpgPlayer *player = m_rpgGame->createPlayer(scene, *characterPtr, config.o);

	if (!player) {
		LOG_CERROR("game") << "Player create error";
		return nullptr;
	}

	if (characterPtr->cast != RpgPlayerCharacterConfig::CastInvalid) {
		loadWeapon(player, TiledWeapon::WeaponMageStaff);
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
					   RpgArmory::weaponHash().key(s, TiledWeapon::WeaponInvalid),
					   /*(*it)->bullet() ? (*it)->bullet()->amount() : 0*/
					   (*it)->market().cost == 0 ? -1 : (*it)->amount());
		}
	}



	if (playerData.p.size() > 1)
		player->emplace(playerData.p.at(0), playerData.p.at(1));

	player->setCurrentAngle(playerData.a);
	//player->setCurrentSceneStartPosition(d->m_playerPosition.value_or(QPointF{0,0}));

	return player;
}





bool ActionRpgMultiplayerGame::onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable)
{
	return false;
}

bool ActionRpgMultiplayerGame::onPlayerAttackEnemy(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType)
{
	return false;
}

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
 * @param weaponType
 * @return
 */

bool ActionRpgMultiplayerGame::onPlayerHit(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType)
{
	if (!player)
		return false;

	LOG_CWARNING("game") << "PLAYER" << m_playerId << "HIT" << enemy << weaponType << m_rpgGame->tickTimer()->currentTick();

	RpgGameData::Player p = player->serialize(m_rpgGame->tickTimer()->currentTick());
	p.st = RpgGameData::Player::PlayerHit;

	q->m_toSend.appendSnapshot(player->baseData(), p);

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

bool ActionRpgMultiplayerGame::onPlayerShot(RpgPlayer *player, const TiledWeapon::WeaponType &weaponType, TiledScene *scene,
											const IsometricBullet::Targets &targets, const qreal &angle)
{
	LOG_CWARNING("game") << "PLAYER" << player << "SHOT" << weaponType << scene << targets << angle;

	return true;
}




/**
 * @brief ActionRpgMultiplayerGame::onEnemyAttackPlayer
 * @param enemy
 * @param player
 * @param weaponType
 * @return
 */

bool ActionRpgMultiplayerGame::onEnemyAttackPlayer(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	return false;
}




/**
 * @brief ActionRpgMultiplayerGame::beforeWorldStep
 */

void ActionRpgMultiplayerGame::beforeWorldStep(const qint64 &lagMsec)
{
	Q_ASSERT(q);

	q->m_currentSnapshot = m_engine->m_snapshots.getFullSnapshot(m_rpgGame->tickTimer()->currentTick()-lagMsec);
}





/**
 * @brief ActionRpgMultiplayerGame::afterWorldStep
 */

void ActionRpgMultiplayerGame::afterWorldStep(const qint64 &lagMsec)
{
	Q_ASSERT(q);

	q->m_currentSnapshot.clear();

	if (m_config.gameState != RpgConfig::StatePlay)
		return;

	const qint64 tick = m_rpgGame->tickTimer()->currentTick() - lagMsec;

	const bool forceKeyFrame = q->m_lastSentTick < 0 || (tick - q->m_lastSentTick >= 1000./30.);

	for (auto &b : m_rpgGame->bodyList()) {
		RpgGameDataInterface<RpgGameData::Player, RpgGameData::BaseData> *iface =
				dynamic_cast<RpgGameDataInterface<RpgGameData::Player, RpgGameData::BaseData>*> (b.get());

		if (!iface)
			continue;

		if (m_rpgGame->controlledPlayer() == iface && forceKeyFrame) {
			q->m_toSend.appendSnapshot(iface->baseData(), iface->serialize(tick));
		}
	}


	if (!forceKeyFrame)
		return;

	RpgGameData::CurrentSnapshot snapshot = q->m_toSend.getCurrentSnapshot();
	if (QCborMap map = snapshot.toCbor(); !map.isEmpty()) {
		const QByteArray &data = map.toCborValue().toCbor();
		sendData(data, true);
	}

	q->m_lastSentTick = tick;

	q->m_toSend.clear();
}



/**
 * @brief ActionRpgMultiplayerGame::worldStep
 * @param body
 */

void ActionRpgMultiplayerGame::worldStep(TiledObjectBody *body)
{
	if (m_config.gameState != RpgConfig::StatePlay || !body)
		return;

	if (body->objectId().ownerId == m_playerId /*|| (body.owner == -1 && m_isHost)*/ ) {
		// TODO: snapshot reconciliation

		body->worldStep();
	} else {
		q->updateBody(body);
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
			for (const TiledGame::PlayerPosition &p : m_rpgGame->playerPositionList()) {
				gameConfig.positionList.emplaceBack(p.sceneId, p.position.x(), p.position.y());
			}
		}

		gameConfig.terrain = m_playerConfig.terrain;

		QCborMap cfgMap = gameConfig.toCborMap(true);
		map.insert(QStringLiteral("cfg"), cfgMap);


		QCborArray eList;

		for (RpgGame::EnemyData &e : m_rpgGame->m_enemyDataList) {
			RpgGameData::EnemyBaseData enemy(e.type, 99, e.objectId.sceneId, e.objectId.id);
			eList.append(enemy.toCborMap(true));
		}

		map.insert(QStringLiteral("ee"), eList);

		config.prepared = m_gamePrepared;
	} else {
		config.prepared = m_gamePrepared && m_playersSynced && m_enemiesSynced;
	}


	QCborMap cfgMap = config.toCborMap(true);
	map.insert(QStringLiteral("pr"), cfgMap);

	sendData(map.toCborValue().toCbor(), true);
}




/**
 * @brief ActionRpgMultiplayerGame::sendDataPlay
 */

void ActionRpgMultiplayerGame::sendDataPlay()
{
	// THIS IS AFTER WORLD STEP !!!

	QCborMap map;

	if (m_gameMode == MultiPlayerHost) {
		// Enemies
	}


	if (!m_rpgGame)
		return;
	/*
	if (m_rpgGame->controlledPlayer()) {
		RpgGameData::Player p;
		RpgGameData::PickableBaseData pd;

		p.f = m_rpgGame->tickTimer()->currentTick();
		pd.o = m_playerId;
		pd.id = m_rpgGame->controlledPlayer()->objectId().id;
		pd.s = m_rpgGame->controlledPlayer()->objectId().sceneId;

		b2Vec2 pos = m_rpgGame->controlledPlayer()->body().GetPosition();
		p.p = { pos.x, pos.y };
		p.a = m_rpgGame->controlledPlayer()->currentAngle();
		p.hp = m_rpgGame->controlledPlayer()->hp();

		const b2Vec2 vel = m_rpgGame->controlledPlayer()->body().GetLinearVelocity();
		if (vel.x != 0. || vel.y != 0.)
			p.st = RpgGameData::Player::PlayerMoving;
		else
			p.st = RpgGameData::Player::PlayerIdle;

		map.insert(QStringLiteral("pd"), pd.toCborMap(true));
		map.insert(QStringLiteral("p"), p.toCborMap(true));

		sendData(map.toCborValue().toCbor(), true);
	}
*/

	/*static QFile *tmpFile = nullptr;

	map.insert(QStringLiteral("tick"), m_rpgGame->tickTimer()->currentTick());

	QByteArray d = QJsonDocument(map.toJsonObject()).toJson();

	if (!tmpFile) {
		tmpFile = new QFile("/tmp/_data.json");
		tmpFile->open(QIODevice::WriteOnly);
		tmpFile->write("[\n");
	}


	tmpFile->write(d);
	tmpFile->write(",\n");
	tmpFile->flush();*/

}


/**
 * @brief ActionRpgMultiplayerGame::setTickTimer
 * @param tick
 */

void ActionRpgMultiplayerGame::setTickTimer(const qint64 &tick)
{
	if (m_rpgGame) {
		if (m_rpgGame->tickTimer()->startTick() < tick)
			m_rpgGame->tickTimer()->start(this, tick);
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

void ActionRpgMultiplayerGamePrivate::updateBody(TiledObjectBody *body)
{
	Q_ASSERT(d);

	if (RpgPlayer *p = dynamic_cast<RpgPlayer*>(body)) {
		const auto ptr = m_currentSnapshot.getPlayer(p->baseData());

		if (!ptr) {
			LOG_CERROR("game") << "Invalid player" << p;
			return;
		} else {
			p->updateFromSnapshot(ptr.value());
		}
	}

}
