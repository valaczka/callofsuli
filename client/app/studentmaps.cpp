/*
 * ---- Call of Suli ----
 *
 * studentmaps.cpp
 *
 * Created on: 2020. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMaps
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "studentmaps.h"
#include <QQmlEngine>
#include "mapimage.h"
#include "teachermaps.h"
#include "cosmessage.h"

StudentMaps::StudentMaps(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassStudent, parent)
	, m_demoMapFile(":/internal/game/demo.map")
	, m_modelMapList(new ObjectGenericListModel<MapListObject>(this))
	, m_currentMap(nullptr)
	, m_demoMode(false)
	, m_demoSolverMap()
	, m_baseXP(100)
	, m_selectedGroupId(-1)
	, m_missionNameMap()
	, m_liteMode(false)
{
	connect(this, &StudentMaps::mapListGet, this, &StudentMaps::onMapListGet);
	connect(this, &StudentMaps::missionListGet, this, &StudentMaps::onMissionListGet);
	connect(this, &StudentMaps::gameCreate, this, &StudentMaps::onGameCreate);
	connect(this, &StudentMaps::gameFinish, this, &StudentMaps::onGameFinish);

	connect(this, &StudentMaps::gameListUserGet, this, &StudentMaps::onGameListUserGet);
	connect(this, &StudentMaps::gameListCampaignGet, this, &StudentMaps::onGameListUserGet);
	connect(this, &StudentMaps::campaignGet, this, &StudentMaps::onCampaignGet);
}





/**
 * @brief StudentMaps::~StudentMaps
 */

StudentMaps::~StudentMaps()
{
	if (m_demoMode) {
		QJsonDocument doc(QJsonObject::fromVariantMap(m_demoSolverMap));
		Client::saveJsonDocument(doc, Client::standardPath("demomap.json"));
	}

	delete m_modelMapList;

	unloadGameMap();

	if (m_downloader)
		delete m_downloader;
}






/**
 * @brief StudentMaps::studentMapsDb
 * @param client
 * @param parent
 * @param connectionName
 * @return
 */

CosDb *StudentMaps::studentMapsDb(Client *client, QObject *parent, const QString &connectionName)
{
	if (!client || client->serverDataDir().isEmpty())
		return nullptr;

	QString dbname = client->serverDataDir()+"/studentmaps.db";

	CosDb *db = new CosDb(connectionName, parent);
	db->setDatabaseName(dbname);

	if (!db->open()) {
		qWarning() << "Can't open database" << dbname;
		delete db;
		return nullptr;
	}

	QVariantList tables = db->execSelectQuery("SELECT name FROM sqlite_master WHERE type ='table' AND name='maps'");

	if (tables.isEmpty()) {
		qInfo() << tr("A pályaadatbázis üres, előkészítem.");

		if (!db->execSimpleQuery("CREATE TABLE maps("
								 "uuid TEXT NOT NULL PRIMARY KEY,"
								 "data BLOB NOT NULL"
								 ")")) {
			qWarning() << tr("Nem sikerült előkészíteni az adatbázist:") << dbname;
			db->close();
			delete db;
			return nullptr;
		}
	}

	return db;
}


/**
 * @brief StudentMaps::init
 * @param demoMode
 */

void StudentMaps::init(const bool &demoMode, const QString &fileToOpen)
{
	m_demoMode = demoMode;
	emit demoModeChanged(m_demoMode);

	if (!m_demoMode) {
		CosDb *db = studentMapsDb(Client::clientInstance(), this);
		if (db)
			addDb(db, false);
	} else {
		if (!fileToOpen.isEmpty())
			m_demoMapFile = fileToOpen;

		demoMapLoad();
	}
}





/**
 * @brief StudentMaps::gameMapLoad
 * @param data
 */

