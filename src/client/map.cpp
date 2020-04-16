/*
 * ---- Call of Suli ----
 *
 * map.cpp
 *
 * Created on: 2020. 03. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Map
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

#include "map.h"
#include "cosclient.h"

#define DATETIME_JSON_FORMAT QString("yyyy-MM-dd hh:mm:ss")

Map::Map(QObject *parent)
	: AbstractDbActivity("mapDB", parent)
{
	m_tableNames << "info"
				 << "intro"
				 << "campaign"
				 << "campaignLock"
				 << "mission"
				 << "missionLevel"
				 << "summary"
				 << "summaryLevel"
				 << "bindCampaignMission"
				 << "chapter"
				 << "bindMissionChapter"
				 << "bindIntroCampaign"
				 << "bindIntroMission"
				 << "bindIntroSummary"
				 << "bindIntroChapter"
				 << "bindSummaryChapter";

	m_mapType = MapInvalid;
	m_mapModified = false;
}


/**
 * @brief Map::~Map
 */

Map::~Map()
{
	if (m_db->isOpen())
		m_db->close();

	if (QFile::exists(m_databaseFile)) {
		qDebug() << tr("Remove temporary map file ")+m_databaseFile << QFile::remove(m_databaseFile);
	}
}





/**
 * @brief Map::save
 */

void Map::save(const int &mapId, const bool &binaryFormat)
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
 * @brief Map::loadFromJson
 * @param data
 * @return
 */

bool Map::loadFromJson(const QByteArray &data, const bool &binaryFormat)
{
	Q_ASSERT (m_client);

	if (!databaseOpen() || !databasePrepare()) {
		return false;
	}

	QJsonDocument doc = binaryFormat ? QJsonDocument::fromBinaryData(data) : QJsonDocument::fromJson(data);

	if (doc.isNull())
		return false;

	QJsonObject root = doc.object();

	QJsonObject fileinfo = root["callofsuli"].toObject();

	if (fileinfo.isEmpty()) {
		return false;
	}

	foreach (QString t, m_tableNames) {
		if (!JsonToTable(root[t].toArray(), t, false))
			return false;
	}

	if (!JsonToTable(root["storage"].toArray(), "storage", true))
		return false;

	if (!JsonToTable(root["objective"].toArray(), "objective", true))
		return false;

	QString uuid = fileinfo["uuid"].toString();
	setMapUuid(uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);
	setMapTimeCreated(fileinfo["timeCreated"].toString());

	m_db->execSimpleQuery("INSERT INTO info SELECT '' as title WHERE NOT EXISTS(SELECT * FROM info)");

	emit mapLoaded();

	return true;
}


/**
 * @brief Map::loadFromFile
 * @param filename
 * @return
 */

bool Map::loadFromFile(const QString &filename, const bool &binaryFormat)
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
 * @brief Map::loadFromBackup
 * @return
 */

