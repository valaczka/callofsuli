/*
 * ---- Call of Suli ----
 *
 * teachermaps.cpp
 *
 * Created on: 2020. 12. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMaps
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

#include "teachermaps.h"

#include <QFile>

TeacherMaps::TeacherMaps(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassTeacher, parent)
	, m_modelMapList(nullptr)
	, m_selectedMapId("")
{
	m_modelMapList = new VariantMapModel({
											 "uuid",
											 "name" ,
											 "version",
											 "dataSize" ,
											 "binded" ,
											 "used" ,
											 "downloaded" ,
											 "md5",
											 "lastModified"
										 },
										 this);

	connect(this, &TeacherMaps::mapListGet, this, &TeacherMaps::onMapListGet);
	connect(this, &TeacherMaps::selectedMapIdChanged, this, &TeacherMaps::onMapSelected);
}



/**
 * @brief TeacherMaps::~TeacherMaps
 */

TeacherMaps::~TeacherMaps()
{
	delete m_modelMapList;

	if (m_downloader)
		delete m_downloader;

}


/**
 * @brief TeacherMaps::teacherMapsDb
 * @param client
 * @param connectionName
 * @return
 */

CosDb *TeacherMaps::teacherMapsDb(Client *client, QObject *parent, const QString &connectionName)
{
	if (!client || client->serverDataDir().isEmpty())
		return nullptr;

	QString dbname = client->serverDataDir()+"/teachermaps.db";

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
 * @brief TeacherMaps::mapDownloadInfoReload
 */

QVariantMap TeacherMaps::mapDownloadInfo(CosDb *db)
{
	Q_ASSERT(db);

	QVariantMap ret;

	QVariantList list = db->execSelectQuery("SELECT uuid, data FROM maps");

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		QByteArray d = m.value("data").toByteArray();
		QString uuid = m.value("uuid").toString();

		ret[uuid] = QVariantMap({
									{ "md5", QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex()) },
									{ "dataSize", d.size() }
								});
	}

	return ret;
}




/**
 * @brief TeacherMaps::mapDownloadPrivate
 * @return
 */

void TeacherMaps::mapDownloadPrivate(const QVariantMap &data, CosDownloader *downloader, VariantMapModel *mapModel)
{
	Q_ASSERT(downloader);
	Q_ASSERT(mapModel);

	QVariantList uuidList;

	if (data.contains("list")) {
		uuidList = data.value("list").toList();
	} else if (data.contains("uuid")) {
		uuidList.append(data.value("uuid"));
	}

	foreach (QVariant v, uuidList) {
		QString uuid = v.toString();
		int index = mapModel->variantMapData()->find("uuid", uuid);
		if (index == -1) {
			continue;
		}

		QVariantMap o = mapModel->variantMapData()->at(index).second;

		downloader->append(o.value("uuid").toString(),
						   "",
						   o.value("dataSize").toInt(),
						   o.value("md5").toString(),
						   false,
						   0.0);
	}
}






/**
 * @brief TeacherMaps::clientSetup
 */

void TeacherMaps::clientSetup()
{
	if (!m_client)
		return;

	CosDb *db = teacherMapsDb(m_client, this);
	addDb(db, false);
}






/**
 * @brief TeacherMaps::mapDownload
 * @param data
 */

void TeacherMaps::mapDownload(QVariantMap data)
{
	if (!m_downloader) {
		CosDownloader *dl = new CosDownloader(this, CosMessage::ClassUserInfo, "downloadMap", this);
		dl->setJsonKeyFileName("uuid");
		setDownloader(dl);

		connect(m_downloader, &CosDownloader::oneDownloadFinished, this, &TeacherMaps::onOneDownloadFinished);
		connect(m_downloader, &CosDownloader::downloadFinished, this, [=](){
			send("mapListGet");
		});
	}

	m_downloader->clear();

	mapDownloadPrivate(data, m_downloader, m_modelMapList);

	if (m_downloader->hasDownloadable()) {
		emit mapDownloadRequest(Client::formattedDataSize(m_downloader->fullSize()));
	} else {
		send("mapListGet");
	}
}




/**
 * @brief TeacherMaps::mapUpload
 * @param data
 */

void TeacherMaps::mapUpload(const QUrl &url)
{
	QFile f(url.toLocalFile());

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		m_client->sendMessageWarning(tr("Fájl megnyitási hiba"), tr("Nem sikerült megnyitni a fájlt:\n%1").arg(f.fileName()));
		return;
	}

	QByteArray content = f.readAll();

	GameMap *map = GameMap::fromBinaryData(content);
	if (!map) {
		m_client->sendMessageWarning(tr("Fájl hiba"), tr("Érvénytelen formátumú fájl:\n%1").arg(f.fileName()));
		return;
	}

	delete map;

	QString name = url.fileName();
	if (name.endsWith(".map"))
		name.chop(4);

	f.close();

	send("mapAdd", {{"name", name}}, content);
}



