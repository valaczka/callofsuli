/*
 * ---- Call of Suli ----
 *
 * actionrpggame.cpp
 *
 * Created on: 2024. 03. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ActionRpgGame
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

#include "actionrpggame.h"
#include "Logger.h"
#include "client.h"
#include "rpgquestion.h"
#include "server.h"
#include "rpgshield.h"


/**
 * @brief ActionRpgGame::ActionRpgGame
 * @param missionLevel
 * @param client
 */

ActionRpgGame::ActionRpgGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(GameMap::Rpg, missionLevel, client)
	, m_rpgQuestion(new RpgQuestion(this))
{
	LOG_CTRACE("game") << "Action RPG game constructed" << this;

	if (m_missionLevel && m_missionLevel->map()) {
		m_config.mapUuid = m_missionLevel->map()->uuid();
		m_config.missionUuid = m_missionLevel->mission()->uuid();
		m_config.missionLevel = m_missionLevel->level();
	}

	connect(this, &AbstractLevelGame::gameTimeout, this, &ActionRpgGame::onGameTimeout);
	connect(this, &AbstractLevelGame::msecLeftChanged, this, &ActionRpgGame::onMsecLeftChanged);
	/*connect(this, &ActionGame::toolChanged, this, &ActionGame::toolListIconsChanged);*/
}



/**
 * @brief ActionRpgGame::~ActionRpgGame
 */

ActionRpgGame::~ActionRpgGame()
{

}


/**
 * @brief ActionRpgGame::gameAbort
 */

void ActionRpgGame::gameAbort()
{
	if (m_config.gameState == RpgConfig::StateFinished)
		return;
	else if (m_config.gameState == RpgConfig::StateConnect ||
			 m_config.gameState == RpgConfig::StateDownloadContent ||
			 m_config.gameState == RpgConfig::StateError ||
			 m_config.gameState == RpgConfig::StateCharacterSelect) {
		setFinishState(Neutral);

		LOG_CINFO("game") << "Game cancelled:" << this;

		gameFinish();
		return;
	}

	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	gameFinish();

	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
}




/**
 * @brief ActionRpgGame::playMenuBgMusic
 */

void ActionRpgGame::playMenuBgMusic()
{
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/menu/bg.mp3"), Sound::MusicChannel);
}



/**
 * @brief ActionRpgGame::stopMenuBgMusic
 */

void ActionRpgGame::stopMenuBgMusic()
{
	m_client->sound()->stopMusic();
}



/**
 * @brief ActionRpgGame::selectCharacter
 * @param character
 */

void ActionRpgGame::selectCharacter(const QString &character)
{
	LOG_CWARNING("game") << "Character selected:" << character << m_config.gameState;

	m_playerConfig.character = character;

	m_config.gameState = RpgConfig::StatePrepare;
	updateConfig();
}



/**
 * @brief ActionRpgGame::rpgGameActivated
 */

void ActionRpgGame::rpgGameActivated()
{
	if (!m_rpgGame)
		return;

	const auto &ptr = RpgGame::readGameDefinition(m_missionLevel->terrain());

	if (!ptr)
		return;

	m_config.duration = ptr->duration > 0 ? ptr->duration : m_missionLevel->duration();
	updateConfig();

	if (!m_rpgGame->load(ptr.value())) {
		LOG_CERROR("game") << "Game load error";
		return;
	}

	TiledScene *firstScene = m_rpgGame->findScene(ptr->firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Game load error";
		return;
	}


	const auto &ptrPos = m_rpgGame->playerPosition(firstScene, 0);

	QList<RpgPlayer*> list;

	RpgPlayer *player = RpgPlayer::createPlayer(m_rpgGame, firstScene, m_playerConfig.character);

	player->setHp(m_missionLevel->startHP());
	player->setMaxHp(m_missionLevel->startHP());
	loadInventory(player);

	player->emplace(ptrPos.value_or(QPointF{0,0}));
	player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::West));

	firstScene->appendToObjects(player);
	m_rpgGame->setFollowedItem(player);
	m_rpgGame->setControlledPlayer(player);

	player->setCurrentSceneStartPosition(ptrPos.value_or(QPointF{0,0}));

	list.append(player);

	m_rpgGame->setPlayers(list);

	for (TiledScene *s : m_rpgGame->sceneList()) {
		m_rpgGame->setQuestions(s, m_missionLevel->questions());
	}

	emit m_rpgGame->gameLoaded();
}



