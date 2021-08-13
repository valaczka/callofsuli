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
#include "sqlimage.h"


#define XP_FACTOR_TARGET_BASE	0.1
#define XP_FACTOR_LEVEL			0.5
#define XP_FACTOR_NO_SOLVED		2.5
#define XP_FACTOR_DEATHMATCH	2.3

StudentMaps::StudentMaps(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassStudent, parent)
	, m_modelMapList(nullptr)
	, m_currentMap(nullptr)
	, m_imageDb(nullptr)
	, m_modelMissionList(nullptr)
	, m_demoMode(false)
	, m_demoSolverMap()
	, m_baseXP(100)
	, m_selectedGroupId(-1)
	, m_isGameRunning(false)
	, m_modelUserList(nullptr)
{
	m_modelMapList = new VariantMapModel({
											 "uuid",
											 "name" ,
											 "active",
											 "dataSize" ,
											 "downloaded",
											 "md5"
										 },
										 this);



	m_modelMissionList = new VariantMapModel({
												 "num",
												 "type",
												 "orphan",
												 "solved",
												 "tried",
												 "lockDepth",
												 "uuid",
												 "name",
												 "cname",
												 "levels",
												 "medalImage"
											 },
											 this);

	m_modelUserList = new VariantMapModel({
											  "username",
											  "firstname",
											  "lastname",
											  "nickname",
											  "rankid",
											  "rankname",
											  "ranklevel",
											  "rankimage",
											  "t1",
											  "t2",
											  "t3",
											  "d1",
											  "d2",
											  "d3",
											  "sumxp"
										  },
										  this);

	m_modelMedalList = new VariantMapModel({
											   "level",
											   "deathmatch",
											   "image",
											   "solved"
										   },
										   this);

	//	m_modelCharacterList->setVariantList(Client::mapToList(Client::characterData(), "dir"), "dir");

	connect(this, &StudentMaps::mapListGet, this, &StudentMaps::onMapListGet);
	connect(this, &StudentMaps::missionListGet, this, &StudentMaps::onMissionListGet);
	connect(this, &StudentMaps::userListGet, this, &StudentMaps::onUserListGet);
	connect(this, &StudentMaps::medalListGet, this, &StudentMaps::onMedalListGet);
	connect(this, &StudentMaps::gameCreate, this, &StudentMaps::onGameCreate);
	connect(this, &StudentMaps::gameFinish, this, &StudentMaps::onGameFinish);
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
	delete m_modelMissionList;
	delete m_modelUserList;
	delete m_modelMedalList;

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
 * @brief StudentMaps::gameMapLoad
 * @param data
 */

void StudentMaps::mapLoad(QVariantMap data)
{
	if (m_demoMode)
		return;

	QString uuid = data.value("uuid").toString();
	QString name = data.value("name").toString();

	m_modelMissionList->clear();

	if (uuid.isEmpty()) {
		unloadGameMap();
		return;
	}

	QVariantList l;
	l.append(uuid);
	QVariantMap r = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", l);

	if (r.isEmpty()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Érvénytelen pályaazonosító!"), uuid);
		return;
	}

	QByteArray b = r.value("data").toByteArray();

	GameMap *map = GameMap::fromBinaryData(b);

	if (!map) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Hibás pályaadatok!"), uuid);
		return;
	}

	loadGameMap(map, name);
}


/**
 * @brief StudentMaps::demoMapLoad
 */

void StudentMaps::demoMapLoad()
{
	m_modelMissionList->clear();

	QString demoMap = ":/internal/game/demo.map";

	if (!QFile::exists(demoMap)) {
		qWarning() << demoMap << "not exists";
		unloadGameMap();
		return;
	}

	qDebug() << "Load demo map" << demoMap;

	m_demoSolverMap = Client::readJsonDocument(Client::standardPath("demomap.json")).object().toVariantMap();


	QFile f(demoMap);
	if (f.open(QIODevice::ReadOnly)) {

		QByteArray b = f.readAll();

		f.close();

		GameMap *map = GameMap::fromBinaryData(b);

		if (!map) {
			m_client->sendMessageError(tr("Belső hiba"), tr("Hibás pályaadatok!"));
			return;
		}

		loadGameMap(map, tr("Demo pálya"));
	}
}



/**
 * @brief StudentMaps::getMissionList
 */

