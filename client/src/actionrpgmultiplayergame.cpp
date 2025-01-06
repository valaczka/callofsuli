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
#include "actionrpgmultiplayergame_p.h"
#include "client.h"
#include "qcborarray.h"
#include "rpgplayer.h"
#include "utils_.h"



/**
 * @brief ActionRpgMultiplayerGame::ActionRpgMultiplayerGame
 * @param missionLevel
 * @param client
 */

ActionRpgMultiplayerGame::ActionRpgMultiplayerGame(GameMapMissionLevel *missionLevel, Client *client)
	: ActionRpgGame(missionLevel, client)
	, m_playersModel(std::make_unique<QSListModel>())
	, d(new RpgUdpEnginePrivate(this))
{
	LOG_CDEBUG("game") << "ActionRpgMultiplayerGame constructed";

	setGameMode(MultiPlayerGuest);

	m_playersModel->setRoleNames(Utils::getRolesFromObject(RpgGameData::CharacterSelect().metaObject()));

	d->moveToThread(&m_dThread);
	QObject::connect(&m_dThread, &QThread::finished, d, &QObject::deleteLater);
	m_dThread.start();

	if (m_client->server()) {
		if (User *u = m_client->server()->user()) {
			m_playerConfig.username = u->username();
			m_playerConfig.nickname = u->fullNickName();
		}
	}

	m_keepAliveTimer.start(1000, this);
}


/**
 * @brief ActionRpgMultiplayerGame::~ActionRpgMultiplayerGame
 */