/**
 * @brief ActionRpgGame::finishGame
 */

void ActionRpgGame::finishGame()
{
	LOG_CDEBUG("game") << "Finish game";

	m_config.gameState = RpgConfig::StateFinished;
	updateConfig();
}




/**
 * @brief ActionRpgGame::loadPage
 * @return
 */

QQuickItem *ActionRpgGame::loadPage()
{

	QQuickItem *item = m_client->stackPushPage(QStringLiteral("PageActionRpgGame.qml"), QVariantMap({
																										{ QStringLiteral("game"), QVariant::fromValue(this) }
																									}));

	if (!item)
		return nullptr;

	m_config.gameState = RpgConfig::StateDownloadContent;
	updateConfig();

	downloadGameData();

	return item;
}



/**
 * @brief ActionRpgGame::timerEvent
 */

void ActionRpgGame::timerEvent(QTimerEvent *)
{

}



/**
 * @brief ActionRpgGame::connectGameQuestion
 */

void ActionRpgGame::connectGameQuestion()
{

}


/**
 * @brief ActionRpgGame::gameStartEvent
 * @return
 */

bool ActionRpgGame::gameStartEvent()
{
	return true;
}

bool ActionRpgGame::gameFinishEvent()
{
	m_timerLeft.stop();
	return true;
}


/**
 * @brief ActionRpgGame::gamePrepared
 */

void ActionRpgGame::gamePrepared()
{
	m_config.gameState = RpgConfig::StatePlay;
	updateConfig();
}



/**
 * @brief ActionRpgGame::onPlayerDead
 * @param player
 */

void ActionRpgGame::onPlayerDead(RpgPlayer *player)
{
	LOG_CDEBUG("game") << "Player dead" << player;

	/// TODO: check m_gameMode

	if (deathmatch())
		QTimer::singleShot(5000, this, &ActionRpgGame::onGameFailed);
	else if (m_rpgGame)
		QTimer::singleShot(5000, this, [this, p = QPointer<RpgPlayer>(player)]() {
			if (finishState() == Fail || finishState() == Success)
				return;

			if (p)
				m_rpgGame->setQuestions(p->scene(), m_missionLevel->questions());

			m_rpgGame->resurrectEnemiesAndPlayer(p);
			loadInventory(p);
		});
}





/**
 * @brief ActionRpgGame::onGameTimeout
 */

void ActionRpgGame::onGameTimeout()
{
	Sound *sound = m_client->sound();

	sound->stopMusic();
	sound->stopMusic2();

	setFinishState(Fail);
	gameFinish();

	sound->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	sound->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);

	emit finishDialogRequest(tr("Time out"),
							 QStringLiteral("qrc:/Qaterial/Icons/timer-sand.svg"),
							 false);
}



/**
 * @brief ActionRpgGame::onGameSuccess
 */

void ActionRpgGame::onGameSuccess()
{
	Sound *sound = m_client->sound();

	setFinishState(Success);
	gameFinish();

	sound->stopMusic();
	sound->stopMusic2();
	sound->playSound(QStringLiteral("qrc:/sound/sfx/win.mp3"), Sound::SfxChannel);


	QTimer::singleShot(3000, this, [this, sound](){
		sound->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
		sound->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);

		emit finishDialogRequest(m_isFlawless ? tr("Mission completed\nHibátlan győzelem!")
											  : tr("Mission completed"),
								 QStringLiteral("qrc:/Qaterial/Icons/trophy.svg"),
								 true);
	});
}



/**
 * @brief ActionRpgGame::onGameFailed
 */

void ActionRpgGame::onGameFailed()
{
	Sound *sound = m_client->sound();

	sound->stopMusic();
	sound->stopMusic2();

	setFinishState(Fail);
	gameFinish();
	sound->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	sound->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);

	emit finishDialogRequest(tr("Your man has died"),
							 QStringLiteral("qrc:/Qaterial/Icons/skull-crossbones.svg"),
							 false);
}



/**
 * @brief ActionRpgGame::updateConfig
 */

void ActionRpgGame::updateConfig()
{
	onConfigChanged();
	emit configChanged();
}