bool Map::loadFromBackup()
{
	Q_ASSERT(m_client);

	if (!databaseOpen()) {
		return false;
	}

	QVariantMap m = m_db->runSimpleQuery("SELECT originalFile, uuid, timeCreated from mapeditor");
	if (!m["error"].toBool() && m["records"].toList().count()) {
		QVariantMap r = m["records"].toList().value(0).toMap();
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
 * @brief Map::saveToJson
 * @return
 */

QByteArray Map::saveToJson(const bool &binaryFormat)
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
 * @brief Map::create
 * @return
 */

QByteArray Map::create(const bool &binaryFormat)
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
 * @brief Map::saveToFile
 * @param filename
 * @return
 */

bool Map::saveToFile(const QString &filename, const QByteArray &data)
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
 * @brief Map::saveToFile
 * @param url
 * @param data
 * @return
 */









/**
 * @brief Map::updateMapOriginalFile
 * @param filename
 */

void Map::updateMapOriginalFile(const QString &filename)
{
	QVariantMap l;
	l["originalFile"] = filename;
	m_db->execInsertQuery("INSERT OR REPLACE INTO mapeditor (?k?) VALUES(?)", l);
	setMapOriginalFile(filename);
}


/**
 * @brief Map::updateMapServerId
 * @param serverId
 * @param mapId
 */

void Map::updateMapServerId(const int &serverId, const int &mapId)
{
	QVariantMap l;
	l["serverid"] = serverId;
	l["mapid"] = mapId;
	m_db->execInsertQuery("INSERT OR REPLACE INTO mapeditor (?k?) VALUES(?)", l);
}



void Map::setMapOriginalFile(QString mapOriginalFile)
{
	if (m_mapOriginalFile == mapOriginalFile)
		return;

	m_mapOriginalFile = mapOriginalFile;
	emit mapOriginalFileChanged(m_mapOriginalFile);
}

void Map::setMapModified(bool mapModified)
{
	if (m_mapModified == mapModified)
		return;

	m_mapModified = mapModified;
	emit mapModifiedChanged(m_mapModified);
}


/**
 * @brief Map::getInfo
 * @return
 */

QVariantMap Map::infoGet()
{
	QVariantMap ret;
	m_db->execSelectQueryOneRow("SELECT title FROM info", QVariantList(), &ret);

	if (!ret.contains("title"))
		ret["title"] = m_mapOriginalFile;

	return ret;
}


/**
 * @brief Map::updateInfo
 * @param map
 */

void Map::infoUpdate(const QVariantMap &map)
{
	m_db->execUpdateQuery("UPDATE INFO SET ?", map);
	setMapModified(true);
}


/**
 * @brief Map::getCampaign
 * @param id
 * @return
 */

QVariantMap Map::campaignGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	m_db->execSelectQueryOneRow("SELECT num, name FROM campaign WHERE id=?", l, &map);

	m_db->execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
								"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroCampaign "
								"LEFT JOIN intro ON (bindIntroCampaign.introid=intro.id) "
								"WHERE campaignid=? AND outro=false", l, &map);

	m_db->execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
								"levelMin as outroLevelMin, levelMax as outroLevelMax FROM bindIntroCampaign "
								"LEFT JOIN intro ON (bindIntroCampaign.introid=intro.id) "
								"WHERE campaignid=? AND outro=true", l, &map);

	m_db->execSelectQueryOneRow("SELECT id as summaryId FROM summary WHERE campaignid=?", l, &map);

	if (!map.contains("introId"))
		map["introId"] = -1;

	if (!map.contains("outroId"))
		map["outroId"] = -1;

	if (!map.contains("summaryId"))
		map["summaryId"] = -1;



	QVariantList locks;

	m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM campaignLock "
						  "LEFT JOIN campaign ON (campaignLock.lockId=campaign.id) "
						  "WHERE campaignId=?", l, &locks);

	map["locks"] = locks;

	return map;
}



/**
 * @brief Map::getCampaignList
 * @return
 */


QVariantList Map::campaignListGet()
{
	QVariantList list;
	m_db->execSelectQuery("SELECT id, num, name FROM campaign ORDER BY num", QVariantList(), &list);
	return list;
}



/**
 * @brief Map::missionGet
 * @param id
 * @return
 */

QVariantMap Map::missionGet(const int &id, const bool &isSummary)
{
	QVariantList l;
	l << id;
	QVariantMap map;

	if (isSummary) {
		m_db->execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
									"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroSummary "
									"LEFT JOIN intro ON (bindIntroSummary.introid=intro.id) "
									"WHERE summaryid=? AND outro=false", l, &map);

		m_db->execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
									"levelMin as outroLevelMin, levelMax as outroLevelMax FROM bindIntroSummary "
									"LEFT JOIN intro ON (bindIntroSummary.introid=intro.id) "
									"WHERE summaryid=? AND outro=true", l, &map);
	} else {
		m_db->execSelectQueryOneRow("SELECT name FROM mission WHERE id=?", l, &map);

		m_db->execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
									"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroMission "
									"LEFT JOIN intro ON (bindIntroMission.introid=intro.id) "
									"WHERE missionid=? AND outro=false", l, &map);

		m_db->execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
									"levelMin as outroLevelMin, levelMax as outroLevelMax FROM bindIntroMission "
									"LEFT JOIN intro ON (bindIntroMission.introid=intro.id) "
									"WHERE missionid=? AND outro=true", l, &map);
	}

	if (!map.contains("introId"))
		map["introId"] = -1;

	if (!map.contains("outroId"))
		map["outroId"] = -1;



	QVariantList campaigns;

	if (isSummary) {
		m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM summary "
							  "LEFT JOIN campaign ON (summary.campaignid=campaign.id) "
							  "WHERE summary.id=?", l, &campaigns);
	} else {
		m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM bindCampaignMission "
							  "LEFT JOIN campaign ON (bindCampaignMission.campaignid=campaign.id) "
							  "WHERE missionid=? ORDER BY campaign.num", l, &campaigns);
	}


	map["campaigns"] = campaigns;


	QVariantList levels;

	if (isSummary) {
		m_db->execSelectQuery("SELECT id, level, sec, hp, 0 as mode FROM summaryLevel WHERE summaryid=? ORDER BY level", l, &levels);
	} else {
		m_db->execSelectQuery("SELECT id, level, sec, hp, mode FROM missionLevel WHERE missionid=? ORDER BY level", l, &levels);
	}

	map["levels"] = levels;



	QVariantList chapters = chapterListGet(isSummary ? -1 : id, isSummary ? id : -1);

	map["chapters"] = chapters;

	return map;
}




