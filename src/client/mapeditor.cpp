/*
 * ---- Call of Suli ----
 *
 * mapeditor.cpp
 *
 * Created on: 2020. 05. 10.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditor
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


#include "mapeditor.h"
#include "gameengine.h"

MapEditor::MapEditor(QObject *parent)
	: MapData("mapEditorDB", parent)
{
	m_databaseInitSql << ":/sql/mapeditor.sql";

	m_mapModified = false;

	setDatabaseFile(Client::standardPath("tmpmapeditor.db"));
}




/**
 * @brief MapEditor::save
 */

void MapEditor::save(const int &mapId, const bool &binaryFormat)
{
	QByteArray data = saveToJson(binaryFormat);

	if (!m_mapOriginalFile.isEmpty()) {
		qDebug() << tr("Adatbázis mentése fájlba: ")+m_mapOriginalFile;
		if (saveToFile(m_mapOriginalFile, data)) {
			emit mapSaved(data, m_mapUuid, mapId);
			setMapModified(false);
		}
	} else {
		qDebug() << tr("Adatbázis mentése");
		emit mapSaved(data, m_mapUuid, mapId);
		setMapModified(false);
	}
}






/**
 * @brief MapEditor::loadFromJson
 * @param data
 * @return
 */

bool MapEditor::loadFromJson(const QByteArray &data, const bool &binaryFormat)
{
	Q_ASSERT (m_client);

	double steps = 1		// undoTables
				   +m_tableNames.count()+2;		// triggers

	double currentStep = 0.0;

	QJsonObject root = MapData::loadFromJson(data, binaryFormat, &steps, &currentStep);

	if (root.isEmpty())
		return false;

	QJsonObject fileinfo = root.value("callofsuli").toObject();

	if (fileinfo.isEmpty()) {
		return false;
	}

	QString uuid = fileinfo.value("uuid").toString();
	setMapUuid(uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);
	setMapTimeCreated(fileinfo.value("timeCreated").toString());

	m_db->execSimpleQuery("INSERT INTO info SELECT '' as title WHERE NOT EXISTS(SELECT * FROM info)");

	m_db->createUndoTables();

	emit mapLoadingProgress(++currentStep/steps);
	QCoreApplication::processEvents();

	QStringList l = m_tableNames;
	l << "objective";
	l << "storage";

	foreach (QString t, l) {
		m_db->createTrigger(t);
		emit mapLoadingProgress(++currentStep/steps);
		QCoreApplication::processEvents();
	}

	emit mapLoaded();

	return true;
}


/**
 * @brief MapEditor::loadFromFile
 * @param filename
 * @return
 */

bool MapEditor::loadFromFile(const QString &filename, const bool &binaryFormat)
{
	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		m_client->sendMessageError(tr("Fájl megnyitási hiba"), tr("A fájl nem található vagy nem olvasható!"), filename);
		return false;
	}

	QByteArray b = f.readAll();

	f.close();

	bool ret = loadFromJson(b, binaryFormat);

	return ret;
}


/**
 * @brief MapEditor::loadFromBackup
 * @return
 */

bool MapEditor::loadFromBackup()
{
	Q_ASSERT(m_client);

	if (!databaseOpen()) {
		return false;
	}

	QVariantMap m = m_db->runSimpleQuery("SELECT originalFile, uuid, timeCreated from mapeditor");
	if (!m.value("error").toBool() && m.value("records").toList().count()) {
		QVariantMap r = m.value("records").toList().value(0).toMap();
		QString filename = r.value("originalFile").toString();
		QString uuid = r.value("uuid").toString();
		QString timeCreated = r.value("timeCreated").toString();

		setMapUuid(uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);
		setMapTimeCreated(timeCreated);
		setMapOriginalFile(filename);

		emit mapLoadedFromBackup();

		return true;
	}

	return false;
}



/**
 * @brief MapEditor::saveToJson
 * @return
 */