ActionRpgMultiplayerGame::~ActionRpgMultiplayerGame()
{
	d->stop();
	d = nullptr;
	m_dThread.quit();
	m_dThread.wait();

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

		m_rpgGame->m_funcBodyStep = std::bind(&RpgUdpEnginePrivate::worldStep, d, std::placeholders::_1);
		//m_rpgGame->m_funcBeforeWorldStep = std::bind(&ActionRpgMultiplayerGame::beforeWorldStep, this);
		m_rpgGame->m_funcAfterWorldStep = std::bind(&ActionRpgMultiplayerGame::afterWorldStep, this);
		m_rpgGame->m_funcTimeStep = std::bind(&RpgUdpEnginePrivate::timeStepped, d);



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
 * @brief ActionRpgMultiplayerGame::connectToHost
 */

void RpgUdpEnginePrivate::connectToServer()
{
	Server *s = q->m_client->server();
	if (!s)
		return;

	if (connectToHost(s->host().toLatin1(), s->port()))
		run();
}


/**
 * @brief ActionRpgMultiplayerGame::disconnect
 */

void ActionRpgMultiplayerGame::disconnectFromHost()
{
	d->disconnect();
}


/**
 * @brief ActionRpgMultiplayerGame::isConnected
 * @return
 */

bool ActionRpgMultiplayerGame::isConnected() const
{
	return d->m_enet_host != nullptr && d->m_enet_peer != nullptr;
}



/**
 * @brief ActionRpgMultiplayerGame::onConfigChanged
 */

void ActionRpgMultiplayerGame::onConfigChanged()
{
	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	/*if (m_config.gameState == RpgConfig::StateConnect) {
		if (!isConnected()) {
			LOG_CDEBUG("game") << "CONNECT";
			QMetaObject::invokeMethod(d, &RpgUdpEnginePrivate::connectToServer);
			return;
		}
		return;
	}*/

	if (m_config.gameState == RpgConfig::StatePlay)
		m_keepAliveTimer.stop();

	ActionRpgGame::onConfigChanged();

	/*if (m_config.gameState == RpgConfig::StateCharacterSelect) {
		QMetaObject::invokeMethod(d, &RpgUdpEnginePrivate::connectToServer);
	}*/
}


/**
 * @brief ActionRpgMultiplayerGame::timerEvent
 */

void ActionRpgMultiplayerGame::timerEvent(QTimerEvent *)
{
	//LOG_CDEBUG("game")	<< "timer override";
	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (m_config.gameState == RpgConfig::StateConnect && !isConnected()) {
		LOG_CDEBUG("game") << "CONNECT";
		QMetaObject::invokeMethod(d, &RpgUdpEnginePrivate::connectToServer);
		return;
	}

	if ((m_config.gameState == RpgConfig::StateCharacterSelect ||
		 m_config.gameState == RpgConfig::StateDownloadContent ||
		 m_config.gameState == RpgConfig::StatePrepare ||
		 m_config.gameState == RpgConfig::StatePlay) &&
			!isConnected()) {
		LOG_CDEBUG("game") << "RECONNECT";
		QMetaObject::invokeMethod(d, &RpgUdpEnginePrivate::connectToServer);
		return;
	}

	if (m_config.gameState == RpgConfig::StateCharacterSelect)
		sendDataChrSel();

}

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

RpgPlayer* ActionRpgMultiplayerGame::createPlayer(TiledScene *scene, const RpgPlayerConfig &config)
{
	if (!m_rpgGame) {
		LOG_CERROR("game") << "Missing RpgGame";
		return nullptr;

	}
	const auto characterPtr = RpgGame::characters().find(config.character);

	if (characterPtr == RpgGame::characters().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid character" << config.character;
		return nullptr;
	}

	RpgPlayer *player = m_rpgGame->createPlayer(scene, *characterPtr);

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
		for (const QString &s : config.weapons) {
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



	//player->emplace(m_rpgGame->playerPositionsCount());
	player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::West));
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

bool ActionRpgMultiplayerGame::onEnemyAttackPlayer(IsometricEnemy *enemy, RpgPlayer *player, const TiledWeapon::WeaponType &weaponType)
{
	return false;
}

void ActionRpgMultiplayerGame::beforeWorldStep()
{

}



/**
 * @brief ActionRpgMultiplayerGame::afterWorldStep
 */

void ActionRpgMultiplayerGame::afterWorldStep()
{
	if (m_config.gameState != RpgConfig::StatePlay)
		return;

	QCborArray enemies, enemiesKF;
	QCborArray players, playersKF;

	bool forceKeyFrame = d->forceKeyFrame();

	for (TiledGame::Body &b : m_rpgGame->bodyList()) {
		RpgGameDataInterface *iface = dynamic_cast<RpgGameDataInterface*>(b.body.get());

		if (!iface)
			continue;

		if (const auto &p = iface->serialize<RpgGameData::Player>()) {
			if (iface->keyFrameRequired() || forceKeyFrame) {
				QCborMap map = p->toCborMap();
				b.lastState = map;
				playersKF.append(map);
				iface->setKeyFrameRequired(false);
			} else {
				QCborMap map = p->toCborMap(b.lastState);
				if (!map.isEmpty())
					players.append(map);
			}
		}

		if (const auto &p = iface->serialize<RpgGameData::Enemy>()) {
			if (iface->keyFrameRequired() || forceKeyFrame) {
				QCborMap map = p->toCborMap();
				b.lastState = map;
				enemiesKF.append(map);
				iface->setKeyFrameRequired(false);
			} else {
				QCborMap map = p->toCborMap(b.lastState);
				if (!map.isEmpty())
					enemies.append(map);
			}
		}
	}

	QCborMap base;
	base.insert(QStringLiteral("t"), m_rpgGame->tickTimer()->currentTick());

	if (!enemies.isEmpty() || !players.isEmpty()) {
		QCborMap map = base;
		if (!enemies.isEmpty())
			map.insert(QStringLiteral("ee"), enemies);
		if (!players.isEmpty())
			map.insert(QStringLiteral("pp"), players);
		const QByteArray &data = map.toCborValue().toCbor();
		sendData(data, false);
	}

	if (!enemiesKF.isEmpty() || !playersKF.isEmpty()) {
		QCborMap map = base;
		if (!enemiesKF.isEmpty())
			map.insert(QStringLiteral("ee"), enemiesKF);
		if (!playersKF.isEmpty())
			map.insert(QStringLiteral("pp"), playersKF);
		const QByteArray &data = map.toCborValue().toCbor();
		sendData(data, true);
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

	//QCoreApplication::processEvents();

	/*for (PlayerData &d : d->m_playerData) {
		if (d.player)
			continue;

		LOG_CDEBUG("game") << "Create player item" << d.s << d.id;

		RpgPlayer *player = q->createPlayer(q->m_rpgGame->currentScene(), RpgGame::characters().find(d.config.character).value());

		if (!player) {
			LOG_CERROR("game") << "ERROR";
			return;
		}

		const int hp = q->m_missionLevel->startHP();

		player->setHp(hp);
		player->setMaxHp(hp);
		q->m_rpgGame->setFollowedItem(player);
		QList<RpgPlayer*> list = q->m_rpgGame->players();
		list.append(player);
		q->m_rpgGame->setPlayers(list);

		player->setObjectId(d.s, d.id);

		d.player = player;

		if (d.id == q->m_playerId)
			q->m_rpgGame->setControlledPlayer(player);
	}*/
}


/**
 * @brief ActionRpgMultiplayerGame::sendData
 * @param data
 * @param reliable
 */

void ActionRpgMultiplayerGame::sendData(const QByteArray &data, const bool &reliable)
{
	QMetaObject::invokeMethod(d, &RpgUdpEnginePrivate::sendMessage, Qt::QueuedConnection, data, reliable);
}


/**
 * @brief ActionRpgMultiplayerGame::sendDataChrSel
 */

void ActionRpgMultiplayerGame::sendDataChrSel()
{
	if (m_config.gameState == RpgConfig::StateCharacterSelect ) {
		QCborMap map;

		if (m_gameMode == MultiPlayerHost && m_rpgGame) {
			RpgGameData::GameConfig gameConfig;
			for (const TiledGame::PlayerPosition &p : m_rpgGame->playerPositionList()) {
				gameConfig.positionList.emplaceBack(p.sceneId, p.position.x(), p.position.y());
			}

			gameConfig.terrain = m_playerConfig.terrain;

			QCborMap cfgMap = gameConfig.toCborMap(true);
			map.insert(QStringLiteral("cfg"), cfgMap);
		}

		RpgGameData::CharacterSelect config(m_playerConfig);
		config.completed = m_selectionCompleted;

		LOG_CDEBUG("game") << "Send CONFIG" << config.character << config.terrain << config.completed;

		QCborMap cfgMap = config.toCborMap(true);
		map.insert(QStringLiteral("chr"), cfgMap);

		sendData(map.toCborValue().toCbor(), true);
	}
}


/**
 * @brief RpgUdpEnginePrivate::~RpgUdpEnginePrivate
 */

RpgUdpEnginePrivate::RpgUdpEnginePrivate(ActionRpgMultiplayerGame *game)
	: QObject()
	, q(game)
{

}



RpgUdpEnginePrivate::~RpgUdpEnginePrivate()
{
	disconnect();
}



/**
 * @brief RpgUdpEnginePrivate::worldStep
 * @param body
 */

void RpgUdpEnginePrivate::worldStep(TiledObjectBody *body)
{
	if (q->m_gameMode == ActionRpgGame::MultiPlayerHost)
		body->worldStep();

}




/**
 * @brief RpgUdpEnginePrivate::timeStepped
 */

void RpgUdpEnginePrivate::timeStepped()
{
	QMutexLocker locker(&m_mutex);


}



/**
 * @brief RpgUdpEnginePrivate::run
 */





/**
 * @brief RpgUdpEnginePrivate::connectToHost
 * @param host
 * @param port
 * @return
 */

bool RpgUdpEnginePrivate::connectToHost(const char *host, const int &port)
{
	if (m_enet_host)
		disconnect();

	m_canRun = false;

	ENetHost *client = enet_host_create(NULL, 1, 2, 0, 0);

	if (!client) {
		LOG_CERROR("game") << "Connection refused" << host << port;
		return false;
	}

	ENetAddress address;
	ENetEvent event;
	ENetPeer *peer;

	enet_address_set_host(&address, q->m_client->server()->host().toLatin1());
	address.port = q->m_client->server()->port();

	peer = enet_host_connect (client, &address, 2, 0);

	if (!peer) {
		LOG_CWARNING("game") << "Connection refused" << host << port;
		enet_host_destroy(client);
		return false;
	}

	if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		LOG_CDEBUG("game") << "Connected to host" << host << port;
	} else {
		LOG_CWARNING("game") << "Connnection failed" << host << port;
		enet_peer_reset(peer);
		enet_host_destroy(client);
		return false;
	}

	m_enet_host = client;
	m_enet_peer = peer;

	m_canRun = true;

	return true;
}



/**
 * @brief RpgUdpEnginePrivate::disconnect
 */

void RpgUdpEnginePrivate::disconnect()
{
	if (m_enet_host) {
		if (m_enet_peer) {
			enet_peer_disconnect(m_enet_peer, 0);
			ENetEvent event;
			enet_host_service(m_enet_host, &event, 200);
			enet_peer_reset(m_enet_peer);
			m_enet_peer = nullptr;
		}
		enet_host_destroy(m_enet_host);
	}

	m_enet_host = nullptr;
	m_canRun = false;
}



/**
 * @brief RpgUdpEnginePrivate::packetReceived
 * @param packet
 */

void RpgUdpEnginePrivate::packetReceived(ENetPacket *packet)
{
	Q_ASSERT(packet);
	QByteArray data((char*)packet->data, packet->dataLength);

	QCborMap map = QCborValue::fromCbor(data).toMap();

	QCborArray pList = map.value("pp").toArray();

	/*LOG_CDEBUG("game") << "RCV" << map.value("t").toInteger(-1) << map.value("hst").toBool() << pList.size() <<
						  "IDX" << map.value("ply").toInteger(-1);*/

	m_lastTick = map.value("t").toInteger(-1);

	///int myId = map.value("cfg").toMap().value("playerId").toInteger(-1);

	q->setGameMode(map.value("hst").toBool() ? ActionRpgGame::MultiPlayerHost : ActionRpgGame::MultiPlayerGuest);

	const int myId = map.value("ply").toInteger(-1);

	q->setPlayerId(myId);

	if (myId == -1)
		return;


	if (q->m_config.gameState == RpgConfig::StateConnect) {
		q->m_config.gameState = RpgConfig::StateCharacterSelect;
		q->updateConfig();
	}

	if (q->m_config.gameState == RpgConfig::StateCharacterSelect)
		packetReceivedChrSel(map);

	if (!q->m_rpgGame)
		return;

/*	QMutexLocker locker(&m_mutex);

	for (const QCborValue &v : pList) {
		RpgGameData::Player playerData;
		playerData.fromCbor(v);

		if (playerData.s < 0 || playerData.id < 0) {
			LOG_CERROR("game") << "Invalid player";
			continue;
		}

		const auto it = std::find_if(m_playerData.cbegin(), m_playerData.cend(),
									 [&playerData](const PlayerData &p){
			return p.s == playerData.s && p.id == playerData.id;
		});

		if (it == m_playerData.cend()) {
			LOG_CINFO("game") << "CREATE PLAYER" << playerData.s << playerData.id;

			PlayerData &pd = m_playerData.emplace_back(playerData);
			//pd.config = q->m_playerConfig;

			// TODO: config?
		} else {
			if (!it->player) {
				LOG_CERROR("game") << "Missing player";
				continue;
			}

			if (q->m_gameMode != ActionRpgGame::MultiPlayerGuest)
				continue;

			if (playerData.p.size() < 2) {
				LOG_CERROR("game") << "Missing player POS";
				continue;
			}

			QVector2D serverPos(playerData.p.at(0), playerData.p.at(1));


			if (it->player->distanceToPoint(serverPos) > 5) {
				it->player->moveTowards(serverPos, 108.);
			} else {
				it->player->stop();
			}
		}
	}*/
}


/**
 * @brief RpgUdpEnginePrivate::packetReceivedChrSel
 * @param data
 */

void RpgUdpEnginePrivate::packetReceivedChrSel(const QCborMap &data)
{
	QCborArray pList = data.value(QStringLiteral("pp")).toArray();

	QVariantList list;

	for (const QCborValue &v : pList) {
		RpgGameData::CharacterSelect ch;
		ch.fromCbor(v.toMap());
		list.append(ch.toJson().toVariantMap());
	}

	Utils::patchSListModel(q->m_playersModel.get(), list, QStringLiteral("username"));


	QCborMap cfg = data.value(QStringLiteral("g")).toMap();

	if (!cfg.isEmpty()) {
		RpgGameData::GameConfig config;
		config.fromCbor(cfg);

		if (!config.terrain.isEmpty()) {
			if (q->m_client->server() && q->m_client->server()->user() &&
					q->m_client->server()->user()->wallet()->world())
				q->m_client->server()->user()->wallet()->world()->select(config.terrain, true);
		}
	}
}




/**
 * @brief RpgUdpEnginePrivate::run
 */

void RpgUdpEnginePrivate::run()
{
	if (!m_enet_host)
		return;

	LOG_CINFO("game") << "RUN";

	ENetEvent event;

	while (int r = enet_host_service (m_enet_host, &event, 100) >= 0) {
		if (!m_canRun)
			break;

		if (r > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_CONNECT:
					LOG_CINFO("engine") << "CONNECT" << event.peer->address.host << event.peer->address.port;
					//event.peer->data = "mydata";
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
					LOG_CINFO("engine") << "DISCONNECT" << event.peer->address.host << event.peer->address.port; //<< event.peer->data;
					m_canRun = false;
					break;

				case ENET_EVENT_TYPE_RECEIVE:
					packetReceived(event.packet);
					enet_packet_destroy(event.packet);
					break;

				case ENET_EVENT_TYPE_NONE:
					break;
			}
		}

		if (m_canRun) {
			QMutexLocker locker(&m_mutex);

			for (const auto &b : m_sendList) {
				ENetPacket *packet = enet_packet_create(b.data.data(), b.data.size(),
														b.reliable ? ENET_PACKET_FLAG_RELIABLE :
																	 ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
				enet_peer_send(m_enet_peer, 0, packet);
			}

			m_sendList.clear();
		}

		QCoreApplication::processEvents();
	}

	LOG_CINFO("engine") << "UPD ENGINE RUN FINISHED";

	disconnect();
}



/**
 * @brief RpgUdpEnginePrivate::sendMessage
 * @param data
 */

void RpgUdpEnginePrivate::sendMessage(QByteArray data, const bool &reliable)
{
	QMutexLocker locker(&m_mutex);
	m_sendList.append({.data = data, .reliable = reliable});
}


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
