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
#include "mapplaycampaign.h"
#include "rpgbroadsword.h"
#include "rpgdagger.h"
#include "rpglongsword.h"
#include "rpgquestion.h"
#include "rpguserwallet.h"
#include "server.h"
#include "rpgshield.h"
#include "tilelayeritem.h"
#include <libtiled/imagecache.h>
#include "rpglongbow.h"
#include "utils_.h"


/**
 * @brief ActionRpgGame::ActionRpgGame
 * @param missionLevel
 * @param client
 */

ActionRpgGame::ActionRpgGame(GameMapMissionLevel *missionLevel, Client *client)
	: AbstractLevelGame(GameMap::Rpg, missionLevel, client)
	, m_rpgQuestion(new RpgQuestion(this))
	, m_downloader(new Downloader)
{
	LOG_CTRACE("game") << "Action RPG game constructed" << this;

	if (m_missionLevel && m_missionLevel->map()) {
		m_config.mapUuid = m_missionLevel->map()->uuid();
		m_config.missionUuid = m_missionLevel->mission()->uuid();
		m_config.missionLevel = m_missionLevel->level();
	}

	connect(this, &AbstractLevelGame::gameTimeout, this, &ActionRpgGame::onGameTimeout);
	connect(this, &AbstractLevelGame::msecLeftChanged, this, &ActionRpgGame::onMsecLeftChanged);

	connect(client->downloader(), &Downloader::contentDownloaded, this, [this]() {
		m_config.gameState = RpgConfig::StateCharacterSelect;
		updateConfig();
	});

	connect(client->downloader(), &Downloader::downloadError, this, &ActionRpgGame::setError);


	connect(m_downloader.get(), &Downloader::contentDownloaded, this, [this]() {
		m_config.gameState = RpgConfig::StatePrepare;
		QMetaObject::invokeMethod(this, &ActionRpgGame::updateConfig, Qt::QueuedConnection);
	});
}



/**
 * @brief ActionRpgGame::~ActionRpgGame
 */