void StudentMaps::mapLoad(MapListObject *map)
{
	if (m_demoMode)
		return;

	if (!map) {
		unloadGameMap();
		return;
	}

	QVariantMap r = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", {map->uuid()});

	if (r.isEmpty()) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen pályaazonosító!"), map->uuid());
		return;
	}

	QByteArray b = r.value("data").toByteArray();

	GameMap *gmap = GameMap::fromBinaryData(b);

	if (!gmap) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hibás pályaadatok!"), map->uuid());
		return;
	}

	GameMapMissionIface *merror = gmap->checkLockTree();

	if (merror) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hibás zárolás!"));
		return;
	}

	if (!StudentMaps::checkTerrains(gmap)) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Nem létező harcmező!"));
		return;
	}

	if (gmap->appVersion() > 0 && gmap->appVersion() > CosMessage::versionNumber()) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/update.svg", tr("Frissítés szükséges"), tr("A pálya az alkalmazásnál magasabb verziószámmal készült, elképzelhető, hogy nem minden funkció fog helyesen működni.\nFrissítsd az alkalmazást a legfrissebb verzióra!"));
	}


	if (!loadGameMap(gmap, map))
		delete gmap;
}


/**
 * @brief StudentMaps::demoMapLoad
 */

void StudentMaps::demoMapLoad()
{
	if (!QFile::exists(m_demoMapFile)) {
		qWarning() << m_demoMapFile << "doesn't exists";
		unloadGameMap();
		return;
	}

	qDebug() << "Load demo map" << m_demoMapFile;

	MapListObject *demoMapObject = new MapListObject(this);
	demoMapObject->setActive(true);
	demoMapObject->setName(tr("Demo pálya"));

	m_demoSolverMap = Client::readJsonDocument(Client::standardPath("demomap.json")).object().toVariantMap();


	QFile f(m_demoMapFile);
	if (f.open(QIODevice::ReadOnly)) {

		QByteArray b = f.readAll();

		f.close();

		GameMap *map = GameMap::fromBinaryData(b);

		if (!map) {
			Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hibás pályaadatok!"));
			return;
		}

		GameMapMissionIface *merror = map->checkLockTree();

		if (merror) {
			Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hibás zárolás!"));
			return;
		}

		if (!StudentMaps::checkTerrains(map)) {
			Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Nem létező harcmező!"));
			return;
		}

		if (!loadGameMap(map, demoMapObject))
			delete map;
	}
}



/**
 * @brief StudentMaps::getMissionList
 */

void StudentMaps::getMissionList()
{
	if (!m_currentMap) {
		return;
	}

	if (m_demoMode) {
		QJsonObject o;

		QJsonArray list;

		QMapIterator<QString, QVariant> it(m_demoSolverMap);

		while (it.hasNext()) {
			it.next();
			QVariantMap m = it.value().toMap();

			QJsonObject oo;
			oo["missionid"] = it.key();
			oo["t1"] = m.value("t1", 0).toInt();
			oo["t2"] = m.value("t2", 0).toInt();
			oo["t3"] = m.value("t3", 0).toInt();
			oo["d1"] = m.value("d1", 0).toInt();
			oo["d2"] = m.value("d2", 0).toInt();
			oo["d3"] = m.value("d3", 0).toInt();
			list.append(oo);
		}

		o["list"] = list;
		onMissionListGet(o, QByteArray());
		return;
	}

	QJsonObject o;
	o["map"] = QString(m_currentMap->uuid());
	send("missionListGet", o);
}





/**
 * @brief StudentMaps::getLevelInfo
 * @param uuid
 * @param level
 * @param deathmatch
 */