/**
 * @brief Map::getMissionList
 * @param missionId
 * @return
 */

QVariantList Map::missionListGet(const int &campaignId)
{
	QVariantList list;

	if (campaignId != -1) {
		QVariantList l;
		l << campaignId;
		m_db->execSelectQuery("SELECT missionid as id, name, num FROM bindCampaignMission "
							  "LEFT JOIN mission ON (mission.id=bindCampaignMission.missionid) "
							  "WHERE campaignid=? "
							  "ORDER BY num", l, &list);
	} else {
		m_db->execSelectQuery("SELECT id, name, 0 as num FROM mission ORDER BY name", QVariantList(), &list);
	}

	return list;
}


/**
 * @brief Map::missionUpdate
 * @param id
 * @param params
 * @return
 */

bool Map::missionUpdate(const int &id, const QVariantMap &params, const int &campaignId)
{
	QVariantMap bind;
	bind[":id"] = id;

	if (m_db->execUpdateQuery("UPDATE mission SET ? WHERE id=:id", params, bind)) {
		emit missionUpdated(id);
		emit missionListUpdated(campaignId);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief Map::missionAdd
 * @param params
 * @return
 */

int Map::missionAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT INTO mission (?k?) values (?)", params);
	if (id != -1) {
		emit missionListUpdated(-1);
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief Map::missionLevelAdd
 * @param id
 * @param params
 * @return
 */

int Map::missionLevelAdd(const QVariantMap &params)
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
 * @brief Map::missionLevelUpdate
 * @param id
 * @param params
 * @return
 */

bool Map::missionLevelUpdate(const int &id, const int &missionId, const QVariantMap &params)
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
 * @brief Map::missionLevelRemove
 * @param id
 * @return
 */

bool Map::missionLevelRemove(const int &id, const int &missionId)
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
 * @brief Map::missionChapterAdd
 * @param params
 * @return
 */

int Map::missionChapterAdd(const QVariantMap &params)
{
	int missionId = params.value("missionid", -1).toInt();
	int num = params.value("num", -1).toInt();

	if (num == -1) {
		QVariantMap m;
		QVariantList l;
		l << missionId;
		m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM bindMissionChapter WHERE missionid=?", l, &m);
		if (m.contains("num"))
			num = m["num"].toInt();
		else
			num = 1;
	}

	QVariantMap p2 = params;
	p2["num"] = num;

	int id = m_db->execInsertQuery("INSERT OR IGNORE INTO bindMissionChapter (?k?) values (?)", p2);

	if (id != -1) {
		emit missionUpdated(missionId);
		emit chapterListUpdated(missionId, -1);
		setMapModified(true);
		return id;
	}

	return -1;
}



/**
 * @brief Map::missionChapterUpdate
 * @param id
 * @param missionId
 * @param params
 * @return
 */

bool Map::missionChapterUpdate(const int &id, const int &missionId, const QVariantMap &params)
{
	QVariantMap bind;
	bind[":id"] = id;
	bind[":missionid"] = missionId;

	if (m_db->execUpdateQuery("UPDATE bindMissionChapter SET ? WHERE id=:id AND missionid=:missionid", params, bind)) {
		emit missionUpdated(missionId);
		emit chapterListUpdated(missionId, -1);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief Map::missionChapterRemove
 * @param id
 * @param missionId
 * @return
 */


bool Map::missionChapterRemove(const int &missionId, const int &chapterId)
{
	QVariantList l;
	l << chapterId;
	l << missionId;
	bool r = m_db->execSimpleQuery("DELETE FROM bindMissionChapter WHERE chapterid=? AND missionid=?", l);
	if (r) {
		emit missionUpdated(missionId);
		emit chapterUpdated(chapterId);
		setMapModified(true);
	}
	return r;
}


/**
 * @brief Map::missionIntroAdd
 * @param missionId
 * @param introId
 * @param isOutro
 * @return
 */

bool Map::missionIntroAdd(const int &id, const int &introId, const bool &isOutro)
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
 * @brief Map::missionCampaignListSet
 * @param id
 * @param campaignIdList
 * @return
 */

bool Map::missionCampaignListSet(const int &id, const QVariantList &campaignIdList)
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
 * @brief Map::summaryAdd
 * @param params
 * @return
 */

int Map::summaryAdd(const int &campaignId)
{
	QVariantMap m;
	m["campaignid"] = campaignId;
	int id = m_db->execInsertQuery("INSERT INTO summary (?k?) values (?)", m);
	if (id != -1) {
		emit campaignUpdated(campaignId);
		setMapModified(true);
	}

	return id;
}


/**
 * @brief Map::summaryLevelAdd
 * @param params
 * @return
 */

int Map::summaryLevelAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT INTO summaryLevel (?k?) values (?)", params);
	if (id != -1) {
		emit summaryUpdated(params.value("summaryid", -1).toInt());
		setMapModified(true);
	}

	return id;
}


/**
 * @brief Map::summaryLevelUpdate
 * @param id
 * @param missionId
 * @param params
 * @return
 */

bool Map::summaryLevelUpdate(const int &id, const int &summaryId, const QVariantMap &params)
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
 * @brief Map::summaryLevelRemove
 * @param id
 * @param missionId
 * @return
 */

bool Map::summaryLevelRemove(const int &id, const int &summaryId)
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
 * @brief Map::summaryChapterAdd
 * @param params
 * @return
 */

int Map::summaryChapterAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT OR IGNORE INTO bindSummaryChapter (?k?) values (?)", params);
	if (id != -1) {
		int summaryId = params.value("summaryid", -1).toInt();
		emit summaryUpdated(summaryId);
		emit chapterListUpdated(-1, summaryId);
		setMapModified(true);
		return id;
	}

	return -1;
}


/**
 * @brief Map::summaryChapterRemove
 * @param id
 * @param missionId
 * @return
 */


bool Map::summaryChapterRemove(const int &summaryId, const int &chapterId)
{
	QVariantList l;
	l << chapterId;
	l << summaryId;
	bool r = m_db->execSimpleQuery("DELETE FROM bindSummaryChapter WHERE chapterid=? AND summaryid=?", l);
	if (r) {
		emit summaryUpdated(summaryId);
		emit chapterUpdated(chapterId);
		setMapModified(true);
	}
	return r;
}




/**
 * @brief Map::summaryIntroAdd
 * @param summaryId
 * @param introId
 * @param isOutro
 * @return
 */

bool Map::summaryIntroAdd(const int &id, const int &introId, const bool &isOutro)
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
 * @brief Map::chapterGet
 * @param id
 * @return
 */

QVariantMap Map::chapterGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;

	m_db->execSelectQueryOneRow("SELECT name FROM chapter where id=?", l, &map);

	m_db->execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
								"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroChapter "
								"LEFT JOIN intro ON (bindIntroChapter.introid=intro.id) "
								"WHERE chapterid=?", l, &map);

	if (!map.contains("introId"))
		map["introId"] = -1;



	QVariantList missions;

	m_db->execSelectQuery("SELECT mission.id as id, mission.name as name FROM bindMissionChapter "
						  "LEFT JOIN mission ON (mission.id=bindMissionChapter.missionid) "
						  "WHERE chapterid=? ORDER BY mission.name", l, &missions);

	map["missions"] = missions;


	QVariantList campaigns;

	m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM bindSummaryChapter "
						  "LEFT JOIN summary ON (summary.id=bindSummaryChapter.summaryid) "
						  "LEFT JOIN campaign ON (campaign.id=summary.campaignid) "
						  "WHERE chapterid=? ORDER BY campaign.name", l, &campaigns);

	map["campaigns"] = campaigns;



	QVariantList storages;

	m_db->execSelectQuery("SELECT id, name, module, data FROM storage WHERE storage.chapterid=? ORDER BY id", l, &storages);

	map["storages"] = storages;


	QVariantList storageObjectives;

	foreach (QVariant s, storages) {
		int sId = s.toMap().value("id", -1).toInt();
		QVariantList p;
		p << sId;
		QVariantList oo;
		m_db->execSelectQuery("SELECT id, name, module, data FROM objective WHERE objecive.storeageid=? ORDER BY id", p, &oo);
		QVariantMap r;
		r["id"] = sId;
		r["objectives"] = oo;
		storageObjectives << r;
	}

	map["storageObjectives"] = storageObjectives;



	QVariantList objectives;

	m_db->execSelectQuery("SELECT id, name, module, data FROM objective WHERE objective.chapterid=? ORDER BY id", l, &storages);

	map["objectives"] = objectives;

	return map;
}


