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
#include "application.h"
#include "gamequestion.h"
#include "litegame.h"
#include "rpgnpc.h"
#include "rpgplayer.h"
#include "rpgstream.h"
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


/**
 * @brief RpgGame::RpgGame
 * @param missionLevel
 * @param client
 */

RpgGame::RpgGame(GameMapMissionLevel *missionLevel, Client *client, const bool &multiplayer)
	: AbstractLevelGame(GameMap::Rpg, missionLevel, client)
	, d(new RpgGamePrivate(this, multiplayer))
	, m_gameMode(multiplayer ? MultiPlayer : SinglePlayer)
	, m_modelLobby(new QSListModel)
	, m_modelPlayer(new QSListModel)
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
	if (m_gameState == GameStateFinished)
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

	setFinishState(Fail);

	LOG_CINFO("game") << "Game aborted:" << this;

	gameFinish();

	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/game_over.mp3"), Sound::VoiceoverChannel);
	m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/you_lose.mp3"), Sound::VoiceoverChannel);
}



/**
 * @brief RpgGame::loadGameItem
 */

void RpgGame::loadGameItem()
{
	LOG_CDEBUG("game") << "LOAD GAME ITEM";

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
	/////m_client->sound()->playSound(QStringLiteral("qrc:/sound/menu/bg.mp3"), Sound::MusicChannel);
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

		m_characterSelect.data().setConfig(def.toPlayerConfig());
		m_characterSelect.data().setCharacterResolved(character);

		LOG_CINFO("game") << "PLAYER" << character << def.hp << m_characterSelect.data().config().entity().maxHp();
	}

	if (data.contains(QStringLiteral("nickname")))
		m_characterSelect.data().setNickName(data.value(QStringLiteral("nickname")).toString().toUtf8());

	if (data.contains(QStringLiteral("terrain"))) {
		LOG_CINFO("game") << "TERRAIN" << data.value(QStringLiteral("terrain")).toString()
						  << RpgStream::HashFnv1A64::hashFnv1a64(data.value(QStringLiteral("terrain")).toString().toStdString());

		m_characterSelect.gameConfig().setTerrainResolved(data.value(QStringLiteral("terrain")).toString());

		LOG_CINFO("game") << "*****" << m_characterSelect.gameConfig().terrain();
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
	if (q->m_gameState != RpgGame::GameStateCharacterSelect)
		return;

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
		auto p = m_logic->playerAdd(m_characterSelect.data(), &id, &tagId);

		////////////////////////////////////////////////////////////////////////
		LOG_CERROR("game") << "REMOVE<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";

		const QString character = "soldier04";

		RpgNpcDefinition def = q->readNpcDefinition(character).value_or(RpgNpcDefinition{});

		RpgStream::NpcData d;
		d.setCharacterResolved(character);
		d.setTeam(RpgStream::TeamNone);
		d.setType(RpgStream::NpcData::TowerAttacker);
		d.setEntity(def.toEntityConfig());

		m_logic->addNpc(d, p);

		////////////////////////////////////////////////////////////////////////


		q->setGameState(RpgGame::GameStatePrepare);
	}
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

	const auto &ptr = TiledGame::getDynamicTilesets(def);

	if (!ptr) {
		q->setError(tr("Nem sikerült betölteni a terepet"));
		return;
	}

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

	///////////////////////
	LOG_CERROR("game") << "REMOVE<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
	QStringList l = def.required;
	l << "soldier01" << "soldier02" << "soldier04";

	//for (const QString &s : def.required) {

	////////////////////////
	for (const QString &s : l) {
		if (!q->m_client->downloader()->loadDynamicContent(s+QStringLiteral(".dres"))) {
			q->setError(tr("Nem sikerült betölteni a terepet"));
			return;
		}

		if (!q->readNpcDefinition(s)) {
			q->setError(tr("Nem sikerült betölteni a terepet"));
			return;
		}
	}

	LOG_CDEBUG("game") << "LOAD" << cfg->terrain() << terrain << def.name;

	if (q->load(def)) {
		q->m_gameItem->setIsContentReady(true);
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

	LOG_CERROR("game") << "#####xLOAD" << m_mapData.chunkGrid().excludeList().size()
					   << m_mapData.towerList().size();

	logic->loadMapData(m_mapData);
	m_isMapLoaded = true;

	RpgStream::GameConfig cfg = logic->start();

	syncObjects();
	syncGameState();
	syncGameConfig(cfg, 0);

	QTimer::singleShot(3000, this, [this, logic]() {
		LOG_CERROR("game") << "REMOVE THIS" << logic->serverTick();

		syncGameConfig(logic->startGame(), logic->serverTick());

		LOG_CERROR("game") << "REMOVE ...." << logic->serverTick();
	});

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

void RpgGamePrivate::mpEmitterAdd(const QPointF &pos, const quint32 &tagId)
{
	RpgStream::MpEmitter p;
	p.setTagId(tagId);
	p.setPosXAsFloat(pos.x());
	p.setPosYAsFloat(pos.y());

	m_mapData.mpEmitterList().emplace_back(std::move(p));
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

	m_towerList.append(tower);
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

	if (setFromGamepad(motor))
		return;

	if (joystick == TiledGame::JoystickA)
		motor->setCurrentJoystickState(state);
	else if (joystick == TiledGame::JoystickB)
		motor->setControlJoystickState(state);
	else if (joystick == TiledGame::JoystickC)
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
		motor->setControlJoystickState(stateB);



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

	motor->putDefender(clicked);
}


/**
 * @brief RpgGamePrivate::joystickClickedC
 * @param clicked
 */

void RpgGamePrivate::joystickClickedC(const bool &/*clicked*/)
{
	if (!q->m_controlledPlayer)
		return;

	RpgMotorPlayerControlled *motor = dynamic_cast<RpgMotorPlayerControlled*>(q->m_controlledPlayer->currentMotor());

	if (!motor)
		return;

	motor->attackCurrentTarget();
}


/**
 * @brief RpgGamePrivate::joystickClickedD
 * @param clicked
 */

void RpgGamePrivate::joystickClickedD(const bool &clicked)
{
	LOG_CINFO("game") << "CLICKED D" << clicked;
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

	int xp = gq->questionData().value(QStringLiteral("xpFactor"), 0.0).toReal() * 10.;


	gq->answerReveal(answer);
	gq->setMsecBeforeHide(0);
	///gq->finish();			- finish by motor

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
	q->m_client->sound()->playSound(QStringLiteral("qrc:/sound/voiceover/fight.mp3"), Sound::VoiceoverChannel);
}


/**
 * @brief RpgGamePrivate::onQuestionFinished
 */

void RpgGamePrivate::onQuestionFinished()
{
	q->m_gameItem->forceActiveFocus(Qt::OtherFocusReason);
}



/**
 * @brief RpgGamePrivate::startGame
 */

void RpgGamePrivate::startGame(const quint32 &tick)
{
	LOG_CINFO("game") << "START GAME" << tick << m_logic->serverTick() << m_logic->estimatedServerTick() << "AUTH" << m_logic->lastAuthTick();

	q->setGameState(RpgGame::GameStatePlay);
	q->m_gameItem->tickTimer()->start(q, tick);
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

void RpgGamePrivate::finishGame()
{
	if (q->gameState() != RpgGame::GameStatePlay) {
		LOG_CERROR("game") << "Invalid state" << q->gameState();
		return;
	}

	LOG_CINFO("game") << "FINISH GAME" << q->m_gameItem->tickTimer()->currentTick() << m_logic->serverTick() << m_logic->estimatedServerTick() << "AUTH" << m_logic->lastAuthTick();

	q->setGameState(RpgGame::GameStateFinished);
	q->m_gameItem->tickTimer()->stop();

	q->setFinishState(AbstractGame::Fail);			// TODO

	q->gameFinish();
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
		LOG_CINFO("game") << "START:" << cfg->stage() << "->" << config.stage();
		q->m_gameItem->message(QObject::tr("START"));

		cfg->flags().setFlag(RpgStream::GameConfig::FlagPlaying);
		q->setGameState(RpgGame::GameStateInit);
	}

	if (!cfg->flags().testFlag(RpgStream::GameConfig::FlagFinished) &&
			config.flags().testFlag(RpgStream::GameConfig::FlagFinished)) {
		LOG_CINFO("game") << "STOP:" << cfg->stage() << "->" << config.stage();
		q->m_gameItem->message(QObject::tr("FINISHED"));

		cfg->flags().setFlag(RpgStream::GameConfig::FlagFinished);
	}

	if (config.stage() > cfg->stage()) {
		LOG_CINFO("game") << "New stage:" << cfg->stage() << "->" << config.stage() << tick;

		q->m_gameItem->onStageChanged(config.stage());

		if (cfg->stage() < RpgStream::GameConfig::StageWarmingUp && config.stage() >= RpgStream::GameConfig::StageWarmingUp) {
			const quint32 serverTick = m_logic->estimatedServerTick();
			LOG_CINFO("game") << "START GAME AT" << serverTick;

			startGame(serverTick);
			q->m_gameItem->overrideCurrentFrame(serverTick);
			m_deadlineTick = cfg->duration();
		}

		if (config.stage() == RpgStream::GameConfig::StageFinished) {
			LOG_CINFO("game") << "STOP******:" << cfg->stage() << "->" << config.stage();
			q->m_gameItem->message(QObject::tr("FINISHED****"));

			finishGame();
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

	q->setColorTeam(RpgGameItem::teamColor().value(q->m_controlledPlayer->team()));
	q->setColorOpponent(RpgGameItem::teamColor().value(q->m_controlledPlayer->team() == RpgStream::TeamA ?
														   RpgStream::TeamB : RpgStream::TeamA));

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


	syncPlayers();
	syncMp();
	syncDefenders();
	syncNpc();
}



/**
 * @brief RpgGamePrivate::syncPlayers
 */

void RpgGamePrivate::syncPlayers()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	Rpg::RpgLogicControlledObjects *controlledObjects = scope.getCtx<Rpg::RpgLogicControlledObjects>();

	auto view = scope.view<Rpg::Player>(entt::exclude<Rpg::LocalIdTag>);

	for (auto entity : view) {
		const RpgStream::PlayerState *state = scope.getCurrentState<RpgStream::PlayerState>(entity);

		const Rpg::Player &p = scope.get<Rpg::Player>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);

		LOG_CWARNING("game") << "CREATE PLAYER" << p.playerData.playerId() << p.playerData.character()
							 << p.playerData.characterResolved(m_characterHash);

		RpgPlayerDefinition def = RpgGame::characters().value(p.playerData.characterResolved(m_characterHash));

		if (def.name.isEmpty()) {
			LOG_CERROR("game") << "Invalid player" << p.playerData.character();
			continue;
		}

		cpVect pos;

		if (state) {
			pos.x = state->entityState().posXAsFloat();
			pos.y = state->entityState().posYAsFloat();
		}

		RpgPlayer *obj = q->m_gameItem->createObject<RpgPlayer>(RpgLogicObjectMapper::toObjectId(p.idTag()), scene,
																q->m_gameItem, pos);

		Q_ASSERT(obj);

		obj->setDisplayName(QString("Player #%1").arg(p.playerData.playerId()));
		obj->load(def);

		if (state) {
			obj->setHp(state->hp());
			obj->setMp(state->mp());
			obj->setBullet(state->bullet());
			obj->setDefender(state->defender(), state->hasDefender());
			obj->setTeam(p.team);

			if (auto ptr = addToScatter(0)) {
				obj->setScatterPoint(ptr.value());
				ptr->scatter->setPointConfiguration(ptr->index, QXYSeries::PointConfiguration::Color,
													RpgGameItem::teamColor().value(p.team));
			}
		}

		const quint32 pid = logicRegisterObject(obj);
		m_logic->addLocalIdTag(entity);

		LOG_CINFO("game") << "ADDED" << pid << "==" << obj->hp() << "HP" << "/" << obj->maxHp() << "MaxHp" << "|" << obj->bullet();

		LOG_CWARNING("game") << "--- check" << (controlledObjects ? controlledObjects->player : 0) << p.idTag();

		if ((controlledObjects && controlledObjects->player == p.idTag()) || q->m_gameMode == RpgGame::SinglePlayer) {
			obj->setSecondaryMotor(std::make_unique<RpgMotorPlayerControlled>(obj));
			q->setControlledPlayer(obj);

			LOG_CWARNING("game") << "***** CONTROLLED" << obj;
		}
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


		LOG_CWARNING("game") << "CREATE MP" << mp.idTag << mp.pos.x << mp.pos.y << "FROM" << mp.origin.x << mp.origin.y;


		RpgMp *obj = q->m_gameItem->createObject<RpgMp>(RpgLogicObjectMapper::toObjectId(mp.idTag), scene,
														q->m_gameItem, mp.origin, mp.pos);

		Q_ASSERT(obj);


		const quint32 pid = logicRegisterObject(obj);

		m_logic->addLocalIdTag(entity);


		LOG_CINFO("game") << "ADDED MP" << RpgLogicObjectMapper::toObjectId(pid).ownerId
						  << RpgLogicObjectMapper::toObjectId(pid).sceneId
						  << RpgLogicObjectMapper::toObjectId(pid).id
						  << "->" << pid << "==" << obj->bodyPositionF();
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


		LOG_CWARNING("game") << "CREATE DEFENDER" << def.idTag << def.pos.x << def.pos.y;


		RpgDefender *obj = q->m_gameItem->createObject<RpgDefender>(RpgLogicObjectMapper::toObjectId(def.idTag),
																	scene, q->m_gameItem,
																	def.pos);

		Q_ASSERT(obj);

		obj->setTeam(def.team);
		obj->setMaxHp(def.maxHp);

		if (const Rpg::Defender *base = scope.try_get<Rpg::Defender>(def.defender)) {
			const Rpg::Tower *tower = scope.try_get<Rpg::Tower>(base->tower);

			for (RpgTower *t : m_towerList) {
				if (RpgLogicObjectMapper::getId(t->objectId()) == tower->idTag) {
					LOG_CERROR("game") << "FOUND TOWER" << t;
					obj->setTower(t);

					for (RpgDefenderPoint *p : t->defenderPoints()) {
						if (RpgLogicObjectMapper::getId(p->objectId()) == base->idTag) {
							LOG_CERROR("game") << "FOUND BASE" << p;
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

		const quint32 pid = logicRegisterObject(obj);

		m_logic->addLocalIdTag(entity);


		LOG_CINFO("game") << "ADDED DEFENDER" << RpgLogicObjectMapper::toObjectId(pid).ownerId
						  << RpgLogicObjectMapper::toObjectId(pid).sceneId
						  << RpgLogicObjectMapper::toObjectId(pid).id
						  << "->" << pid << "==" << obj->bodyPositionF();


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

	auto view = scope.view<Rpg::Npc>(entt::exclude<Rpg::LocalIdTag>);

	for (auto entity : view) {
		const RpgStream::NpcState *state = scope.getCurrentState<RpgStream::NpcState>(entity);

		const Rpg::Npc &p = scope.get<Rpg::Npc>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);

		LOG_CWARNING("game") << "CREATE NPC" << p.idTag << p.data.character()
							 << p.data.characterResolved(m_npcHash);

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

		RpgNpc *obj = RpgNpc::createNpc(p, q->m_gameItem, scene, pos);

		Q_ASSERT(obj);

		obj->setDisplayName(QString("NPC #%1").arg(p.idTag));
		obj->load(def);

		if (state) {
			obj->setHp(state->hp());
			obj->setTeam(p.data.team());

			if (auto ptr = addToScatter(2)) {
				obj->setScatterPoint(ptr.value());
				ptr->scatter->setPointConfiguration(ptr->index, QXYSeries::PointConfiguration::Color,
													RpgGameItem::teamColor().value(p.data.team()));
			}
		}

		const quint32 pid = logicRegisterObject(obj);
		m_logic->addLocalIdTag(entity);

		LOG_CINFO("game") << "ADDED" << pid << "==" << obj->hp() << "HP" << "/" << obj->maxHp() << "MaxHp";

		LOG_CWARNING("game") << "--- check" << (controlledObjects ? controlledObjects->entities.size() : 0) << p.idTag;

		if ((controlledObjects && controlledObjects->entities.contains(p.idTag)) || q->m_gameMode == RpgGame::SinglePlayer) {
			obj->setSecondaryMotor(obj->getControlledMotor());

			LOG_CWARNING("game") << "***** CONTROLLED NPC" << obj << obj->secondaryMotor();
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

		LOG_CINFO("game") << "########### EVENT" << event.tick() << "### CURR" << tick << "### LAST" << m_lastProcessedEventTick;

		// Mp emitted

		if (event.flags().testFlag(RpgStream::Events::Emitter))
			processEvents(event.emitter());


		// Players events

		if (event.flags().testFlag(RpgStream::Events::Player))
			processEvents(event.player());


		// Npc events

		if (event.flags().testFlag(RpgStream::Events::Npc))
			processEvents(event.npc());



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

	LOG_CWARNING("game") << "GAME STATE CHANGED" << m_gameState << "->" << newGameState;

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

RpgGamePrivate::RpgGamePrivate(RpgGame *game, const bool &multi)
	: QObject()
	, q(game)
{
	if (multi)
		m_logic = std::make_unique<Rpg::RpgLogicClientMulti>();
	else
		m_logic = std::make_unique<Rpg::RpgLogicClientSingle>();


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
	LOG_CDEBUG("game") << "DELETE PRIVATE" << this;

	if (Rpg::RpgLogicClientMulti *l = dynamic_cast<Rpg::RpgLogicClientMulti*>(m_logic.get())) {
		l->setEngine(nullptr);
	}

	m_engine.reset();
	LOG_CDEBUG("game") << "ENGINE RESET" << this;

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
	LOG_CINFO("client") << "GAME CONNECTED";

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
	LOG_CINFO("game") << "CONNECTED";

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
	LOG_CERROR("game") << "FAILED" << err;

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
 * @brief RpgGamePrivate::contentPrepare
 */

void RpgGamePrivate::contentPrepare()
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


	return cfg;
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

QColor RpgGame::colorTeam() const
{
	return m_colorTeam;
}

void RpgGame::setColorTeam(const QColor &newColorTeam)
{
	if (m_colorTeam == newColorTeam)
		return;
	m_colorTeam = newColorTeam;
	emit colorTeamChanged();
}

QColor RpgGame::colorOpponent() const
{
	return m_colorOpponent;
}

void RpgGame::setColorOpponent(const QColor &newColorOpponent)
{
	if (m_colorOpponent == newColorOpponent)
		return;
	m_colorOpponent = newColorOpponent;
	emit colorOpponentChanged();
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
