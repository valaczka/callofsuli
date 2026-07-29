/*
 * ---- Call of Suli ----
 *
 * rpggame.cpp
 *
 * Created on: 2026. 05. 08.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * RpgGame
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

#include <libtiledquick/tilelayeritem.h>
#include <libtiled/imagecache.h>
#include <libtiled/map.h>
#include <libtiled/mapreader.h>
#include "application.h"
#include "gamequestion.h"
#include "litegame.h"
#include "rpgchanger.h"
#include "rpgnpc.h"
#include "rpgplayer.h"
#include "rpgstream.h"
#include "rpgconfig.h"
#include "rpgcontrol.h"
#include "rpguserwallet.h"
#include "tiledgame.h"
#include "rpgdefender.h"
#include "rpggame.h"
#include "rpggame_p.h"
#include "rpggameitem.h"
#include "rpgmp.h"
#include "server.h"
#include "Logger.h"
#include "client.h"
#include "downloader.h"
#include "utils_.h"


#ifndef Q_OS_WASM
#include "standaloneclient.h"
#endif

#ifdef WITH_FTXUI
#include "desktopapplication.h"
#endif


QHash<QString, RpgGameDefinition> RpgGame::m_terrains = {};
QHash<QString, RpgPlayerDefinition> RpgGame::m_characters = {};

const QColor RpgGame::m_colorTeam = QColorConstants::Svg::dodgerblue;
const QColor RpgGame::m_colorOpponent = QColorConstants::Svg::red;
const QColor RpgGame::m_colorNeutral = QColorConstants::Svg::whitesmoke;
const QColor RpgGame::m_colorGlow = QColorConstants::Svg::wheat;


/**
 * @brief RpgGame::RpgGame
 * @param missionLevel
 * @param client
 */

RpgGame::RpgGame(GameMapMissionLevel *missionLevel, Client *client, const bool &multiplayer,
				 std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial)
	: AbstractLevelGame(GameMap::Rpg, missionLevel, client)
	, d(new RpgGamePrivate(this, multiplayer, std::move(tutorial)))
	, m_gameMode(multiplayer ? MultiPlayer : SinglePlayer)
	, m_modelLobby(new QSListModel)
	, m_modelPlayer(new QSListModel)
	, m_modelCharacters(new QSListModel)
{
	Q_ASSERT(client);

	LOG_CDEBUG("client") << "RpgGame created" << this;

	connect(client->downloader(), &Downloader::stateChanged, d, &RpgGamePrivate::onDownloaderStateChanged);
	connect(client->downloader(), &Downloader::downloadError, d, &RpgGamePrivate::onContentError);

	m_modelLobby->setRoleNames(QStringList{
								   QStringLiteral("roomId"),
								   QStringLiteral("readableId"),
							   });


	m_modelPlayer->setRoleNames(QStringList{
									QStringLiteral("playerId"),
									QStringLiteral("username"),
									QStringLiteral("nickname"),
									QStringLiteral("character"),
									QStringLiteral("power"),
									QStringLiteral("team"),
								});

	m_modelCharacters->setRoleNames(Utils::getRolesFromObject(RpgUserCharacter().metaObject())
									<< QStringLiteral("nextPoint")
									<< QStringLiteral("isTarget")
									<< QStringLiteral("targetToken")
									);
}


/**
 * @brief RpgGame::~RpgGame
 */

RpgGame::~RpgGame()
{
	d->clearSharedTextures();

	delete d;
	d = nullptr;

	LOG_CDEBUG("client") << "RpgGame destroyed" << this;
}



/**
 * @brief RpgGame::gameAbort
 */

void RpgGame::gameAbort()
{
	if (m_gameState == GameStateFinished ||
			m_gameState == GameStateResult ||
			m_gameState == GameStateAbort)
		return;

	else if (m_gameState == GameStateConnect ||
			 m_gameState == GameStateDownloadContent ||
			 m_gameState == GameStateLobby ||
			 m_gameState == GameStateError ||
			 m_gameState == GameStateCharacterSelect) {
		setFinishState(Neutral);

		LOG_CINFO("game") << "Game cancelled:" << this;

		gameFinish();
		return;
	}

	LOG_CINFO("game") << "Game aborted:" << this;

	d->finishGame(true);
}



/**
 * @brief RpgGame::loadGameItem
 */

void RpgGame::loadGameItem()
{
	LOG_CDEBUG("game") << "Load GameItem";

	QMetaObject::invokeMethod(d, &RpgGamePrivate::prepareGameItem, Qt::QueuedConnection);
}



/**
 * @brief RpgGame::gameItemPrepared
 */

void RpgGame::gameItemPrepared()
{
	LOG_CDEBUG("game") << "GameItem prepared";

	QMetaObject::invokeMethod(d, &RpgGamePrivate::onGameItemPrepared, Qt::QueuedConnection);
}





/**
 * @brief RpgGame::downloadAccepted
 */

void RpgGame::downloadAccepted()
{
	m_client->downloader()->download();
}



/**
 * @brief RpgGame::menuBgMusicPlay
 */

void RpgGame::menuBgMusicPlay()
{
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/menu/bg.mp3"), Sound::MusicChannel);
}


/**
 * @brief RpgGame::menuBgMusicStop
 */

void RpgGame::menuBgMusicStop()
{
	m_client->sound()->stopMusic();
}



/**
 * @brief RpgGame::reloadLobby
 */

void RpgGame::reloadLobby()
{
	if (d->m_engine)
		d->m_engine->lobbyReload();
}


/**
 * @brief RpgGame::connectLobby
 * @param data
 */

void RpgGame::connectLobby(const QVariantMap &data)
{
	if (!d->m_engine)
		return;

	int room = data.value(QStringLiteral("roomId")).toInt();

	LOG_CERROR("game") << "***" << room;

	if (room == 0)
		d->m_engine->lobbyCreate();
	else
		d->m_engine->lobbyConnect(room);
}


/**
 * @brief RpgGame::characterSelect
 * @param data
 */

void RpgGame::characterSelect(const QVariantMap &data)
{
	d->characterSelect(data);
}


/**
 * @brief RpgGame::questSelect
 * @param data
 */

void RpgGame::questSelect(const QVariantMap &data)
{
	d->questSelect(data);
}


/**
 * @brief RpgGame::getCharacterImage
 * @param character
 * @return
 */

QUrl RpgGame::getCharacterImage(const QString &character) const
{
	return RpgGame::characters().value(character).image;
}



/**
 * @brief RpgGame::loadTutorial
 * @param character
 * @return
 */

bool RpgGame::loadTutorial(const QString &character)
{
	if (m_client->loadDemoMap(QUrl("tutorial://test_tutorial1")))
		return true;

	return false;
}


/**
 * @brief RpgGame::createEmptyGame
 * @param client
 * @return
 */

RpgGame *RpgGame::createEmptyGame(Client *client)
{
	Q_ASSERT(client);

	std::unique_ptr<RpgGame> game = std::make_unique<RpgGame>(nullptr, client, false, nullptr);

	if (!game->AbstractLevelGame::load()) {
		client->messageError(tr("Nem lehet betölteni az oldalt!"));
		return nullptr;
	}


	connect(game.get(), &AbstractGame::gameDestroyRequest, game.get(), &RpgGame::deleteLater);

	connect(game.get(), &QObject::destroyed, client, []() {
		LOG_CERROR("client") << "######################";
	});

	game->setReadyToDestroy(true);

	return game.release();
}



/**
 * @brief RpgGame::load
 * @param def
 * @return
 */

bool RpgGame::load(const RpgGameDefinition &def)
{
	if (!m_gameItem) {
		LOG_CERROR("game") << "Missing GameItem";
		return false;
	}

	return m_gameItem->load(def);
}


/**
 * @brief RpgGame::isEmpty
 * @return
 */

bool RpgGame::isEmpty() const
{
	return !m_missionLevel;
}

QVariantMap RpgGame::gameResultData() const
{
	return m_gameResultData;
}

void RpgGame::setGameResultData(const QVariantMap &newGameResultData)
{
	if (m_gameResultData == newGameResultData)
		return;
	m_gameResultData = newGameResultData;
	emit gameResultDataChanged();
}

int RpgGame::campaignId() const
{
	return m_campaignId;
}

void RpgGame::setCampaignId(int newCampaignId)
{
	if (m_campaignId == newCampaignId)
		return;
	m_campaignId = newCampaignId;
	emit campaignIdChanged();
}

int RpgGame::gameId() const
{
	return m_gameId;
}

void RpgGame::setGameId(int newGameId)
{
	if (m_gameId == newGameId)
		return;
	m_gameId = newGameId;
	emit gameIdChanged();
}






QSListModel* RpgGame::modelCharacters() const
{
	return m_modelCharacters.get();
}



QColor RpgGame::colorGlow()
{
	return m_colorGlow;
}




/**
 * @brief RpgGame::entryPoint
 * @param entry
 * @return
 */

std::optional<QPointF> RpgGame::entryPoint(const QString &entry) const
{
	const auto it = d->m_entryPoint.find(entry);

	if (it == d->m_entryPoint.cend())
		return std::nullopt;
	else
		return *it;
}




/**
 * @brief RpgGame::modelPlayer
 * @return
 */

QSListModel* RpgGame::modelPlayer() const
{
	return m_modelPlayer.get();
}


/**
 * @brief RpgGame::modelLobby
 * @return
 */

QSListModel* RpgGame::modelLobby() const
{
	return m_modelLobby.get();
}






/**
 * @brief RpgGame::gameMode
 * @return
 */

RpgGame::GameMode RpgGame::gameMode() const
{
	return m_gameMode;
}

void RpgGame::setGameMode(const GameMode &newGameMode)
{
	if (m_gameMode == newGameMode)
		return;
	m_gameMode = newGameMode;
	emit gameModeChanged();
}






/**
 * @brief RpgGame::reloadTerrains
 */

void RpgGame::reloadTerrains()
{
	LOG_CDEBUG("game") << "Reload available RPG terrains...";

	m_terrains.clear();
	RpgGamePrivate::m_terrainHash.clear();

	QDirIterator it(QStringLiteral(":/map"), {QStringLiteral("game.json")}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		const QString &f = it.next();

		QString id = f.section('/',-2,-2);

		if (const auto &ptr = readGameDefinition(id)) {
			m_terrains.insert(id, ptr.value());
			RpgGamePrivate::m_terrainHash.insert(id);
		} else {
			LOG_CWARNING("game") << "Invalid RPG terrain:" << qPrintable(f);
		}
	}

	LOG_CDEBUG("game") << "...loaded " << m_terrains.size() << " terrains";
}





/**
 * @brief RpgGame::readGameDefinition
 * @param map
 * @return
 */

std::optional<RpgGameDefinition> RpgGame::readGameDefinition(const QString &map)
{
	if (map.isEmpty())
		return std::nullopt;

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/map/%1/game.json").arg(map));

	if (!ptr)
		return std::nullopt;

	RpgGameDefinition def;
	def.fromJson(ptr.value());

	def.basePath = QStringLiteral("qrc:/map/").append(map);

	return def;
}





/**
 * @brief RpgGame::reloadWorld
 */

void RpgGame::reloadWorld()
{
	Server *s = Application::instance()->client()->server();

	if (!s) {
		LOG_CTRACE("game") << "Missing server";
		return;
	}

	if (!s->user() || !s->user()->wallet()) {
		LOG_CTRACE("game") << "Missing user or wallet";
		return;
	}

	s->user()->wallet()->loadWorld();
}



/**
 * @brief RpgGame::commonMap
 * @param scene
 * @return
 */

const Tiled::Map* RpgGame::commonMap(const QString &scene) const
{
	const auto it = d->m_commonMaps.find(scene);

	if (it == d->m_commonMaps.cend()) {
		LOG_CERROR("game") << "Invalid common map" << scene;
		return nullptr;
	}

	return it->second.first.get();
}



/**
 * @brief RpgGame::commonRenderer
 * @param scene
 * @return
 */

Tiled::MapRenderer *RpgGame::commonRenderer(const QString &scene) const
{
	const auto it = d->m_commonMaps.find(scene);

	if (it == d->m_commonMaps.cend()) {
		LOG_CERROR("game") << "Invalid common map" << scene;
		return nullptr;
	}

	return it->second.second.get();
}



/**
 * @brief RpgGame::rpgLogic
 * @return
 */

Rpg::RpgLogicClient *RpgGame::rpgLogicClient()
{
	return d->m_logic.get();
}


/**
 * @brief RpgGame::loadNextQuestion
 * @return
 */

bool RpgGame::loadNextQuestion()
{
	return d->nextQuestion();
}



/**
 * @brief RpgGame::msecLeft
 * @return
 */