/**
 * @brief ActionRpgGame::onConfigChanged
 */

void ActionRpgGame::onConfigChanged()
{
	//LOG_CDEBUG("game") << "ConquestGame state:" << m_config.gameState << m_config.currentStage << m_config.currentTurn;

	/*if (m_config.currentTurn >= 0 && m_config.currentTurn < m_config.turnList.size())
		setCurrentTurn(m_config.turnList.at(m_config.currentTurn));
	else
		setCurrentTurn({});

	setCurrentStage(m_config.currentStage);*/

	if (m_config.gameState == RpgConfig::StateInvalid)
		return;

	if (m_config.gameState == RpgConfig::StatePrepare && m_oldGameState != RpgConfig::StatePrepare) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/prepare_yourself.mp3"), Sound::VoiceoverChannel);
		//m_client->sound()->playSound(backgroundMusic(), Sound::MusicChannel);
		stopMenuBgMusic();
	}

	if (m_config.gameState == RpgConfig::StatePlay && m_oldGameState != RpgConfig::StatePlay) {
		if (!m_rpgGame) {
			m_config.gameState = RpgConfig::StateError;
			updateConfig();
			return;
		}

		startWithRemainingTime(m_config.duration*1000);

		if (m_deathmatch) {
			m_rpgGame->message(tr("LEVEL %1").arg(level()));
			m_rpgGame->message(tr("SUDDEN DEATH"));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"), Sound::VoiceoverChannel);
		} else {
			m_rpgGame->message(tr("LEVEL %1").arg(level()));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);
		}
	}

	/*if (m_config.gameState == ConquestConfig::StateFinished && m_oldGameState != ConquestConfig::StateFinished) {
		m_client->sound()->stopMusic();
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/win.mp3"), Sound::VoiceoverChannel);
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
		if (m_gameSuccess)
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);
		else
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
	}


	if ((m_config.gameState == ConquestConfig::StatePrepare || m_config.gameState == ConquestConfig::StatePlay) &&
			m_config.world.name != m_loadedWorld) {
		reloadLandList();
		message(tr("Waiting for other players..."));
	}

	for (ConquestLandData *land : *m_landDataList) {
		const int &idx = m_config.world.landFind(land->landId());
		if (idx != -1)
			land->loadFromConfig(m_config.world.landList[idx]);
	}
	*/

	/*if (m_config.gameState == ConquestConfig::StatePrepare && m_oldGameState != ConquestConfig::StatePrepare) {
		sendWebSocketMessage(QJsonObject{
								 { QStringLiteral("cmd"), QStringLiteral("prepare") },
								 { QStringLiteral("engine"), m_engineId },
								 { QStringLiteral("ready"), true }
							 });
	}*/

	m_oldGameState = m_config.gameState;
}


/**
 * @brief ActionRpgGame::downloadGameData
 */

void ActionRpgGame::downloadGameData()
{
	Server *server = m_client->server();


	if (!server) {
		LOG_CERROR("game") << "Missing server";
		setError();
		return;
	}

	const auto &ptr = RpgGame::readGameDefinition(m_missionLevel->terrain());

	if (!ptr) {
		LOG_CERROR("game") << "Invalid game";
		setError();
		return;
	}

	const auto &listPtr = TiledGame::getDynamicTilesets(ptr.value());

	if (!listPtr)
		return;


	m_loadableContentCount = listPtr->size();
	m_downloadProgress = 0.;
	emit downloadProgressChanged();


	connect(server, &Server::loadableContentError, this, [this]() {
		LOG_CERROR("game") << "Loadable content error";
		setError();
	});

	connect(server, &Server::loadableContentReady, this, [this]() {
		m_config.gameState = RpgConfig::StateCharacterSelect;
		updateConfig();
	});

	connect(server, &Server::loadableContentOneDownloaded, this, [this](const QString &){
		Server *s = qobject_cast<Server*>(sender());
		if (!m_loadableContentCount || !s)
			return;

		qreal c = s->loadableContentSize();
		m_downloadProgress = c / (qreal) m_loadableContentCount;
		emit downloadProgressChanged();
	});

	server->downloadLoadableContentDict(m_client, *listPtr);
}


/**
 * @brief ActionRpgGame::setError
 */