ActionRpgGame::~ActionRpgGame()
{
	clearSharedTextures();
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
 * @param world
 * @param character
 * @param weaponList
 */

void ActionRpgGame::selectCharacter(const QString &terrain, const QString &character, const QStringList &weaponList)
{
	LOG_CDEBUG("game") << "World/Skin/Weapons selected:" << terrain << character << weaponList;

	m_playerConfig.terrain = terrain;
	m_playerConfig.character = character;
	m_playerConfig.weapons = weaponList;

	m_config.gameState = RpgConfig::StateDownloadContent;
	updateConfig();

	downloadGameData();

	//////m_config.gameState = RpgConfig::StatePrepare;
	///////QMetaObject::invokeMethod(this, &ActionRpgGame::updateConfig, Qt::QueuedConnection);
}



/**
 * @brief ActionRpgGame::rpgGameActivated
 */

void ActionRpgGame::rpgGameActivated()
{
	if (!m_rpgGame)
		return;

	QMetaObject::invokeMethod(this, &ActionRpgGame::rpgGameActivated_, Qt::QueuedConnection);
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

	m_config.gameState = RpgConfig::StateDownloadStatic;
	updateConfig();

	m_client->downloader()->download();

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
	QMetaObject::invokeMethod(this, &ActionRpgGame::updateConfig, Qt::QueuedConnection);
}


/**
 * @brief ActionRpgGame::clearSharedTextures
 */

void ActionRpgGame::clearSharedTextures()
{
	LOG_CTRACE("game") << "Clear shared textures";

	TiledQuick::TileLayerItem::clearSharedTextures();
	TiledGame::clearSharedTextures();
	Tiled::ImageCache::clear();
}


/**
 * @brief ActionRpgGame::addWallet
 * @param wallet
 */

void ActionRpgGame::addWallet(RpgUserWallet *wallet)
{
	if (!wallet || !m_rpgGame)
		return;

	RpgPlayer *player = m_rpgGame->controlledPlayer();

	if (!player)
		return;

	LOG_CDEBUG("game") << "Add wallet" << wallet->market().type << wallet->market().name;

	switch (wallet->market().type) {
		case RpgMarket::Weapon: {
			const TiledWeapon::WeaponType type = RpgArmory::weaponHash().key(wallet->market().name, TiledWeapon::WeaponInvalid);
			if (type == TiledWeapon::WeaponInvalid) {
				LOG_CERROR("game") << "Invalid weapon" << wallet->market().name;
				return;
			}

			TiledWeapon *weapon = player->armory()->weaponFind(type);

			if (weapon) {
				LOG_CDEBUG("game") << "Weapon already exists";
				return;
			}

			loadWeapon(player, type, wallet->bullet() ? wallet->bullet()->amount() : 0);

			return;
		}

		case RpgMarket::Bullet: {
			const RpgPickableObject::PickableType ptype = RpgPickableObject::typeFromString(wallet->market().name);
			loadBullet(player, ptype, wallet->market().amount);
			return;
		}

		case RpgMarket::Hp:
			player->setHp(player->hp() + wallet->market().amount);
			return;

		case RpgMarket::Time:
			addToDeadline(wallet->market().amount * 1000);
			return;


		case RpgMarket::Map:
		case RpgMarket::Skin:
		case RpgMarket::Xp:
		case RpgMarket::Pickable:
		case RpgMarket::Other:
		case RpgMarket::Invalid:
			LOG_CTRACE("game") << "Wallet skipped";
			break;
	}
}



/**
 * @brief ActionRpgGame::onPlayerDead
 * @param player
 */

void ActionRpgGame::onPlayerDead(RpgPlayer *player)
{
	LOG_CDEBUG("game") << "Player dead" << player;

	/*if (deathmatch())
		QTimer::singleShot(5000, this, &ActionRpgGame::onGameFailed);
	else*/

	if (m_rpgGame)
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

	m_rpgGame->checkFinalQuests();

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
	if (m_rpgGame) {
		m_rpgGame->saveSceneState();
	}

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
 * @brief ActionRpgGame::onGameLoadFailed
 */

void ActionRpgGame::onGameLoadFailed(const QString &)
{
	m_client->sound()->setVolumeSfx(m_tmpSoundSfxVolume);
}


/**
 * @brief ActionRpgGame::rpgGameActivated_
 */

void ActionRpgGame::rpgGameActivated_()
{
	if (!m_rpgGame)
		return;

	const auto &ptr = RpgGame::terrains().find(m_playerConfig.terrain);

	if (ptr == RpgGame::terrains().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid terrain" << m_playerConfig.terrain;
		return;
	}

	m_config.duration = ptr->duration + m_missionLevel->duration();
	updateConfig();

	if (!m_rpgGame->load(ptr.value())) {
		LOG_CERROR("game") << "Game load error";
		return;
	}

	TiledScene *firstScene = m_rpgGame->findScene(ptr->firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Game load error: missing first scene";
		return;
	}


	const auto characterPtr = RpgGame::characters().find(m_playerConfig.character);

	if (characterPtr == RpgGame::characters().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid character" << m_playerConfig.character;
		return;
	}

	const auto &ptrPos = m_rpgGame->playerPosition(firstScene, 0);

	QList<RpgPlayer*> list;

	RpgPlayer *player = RpgPlayer::createPlayer(m_rpgGame, firstScene, *characterPtr);

	player->setHp(m_missionLevel->startHP());
	player->setMaxHp(m_missionLevel->startHP());
	loadInventory(player);


	// Set user name

	if (Server *s = m_client->server()) {
		player->setDisplayName(s->user()->fullNickName());
	}


	// From wallet

	RpgUserWalletList* wallet = m_client->server() ? m_client->server()->user()->wallet() : nullptr;

	if (wallet) {
		for (const QString &s : m_playerConfig.weapons) {
			const auto it = std::find_if(wallet->constBegin(), wallet->constEnd(), [&s](RpgUserWallet *w) {
							return w->market().type == RpgMarket::Weapon && w->market().name == s;
		});
			if (it == wallet->constEnd()) {
				LOG_CERROR("game") << "Missing weapon" << s;
				continue;
			}

			loadWeapon(player,
					   RpgArmory::weaponHash().key(s, TiledWeapon::WeaponInvalid),
					   (*it)->bullet() ? (*it)->bullet()->amount() : 0);
		}
	}


	player->emplace(ptrPos.value_or(QPointF{0,0}));
	player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::West));

	firstScene->appendToObjects(player);
	m_rpgGame->setFollowedItem(player);
	m_rpgGame->setControlledPlayer(player);

	player->setCurrentSceneStartPosition(ptrPos.value_or(QPointF{0,0}));

	list.append(player);

	m_rpgGame->setPlayers(list);

	m_rpgQuestion->initialize();

	qreal factor = 0.35;

	if (m_missionLevel->level() > 2)
		factor = 0.45;
	else if (m_missionLevel->level() == 2)
		factor = 0.4;

	int sum = 0;

	for (TiledScene *s : m_rpgGame->sceneList()) {
		sum += m_rpgGame->setQuestions(s, /*m_missionLevel->questions()*/ factor);
	}

	m_rpgGame->loadDefaultQuests(sum);

	emit m_rpgGame->gameLoaded();
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
		m_tmpSoundSfxVolume = m_client->sound()->volumeSfx();
		m_client->sound()->setVolumeSfx(0);
	}

	if (m_config.gameState == RpgConfig::StatePlay && m_oldGameState != RpgConfig::StatePlay) {
		m_client->sound()->setVolumeSfx(m_tmpSoundSfxVolume);

		if (!m_rpgGame) {
			m_config.gameState = RpgConfig::StateError;
			updateConfig();
			return;
		}

		startWithRemainingTime(m_config.duration*1000);

		if (!m_rpgGame->m_gameDefinition.music.isEmpty())
			m_client->sound()->playSound(m_rpgGame->m_gameDefinition.music, Sound::MusicChannel);

		if (m_deathmatch) {
			m_rpgGame->message(tr("LEVEL %1").arg(level()));
			m_rpgGame->message(tr("SUDDEN DEATH"));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/sudden_death.mp3"), Sound::VoiceoverChannel);
		} else {
			m_rpgGame->message(tr("LEVEL %1").arg(level()));
			m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/begin.mp3"), Sound::VoiceoverChannel);
			///m_client->sound()->setVolumeSfx(m_tmpSoundSfxVolume);
		}
	}

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

	const auto &ptr = RpgGame::terrains().find(m_playerConfig.terrain);

	if (ptr == RpgGame::terrains().constEnd()) {
		LOG_CERROR("game") << "Invalid game";
		setError();
		return;
	}

	auto listPtr = TiledGame::getDynamicTilesets(ptr.value());

	if (!listPtr)
		return;

	m_downloader->setServer(server);


	listPtr->append(m_playerConfig.character+QStringLiteral(".full.dres"));

	if (auto it = RpgGame::characters().find(m_playerConfig.character); it != RpgGame::characters().constEnd() && !it->base.isEmpty()) {
		listPtr->append(it->base+QStringLiteral(".full.dres"));
	}

	listPtr->append(ptr->required);

	downloadLoadableContentDict(*listPtr);
}






