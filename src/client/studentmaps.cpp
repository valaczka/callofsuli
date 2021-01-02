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

StudentMaps::StudentMaps(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassStudent, parent)
	, m_modelMapList(nullptr)
	, m_currentMap(nullptr)
	, m_imageDb(nullptr)
	, m_modelMissionList(nullptr)
{
	m_modelMapList = new VariantMapModel({
											 "uuid",
											 "name" ,
											 "dataSize" ,
											 "downloaded" ,
											 "md5"
										 },
										 this);



	m_modelMissionList = new VariantMapModel({
												 "num",
												 "type",
												 "solved",
												 "tried",
												 "lockDepth",
												 "uuid",
												 "name",
												 "cname",
												 "levels"
											 },
											 this);

	connect(this, &StudentMaps::mapListGet, this, &StudentMaps::onMapListGet);
	connect(this, &StudentMaps::missionListGet, this, &StudentMaps::onMissionListGet);
	connect(this, &StudentMaps::gameCreate, this, &StudentMaps::onGameCreate);
	connect(this, &StudentMaps::gameFinish, this, &StudentMaps::onGameFinish);
	connect(this, &StudentMaps::gameUpdate, this, &StudentMaps::onGameUpdate);
}





/**
 * @brief StudentMaps::~StudentMaps
 */

StudentMaps::~StudentMaps()
{
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
 * @brief StudentMaps::gameMapLoad
 * @param data
 */

void StudentMaps::mapLoad(QVariantMap data)
{
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
 * @brief StudentMaps::getMissionList
 */

void StudentMaps::getMissionList()
{
	if (!m_currentMap) {
		m_modelMissionList->clear();
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

	QJsonObject o;
	o["map"] = QString(m_currentMap->uuid());
	o["mission"] = data.value("uuid").toString();
	o["level"] = data.value("level", 1).toInt();

	send("gameCreate", o);
}






/**
 * @brief StudentMaps::mapDownload
 * @param data
 */

void StudentMaps::mapDownload(QVariantMap data)
{
	if (!m_downloader) {
		CosDownloader *dl = new CosDownloader(this, CosMessage::ClassUserInfo, "downloadMap", this);
		dl->setJsonKeyFileName("uuid");
		setDownloader(dl);

		connect(m_downloader, &CosDownloader::oneDownloadFinished, this, &StudentMaps::onOneDownloadFinished);
		connect(m_downloader, &CosDownloader::downloadFinished, this, [=]() { send("mapListGet"); });
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
		send("mapListGet");
	}
}




/**
 * @brief StudentMaps::clientSetup
 */

void StudentMaps::clientSetup()
{
	if (!m_client)
		return;

	CosDb *db = studentMapsDb(m_client, this);
	addDb(db, false);
}





/**
 * @brief StudentMaps::onMapListGet
 * @param jsonData
 */

void StudentMaps::onMapListGet(QJsonObject jsonData, QByteArray)
{
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
	emit gameMapLoaded(mapName);


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
 * @brief StudentMaps::onMissionListGet
 * @param jsonData
 */

void StudentMaps::onMissionListGet(QJsonObject jsonData, QByteArray)
{
	QJsonArray list = jsonData.value("list").toArray();

	if (!m_currentMap) {
		m_modelMissionList->clear();
		return;
	}

	m_currentMap->setSolver(list.toVariantList());

	GameMap::CampaignLockHash clock = m_currentMap->campaignLockTree();
	GameMap::MissionLockHash mlock = m_currentMap->missionLockTree();

	if (clock.isEmpty() || mlock.isEmpty()) {
		m_modelMissionList->clear();
		return;
	}


	QVariantList ret;

	int num = 1;

	foreach (GameMap::Campaign *c, m_currentMap->campaigns()) {
		QVariantMap m;
		m["num"] = num++;
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

		ret.append(m);

		if (c->getLocked())
			continue;

		foreach (GameMap::Mission *mis, c->missions()) {
			if (mis->getLockDepth() > 1)
				continue;

			QVariantMap m;
			m["num"] = num++;
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

			int lMin = mis->getLockDepth() == 0 ?
						   qMax(mis->getSolvedLevel()+1, 1) :
						   -1;
			QVariantList l;
			foreach (GameMap::MissionLevel *ml, mis->levels()) {
				QVariantMap mm;
				mm["level"] = ml->level();
				mm["available"] = (ml->level() <= lMin);
				mm["solved"] = (mis->getSolvedLevel() >= ml->level());
				l.append(mm);
			}

			m["levels"] = l;

			ret.append(m);
		}
	}

	m_modelMissionList->setVariantList(ret, "num");

	emit missionListChanged();
}






/**
 * @brief StudentMaps::onGameCreate
 * @param jsonData
 */

void StudentMaps::onGameCreate(QJsonObject jsonData, QByteArray)
{
	qDebug() << "#### GAME CREATED" << jsonData;

	GameMap::MissionLevel *missionLevel = nullptr;

	if (m_currentMap)
		missionLevel = m_currentMap->missionLevel(jsonData.value("missionid").toString().toLatin1(),
												  jsonData.value("level").toInt());

	int gameId = jsonData.value("gameid").toInt();

	if (!m_currentMap || !missionLevel) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Pályaadatok nem elérhetőek!"));
		QJsonObject o;
		o["id"] = gameId;
		o["xp"] = 0;
		o["success"] = false;
		send("gameFinish", o);
		return;
	}


	GameMatch *m_gameMatch = new GameMatch(missionLevel, this);
	m_gameMatch->setDeleteGameMap(false);
	m_gameMatch->setImageDbName("mapimagedb");
	m_gameMatch->setGameId(gameId);

	emit gamePlayReady(m_gameMatch);
}




/**
 * @brief StudentMaps::onGameUpdate
 * @param jsonData
 */

void StudentMaps::onGameUpdate(QJsonObject jsonData, QByteArray)
{
qDebug() << "#### GAME UPDATED ###";
}



/**
 * @brief StudentMaps::onGameFinish
 * @param jsonData
 */

void StudentMaps::onGameFinish(QJsonObject jsonData, QByteArray)
{
	qDebug() << "#### GAME FINISHED" << jsonData;
	getMissionList();
}