int RpgGame::msecLeft() const
{
	if (!d->m_questSelectTimer.isForever() && !d->m_questSelectTimer.hasExpired())
		return d->m_questSelectTimer.remainingTime();

	if (!m_gameItem || !m_gameItem->tickTimer())
		return -1;

	return std::max(0ll, (qint64) d->m_deadlineTick - m_gameItem->tickTimer()->currentTick()) * 1000./60.;
}


/**
 * @brief RpgGame::loadPage
 * @return
 */

QQuickItem *RpgGame::loadPage()
{
	QQuickItem *item = m_client->stackPushPage(QStringLiteral("PageRpgGame.qml"),
											   QVariantMap({
															   { QStringLiteral("game"), QVariant::fromValue(this) }
														   })
											   );

	if (!item)
		return nullptr;

	d->contentPrepare();

	return item;

}


/**
 * @brief RpgGame::connectGameQuestion
 */

void RpgGame::connectGameQuestion()
{
	connect(m_gameQuestion, &GameQuestion::success, d, &RpgGamePrivate::onQuestionSuccess);
	connect(m_gameQuestion, &GameQuestion::failed, d, &RpgGamePrivate::onQuestionFailed);
	connect(m_gameQuestion, &GameQuestion::finished, d, &RpgGamePrivate::onQuestionFinished);
	connect(m_gameQuestion, &GameQuestion::started, d, &RpgGamePrivate::onQuestionStarted);
}



/**
 * @brief RpgGame::gameFinishEvent
 * @return
 */

bool RpgGame::gameFinishEvent()
{
	if (m_closedSuccesfully)
		return false;

	m_closedSuccesfully = true;
	return true;
}




/**
 * @brief RpgGame::setError
 * @param str
 */

void RpgGame::setError(const QString &str)
{
	setErrorString(str);
	setGameState(GameStateError);
}

RpgGameItem *RpgGame::gameItem() const
{
	return m_gameItem;
}

void RpgGame::setGameItem(RpgGameItem *newGameItem)
{
	if (m_gameItem == newGameItem)
		return;
	m_gameItem = newGameItem;
	emit gameItemChanged();
}


/**
 * @brief RpgGame::onContentDownloaded
 */

void RpgGamePrivate::onContentDownloaded()
{
	if (q->m_gameState != RpgGame::GameStateDownloadContent && q->m_gameState != RpgGame::GameStateInvalid) {
		LOG_CERROR("client") << "Invalid state" << q->m_gameState;
		q->setError(tr("Ismeretlen hiba: onContentDownloaded"));
		return;
	}


	q->m_client->downloader()->loadDynamicContent();



	connectionPrepare();
}


/**
 * @brief RpgGame::onContentError
 */

void RpgGamePrivate::onContentError()
{
	q->setError(tr("Sikertelen letöltés"));
}






/**
 * @brief RpgGamePrivate::characterSelect
 * @param data
 */

void RpgGamePrivate::characterSelect(const QVariantMap &data)
{
	if (q->m_gameState != RpgGame::GameStateCharacterSelect)
		return;


	if (data.contains(QStringLiteral("character"))) {
		const QString character = data.value(QStringLiteral("character")).toString();

		RpgPlayerDefinition def = RpgGame::characters().value(character);

		if (def.name.isEmpty()) {
			q->m_client->messageWarning(tr("Érvénytelen karakter"));
			return;
		}

		///////////////////////////////////////////
		LOG_CERROR("game") << "<<<<<<<<<<<<<<<<<<<<< REMOVE";
		def.defender.append(RpgStream::BaseDefenderObject::Pulse);
		def.defender.append(RpgStream::BaseDefenderObject::Fog);
		def.defender.append(RpgStream::BaseDefenderObject::Multiplier1);
		def.utility.append(RpgStream::PlayerConfig::UtilityMissionary);
		def.utility.append(RpgStream::PlayerConfig::UtilitySniper);
		//////////////////////////////////


		m_characterSelect.data().setConfig(def.toPlayerConfig());
		m_characterSelect.data().setCharacterResolved(character);
	}

	if (data.contains(QStringLiteral("nickname")))
		m_characterSelect.data().setNickName(data.value(QStringLiteral("nickname")).toString().toUtf8());

	if (data.contains(QStringLiteral("terrain"))) {
		m_characterSelect.gameConfig().setTerrainResolved(data.value(QStringLiteral("terrain")).toString());
	}

	if (data.value(QStringLiteral("ready"), false).toBool()) {
		m_characterSelect.data().flags().setFlag(RpgStream::PlayerData::FlagCompleted);
	}

	//m_characterSelect.data().setTeam(RpgStream::TeamB);

	if (m_engine)
		m_engine->sendCharacterSelect(m_characterSelect);
	else
		QMetaObject::invokeMethod(this, &RpgGamePrivate::updateCharacterSelect, Qt::QueuedConnection);
}




/**
 * @brief RpgGamePrivate::updateCharacterSelect
 */

void RpgGamePrivate::updateCharacterSelect()
{
	if (!m_characterSelect.data().flags().testFlag(RpgStream::PlayerData::FlagCompleted))
		return;

	if (m_characterSelect.gameConfig().terrain() == 0 ||
			m_characterSelect.data().character() == 0) {
		LOG_CWARNING("game") << "Missing character or terrain";
		return;
	}

	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	Q_ASSERT(cfg);

	*cfg = m_characterSelect.gameConfig();

	q->setTerrain(cfg->terrainResolved(m_terrainHash));

	if (q->m_gameMode == RpgGame::SinglePlayer) {
		cfg->flags().setFlag(RpgStream::GameConfig::FlagSelected);

		m_characterSelect.data().setTeam(RpgStream::TeamA);

		quint32 id = 0;
		quint32 tagId = 0;

		m_logic->playerAdd(m_characterSelect.data(), &id, &tagId);


		if (Rpg::RpgLogicClientTutorial *tutorial = dynamic_cast<Rpg::RpgLogicClientTutorial*>(m_logic.get())) {
			LOG_CINFO("game") << "INIT TUTORIAL";

			tutorial->initialize();
		}

		q->setGameState(RpgGame::GameStatePrepare);
	}
}




/**
 * @brief RpgGamePrivate::loadQuests
 * @param list
 */

void RpgGamePrivate::loadQuests(const Rpg::QuestList &list, const quint32 &msecLeft)
{
	m_questSelect.setDefender(RpgStream::BaseDefenderObject::None);
	m_questSelect.setUtility(RpgStream::PlayerConfig::UtilityNone);
	m_questSelect.setQuest(0);

	if (!q->m_questSelectData.isEmpty())
		return;

	questSelectTimerSet(msecLeft);

	if (!q->m_controlledPlayer) {
		LOG_CERROR("game") << "Missing controlled player";
		return;
	}

	const RpgPlayerDefinition &config = q->m_controlledPlayer->config();

	QVariantList d;

	for (const auto &ptr : config.defender) {
		QVariantMap m = RpgChanger::dataDefenders().value(ptr);

		if (!m.isEmpty()) {
			m.insert(QStringLiteral("key"), ptr);
			d.append(m);
		}
	}

	QVariantList u;

	for (const auto &ptr : config.utility) {
		QVariantMap m = RpgChanger::dataUtilities().value(ptr);

		if (!m.isEmpty()) {
			m.insert(QStringLiteral("key"), ptr);
			u.append(m);
		}
	}

	QVariantList l;

	for (const RpgStream::Quest &quest : list) {
		QVariantMap m;

		m[QStringLiteral("question")] = quest.question();
		m[QStringLiteral("streak")] = quest.streak();
		m[QStringLiteral("pts")] = quest.pts();

		m[QStringLiteral("xp")] = quest.xp();
		m[QStringLiteral("token")] = quest.token();

		l.append(m);
	}

	QVariantMap data;

	data[QStringLiteral("defenders")] = d;
	data[QStringLiteral("utilities")] = u;
	data[QStringLiteral("quests")] = l;

	q->setQuestSelectData(data);
}





/**
 * @brief RpgGamePrivate::questSelect
 * @param data
 */

void RpgGamePrivate::questSelect(const QVariantMap &data)
{
	if (!q->m_controlledPlayer) {
		LOG_CERROR("game") << "Missing controlled player";
		return;
	}

	const RpgPlayerDefinition &config = q->m_controlledPlayer->config();

	int idxD = data.value(QStringLiteral("defender"), -1).toInt();
	int idxU = data.value(QStringLiteral("utility"), -1).toInt();
	int idxC = data.value(QStringLiteral("quest"), -1).toInt();

	if (idxD >= 0 && idxD < config.defender.size())
		m_questSelect.setDefender(config.defender.at(idxD));

	if (idxU >= 0 && idxU < config.utility.size())
		m_questSelect.setUtility(config.utility.at(idxU));

	if (idxC >= 0)
		m_questSelect.setQuest(idxC);

	if (data.value(QStringLiteral("ready")).toBool()) {
		m_questSelect.setTagId(RpgLogicObjectMapper::getId(q->m_controlledPlayer->objectId()));

		if (m_engine)
			m_engine->sendQuestSelect(m_questSelect);
		else {
			questSelectTimerSet(0);

			Rpg::RpgLogicClientSingle *logic = dynamic_cast<Rpg::RpgLogicClientSingle*>(m_logic.get());

			if (!logic) {
				LOG_CERROR("game") << "Invalid logic";
				q->setError(tr("Belső hiba"));
				return;
			}

			logic->selectQuest(m_questSelect);

			syncGameConfig(logic->startGame(), logic->serverTick());
		}
	}
}


/**
 * @brief RpgGamePrivate::questSelectTimerSet
 * @param msec
 */

void RpgGamePrivate::questSelectTimerSet(const qint64 msec)
{
	if (msec <= 0) {
		if (GameQuestion *gq = q->gameQuestion())
			gq->setProperty("msecLeft", 0);

		m_questBasicTimer.stop();
		m_questSelectTimer.setRemainingTime(-1);
	} else {
		m_questSelectTimer.setRemainingTime(msec);

		static const QColor iconColor = QColorConstants::Svg::cyan;
		if (GameQuestion *gq = q->gameQuestion()) {
			gq->setProperty("progressColor", iconColor);
			gq->setProperty("msecLeft", 1 /*q->msecLeft() - msecLeft*/);
		}

		if (!m_questBasicTimer.isActive())
			m_questBasicTimer.start(100, q);
	}
}




/**
 * @brief RpgGamePrivate::waitForGameId
 */

void RpgGamePrivate::waitForGameId()
{
	if (!q->m_missionLevel) {
		LOG_CINFO("game") << "RpgGame without MissionLevel";
		onGameIdReady();
		return;
	}

	q->m_client->send(HttpConnection::ApiUser, QStringLiteral("campaign/%1/game/create").arg(
						  q->m_campaignId > 0 ? q->m_campaignId : 0
												), {
						  { QStringLiteral("map"), q->m_missionLevel->map()->uuid() },
						  { QStringLiteral("mission"), q->uuid() },
						  { QStringLiteral("level"), q->level() },
						  { QStringLiteral("mode"), q->mode() },
						  { QStringLiteral("terrain"), m_characterSelect.gameConfig().terrainResolved(m_terrainHash) },
						  { QStringLiteral("character"), m_characterSelect.data().characterResolved(m_characterHash) },
					  })
			->error(this, [this](const QNetworkReply::NetworkError &){
		q->setError(tr("Játék indítása sikertelen"));
	})
			->fail(this, [this](const QString &err){
		q->setError(err);
	})
			->done(this, [this](const QJsonObject &data){
		const int &gameId = data.value(QStringLiteral("id")).toInt(-1);
		if (gameId < 0) {
			q->setError(tr("Érvénytelen játékazonosító érkezett"));
			return;
		}

		LOG_CDEBUG("client") << "Game play (campaign)" << gameId;

		q->setGameId(gameId);

		QMetaObject::invokeMethod(this, &RpgGamePrivate::onGameIdReady);
	});
}


/**
 * @brief RpgGamePrivate::onGameIdReady
 */


void RpgGamePrivate::onGameIdReady()
{
	LOG_CDEBUG("game") << "GAME ID READ" << q->m_gameId;

	q->m_gameItem->setIsContentReady(true);
}



/**
 * @brief RpgGamePrivate::prepareGameItem
 */