/**
 * @brief ActionRpgGame::downloadLoadableContentDict
 * @param fileList
 */

void ActionRpgGame::downloadLoadableContentDict(const QStringList &fileList)
{
	if (m_loadableContentDict.isEmpty()) {
		m_client->send(HttpConnection::ApiGeneral, QStringLiteral("loadableDict"))
				->done(this, [this, fileList](const QJsonObject &json)
		{
			m_loadableContentDict = json;

			downloadLoadableContentDict(fileList);
		})
				->fail(this, [this](const QString &err){
			LOG_CERROR("game") << "Loadable content error" << qPrintable(err);
			setError();
		});
	} else {
		QStringList fList;

		for (QString s : fileList) {
			s.replace(QStringLiteral(":/"), QStringLiteral(""));
			if (m_loadableContentDict.contains(s))
				fList.append(m_loadableContentDict.value(s).toString());
			else {
				fList.append(s);
				//LOG_CWARNING("client") << "Invalid loadable file:" << s;
			}
		}

		fList.removeDuplicates();

		downloadLoadableContent(fList);
	}
}





/**
 * @brief ActionRpgGame::downloadLoadableContent
 * @param fileList
 */

void ActionRpgGame::downloadLoadableContent(const QStringList &fileList)
{
	LOG_CTRACE("game") << "Download loadable content:" << fileList;

	if (m_loadableContentListBase.isEmpty()) {
		m_client->send(HttpConnection::ApiGeneral, QStringLiteral("loadable"))
				->done(this, [this, fileList](const QJsonObject &json)
		{
			m_loadableContentListBase.clear();

			const QJsonArray &list = json.value(QStringLiteral("list")).toArray();
			for (const QJsonValue &v : list) {
				const QJsonObject &o = v.toObject();

				Server::DynamicContent content;
				content.name = o.value(QStringLiteral("file")).toString();
				content.md5 = o.value(QStringLiteral("md5")).toString();
				content.size = JSON_TO_INTEGER(o.value(QStringLiteral("size")));
				m_loadableContentListBase.append(content);
			}

			downloadLoadableContent(fileList);
		})
				->fail(this, [this](const QString &err){
			LOG_CERROR("game") << "Loadable content error" << qPrintable(err);
			setError();
		});
	} else {
		m_downloader->contentClear();

		for (const QString &s : fileList) {
			auto it = std::find_if(m_loadableContentListBase.constBegin(),
								   m_loadableContentListBase.constEnd(),
								   [&s](const Server::DynamicContent &c) {
				return c.name == s;
			});

			if (it == m_loadableContentListBase.constEnd()) {
				LOG_CERROR("game") << "Invalid loadable resource:" << qPrintable(s);
				setError();
				return;
			}

			m_downloader->contentAdd(*it);
		}

		m_downloader->download();
	}
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


	// Game inventory

	for (const QString &s : m_rpgGame->m_gameDefinition.inventoryOnce) {
		loadInventory(player, RpgPickableObject::typeFromString(s));
	}

	m_rpgGame->m_gameDefinition.inventoryOnce.clear();

	for (const QString &s : m_rpgGame->m_gameDefinition.inventory) {
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

		case RpgPickableObject::PickableKey:
			player->inventoryAdd(pickableType /*, name ???? */);		/// TODO: name handling
			break;

		case RpgPickableObject::PickableHp:
		case RpgPickableObject::PickableShortbow:
		case RpgPickableObject::PickableLongbow:
		case RpgPickableObject::PickableArrow:
		case RpgPickableObject::PickableFireball:
		case RpgPickableObject::PickableLongsword:
		case RpgPickableObject::PickableDagger:
		case RpgPickableObject::PickableTime:
			LOG_CERROR("game") << "Inventory type not supported:" << pickableType;
			break;

		case RpgPickableObject::PickableInvalid:
			LOG_CERROR("game") << "Invalid inventory type";
			break;
	}

}



