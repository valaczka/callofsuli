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
#include "rpgplayer.h"
#include "rpgstream.h"
#include "rpguserwallet.h"
#include "tiledgame.h"
#include "rpggame.h"
#include "rpggame_p.h"
#include "rpggameitem.h"
#include "rpgmp.h"
#include "Logger.h"
#include "client.h"
#include "downloader.h"
#include "utils_.h"




QHash<QString, RpgGameDefinition> RpgGame::m_terrains = {};


/**
 * @brief RpgGame::RpgGame
 * @param missionLevel
 * @param client
 */

RpgGame::RpgGame(GameMapMissionLevel *missionLevel, Client *client, const bool &multiplayer)
	: AbstractLevelGame(GameMap::Rpg, missionLevel, client)
	, d(new RpgGamePrivate(this, multiplayer))
	, m_gameMode(multiplayer ? MultiPlayerGuest : SinglePlayer)
{
	Q_ASSERT(client);

	LOG_CDEBUG("client") << "RpgGame created" << this;

	connect(client->downloader(), &Downloader::contentDownloaded, d, &RpgGamePrivate::onContentDownloaded);
	connect(client->downloader(), &Downloader::downloadError, d, &RpgGamePrivate::onContentError);

}


/**
 * @brief RpgGame::~RpgGame
 */

RpgGame::~RpgGame()
{
	d->clearSharedTextures();
	d->deleteLater();

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
			 m_gameState == GameStateDownloadStatic ||
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

	d->prepareGameItem();
}



/**
 * @brief RpgGame::gameItemPrepared
 */

void RpgGame::gameItemPrepared()
{
	LOG_CDEBUG("game") << "GameItem prepared";

	d->onGameItemPrepared();
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

	d->connectionPrepare();

	return item;

}


/**
 * @brief RpgGame::connectGameQuestion
 */

void RpgGame::connectGameQuestion()
{

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
	if (q->m_gameState == RpgGame::GameStateDownloadStatic) {
		q->setGameState(RpgGame::GameStateLobby);
	} else {
		LOG_CERROR("client") << "Invalid state" << q->m_gameState;
		q->setError(tr("Ismeretlen hiba: onContentDownloaded"));
	}
}


/**
 * @brief RpgGame::onContentError
 */

void RpgGamePrivate::onContentError()
{
	q->setError(tr("Sikertelen letöltés"));
}



/**
 * @brief RpgGamePrivate::prepareGameItem
 */