QByteArray MapEditor::saveToJson(const bool &binaryFormat)
{
	QJsonObject root;

	QJsonObject fileinfo;
	fileinfo["versionMajor"] = m_client->clientVersionMajor();
	fileinfo["versionMinor"] = m_client->clientVersionMinor();
	fileinfo["uuid"] = m_mapUuid;
	fileinfo["timeCreated"] = m_mapTimeCreated.isEmpty() ? QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT) : m_mapTimeCreated;
	fileinfo["timeModified"] = QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT);

	root["callofsuli"] = fileinfo;

	foreach (QString t, m_tableNames) {
		root[t] = tableToJson(t);
	}

	root["storage"] = tableToJson("storage", true);
	root["objective"] = tableToJson("objective", true);

	QJsonDocument d(root);


	return binaryFormat ? d.toBinaryData() : d.toJson(QJsonDocument::Indented);
}




/**
 * @brief MapEditor::create
 * @return
 */

QByteArray MapEditor::create(const bool &binaryFormat)
{
	QJsonObject root;

	QJsonObject fileinfo;
	fileinfo["versionMajor"] = Client::clientVersionMajor();
	fileinfo["versionMinor"] = Client::clientVersionMinor();
	fileinfo["uuid"] = QUuid::createUuid().toString();
	fileinfo["timeCreated"] = QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT);
	fileinfo["timeModified"] = QDateTime::currentDateTime().toString(DATETIME_JSON_FORMAT);

	root["callofsuli"] = fileinfo;

	QJsonDocument d(root);

	return binaryFormat ? d.toBinaryData() : d.toJson(QJsonDocument::Indented);
}



/**
 * @brief MapEditor::saveToFile
 * @param filename
 * @return
 */

bool MapEditor::saveToFile(const QString &filename, const QByteArray &data)
{
	QSaveFile f;

	f.setFileName(filename);

	if (!f.open(QIODevice::WriteOnly)) {
		qWarning() << f.errorString();
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem sikerült menteni a fájlt!"), filename);
		return false;
	}

	if (data.isNull()) {
		QByteArray b = saveToJson(true);
		f.write(b);
	} else {
		f.write(data);
	}

	if (!f.commit()) {
		m_client->sendMessageError(tr("Mentési hiba"), tr("Nem sikerült menteni a fájlt!"), filename);
		return false;
	}

	return true;
}


/**
 * @brief MapEditor::saveToFile
 * @param url
 * @param data
 * @return
 */









/**
 * @brief MapEditor::updateMapOriginalFile
 * @param filename
 */

void MapEditor::updateMapOriginalFile(const QString &filename)
{
	QVariantMap l;
	l["originalFile"] = filename;
	m_db->execInsertQuery("INSERT OR REPLACE INTO mapeditor (?k?) VALUES(?)", l);
	setMapOriginalFile(filename);
}


/**
 * @brief MapEditor::updateMapServerId
 * @param serverId
 * @param mapId
 */

void MapEditor::updateMapServerId(const int &serverId, const int &mapId)
{
	QVariantMap l;
	l["serverid"] = serverId;
	l["mapid"] = mapId;
	m_db->execInsertQuery("INSERT OR REPLACE INTO mapeditor (?k?) VALUES(?)", l);
}



void MapEditor::setMapOriginalFile(QString mapOriginalFile)
{
	if (m_mapOriginalFile == mapOriginalFile)
		return;

	m_mapOriginalFile = mapOriginalFile;
	emit mapOriginalFileChanged(m_mapOriginalFile);
}

void MapEditor::setMapModified(bool mapModified)
{
	if (m_mapModified == mapModified)
		return;

	m_mapModified = mapModified;
	emit mapModifiedChanged(m_mapModified);
}





/**
 * @brief MapEditor::getInfo
 * @return
 */