/**
 * @brief Map::chapterListGet
 * @param missionId
 * @param summaryId
 * @return
 */

QVariantList Map::chapterListGet(const int &missionId, const int &summaryId)
{
	QVariantList list;

	if (summaryId != -1) {
		QVariantList l;
		l << summaryId;
		m_db->execSelectQuery("SELECT chapterid as id, name FROM bindSummaryChapter "
							  "LEFT JOIN chapter ON (chapter.id=bindSummaryChapter.chapterid) "
							  "WHERE summaryid=? "
							  "ORDER BY chapterid", l, &list);
	} else if (missionId != -1) {
		QVariantList l;
		l << missionId;
		m_db->execSelectQuery("SELECT chapterid as id, name, num FROM bindMissionChapter "
							  "LEFT JOIN chapter ON (chapter.id=bindMissionChapter.chapterid) "
							  "WHERE missionid=? "
							  "ORDER BY num", l, &list);
	} else {
		m_db->execSelectQuery("SELECT id, name, 0 as num FROM chapter ORDER BY name, id", QVariantList(), &list);
	}

	return list;
}



/**
 * @brief Map::chapterAdd
 * @param params
 * @return
 */

int Map::chapterAdd(const QVariantMap &params)
{
	int id = m_db->execInsertQuery("INSERT INTO chapter (?k?) values (?)", params);
	if (id != -1) {
		emit chapterListUpdated(-1, -1);
		setMapModified(true);
		return id;
	}

	return -1;
}