/**
 * @brief TeacherMaps::mapOverride
 * @param uuid
 * @param url
 */

void TeacherMaps::mapOverride(const QUrl &url)
{
	if (m_selectedMapId.isEmpty())
		return;

	QFile f(url.toLocalFile());

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		m_client->sendMessageWarning(tr("Fájl megnyitási hiba"), tr("Nem sikerült megnyitni a fájlt:\n%1").arg(f.fileName()));
		return;
	}

	QByteArray content = f.readAll();

	GameMap *map = GameMap::fromBinaryData(content);
	if (!map) {
		m_client->sendMessageWarning(tr("Fájl hiba"), tr("Érvénytelen formátumú fájl:\n%1").arg(f.fileName()));
		return;
	}

	QString fuuid = map->uuid();

	delete map;

	if (fuuid != m_selectedMapId) {
		m_client->sendMessageWarning(tr("Fájl hiba"), tr("A fájl nem ennek a pályának módosított változata:\n%1").arg(f.fileName()));
		return;
	}

	QString name = url.fileName();
	if (name.endsWith(".map"))
		name.chop(4);

	f.close();

	send("mapModify", {{"uuid", m_selectedMapId}}, content);
}




/**
 * @brief TeacherMaps::mapExport
 * @param data
 */

void TeacherMaps::mapExport(const QUrl &url)
{
	if (m_selectedMapId.isEmpty())
		return;

	QVariantMap m = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", {m_selectedMapId});

	if (m.isEmpty()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Érvénytelen pályaazonosító!"));
		return;
	}

	QByteArray b = m.value("data").toByteArray();

	QFile f(url.toLocalFile());
	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem lehet írni a fájlba:\n%1").arg(f.fileName()));
		return;
	}
	f.write(b);
	f.close();

	m_client->sendMessageInfo(tr("Exportálás"), tr("Az exportálás sikerült: %1").arg(f.fileName()));
	return;
}






void TeacherMaps::setSelectedMapId(QString selectedMapId)
{
	if (m_selectedMapId == selectedMapId)
		return;

	m_selectedMapId = selectedMapId;
	emit selectedMapIdChanged(m_selectedMapId);
}






/**
 * @brief TeacherMaps::mapListGet
 * @param jsonData
 */

void TeacherMaps::onMapListGet(QJsonObject jsonData, QByteArray)
{
	m_modelMapList->unselectAll();

	QJsonArray ret;

	QVariantMap info = mapDownloadInfo(db());

	QJsonArray list = jsonData.value("list").toArray();

	foreach (QJsonValue v, list) {
		QJsonObject m = v.toObject();
		QString uuid = m.value("uuid").toString();

		m["downloaded"] = false;

		if (info.contains(uuid)) {
			QVariantMap dm = info.value(uuid).toMap();
			if (m.value("md5") == dm.value("md5") && m.value("dataSize") == dm.value("dataSize"))
				m["downloaded"] = true;
		}

		ret.append(m);
	}

	m_modelMapList->setJsonArray(ret, "uuid");

	onMapSelected();
}


/**
 * @brief TeacherMaps::onOneDownloadFinished
 * @param item
 * @param data
 * @param jsonData
 */

void TeacherMaps::onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &)
{
	mapDownloadFinished(db(), item, data);
}


/**
 * @brief TeacherMaps::onMapSelected
 */

void TeacherMaps::onMapSelected(const QString &)
{
	QVariantMap mapdata;
	mapdata["uuid"] = m_selectedMapId;
	mapdata["mapReady"] = false;

	if (!m_selectedMapId.isEmpty()) {
		int key = m_modelMapList->findKey("uuid", m_selectedMapId);
		if (key == -1) {
			setSelectedMapId("");
			return;
		}

		QVariantMap d = m_modelMapList->getByKey(key);
		mapdata.insert(d);

		bool dl = d.value("downloaded").toBool();

		if (dl) {
			QByteArray data = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", {m_selectedMapId}).value("data").toByteArray();

			if (!data.isEmpty()) {
				GameMap *m = GameMap::fromBinaryData(data);

				if (m) {
					QVariantList mlist;
					foreach (GameMap::Mission *mis, m->missions()) {
						mlist.append(mis->name());
					}
					mapdata["mapReady"] = true;
					mapdata["missionList"] = mlist;

					delete m;
				}
			}
		}
	}


	emit mapDataReady(mapdata);
}



/**
 * @brief TeacherMaps::onOneDownloadFinished
 * @param item
 * @param data
 */

void TeacherMaps::mapDownloadFinished(CosDb *db, const CosDownloaderItem &item, const QByteArray &data)
{
	Q_ASSERT(db);

	QVariantMap m;
	m["uuid"] = item.remoteFile;
	m["data"] = data;

	db->execInsertQuery("INSERT OR REPLACE INTO maps (?k?) VALUES (?)", m);
}