void StudentMaps::getLevelInfo(const QString &uuid, const int &level, const bool &deathmatch)
{
	if (!m_currentMap) {
		return;
	}

	GameMapMissionLevel *missionLevel = m_currentMap->missionLevel(uuid, level);

	if (!missionLevel) {
		qWarning() << "Invalid mission level!";
		return;
	}

	GameMapMission *mission = missionLevel->mission();

	if (!mission) {
		qWarning() << "Invalid mission!";
		return;
	}

	if (deathmatch && !missionLevel->canDeathmatch()) {
		qWarning() << "Mission level missing deathmatch mode!";
		return;
	}

	QVariantMap ret;

	ret["name"] = mission->name();
	ret["description"] = mission->description();


	const int lMin = mission->lockDepth() == 0 ?
						 qMax(mission->solvedLevel()+1, 1) :
						 -1;

	if (deathmatch)
		ret["available"] = (mission->solvedLevel() >= missionLevel->level() && missionLevel->level() <= lMin && !m_liteMode);
	else
		ret["available"] = (missionLevel->level() <= lMin);


	ret["enemies"] = Client::terrain(missionLevel->terrain(), level).enemies;
	ret["hp"] = missionLevel->startHP();
	ret["duration"] = missionLevel->duration();


	emit levelInfoReady(ret);
}








/**
 * @brief StudentMaps::playGame
 * @param data
 */

void StudentMaps::playGame(const QString &uuid, const int &level, const bool &deathmatch)
{
	if (!m_currentMap)
		return;

	if (m_demoMode) {
		QJsonObject o;
		o["gameid"] = -1;
		o["missionid"] = uuid;
		o["level"] = level;
		o["deathmatch"] = deathmatch;
		o["lite"] = m_liteMode;
		onGameCreate(o, QByteArray());
		return;
	}

	QJsonObject o;
	o["map"] = QString(m_currentMap->uuid());
	o["mission"] = uuid;
	o["level"] = level;
	o["deathmatch"] = deathmatch;
	o["lite"] = m_liteMode;

	send("gameCreate", o);
}





void StudentMaps::setBaseXP(int baseXP)
{
	if (m_baseXP == baseXP)
		return;

	m_baseXP = baseXP;
	emit baseXPChanged(m_baseXP);
}

void StudentMaps::setSelectedGroupId(int selectedGroupId)
{
	if (m_selectedGroupId == selectedGroupId)
		return;

	m_selectedGroupId = selectedGroupId;
	emit selectedGroupIdChanged(m_selectedGroupId);
}



/**
 * @brief StudentMaps::mapDownload
 * @param data
 */

void StudentMaps::mapDownload(MapListObject *map)
{
	if (!map)
		return;

	if (m_demoMode) {
		return;
	}

	if (!m_downloader)
		_createDownloader();

	m_downloader->clear();

	m_downloader->append(map->uuid(),
						 "",
						 map->dataSize(),
						 map->md5(),
						 false,
						 0.0);


	if (m_downloader->hasDownloadable()) {
		emit mapDownloadRequest(Client::formattedDataSize(m_downloader->fullSize()));
	} else {
		QJsonObject o;
		o["groupid"] = m_selectedGroupId;
		send("mapListGet", o);
	}
}



/**
 * @brief StudentMaps::mapDownload
 * @param list
 */

void StudentMaps::mapDownload(QList<QObject *> list)
{
	if (m_demoMode) {
		return;
	}

	if (!m_downloader)
		_createDownloader();

	m_downloader->clear();


	foreach (QObject *o, list) {
		MapListObject *map = qobject_cast<MapListObject*>(o);
		if (!map)
			continue;
		m_downloader->append(map->uuid(),
							 "",
							 map->dataSize(),
							 map->md5(),
							 false,
							 0.0);
	}

	if (m_downloader->hasDownloadable()) {
		emit mapDownloadRequest(Client::formattedDataSize(m_downloader->fullSize()));
	} else {
		QJsonObject o;
		o["groupid"] = m_selectedGroupId;
		send("mapListGet", o);
	}
}







/**
 * @brief StudentMaps::onMapListGet
 * @param jsonData
 */

void StudentMaps::onMapListGet(QJsonObject jsonData, QByteArray)
{
	if (m_demoMode)
		return;

	m_modelMapList->unselectAll();

	QJsonArray list = jsonData.value("list").toArray();
	QJsonArray retList;

	foreach (QJsonValue v, list) {
		QJsonObject m = v.toObject();
		QString uuid = m.value("uuid").toString();

		m["downloaded"] = false;

		QVariantMap r = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", {uuid});

		QByteArray d = r.value("data").toByteArray();

		if (!r.isEmpty() && !d.isEmpty() &&
			m.value("md5").toString() == QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex()) &&
			m.value("dataSize").toInt() == d.size()) {
			m["downloaded"] = true;
		}

		retList.append(m);
	}

	m_modelMapList->updateJsonArray(retList, "uuid");
}