/**
 * @brief Map::chapterUpdate
 * @param id
 * @param params
 * @return
 */

bool Map::chapterUpdate(const int &id, const QVariantMap &params, const int &missionId, const int &summaryId)
{
	QVariantMap bind;
	bind[":id"] = id;

	if (m_db->execUpdateQuery("UPDATE chapter SET ? WHERE id=:id", params, bind)) {
		emit chapterUpdated(id);
		emit chapterListUpdated(missionId, summaryId);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief Map::chapterIntroAdd
 * @param id
 * @param introId
 * @param isOutro
 * @return
 */

bool Map::chapterIntroAdd(const int &id, const int &introId)
{
	QVariantMap m;
	m["chapterid"] = id;
	m["introid"] = introId;

	int r = m_db->execInsertQuery("INSERT INTO bindIntroChapter (?k?) VALUES (?)", m);

	if (r != -1) {
		emit chapterUpdated(id);
		setMapModified(true);
		return true;
	}

	return false;
}


/**
 * @brief Map::chapterMissionListSet
 * @param id
 * @param missionIdList
 * @return
 */

bool Map::chapterMissionListSet(const int &id, const QVariantList &missionIdList)
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
		return m_db->execSimpleQuery("DELETE FROM bindMissionChapter WHERE chapterid=?", params);
	}

	bool r1 = m_db->execSimpleQuery("DELETE FROM bindMissionChapter WHERE chapterid=? AND missionid NOT IN ("+il.join(",")+")", params);

	params << id;

	bool r2 = m_db->execSimpleQuery("INSERT INTO bindMissionChapter(missionid, chapterid, num) "
									"SELECT T.id as missionid, "
									"? as chapterid, "
									"COALESCE((SELECT MAX(num)+1 FROM bindMissionChapter WHERE missionid=T.id), 1) as num "
									"FROM (SELECT column1 as id FROM (values "+vl.join(",")+" ) EXCEPT SELECT missionid from bindMissionChapter WHERE chapterid=?) T",
									params);

	emit chapterUpdated(id);
	setMapModified(true);

	return (r1 && r2);
}