QVariantMap MapEditor::infoGet()
{
	QVariantMap ret;
	m_db->execSelectQueryOneRow("SELECT title FROM info", QVariantList(), &ret);

	if (!ret.contains("title"))
		ret["title"] = m_mapOriginalFile;

	return ret;
}



/**
 * @brief MapEditor::updateInfo
 * @param map
 */

void MapEditor::infoUpdate(const QVariantMap &map)
{
	m_db->execUpdateQuery("UPDATE INFO SET ?", map);
	setMapModified(true);
}






/**
 * @brief MapEditor::missionUpdate
 * @param id
 * @param params
 * @return
 */

bool MapEditor::missionUpdate(const int &id, const QVariantMap &params)
{
	QVariantMap bind;
	bind[":id"] = id;

	if (m_db->execUpdateQuery("UPDATE mission SET ? WHERE id=:id", params, bind)) {
		emit missionUpdated(id);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::missionAdd
 * @param params
 * @return
 */

int MapEditor::missionAdd(const QVariantMap &params)
{
	QVariantMap p = params;

	if (!p.contains("uuid"))
		p["uuid"] = QUuid::createUuid().toString();

	int id = m_db->execInsertQuery("INSERT INTO mission (?k?) values (?)", p);
	if (id != -1) {
		emit missionListUpdated(-1);
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief MapEditor::missionLevelAdd
 * @param id
 * @param params
 * @return
 */

int MapEditor::missionLevelAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT INTO missionLevel (?k?) values (?)", params);
	if (id != -1) {
		emit missionUpdated(params.value("missionid", -1).toInt());
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief MapEditor::missionLevelUpdate
 * @param id
 * @param params
 * @return
 */

bool MapEditor::missionLevelUpdate(const int &id, const int &missionId, const QVariantMap &params)
{
	QVariantMap bind;
	bind[":id"] = id;
	bind[":missionid"] = missionId;

	if (m_db->execUpdateQuery("UPDATE missionLevel SET ? WHERE id=:id AND missionid=:missionid", params, bind)) {
		emit missionUpdated(missionId);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::missionLevelRemove
 * @param id
 * @return
 */

bool MapEditor::missionLevelRemove(const int &id, const int &missionId)
{
	QVariantList l;
	l << id;
	l << missionId;
	bool r = m_db->execSimpleQuery("DELETE FROM missionLevel WHERE id=? AND missionid=?", l);
	if (r) {
		emit missionUpdated(missionId);
		setMapModified(true);
	}
	return r;
}

/**
 * @brief MapEditor::missionStorageAdd
 * @param params
 * @return
 */

int MapEditor::missionStorageAdd(const QVariantMap &params)
{
	int missionId = params.value("missionid", -1).toInt();
	int num = params.value("num", -1).toInt();

	if (num == -1) {
		QVariantMap m;
		QVariantList l;
		l << missionId;
		m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM bindMissionStorage WHERE missionid=?", l, &m);
		if (m.contains("num"))
			num = m.value("num").toInt();
		else
			num = 1;
	}

	QVariantMap p2 = params;
	p2["num"] = num;

	int id = m_db->execInsertQuery("INSERT OR IGNORE INTO bindMissionStorage (?k?) values (?)", p2);

	if (id != -1) {
		emit missionUpdated(missionId);
		emit storageListUpdated(missionId, -1);
		setMapModified(true);
		return id;
	}

	return -1;
}



/**
 * @brief MapEditor::missionStorageUpdate
 * @param id
 * @param missionId
 * @param params
 * @return
 */

bool MapEditor::missionStorageUpdate(const int &id, const int &missionId, const QVariantMap &params)
{
	QVariantMap bind;
	bind[":id"] = id;
	bind[":missionid"] = missionId;

	if (m_db->execUpdateQuery("UPDATE bindMissionStorage SET ? WHERE id=:id AND missionid=:missionid", params, bind)) {
		emit missionUpdated(missionId);
		emit storageListUpdated(missionId, -1);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::missionStorageRemove
 * @param id
 * @param missionId
 * @return
 */


bool MapEditor::missionStorageRemove(const int &missionId, const int &storageId)
{
	QVariantList l;
	l << storageId;
	l << missionId;
	bool r = m_db->execSimpleQuery("DELETE FROM bindMissionStorage WHERE storageid=? AND missionid=?", l);
	if (r) {
		emit missionUpdated(missionId);
		emit storageUpdated(storageId);
		setMapModified(true);
	}
	return r;
}


/**
 * @brief MapEditor::missionIntroAdd
 * @param missionId
 * @param introId
 * @param isOutro
 * @return
 */

bool MapEditor::missionIntroAdd(const int &id, const int &introId, const bool &isOutro)
{
	QVariantMap m;
	m["missionid"] = id;
	m["introid"] = introId;
	m["outro"] = isOutro;

	int r = m_db->execInsertQuery("INSERT INTO bindIntroMission (?k?) VALUES (?)", m);

	if (r != -1) {
		emit missionUpdated(id);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::missionCampaignListSet
 * @param id
 * @param campaignIdList
 * @return
 */

bool MapEditor::missionCampaignListSet(const int &id, const QVariantList &campaignIdList)
{
	QStringList vl;
	QStringList il;
	foreach (QVariant p, campaignIdList) {
		vl << QString("(%1)").arg(p.toInt());
		il << QString::number(p.toInt());
	}

	QVariantList params;
	params << id;

	if (campaignIdList.isEmpty()) {
		return m_db->execSimpleQuery("DELETE FROM bindCampaignMission WHERE missionid=?", params);
	}

	bool r1 = m_db->execSimpleQuery("DELETE FROM bindCampaignMission WHERE missionid=? AND campaignid NOT IN ("+il.join(",")+")", params);

	params << id;

	bool r2 = m_db->execSimpleQuery("INSERT INTO bindCampaignMission(campaignid, missionid, num) "
									"SELECT T.id, "
									"? as missionid, "
									"COALESCE((SELECT MAX(num)+1 FROM bindCampaignMission WHERE campaignid=T.id), 1) as num "
									"FROM (SELECT column1 as id FROM (values "+vl.join(",")+" ) EXCEPT SELECT campaignid from bindCampaignMission WHERE missionid=?) T",
									params);

	emit missionUpdated(id);
	setMapModified(true);

	return (r1 && r2);
}


/**
 * @brief MapEditor::missionRemove
 * @param id
 * @return
 */

bool MapEditor::missionRemove(const int &id)
{
	QVariantList l;
	l << id;
	bool ret = m_db->execSimpleQuery("DELETE FROM mission WHERE id=?", l);
	if (ret) {
		emit missionListUpdated(-1);
		setMapModified(true);
	}

	return ret;
}




/**
 * @brief MapEditor::summaryAdd
 * @param params
 * @return
 */

int MapEditor::summaryAdd(const int &campaignId)
{
	QVariantMap m;
	m["campaignid"] = campaignId;
	m["uuid"] = QUuid::createUuid().toString();

	int id = m_db->execInsertQuery("INSERT INTO summary (?k?) values (?)", m);
	if (id != -1) {
		emit campaignUpdated(campaignId);
		setMapModified(true);
	}

	return id;
}


/**
 * @brief MapEditor::summaryLevelAdd
 * @param params
 * @return
 */

int MapEditor::summaryLevelAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT INTO summaryLevel (?k?) values (?)", params);
	if (id != -1) {
		emit summaryUpdated(params.value("summaryid", -1).toInt());
		setMapModified(true);
	}

	return id;
}


/**
 * @brief MapEditor::summaryLevelUpdate
 * @param id
 * @param missionId
 * @param params
 * @return
 */

bool MapEditor::summaryLevelUpdate(const int &id, const int &summaryId, const QVariantMap &params)
{
	QVariantMap bind;
	bind[":id"] = id;
	bind[":summaryid"] = summaryId;

	if (m_db->execUpdateQuery("UPDATE summaryLevel SET ? WHERE id=:id AND summaryid=:summaryid", params, bind)) {
		emit summaryUpdated(summaryId);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::summaryLevelRemove
 * @param id
 * @param missionId
 * @return
 */

bool MapEditor::summaryLevelRemove(const int &id, const int &summaryId)
{
	QVariantList l;
	l << id;
	l << summaryId;
	bool r = m_db->execSimpleQuery("DELETE FROM summaryLevel WHERE id=? AND summaryid=?", l);
	if (r) {
		emit summaryUpdated(summaryId);
		setMapModified(true);
	}
	return r;
}


/**
 * @brief MapEditor::summaryStorageAdd
 * @param params
 * @return
 */

int MapEditor::summaryStorageAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT OR IGNORE INTO bindSummaryStorage (?k?) values (?)", params);
	if (id != -1) {
		int summaryId = params.value("summaryid", -1).toInt();
		emit summaryUpdated(summaryId);
		emit storageListUpdated(-1, summaryId);
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief MapEditor::summaryStorageRemove
 * @param id
 * @param missionId
 * @return
 */


bool MapEditor::summaryStorageRemove(const int &summaryId, const int &storageId)
{
	QVariantList l;
	l << storageId;
	l << summaryId;
	bool r = m_db->execSimpleQuery("DELETE FROM bindSummaryStorage WHERE storageid=? AND summaryid=?", l);
	if (r) {
		emit summaryUpdated(summaryId);
		emit storageUpdated(storageId);
		setMapModified(true);
	}
	return r;
}




/**
 * @brief MapEditor::summaryIntroAdd
 * @param summaryId
 * @param introId
 * @param isOutro
 * @return
 */

bool MapEditor::summaryIntroAdd(const int &id, const int &introId, const bool &isOutro)
{
	QVariantMap m;
	m["summaryid"] = id;
	m["introid"] = introId;
	m["outro"] = isOutro;

	int r = m_db->execInsertQuery("INSERT INTO bindIntroSummary (?k?) VALUES (?)", m);

	if (r != -1) {
		emit summaryUpdated(id);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::summaryRemove
 * @param id
 * @return
 */

bool MapEditor::summaryRemove(const int &id)
{
	QVariantList l;
	l << id;
	bool ret = m_db->execSimpleQuery("DELETE FROM summary WHERE id=?", l);
	if (ret) {
		setMapModified(true);
	}

	return ret;
}





/**
 * @brief MapEditor::storageAdd
 * @param params
 * @return
 */

int MapEditor::storageAdd(const QVariantMap &params, const int &missionId, const int &summaryId)
{
	int id = m_db->execInsertQuery("INSERT INTO storage (?k?) values (?)", params);

	if (id != -1) {
		if (missionId != -1) {
			QVariantMap m;
			QVariantList l;
			int num = 1;
			l << missionId;
			m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM bindMissionStorage WHERE missionid=?", l, &m);
			if (m.contains("num"))
				num = m.value("num").toInt();

			QVariantMap p2;
			p2["missionid"] = missionId;
			p2["storageid"] = id;
			p2["num"] = num;

			if (m_db->execInsertQuery("INSERT OR IGNORE INTO bindMissionStorage (?k?) values (?)", p2) != -1)
				emit missionUpdated(missionId);
		} else if (summaryId != -1) {
			QVariantMap p2;
			p2["summaryid"] = summaryId;
			p2["storageid"] = id;

			if (m_db->execInsertQuery("INSERT OR IGNORE INTO bindSummaryStorage (?k?) values (?)", p2) != -1)
				emit summaryUpdated(summaryId);
		}

		emit storageListUpdated(missionId, summaryId);
		setMapModified(true);
		return id;
	}

	return -1;
}






bool MapEditor::storageUpdate(const int &id, const QVariantMap &params, const QJsonObject &jsonData, const int &missionId, const int &summaryId)
{
	QVariantMap bind;
	bind[":id"] = id;

	QVariantMap newParams = params;

	if (!jsonData.isEmpty()) {
		QJsonDocument doc(jsonData);
		newParams["data"] = QString(doc.toJson(QJsonDocument::Compact));
	}

	if (m_db->execUpdateQuery("UPDATE storage SET ? WHERE id=:id", newParams, bind)) {
		emit storageUpdated(id);
		emit storageListUpdated(missionId, summaryId);
		setMapModified(true);
		return true;
	}

	return false;
}




/**
 * @brief MapEditor::storageMissionListSet
 * @param id
 * @param missionIdList
 * @return
 */

bool MapEditor::storageMissionListSet(const int &id, const QVariantList &missionIdList)
{
	QStringList vl;
	QStringList il;
	foreach (QVariant p, missionIdList) {
		vl << QString("(%1)").arg(p.toInt());
		il << QString::number(p.toInt());
	}

	QVariantList params;
	params << id;

	if (missionIdList.isEmpty()) {
		return m_db->execSimpleQuery("DELETE FROM bindMissionStorage WHERE storageid=?", params);
	}

	bool r1 = m_db->execSimpleQuery("DELETE FROM bindMissionStorage WHERE storageid=? AND missionid NOT IN ("+il.join(",")+")", params);

	params << id;

	bool r2 = m_db->execSimpleQuery("INSERT INTO bindMissionStorage(missionid, storageid, num) "
									"SELECT T.id as missionid, "
									"? as storageid, "
									"COALESCE((SELECT MAX(num)+1 FROM bindMissionStorage WHERE missionid=T.id), 1) as num "
									"FROM (SELECT column1 as id FROM (values "+vl.join(",")+" ) EXCEPT SELECT missionid from bindMissionStorage WHERE storageid=?) T",
									params);

	emit storageUpdated(id);
	setMapModified(true);

	return (r1 && r2);
}



/**
 * @brief MapEditor::storageCampaignListSet
 * @param id
 * @param missionIdList
 * @return
 */

bool MapEditor::storageSummaryListSet(const int &id, const QVariantList &summaryIdList)
{
	QStringList vl;
	QStringList il;
	foreach (QVariant p, summaryIdList) {
		vl << QString("(%1)").arg(p.toInt());
		il << QString::number(p.toInt());
	}

	QVariantList params;
	params << id;

	if (summaryIdList.isEmpty()) {
		return m_db->execSimpleQuery("DELETE FROM bindSummaryStorage WHERE storageid=?", params);
	}

	bool r1 = m_db->execSimpleQuery("DELETE FROM bindSummaryStorage WHERE storageid=? AND summaryid NOT IN ("+il.join(",")+")", params);

	params << id;

	bool r2 = m_db->execSimpleQuery("INSERT INTO bindSummaryStorage(summaryid, storageid) "
									"SELECT T.id as summaryid, ? as storageid "
									"FROM (SELECT column1 as id FROM (values "+vl.join(",")+" ) EXCEPT SELECT summaryid from bindSummaryStorage WHERE storageid=?) T",
									params);

	emit storageUpdated(id);
	setMapModified(true);

	return (r1 && r2);
}


/**
 * @brief MapEditor::storageRemove
 * @param id
 * @return
 */

bool MapEditor::storageRemove(const int &id, const int &missionId, const int &summaryId)
{
	QVariantList l;
	l << id;
	bool ret = m_db->execSimpleQuery("DELETE FROM storage WHERE id=?", l);
	if (ret) {
		emit storageListUpdated(missionId, summaryId);
		setMapModified(true);
	}

	return ret;
}




/**
 * @brief MapEditor::introAdd
 * @param params
 * @return
 */

int MapEditor::introAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT INTO intro (?k?) values (?)", params);
	if (id != -1) {
		emit introListUpdated(-1, IntroUndefined);
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief MapEditor::introUpdate
 * @param id
 * @param params
 * @param parentId
 * @param type
 * @return
 */

bool MapEditor::introUpdate(const int &id, const QVariantMap &params, const int &parentId, const MapEditor::IntroType &type)
{
	QVariantMap bind;
	bind[":id"] = id;

	if (m_db->execUpdateQuery("UPDATE intro SET ? WHERE id=:id", params, bind)) {
		emit introUpdated(id);
		emit introListUpdated(parentId, type);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::introRemove
 * @param id
 * @return
 */

bool MapEditor::introRemove(const int &id)
{
	QVariantList l;
	l << id;
	bool ret = m_db->execSimpleQuery("DELETE FROM intro WHERE id=?", l);
	if (ret) {
		emit introListUpdated(-1, IntroUndefined);
		setMapModified(true);
	}

	return ret;
}







/**
 * @brief MapEditor::objectiveAdd
 * @param params
 * @return
 */

int MapEditor::objectiveAdd(const QVariantMap &params, const int &missionId, const int &summaryId)
{
	int id = m_db->execInsertQuery("INSERT INTO objective (?k?) values (?)", params);
	if (id != -1) {
		emit storageListUpdated(missionId, summaryId);
		setMapModified(true);
		return id;
	}

	return -1;
}





/**
 * @brief MapEditor::objectiveRemove
 * @param id
 * @return
 */

bool MapEditor::objectiveRemove(const int &id, const int &missionId, const int &summaryId)
{
	QVariantList l;
	l << id;

	bool ret = m_db->execSimpleQuery("DELETE FROM objective WHERE id=?", l);
	if (ret) {
		emit storageListUpdated(missionId, summaryId);
		setMapModified(true);
	}

	return ret;
}


/**
 * @brief MapEditor::objectiveUpdate
 * @param id
 * @param level
 * @param isSummary
 * @return
 */

bool MapEditor::objectiveUpdate(const int &id, const QVariantMap &params, const QJsonObject &jsonData, const int &missionId, const int &summaryId)
{
	QVariantMap bind;
	bind[":id"] = id;

	QVariantMap newParams = params;

	if (!jsonData.isEmpty()) {
		QJsonDocument doc(jsonData);
		newParams["data"] = QString(doc.toJson(QJsonDocument::Compact));
	}

	if (m_db->execUpdateQuery("UPDATE objective SET ? WHERE id=:id", newParams, bind)) {
		emit objectiveUpdated(id);

		emit storageListUpdated(missionId, summaryId);

		setMapModified(true);
		return true;
	}

	return false;
}






/**
 * @brief MapEditor::updateCampaign
 * @param id
 * @param params
 * @return
 */

bool MapEditor::campaignUpdate(const int &id, const QVariantMap &params)
{
	QVariantMap bind;
	bind[":id"] = id;

	if (m_db->execUpdateQuery("UPDATE campaign SET ? WHERE id=:id", params, bind)) {
		emit campaignUpdated(id);
		emit campaignListUpdated();
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::campaignAdd
 * @param params
 * @return
 */

int MapEditor::campaignAdd(const QVariantMap &params)
{
	QVariantMap p = params;

	int num = 1;

	if (p.contains("num")) {
		num = p.value("num").toInt();
	} else {
		QVariantMap m;
		m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM campaign", QVariantList(), &m);
		if (m.contains("num"))
			num = m.value("num").toInt();
	}

	p["num"] = num;

	int id = m_db->execInsertQuery("INSERT INTO campaign (?k?) values (?)", p);
	if (id != -1) {
		emit campaignListUpdated();
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief MapEditor::campaignAddMission
 * @param campaignId
 * @param missionId
 * @param num
 * @return
 */

bool MapEditor::campaignMissionAdd(const int &id, const int &missionId, const int &num)
{
	int realnum = num;
	if (num == -1) {
		QVariantMap m;
		QVariantList l;
		l << id;
		m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM bindCampaignMission WHERE campaignid=?", l, &m);
		if (m.contains("num"))
			realnum = m.value("num").toInt();
		else
			realnum = 1;
	}

	QVariantMap m;
	m["campaignid"] = id;
	m["missionid"] = missionId;
	m["num"] = realnum;

	int r = m_db->execInsertQuery("INSERT INTO bindCampaignMission (?k?) VALUES (?)", m);

	if (r != -1) {
		emit campaignUpdated(id);
		setMapModified(true);
		return true;
	}

	return false;
}



/**
 * @brief MapEditor::campaignSummaryAdd
 * @param campaignId
 * @return
 */

int MapEditor::campaignSummaryAdd(const int &id)
{
	QVariantMap m;
	m["campaignid"] = id;

	int r = m_db->execInsertQuery("INSERT INTO summary (?k?) VALUES (?)", m);

	if (r != -1) {
		emit campaignUpdated(id);
		setMapModified(true);
	}

	return r;
}


/**
 * @brief MapEditor::campaignIntroAdd
 * @param campaignId
 * @param isOutro
 * @return
 */

bool MapEditor::campaignIntroAdd(const int &id, const int &introId, const bool &isOutro)
{
	QVariantMap m;
	m["campaignid"] = id;
	m["introid"] = introId;
	m["outro"] = isOutro;

	int r = m_db->execInsertQuery("INSERT INTO bindIntroCampaign (?k?) VALUES (?)", m);

	if (r != -1) {
		emit campaignUpdated(id);
		setMapModified(true);
		return true;
	}

	return false;

}


/**
 * @brief MapEditor::campaignLockSet
 * @param id
 * @param lockIdList
 * @return
 */

bool MapEditor::campaignLockSet(const int &id, const QVariantList &lockIdList)
{
	QStringList vl;
	QStringList il;
	foreach (QVariant p, lockIdList) {
		vl << QString("(%1)").arg(p.toInt());
		il << QString::number(p.toInt());
	}

	QVariantList params;
	params << id;

	if (lockIdList.isEmpty()) {
		return m_db->execSimpleQuery("DELETE FROM campaignLock WHERE campaignId=?", params);
	}

	bool r1 = m_db->execSimpleQuery("DELETE FROM campaignLock WHERE campaignId=? AND lockId NOT IN ("+il.join(",")+")", params);

	params << id;

	bool r2 = m_db->execSimpleQuery("INSERT INTO campaignLock(lockId, campaignId) "
									"SELECT T.id as lockId, ? as campaignId "
									"FROM (SELECT column1 as id FROM (values "+vl.join(",")+" ) EXCEPT SELECT lockId from campaignLock WHERE campaignId=?) T",
									params);

	emit campaignUpdated(id);
	setMapModified(true);

	return (r1 && r2);
}


/**
 * @brief MapEditor::campaignRemove
 * @param id
 * @return
 */

bool MapEditor::campaignRemove(const int &id)
{
	QVariantList l;
	l << id;
	bool ret = m_db->execSimpleQuery("DELETE FROM campaign WHERE id=?", l);
	if (ret) {
		emit campaignListUpdated();
		setMapModified(true);
	}

	return ret;
}




void MapEditor::setMapTimeCreated(QString mapTimeCreated)
{
	if (m_mapTimeCreated == mapTimeCreated)
		return;

	m_mapTimeCreated = mapTimeCreated;
	emit mapTimeCreatedChanged(m_mapTimeCreated);
}



void MapEditor::setMapUuid(QString mapUuid)
{
	if (m_mapUuid == mapUuid)
		return;

	m_mapUuid = mapUuid;
	emit mapUuidChanged(m_mapUuid);
}