/**
 * @brief StudentMaps::onOneDownloadFinished
 * @param item
 * @param data
 * @param jsonData
 */

void StudentMaps::onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &)
{
	if (m_demoMode)
		return;

	QVariantMap m;
	m["uuid"] = item.remoteFile;
	m["data"] = data;

	db()->execInsertQuery("INSERT OR REPLACE INTO maps (?k?) VALUES (?)", m);
}


/**
 * @brief StudentMaps::setGameMap
 * @param map
 */

bool StudentMaps::loadGameMap(GameMap *map, MapListObject *mapObject)
{
	unloadGameMap();

	if (!map)
		return false;

	m_currentMap = map;
	emit gameMapLoaded(mapObject);


	qDebug() << "Add mapimage provider";
	MapImage *mapImage = new MapImage(map);
	Client::clientInstance()->rootEngine()->addImageProvider("mapimage", mapImage);

	return true;
}





/**
 * @brief StudentMaps::unsetGameMap
 */

void StudentMaps::unloadGameMap()
{
	QQmlEngine *engine = Client::clientInstance()->rootEngine();

	if (!engine) {
		qWarning() << "Invalid engine" << this;
	} else if (engine->imageProvider("mapimage")) {
		qDebug() << "Remove image provider mapimage";
		engine->removeImageProvider("mapimage");
	}

	if (m_currentMap) {
		delete m_currentMap;

		m_currentMap = nullptr;
		emit gameMapUnloaded();
	}
}




/**
 * @brief StudentMaps::onDemoMapWin
 */

void StudentMaps::onDemoGameWin()
{
	GameMatch *match = qobject_cast<GameMatch *>(sender());

	Q_ASSERT(match);

	QString uuid = match->missionUuid();

	GameMap::SolverInfo oldSolver = m_demoSolverMap.value(uuid).toMap();
	int solvedXP = GameMap::computeSolvedXpFactor(oldSolver, match->level(), match->deathmatch(), match->mode() == GameMatch::ModeLite) * m_baseXP;

	GameMap::SolverInfo newSolver = oldSolver.solve(match->level(), match->deathmatch());
	m_demoSolverMap[uuid] = newSolver.toVariantMap();


	QJsonObject o;
	o["finished"] = true;
	o["success"] = true;
	o["missionid"] = uuid;
	o["level"] = match->level();
	o["deathmatch"] = match->deathmatch();
	o["lite"] = m_liteMode;
	o["solvedCount"] = newSolver.solved(match->level(), match->deathmatch());
	o["flawless"] = match->isFlawless();

	o["xp"] = QJsonObject::fromVariantMap({
											  { "game", match->xp() },
											  { "solved", solvedXP }
										  });

	o["duration"] = match->elapsedTime();


	onGameFinish(o, QByteArray());

}


/**
 * @brief StudentMaps::onGameLose
 * @param uuid
 * @param level
 */

void StudentMaps::onGameEnd(GameMatch *match, const bool &win)
{
	QJsonObject o;
	o["id"] = match->gameId();
	o["xp"] = match->xp();
	o["success"] = win;
	o["duration"] = match->elapsedTime();
	o["stat"] = match->takeStatistics();
	o["flawless"] = match->isFlawless();
	Client::clientInstance()->socketSend(CosMessage::ClassStudent, "gameFinish", o);
}




/**
 * @brief StudentMaps::onMissionListGet
 * @param jsonData
 */