/**
 * @brief Map::chapterCampaignListSet
 * @param id
 * @param missionIdList
 * @return
 */

bool Map::chapterSummaryListSet(const int &id, const QVariantList &summaryIdList)
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
		return m_db->execSimpleQuery("DELETE FROM bindSummaryChapter WHERE chapterid=?", params);
	}

	bool r1 = m_db->execSimpleQuery("DELETE FROM bindSummaryChapter WHERE chapterid=? AND summaryid NOT IN ("+il.join(",")+")", params);

	params << id;

	bool r2 = m_db->execSimpleQuery("INSERT INTO bindSummaryChapter(summaryid, chapterid) "
									"SELECT T.id as summaryid, ? as chapterid "
									"FROM (SELECT column1 as id FROM (values "+vl.join(",")+" ) EXCEPT SELECT summaryid from bindSummaryChapter WHERE chapterid=?) T",
									params);

	emit chapterUpdated(id);
	setMapModified(true);

	return (r1 && r2);
}





/**
 * @brief Map::introGet
 * @param id
 * @return
 */

QVariantMap Map::introGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;

	m_db->execSelectQueryOneRow("SELECT ttext, img, media, sec, levelMin, levelMax FROM intro where id=?", l, &map);


	QVariantList campaigns;

	m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name, outro FROM bindIntroCampaign "
						  "LEFT JOIN campaign ON (campaign.id=bindIntroCampaign.campaignid) "
						  "WHERE introid=? ORDER BY campaign.name", l, &campaigns);

	map["campaigns"] = campaigns;



	QVariantList missions;

	m_db->execSelectQuery("SELECT mission.id as id, mission.name as name, outro FROM bindIntroMission "
						  "LEFT JOIN mission ON (mission.id=bindIntroMission.missionid) "
						  "WHERE introid=? ORDER BY mission.name", l, &missions);

	map["missions"] = missions;


	QVariantList summaries;

	m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name, outro FROM bindIntroSummary "
						  "LEFT JOIN summary ON (summary.id=bindIntroSummary.summaryid) "
						  "LEFT JOIN campaign ON (campaign.id=summary.campaignid) "
						  "WHERE introid=? ORDER BY campaign.name", l, &summaries);

	map["summaries"] = summaries;




	QVariantList chapters;

	m_db->execSelectQuery("SELECT chapter.id as id, chapter.name as name FROM bindIntroChapter "
						  "LEFT JOIN chapter ON (chapter.id=bindIntroChapter.chapterid) "
						  "WHERE introid=? ORDER BY chapter.name", l, &chapters);

	map["chapters"] = chapters;


	return map;
}


/**
 * @brief Map::introListGet
 * @param parentId
 * @param type
 * @return
 */

QVariantList Map::introListGet(const int &parentId, const Map::IntroType &type)
{
	QVariantList list;

	if (parentId == -1 || type == IntroUndefined) {
		m_db->execSelectQuery("SELECT id, ttext, img, media, sec, levelMin, levelMax FROM intro ORDER BY id", QVariantList(), &list);
	} else if (type == IntroCampaign) {
		QVariantList l;
		l << parentId;
		m_db->execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax, outro FROM bindIntroCampaign "
							  "LEFT JOIN intro ON (intro.id=bindIntroCampaign.introid) "
							  "WHERE campaignid=? "
							  "ORDER BY introid", l, &list);
	} else if (type == IntroMission) {
		QVariantList l;
		l << parentId;
		m_db->execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax, outro FROM bindIntroMission "
							  "LEFT JOIN intro ON (intro.id=bindIntroMission.introid) "
							  "WHERE missionid=? "
							  "ORDER BY introid", l, &list);
	} else if (type == IntroSummary) {
		QVariantList l;
		l << parentId;
		m_db->execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax, outro FROM bindIntroSummary "
							  "LEFT JOIN intro ON (intro.id=bindIntroSummary.introid) "
							  "WHERE summaryid=? "
							  "ORDER BY introid", l, &list);
	} else if (type == IntroChapter) {
		QVariantList l;
		l << parentId;
		m_db->execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax FROM bindIntroChapter "
							  "LEFT JOIN intro ON (intro.id=bindIntroChapter.introid) "
							  "WHERE chapterid=? "
							  "ORDER BY introid", l, &list);
	}

	return list;
}