void RpgGamePrivate::prepareGameItem()
{
	Q_ASSERT(!RpgGame::terrains().isEmpty());

	if (m_engine)
		m_engine->m_gameFlags.setFlag(RpgStream::PlayerData::FlagLoadStarted);

	if (!initializeQuestions()) {
		q->setError(tr("Nem sikerült betölteni a kérdéseket"));
		return;
	}

	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	Q_ASSERT(cfg);

	const QString &terrain = cfg->terrainResolved(m_terrainHash);

	const RpgGameDefinition def = RpgGame::terrains().value(terrain);

	auto ptr = TiledGame::getDynamicTilesets(def);

	if (!ptr) {
		q->setError(tr("Nem sikerült betölteni a terepet"));
		return;
	}

	const auto &common = TiledGame::getDynamicTilesets(m_commonGameDefinition);

	if (!common) {
		q->setError(tr("Nem sikerült betölteni a terepet"));
		return;
	}

	ptr->append(common.value());

	for (const QString &s : ptr.value()) {
		const QString res = q->m_client->downloader()->contentDict().value(s);

		if (res.isEmpty()) {
			q->setError(tr("Nem sikerült betölteni a terepet"));
			return;
		}

		if (!q->m_client->downloader()->loadDynamicContent(res)) {
			q->setError(tr("Nem sikerült betölteni a terepet"));
			return;
		}
	}

	const QStringList npcList = def.getDynamicContent();

	for (const QString &s : npcList) {
		if (!q->m_client->downloader()->loadDynamicContent(s+QStringLiteral(".dres"))) {
			q->setError(tr("Nem sikerült betölteni a terepet"));
			return;
		}

		if (!q->readNpcDefinition(s)) {
			q->setError(tr("Nem sikerült betölteni a terepet"));
			return;
		}
	}


	if (!loadCommonMaps()) {
		q->setError(tr("Nem sikerült betölteni a terepet"));
		return;
	}

	if (q->load(def)) {
		m_gameDefinition = def;
		waitForGameId();
	} else {
		q->setError(tr("Nem sikerült betölteni a terepet"));
		return;
	}
}





/**
 * @brief RpgGamePrivate::onGamePrepared
 */

void RpgGamePrivate::onGameItemPrepared()
{
	LOG_CINFO("game") << "********************************************************";

	connectJoysticks();

	loadChunkGrid();
	loadHeat();


	// Set tower points

	for (RpgTower *tower : m_towerList) {
		if (!tower->scatterPoint().isValid())
			continue;

		QPointF p = tower->bodyPositionF();
		p.setY(tower->scene()->height() - p.y());
		tower->scatterPoint().scatter->replace(tower->scatterPoint().index, p);
	}


	// Load game

	if (q->m_gameMode == RpgGame::MultiPlayer) {
		m_engine->m_isMapReady = true;
		m_engine->m_gameFlags.setFlag(RpgStream::PlayerData::FlagLoadCompleted);

		return;
	}


	// Single Player

	Rpg::RpgLogicClientSingle *logic = dynamic_cast<Rpg::RpgLogicClientSingle*>(m_logic.get());

	if (!logic) {
		LOG_CERROR("game") << "Invalid logic";
		q->setError(tr("Belső hiba"));
		return;
	}

	logic->overrideMapData(m_mapData);

	logic->loadMapData(m_mapData);
	m_isMapLoaded = true;

	RpgStream::GameConfig cfg = logic->start();

	syncObjects();
	syncGameState();
	syncGameConfig(cfg, 0);

	loadChunkGrid();
	logic->reloadMapData(m_mapData);

	Rpg::RpgLogicScope scope = m_logic->getScope();

	loadQuests(*scope.getCtx<Rpg::QuestList>(), CFG_GAME_STAGE_SELECT);
}



/**
 * @brief RpgGamePrivate::loadChunkGrid
 */

void RpgGamePrivate::loadChunkGrid()
{
	static const float size = 64.;

	TiledGame::TcodMapData *map = q->gameItem()->reloadTcodMap(q->gameItem()->currentScene(),
															   RpgGameItem::FixtureGround | RpgGameItem::FixtureExcluded,
															   size);

	Q_ASSERT(map);

	const int width = map->map->getWidth();
	const int height = map->map->getHeight();

	Rpg::ChunkGrid grid;

	grid.viewport = map->viewport;
	grid.chunkSize.setWidth(map->chunkWidth);
	grid.chunkSize.setHeight(map->chunkHeight);


	for (int i=0; i<height; ++i) {
		for (int j=0; j<width; ++j) {
			if (map->map->isWalkable(j, i))
				continue;

			grid.excludeSet.insert(QPair<quint32, quint32>(j, i));
		}
	}

	m_mapData.setChunkGrid(grid.toRpgStream());

	q->gameItem()->reloadTcodMap(q->gameItem()->currentScene());			// Reset!

}



/**
 * @brief RpgGamePrivate::loadHeat
 */

void RpgGamePrivate::loadHeat()
{
	std::vector<RpgStream::Heat> heat;

	heat.reserve(m_gameDefinition.heat.size());

	for (const RpgHeat &h : m_gameDefinition.heat) {
		RpgStream::Heat item;

		item.npc().reserve(h.npc.size());

		for (const RpgHeatNpc &npc : h.npc) {
			RpgStream::HeatNpc n;

			const auto &ptr = q->readNpcDefinition(npc.type);

			if (!ptr) {
				LOG_CERROR("game") << "Invalid NPC type" << npc.type;
				continue;
			}

			n.setData(ptr->toNpcData());

			n.data().setCharacterResolved(npc.type);

			n.setNum(npc.num);
			n.setDelay(AbstractGame::TickTimer::msecToTick(npc.delay));
			n.positionList().reserve(npc.entry.size());

			for (const QString &entry : npc.entry) {
				const QPointF pos = m_entryPoint.value(entry);
				if (pos.isNull()) {
					LOG_CERROR("game") << "Invalid entry point" << entry;
					continue;
				}

				RpgStream::PlayerPosition p;
				p.setPosXAsFloat(pos.x());
				p.setPosYAsFloat(pos.y());

				n.positionList().emplace_back(std::move(p));
			}


			item.npc().emplace_back(std::move(n));
		}

		heat.emplace_back(std::move(item));
	}

	LOG_CDEBUG("game") << "Loaded" << heat.size() << "heat";

	m_mapData.setHeat(heat);
}



/**
 * @brief RpgGamePrivate::playerPositionAdd
 * @param pos
 * @param team
 */

void RpgGamePrivate::playerPositionAdd(const QPointF &pos, const RpgStream::Team &team)
{
	RpgStream::PlayerPosition p;
	p.setPosXAsFloat(pos.x());
	p.setPosYAsFloat(pos.y());
	p.setTeam(team);

	m_mapData.playerPositionList().emplace_back(std::move(p));
}


/**
 * @brief RpgGamePrivate::mpEmitterAdd
 * @param pos
 */

void RpgGamePrivate::mpEmitterAdd(const QPointF &pos, const quint32 &tagId, QQuickItem *visualItem,
								  const QList<TiledObjectBody *> &excludeList)
{
	RpgStream::MpEmitter p;
	p.setTagId(tagId);
	p.setPosXAsFloat(pos.x());
	p.setPosYAsFloat(pos.y());

	m_mapData.mpEmitterList().emplace_back(std::move(p));

	if (visualItem)
		m_emitters[tagId] = qMakePair(visualItem, excludeList);
}



/**
 * @brief RpgGamePrivate::towerAdd
 * @param tower
 */

void RpgGamePrivate::towerAdd(RpgTower *tower)
{
	Q_ASSERT(tower);

	const quint32 id = RpgLogicObjectMapper::getId(tower->objectId());

	RpgStream::Tower t;
	t.setTagId(id);
	t.setPosXAsFloat(tower->bodyPosition().x);
	t.setPosYAsFloat(tower->bodyPosition().y);

	for (RpgDefenderPoint *p : tower->defenderPoints()) {
		Q_ASSERT(p);

		const QPointF pos = p->bodyPositionF();

		RpgStream::Defender d;
		d.setTagId(RpgLogicObjectMapper::getId(p->objectId()));
		d.setPosXAsFloat(pos.x());
		d.setPosYAsFloat(pos.y());

		t.defenders().emplace_back(std::move(d));
	}

	m_mapData.towerList().emplace_back(std::move(t));

	m_towerList[id] = tower;
}



/**
 * @brief RpgGamePrivate::chestPositionAdd
 * @param pos
 */

void RpgGamePrivate::chestPositionAdd(const QPointF &pos)
{
	RpgStream::PlayerPosition p;
	p.setPosXAsFloat(pos.x());
	p.setPosYAsFloat(pos.y());

	m_mapData.chestPositionList().emplace_back(std::move(p));
}




/**
 * @brief RpgGamePrivate::entryPointAdd
 * @param name
 * @param pos
 */

void RpgGamePrivate::entryPointAdd(const QString &name, const QPointF &pos)
{
	m_entryPoint[name] = pos;
}




/**
 * @brief RpgGamePrivate::addLocationSound
 * @param object
 * @param sound
 * @param baseVolume
 * @param channel
 */

void RpgGamePrivate::addLocationSound(TiledObjectBody *object, const QString &sound, const qreal &baseVolume, const Sound::ChannelType &channel)
{
	LOG_CTRACE("game") << "Add location sound" << sound << baseVolume << object << channel;
	m_sfxLocations.emplace_back(new TiledGameSfxLocation(sound, baseVolume, object, channel));
}







/**
 * @brief RpgGamePrivate::connectJoysticks
 */

void RpgGamePrivate::connectJoysticks()
{
	if (!q->m_gameItem)
		return;


	const QList<QPair<QQuickItem*, const char*> > list = {
		{ q->m_gameItem->joystickA(), "joystickClickedA(bool)" },
		{ q->m_gameItem->joystickB(), "joystickClickedB(bool)" },
		{ q->m_gameItem->joystickC(), "joystickClickedC(bool)" },
		{ q->m_gameItem->joystickD(), "joystickClickedD(bool)" },
	};


	const QMetaObject *moThis = this->metaObject();


	for (const auto &p : list) {
		if (!p.first)
			continue;

		const QMetaObject *mo = p.first->metaObject();

		const int methodIndex = mo->indexOfMethod("released(bool)");

		if (methodIndex < 0)
			continue;

		connect(p.first, mo->method(methodIndex), this, moThis->method(moThis->indexOfMethod(p.second)));
	}

}



/**
 * @brief RpgGamePrivate::setJoystickState
 * @param player
 * @param joystick
 * @param state
 */

void RpgGamePrivate::setJoystickState(RpgPlayer *player, const TiledGame::Joystick &joystick, const TiledGame::JoystickState &state)
{
	if (!player)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(player->currentMotor());

	if (!motor)
		return;

	if (!player->isAlive()) {
		motor->setCurrentJoystickState(TiledGame::JoystickState{});
		return;
	}

	bool gamepad = setFromGamepad(motor);

	if (joystick == TiledGame::JoystickA && (!gamepad || state.hasKeyboard || state.hasTouch))
		motor->setCurrentJoystickState(state);
	/*else if (joystick == TiledGame::JoystickB && (!gamepad || state.hasKeyboard || state.hasTouch))
		motor->setControlJoystickState(state);
	else if (joystick == TiledGame::JoystickC && (!gamepad || state.hasKeyboard || state.hasTouch))
		motor->setTargetJoystickState(state);*/
	else if (joystick == TiledGame::JoystickB && (!gamepad || state.hasKeyboard || state.hasTouch))
		motor->setTargetJoystickState(state);

}



/**
 * @brief RpgGamePrivate::setFromGamepad
 * @param motor
 * @return
 */

bool RpgGamePrivate::setFromGamepad(RpgMotorPlayerControlled *motor)
{
#ifdef WITH_GAMEPAD
	if (m_gamePad) {
		TiledGame::JoystickState stateA;
		TiledGame::JoystickState stateB;
		TiledGame::JoystickState stateC;

		if (std::abs(m_gamePad->axisLeftX()) > 0.1 ||
				std::abs(m_gamePad->axisLeftY()) > 0.1 ||
				std::abs(m_gamePad->axisRightX()) > 0.1 ||
				std::abs(m_gamePad->axisRightY()) > 0.1) {
			q->m_gameItem->setUsingGamepad(true);
		}


		if (m_gamePad->buttonL3() || m_gamePad->buttonL1()) {
			stateB.hasTouch = true;
			stateB.dx = m_gamePad->axisLeftX();
			stateB.dy = m_gamePad->axisLeftY();
			stateB.angle = atan2(stateB.dy, stateB.dx);
			stateB.distance = (stateB.dx*stateB.dx + stateB.dy*stateB.dy)*1.1;
		} else {
			stateA.dx = m_gamePad->axisLeftX();
			stateA.dy = m_gamePad->axisLeftY();
			stateA.angle = atan2(stateA.dy, stateA.dx);
			stateA.distance = (stateA.dx*stateA.dx + stateA.dy*stateA.dy)*1.1;
		}

		motor->setCurrentJoystickState(stateA);
		//motor->setControlJoystickState(stateB);



		if (m_gamePad->axisRightX() > 0.1 || m_gamePad->axisRightY() > 0.1) {
			stateC.hasTouch = true;
			stateC.dx = m_gamePad->axisRightX();
			stateC.dy = m_gamePad->axisRightY();
			stateC.angle = atan2(stateC.dy, stateC.dx);
			stateC.distance = (stateC.dx*stateC.dx + stateC.dy*stateC.dy)*1.1;
		}

		motor->setTargetJoystickState(stateC);

		return true;
	}
#endif

	return false;
}