void ActionRpgGame::setError()
{
	m_config.gameState = RpgConfig::StateError;
	updateConfig();
}



/**
 * @brief ActionRpgGame::onMsecLeftChanged
 */

void ActionRpgGame::onMsecLeftChanged()
{
	const int &msec = msecLeft();

	struct TimeNotify {
		int msec;
		QString message;
		QString sound;
	};

	static const QVector<TimeNotify> notifications = {
		TimeNotify{
			30000,
			tr("You have 30 seconds left"),
			QStringLiteral("qrc:/sound/voiceover/final_round.mp3")
		},
		TimeNotify{
			60000,
			tr("You have 60 seconds left"),
			QStringLiteral("qrc:/sound/voiceover/time.mp3")
		}
	};

	if (m_msecNotifyAt == 0) {
		for (const auto &n : notifications) {
			if (n.msec > m_msecNotifyAt && n.msec < msec)
				m_msecNotifyAt = n.msec;
		}
	}



	if (m_msecNotifyAt > msec) {
		const auto it = std::find_if(notifications.constBegin(), notifications.constEnd(),
									 [this](const TimeNotify &n) {
			return n.msec == m_msecNotifyAt;
		});

		if (it != notifications.constEnd()) {
			m_rpgGame->messageColor(it->message, QStringLiteral("#00bcd4"));
			m_client->sound()->playSound(it->sound, Sound::VoiceoverChannel);
		}

		m_msecNotifyAt = 0;
	}
}




/**
 * @brief ActionRpgGame::loadInventory
 * @param player
 */

void ActionRpgGame::loadInventory(RpgPlayer *player)
{
	if (!player)
		return;

	LOG_CTRACE("game") << "Load player inventory" << player;

	for (const QString &s : player->m_config.inventoryOnce) {
		loadInventory(player, RpgPickableObject::typeFromString(s));
	}

	player->m_config.inventoryOnce.clear();

	for (const QString &s : player->m_config.inventory) {
		loadInventory(player, RpgPickableObject::typeFromString(s));
	}
}



/**
 * @brief ActionRpgGame::loadInventory
 * @param player
 * @param pickableType
 */

void ActionRpgGame::loadInventory(RpgPlayer *player, const RpgPickableObject::PickableType &pickableType)
{
	if (!player)
		return;

	switch (pickableType) {
		case RpgPickableObject::PickableShield:
			RpgShieldPickable::pick(player, m_rpgGame);
			break;

		case RpgPickableObject::PickableHp:
		case RpgPickableObject::PickableShortbow:
		case RpgPickableObject::PickableLongbow:
		case RpgPickableObject::PickableArrow:
		case RpgPickableObject::PickableFireball:
		case RpgPickableObject::PickableLongsword:
		case RpgPickableObject::PickableTime:
			LOG_CERROR("game") << "Inventory type not supported:" << pickableType;
			break;

		case RpgPickableObject::PickableInvalid:
			LOG_CERROR("game") << "Invalid inventory type";
			break;
	}

}




/**
 * @brief ActionRpgGame::onPlayerPick
 * @param player
 * @param pickable
 * @return
 */

bool ActionRpgGame::onPlayerPick(RpgPlayer *player, RpgPickableObject *pickable)
{
	if (!player || !pickable)
		return false;


	if (pickable->pickableType() == RpgPickableObject::PickableTime) {
		static int sec = 60;
		addToDeadline(sec*1000);
		m_msecNotifyAt = 0;
		m_rpgGame->messageColor(tr("%1 seconds gained").arg(sec), QStringLiteral("#00bcd4"));
	}

	/*if (pickable->pickableType() == RpgPickableObject::PickableLongsword) {
		m_rpgGame->messageColor("Nem lehet", "#DD0000");
		return false;
	}*/

	return true;
}


/**
 * @brief ActionRpgGame::onPlayerAttackEnemy
 * @param player
 * @param enemy
 * @param weaponType
 * @return
 */