void StudentMaps::onMissionListGet(QJsonObject jsonData, QByteArray)
{
	QJsonArray list = jsonData.value("list").toArray();
	int baseXP = jsonData.value("baseXP").toInt();

	if (baseXP > 0)
		setBaseXP(baseXP);

	if (!m_currentMap || (!m_demoMode && m_currentMap->uuid() != jsonData.value("uuid").toString())) {
		qWarning() << "Missing current map or invalid uuid";
		return;
	}


	GameMapMissionIface *merror = m_currentMap->checkLockTree();

	if (merror) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hibás pálya!"));
		return;
	}

	m_currentMap->setSolver(list.toVariantList());

	QHash<QString, GameMap::SolverInfo> solvers;

	foreach (QVariant v, list.toVariantList()) {
		QVariantMap m = v.toMap();
		QString id = m.value("missionid").toString();

		if (!id.isEmpty())
			solvers.insert(id, GameMap::SolverInfo(m));

	}



	QVariantList ret;

	int num = 1;


	foreach (GameMapMission *mis, m_currentMap->missions()) {
		if (mis->lockDepth() > 1)
			continue;

		GameMap::SolverInfo info = solvers.value(mis->uuid(), GameMap::SolverInfo(0,0,0,0,0,0));


		QVariantMap m;
		m["num"] = num++;
		m["lockDepth"] = mis->lockDepth();
		m["uuid"] = QString(mis->uuid());
		m["name"] = mis->name();
		m["description"] = mis->description();
		m["medalImage"] = mis->medalImage();

		const int lMin = mis->lockDepth() == 0 ?
							 qMax(mis->solvedLevel()+1, 1) :
							 -1;

		bool isFullSolved = true;

		QVariantList levelList;
		foreach (GameMapMissionLevel *ml, mis->levels()) {
			QVariantMap mm;
			mm["level"] = ml->level();

			if (!ml->solvedNormal() || (ml->canDeathmatch() && !ml->solvedDeathmatch()))
				isFullSolved = false;

			{
				QVariantMap mm2 = mm;
				int xp = m_baseXP * GameMap::computeSolvedXpFactor(info, ml->level(), false, m_liteMode);
				mm2["deathmatch"] = false;
				mm2["available"] = (ml->level() <= lMin);
				mm2["xp"] = xp;
				mm2["solved"] = ml->solvedNormal();
				levelList.append(mm2);
			}

			if (ml->canDeathmatch()) {
				QVariantMap mm2 = mm;
				int xp = m_baseXP * GameMap::computeSolvedXpFactor(info, ml->level(), true, m_liteMode);


				mm2["deathmatch"] = true;
				mm2["available"] = (mis->solvedLevel() >= ml->level() && ml->level() <= lMin && !m_liteMode);
				mm2["xp"] = xp;
				mm2["solved"] = ml->solvedDeathmatch();
				levelList.append(mm2);
			}

		}


		/*if (!m.contains("backgroundImage")) {
			QString imageFolder = ml->imageFolder();
			QString imageFile = ml->imageFile();

			if (!imageFolder.isEmpty() && !imageFile.isEmpty())
				m["backgroundImage"] = "image://mapimagedb/"+imageFolder+"/"+imageFile;
			else
				m["backgroundImage"] = "qrc:/internal/game/bg.png";
		}*/

		m["fullSolved"] = isFullSolved;
		m["levels"] = levelList;

		ret.append(m);
	}

	emit solvedMissionListReady(ret);

}









/**
 * @brief StudentMaps::onGameCreate
 * @param jsonData
 */