/**
 * @brief RpgGamePrivate::joystickClicked
 * @param joystick
 */

void RpgGamePrivate::joystickClickedA(const bool &clicked)
{
	if (!q->m_controlledPlayer || !clicked)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(q->m_controlledPlayer->currentMotor());

	if (!motor)
		return;

	motor->useCurrentControl();
}



/**
 * @brief RpgGamePrivate::joystickClickedB
 * @param clicked
 */

void RpgGamePrivate::joystickClickedB(const bool &clicked)
{
	if (!q->m_controlledPlayer)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(q->m_controlledPlayer->currentMotor());

	if (!motor)
		return;

	if (q->m_controlledPlayer->joystickMode() == RpgPlayer::JoystickModeControl)
		motor->putDefender(clicked);
	else if (q->m_controlledPlayer->joystickMode() == RpgPlayer::JoystickModeTarget)
		motor->attackCurrentTarget();
	else if (q->m_controlledPlayer->joystickMode() == RpgPlayer::JoystickModeUtility)
		motor->useCurrentUtility();
}


/**
 * @brief RpgGamePrivate::joystickClickedC
 * @param clicked
 */

void RpgGamePrivate::joystickClickedC(const bool &/*clicked*/)
{
	/*if (!q->m_controlledPlayer)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(q->m_controlledPlayer->currentMotor());

	if (!motor)
		return;

	motor->attackCurrentTarget();*/
}


/**
 * @brief RpgGamePrivate::joystickClickedD
 * @param clicked
 */

void RpgGamePrivate::joystickClickedD(const bool &clicked)
{
	Q_UNUSED(clicked);
}




/**
 * @brief RpgGamePrivate::reloadQuestions
 */

void RpgGamePrivate::reloadQuestions()
{
	if (!m_logic)
		return;

	m_questionList = q->createQuestions();

	std::shuffle(m_questionList.begin(), m_questionList.end(), m_logic->rnd());

	m_questionIterator = m_questionList.constBegin();
}




/**
 * @brief RpgGamePrivate::initializeQuestions
 */

bool RpgGamePrivate::initializeQuestions()
{
	if (m_questionInitialized) {
		LOG_CWARNING("game") << "RpgQuestion already initialized";
		return false;
	}

	LOG_CTRACE("game") << "Initialize questions";

	reloadQuestions();

	if (m_questionList.isEmpty())
		return false;


	m_questionDuration = 0;

	for (const Question &q : m_questionList) {
		ModuleInterface *iface = Application::instance()->objectiveModules().value(q.module());

		if (!iface)
			continue;

		m_questionDuration += SECOND_PER_QUESTION;

		qreal factor = (iface->xpFactor()-1.0) * 2.0;

		// Exponenciálisan növeljük

		m_questionDuration += factor * SECOND_PER_QUESTION;
	}

	m_questionInitialized = true;

	return true;
}




/**
 * @brief RpgGamePrivate::nextQuestion
 */

bool RpgGamePrivate::nextQuestion()
{
	GameQuestion *gq = q->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return false;
	}

	if (gq->questionComponent()) {
		LOG_CERROR("game") << "GameQuestionComponent already loaded";
		return false;
	}

	if (m_questionIterator == m_questionList.constEnd()) {
		LOG_CDEBUG("game") << "Reload questions";
		reloadQuestions();
	}

	if (m_questionIterator == m_questionList.constEnd()) {
		LOG_CERROR("game") << "Reload questions error";
		return false;
	}

	gq->loadQuestion(*m_questionIterator);
	++m_questionIterator;

	return true;
}



/**
 * @brief RpgGamePrivate::onQuestionSuccess
 * @param answer
 */

void RpgGamePrivate::onQuestionSuccess(const QVariantMap &answer)
{
	// ++winnerstreak

	q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/sfx/correct.mp3"), Sound::SfxChannel);
	q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/winner.mp3"), Sound::VoiceoverChannel);

	GameQuestion *gq = q->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return;
	}

	q->addStatistics(gq->module(), gq->objectiveUuid(), true, gq->elapsedMsec());

	//int xp = gq->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * 10.;


	gq->answerReveal(answer);
	gq->setMsecBeforeHide(0);

	if (!q->controlledPlayer()) {
		LOG_CERROR("game") << "Missing RpgPlayer";
		gq->finish();
		return;
	}

	if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(q->controlledPlayer()->currentMotor())) {
		motor->questionFinished(true);
	} else {
		LOG_CERROR("game") << "Missing RpgMotorPlayerControlled";
		gq->finish();
		return;
	}
}


/**
 * @brief RpgGamePrivate::onQuestionFailed
 * @param answer
 */

void RpgGamePrivate::onQuestionFailed(const QVariantMap &answer)
{
	vibrate();

	//setWinnerStreak(0);

	q->setIsFlawless(false);

	q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/loser.mp3"), Sound::VoiceoverChannel);

	GameQuestion *gq = q->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return;
	}

	q->addStatistics(gq->module(), gq->objectiveUuid(), false, gq->elapsedMsec());


	gq->answerReveal(answer);
	gq->setMsecBeforeHide(1250);


	if (!q->controlledPlayer()) {
		LOG_CERROR("game") << "Missing RpgPlayer";
		gq->finish();
		return;
	}

	if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(q->controlledPlayer()->currentMotor())) {
		motor->questionFinished(false);
	} else {
		LOG_CERROR("game") << "Missing RpgMotorPlayerControlled";
		gq->finish();
		return;
	}

}




/**
 * @brief RpgGamePrivate::onQuestionStarted
 */

void RpgGamePrivate::onQuestionStarted()
{
	///q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/fight.mp3"), Sound::VoiceoverChannel);
}


/**
 * @brief RpgGamePrivate::onQuestionFinished
 */

void RpgGamePrivate::onQuestionFinished()
{
	q->m_gameItem->forceActiveFocus(Qt::OtherFocusReason);

	GameQuestion *gq = q->gameQuestion();

	if (!gq) {
		LOG_CERROR("game") << "Missing GameQuestion";
		return;
	}
}



/**
 * @brief RpgGamePrivate::startGame
 */

void RpgGamePrivate::startGame(const quint32 &tick)
{
	q->setGameState(RpgGame::GameStatePlay);
	q->m_gameItem->tickTimer()->start(q, tick);
	emit q->questSelectCompleted();
	questSelectTimerSet(0);
	q->gameStart();
}




/**
 * @brief RpgGamePrivate::onBeforeWorldStep
 */

void RpgGamePrivate::onBeforeWorldStep(const qint64 &tick)
{
	syncGameState();
	syncObjects();

	if (q->m_gameMode == RpgGame::SinglePlayer) {
		if (tick >= 0)
			m_logic->render();
	} else {
		if (m_engine)
			m_engine->onBeforeWorldStep(tick);

		m_logic->renderUpdate();				// delete tags
	}

	QString txt;

	RpgStream::FullState full = m_logic->getFullState(1, &txt);

	if (full.flags().testFlag(RpgStream::FullState::Event))
		processEvents(full.events(), tick);

	deleteMissingObjects(extractObjects(full));

#ifdef WITH_FTXUI
	if (DesktopApplication *app = dynamic_cast<DesktopApplication*>(Application::instance())) {
		QCborMap m;
		m.insert(QStringLiteral("mode"), QStringLiteral("SND"));
		m.insert(QStringLiteral("txt"), txt);
		app->writeToSocket(m);
	}
#endif

}




/**
 * @brief RpgGamePrivate::onAfterWorldStep
 * @param full
 */

void RpgGamePrivate::onAfterWorldStep(const RpgStream::FullState &full)
{
	if (q->m_gameMode == RpgGame::SinglePlayer) {
		m_logic->fullStateLoad(full, {});
	} else if (m_engine) {
		m_engine->sendState(full);
	}
}



/**
 * @brief RpgGamePrivate::finishGame
 */

void RpgGamePrivate::finishGame(const bool &abort)
{
	if (abort) {
		LOG_CERROR("game") << "***ABORTED***";

		if (Rpg::RpgLogicClientSingle *logic = dynamic_cast<Rpg::RpgLogicClientSingle*>(m_logic.get())) {
			RpgStream::Result r = logic->getResult();
			for (const RpgStream::PlayerResult &p : r.players()) {
				if (p.playerId() == RpgLogicObjectMapper::getId(q->m_controlledPlayer)) {
					q->setQuestResultData(getQuestResult(r, p));
					break;
				}
			}
		}

		q->setGameState(RpgGame::GameStateAbort);
		q->m_gameItem->tickTimer()->stop();
		q->setFinishState(AbstractGame::Fail);

		q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
		q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);

		q->gameFinish();

		return;
	}

	if (!q->m_questResultData.isEmpty())
		return;

	if (!q->m_controlledPlayer) {
		q->setError(tr("Missing player"));
		return;
	}

	Rpg::RpgLogicScope scope = m_logic->getScope();

	if (scope.getCtx<RpgStream::GameConfig>()->stage() != RpgStream::GameConfig::StageFinished) {
		return;
	}

	if (q->gameState() != RpgGame::GameStatePlay && q->gameState() != RpgGame::GameStateFinished) {
		LOG_CERROR("game") << "Invalid state" << q->gameState();
		return;
	}

	LOG_CINFO("game") << "Finish game at" << q->m_gameItem->tickTimer()->currentTick();

	q->setGameState(RpgGame::GameStateFinished);
	q->m_gameItem->tickTimer()->stop();


	RpgStream::Result *r = scope.getCtx<RpgStream::Result>();

	if (!r)
		return;



	const RpgStream::PlayerResult *pr = nullptr;

	for (const RpgStream::PlayerResult &p : r->players()) {
		if (p.playerId() == RpgLogicObjectMapper::getId(q->m_controlledPlayer)) {
			pr = &p;
			break;
		}
	}


	if (!pr) {
		LOG_CERROR("game") << "Player not found";
		q->setError(tr("Belső hiba"));
		return;
	}


	q->setQuestResultData(getQuestResult(*r, *pr));


	q->setFinishState(pr->success() ? AbstractGame::Success : AbstractGame::Fail);

	q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);

	if (pr->success())
		q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_win.mp3"), Sound::VoiceoverChannel);
	else
		q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);

	q->gameFinish();
}



/**
 * @brief RpgGamePrivate::getQuestResult
 * @param result
 * @param player
 * @return
 */

QVariantMap RpgGamePrivate::getQuestResult(const RpgStream::Result &result, const RpgStream::PlayerResult &player) const
{
	QVariantMap m;

	m[QStringLiteral("team")] = result.team();
	m[QStringLiteral("success")] = player.success();

	m[QStringLiteral("questionRq")] = player.quest().question();
	m[QStringLiteral("streakRq")] = player.quest().streak();
	m[QStringLiteral("ptsRq")] = player.quest().pts();

	m[QStringLiteral("question")] = player.result().question();
	m[QStringLiteral("streak")] = player.result().streak();
	m[QStringLiteral("pts")] = player.result().pts();


	m[QStringLiteral("xp")] = player.quest().xp();
	m[QStringLiteral("token")] = player.quest().token();

	m[QStringLiteral("xpReal")] = player.result().xp();
	m[QStringLiteral("tokenReal")] = player.result().token();

	return m;
}




/**
 * @brief RpgGamePrivate::syncGameConfig
 */

void RpgGamePrivate::syncGameConfig(const RpgStream::GameConfig &config, const quint32 &tick)
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	Q_ASSERT(cfg);

	cfg->setDuration(config.duration());

	if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagPlaying) &&
			config.flags().testFlag(RpgStream::GameConfig::FlagPlaying)) {
		q->m_gameItem->message(QObject::tr("START"));

		cfg->flags().setFlag(RpgStream::GameConfig::FlagPlaying);
		q->setGameState(RpgGame::GameStateInit);
	}

	if (config.flags().testFlag(RpgStream::GameConfig::FlagFinished)) {
		//q->m_gameItem->message(QObject::tr("FINISHED"));
		LOG_CINFO("game") << "*******FINISHED";

		cfg->flags().setFlag(RpgStream::GameConfig::FlagFinished);
	}

	if (config.stage() > cfg->stage()) {
		LOG_CINFO("game") << "New stage:" << cfg->stage() << "->" << config.stage() << tick;

		q->m_gameItem->onStageChanged(config.stage());

		if (cfg->stage() < RpgStream::GameConfig::StageWarmingUp && config.stage() >= RpgStream::GameConfig::StageWarmingUp) {
			const quint32 serverTick = m_logic->estimatedServerTick();

			startGame(serverTick);
			q->m_gameItem->overrideCurrentFrame(serverTick);
			m_deadlineTick = cfg->duration();
		}

		if (config.stage() == RpgStream::GameConfig::StageFinished) {
			LOG_CINFO("game") << "STOP******:" << cfg->stage() << "->" << config.stage();
			q->m_gameItem->message(QObject::tr("FINISHED****"));

			cfg->setStage(config.stage());
			finishGame(false);
		}

		cfg->setStage(config.stage());
	}
}



