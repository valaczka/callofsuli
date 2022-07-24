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
	, m_modelMapList(new ObjectGenericListModel<MapListObject>(this))
	, m_selectedMapId("")
{
	connect(this, &TeacherMaps::mapListGet, this, &TeacherMaps::onMapListGet);

	CosDb *db = teacherMapsDb(Client::clientInstance(), this);
	addDb(db, false);
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

void TeacherMaps::mapDownloadPrivate(const QVariantMap &data, CosDownloader *downloader, ObjectGenericListModel<MapListObject> *mapModel)
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
		QList<MapListObject*> l = mapModel->find("uuid", uuid);
		if (l.size() != 1)
			continue;

		MapListObject *o = l.at(0);

		downloader->append(o->uuid(),
						   "",
						   o->dataSize(),
						   o->md5(),
						   false,
						   0.0);
	}
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
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Fájl megnyitási hiba"), tr("Nem sikerült megnyitni a fájlt:\n%1").arg(f.fileName()));
		return;
	}

	QByteArray content = f.readAll();

	GameMap *map = GameMap::fromBinaryData(content);
	if (!map) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Fájl hiba"), tr("Érvénytelen formátumú fájl:\n%1").arg(f.fileName()));
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
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Fájl megnyitási hiba"), tr("Nem sikerült megnyitni a fájlt:\n%1").arg(f.fileName()));
		return;
	}

	QByteArray content = f.readAll();

	GameMap *map = GameMap::fromBinaryData(content);
	if (!map) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Fájl hiba"), tr("Érvénytelen formátumú fájl:\n%1").arg(f.fileName()));
		return;
	}

	QString fuuid = map->uuid();

	delete map;

	if (fuuid != m_selectedMapId) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Fájl hiba"), tr("A fájl nem ennek a pályának módosított változata:\n%1").arg(f.fileName()));
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
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen pályaazonosító!"));
		return;
	}

	QByteArray b = m.value("data").toByteArray();

	QFile f(url.toLocalFile());
	if (!f.open(QIODevice::WriteOnly)) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Mentési hiba"), tr("Nem lehet írni a fájlba:\n%1").arg(f.fileName()));
		return;
	}
	f.write(b);
	f.close();

	Client::clientInstance()->sendMessageInfo(tr("Exportálás"), tr("Az exportálás sikerült: %1").arg(f.fileName()));
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

	m_modelMapList->updateJsonArray(ret, "uuid");

	getSelectedMapInfo();
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

void TeacherMaps::getSelectedMapInfo()
{
	MapListObject *map = nullptr;
	QVariantList mlist;
	bool mapReady = false;

	if (!m_selectedMapId.isEmpty()) {
		QList<MapListObject *> mapList = m_modelMapList->find("uuid", m_selectedMapId);
		if (mapList.size() != 1) {
			setSelectedMapId("");
			return;
		}
		map = mapList.at(0);

		if (map->downloaded()) {
			QByteArray data = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", {m_selectedMapId}).value("data").toByteArray();

			if (!data.isEmpty()) {
				GameMap *m = GameMap::fromBinaryData(data);

				if (m) {
					foreach (GameMapMission *mis, m->missions()) {
						mlist.append(mis->name());
					}
					mapReady = true;

					delete m;
				}
			}
		}
	}


	emit mapDataReady(map, mlist, mapReady);
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


/**
 * @brief TeacherMaps::missionNames
 * @param db
 * @return
 */

QVariantMap TeacherMaps::missionNames(CosDb *db)
{
	Q_ASSERT(db);

	QVariantList list = db->execSelectQuery("SELECT uuid, data FROM maps");

	QVariantMap ret;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();

		QByteArray b = m.value("data").toByteArray();

		GameMap *map = GameMap::fromBinaryData(b);
		QVariantMap r;

		if (map) {
			foreach(GameMapMission *mis, map->missions()) {
				const QString missionid = mis->uuid();
				const QString name = mis->name();
				const QString medal = mis->medalImage();

				r[missionid] = QVariantMap({
											   { "name", name },
											   { "medalImage", medal }
										   });
			}
		}

		ret[m.value("uuid").toString()] = r;
	}

	return ret;
}




/**
 * @brief TeacherMaps::gradeList
 * @param list
 * @return
 */

QMap<int, QVariantMap> TeacherMaps::gradeList(const QJsonArray &list)
{
	QMap<int, QVariantMap> ret;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		if (!m.contains("id"))
			continue;

		int id = m.value("id").toInt();
		m.remove("id");

		ret.insert(id, m);
	}

	return ret;
}



/**
 * @brief TeacherMaps::campaignList
 * @param list
 * @return
 */

QJsonArray TeacherMaps::campaignList(const QJsonArray &list, const QVariantMap &missionMap, ObjectGenericListModel<MapListObject> *mapModel)
{
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

						mission = missionMap.value(map).toMap().value(mission).toString();

						if (mission.isEmpty())
							mission = "???";

						if (mapModel) {
							QList<MapListObject*> l = mapModel->find("uuid", map);
							map = l.isEmpty() ? "???" : l.at(0)->name();
						} else {
							map = "???";
						}

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

						const QVariantMap missionInfo = missionMap.value(map).toMap().value(mission).toMap();
						const QString medalImage = missionInfo.value("medalImage").toString();

						mission = missionInfo.value("name").toString();

						if (mission.isEmpty())
							mission = "???";

						if (mapModel) {
							QList<MapListObject*> l = mapModel->find("uuid", map);
							map = l.isEmpty() ? "???" : l.at(0)->name();
						} else {
							map = "???";
						}

						criterion["map"] = map;
						criterion["mission"] = mission;
						criterion["medalImage"] = medalImage;
					}

					o["criterion"] = criterion;
					newCList.append(o);
				}

				o["criteria"] = newCList;
				newGradeArray.append(o);
			}


			// GRADES

			QJsonArray grades = o.value("grades").toArray();
			QJsonArray newGrades;

			foreach (QJsonValue v, grades) {
				QJsonObject o = v.toObject();
				o["forecast"] = o.value("forecast").toBool(false);
				newGrades.append(o);
			}

			grading["xp"] = newXpArray;
			grading["grade"] = newGradeArray;
			o["grading"] = grading;
			o["grades"] = newGrades;
			newAList.append(o);
		}

		o["assignment"] = newAList;
		newList.append(o);
	}

	return newList;
}