void StudentMaps::onGameCreate(QJsonObject jsonData, QByteArray)
{
	GameMapMissionLevel *missionLevel = nullptr;

	if (m_currentMap)
		missionLevel = m_currentMap->missionLevel(jsonData.value("missionid").toString(),
												  jsonData.value("level").toInt());

	int gameId = jsonData.value("gameid").toInt();
	bool deathmatch = jsonData.value("deathmatch").toBool();
	GameMatch::GameMode mode = jsonData.value("lite").toBool() ? GameMatch::ModeLite : GameMatch::ModeNormal;

	qDebug() << "GAME CREATE" << m_currentMap << missionLevel << gameId << mode;

	if (!m_currentMap || !missionLevel) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Pályaadatok nem elérhetőek!"));

		if (m_demoMode) {
			return;
		}

		QJsonObject o;
		o["id"] = gameId;
		o["xp"] = 0;
		o["success"] = false;
		send("gameFinish", o);
		return;
	}


	GameMatch *m_gameMatch = new GameMatch(missionLevel, m_currentMap, this);

	QString err;
	if (!m_gameMatch->check(&err)) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), err);

		delete m_gameMatch;

		if (m_demoMode) {
			return;
		}

		QJsonObject o;
		o["id"] = gameId;
		o["xp"] = 0;
		o["success"] = false;
		send("gameFinish", o);
		return;
	}


	m_gameMatch->setDeleteGameMap(false);
	m_gameMatch->setImageDbName("mapimage");
	m_gameMatch->setGameId(gameId);
	m_gameMatch->setBaseXP(m_baseXP*XP_FACTOR_TARGET_BASE);
	m_gameMatch->setDeathmatch(deathmatch);
	m_gameMatch->setMode(mode);

	if (mode == GameMatch::ModeLite)
		m_gameMatch->setBgImage("qrc:/internal/img/villa.png");

	if (!Client::clientInstance()->userPlayerCharacter().isEmpty())
		m_gameMatch->setPlayerCharacter(Client::clientInstance()->userPlayerCharacter());
	else
		m_gameMatch->setPlayerCharacter("default");


	if (m_demoMode) {
		connect(m_gameMatch, &GameMatch::gameWin, this, &StudentMaps::onDemoGameWin);
	} else {
		connect(m_gameMatch, &GameMatch::gameWin, this, [=]() {
			m_gameMatch->setXP(m_gameMatch->xp());
			onGameEnd(m_gameMatch, true);
		});

		connect(m_gameMatch, &GameMatch::gameLose, this, [=]() {
			onGameEnd(m_gameMatch, false);
		});
	}

	emit gamePlayReady(m_gameMatch);
}





/**
 * @brief StudentMaps::onGameFinish
 * @param jsonData
 */

void StudentMaps::onGameFinish(QJsonObject jsonData, QByteArray)
{
	if (jsonData.value("success").toBool(false)) {
		QVariantMap d = jsonData.toVariantMap();
		QVariantMap info = d;

		if (m_currentMap) {
			QString missionid = d.value("missionid").toString();
			int level = d.value("level").toInt();
			bool deathmatch = d.value("deathmatch").toBool();
			bool lite = d.value("lite").toBool();
			bool flawless = d.value("flawless").toBool();

			GameMapMissionLevel *missionLevel = m_currentMap->missionLevel(missionid, level);

			info["uuid"] = QString(missionid);
			info["name"] = missionLevel->mission()->name();
			info["level"] = level;
			info["deathmatch"] = deathmatch;
			info["lite"] = lite;
			info["flawless"] = flawless;

			QVector<GameMap::MissionLevelDeathmatch> unlockedLevels = m_currentMap->getUnlocks(missionid, level, deathmatch);

			if (deathmatch && !missionLevel->solvedDeathmatch()) {
				info["medalImage"] = missionLevel->mission()->medalImage();
			} else if (!deathmatch && !missionLevel->solvedNormal()) {
				info["medalImage"] = missionLevel->mission()->medalImage();
			}

			QVariantList list;

			foreach(GameMap::MissionLevelDeathmatch d, unlockedLevels) {
				GameMapMissionLevel *ml = d.first;
				bool isDeathmatch = d.second;

				if (lite && isDeathmatch)
					continue;

				list.append(QVariantMap({
											{ "missionid", ml->mission()->uuid() },
											{ "level", ml->level() },
											{ "name", ml->mission()->name() },
											{ "image", ml->mission()->medalImage() },
											{ "deathmatch", isDeathmatch }
										}));
			}

			info["unlocks"] = list;

			if (unlockedLevels.isEmpty()) {
				GameMap::MissionLevelDeathmatch nextLevel = m_currentMap->getNextMissionLevel(missionid, level, deathmatch, lite);

				if (nextLevel.first) {
					GameMapMissionLevel *ml = nextLevel.first;
					info["next"] = QVariantMap({
												   { "missionid", ml->mission()->uuid() },
												   { "level", ml->level() },
												   { "name", ml->mission()->name() },
												   { "image", ml->mission()->medalImage() },
												   { "deathmatch", nextLevel.second }
											   });
				}
			}
		}

		if (m_liteMode)
			emit gameFinishDialogReady(info);
		else
			QTimer::singleShot(2000, this, [=]() {
				emit gameFinishDialogReady(info);
			});
	}

	getMissionList();
}