/**
 * @brief RpgGamePrivate::syncGameState
 */

void RpgGamePrivate::syncGameState()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgStream::GameState *state = scope.getCtx<RpgStream::GameState>();

	if (!state) {
		LOG_CTRACE("game") << "Missing GameState";
		return;
	}

	if (!q->m_controlledPlayer) {
		LOG_CTRACE("game") << "Missing controlled player";
		return;
	}

	q->setPtsTeam(q->m_controlledPlayer->team() == RpgStream::TeamA ?
					  state->ptsA() : state->ptsB());

	q->setPtsOpponent(q->m_controlledPlayer->team() == RpgStream::TeamA ?
						  state->ptsB() : state->ptsA());

	q->setHeat(state->heat());
}



/**
 * @brief RpgGamePrivate::syncTowersAndEmitters
 */

void RpgGamePrivate::syncTowersAndEmitters()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	for (auto entity : scope.view<Rpg::Tower>()) {
		const Rpg::Tower &t = scope.get<Rpg::Tower>(entity);

		if (RpgTower *tower = m_towerList.value(t.idTag)) {
			tower->setVisible(t.active);
		}
	}


	for (auto entity : scope.view<Rpg::MpEmitter>()) {
		const Rpg::MpEmitter &t = scope.get<Rpg::MpEmitter>(entity);

		const auto &v = m_emitters.value(t.idTag);

		if (v.first && v.first->isVisible() != t.active) {
			v.first->setVisible(t.active);

			for (TiledObjectBody *b : v.second) {
				b->filterSet(t.active ? RpgGameItem::FixtureExcluded : RpgGameItem::FixtureInvalid,
							 t.active ? RpgGameItem::FixtureAll : RpgGameItem::FixtureInvalid);
			}
		}
	}
}




/**
 * @brief RpgGamePrivate::syncObjects
 */

void RpgGamePrivate::syncObjects()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	Q_ASSERT(cfg);

	if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagDataCompleted)) {
		LOG_CTRACE("game") << "Incompleted data";
		return;
	}


	syncTowersAndEmitters();
	syncPlayers();
	syncMp();
	syncDefenders();
	syncNpc();
	syncControls();

	if (m_isMapLoaded)
		m_isMapSynchronized = true;
}



/**
 * @brief RpgGamePrivate::syncPlayers
 */

void RpgGamePrivate::syncPlayers()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	Rpg::RpgLogicControlledObjects *controlledObjects = scope.getCtx<Rpg::RpgLogicControlledObjects>();

	bool recolor = false;

	for (auto entity : scope.view<Rpg::Player>()) {
		const Rpg::Player &p = scope.get<Rpg::Player>(entity);

		if ((controlledObjects && controlledObjects->player == p.idTag()) || q->m_gameMode == RpgGame::SinglePlayer) {
			q->setQuestPtsRq(p.playerData.quest().pts());
			q->setQuestQuestionRq(p.playerData.quest().question());
			q->setQuestStreakRq(p.playerData.quest().streak());
		}

		if (scope.try_get<Rpg::LocalIdTag>(entity))
			continue;

		const RpgStream::PlayerState *state = scope.getCurrentState<RpgStream::PlayerState>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);

		LOG_CWARNING("game") << "CREATE PLAYER" << p.playerData.playerId() << p.playerData.character()
							 << p.playerData.characterResolved(m_characterHash)
							 << p.playerData.config().defenders().size();

		RpgPlayerDefinition def = RpgGame::characters().value(p.playerData.characterResolved(m_characterHash));

		if (def.name.isEmpty()) {
			LOG_CERROR("game") << "Invalid player" << p.playerData.character();
			continue;
		}

		def.loadPlayerConfig(p.playerData.config());

		cpVect pos;

		if (state) {
			pos.x = state->entityState().posXAsFloat();
			pos.y = state->entityState().posYAsFloat();
		}

		RpgPlayer *obj = q->m_gameItem->createObject<RpgPlayer>(RpgLogicObjectMapper::toObjectId(p.idTag()), scene,
																q->m_gameItem, pos);

		Q_ASSERT(obj);

		if (p.playerData.nickName().simplified().isEmpty())
			obj->setDisplayName(QString::fromUtf8(p.playerData.userName()));
		else
			obj->setDisplayName(QString::fromUtf8(p.playerData.nickName()));

		obj->load(def);


		logicRegisterObject(obj);
		m_logic->addLocalIdTag(entity);


		if ((controlledObjects && controlledObjects->player == p.idTag()) || q->m_gameMode == RpgGame::SinglePlayer) {
			obj->setSecondaryMotor(std::make_unique<RpgMotorPlayerControlled>(obj));

			q->setControlledPlayer(obj);

			recolor = true;
		}


		if (state) {
			obj->setHp(state->hp());
			obj->setMp(state->mp());
			obj->setBullet(state->bullet());
			obj->setDefender(state->defender(), state->hasDefender());
			obj->setUtility(state->utility(), state->hasUtility());
			obj->setTeam(p.team);

			if (auto ptr = addToScatter(0)) {
				obj->setScatterPoint(ptr.value());
				ptr->scatter->setPointConfiguration(ptr->index, QXYSeries::PointConfiguration::Color,
													q->getColor(p.team));
			}
		}

	}


	if (!recolor)
		return;


	for (auto entity : scope.view<Rpg::Player>()) {
		const Rpg::Player &p = scope.get<Rpg::Player>(entity);

		RpgPlayer *obj = qobject_cast<RpgPlayer*>(scope.getCtx<RpgLogicObjectMapper>()->get(p.idTag()));

		if (!obj)
			continue;

		const auto &ptr = obj->scatterPoint();

		if (!ptr.isValid())
			continue;

		ptr.scatter->setPointConfiguration(ptr.index, QXYSeries::PointConfiguration::Color,
										   q->getColor(obj->team()));
	}
}




/**
 * @brief RpgGamePrivate::syncMp
 */

void RpgGamePrivate::syncMp()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	auto view = scope.view<Rpg::Mp>(entt::exclude<Rpg::LocalIdTag>);

	for (auto entity : view) {
		const Rpg::Mp &mp = scope.get<Rpg::Mp>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);

		RpgMp *obj = q->m_gameItem->createObject<RpgMp>(RpgLogicObjectMapper::toObjectId(mp.idTag), scene,
														q->m_gameItem, mp.origin, mp.pos);

		Q_ASSERT(obj);

		logicRegisterObject(obj);

		m_logic->addLocalIdTag(entity);
	}
}




/**
 * @brief RpgGamePrivate::syncDefenders
 */

void RpgGamePrivate::syncDefenders()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	auto view = scope.view<Rpg::DefenderObject>(entt::exclude<Rpg::LocalIdTag>);

	for (auto entity : view) {
		const Rpg::DefenderObject &def = scope.get<Rpg::DefenderObject>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);


		RpgDefender *obj = RpgDefender::createDefender(def, q->m_gameItem, scene);


		Q_ASSERT(obj);

		obj->setTeam(def.team);
		obj->setMaxHp(def.maxHp);

		if (const Rpg::Defender *base = scope.try_get<Rpg::Defender>(def.defender)) {
			const Rpg::Tower *tower = scope.try_get<Rpg::Tower>(base->tower);

			for (RpgTower *t : m_towerList) {
				if (RpgLogicObjectMapper::getId(t->objectId()) == tower->idTag) {
					obj->setTower(t);

					for (RpgDefenderPoint *p : t->defenderPoints()) {
						if (RpgLogicObjectMapper::getId(p->objectId()) == base->idTag) {
							obj->setDefenderPoint(p);
							p->setDefender(obj);
							break;
						}
					}

					t->reloadDefenderLayersVisibility();

					break;
				}
			}
		}

		logicRegisterObject(obj);

		m_logic->addLocalIdTag(entity);


		if (obj->defenderPoint())
			obj->defenderPoint()->visualItem()->setVisible(false);
	}
}




/**
 * @brief RpgGamePrivate::syncNpc
 */

void RpgGamePrivate::syncNpc()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	Rpg::RpgLogicControlledObjects *controlledObjects = scope.getCtx<Rpg::RpgLogicControlledObjects>();

	for (auto entity : scope.view<Rpg::Npc>()) {
		const RpgStream::NpcState *state = scope.getCurrentState<RpgStream::NpcState>(entity);

		const Rpg::Npc &p = scope.get<Rpg::Npc>(entity);

		RpgNpc *obj = nullptr;

		if (scope.try_get<Rpg::LocalIdTag>(entity)) {
			obj = qobject_cast<RpgNpc*>(scope.getCtx<RpgLogicObjectMapper>()->get(p.idTag));

			if (!obj) {
				LOG_CERROR("game") << "Invalid NPC" << p.idTag;
				continue;
			}

			obj->setMaxHp(p.data.entity().maxHp());
			obj->setTeam(p.data.team());
		} else {
			TiledScene *scene = q->m_gameItem->currentScene();

			Q_ASSERT(scene);

			const auto ptr = q->readNpcDefinition(p.data.characterResolved(m_npcHash));

			if (!ptr) {
				LOG_CERROR("game") << "Invalid NPC" << p.data.character();
				continue;
			}

			const RpgNpcDefinition &def = ptr.value();

			cpVect pos;

			if (state) {
				pos.x = state->entityState().posXAsFloat();
				pos.y = state->entityState().posYAsFloat();
			}

			obj = RpgNpc::createNpc(p, q->m_gameItem, scene, pos);

			Q_ASSERT(obj);

			//obj->setDisplayName(QString("NPC #%1").arg(p.idTag));
			obj->load(def);

			if (state) {
				obj->setHp(state->hp());
				obj->setTeam(p.data.team());

				if (auto ptr = addToScatter(2)) {
					obj->setScatterPoint(ptr.value());
					ptr->scatter->setPointConfiguration(ptr->index, QXYSeries::PointConfiguration::Color,
														obj->getColor());
				}
			}

			logicRegisterObject(obj);
			m_logic->addLocalIdTag(entity);
		}

		if (!obj->secondaryMotor()) {
			if ((controlledObjects && controlledObjects->entities.contains(p.idTag)) || q->m_gameMode == RpgGame::SinglePlayer) {
				obj->setSecondaryMotor(obj->getControlledMotor());
			}
		}
	}
}





/**
 * @brief RpgGamePrivate::syncControls
 */

void RpgGamePrivate::syncControls()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	for (auto entity : scope.view<Rpg::Control>()) {

		const Rpg::Control &p = scope.get<Rpg::Control>(entity);

		RpgControl *obj = nullptr;

		if (scope.try_get<Rpg::LocalIdTag>(entity)) {
			obj = qobject_cast<RpgControl*>(scope.getCtx<RpgLogicObjectMapper>()->get(p.idTag));

			if (!obj) {
				LOG_CERROR("game") << "Invalid control" << p.idTag;
				continue;
			}

			/*if (state) {
				obj->setIsAlive(state->isAlive());
			}*/

			//obj->setMaxHp(p.data.entity().maxHp());
			//obj->setTeam(p.data.team());
		} else {
			const RpgStream::ControlState *state = scope.getCurrentState<RpgStream::ControlState>(entity);

			TiledScene *scene = q->m_gameItem->currentScene();

			Q_ASSERT(scene);

			obj = RpgControl::createControl(p, q->m_gameItem, scene);

			Q_ASSERT(obj);

			obj->initControl();

			if (state) {
				obj->setIsAlive(state->isAlive());
			}

			logicRegisterObject(obj);
			m_logic->addLocalIdTag(entity);
		}
	}
}





/**
 * @brief RpgGamePrivate::addToScatter
 * @param scatter
 * @return
 */

std::optional<ScatterPoint> RpgGamePrivate::addToScatter(const int &scatter)
{
	if (scatter < 0 || scatter >= m_scatters.size()) {
		LOG_CERROR("game") << "Invalid scatter";
		return std::nullopt;
	}

	m_scatters[scatter]->append(QPointF());

	ScatterPoint p;
	p.scatter = m_scatters[scatter];
	p.index = m_scatters[scatter]->count()-1;

	return p;
}









