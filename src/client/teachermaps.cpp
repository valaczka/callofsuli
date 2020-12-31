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


TeacherMaps::TeacherMaps(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassTeacherMap, parent)
	, m_modelMapList(nullptr)
	, m_isUploading(false)
{
	m_modelMapList = new VariantMapModel({
											 "uuid",
											 "name" ,
											 "dataSize" ,
											 "binded" ,
											 "used" ,
											 "upload" ,
											 "download" ,
											 "local",
											 "lastModified"
										 },
										 this);

	connect(this, &TeacherMaps::mapListGet, this, &TeacherMaps::onMapListGet);
	connect(this, &TeacherMaps::mapUpdate, this, &TeacherMaps::onMapUpdated);
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
								 "name TEXT NOT NULL,"
								 "md5 TEXT NOT NULL,"
								 "lastModified TEXT NOT NULL,"
								 "data BLOB NOT NULL"
								 ")")) {
			qWarning() << tr("Nem sikerült előkészíteni az adatbázist:") << dbname;
			db->close();
			delete db;
			return nullptr;
		}

		if (!db->execSimpleQuery("CREATE TABLE localmaps("
								 "uuid TEXT NOT NULL PRIMARY KEY,"
								 "name TEXT NOT NULL,"
								 "md5 TEXT NOT NULL,"
								 "lastModified TEXT NOT NULL DEFAULT (datetime('now')),"
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
 * @brief TeacherMaps::mapAdd
 * @param data
 */

void TeacherMaps::mapAdd(QVariantMap data)
{
	if (data.value("name").toString().isEmpty())
		return;

	QUuid uuid = QUuid::createUuid();

	GameMap map(uuid.toByteArray());
	QByteArray b = map.toBinaryData();

	QVariantMap m;
	m["uuid"] = uuid.toString();
	m["name"] = data.value("name").toString();
	m["data"] = b;
	m["md5"] = QString(QCryptographicHash::hash(b, QCryptographicHash::Md5).toHex());

	db()->execInsertQuery("INSERT INTO localmaps(?k?) values (?)", m);
	send("mapListGet");
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
		int index = m_modelMapList->variantMapData()->find("uuid2", uuid);			// Mert ha helyi, akkor uuid2 = uuid+"UP"
		if (index == -1) {
			qDebug() << "Skip" << uuid;
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
 * @brief TeacherMaps::mapUpload
 * @param data
 */

void TeacherMaps::mapUpload(QVariantMap data)
{
	QVariantList uuidList;

	if (data.contains("list")) {
		uuidList = data.value("list").toList();
	} else if (data.contains("uuid")) {
		uuidList.append(data.value("uuid"));
	}

	foreach (QVariant v, uuidList) {
		QString uuid = v.toString();

		QVariantList l;
		l.append(uuid);
		QVariantMap m = db()->execSelectQueryOneRow("SELECT data, name FROM localmaps WHERE uuid=?", l);

		if (m.isEmpty()) {
			qDebug() << "SKIP" << uuid;
			continue;
		}

		QByteArray b = m.value("data").toByteArray();

		QJsonObject j;
		j["uuid"] = uuid;
		j["name"] = m.value("name").toString();

		setIsUploading(true);
		send("mapUpdate", j, b);
	}
}


/**
 * @brief TeacherMaps::mapRename
 * @param data
 */

void TeacherMaps::mapRename(QVariantMap data)
{
	QString uuid = data.value("uuid").toString();
	QString name = data.value("name").toString();
	bool local = data.value("local").toBool();

	if (uuid.isEmpty() || name.isEmpty())
		return;

	if (!local) {
		QJsonObject j;
		j["uuid"] = uuid;
		j["name"] = name;
		setIsUploading(true);
		send("mapUpdate", j);
	} else {
		QVariantList l;
		l.append(name);
		l.append(uuid);
		db()->execSimpleQuery("UPDATE localmaps SET name=? WHERE uuid=?", l);
		send("mapListGet");
	}
}




/**
 * @brief TeacherMaps::mapLocalCopy
 * @param data
 */

void TeacherMaps::mapLocalCopy(QVariantMap data)
{
	QString uuid = data.value("uuid").toString();

	QVariantList l;
	l.append(uuid);

	QVariantMap m = db()->execSelectQueryOneRow("SELECT name FROM localmaps WHERE uuid=?", l);

	if (!m.isEmpty()) {
		m_client->sendMessageWarning(tr("Szerkesztés"), tr("Már létezik egy helyi példány a pályából!"));
		return;
	}

	QVariantMap mm = db()->execSelectQueryOneRow("SELECT data, name, md5 FROM maps WHERE uuid=?", l);

	if (mm.isEmpty()) {
		m_client->sendMessageError(tr("Szerkesztés"), tr("Érvénytelen azonosító!"));
		return;
	}

	QVariantMap c;
	c["uuid"] = uuid;
	c["name"] = mm.value("name").toString();
	c["md5"] = mm.value("md5").toString();
	c["data"] = mm.value("data").toByteArray();

	if (db()->execInsertQuery("INSERT INTO localmaps (?k?) VALUES (?)", c) != -1) {
		send("mapListGet");
	}
}



/**
 * @brief TeacherMaps::setIsUploading
 * @param isUploading
 */

void TeacherMaps::setIsUploading(bool isUploading)
{
	if (m_isUploading == isUploading)
		return;

	m_isUploading = isUploading;
	emit isUploadingChanged(m_isUploading);
}






/**
 * @brief TeacherMaps::mapListGet
 * @param jsonData
 */

void TeacherMaps::onMapListGet(QJsonObject jsonData, QByteArray)
{
	m_modelMapList->unselectAll();

	QJsonArray list = jsonData.value("list").toArray();

	QVariantList ret;
	QStringList usedUuids;

	foreach (QJsonValue v, list) {
		QVariantMap m = v.toObject().toVariantMap();
		QString uuid = m.value("uuid").toString();

		usedUuids.append(uuid);

		m["uuid2"] = uuid;
		m["download"] = false;
		m["upload"] = false;
		m["local"] = false;

		QVariantList l;
		l.append(uuid);

		QVariantMap r = db()->execSelectQueryOneRow("SELECT name, md5, COALESCE(LENGTH(data),0) as dataSize FROM maps WHERE uuid=?", l);

		if (r.isEmpty()) {
			m["download"] = true;
		} else {
			if (r.value("md5").toString() != m.value("md5").toString() || m.value("dataSize").toInt() != r.value("dataSize").toInt()) {
				m["download"] = true;
			}
			if (r.value("name").toString() != m.value("name").toString()) {
				QVariantList ll = l;
				ll.prepend(m.value("name").toString());
				db()->execSimpleQuery("UPDATE maps SET name=? WHERE uuid=?", ll);
			}
		}


		QVariantMap rl = db()->execSelectQueryOneRow("SELECT md5 FROM localmaps WHERE uuid=?", l);

		if (!rl.isEmpty()) {
			if (m.value("download").toBool()) {
				QVariantMap m2 = m;
				m2["uuid2"] = uuid+"UP";
				m2["upload"] = true;
				ret.append(m2);
			} else {
				m["upload"] = true;
			}
		}

		ret.append(m);
	}



	QVariantList l = db()->execSelectQuery("SELECT uuid, name, md5, lastModified, COALESCE(LENGTH(data),0) as dataSize FROM localmaps");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();
		QString uuid = m.value("uuid").toString();

		if (usedUuids.contains(uuid))
			continue;

		m["version"] = 0;
		m["binded"] = false;
		m["used"] = false;
		m["uuid2"] = uuid+"UP";
		m["download"] = false;
		m["upload"] = true;
		m["local"] = true;

		ret.append(m);
	}

	m_modelMapList->setVariantList(ret, "uuid2");
}



/**
 * @brief TeacherMaps::onMapUpdated
 * @param jsonData
 */

void TeacherMaps::onMapUpdated(QJsonObject jsonData, QByteArray)
{
	QString uuid = jsonData.value("uuid").toString();
	QString md5 = jsonData.value("md5").toString();

	if (jsonData.contains("error")) {
		m_client->sendMessageError(tr("Hiba"), jsonData.value("error").toString());
		return;
	}

	if (!uuid.isEmpty() && !md5.isEmpty() && (jsonData.value("created").toBool() || jsonData.value("updated").toBool())) {
		QVariantList l;
		l.append(uuid);
		l.append(md5);
		QVariantMap m = db()->execSelectQueryOneRow("SELECT data FROM localmaps WHERE uuid=? AND md5=?", l);

		if (!m.isEmpty()) {
			QVariantList ll;
			ll.append(m.value("data").toByteArray());
			ll.append(md5);
			ll.append(uuid);
			db()->execSimpleQuery("UPDATE maps SET data=?, md5=? WHERE uuid=?", ll);
			db()->execSimpleQuery("DELETE FROM localmaps WHERE uuid=? AND md5=?", l);
		}
	}
}



/**
 * @brief TeacherMaps::onOneDownloadFinished
 * @param item
 * @param data
 */

void TeacherMaps::onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &jsonData)
{
	QVariantMap m;
	m["uuid"] = item.remoteFile;
	m["name"] = jsonData.value("name");
	m["md5"] = jsonData.value("md5");
	m["lastModified"] = jsonData.value("lastModified");
	m["data"] = data;

	db()->execInsertQuery("INSERT OR REPLACE INTO maps (?k?) VALUES (?)", m);
}