void StudentMaps::getMissionList()
{
	if (!m_currentMap) {
		m_modelMissionList->clear();
		return;
	}

	if (m_demoMode) {
		QJsonObject o;

		QJsonArray list;

		QVariantMap::const_iterator it;

		for (it=m_demoSolverMap.constBegin(); it != m_demoSolverMap.constEnd(); ++it) {
			QJsonObject oo;
			oo["missionid"] = it.key();
			oo["maxLevel"] = it.value().toInt();
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
 * @brief StudentMaps::playGame
 * @param data
 */

void StudentMaps::playGame(QVariantMap data)
{
	if (!m_currentMap)
		return;

	if (m_demoMode) {
		QJsonObject o;
		o["gameid"] = -1;
		o["missionid"] = data.value("uuid").toString();
		o["level"] = data.value("level", 1).toInt();
		o["deathmatch"] = data.value("deathmatch", false).toBool();
		onGameCreate(o, QByteArray());
		return;
	}

	QJsonObject o;
	o["map"] = QString(m_currentMap->uuid());
	o["mission"] = data.value("uuid").toString();
	o["level"] = data.value("level", 1).toInt();
	o["hasSolved"] = data.value("hasSolved", false).toBool();
	o["deathmatch"] = data.value("deathmatch", false).toBool();

	send("gameCreate", o);
}

void StudentMaps::setDemoMode(bool demoMode)
{
	if (m_demoMode == demoMode)
		return;

	m_demoMode = demoMode;
	emit demoModeChanged(m_demoMode);

	if (m_demoMode && !m_currentMap) {
		demoMapLoad();
	}
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

void StudentMaps::setIsGameRunning(bool isGameRunning)
{
	if (m_isGameRunning == isGameRunning)
		return;

	m_isGameRunning = isGameRunning;
	emit isGameRunningChanged(m_isGameRunning);
}

void StudentMaps::setModelUserList(VariantMapModel *modelUserList)
{
	if (m_modelUserList == modelUserList)
		return;

	m_modelUserList = modelUserList;
	emit modelUserListChanged(m_modelUserList);
}

void StudentMaps::setModelMedalList(VariantMapModel *modelMedalList)
{
	if (m_modelMedalList == modelMedalList)
		return;

	m_modelMedalList = modelMedalList;
	emit modelMedalListChanged(m_modelMedalList);
}




/**
 * @brief StudentMaps::mapDownload
 * @param data
 */

void StudentMaps::mapDownload(QVariantMap data)
{
	if (m_demoMode) {
		return;
	}

	if (!m_downloader) {
		CosDownloader *dl = new CosDownloader(this, CosMessage::ClassUserInfo, "downloadMap", this);
		dl->setJsonKeyFileName("uuid");
		setDownloader(dl);

		connect(m_downloader, &CosDownloader::oneDownloadFinished, this, &StudentMaps::onOneDownloadFinished);
		connect(m_downloader, &CosDownloader::downloadFinished, this, [=]() {
			QJsonObject o;
			o["groupid"] = m_selectedGroupId;
			send("mapListGet", o);
		});
	}

	m_downloader->clear();

	QVariantList uuidList;

	if (data.contains("list")) {
		uuidList = data.value("list").toList();
	} else if (data.contains("uuid")) {
		uuidList.append(data.value("uuid"));
	}

	foreach (QVariant v, uuidList) {
		QString uuid = v.toString();
		int index = m_modelMapList->variantMapData()->find("uuid", uuid);
		if (index == -1) {
			continue;
		}

		QVariantMap o = m_modelMapList->variantMapData()->at(index).second;

		m_downloader->append(o.value("uuid").toString(),
							 "",
							 o.value("dataSize").toInt(),
							 o.value("md5").toString(),
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
 * @brief StudentMaps::clientSetup
 */

void StudentMaps::clientSetup()
{
	if (!m_client)
		return;

	if (!m_demoMode) {
		CosDb *db = studentMapsDb(m_client, this);
		if (db)
			addDb(db, false);
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

	QVariantList ret;

	foreach (QJsonValue v, list) {
		QVariantMap m = v.toObject().toVariantMap();
		QString uuid = m.value("uuid").toString();

		m["downloaded"] = false;

		QVariantList l;
		l.append(uuid);

		QVariantMap r = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", l);

		QByteArray d = r.value("data").toByteArray();

		if (!r.isEmpty() && !d.isEmpty() &&
			m.value("md5").toString() == QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex()) &&
			m.value("dataSize").toInt() == d.size()) {
			m["downloaded"] = true;
		}

		ret.append(m);
	}

	m_modelMapList->setVariantList(ret, "uuid");
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

void StudentMaps::loadGameMap(GameMap *map, const QString &mapName)
{
	unloadGameMap();

	if (!map)
		return;

	m_imageDb = new CosDb("tmpmapimagedb", this);
	m_imageDb->setDatabaseName(Client::standardPath("tmpmapimage.db"));
	m_imageDb->open();

	if (!map->imagesToDb(m_imageDb)) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Nem sikerült elkészíteni a képadatbázist!"));
		delete m_imageDb;
		m_imageDb = nullptr;
		return;
	}

	map->deleteImages();

	m_currentMap = map;
	emit gameMapLoaded(QString(map->uuid()), mapName);


	qDebug() << "Add sqlimage provider mapimagedb";
	QQmlEngine *engine = qmlEngine(this);
	SqlImage *sqlImage = new SqlImage(m_client, m_imageDb, "images");
	engine->addImageProvider("mapimagedb", sqlImage);
}





/**
 * @brief StudentMaps::unsetGameMap
 */

void StudentMaps::unloadGameMap()
{
	if (m_imageDb) {
		QQmlEngine *engine = qmlEngine(this);
		qDebug() << "Remove image provider mapimagedb";
		if (engine)
			engine->removeImageProvider("mapimagedb");

		QString db = m_imageDb->databaseName();

		if (m_imageDb->isOpen())
			m_imageDb->close();

		delete m_imageDb;
		m_imageDb = nullptr;

		QFile::remove(db);
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

void StudentMaps::onDemoGameWin(const QString &uuid, const int level)
{
	GameMatch *match = qobject_cast<GameMatch *>(sender());


	QJsonObject o;

	if (match)
		o["xp"] = match->xp();
	else
		o["xp"] = 0;

	o["solved"] = 1;
	o["success"] = true;
	o["level"] = match->level();
	o["deathmatch"] = match->deathmatch();

	onGameFinish(o, QByteArray());


	int maxLevel = m_demoSolverMap.value(uuid, -1).toInt();

	if (maxLevel > level)
		return;

	m_demoSolverMap[uuid] = level;
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
	o["medalImage"] = match->missionLevel()->mission()->medalImage();
	m_client->socketSend(CosMessage::ClassStudent, "gameFinish", o);
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

	if (!m_currentMap) {
		m_modelMissionList->clear();
		return;
	}

	m_currentMap->setSolver(list.toVariantList());


	GameMap::Campaign *cerror = nullptr;
	GameMap::Mission *merror = nullptr;

	m_currentMap->campaignLockTree(&cerror);
	m_currentMap->missionLockTree(&merror);

	if (cerror || merror) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Hibás pálya!"));
		m_modelMissionList->clear();
		return;
	}


	QVariantList ret;

	int num = 1;

	foreach (GameMap::Campaign *c, m_currentMap->campaigns()) {
		QVariantMap m;
		m["num"] = num++;
		m["orphan"] = false;
		m["type"] = 0;
		m["solved"] = c->getSolved();
		m["tried"] = c->getTried();
		if (c->getLocked())
			m["lockDepth"] = 1;
		else
			m["lockDepth"] = 0;
		m["uuid"] = "";
		m["name"] = c->name();
		m["cname"] = c->name();
		m["levels"] = QVariantList();
		m["medalImage"] = "";

		ret.append(m);

		if (c->getLocked())
			continue;

		foreach (GameMap::Mission *mis, c->missions()) {
			if (mis->getLockDepth() > 1)
				continue;

			QVariantMap m;
			m["num"] = num++;
			m["orphan"] = false;
			m["type"] = 1;
			m["lockDepth"] = mis->getLockDepth();
			m["tried"] = mis->getTried();
			if (mis->getSolvedLevel() > 0)
				m["solved"] = true;
			else
				m["solved"] = false;
			m["uuid"] = mis->uuid();
			m["name"] = mis->name();
			m["cname"] = c->name();
			m["description"] = mis->description();
			m["medalImage"] = mis->medalImage();

			int lMin = mis->getLockDepth() == 0 ?
						   qMax(mis->getSolvedLevel()+1, 1) :
						   -1;
			QVariantList l;
			foreach (GameMap::MissionLevel *ml, mis->levels()) {
				QVariantMap mm;
				mm["level"] = ml->level();
				mm["solved"] = (mis->getSolvedLevel() >= ml->level());
				mm["startHP"] = ml->startHP();
				mm["duration"] = ml->duration();

				TerrainData t = Client::terrain(ml->terrain());
				mm["enemies"] = t.enemies;

				QVariantList modes;
				{
					QVariantMap mode;
					int xp = m_baseXP*XP_FACTOR_LEVEL*ml->level();
					if (mis->getSolvedLevel() < ml->level())
						xp *= XP_FACTOR_NO_SOLVED;
					//xp += t.enemies*m_baseXP*XP_FACTOR_TARGET_BASE;
					mode["type"] = "normal";
					mode["available"] = (ml->level() <= lMin);
					mode["xp"] = xp;
					modes.append(mode);
				}
				if (ml->canDeathmatch()) {
					QVariantMap mode;
					int xp = m_baseXP*XP_FACTOR_LEVEL*ml->level();
					xp *= XP_FACTOR_DEATHMATCH;
					//xp += t.enemies*m_baseXP*XP_FACTOR_TARGET_BASE;
					mode["type"] = "deathmatch";
					mode["available"] = (mis->getSolvedLevel() >= ml->level() && ml->level() <= lMin);
					mode["xp"] = xp;
					modes.append(mode);
				}

				mm["modes"] = modes;

				l.append(mm);

				if (!m.contains("backgroundImage")) {
					QString imageFolder = ml->imageFolder();
					QString imageFile = ml->imageFile();

					if (!imageFolder.isEmpty() && !imageFile.isEmpty())
						m["backgroundImage"] = "image://mapimagedb/"+imageFolder+"/"+imageFile;
					else
						m["backgroundImage"] = "qrc:/internal/game/bg.png";
				}
			}

			m["levels"] = l;

			ret.append(m);
		}
	}



	foreach (GameMap::Mission *mis, m_currentMap->orphanMissions()) {
		if (mis->getLockDepth() > 1)
			continue;

		QVariantMap m;
		m["num"] = num++;
		m["orphan"] = true;
		m["type"] = 1;
		m["lockDepth"] = mis->getLockDepth();
		m["tried"] = mis->getTried();
		if (mis->getSolvedLevel() > 0)
			m["solved"] = true;
		else
			m["solved"] = false;
		m["uuid"] = mis->uuid();
		m["name"] = mis->name();
		m["cname"] = "";
		m["description"] = mis->description();
		m["medalImage"] = mis->medalImage();

		int lMin = mis->getLockDepth() == 0 ?
					   qMax(mis->getSolvedLevel()+1, 1) :
					   -1;
		QVariantList l;
		foreach (GameMap::MissionLevel *ml, mis->levels()) {
			QVariantMap mm;
			mm["level"] = ml->level();
			mm["solved"] = (mis->getSolvedLevel() >= ml->level());
			mm["startHP"] = ml->startHP();
			mm["duration"] = ml->duration();

			TerrainData t = Client::terrain(ml->terrain());
			mm["enemies"] = t.enemies;

			QVariantList modes;
			{
				QVariantMap mode;
				int xp = m_baseXP*XP_FACTOR_LEVEL*ml->level();
				if (mis->getSolvedLevel() < ml->level())
					xp *= XP_FACTOR_NO_SOLVED;
				//xp += t.enemies*m_baseXP*XP_FACTOR_TARGET_BASE;
				mode["type"] = "normal";
				mode["available"] = (ml->level() <= lMin);
				mode["xp"] = xp;
				modes.append(mode);
			}
			if (ml->canDeathmatch()) {
				QVariantMap mode;
				int xp = m_baseXP*XP_FACTOR_LEVEL*ml->level();
				xp *= XP_FACTOR_DEATHMATCH;
				//xp += t.enemies*m_baseXP*XP_FACTOR_TARGET_BASE;
				mode["type"] = "deathmatch";
				mode["available"] = (mis->getSolvedLevel() >= ml->level() && ml->level() <= lMin);
				mode["xp"] = xp;
				modes.append(mode);
			}

			mm["modes"] = modes;

			l.append(mm);

			if (!m.contains("backgroundImage")) {
				QString imageFolder = ml->imageFolder();
				QString imageFile = ml->imageFile();

				if (!imageFolder.isEmpty() && !imageFile.isEmpty())
					m["backgroundImage"] = "image://mapimagedb/"+imageFolder+"/"+imageFile;
				else
					m["backgroundImage"] = "qrc:/internal/game/bg.png";
			}
		}

		m["levels"] = l;

		ret.append(m);
	}



	m_modelMissionList->setVariantList(ret, "num");

	emit missionListChanged();
}



/**
 * @brief StudentMaps::onMedalListGet
 * @param jsonData
 */

void StudentMaps::onMedalListGet(QJsonObject jsonData, QByteArray)
{
	QString uuid = jsonData.value("uuid").toString();
	QJsonArray list = jsonData.value("list").toArray();

	if (uuid.isEmpty()) {
		qWarning() << "Missing map uuid";
		return;
	}

	QVariantList l;
	l.append(uuid);
	QVariantMap r = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", l);

	if (r.isEmpty()) {
		qWarning() << tr("Érvénytelen pályaazonosító") << uuid;
		return;
	}

	QByteArray b = r.value("data").toByteArray();

	GameMap *map = GameMap::fromBinaryData(b);

	if (!map) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Hibás pályaadatok!"), uuid);
		return;
	}

	QVector<GameMap::MissionLevelData> levelData = map->missionLevelsData();

	QVariantList ret;

	int idx=0;

	foreach (GameMap::MissionLevelData d, levelData) {
		QVariantMap r;
		r["index"] = idx++;
		r["level"] = d.missionLevel()->level();
		r["deathmatch"] = d.deathmatch();
		r["image"] = d.mission()->medalImage();

		bool solved = false;

		QString missionUuid = QString(d.mission()->uuid());

		foreach (QJsonValue v, list) {
			QJsonObject o = v.toObject();
			QString key = QString("%1%2").arg(d.deathmatch() ? "d" : "t").arg(d.missionLevel()->level());
			if (o.value("missionid").toString() == missionUuid && o.value(key).toInt(0) > 0) {
				solved = true;
				break;
			}
		}

		r["solved"] = solved;
		ret.append(r);
	}


	m_modelMedalList->clear();
	m_modelMedalList->setVariantList(ret, "index");

	delete map;
}




/**
 * @brief StudentMaps::onUserListGet
 * @param jsonData
 */

void StudentMaps::onUserListGet(QJsonObject jsonData, QByteArray)
{
	if (m_demoMode)
		return;

	m_modelUserList->unselectAll();
	m_modelUserList->setJsonArray(jsonData.value("list").toArray(), "username");
}






/**
 * @brief StudentMaps::onGameCreate
 * @param jsonData
 */

void StudentMaps::onGameCreate(QJsonObject jsonData, QByteArray)
{
	GameMap::MissionLevel *missionLevel = nullptr;

	if (m_currentMap)
		missionLevel = m_currentMap->missionLevel(jsonData.value("missionid").toString().toLatin1(),
												  jsonData.value("level").toInt());

	int gameId = jsonData.value("gameid").toInt();

	if (!m_currentMap || !missionLevel) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Pályaadatok nem elérhetőek!"));

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
		m_client->sendMessageError(tr("Belső hiba"), err);

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
	m_gameMatch->setImageDbName("mapimagedb");
	m_gameMatch->setGameId(gameId);
	m_gameMatch->setBaseXP(m_baseXP*XP_FACTOR_TARGET_BASE);
	m_gameMatch->setDeathmatch(jsonData.value("deathmatch").toBool(false));

	if (!m_client->userPlayerCharacter().isEmpty())
		m_gameMatch->setPlayerCharacter(m_client->userPlayerCharacter());
	else
		m_gameMatch->setPlayerCharacter("default");

	bool hasSolved = jsonData.value("hasSolved").toBool(false);

	if (m_demoMode) {
		if (!m_demoSolverMap.contains(m_gameMatch->missionUuid())) {
			m_demoSolverMap[m_gameMatch->missionUuid()] = -1;
		}
		connect(m_gameMatch, &GameMatch::gameWin, this, &StudentMaps::onDemoGameWin);
	} else {
		connect(m_gameMatch, &GameMatch::gameWin, this, [=](const QString &, const int level) {
			int xp = m_baseXP*XP_FACTOR_LEVEL*level;
			if (!hasSolved)
				xp *= XP_FACTOR_NO_SOLVED;
			else if (m_gameMatch->deathmatch())
				xp *= XP_FACTOR_DEATHMATCH;



			m_gameMatch->setXP(m_gameMatch->xp()+xp);
			onGameEnd(m_gameMatch, true);
		});

		connect(m_gameMatch, &GameMatch::gameLose, this, [=](const QString &, const int) {
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

		QTimer::singleShot(2000, [=]() {
			emit gameFinishDialogReady(d);
		});
	}

	getMissionList();
}