/**
 * @brief Map::introAdd
 * @param params
 * @return
 */

int Map::introAdd(const QVariantMap &params)
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
 * @brief Map::introUpdate
 * @param id
 * @param params
 * @param parentId
 * @param type
 * @return
 */

bool Map::introUpdate(const int &id, const QVariantMap &params, const int &parentId, const Map::IntroType &type)
{
	QVariantMap bind;
	bind[":id"] = id;

	qDebug() << params;

	if (m_db->execUpdateQuery("UPDATE intro SET ? WHERE id=:id", params, bind)) {
		emit introUpdated(id);
		emit introListUpdated(parentId, type);
		setMapModified(true);
		return true;
	}

	return false;
}





/**
 * @brief Map::updateCampaign
 * @param id
 * @param params
 * @return
 */

bool Map::campaignUpdate(const int &id, const QVariantMap &params)
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
 * @brief Map::campaignAdd
 * @param params
 * @return
 */

int Map::campaignAdd(const QVariantMap &params)
{
	QVariantMap p = params;

	int num = 1;

	if (p.contains("num")) {
		num = p["num"].toInt();
	} else {
		QVariantMap m;
		m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM campaign", QVariantList(), &m);
		if (m.contains("num"))
			num = m["num"].toInt();
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
 * @brief Map::campaignAddMission
 * @param campaignId
 * @param missionId
 * @param num
 * @return
 */

bool Map::campaignMissionAdd(const int &id, const int &missionId, const int &num)
{
	int realnum = num;
	if (num == -1) {
		QVariantMap m;
		QVariantList l;
		l << id;
		m_db->execSelectQueryOneRow("SELECT MAX(num)+1 AS num FROM bindCampaignMission WHERE campaignid=?", l, &m);
		if (m.contains("num"))
			realnum = m["num"].toInt();
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
 * @brief Map::campaignSummaryAdd
 * @param campaignId
 * @return
 */

int Map::campaignSummaryAdd(const int &id)
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
 * @brief Map::campaignIntroAdd
 * @param campaignId
 * @param isOutro
 * @return
 */

bool Map::campaignIntroAdd(const int &id, const int &introId, const bool &isOutro)
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
 * @brief Map::campaignLockSet
 * @param id
 * @param lockIdList
 * @return
 */

bool Map::campaignLockSet(const int &id, const QVariantList &lockIdList)
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




void Map::setMapTimeCreated(QString mapTimeCreated)
{
	if (m_mapTimeCreated == mapTimeCreated)
		return;

	m_mapTimeCreated = mapTimeCreated;
	emit mapTimeCreatedChanged(m_mapTimeCreated);
}



void Map::setMapUuid(QString mapUuid)
{
	if (m_mapUuid == mapUuid)
		return;

	m_mapUuid = mapUuid;
	emit mapUuidChanged(m_mapUuid);
}



/**
 * @brief Map::tableToJson
 * @param table
 * @return
 */


QJsonArray Map::tableToJson(const QString &table, const bool &convertData)
{
	QVariantMap m = m_db->runSimpleQuery("SELECT * from "+table+" ORDER BY rowid");

	if (m["error"].toBool()) {
		return QJsonArray();
	}

	QJsonArray list;

	if (convertData) {
		QVariantList r = m["records"].toList();
		for (int i=0; i<r.count(); ++i) {
			QJsonObject rrObj;
			QVariantMap rr = r.value(i).toMap();

			QStringList keys = rr.keys();

			foreach (QString k, keys) {
				if (k == "data") {
					QByteArray data = rr.value(k).toString().toUtf8();
					QJsonDocument doc = QJsonDocument::fromJson(data);
					if (doc.isNull()) {
						qWarning() << tr("Objective JSON error ") << i << data;
					} else {
						rrObj[k] = doc.object();
					}
				} else {
					rrObj[k] = rr.value(k).toJsonValue();
				}
			}

			list << rrObj;
		}

	} else {
		list = m["records"].toJsonArray();
	}

	return list;
}




/**
 * @brief Map::JsonToTable
 * @param array
 * @param convertData
 * @return
 */

bool Map::JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData)
{
	for (int i=0; i<array.count(); ++i) {
		QJsonObject rec = array[i].toObject();
		QVariantMap rr;

		if (convertData) {
			QStringList keys = rec.keys();

			foreach (QString k, keys) {
				if (k == "data") {
					QJsonDocument doc(rec.value(k).toObject());
					rr[k] = QString(doc.toJson(QJsonDocument::Compact));
				} else {
					rr[k] = rec.value(k).toVariant();
				}
			}


		} else {
			rr = rec.toVariantMap();
		}

		if (!m_db->execInsertQuery("INSERT INTO "+table+" (?k?) VALUES (?)", rr))
			return false;

	}

	return true;
}








/**
 * @brief Map::databaseInit
 * @return
 */

bool Map::databasePrepare()
{
	Q_ASSERT(m_client);

	if (!m_db->batchQueryFromFile(":/sql/map.sql")) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), m_databaseFile);
		return false;
	}

	if (!m_db->batchQueryFromFile(":/sql/mapeditor.sql")) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), m_databaseFile);
		return false;
	}

	return true;
}