/**
 * @brief StudentMaps::onGameListUserGet
 * @param jsonData
 */

void StudentMaps::onGameListUserGet(QJsonObject jsonData, QByteArray)
{
	if (jsonData.value("groupid").toInt() != m_selectedGroupId) {
		qDebug() << "Invalid groupid";
		return;
	}

	if (jsonData.contains("error")) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Lekérdezési hiba"), jsonData.value("error").toString());
		return;
	}

	QJsonArray list = jsonData.value("list").toArray();
	QVariantList ret;

	if (m_missionNameMap.isEmpty()) {
		m_missionNameMap = TeacherMaps::missionNames(db());
	}

	foreach (QJsonValue v, list) {
		QVariantMap m = v.toObject().toVariantMap();
		QString missionname = m_missionNameMap.value(m.value("mapid").toString()).toMap()
							  .value(m.value("missionid").toString()).toString();

		m["missionname"] = missionname;
		m["duration"] = QTime(0,0).addSecs(m.value("duration").toInt()).toString("mm:ss");
		ret.append(m);
	}

	emit gameListUserReady(ret, jsonData.value("username").toString(), jsonData.value("offset").toInt());
}




/**
 * @brief StudentMaps::onCampaignGet
 * @param jsonData
 */

void StudentMaps::onCampaignGet(QJsonObject jsonData, QByteArray)
{
	QJsonArray list = jsonData.value("list").toArray();

	loadGradeList(jsonData.value("gradeList").toArray());

	if (m_missionNameMap.isEmpty()) {
		m_missionNameMap = TeacherMaps::missionNames(db());
	}

	QJsonArray newList;

	foreach (QJsonValue v, list) {
		QJsonObject o = v.toObject();

		QJsonArray alist = o.value("assignment").toArray();
		QJsonArray newAList;

		foreach (QJsonValue v, alist) {
			QJsonObject o = v.toObject();

			QJsonObject grading = o.value("grading").toObject();

			QJsonArray xpArray = grading.value("xp").toArray();
			QJsonArray gradeArray = grading.value("grade").toArray();

			QJsonArray newXpArray, newGradeArray;


			// XP

			foreach (QJsonValue v, xpArray) {
				QJsonObject o = v.toObject();
				QJsonArray cList = o.value("criteria").toArray();
				QJsonArray newCList;

				foreach (QJsonValue v, cList) {
					QJsonObject o = v.toObject();
					QJsonObject criterion = o.value("criterion").toObject();

					if (criterion.value("module").toString() == "missionlevel") {
						QString map = criterion.value("map").toString();
						QString mission = criterion.value("mission").toString();

						mission = m_missionNameMap.value(map).toMap().value(mission).toString();

						if (mission.isEmpty())
							mission = "???";

						QList<MapListObject*> l = m_modelMapList->find("uuid", map);
						map = l.isEmpty() ? "???" : l.at(0)->name();

						criterion["map"] = map;
						criterion["mission"] = mission;
					}

					o["criterion"] = criterion;
					newCList.append(o);
				}

				o["criteria"] = newCList;
				newXpArray.append(o);
			}


			// GRADE

			foreach (QJsonValue v, gradeArray) {
				QJsonObject o = v.toObject();
				QJsonArray cList = o.value("criteria").toArray();
				QJsonArray newCList;

				foreach (QJsonValue v, cList) {
					QJsonObject o = v.toObject();
					QJsonObject criterion = o.value("criterion").toObject();

					if (criterion.value("module").toString() == "missionlevel") {
						QString map = criterion.value("map").toString();
						QString mission = criterion.value("mission").toString();

						mission = m_missionNameMap.value(map).toMap().value(mission).toString();

						if (mission.isEmpty())
							mission = "???";

						QList<MapListObject*> l = m_modelMapList->find("uuid", map);
						map = l.isEmpty() ? "???" : l.at(0)->name();

						criterion["map"] = map;
						criterion["mission"] = mission;
					}

					o["criterion"] = criterion;
					newCList.append(o);
				}

				o["criteria"] = newCList;
				newGradeArray.append(o);
			}

			grading["xp"] = newXpArray;
			grading["grade"] = newGradeArray;
			o["grading"] = grading;
			newAList.append(o);
		}

		o["assignment"] = newAList;
		newList.append(o);
	}


	emit campaignGetReady(newList.toVariantList());
}