void RpgGamePrivate::prepareGameItem()
{
	Q_ASSERT(!RpgGame::terrains().isEmpty());


	Rpg::RpgLogicScope scope = m_logic->getScope();
	RpgStream::GameConfig *cfg = scope.getCtx<RpgStream::GameConfig>();

	Q_ASSERT(cfg);

	const QString &terrain = m_terrainHash.value(cfg->terrain());


	const RpgGameDefinition def = RpgGame::terrains().value(terrain);

	LOG_CDEBUG("game") << "LOAD" << cfg->terrain() << terrain << def.name;

	if (q->load(def)) {
		q->m_gameItem->setIsContentReady(true);
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

	m_logic->loadMapData(m_mapData);

	logicAddPlayer(Rpg::TeamTag::TeamA, 2);

	{
	Rpg::RpgLogicScope scope = m_logic->getScope();
	m_deadlineTick = scope.getCtx<RpgStream::GameConfig>()->duration();
	}


	m_logic->emplacePlayers();

	syncObjects();

	/// TODO...
	///q->setGameState(RpgGame::GameStateInit);

	startGame();
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

void RpgGamePrivate::playerPositionAdd(const QPointF &pos, const Rpg::TeamTag::Team &team)
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
 * @brief RpgGamePrivate::connectJoysticks
 */

void RpgGamePrivate::connectJoysticks()
{
	if (!q->m_gameItem)
		return;


	const QList<QPair<QQuickItem*, const char*> > list = {
		{ q->m_gameItem->joystickA(), "joystickClickedA()" },
		{ q->m_gameItem->joystickB(), "joystickClickedB()" },
		{ q->m_gameItem->joystickC(), "joystickClickedC()" },
		{ q->m_gameItem->joystickD(), "joystickClickedD()" },
	};


	const QMetaObject *moThis = this->metaObject();


	for (const auto &p : list) {
		if (!p.first)
			continue;

		const QMetaObject *mo = p.first->metaObject();

		const int methodIndex = mo->indexOfMethod("clicked()");

		if (methodIndex < 0)
			continue;

		connect(p.first, mo->method(methodIndex), this, moThis->method(moThis->indexOfMethod(p.second)));
	}

}



/**
 * @brief RpgGamePrivate::joystickClicked
 * @param joystick
 */

void RpgGamePrivate::joystickClickedA()
{
	LOG_CINFO("game") << "CLICKED";
}

void RpgGamePrivate::joystickClickedB()
{

}

void RpgGamePrivate::joystickClickedC()
{

}

void RpgGamePrivate::joystickClickedD()
{

}



/**
 * @brief RpgGamePrivate::startGame
 */

void RpgGamePrivate::startGame()
{
	LOG_CINFO("game") << "START GAME";

	q->setGameState(RpgGame::GameStatePlay);
	q->m_gameItem->tickTimer()->start(q);
}




/**
 * @brief RpgGamePrivate::onBeforeWorldStep
 */

void RpgGamePrivate::onBeforeWorldStep()
{
	if (q->m_gameMode == RpgGame::SinglePlayer) {
		m_logic->render();
		//RpgStream::FullState full = m_logic->getFullState(1);
	}

	syncObjects();
}




/**
 * @brief RpgGamePrivate::onAfterWorldStep
 * @param full
 */

void RpgGamePrivate::onAfterWorldStep(const RpgStream::FullState &full)
{
	if (q->m_gameMode == RpgGame::SinglePlayer) {
		m_logic->fullStateLoad(full, {});
	}
}




/**
 * @brief RpgGamePrivate::syncObjects
 */

void RpgGamePrivate::syncObjects()
{
	syncPlayers();
	syncMp();

	syncDeleted();
}



/**
 * @brief RpgGamePrivate::syncPlayers
 */

void RpgGamePrivate::syncPlayers()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	auto view = scope.view<Rpg::Player>(entt::exclude<Rpg::IdTag>);

	for (auto entity : view) {
		const RpgStream::PlayerState *state = scope.getCurrentState<RpgStream::PlayerState>(entity);

		const Rpg::Player &p = scope.get<Rpg::Player>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);

		LOG_CWARNING("game") << "CREATE PLAYER" << p.playerData.playerId() << p.playerData.character()
							 << p.playerData.characterResolved(m_characterHash);

		cpVect pos;

		if (state) {
			pos.x = state->entityState().posXAsFloat();
			pos.y = state->entityState().posYAsFloat();
		}

		RpgPlayer *obj = q->m_gameItem->createObject<RpgPlayer>(RpgLogicObjectMapper::toObjectId(p.idTag()), scene,
																q->m_gameItem, pos);

		Q_ASSERT(obj);

		if (state) {
			obj->setHp(state->hp());
			//obj->setMaxHp(state->hp());
		}

		const quint32 pid = logicRegisterObject(obj);

		m_logic->entitySetIdTag(entity, pid);

		LOG_CINFO("game") << "ADDED" << RpgLogicObjectMapper::toObjectId(pid).ownerId
						  << RpgLogicObjectMapper::toObjectId(pid).sceneId
						  << RpgLogicObjectMapper::toObjectId(pid).id
						  << "->" << pid << "==" << obj->bodyPositionF() << "|" << obj->hp() << "HP" << "/" << obj->maxHp() << "MaxHp";


		if (p.playerData.playerId() == 1) {
			QSizeF s = scope.getCtx<Rpg::ChunkGrid>()->chunkSize;
			LOG_CWARNING("game") << "***** CONTROLLED" << obj << s;

			obj->setSecondaryMotor(std::make_unique<RpgMotorPlayerControlled>(obj));
			q->setControlledPlayer(obj);

			obj->setChunkRadius(std::max(s.width(), s.height()) * 0.75);
		}
	}
}




/**
 * @brief RpgGamePrivate::syncMp
 */

void RpgGamePrivate::syncMp()
{
	Rpg::RpgLogicScope scope = m_logic->getScope();

	auto view = scope.view<Rpg::Mp>(entt::exclude<Rpg::IdTag>);

	for (auto entity : view) {
		const Rpg::Mp &mp = scope.get<Rpg::Mp>(entity);

		TiledScene *scene = q->m_gameItem->currentScene();

		Q_ASSERT(scene);

		cpVect fromPos = cpvzero;

		if (Rpg::MpEmitter *emitter = scope.try_get<Rpg::MpEmitter>(mp.emitter)) {
			fromPos = emitter->pos;
		}

		LOG_CWARNING("game") << "CREATE MP" << mp.idTag << mp.pos.x << mp.pos.y << "FROM" << fromPos.x << fromPos.y;


		RpgMp *obj = q->m_gameItem->createObject<RpgMp>(RpgLogicObjectMapper::toObjectId(mp.idTag), scene,
														q->m_gameItem, fromPos, mp.pos);

		Q_ASSERT(obj);


		const quint32 pid = logicRegisterObject(obj);

		m_logic->entitySetIdTag(entity, pid);

		LOG_CINFO("game") << "ADDED MP" << RpgLogicObjectMapper::toObjectId(pid).ownerId
						  << RpgLogicObjectMapper::toObjectId(pid).sceneId
						  << RpgLogicObjectMapper::toObjectId(pid).id
						  << "->" << pid << "==" << obj->bodyPositionF();
	}
}