/**
 * @brief Map::databaseChecio
 */

bool Map::databaseCheck()
{
	Q_ASSERT(m_client);

	if (m_mapType == MapInvalid) {
		m_client->sendMessageError(tr("Internal error"), tr("Az adatbázis típusa nincs megadva!"));
		return false;
	}

	if (m_mapType == MapEditor) {
		setDatabaseFile(Client::standardPath("tmpmapeditor.db"));
	} else if (m_mapType == MapGame) {
		setDatabaseFile(Client::standardPath("tmpmapgame.db"));
	}

	if (m_mapType == MapEditor && QFile::exists(m_databaseFile)) {
		qInfo() << tr("Létező ideiglenes adatbázis: ")+m_databaseFile;
		if (!databaseOpen()) {
			m_db->close();

			qInfo() << tr("Nem sikerült megnyitni a fájlt, törlöm: ")+m_databaseFile;

			if (!QFile::remove(m_databaseFile)) {
				m_client->sendMessageError(tr("Internal error"), tr("Nem sikerült törölni az ideiglenes adatbázist!"), m_databaseFile);
				return false;
			}

			return true;
		}

		QVariantMap m = m_db->runSimpleQuery("SELECT serverid, mapid, originalFile, uuid from mapeditor");
		if (!m["error"].toBool() && m["records"].toList().count()) {
			QVariantMap r = m["records"].toList().value(0).toMap();
			int serverid = r.value("serverid").toInt();
			int mapid = r.value("mapid").toInt();
			QString filename = r.value("originalFile").toString();
			QString uuid = r.value("uuid").toString();

			if (!uuid.isEmpty()) {
				emit mapBackupExists(filename, uuid, serverid, mapid);
				m_db->close();
				return true;
			}
		}

		m_db->close();

		qInfo() << tr("Hibás adatbázis, törlöm: ")+m_databaseFile;

		if (!QFile::remove(m_databaseFile)) {
			m_client->sendMessageError(tr("Internal error"), tr("Nem sikerült törölni az ideiglenes adatbázist!"), m_databaseFile);
			return false;
		}

		return true;
	}

	return true;
}




void Map::setMapType(Map::MapType mapType)
{
	if (m_mapType == mapType)
		return;

	switch (mapType) {
		case MapEditor:
			setDatabaseFile(Client::standardPath("tmpmapeditor.db"));
			break;
		case MapGame:
			setDatabaseFile(Client::standardPath("tmpgame.db"));
			break;
		default:
			break;
	}

	m_mapType = mapType;
	emit mapTypeChanged(m_mapType);
}