/**
 * @brief RpgGamePrivate::extractObjects
 * @param full
 * @return
 */

RpgGamePrivate::ObjectSet RpgGamePrivate::extractObjects(const RpgStream::FullState &full) const
{
	ObjectSet ret;

	for (const auto &p : full.players())
		ret.player.insert(p.tagId());

	for (const auto &p : full.mps())
		ret.mp.insert(p.tagId());

	for (const auto &p : full.defenders())
		ret.defender.insert(p.tagId());

	for (const auto &p : full.controls())
		ret.control.insert(p.tagId());

	return ret;
}




/**
 * @brief RpgGamePrivate::deleteMissingObjects
 * @param objects
 */

void RpgGamePrivate::deleteMissingObjects(const ObjectSet &objects)
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	for (auto it = mapper->map.cbegin(); it != mapper->map.cend(); ) {
		bool found = true;

		if (qobject_cast<RpgPlayer*>(it.value()))
			found = objects.player.contains(it.key());
		else if (qobject_cast<RpgMp*>(it.value()))
			found = objects.mp.contains(it.key());
		else if (RpgDefender *d = qobject_cast<RpgDefender*>(it.value())) {
			found = objects.defender.contains(it.key());

			if (!found) {
				if (RpgDefenderPoint *p = d->defenderPoint())
					p->setDefender(nullptr);

				if (RpgTower *t = d->tower())
					t->reloadDefenderLayersVisibility();

				d->setDefenderPoint(nullptr);
			}
		}


		if (found) {
			++it;
			continue;
		}

		q->m_gameItem->removeObject(it.value());

		it = mapper->map.erase(it);
	}
}





/**
 * @brief RpgGamePrivate::processEvents
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::Events> &list, const qint64 &tick)
{
	if (list.empty())
		return;

	for (const RpgStream::Events &event : list) {

		// Stage events (ezt nézzük mindig, mert az elején a -1 tick alatt több is történhet

		if (event.flags().testFlag(RpgStream::Events::Stage))
			processEvents(event.stage());

		// A többit nem dolgozzuk fel többször

		if ((qint64) event.tick() <= m_lastProcessedEventTick)
			continue;

		LOG_CINFO("game") << "########### EVENT" << event.tick() << event.flags() << "AT" << tick;

		// Mp emitted

		if (event.flags().testFlag(RpgStream::Events::Emitter))
			processEvents(event.emitter());


		// Players events

		if (event.flags().testFlag(RpgStream::Events::Player))
			processEvents(event.player());


		// Npc events

		if (event.flags().testFlag(RpgStream::Events::Npc))
			processEvents(event.npc());

		// Control events

		if (event.flags().testFlag(RpgStream::Events::Control))
			processEvents(event.control());

		// Defender events

		if (event.flags().testFlag(RpgStream::Events::Defender))
			processEvents(event.defender());


		m_lastProcessedEventTick = event.tick();
	}
}


/**
 * @brief RpgGamePrivate::processEvents
 * @param list
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::EventPlayer> &list)
{
	if (list.empty())
		return;

	Rpg::RpgLogicScope scope = m_logic->getScope();

	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::EventPlayer &event : list) {
		RpgPlayer *player = mapper->get<RpgPlayer>(event.tagId());

		if (!player) {
			LOG_CERROR("game") << "Invalid player id" << event.tagId();
			continue;
		}


		if (RpgMotorPlayerEventIface *motor = dynamic_cast<RpgMotorPlayerEventIface*>(player->currentMotor()))
			motor->processEvent(event);


		if (player != q->m_controlledPlayer)
			continue;

		if (event.type() == RpgStream::EventPlayer::EventMpPick) {

			q->m_gameItem->playSfx(QStringLiteral(":/rpg/common/leather_inventory.mp3"),
								   player->scene(),
								   player->bodyPositionF());
		} else if (event.type() == RpgStream::EventPlayer::EventRespawn) {

			if (const qint64 delta = q->m_gameItem->tickTimer()->tickTo(event.at()); delta > 0) {
				int sec = std::ceil(AbstractGame::TickTimer::tickToMsec(delta)/1000.);
				q->m_gameItem->message(QObject::tr("Back in %1 sec").arg(sec));

				if (GameQuestion *gq = q->gameQuestion()) {
					gq->setProperty("progressColor", QColorConstants::Svg::red);
					gq->setProperty("msecLeft", q->msecLeft()
									-AbstractGame::TickTimer::tickToMsec(delta));
				}
			}
		} else if (event.type() == RpgStream::EventPlayer::EventStreak) {
			q->m_gameItem->message(QObject::tr("%1 streak").arg(event.at()));
		}

	}
}




/**
 * @brief RpgGamePrivate::processEvents
 * @param list
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::EventStageChanged> &list)
{
	if (list.empty())
		return;

	for (const RpgStream::EventStageChanged &event : list) {
		syncGameConfig(event.config(), event.tick());
	}
}





/**
 * @brief RpgGamePrivate::processEvents
 * @param list
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::EventMpEmitter> &list)
{
	if (list.empty())
		return;

	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::EventMpEmitter &event : list) {
		RpgObject *obj = mapper->get(event.tagId());
		QPointF pos;

		if (!obj) {
			for (const RpgStream::MpEmitter &emitter : m_mapData.mpEmitterList()) {
				if (emitter.tagId() == event.tagId()) {
					pos.setX(emitter.posXAsFloat());
					pos.setY(emitter.posYAsFloat());
					break;
				}
			}
		}

		if (!pos.isNull())
			q->m_gameItem->playSfx(QStringLiteral(":/sound/sfx/pick.mp3"), q->m_gameItem->currentScene(), pos);
	}
}



/**
 * @brief RpgGamePrivate::processEvents
 * @param list
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::EventNpc> &list)
{
	if (list.empty())
		return;

	Rpg::RpgLogicScope scope = m_logic->getScope();

	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::EventNpc &event : list) {
		RpgNpc *npc = mapper->get<RpgNpc>(event.tagId());

		if (!npc) {
			LOG_CERROR("game") << "Invalid NPC id" << event.tagId();
			continue;
		}

		if (RpgMotorNpcEventIface *motor = dynamic_cast<RpgMotorNpcEventIface*>(npc->currentMotor()))
			motor->processEvent(event);
	}
}



/**
 * @brief RpgGamePrivate::processEvents
 * @param list
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::EventControl> &list)
{
	if (list.empty())
		return;

	/*Rpg::RpgLogicScope scope = m_logic->getScope();

	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	Q_ASSERT(mapper);

	for (const RpgStream::EventControl &event : list) {
		RpgNpc *npc = mapper->get<RpgNpc>(event.tagId());

		if (!npc) {
			LOG_CERROR("game") << "Invalid NPC id" << event.tagId();
			continue;
		}

		if (RpgMotorNpcEventIface *motor = dynamic_cast<RpgMotorNpcEventIface*>(npc->currentMotor()))
			motor->processEvent(event);
	}*/
}




/**
 * @brief RpgGamePrivate::processEvents
 * @param list
 */

void RpgGamePrivate::processEvents(const std::vector<RpgStream::EventDefender> &list)
{
	if (list.empty())
		return;
}





/**
 * @brief RpgGamePrivate::onTimeStepped
 */

void RpgGamePrivate::onTimeStepped(const std::vector<TiledObjectBody *> &aboutDestruction)
{
	for (TiledObjectBody *b : aboutDestruction) {
		if (q->m_controlledPlayer && q->m_controlledPlayer->targetControl() == b)
			q->m_controlledPlayer->setTargetControl(nullptr);

		if (q->m_controlledPlayer && q->m_controlledPlayer->targetEntity() == b)
			q->m_controlledPlayer->setTargetEntity(nullptr);
	}


	// SfxLocations

	for (const auto &ptr : m_sfxLocations) {
		if (ptr->baseObject()->scene() != ptr->connectedScene())
			ptr->setConnectedScene(ptr->baseObject()->scene());
		ptr->checkPosition();
	}
}










/**
 * @brief RpgGamePrivate::logicRegisterObject
 * @param object
 */

quint32 RpgGamePrivate::logicRegisterObject(RpgObject *object)
{
	if (!object) {
		LOG_CERROR("game") << "Invalid object";
		return 0;
	}

	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	Q_ASSERT(mapper);

	return mapper->set(object);
}





#ifdef WITH_GAMEPAD


/**
 * @brief RpgGamePrivate::onGamePadChanged
 */

void RpgGamePrivate::onGamePadChanged(const double &)
{
	if (RpgPlayer *p = q->controlledPlayer()) {
		if (RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(p->currentMotor())) {
			setFromGamepad(motor);
		}
	}
}


/**
 * @brief RpgGamePrivate::onGamePadButtonL3Changed
 * @param pressed
 */

void RpgGamePrivate::onGamePadButtonL3Changed(const bool &pressed)
{
	if (!pressed)
		joystickClickedB(true);
}


/**
 * @brief RpgGamePrivate::onGamePadButtonR3Changed
 * @param pressed
 */

void RpgGamePrivate::onGamePadButtonR3Changed(const bool &pressed)
{
	if (!pressed)
		joystickClickedC(true);
}


/**
 * @brief RpgGamePrivate::onGamePadButtonL1Changed
 * @param pressed
 */

void RpgGamePrivate::onGamePadButtonL1Changed(const bool &pressed)
{
	if (!pressed)
		joystickClickedB(true);
}


/**
 * @brief RpgGamePrivate::onGamePadButtonR1Changed
 * @param pressed
 */

void RpgGamePrivate::onGamePadButtonR1Changed(const bool &pressed)
{
	if (!pressed)
		joystickClickedC(true);
}


#endif



/**
 * @brief RpgGame::gameState
 * @return
 */


RpgGame::GameState RpgGame::gameState() const
{
	return m_gameState;
}

void RpgGame::setGameState(const GameState &newGameState)
{
	if (m_gameState == newGameState)
		return;

	m_gameState = newGameState;
	emit gameStateChanged();
}



/**
 * @brief RpgGame::errorString
 * @return
 */

QString RpgGame::errorString() const
{
	return m_errorString;
}

void RpgGame::setErrorString(const QString &newErrorString)
{
	if (m_errorString == newErrorString)
		return;
	m_errorString = newErrorString;
	emit errorStringChanged();
}



/**
 * @brief RpgGamePrivate::clearSharedTextures
 */

RpgGamePrivate::RpgGamePrivate(RpgGame *game, const bool &multi, std::unique_ptr<Rpg::RpgLogicClientTutorial::Tutorial> tutorial)
	: QObject()
	, q(game)
{
	if (tutorial)
		m_logic = std::make_unique<Rpg::RpgLogicClientTutorial>(q, std::move(tutorial));
	else if (multi)
		m_logic = std::make_unique<Rpg::RpgLogicClientMulti>();
	else
		m_logic = std::make_unique<Rpg::RpgLogicClientSingle>(q);


	m_questionIterator = m_questionList.constBegin();


#ifdef WITH_GAMEPAD

	//QLoggingCategory::setFilterRules(QStringLiteral("qt.gamepad.debug=true"));

	bool gpEnabled = Utils::settingsGet(QStringLiteral("game/gamepad"), true).toBool();

	if (auto l = QGamepadManager::instance()->connectedGamepads(); !l.isEmpty()) {
		LOG_CDEBUG("app") << "Found gamepads:" << l.size();

		if (!gpEnabled) {
			LOG_CINFO("app") << "Gamepad disabled";
		} else {
			m_gamePad.reset(new QGamepad(l.first(), nullptr));

			LOG_CINFO("app") << "Connected gamepad:" << m_gamePad->name();

			connect(m_gamePad.get(), &QGamepad::axisLeftXChanged, this, &RpgGamePrivate::onGamePadChanged);
			connect(m_gamePad.get(), &QGamepad::axisLeftYChanged, this, &RpgGamePrivate::onGamePadChanged);
			connect(m_gamePad.get(), &QGamepad::axisRightXChanged, this, &RpgGamePrivate::onGamePadChanged);
			connect(m_gamePad.get(), &QGamepad::axisRightYChanged, this, &RpgGamePrivate::onGamePadChanged);

			connect(m_gamePad.get(), &QGamepad::buttonL3Changed, this, &RpgGamePrivate::onGamePadButtonL3Changed);
			connect(m_gamePad.get(), &QGamepad::buttonR3Changed, this, &RpgGamePrivate::onGamePadButtonR3Changed);
			connect(m_gamePad.get(), &QGamepad::buttonL1Changed, this, &RpgGamePrivate::onGamePadButtonL1Changed);
			connect(m_gamePad.get(), &QGamepad::buttonR1Changed, this, &RpgGamePrivate::onGamePadButtonR1Changed);
		}
	}
#endif

}