bool ActionRpgGame::onPlayerAttackEnemy(RpgPlayer *player, IsometricEnemy *enemy, const TiledWeapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;

	if (auto ptr = m_rpgGame->enemyFind(enemy); ptr != m_rpgGame->m_enemyDataList.end() && ptr->hasQuestion) {
		const int &hp = enemy->getNewHpAfterAttack(enemy->hp(), weaponType, player);
		if (hp <= 0 && m_rpgQuestion->nextQuestion(player, enemy, weaponType)) {
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"), Sound::SfxChannel);
			return true;
		}
	}

	int hp = enemy->hp();

	enemy->attackedByPlayer(player, weaponType);

	int xp = std::max(0, hp-enemy->hp());

	setXp(m_xp+xp);

	return true;
}



/**
 * @brief ActionRpgGame::onQuestionSuccess
 * @param player
 * @param enemy
 * @param xp
 */

void ActionRpgGame::onQuestionSuccess(RpgPlayer */*player*/, IsometricEnemy *enemy, int xp)
{
	if (enemy)
		enemy->setHp(0);

	setXp(m_xp+xp);
}


/**
 * @brief ActionRpgGame::onQuestionFailed
 * @param player
 * @param enemy
 */

void ActionRpgGame::onQuestionFailed(RpgPlayer *player, IsometricEnemy */*enemy*/)
{
	if (player)
		player->setHp(std::max(0, player->hp()-1));

	setIsFlawless(false);
}



/**
 * @brief ActionRpgGame::characterList
 * @return
 */

QVariantList ActionRpgGame::characterList() const
{
	QVariantList list;

	for (const RpgPlayer::CharacterData &ch : RpgPlayer::availableCharacters()) {
		QVariantMap m;
		m[QStringLiteral("id")] = ch.id;
		m[QStringLiteral("displayName")] = ch.config.name;
		m[QStringLiteral("image")] = ch.config.image;
		list.append(m);
	}

	return list;
}


/**
 * @brief ActionRpgGame::downloadProgress
 * @return
 */

qreal ActionRpgGame::downloadProgress() const
{
	return m_downloadProgress;
}




ActionRpgGame::GameMode ActionRpgGame::gameMode() const
{
	return m_gameMode;
}

void ActionRpgGame::setGameMode(const GameMode &newGameMode)
{
	if (m_gameMode == newGameMode)
		return;
	m_gameMode = newGameMode;
	emit gameModeChanged();
}



/**
 * @brief ActionRpgGame::playerConfig
 * @return
 */

RpgPlayerConfig ActionRpgGame::playerConfig() const
{
	return m_playerConfig;
}

void ActionRpgGame::setPlayerConfig(const RpgPlayerConfig &newPlayerConfig)
{
	if (m_playerConfig == newPlayerConfig)
		return;
	m_playerConfig = newPlayerConfig;
	emit playerConfigChanged();
}




/**
 * @brief ActionRpgGame::rpgGame
 * @return
 */

RpgGame *ActionRpgGame::rpgGame() const
{
	return m_rpgGame;
}



/**
 * @brief ActionRpgGame::setRpgGame
 * @param newRpgGame
 */

void ActionRpgGame::setRpgGame(RpgGame *newRpgGame)
{
	if (m_rpgGame == newRpgGame)
		return;

	if (m_rpgGame) {
		setGameQuestion(nullptr);
		disconnect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgGame::onGameSuccess);
		disconnect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgGame::onPlayerDead);
		m_rpgGame->setRpgQuestion(nullptr);
		m_rpgGame->setFuncPlayerPick(nullptr);
		m_rpgGame->setFuncPlayerAttackEnemy(nullptr);
	}

	m_rpgGame = newRpgGame;
	emit rpgGameChanged();

	if (m_rpgGame) {
		setGameQuestion(m_rpgGame->gameQuestion());
		m_rpgGame->setRpgQuestion(m_rpgQuestion.get());
		connect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgGame::onGameSuccess);
		connect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgGame::onPlayerDead);
		m_rpgGame->setFuncPlayerPick(std::bind(&ActionRpgGame::onPlayerPick, this, std::placeholders::_1, std::placeholders::_2));
		m_rpgGame->setFuncPlayerAttackEnemy(std::bind(&ActionRpgGame::onPlayerAttackEnemy, this,
													  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
}



/**
 * @brief ActionRpgGame::config
 * @return
 */

RpgConfig ActionRpgGame::config() const
{
	return m_config;
}

void ActionRpgGame::setConfig(const RpgConfig &newConfig)
{
	if (m_config == newConfig)
		return;
	m_config = newConfig;
	updateConfig();
}