/**
 * @brief RpgGamePrivate::syncDeleted
 */

void RpgGamePrivate::syncDeleted()
{
	// Delete tag lenne jó vagy ilyesmi már a fullstate betöltése után

	Rpg::RpgLogicScope scope = m_logic->getScope();

	RpgLogicObjectMapper *mapper = scope.getCtx<RpgLogicObjectMapper>();

	auto view = scope.view<Rpg::IdTag>();

	for (auto it = mapper->map.cbegin(); it != mapper->map.cend(); ) {
		if (qobject_cast<RpgPlayer*>(it.value())) {
			++it;
			continue;
		}

		bool found = false;
		for (const auto &e : view) {
			if (scope.get<Rpg::IdTag>(e).id == it.key()) {
				found = true;
				break;
			}
		}

		if (found) {
			++it;
			continue;
		}

		LOG_CERROR("game") << "DELETE" << it.value() << it.key() <<
							  q->m_gameItem->removeObject(it.value());

		it = mapper->map.erase(it);
	}

}



/**
 * @brief RpgGamePrivate::onTimeStepped
 */

void RpgGamePrivate::onTimeStepped()
{
	syncChunkMarker();
}



/**
 * @brief RpgGamePrivate::syncChunkMarker
 */

void RpgGamePrivate::syncChunkMarker()
{
	if (!m_chunkMarkerLayer)
		return;

	if (!q->m_controlledPlayer) {
		m_chunkMarkerLayer->setVisible(false);
		return;
	}

	const QPointF p = q->m_controlledPlayer->currentChunkCenter();

	if (p.x() < 0 || p.y() < 0) {
		m_chunkMarkerLayer->setVisible(false);
		return;
	}

	m_chunkMarkerLayer->setPosition(m_chunkMarkerBaseOffset + p);
	m_chunkMarkerLayer->setVisible(true);

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
 * @brief RpgGamePrivate::logicAddPlayer
 * @param team
 * @return
 */

void RpgGamePrivate::logicAddPlayer(const Rpg::TeamTag::Team &team, const int &count)
{
	for (int i=0; i<count; ++i)
		m_logic->playerAdd(team);
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
	if (!multi)
		m_logic = std::make_unique<Rpg::RpgLogicClientMulti>();
	else
		m_logic = std::make_unique<Rpg::RpgLogicClientSingle>();
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
 * @brief RpgGamePrivate::connectionPrepare
 */

void RpgGamePrivate::connectionPrepare()
{
	if (q->m_client->server()) {
		connectionCheck();
		return;
	}

	q->setGameState(RpgGame::GameStateConnect);

	connect(q->m_client, &Client::serverChanged, this, &RpgGamePrivate::connectionCheck);

}




/**
 * @brief RpgGamePrivate::connectionCheck
 */

void RpgGamePrivate::connectionCheck()
{
	if (!q->m_client->server())
		return;


	LOG_CINFO("client") << "DOWNLOAD" << q->m_client->server()->availableContent().size();

	/// TODO: REMOVE ..................................

	q->setGameState(RpgGame::GameStateDownloadStatic);

	q->m_client->server()->loadDynamicContent("/home/valaczka/Projektek/callofsuli-content/test.cres");
	q->m_client->server()->loadDynamicContent("/home/valaczka/Projektek/callofsuli-content/character01a.cres");
	q->m_client->server()->loadDynamicContent("/home/valaczka/Projektek/callofsuli-content/rpg.cres");

	QDirIterator it(QStringLiteral("/home/valaczka/Projektek/callofsuli-content"),
					{QStringLiteral("*.dres")}, QDir::Files);

	while (it.hasNext())
		q->m_client->server()->loadDynamicContent(it.next());


	q->reloadTerrains();

	if (RpgGame::terrains().empty()) {
		q->setError("NINCS TEREP");
		return;
	}

	Rpg::RpgLogicScope scope = m_logic->getScope();
	scope.getCtx<RpgStream::GameConfig>()->setTerrainResolved("test");
	scope.getCtx<RpgStream::GameConfig>()->setDuration(150*60);

	q->setGameState(RpgGame::GameStatePrepare);

	return;

	/// __________________________


	if (!q->m_client->server()->availableContent().isEmpty()) {
		Downloader *downloader = q->m_client->downloader();

		Q_ASSERT(downloader);

		downloader->contentClear();
		downloader->setServer(q->m_client->server());

		for (const Server::DynamicContent &c : q->m_client->server()->availableContent())
			downloader->contentAdd(c);

		q->setGameState(RpgGame::GameStateDownloadStatic);

		downloader->download();

		return;
	}

	q->setGameState(RpgGame::GameStateConnect);

	connect(q->m_client->server(), &Server::availableContentChanged, this, &RpgGamePrivate::connectionCheck);
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