/**
 * @brief RpgGamePrivate::~RpgGamePrivate
 */

RpgGamePrivate::~RpgGamePrivate()
{
	if (Rpg::RpgLogicClientMulti *l = dynamic_cast<Rpg::RpgLogicClientMulti*>(m_logic.get())) {
		l->setEngine(nullptr);
	}

	m_engine.reset();
	m_logic.reset();
}


/**
 * @brief RpgGamePrivate::toReadableRoomId
 * @param room
 * @return
 */

QString RpgGamePrivate::toReadableRoomId(const RpgStream::Room &room)
{
	int id = room.readableId();

	return QStringLiteral("%1 %2")
			.arg((int) std::floor(id/1000), 3, 10, QChar('0'))
			.arg((int) (id - std::floor(id/1000)*1000), 3, 10, QChar('0'))
			;
}



/**
 * @brief RpgGamePrivate::clearSharedTextures
 */


void RpgGamePrivate::clearSharedTextures()
{
	LOG_CTRACE("game") << "Clear shared textures";

	TiledQuick::TileLayerItem::clearSharedTextures();
	TiledGame::clearSharedTextures();
	Tiled::ImageCache::clear();
}




/**
 * @brief RpgGamePrivate::vibrate
 */

void RpgGamePrivate::vibrate()
{
#ifndef Q_OS_WASM
	StandaloneClient *client = qobject_cast<StandaloneClient*>(Application::instance()->client());
	if (client)
		client->performVibrate();
#endif

}


/**
 * @brief RpgGamePrivate::loadCommonMaps
 */

bool RpgGamePrivate::loadCommonMaps()
{
	if (!m_commonMaps.empty())
		return false;


	LOG_CDEBUG("game") << "Load common maps...";


	for (const TiledSceneDefinition &def : m_commonGameDefinition.scenes) {
		Tiled::MapReader mapReader;

		const QString file = m_commonGameDefinition.basePath+def.file;

		auto ptr = mapReader.readMap(Tiled::urlToLocalFileOrQrc(file));

		if (!ptr) {
			LOG_CERROR("game") << "Map can't loaded" << file;
			m_commonMaps.clear();
			return false;
		} else {
			LOG_CDEBUG("game") << "Map loaded" << file;
			auto renderer = Tiled::MapRenderer::create(ptr.get());
			m_commonMaps[def.file] = std::make_pair(std::move(ptr), std::move(renderer));
		}
	}

	return true;
}



/**
 * @brief RpgGamePrivate::connectionPrepare
 */

void RpgGamePrivate::connectionPrepare()
{
	q->setGameState(RpgGame::GameStateConnect);

	if (q->m_gameMode == RpgGame::SinglePlayer)
		return connectionCheck();

	LOG_CINFO("game") << "PREPARE MULTIPLAYER";

	if (m_engine) {
		LOG_CINFO("client") << "ALREADY HAS ENGINE";
		return;
	}

	LOG_CINFO("client") << "START" << q->client()->server()->url();


	RpgStream::ConnectionToken token;

	token.mapUuid = "---";
	token.missionUuid = "xxxxx";
	token.missionLevel = 1;
	token.campaign = -1;


	q->m_client->send(HttpConnection::ApiUser, QStringLiteral("campaign/%1/game/token").arg(
						  0
						  ), token.toJson())
			->error(this, [this](const QNetworkReply::NetworkError &){
		q->setError(tr("Játék indítása sikertelen"));
	})
			->fail(this, [this](const QString &err){
		if (err.startsWith(QStringLiteral("active"))) {
			//QStringList f = err.split('/', Qt::SkipEmptyParts);

			q->setError(err);

			/*if (f.size() > 2) {
				setActiveSeat(f.at(1).toUInt());
				setActiveEngine(f.at(2).toInt());

				m_client->messageError(tr("Be vagy jelentkezve egy aktív játékba/szobába. "
										  "Próbáld újra úgy, hogy kiválasztod a lezárási lehetőséget."),
									   tr("Aktív játék/szoba van folyamatban"));

				destroyCurrentGame();
				setGameState(StateSelect);
				return;
			}*/
		}

		/*m_client->messageError(err, tr("Játék indítása sikertelen"));
		destroyCurrentGame();*/
	})
			->done(this, [this](const QJsonObject &data){

		const QByteArray &token = data.value(QStringLiteral("token")).toString().toLatin1();

		if (token.isEmpty()) {
			q->setError(tr("Érvénytelen játékazonosító érekezett"));
			return;
		}

		LOG_CDEBUG("client") << "Game play (multiplayer)";

		m_connectionToken = token;

		QMetaObject::invokeMethod(this, &RpgGamePrivate::connectionCheck, Qt::QueuedConnection);
	});

}




/**
 * @brief RpgGamePrivate::connectionCheck
 */

void RpgGamePrivate::connectionCheck()
{
	if (q->m_gameMode == RpgGame::MultiPlayer && m_connectionToken.isEmpty()) {
		q->setError(tr("Játékazonosító hiányzik"));
		return;
	}


	////q->setGameState(RpgGame::GameStateLobby);

	q->reloadTerrains();
	q->reloadCharacters();

	if (RpgGame::terrains().empty()) {
		q->setError("NINCS TEREP");
		return;
	}

	if (q->m_gameMode == RpgGame::SinglePlayer) {
		connectionReady();
		return;
	}

	if (m_engine)
		return;

	if (!q->client()->server()) {
		q->setError(tr("Nincs beállítva szerver"));
		return;
	}

	const auto &ptr = Application::instance()->getSigner();

	if (!ptr) {
		q->setError(tr("Hitelesítés elérhetetlen"));
		return;
	}

	m_engine = std::make_unique<RpgUdpEngine>(this, ptr.value());

	connect(m_engine.get(), &RpgUdpEngine::serverConnected, this, &RpgGamePrivate::onServerConnected);
	connect(m_engine.get(), &RpgUdpEngine::serverDisconnected, this, &RpgGamePrivate::onServerDisconnected);
	connect(m_engine.get(), &RpgUdpEngine::serverConnectFailed, this, &RpgGamePrivate::onConnectionFailed);
	connect(m_engine.get(), &RpgUdpEngine::serverConnectionLost, this, &RpgGamePrivate::onConnectionLost);


	m_engine->setConnectionToken(m_connectionToken);
	m_engine->setUrl(q->client()->server()->url());

	if (Rpg::RpgLogicClientMulti *l = dynamic_cast<Rpg::RpgLogicClientMulti*>(m_logic.get())) {
		l->setEngine(m_engine.get());
	}
}



/**
 * @brief RpgGamePrivate::connectionReady
 */

void RpgGamePrivate::connectionReady()
{
	if (Rpg::RpgLogicClientTutorial *tutorial = dynamic_cast<Rpg::RpgLogicClientTutorial*>(m_logic.get())) {
		LOG_CINFO("game") << "READY TUTORIAL";

		if (!tutorial->loadGameData(&m_characterSelect)) {
			q->setError(tr("Hibás tutorial"));
			return;
		}

		m_characterSelect.data().flags().setFlag(RpgStream::PlayerData::FlagCompleted);

		QMetaObject::invokeMethod(this, &RpgGamePrivate::updateCharacterSelect, Qt::QueuedConnection);

		return;
	}

	if (q->m_gameMode == RpgGame::SinglePlayer) {
		q->setGameState(RpgGame::GameStateCharacterSelect);
		return;
	}

	LOG_CINFO("game") << "READY TO LOBBY";

	q->setGameState(RpgGame::GameStateLobby);
}




/**
 * @brief RpgGamePrivate::onServerConnected
 */

void RpgGamePrivate::onServerConnected()
{
	connectionReady();
}


/**
 * @brief RpgGamePrivate::onServerDisconnected
 */

void RpgGamePrivate::onServerDisconnected()
{
	LOG_CERROR("game") << "DISCONNECTED";
}



/**
 * @brief RpgGamePrivate::onConnectionFailed
 * @param err
 */

void RpgGamePrivate::onConnectionFailed(const QString &err)
{
	q->setError(err);
	m_engine.reset();
}



/**
 * @brief RpgGamePrivate::onConnectionLost
 */

void RpgGamePrivate::onConnectionLost()
{
	LOG_CWARNING("game") << "LOST";
}





/**
 * @brief RpgGamePrivate::downloadServerData
 */

void RpgGamePrivate::downloadServerData()
{
	q->m_client->send(HttpConnection::ApiUser, QStringLiteral("rpg"))/*.arg(
									   m_campaign ? m_campaign->campaignid() : 0
													), {
									   { QStringLiteral("map"), m_gameMap->uuid() },
									   { QStringLiteral("mission"), levelGame->uuid() },
									   { QStringLiteral("level"), levelGame->level() },
									   { QStringLiteral("mode"), levelGame->mode() },
									   { QStringLiteral("extended"), m_extendedData }
								   })*/
			->error(this, [this](const QNetworkReply::NetworkError &){
		q->setError(tr("Játék indítása sikertelen"));
	})
			->fail(this, [this](const QString &err){
		q->setError(tr("Játék indítása sikertelen: %1").arg(err));
	})
			->done(this, [this](const QJsonObject &data){
		LOG_CDEBUG("client") << "RPG DATA LOADED" << data;

		RpgUserData udata;
		udata.fromJson(data);

		QVariantList out;
		out.reserve(udata.characters.size());

		for (const RpgUserCharacter &ch : udata.characters) {
			QJsonObject obj = ch.toJson();

			int to = ch.level > 0 && ch.level < ch.pwrUnlock.size() ? ch.pwrUnlock.at(ch.level) : 0;

			obj[QStringLiteral("nextPoint")] = to;
			obj[QStringLiteral("isTarget")] = ch.character == udata.target;
			obj[QStringLiteral("targetToken")] = ch.unlock;

			out.append(obj.toVariantMap());
		}


		Utils::patchSListModel(q->m_modelCharacters.get(),
							   out,
							   QStringLiteral("character"));

		QMetaObject::invokeMethod(this, &RpgGamePrivate::downloadStaticData, Qt::QueuedConnection);
	});

}



/**
 * @brief RpgGamePrivate::downloadStaticData
 */

void RpgGamePrivate::downloadStaticData()
{
	q->m_client->downloader()->check();

	if (q->m_client->downloader()->state() == Downloader::StateContentReady) {
		return onContentDownloaded();
	}

	q->setGameState(RpgGame::GameStateDownloadContent);

	qint64 size = q->m_client->downloader()->fullSize() - q->m_client->downloader()->downloadedSize();

	if (size > 1000000)
		emit q->downloadRequest(QLocale::system().formattedDataSize(size));
	else
		q->downloadAccepted();
}






/**
 * @brief RpgGamePrivate::contentPrepare
 */

void RpgGamePrivate::contentPrepare()
{
	/// TODO: check demo mode,...etc.

	downloadServerData();

	//downloadStaticData();
}



/**
 * @brief RpgGamePrivate::onDownloaderStateChanged
 */

void RpgGamePrivate::onDownloaderStateChanged()
{
	if (q->m_client->downloader()->state() == Downloader::StateContentReady)
		onContentDownloaded();
}


/**
 * @brief RpgLogicObjectMapper::getId
 * @param id
 * @return
 */

quint32 RpgLogicObjectMapper::getId(const TiledObjectBody::ObjectId &id)
{
	return Rpg::RpgLogic::packId(id.sceneId, id.ownerId, id.id);
}


/**
 * @brief RpgLogicObjectMapper::getId
 * @param object
 * @return
 */

quint32 RpgLogicObjectMapper::getId(const RpgObject *object)
{
	return object ? getId(object->objectId()) : 0;
}


/**
 * @brief RpgLogicObjectMapper::toObjectId
 * @param id
 * @return
 */

TiledObjectBody::ObjectId RpgLogicObjectMapper::toObjectId(const quint32 &id)
{
	TiledObjectBody::ObjectId oid;

	Rpg::RpgLogic::unpackId(id, oid.sceneId, oid.ownerId, oid.id);

	return oid;
}


/**
 * @brief RpgLogicObjectMapper::set
 * @param object
 * @return
 */

quint32 RpgLogicObjectMapper::set(RpgObject *object)
{
	if (!object)
		return 0;

	quint32 id = getId(object);
	map.insert(id, object);
	return id;
}



/**
 * @brief RpgGame::controlledPlayer
 * @return
 */

RpgPlayer *RpgGame::controlledPlayer() const
{
	return m_controlledPlayer;
}

