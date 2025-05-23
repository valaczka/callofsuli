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
#include "rpgcoin.h"
#include "rpgmp.h"
#include "rpgarmory.h"


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
		m_config.gameState = RpgConfig::StateConnect;
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
 * @brief ActionRpgGame::getExtendedData
 * @return
 */

QJsonObject ActionRpgGame::getExtendedData() const
{
	return {
		{ QStringLiteral("map"), m_playerConfig.terrain }
	};
}




/**
 * @brief ActionRpgGame::msecLeft
 * @return
 */

int ActionRpgGame::msecLeft() const
{
	return std::max((qint64) 0, (qint64) (m_deadlineTick-m_elapsedTick * (1000./60.)));
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

	downloadGameData(terrain, {m_playerConfig});

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
			const RpgGameData::Weapon::WeaponType type = RpgArmory::weaponHash().key(wallet->market().name, RpgGameData::Weapon::WeaponInvalid);
			if (type == RpgGameData::Weapon::WeaponInvalid) {
				LOG_CERROR("game") << "Invalid weapon" << wallet->market().name;
				return;
			}

			RpgWeapon *weapon = player->armory()->weaponFind(type);

			if (weapon) {
				LOG_CDEBUG("game") << "Weapon already exists";
				return;
			}

			loadWeapon(player, type, wallet->market().cost == 0 ? -1 : wallet->amount()
																  /*wallet->bullet() ? wallet->bullet()->amount() : 0*/);

			return;
		}

		case RpgMarket::Hp:
			player->setHp(player->hp() + wallet->market().amount);
			return;

		case RpgMarket::Mp:
			player->setMp(player->mp() + wallet->market().amount);
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
 * @brief ActionRpgGame::getDisabledWeapons
 * @param character
 * @return
 */

QStringList ActionRpgGame::getDisabledWeapons(const QString &character)
{
	const auto characterPtr = RpgGame::characters().find(character);

	if (characterPtr == RpgGame::characters().constEnd()) {
		LOG_CERROR("game") << "Invalid character" << character;
		return {};
	}

	return characterPtr->disabledWeapons;
}



/**
 * @brief ActionRpgGame::onPlayerDead
 * @param player
 */

void ActionRpgGame::onPlayerDead(RpgPlayer *player)
{
	LOG_CDEBUG("game") << "Player dead" << player;

	m_playerResurrect.time = m_elapsedTick + AbstractGame::TickTimer::msecToTick(5000);
	m_playerResurrect.player = player;
}



/**
 * @brief ActionRpgGame::onTimerLeftTimeout
 */