/**
 * @brief ActionRpgGame::loadWeapon
 * @param player
 * @param type
 * @param bullet
 */

void ActionRpgGame::loadWeapon(RpgPlayer *player, const TiledWeapon::WeaponType &type, const int &bullet)
{
	TiledWeapon *weapon = player->armory()->weaponFind(type);

	if (!weapon) {
		switch (type) {
			case TiledWeapon::WeaponLongsword:
				weapon = player->armory()->weaponAdd(new RpgLongsword);
				break;

			case TiledWeapon::WeaponShortbow:
				weapon = player->armory()->weaponAdd(new RpgShortbow);
				break;

			case TiledWeapon::WeaponLongbow:
				weapon = player->armory()->weaponAdd(new RpgLongbow);
				break;

			case TiledWeapon::WeaponDagger:
				weapon = player->armory()->weaponAdd(new RpgDagger);
				break;

			case TiledWeapon::WeaponBroadsword:
				weapon = player->armory()->weaponAdd(new RpgBroadsword);
				break;

			case TiledWeapon::WeaponHand:
			case TiledWeapon::WeaponGreatHand:
			case TiledWeapon::WeaponShield:
			case TiledWeapon::WeaponInvalid:
				LOG_CERROR("game") << "Invalid weapon type" << type;
				return;

		}
	}

	weapon->setBulletCount(weapon->bulletCount()+bullet);

	player->armory()->setCurrentWeaponIf(weapon, TiledWeapon::WeaponHand);
}



/**
 * @brief ActionRpgGame::loadBullet
 * @param player
 * @param bulletType
 * @param count
 */