void RpgGame::setControlledPlayer(RpgPlayer *newControlledPlayer)
{
	if (m_controlledPlayer == newControlledPlayer)
		return;
	m_controlledPlayer = newControlledPlayer;
	emit controlledPlayerChanged();

	if (m_gameItem)
		m_gameItem->setFollowedItem(m_controlledPlayer);
}



/**
 * @brief RpgGame::timerEvent
 */

void RpgGame::timerEvent(QTimerEvent *)
{
	if (!d->m_questSelectTimer.isForever() && d->m_questSelectTimer.hasExpired()) {
		d->questSelectTimerSet(0);

		if (m_gameMode == SinglePlayer) {
			if (Rpg::RpgLogicClientSingle *logic = dynamic_cast<Rpg::RpgLogicClientSingle*>(d->m_logic.get())) {
				d->syncGameConfig(logic->startGame(), logic->serverTick());
			}
		}
	}

	//LOG_CDEBUG("game") << "TICK" << m_gameItem->tickTimer()->currentTick();
	emit msecLeftChanged();
}



/**
 * @brief RpgGame::reloadCharacters
 */

void RpgGame::reloadCharacters()
{
	LOG_CDEBUG("game") << "Reload available RPG characters...";

	m_characters.clear();
	RpgGamePrivate::m_characterHash.clear();

	QDirIterator it(QStringLiteral(":/character"), {QStringLiteral("character.json")}, QDir::Files, QDirIterator::Subdirectories);

	static const auto writeOnConfig = [](RpgPlayerDefinition *cfg, const QJsonObject &obj, const QString &path) {
		Q_ASSERT(cfg);
		cfg->fromJson(obj);

		if (cfg->name.isEmpty())
			cfg->name = path;

		cfg->prefixPath = QStringLiteral(":/character/").append(path).append('/');

		if (!cfg->image.isEmpty()) {
			cfg->image.prepend(QStringLiteral("qrc")+cfg->prefixPath);
		}

		cfg->updateSfxPath(cfg->prefixPath);
	};

	while (it.hasNext()) {
		const QString &f = it.next();

		QString id = f.section('/',-2,-2);

		const auto &ptr = Utils::fileToJsonObject(f);

		if (!ptr) {
			LOG_CERROR("game") << "Invalid config json:" << f;
			continue;
		}

		RpgPlayerDefinition config;

		writeOnConfig(&config, ptr.value(), id);


		if (!config.base.isEmpty()) {
			QString basePath = QStringLiteral(":/character/").append(config.base).append(QStringLiteral("/character.json"));

			const auto &basePtr = Utils::fileToJsonObject(basePath);

			if (!basePtr) {
				LOG_CERROR("game") << "Invalid config base:" << config.base << "in:" << f;
			} else {
				QString base = config.base;
				config = RpgPlayerDefinition{};
				writeOnConfig(&config, basePtr.value(), base);
				writeOnConfig(&config, ptr.value(), id);
			}
		}

		m_characters.insert(id, config);
		RpgGamePrivate::m_characterHash.insert(id);
	}

	LOG_CDEBUG("game") << "...loaded " << m_characters.size() << " characters";
}






/**
 * @brief RpgGame::readNpcDefinition
 * @param name
 * @return
 */

std::optional<RpgNpcDefinition> RpgGame::readNpcDefinition(const QString &name)
{
	if (name.isEmpty())
		return std::nullopt;

	if (RpgGamePrivate::m_npcDefinitions.contains(name))
		return RpgGamePrivate::m_npcDefinitions.value(name);

	Downloader *downloader = Application::instance()->client()->downloader();

	Q_ASSERT(downloader);

	if (!downloader->loadDynamicContent(name+QStringLiteral(".dres")))
		return std::nullopt;

	RpgNpcDefinition config;

	config.prefixPath = QStringLiteral(":/enemy/").append(name).append('/');

	const auto &ptr = Utils::fileToJsonObject(QStringLiteral(":/enemy/%1/config.json").arg(name));

	if (ptr) {
		LOG_CINFO("game") << "Load config json:" << name;

		config.fromJson(ptr.value());

		config.updateSfxPath(config.prefixPath);
	}

	RpgGamePrivate::m_npcDefinitions.insert(name, config);
	RpgGamePrivate::m_npcHash.insert(name);

	return config;
}






/**
 * @brief RpgPlayerDefinition::updateSfxPath
 * @param prefix
 */

void RpgPlayerDefinition::updateSfxPath(const QString &prefix)
{
	for (QList<QString> *ptr : std::vector<QList<QString>*>{
		 &sfxAccept,
		 &sfxDecline,
		 &sfxFootStep,
		 &sfxPain,
}) {
		for (QString &s : *ptr) {
			if (!s.isEmpty() && !s.startsWith(QStringLiteral(":/")))
				s.prepend(prefix);
		}
	}

	if (!sfxDead.isEmpty() && !sfxDead.startsWith(QStringLiteral(":/")))
		sfxDead.prepend(prefix);
}



/**
 * @brief RpgPlayerDefinition::toPlayerConfig
 * @return
 */

RpgStream::PlayerConfig RpgPlayerDefinition::toPlayerConfig() const
{
	RpgStream::PlayerConfig cfg;

	cfg.setPower(power);

	cfg.setMaxMp(mp);
	cfg.setMaxBullet(bullet);


	cfg.entity().setMaxHp(hp);

	cfg.entity().setPush(push);
	cfg.entity().setPushDist(pushDistance);
	cfg.entity().setResist(resist);

	cfg.defenders().reserve(defender.size());

	for (const RpgStream::BaseDefenderObject::Type &t : defender)
		cfg.defenders().push_back(t);

	cfg.utilities().reserve(utility.size());

	for (const RpgStream::PlayerConfig::Utility &t : utility)
		cfg.utilities().push_back(t);

	cfg.setTowerPlus(towerPlus);
	cfg.setTowerPlus(towerMinus);

	return cfg;
}


/**
 * @brief RpgPlayerDefinition::loadPlayerConfig
 * @param cfg
 */

void RpgPlayerDefinition::loadPlayerConfig(const RpgStream::PlayerConfig &cfg)
{
	power = cfg.power();

	mp = cfg.maxMp();
	bullet = cfg.maxBullet();

	hp = cfg.entity().maxHp();

	push = cfg.entity().push();
	pushDistance = cfg.entity().pushDist();
	resist = cfg.entity().resist();

	defender.clear();
	defender.reserve(cfg.defenders().size());

	for (const RpgStream::BaseDefenderObject::Type &t : cfg.defenders())
		defender.append(t);

	utility.clear();
	utility.reserve(cfg.utilities().size());

	for (const RpgStream::PlayerConfig::Utility &t : cfg.utilities())
		utility.append(t);

	towerPlus = cfg.towerPlus();
	towerMinus = cfg.towerMinus();
}



int RpgGame::ptsTeam() const
{
	return m_ptsTeam;
}

void RpgGame::setPtsTeam(int newPtsTeam)
{
	if (m_ptsTeam == newPtsTeam)
		return;
	m_ptsTeam = newPtsTeam;
	emit ptsTeamChanged();
}

int RpgGame::ptsOpponent() const
{
	return m_ptsOpponent;
}

void RpgGame::setPtsOpponent(int newPtsOpponent)
{
	if (m_ptsOpponent == newPtsOpponent)
		return;
	m_ptsOpponent = newPtsOpponent;
	emit ptsOpponentChanged();
}





/**
 * @brief RpgGame::readableRoom
 * @return
 */

QString RpgGame::readableRoom() const
{
	if (!d->m_engine)
		return QString();

	if (const auto &ptr = d->m_engine->room())
		return d->toReadableRoomId(ptr.value());
	else
		return QString();
}

QString RpgGame::terrain() const
{
	return m_terrain;
}

void RpgGame::setTerrain(const QString &newTerrain)
{
	if (m_terrain == newTerrain)
		return;
	m_terrain = newTerrain;
	emit terrainChanged();
}





/**
 * @brief RpgNpcDefinition::toEntityConfig
 * @return
 */

void RpgNpcDefinition::updateSfxPath(const QString &prefix)
{
	for (QList<QString> *ptr : std::vector<QList<QString>*>{
		 &sfxFootStep,
		 &sfxPain,
}) {
		for (QString &s : *ptr) {
			if (!s.isEmpty() && !s.startsWith(QStringLiteral(":/")))
				s.prepend(prefix);
		}
	}

	if (!sfxDead.isEmpty() && !sfxDead.startsWith(QStringLiteral(":/")))
		sfxDead.prepend(prefix);
}





/**
 * @brief RpgNpcDefinition::toEntityConfig
 * @return
 */

RpgStream::EntityConfig RpgNpcDefinition::toEntityConfig() const
{
	RpgStream::EntityConfig cfg;

	cfg.setMaxHp(hp);

	cfg.setPush(push);
	cfg.setPushDist(pushDistance);
	cfg.setResist(resist);

	return cfg;
}


/**
 * @brief RpgNpcDefinition::toNpcData
 * @return
 */

RpgStream::NpcData RpgNpcDefinition::toNpcData() const
{
	RpgStream::NpcData d;

	d.setType(type);
	d.setEntity(toEntityConfig());
	d.setMp(mp);

	return d;
}

QColor RpgGame::colorTeam()
{
	return m_colorTeam;
}

QColor RpgGame::colorOpponent()
{
	return m_colorOpponent;
}

QColor RpgGame::colorNeutral()
{
	return m_colorNeutral;
}



/**
 * @brief RpgGame::getColor
 * @param team
 * @param neutral
 * @return
 */

QColor RpgGame::getColor(const RpgStream::Team &team, const QColor &neutral) const
{
	if (!m_controlledPlayer || team == RpgStream::TeamNone)
		return neutral;
	else if (m_controlledPlayer->team() == team)
		return m_colorTeam;
	else
		return m_colorOpponent;
}



/**
 * @brief RpgGameDefinition::getDynamicContent
 * @return
 */

QStringList RpgGameDefinition::getDynamicContent() const
{
	QStringList list = required;

	for (const RpgHeat &h : heat) {
		for (const RpgHeatNpc &npc : h.npc) {
			list << npc.type;
		}
	}

	list.removeDuplicates();

	return list;
}


/**
 * @brief RpgGame::heat
 * @return
 */

int RpgGame::heat() const
{
	return m_heat;
}

void RpgGame::setHeat(int newHeat)
{
	if (m_heat == newHeat)
		return;
	m_heat = newHeat;
	emit heatChanged();
}

int RpgGame::questQuestion() const
{
	return m_questQuestion;
}

void RpgGame::setQuestQuestion(int newQuestQuestion)
{
	if (m_questQuestion == newQuestQuestion)
		return;
	m_questQuestion = newQuestQuestion;
	emit questQuestionChanged();
}

int RpgGame::questQuestionRq() const
{
	return m_questQuestionRq;
}

void RpgGame::setQuestQuestionRq(int newQuestQuestionRq)
{
	if (m_questQuestionRq == newQuestQuestionRq)
		return;
	m_questQuestionRq = newQuestQuestionRq;
	emit questQuestionRqChanged();
}

int RpgGame::questStreak() const
{
	return m_questStreak;
}

void RpgGame::setQuestStreak(int newQuestStreak)
{
	if (m_questStreak == newQuestStreak)
		return;
	m_questStreak = newQuestStreak;
	emit questStreakChanged();
}

int RpgGame::questStreakRq() const
{
	return m_questStreakRq;
}

void RpgGame::setQuestStreakRq(int newQuestStreakRq)
{
	if (m_questStreakRq == newQuestStreakRq)
		return;
	m_questStreakRq = newQuestStreakRq;
	emit questStreakRqChanged();
}

int RpgGame::questPtsRq() const
{
	return m_questPtsRq;
}

void RpgGame::setQuestPtsRq(int newQuestPtsRq)
{
	if (m_questPtsRq == newQuestPtsRq)
		return;
	m_questPtsRq = newQuestPtsRq;
	emit questPtsRqChanged();
}



/**
 * @brief RpgGame::questResultData
 * @return
 */

QVariantMap RpgGame::questResultData() const
{
	return m_questResultData;
}

void RpgGame::setQuestResultData(const QVariantMap &newQuestResultData)
{
	if (m_questResultData == newQuestResultData)
		return;
	m_questResultData = newQuestResultData;
	emit questResultDataChanged();
}


/**
 * @brief RpgGame::questSelectData
 * @return
 */

QVariantMap RpgGame::questSelectData() const
{
	return m_questSelectData;
}

void RpgGame::setQuestSelectData(const QVariantMap &newQuestSelectData)
{
	if (m_questSelectData == newQuestSelectData)
		return;
	m_questSelectData = newQuestSelectData;
	emit questSelectDataChanged();
}