void ActionRpgGame::onTimerLeftTimeout()
{
	if (!m_rpgGame)
		return;

	const qint64 tick = m_rpgGame->tickTimer() ? m_rpgGame->tickTimer()->currentTick() : -1;

	if (tick < 0 || m_deadlineTick <= 0)
		return;


	m_elapsedTick = tick;
	emit msecLeftChanged();

	if (m_deadlineTick > 0 && m_elapsedTick >= m_deadlineTick) {
		LOG_CDEBUG("game") << "Game timeout";

		m_timerLeft.stop();
		emit gameTimeout();
		return;
	}


	if (m_playerResurrect.time > 0 && tick >= m_playerResurrect.time) {
		if (finishState() == Fail || finishState() == Success) {
			m_playerResurrect.player = nullptr;
			m_playerResurrect.time = -1;
			return;
		}

		if (m_playerResurrect.player)
			m_rpgGame->setQuestions(m_playerResurrect.player->scene(), m_missionLevel->questions());

		m_rpgGame->resurrectEnemiesAndPlayer(m_playerResurrect.player);
		loadInventory(m_playerResurrect.player);

		m_playerResurrect.player = nullptr;
		m_playerResurrect.time = -1;
	}

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

	m_rpgGame->m_level = level();

	const auto &ptr = RpgGame::terrains().find(m_playerConfig.terrain);

	if (ptr == RpgGame::terrains().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid terrain" << m_playerConfig.terrain;
		return;
	}

	const auto characterPtr = RpgGame::characters().find(m_playerConfig.character);

	if (characterPtr == RpgGame::characters().constEnd()) {
		LOG_CERROR("game") << "Game load error: invalid character" << m_playerConfig.character;
		return;
	}

	if (!m_rpgGame->load(ptr.value(), characterPtr->cast == RpgPlayerCharacterConfig::CastInvalid)) {
		LOG_CERROR("game") << "Game load error";
		return;
	}

	TiledScene *firstScene = m_rpgGame->findScene(ptr->firstScene);

	if (!firstScene) {
		LOG_CERROR("game") << "Game load error: missing first scene";
		return;
	}

	const auto &ptrPos = m_rpgGame->playerPosition(firstScene, 0);

	QList<RpgPlayer*> list;

	RpgPlayer *player = m_rpgGame->createPlayer(firstScene, *characterPtr, 0);

	if (!player) {
		LOG_CERROR("game") << "Player create error";
		return;
	}

	const int hp = m_missionLevel->startHP() + ptr->playerHP;

	player->setHp(hp*10);
	player->setMaxHp(hp*10);
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
		for (const QString &s : m_playerConfig.weapons) {
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



	player->emplace(ptrPos.value_or(QPointF{0,0}));
	player->setCurrentAngle(TiledObject::directionToRadian(TiledObject::West));

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


	m_config.duration = ptr->duration + (m_rpgQuestion->duration() * factor);
	updateConfig();

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

	if (m_config.gameState == RpgConfig::StateConnect) {
		m_config.gameState = RpgConfig::StateCharacterSelect;
		updateConfig();
		return;
	}

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

		//startWithRemainingTime(m_config.duration*1000);
		gameStart();
		m_deadlineTick = AbstractGame::TickTimer::msecToTick(m_config.duration*1000);
		m_elapsedTick = 0;
		m_rpgGame->tickTimer()->start(this, 0);
		m_timerLeft.start();

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

void ActionRpgGame::downloadGameData(const QString &map, const QList<RpgGameData::CharacterSelect> &players)
{
	Server *server = m_client->server();

	if (!server) {
		LOG_CERROR("game") << "Missing server";
		setError();
		return;
	}

	const auto &ptr = RpgGame::terrains().find(map);

	if (ptr == RpgGame::terrains().constEnd()) {
		LOG_CERROR("game") << "Invalid game";
		setError();
		return;
	}

	auto listPtr = TiledGame::getDynamicTilesets(ptr.value());

	if (!listPtr)
		return;

	for (const auto &config : players) {

		m_downloader->setServer(server);

		listPtr->append(config.character+QStringLiteral(".full.dres"));

		if (auto it = RpgGame::characters().find(config.character); it != RpgGame::characters().constEnd() && !it->base.isEmpty()) {
			listPtr->append(it->base+QStringLiteral(".full.dres"));
		}

		listPtr->append(ptr->required);
	}

	downloadLoadableContentDict(*listPtr);
}




/**
 * @brief ActionRpgGame::onTimeStepPrepare
 */

void ActionRpgGame::onTimeStepPrepare()
{

}


void ActionRpgGame::onTimeBeforeWorldStep(const qint64 &tick)
{
	Q_UNUSED(tick);
}




/**
 * @brief ActionRpgGame::onTimeStepped
 */

void ActionRpgGame::onTimeStepped()
{
	m_rpgGame->iterateOverBodies([this](TiledObjectBody *b){
		if (RpgBullet *iface = dynamic_cast<RpgBullet*> (b)) {
			if (iface->stage() == RpgGameData::LifeCycle::StageDestroy ||
					iface->stage() == RpgGameData::LifeCycle::StageDead) {
				LOG_CINFO("game") << "****** DELETE" << iface;
				ActionRpgGame::onBulletDelete(iface);
			}
		}
	});
}

void ActionRpgGame::onTimeAfterWorldStep(const qint64 &tick)
{
	Q_UNUSED(tick);
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
				content.size = o.value(QStringLiteral("size")).toInteger();
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

void ActionRpgGame::loadInventory(RpgPlayer *player, const RpgGameData::PickableBaseData::PickableType &pickableType)
{
	if (!player)
		return;

	switch (pickableType) {
		case RpgGameData::PickableBaseData::PickableShield:
			RpgShieldPickable::pick(player, m_rpgGame);
			break;

		case RpgGameData::PickableBaseData::PickableKey:
			player->inventoryAdd(pickableType /*, name ???? */);		/// TODO: name handling
			break;

		case RpgGameData::PickableBaseData::PickableMp:
		case RpgGameData::PickableBaseData::PickableHp:
		case RpgGameData::PickableBaseData::PickableCoin:
		case RpgGameData::PickableBaseData::PickableShortbow:
		case RpgGameData::PickableBaseData::PickableLongbow:
		case RpgGameData::PickableBaseData::PickableLongsword:
		case RpgGameData::PickableBaseData::PickableDagger:
		case RpgGameData::PickableBaseData::PickableTime:
			LOG_CERROR("game") << "Inventory type not supported:" << pickableType;
			break;

		case RpgGameData::PickableBaseData::PickableInvalid:
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

void ActionRpgGame::loadWeapon(RpgPlayer *player, const RpgGameData::Weapon::WeaponType &type, const int &bullet)
{
	RpgWeapon *weapon = player->armory()->weaponAdd(type);

	if (!weapon) {
		LOG_CERROR("game") << "Invalid weapon type" << type;
		return;
	}

	if (bullet == -1)
		weapon->setBulletCount(-1);
	else if (weapon->bulletCount() != -1)
		weapon->setBulletCount(weapon->bulletCount()+bullet);
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


	if (pickable->pickableType() == RpgGameData::PickableBaseData::PickableTime) {
		static int sec = 60;
		//addToDeadline(sec*1000);
		m_deadlineTick += AbstractGame::TickTimer::msecToTick(sec*1000);
		m_msecNotifyAt = 0;
		m_rpgGame->messageColor(tr("%1 seconds gained").arg(sec), QStringLiteral("#00bcd4"));
	} else if (pickable->pickableType() == RpgGameData::PickableBaseData::PickableCoin) {
		const auto num = RpgCoinPickable::amount(!m_rpgQuestion->emptyQuestions());
		m_rpgGame->setCurrency(m_rpgGame->currency()+num);
		m_rpgGame->messageColor(tr("%1 coins gained").arg(num), QStringLiteral("#FB8C00"));
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

bool ActionRpgGame::onPlayerAttackEnemy(RpgPlayer *player, RpgEnemy *enemy, const RpgGameData::Weapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;

	RpgGameData::Enemy e = enemy->serialize();

	e.attacked(enemy->baseData(), weaponType, player->baseData());

	int xp = std::max(0, e.hp-enemy->hp());
	setXp(m_xp+xp);

	enemy->attackedByPlayer(player, weaponType);

	enemy->updateFromSnapshot(e);


	return true;
}


/**
 * @brief ActionRpgGame::onPlayerUseContainer
 * @param player
 * @param container
 * @return
 */

bool ActionRpgGame::onPlayerUseControl(RpgPlayer *player, RpgActiveIface *control)
{
	if (!player || !control)
		return false;

	if (control && m_rpgQuestion->emptyQuestions()) {
		m_rpgGame->playerUseControl(player, control);
		return true;
	}

	if (m_rpgQuestion->nextQuestion(player, nullptr, RpgGameData::Weapon::WeaponInvalid, control)) {
		m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/question.mp3"), Sound::SfxChannel);
		return true;
	}


	return false;
}


/**
 * @brief ActionRpgGame::onPlayerUseCast
 * @param player
 * @param weaponType
 * @return
 */

bool ActionRpgGame::onPlayerUseCast(RpgPlayer *player)
{
	if (!player || !player->isAlive())
		return false;

	LOG_CERROR("game") << "Missing implementation";

	return false;

	/*RpgMageStaff *m = qobject_cast<RpgMageStaff*>(player->armory()->weaponFind(RpgGameData::Weapon::WeaponMageStaff));

	if (!m)
		return false;


	const int castValue = m_rpgGame->getMetric(player->config().cast);

	switch (player->config().cast) {
		case RpgPlayerCharacterConfig::CastInvisible:
			if (player->castTimerActive()) {
				onPlayerFinishCast(player);
			} else {
				player->m_castTimer.start();
				player->m_effectRing.setSource(QStringLiteral(":/rpg/common/smoke.png"));
				player->m_effectRing.play();
			}
			break;

		case RpgPlayerCharacterConfig::CastFireball: {
			RpgLongbow w;
			w.setParentObject(player);
			w.setBulletCount(-1);
			if (m_rpgGame->shot(player, &w, player->scene(), IsometricBullet::TargetEnemy, player->currentAngle())) {
				player->setMp(std::max(0, player->mp() - castValue));
			} else {
				return false;
			}
			break;
		}

		case RpgPlayerCharacterConfig::CastFireballTriple: {
			RpgLongbow w;
			w.setParentObject(player);
			w.setBulletCount(-1);
			w.setDisableTimerRepeater(true);

			const qreal origAngle = player->currentAngle();

			for (const qreal &angle : std::vector<qreal>{
				 origAngle - (M_PI * 12.5 / 180.),
				 origAngle + (M_PI * 12.5 / 180.),
				 origAngle
			}) {
				if (!m_rpgGame->shot(player, &w, player->scene(), IsometricBullet::TargetEnemy, angle)) {
					return false;
				}
				player->setMp(std::max(0, player->mp() - castValue));
			}

			break;
		}


		case RpgPlayerCharacterConfig::CastLightning: {
			RpgLightningWeapon w;
			w.setParentObject(player);
			w.setBulletCount(-1);
			if (m_rpgGame->shot(player, &w, player->scene(), IsometricBullet::TargetEnemy, player->currentAngle())) {
				player->setMp(std::max(0, player->mp() - castValue));
			} else {
				return false;
			}
			break;
		}


		case RpgPlayerCharacterConfig::CastFireFog:
			if (player->castTimerActive()) {
				onPlayerFinishCast(player);
			} else {
				player->m_castTimer.start();
				player->m_effectRing.setSource(QStringLiteral(":/rpg/common/firefog.png"));
				player->m_effectRing.play();
				player->attackReachedEnemies(RpgGameData::Weapon::WeaponFireFogWeapon);
			}
			break;


		case RpgPlayerCharacterConfig::CastArrowQuintuple: {
			RpgShortbow w;
			w.setParentObject(player);
			w.setBulletCount(-1);
			w.setDisableTimerRepeater(true);

			const qreal origAngle = player->currentAngle();

			for (const qreal &angle : std::vector<qreal>{
				 origAngle - (M_PI * 12 / 180.),
				 origAngle - (M_PI * 6 / 180.),
				 origAngle + (M_PI * 6 / 180.),
				 origAngle + (M_PI * 12 / 180.),
				 origAngle
			}) {
				if (!m_rpgGame->shot(player, &w, player->scene(), IsometricBullet::TargetEnemy, angle)) {
					return false;
				}
				player->setMp(std::max(0, player->mp() - castValue));
			}

			break;
		}

		case RpgPlayerCharacterConfig::CastProtect:
			if (player->castTimerActive()) {
				onPlayerFinishCast(player);
			} else {
				player->m_castTimer.start();
				player->m_effectRing.setSource(QStringLiteral(":/rpg/common/protect.png"));
				player->m_effectRing.play();
			}
			break;

		case RpgPlayerCharacterConfig::CastInvalid:
			return false;
	}

	player->playAttackEffect(m);
	m->eventUseCast(player->config().cast);

	return true; */
}



/**
 * @brief ActionRpgGame::onPlayerCastTimeout
 * @param player
 * @return
 */

bool ActionRpgGame::onPlayerCastTimeout(RpgPlayer *player)
{
	LOG_CERROR("game") << "Missing implementation";
	return false;

	/*if (!player || player->m_isLocked)
		return false;

	const int castValue = m_rpgGame->getMetric(player->config().cast);

	int nextMp = player->m_mp - castValue;

	if (nextMp <= 0) {
		player->setMp(0);

		m_rpgGame->playerFinishCast(player);
		return true;
	}

	player->setMp(nextMp);

	return true;*/
}




/**
 * @brief ActionRpgGame::onPlayerFinishCast
 * @param player
 * @return
 */

bool ActionRpgGame::onPlayerFinishCast(RpgPlayer *player)
{
	if (!player)
		return false;

	switch (player->config().cast) {
		case RpgPlayerCharacterConfig::CastInvisible:
		case RpgPlayerCharacterConfig::CastFireFog:
		case RpgPlayerCharacterConfig::CastProtect:
			if (player->castTimerActive()) {
				player->m_castTimer.stop();
				player->m_effectRing.stop();
			}
			break;

		case RpgPlayerCharacterConfig::CastFireball:
		case RpgPlayerCharacterConfig::CastFireballTriple:
		case RpgPlayerCharacterConfig::CastArrowQuintuple:
		case RpgPlayerCharacterConfig::CastLightning:
		case RpgPlayerCharacterConfig::CastInvalid:
			return false;
	}


	return true;
}




/**
 * @brief ActionRpgGame::onPlayerHit
 * @param player
 * @param enemy
 * @param weapon
 * @return
 */

bool ActionRpgGame::onPlayerHit(RpgPlayer *player, RpgEnemy *enemy, RpgWeapon *weapon)
{
	LOG_CWARNING("game") << "PLAYER" << "HIT" << enemy << weapon << m_rpgGame->tickTimer()->currentTick();

	if (!player || !weapon)
		return false;

	if (!weapon->hit(enemy))
		return false;

	if (weapon->bulletCount() > 0)
		weapon->setBulletCount(weapon->bulletCount()-1);

	m_rpgGame->playerAttackEnemy(player, enemy, weapon->weaponType());

	if (weapon->pickedBulletCount() > 0)
		weapon->setPickedBulletCount(weapon->pickedBulletCount()-1);
	else
		m_rpgGame->useWeapon(weapon->weaponType());

	player->playAttackEffect(weapon);

	return true;
}




/**
 * @brief ActionRpgGame::onPlayerShot
 * @param player
 * @param weapon
 * @param angle
 * @return
 */

bool ActionRpgGame::onPlayerShot(RpgPlayer *player, RpgWeapon *weapon, const qreal &angle)
{
	if (!player || !weapon)
		return false;

	if (!weapon->shot())
		return false;


	RpgBullet *bullet = m_rpgGame->createBullet(weapon, player->scene(), player->nextObjectId(), player->baseData().o, true);

	if (!bullet) {
		LOG_CWARNING("game") << "Can't create bullet";
		return false;
	}

	LOG_CDEBUG("game") << "BULLET CREATED" << bullet->baseData().o << bullet->baseData().s << bullet->baseData().id;

	bullet->setOwner(RpgGameData::BulletBaseData::OwnerPlayer);
	bullet->setTargets(RpgGameData::BulletBaseData::TargetEnemy | RpgGameData::BulletBaseData::TargetGround);
	bullet->setMaxDistance(weapon->bulletDistance());
	bullet->shot(player->bodyPosition(), angle);

	player->playAttackEffect(weapon);


	if (weapon->bulletCount() > 0)
		weapon->setBulletCount(weapon->bulletCount()-1);

	if (weapon->pickedBulletCount() > 0)
		weapon->setPickedBulletCount(weapon->pickedBulletCount()-1);
	else
		m_rpgGame->useWeapon(weapon->weaponType());

	return true;
}


/**
 * @brief ActionRpgGame::onEnemyHit
 * @param enemy
 * @param player
 * @param weapon
 * @return
 */

bool ActionRpgGame::onEnemyHit(RpgEnemy *enemy, RpgPlayer *player, RpgWeapon *weapon)
{
	if (!enemy || !weapon)
		return false;

	if (!weapon->hit(player))
		return false;


	if (weapon->bulletCount() > 0)
		weapon->setBulletCount(weapon->bulletCount()-1);

	m_rpgGame->enemyAttackPlayer(enemy, player, weapon->weaponType());

	enemy->playAttackEffect(weapon);

	return true;
}


/**
 * @brief ActionRpgGame::onEnemyShot
 * @param enemy
 * @param weapon
 * @param angle
 * @return
 */

bool ActionRpgGame::onEnemyShot(RpgEnemy *enemy, RpgWeapon *weapon, const qreal &angle)
{
	return false;
}



/**
 * @brief ActionRpgGame::onEnemyAttackPlayer
 * @param enemy
 * @param player
 * @param weaponType
 * @return
 */

bool ActionRpgGame::onEnemyAttackPlayer(RpgEnemy *enemy, RpgPlayer *player, const RpgGameData::Weapon::WeaponType &weaponType)
{
	if (!player || !enemy)
		return false;

	RpgGameData::Player p = player->serialize();
	p.attacked(player->baseData(), weaponType, enemy->baseData());

	const bool prot = p.hp == player->hp();

	player->updateFromSnapshot(p);
	player->attackedByEnemy(enemy, weaponType, prot);

	return true;
}



/**
 * @brief ActionRpgGame::onBulletImpact
 * @param bullet
 * @param other
 */

bool ActionRpgGame::onBulletImpact(RpgBullet *bullet, TiledObjectBody *other)
{
	if (!bullet || !other)
		return false;

	bool isGround = false;

	for (const auto &sh : other->bodyShapes()) {
		if ((cpShapeGetFilter(sh).categories & TiledObjectBody::FixtureGround) && other->opaque()) {
			isGround = true;
			break;
		}
	}

	if (isGround) {
		bullet->setImpacted(true);
		bullet->disableBullet();
		bullet->setStage(RpgGameData::LifeCycle::StageDead);
		return true;
	}


	RpgEnemy *enemy = dynamic_cast<RpgEnemy*>(other);
	RpgPlayer *player = dynamic_cast<RpgPlayer*>(other);
	TiledObjectBody *owner = m_rpgGame->findBody(TiledObjectBody::ObjectId{
													 .ownerId = bullet->ownerId().o,
													 .sceneId = bullet->ownerId().s,
													 .id = bullet->ownerId().id
												 });

	if (RpgPlayer *ownP = dynamic_cast<RpgPlayer*>(owner); ownP && enemy) {
		m_rpgGame->playerAttackEnemy(ownP, enemy, bullet->weaponType());
	}

	if (RpgEnemy *ownE = dynamic_cast<RpgEnemy*>(owner); ownE && player) {
		m_rpgGame->enemyAttackPlayer(ownE, player, bullet->weaponType());
	}

	/// TODO: player attack player?

	bullet->setImpacted(true);
	bullet->disableBullet();
	bullet->setStage(RpgGameData::LifeCycle::StageDead);

	return true;
}


/**
 * @brief ActionRpgGame::onBulletDelete
 * @param bullet
 */

void ActionRpgGame::onBulletDelete(IsometricBullet *bullet)
{
	if (m_rpgGame)
		m_rpgGame->removeObject(bullet);
}



/**
 * @brief ActionRpgGame::onQuestionSuccess
 * @param player
 * @param enemy
 * @param xp
 */

void ActionRpgGame::onQuestionSuccess(RpgPlayer *player, RpgEnemy *enemy, RpgActiveIface *control, int xp)
{
	if (enemy)
		enemy->setHp(0);

	if (control)
		m_rpgGame->playerUseControl(player, control);

	setXp(m_xp+xp);

	if (player->config().cast != RpgPlayerCharacterConfig::CastInvalid && m_gameQuestion && !control) {
		const int mp = /*m_gameQuestion->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * */
					   RpgMpPickable::amount() * (2 + (0.33*m_missionLevel->level()));
		RpgMpPickable::pick(player, mp);
	}
}






/**
 * @brief ActionRpgGame::onQuestionFailed
 * @param player
 * @param enemy
 */

void ActionRpgGame::onQuestionFailed(RpgPlayer *player, RpgEnemy *enemy, RpgActiveIface *control)
{
	setIsFlawless(false);

	if (!player)
		return;

	if (control) {
		RpgGameData::Player p = player->serialize();
		p.controlFailed(control->activeType());
		player->updateFromSnapshot(p);
	}

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

		m_rpgGame->disconnect(this);
		this->disconnect(m_rpgGame);

		m_rpgGame->setActionRpgGame(nullptr);

		m_rpgGame->setRpgQuestion(nullptr);
	}

	m_rpgGame = newRpgGame;
	emit rpgGameChanged();

	if (m_rpgGame) {
		setGameQuestion(m_rpgGame->gameQuestion());
		m_rpgGame->setRpgQuestion(m_rpgQuestion.get());

		m_rpgGame->setActionRpgGame(this);

		connect(m_rpgGame, &RpgGame::gameSuccess, this, &ActionRpgGame::onGameSuccess, Qt::QueuedConnection);		// Azért kell, mert különbön az utolsó fegyverhasználatot nem számolja el a szerveren
		connect(m_rpgGame, &RpgGame::playerDead, this, &ActionRpgGame::onPlayerDead);
		connect(m_rpgGame, &RpgGame::gameLoadFailed, this, &ActionRpgGame::onGameLoadFailed);
		connect(this, &ActionRpgGame::marketUnloaded, m_rpgGame, &RpgGame::onMarketUnloaded);
		connect(this, &ActionRpgGame::marketLoaded, m_rpgGame, &RpgGame::onMarketLoaded);
	}
}



/**
 * @brief ActionRpgGame::config
 * @return
 */

const RpgConfig &ActionRpgGame::config() const
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