void ActionRpgGame::loadBullet(RpgPlayer *player, const RpgPickableObject::PickableType &bulletType, const int &count)
{
	if (!player)
		return;

	TiledWeapon *weapon = nullptr;

	switch (bulletType) {
		case RpgPickableObject::PickableArrow:
			weapon = player->armory()->weaponFind(TiledWeapon::WeaponShortbow);
			break;

		case RpgPickableObject::PickableFireball:
			weapon = player->armory()->weaponFind(TiledWeapon::WeaponLongbow);
			break;

		case RpgPickableObject::PickableShield:
		case RpgPickableObject::PickableKey:
		case RpgPickableObject::PickableHp:
		case RpgPickableObject::PickableShortbow:
		case RpgPickableObject::PickableLongbow:
		case RpgPickableObject::PickableLongsword:
		case RpgPickableObject::PickableDagger:
		case RpgPickableObject::PickableTime:
		case RpgPickableObject::PickableInvalid:
			LOG_CERROR("game") << "Invalid bullet type";
			break;
	}

	if (!weapon)
		return;

	weapon->setBulletCount(weapon->bulletCount()+count);

	player->armory()->setCurrentWeaponIf(weapon, TiledWeapon::WeaponHand);
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
 * @brief ActionRpgGame::onPlayerUseContainer
 * @param player
 * @param container
 * @return
 */

bool ActionRpgGame::onPlayerUseContainer(RpgPlayer *player, TiledContainer *container)
{
	if (!player || !container)
		return false;

	if (container && m_rpgQuestion->emptyQuestions()) {
		m_rpgGame->playerUseContainer(player, container);
		return true;
	}

	if (m_rpgQuestion->nextQuestion(player, nullptr, TiledWeapon::WeaponInvalid, container)) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"), Sound::SfxChannel);
		return true;
	}

	return false;
}



/**
 * @brief ActionRpgGame::onQuestionSuccess
 * @param player
 * @param enemy
 * @param xp
 */

void ActionRpgGame::onQuestionSuccess(RpgPlayer *player, IsometricEnemy *enemy, TiledContainer *container, int xp)
{
	if (enemy)
		enemy->setHp(0);

	if (container)
		m_rpgGame->playerUseContainer(player, container);

	setXp(m_xp+xp);
}


/**
 * @brief ActionRpgGame::onQuestionFailed
 * @param player
 * @param enemy
 */

void ActionRpgGame::onQuestionFailed(RpgPlayer *player, IsometricEnemy *enemy, TiledContainer */*container*/)
{
	if (player)
		player->setHp(std::max(0, player->hp()-1));

	setIsFlawless(false);

	if (enemy && player && !enemy->player()) {
		enemy->rotateToPlayer(player);
	}
}



/**
 * @brief ActionRpgGame::gameid
 * @return
 */

int ActionRpgGame::gameid() const
{
	if (const CampaignGameIface *iface = dynamic_cast<const CampaignGameIface*>(this)) {
		return iface->gameId();
	}

	return -1;
}


/**
 * @brief ActionRpgGame::downloader
 * @return
 */

Downloader *ActionRpgGame::downloader() const
{
	return m_downloader.get();
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
		disconnect(m_rpgGame, &RpgGame::gameLoadFailed, this, &ActionRpgGame::onGameLoadFailed);
		disconnect(m_rpgGame, &RpgGame::marketRequest, this, &ActionRpgGame::marketRequest);
		disconnect(this, &ActionRpgGame::marketUnloaded, m_rpgGame, &RpgGame::onMarketUnloaded);
		disconnect(this, &ActionRpgGame::marketLoaded, m_rpgGame, &RpgGame::onMarketLoaded);
		m_rpgGame->setRpgQuestion(nullptr);
		m_rpgGame->setFuncPlayerPick(nullptr);
		m_rpgGame->setFuncPlayerAttackEnemy(nullptr);
		m_rpgGame->setFuncPlayerUseContainer(nullptr);
	}

	m_rpgGame = newRpgGame;
	emit rpgGameChanged();

	if (m_rpgGame) {
		setGameQuestion(m_rpgGame->gameQuestion());
		m_rpgGame->setRpgQuestion(m_rpgQuestion.get());
		connect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgGame::onGameSuccess);
		connect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgGame::onPlayerDead);
		connect(m_rpgGame, &RpgGame::gameLoadFailed, this, &ActionRpgGame::onGameLoadFailed);
		connect(m_rpgGame, &RpgGame::marketRequest, this, &ActionRpgGame::marketRequest);
		connect(this, &ActionRpgGame::marketUnloaded, m_rpgGame, &RpgGame::onMarketUnloaded);
		connect(this, &ActionRpgGame::marketLoaded, m_rpgGame, &RpgGame::onMarketLoaded);
		m_rpgGame->setFuncPlayerPick(std::bind(&ActionRpgGame::onPlayerPick, this, std::placeholders::_1, std::placeholders::_2));
		m_rpgGame->setFuncPlayerAttackEnemy(std::bind(&ActionRpgGame::onPlayerAttackEnemy, this,
													  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_rpgGame->setFuncPlayerUseContainer(std::bind(&ActionRpgGame::onPlayerUseContainer, this, std::placeholders::_1, std::placeholders::_2));
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