/**
 * @brief StudentMaps::onCampaignGet
 * @param jsonData
 */

void StudentMaps::loadGradeList(const QJsonArray &list)
{
	m_gradeMap.clear();

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		if (!m.contains("id"))
			continue;

		int id = m.value("id").toInt();
		m.remove("id");

		m_gradeMap.insert(id, m);
	}
}



/**
 * @brief StudentMaps::_createDownloader
 */

void StudentMaps::_createDownloader()
{
	if (m_downloader)
		return;

	CosDownloader *dl = new CosDownloader(this, CosMessage::ClassUserInfo, "downloadMap", this);
	dl->setJsonKeyFileName("uuid");
	setDownloader(dl);

	connect(m_downloader, &CosDownloader::oneDownloadFinished, this, &StudentMaps::onOneDownloadFinished);
	connect(m_downloader, &CosDownloader::downloadFinished, this, [=]() {
		QJsonObject o;
		o["groupid"] = m_selectedGroupId;
		send("mapListGet", o);

		QList<MapListObject *> maps;

		foreach (CosDownloaderItem item, m_downloader->list()) {
			QList<MapListObject *> o = m_modelMapList->find("uuid", item.remoteFile);
			if (o.size() == 1)
				maps.append(o.at(0));
		}

		emit mapDownloadFinished(maps);
	});
}

bool StudentMaps::liteMode() const
{
	return m_liteMode;
}

void StudentMaps::setLiteMode(bool newLiteMode)
{
	if (m_liteMode == newLiteMode)
		return;
	m_liteMode = newLiteMode;
	emit liteModeChanged();
}


/**
 * @brief StudentMaps::grade
 * @param id
 * @return
 */

QVariantMap StudentMaps::grade(const int &id) const
{
	if (!m_gradeMap.contains(id))
		return QVariantMap({
							   {"id", -1},
							   {"longname", ""},
							   {"shortname", ""},
							   {"value", -1}
						   });
	return m_gradeMap.value(id);
}



/**
 * @brief StudentMaps::getExamContent
 */

void StudentMaps::getExamContent()
{
	qDebug() << "GET EXAM CONTENT";

	QVariantMap m;

	m["test"] = "test";

	emit examContentReady(m);
}


/**
 * @brief StudentMaps::modelMapList
 * @return
 */

ObjectGenericListModel<MapListObject> *StudentMaps::modelMapList() const
{
	return m_modelMapList;
}


/**
 * @brief StudentMaps::checkTerrains
 * @param map
 * @param terrainList
 * @param err
 * @return
 */

bool StudentMaps::checkTerrains(GameMap *map, QList<GameMapMissionLevel *> *levelList)
{
	if (!map)
		return false;

	bool ret = true;

	QVariantMap terrainMap = Client::terrainMap();

	foreach (GameMapMission *m, map->missions()) {
		foreach (GameMapMissionLevel *ml, m->levels()) {
			QString t = ml->terrain();
			if (!terrainMap.contains(t)) {
				if (levelList)
					levelList->append(ml);
				ret = false;
			}
		}
	}

	return ret;
}
