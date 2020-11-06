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

#include "mapdata.h"
#include "cosclient.h"

QVariantList MapData::m_storageModules = QVariantList();
QVariantList MapData::m_objectiveModules = QVariantList();

MapData::MapData(const QString &connectionName, QObject *parent)
	: AbstractDbActivity(connectionName, parent)
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
				 << "bindMissionStorage"
				 << "bindIntroCampaign"
				 << "bindIntroMission"
				 << "bindIntroSummary"
				 << "bindSummaryStorage";

	m_databaseInitSql = QStringList();
}


/**
 * @brief Map::~Map
 */

MapData::~MapData()
{
	if (isOpen())
		close();

	if (QFile::exists(m_databaseFile)) {
		qDebug() << tr("Remove temporary map file ")+m_databaseFile << QFile::remove(m_databaseFile);
	}
}


/**
 * @brief Map::storageModules
 * @return
 */


QVariantList MapData::storageModules()
{
	if (m_storageModules.isEmpty())
		setStorageModules();

	return m_storageModules;
}


/**
 * @brief Map::objectiveModules
 * @return
 */

QVariantList MapData::objectiveModules()
{
	if (m_objectiveModules.isEmpty())
		setObjectiveModules();

	return m_objectiveModules;
}



/**
 * @brief Map::storageModule
 * @param type
 * @return
 */


QVariantMap MapData::storageModule(const QString &type)
{
	if (m_storageModules.isEmpty())
		setStorageModules();

	foreach (QVariant v, m_storageModules) {
		QVariantMap m = v.toMap();
		if (m.value("type", "").toString() == type)
			return m;
	}

	return QVariantMap();
}


/**
 * @brief Map::storageObjectiveModules
 * @param type
 * @return
 */

QVariantList MapData::storageObjectiveModules(const QString &type)
{
	if (m_objectiveModules.isEmpty())
		setObjectiveModules();

	QVariantList ret;

	foreach (QVariant v, m_objectiveModules) {
		QVariantMap m = v.toMap();
		QStringList l = m.value("storages").toStringList();

		if (l.contains(type))
			ret << m;
	}

	return ret;
}


/**
 * @brief Map::objectiveModule
 * @param type
 * @return
 */

QVariantMap MapData::objectiveModule(const QString &type)
{
	if (m_objectiveModules.isEmpty())
		setObjectiveModules();

	foreach (QVariant v, m_objectiveModules) {
		QVariantMap m = v.toMap();
		if (m.value("type", "").toString() == type)
			return m;
	}

	return QVariantMap();
}





/**
 * @brief Map::getInfo
 * @return
 */

QVariantMap MapData::infoGet()
{
	QVariantMap ret;
	execSelectQueryOneRow("SELECT title FROM info", QVariantList(), &ret);

	return ret;
}




/**
 * @brief Map::getCampaign
 * @param id
 * @return
 */

QVariantMap MapData::campaignGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	execSelectQueryOneRow("SELECT id, num, name FROM campaign WHERE id=?", l, &map);

	execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
								"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroCampaign "
								"LEFT JOIN intro ON (bindIntroCampaign.introid=intro.id) "
								"WHERE campaignid=? AND outro=false", l, &map);

	execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
								"levelMin as outroLevelMin, levelMax as outroLevelMax FROM bindIntroCampaign "
								"LEFT JOIN intro ON (bindIntroCampaign.introid=intro.id) "
								"WHERE campaignid=? AND outro=true", l, &map);

	execSelectQueryOneRow("SELECT id as summaryId FROM summary WHERE campaignid=?", l, &map);

	if (!map.contains("introId"))
		map["introId"] = -1;

	if (!map.contains("outroId"))
		map["outroId"] = -1;

	if (!map.contains("summaryId"))
		map["summaryId"] = -1;



	QVariantList locks;

	execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM campaignLock "
						  "LEFT JOIN campaign ON (campaignLock.lockId=campaign.id) "
						  "WHERE campaignId=?", l, &locks);

	map["locks"] = locks;

	return map;
}



/**
 * @brief Map::getCampaignList
 * @return
 */


QVariantList MapData::campaignListGet()
{
	QVariantList list;
	execSelectQuery("SELECT id, COALESCE(num, 0) as num, name FROM campaign ORDER BY num", QVariantList(), &list);
	return list;
}



/**
 * @brief Map::missionGet
 * @param id
 * @return
 */

QVariantMap MapData::missionGet(const int &id, const bool &isSummary, const bool &fullStorage, const int &filterLevel)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	if (isSummary) {
		execSelectQueryOneRow("SELECT summary.id, summary.uuid as uuid, campaign.name as name FROM summary "
									"LEFT JOIN campaign ON (summary.campaignid=campaign.id) "
									"WHERE summary.id=?", l, &map);

		execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
									"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroSummary "
									"LEFT JOIN intro ON (bindIntroSummary.introid=intro.id) "
									"WHERE summaryid=? AND outro=false", l, &map);

		execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
									"levelMin as outroLevelMin, levelMax as outroLevelMax FROM bindIntroSummary "
									"LEFT JOIN intro ON (bindIntroSummary.introid=intro.id) "
									"WHERE summaryid=? AND outro=true", l, &map);
	} else {
		execSelectQueryOneRow("SELECT id, uuid, name FROM mission WHERE id=?", l, &map);

		execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
									"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroMission "
									"LEFT JOIN intro ON (bindIntroMission.introid=intro.id) "
									"WHERE missionid=? AND outro=false", l, &map);

		execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
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
		execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM summary "
							  "LEFT JOIN campaign ON (summary.campaignid=campaign.id) "
							  "WHERE summary.id=?", l, &campaigns);
	} else {
		execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM bindCampaignMission "
							  "LEFT JOIN campaign ON (bindCampaignMission.campaignid=campaign.id) "
							  "WHERE missionid=? ORDER BY campaign.num", l, &campaigns);
	}


	map["campaigns"] = campaigns;


	QVariantList levels;

	if (isSummary) {
		execSelectQuery("SELECT id, level, sec, hp, 0 as mode, showCorrect FROM summaryLevel WHERE summaryid=? ORDER BY level", l, &levels);
	} else {
		execSelectQuery("SELECT id, level, sec, hp, mode, showCorrect FROM missionLevel WHERE missionid=? ORDER BY level", l, &levels);
	}

	map["levels"] = levels;


	if (isSummary && fullStorage) {
		QVariantList storages = summaryStorageListGet(id, filterLevel);

		map["storages"] = storages;
	} else {
		QVariantList storages = storageListGet(isSummary ? -1 : id, isSummary ? id : -1, filterLevel);

		map["storages"] = storages;
	}

	return map;
}




/**
 * @brief Map::getMissionList
 * @param missionId
 * @return
 */

QVariantList MapData::missionListGet(const int &campaignId)
{
	QVariantList list;

	if (campaignId != -1) {
		QVariantList l;
		l << campaignId;
		execSelectQuery("SELECT missionid as id, name, num FROM bindCampaignMission "
							  "LEFT JOIN mission ON (mission.id=bindCampaignMission.missionid) "
							  "WHERE campaignid=? "
							  "ORDER BY num", l, &list);
	} else {
		execSelectQuery("SELECT id, name, 0 as num FROM mission ORDER BY name", QVariantList(), &list);
	}

	return list;
}


/**
 * @brief Map::summaryFullStorageListGet
 * @param id
 * @return
 */

QVariantList MapData::summaryStorageListGet(const int &id, const int &filterLevel)
{
	QVariantList list;
	QVariantList l;
	l << id;

	execSelectQuery("SELECT storageid as id, name, module, data FROM "
						  "(SELECT summary.id as summaryid, storageid FROM summary "
						  "LEFT JOIN bindCampaignMission ON (bindCampaignMission.campaignid=summary.campaignid) "
						  "INNER JOIN bindMissionStorage ON (bindMissionStorage.missionid=bindCampaignMission.missionid) "
						  "UNION SELECT summaryid, storageid FROM bindSummaryStorage) r "
						  "LEFT JOIN storage ON (storage.id=r.storageid) "
						  "WHERE summaryid=? ORDER BY storageid", l, &list);

	QVariantList ret;

	foreach(QVariant v, list) {
		QVariantMap m = v.toMap();

		QVariantList objs = objectiveListGet(m.value("id").toInt(), filterLevel);
		m["objectives"] = objs;
		ret << m;
	}

	return ret;
}




/**
 * @brief Map::storageGet
 * @param id
 * @return
 */

QVariantMap MapData::storageGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	execSelectQueryOneRow("SELECT id, name, module, data FROM storage where id=?", l, &map);


	QVariantList missions;

	execSelectQuery("SELECT mission.id as id, mission.name as name FROM bindMissionStorage "
						  "LEFT JOIN mission ON (mission.id=bindMissionStorage.missionid) "
						  "WHERE storageid=? ORDER BY mission.name", l, &missions);

	map["missions"] = missions;


	QVariantList campaigns;

	execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM bindSummaryStorage "
						  "LEFT JOIN summary ON (summary.id=bindSummaryStorage.summaryid) "
						  "LEFT JOIN campaign ON (campaign.id=summary.campaignid) "
						  "WHERE storageid=? ORDER BY campaign.name", l, &campaigns);

	map["campaigns"] = campaigns;


	QString data = map.value("data").toString();
	QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

	QJsonObject d = doc.object();

	map.remove("data");
	map["data"] = d;


	map["objectives"] = objectiveListGet(id);

	return map;
}


/**
 * @brief Map::storageListGet
 * @param missionId
 * @param summaryId
 * @return
 */

QVariantList MapData::storageListGet(const int &missionId, const int &summaryId, const int &filterLevel)
{
	QVariantList list;

	if (summaryId != -1) {
		QVariantList l;
		l << summaryId;
		execSelectQuery("SELECT storageid as id, 0 as num, name, module, data FROM bindSummaryStorage "
							  "LEFT JOIN storage ON (storage.id=bindSummaryStorage.storageid) "
							  "WHERE summaryid=? "
							  "ORDER BY storageid", l, &list);
	} else if (missionId != -1) {
		QVariantList l;
		l << missionId;
		execSelectQuery("SELECT storageid as id, num, name, module, data FROM bindMissionStorage "
							  "LEFT JOIN storage ON (storage.id=bindMissionStorage.storageid) "
							  "WHERE missionid=? "
							  "ORDER BY num", l, &list);
	} else {
		execSelectQuery("SELECT id, 0 as num, name, module, data FROM storage ORDER BY id", QVariantList(), &list);
	}


	QVariantList ret;

	foreach(QVariant v, list) {
		QVariantMap m = v.toMap();

		QVariantList objs = objectiveListGet(m.value("id").toInt(), filterLevel);
		m["objectives"] = objs;
		ret << m;
	}

	return ret;
}



/**
 * @brief Map::introGet
 * @param id
 * @return
 */

QVariantMap MapData::introGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	execSelectQueryOneRow("SELECT id, ttext, img, media, sec, levelMin, levelMax FROM intro where id=?", l, &map);


	QVariantList campaigns;

	execSelectQuery("SELECT campaign.id as id, campaign.name as name, outro FROM bindIntroCampaign "
						  "LEFT JOIN campaign ON (campaign.id=bindIntroCampaign.campaignid) "
						  "WHERE introid=? ORDER BY campaign.name", l, &campaigns);

	map["campaigns"] = campaigns;



	QVariantList missions;

	execSelectQuery("SELECT mission.id as id, mission.name as name, outro FROM bindIntroMission "
						  "LEFT JOIN mission ON (mission.id=bindIntroMission.missionid) "
						  "WHERE introid=? ORDER BY mission.name", l, &missions);

	map["missions"] = missions;


	QVariantList summaries;

	execSelectQuery("SELECT campaign.id as id, campaign.name as name, outro FROM bindIntroSummary "
						  "LEFT JOIN summary ON (summary.id=bindIntroSummary.summaryid) "
						  "LEFT JOIN campaign ON (campaign.id=summary.campaignid) "
						  "WHERE introid=? ORDER BY campaign.name", l, &summaries);

	map["summaries"] = summaries;



	return map;
}


/**
 * @brief Map::introListGet
 * @param parentId
 * @param type
 * @return
 */

QVariantList MapData::introListGet(const int &parentId, const MapData::IntroType &type)
{
	QVariantList list;

	if (parentId == -1 || type == IntroUndefined) {
		execSelectQuery("SELECT id, ttext, img, media, sec, levelMin, levelMax FROM intro ORDER BY id", QVariantList(), &list);
	} else if (type == IntroCampaign) {
		QVariantList l;
		l << parentId;
		execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax, outro FROM bindIntroCampaign "
							  "LEFT JOIN intro ON (intro.id=bindIntroCampaign.introid) "
							  "WHERE campaignid=? "
							  "ORDER BY introid", l, &list);
	} else if (type == IntroMission) {
		QVariantList l;
		l << parentId;
		execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax, outro FROM bindIntroMission "
							  "LEFT JOIN intro ON (intro.id=bindIntroMission.introid) "
							  "WHERE missionid=? "
							  "ORDER BY introid", l, &list);
	} else if (type == IntroSummary) {
		QVariantList l;
		l << parentId;
		execSelectQuery("SELECT introid as id, ttext, img, media, sec, levelMin, levelMax, outro FROM bindIntroSummary "
							  "LEFT JOIN intro ON (intro.id=bindIntroSummary.introid) "
							  "WHERE summaryid=? "
							  "ORDER BY introid", l, &list);
	}

	return list;
}





/**
 * @brief Map::storageInfo
 * @param module
 * @return
 */

QVariantMap MapData::storageInfo(const QString &type) const
{
	foreach (QVariant v, m_storageModules) {
		QVariantMap m = v.toMap();
		if (m.value("type", "") == type)
			return m;
	}
	return QVariantMap();
}



/**
 * @brief Map::objectiveGet
 * @param id
 * @return
 */


QVariantMap MapData::objectiveGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;

	execSelectQueryOneRow("SELECT objective.id as id, objective.storageid as storageid, objective.module as module, objective.data as data, "
								"objective.level as level, "
								"storage.module as storageModule, storage.name as storageName, storage.data as storageData "
								"FROM objective LEFT JOIN storage ON (storage.id=objective.storageid) WHERE objective.id=?", l, &map);

	QString data = map.value("data").toString();
	QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
	QJsonObject d = doc.object();

	map.remove("data");
	map["data"] = d;



	QString sdata = map.value("storageData").toString();
	QJsonDocument sdoc = QJsonDocument::fromJson(data.toUtf8());
	QJsonObject sd = doc.object();

	map.remove("storageData");
	map["storageData"] = sd;

	return map;
}




/**
 * @brief Map::objectiveListGet
 * @param storageId
 * @return
 */

QVariantList MapData::objectiveListGet(const int &storageId, const int filterLevel)
{
	QVariantList list;

	if (storageId != -1) {
		QVariantList l;
		l << storageId;

		if (filterLevel != -1) {
			l << filterLevel;
			execSelectQuery("SELECT id, storageid, module, data, level FROM objective WHERE storageid=? AND level=? ORDER BY id", l, &list);
		} else {
			execSelectQuery("SELECT id, storageid, module, data, level FROM objective WHERE storageid=? ORDER BY id", l, &list);
		}
	} else {
		if (filterLevel != -1) {
			QVariantList l;
			l << filterLevel;
			execSelectQuery("SELECT id, storageid, module, data, level FROM objective WHERE level=? ORDER BY id", l, &list);
		} else {
			execSelectQuery("SELECT id, storageid, module, data, level FROM objective ORDER BY id", QVariantList(), &list);
		}
	}

	return list;
}




/**
 * @brief Map::objectiveInfo
 * @param type
 * @return
 */

QVariantMap MapData::objectiveInfo(const QString &type) const
{
	foreach (QVariant v, m_objectiveModules) {
		QVariantMap m = v.toMap();
		if (m.value("type", "") == type)
			return m;
	}
	return QVariantMap();
}



/**
 * @brief Map::databaseInit
 * @return
 */

bool MapData::databaseInit()
{
	Q_ASSERT(m_client);

	QStringList l;
	l << ":/sql/map.sql";
	l << m_databaseInitSql;

	foreach (QString s, l) {
		if (!batchQueryFromFile(s)) {
			m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), m_databaseFile);
			return false;
		}
	}

	return true;
}






/**
 * @brief Map::tableToJson
 * @param table
 * @return
 */


QJsonArray MapData::tableToJson(const QString &table, const bool &convertData)
{
	QVariantMap m = runSimpleQuery("SELECT * from "+table+" ORDER BY rowid");

	if (m.value("error").toBool()) {
		return QJsonArray();
	}

	QJsonArray list;

	if (convertData) {
		QVariantList r = m.value("records").toList();
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
		list = m.value("records").toJsonArray();
	}

	return list;
}




/**
 * @brief Map::JsonToTable
 * @param array
 * @param convertData
 * @return
 */

bool MapData::JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData)
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

		if (!execInsertQuery("INSERT INTO "+table+" (?k?) VALUES (?)", rr))
			return false;

	}

	return true;
}


/**
 * @brief Map::generateMissionUuids
 */

void MapData::generateMissionUuids()
{
	QVariantList list;

	execSelectQuery("SELECT id FROM mission WHERE uuid IS NULL OR uuid=''", QVariantList(), &list);

	foreach (QVariant v, list) {
		QVariantList params;
		params << QUuid::createUuid().toString();
		params << v.toMap().value("id").toInt();
		runSimpleQuery("UPDATE mission SET uuid=? WHERE id=?", params);
	}


	QVariantList listS;

	execSelectQuery("SELECT id FROM summary WHERE uuid IS NULL OR uuid=''", QVariantList(), &listS);

	foreach (QVariant v, listS) {
		QVariantList params;
		params << QUuid::createUuid().toString();
		params << v.toMap().value("id").toInt();
		runSimpleQuery("UPDATE summary SET uuid=? WHERE id=?", params);
	}
}






/**
 * @brief Map::setStorageModules
 */

void MapData::setStorageModules()
{
	{
		QVariantMap m;

		m["type"] = "questionpair";
		m["icon"] = "Q";
		m["label"] = tr("Kérdés-válasz");

		m_storageModules << m;
	}

	{
		QVariantMap m;

		m["type"] = "text";
		m["icon"] = "T";
		m["label"] = tr("Szöveg");

		m_storageModules << m;
	}

	{
		QVariantMap m;

		m["type"] = "order";
		m["icon"] = "O";
		m["label"] = tr("Sorrend");

		m_storageModules << m;
	}

	{
		QVariantMap m;

		m["type"] = "number";
		m["icon"] = "N";
		m["label"] = tr("Számok");

		m_storageModules << m;
	}

	{
		QVariantMap m;

		m["type"] = "image";
		m["icon"] = "I";
		m["label"] = tr("Kép");

		m_storageModules << m;
	}

	{
		QVariantMap m;

		m["type"] = "sound";
		m["icon"] = "S";
		m["label"] = tr("Hang");

		m_storageModules << m;
	}

	{
		QVariantMap m;

		m["type"] = "video";
		m["icon"] = "V";
		m["label"] = tr("Videó");

		m_storageModules << m;
	}
}



/**
 * @brief Map::setObjectiveModules
 */

void MapData::setObjectiveModules()
{
	{
		QVariantMap o;

		o["type"] = "pair";
		o["icon"] = "P";
		o["label"] = tr("Párosítás");
		o["storages"] = QStringList { "questionpair" };

		m_objectiveModules << o;
	}

	{
		QVariantMap o;

		o["type"] = "memory";
		o["icon"] = "M";
		o["label"] = tr("Memória");
		o["storages"] = QStringList { "questionpair" };

		m_objectiveModules << o;
	}

	{
		QVariantMap o;

		o["type"] = "simplechoice";
		o["icon"] = "S";
		o["label"] = tr("Egyszerű választás");
		o["storages"] = QStringList { "questionpair", "order", "image", "video", "sound" };
		o["defaultData"] = "{\"suffix\":\"?\"}";

		m_objectiveModules << o;
	}

	{
		QVariantMap o;

		o["type"] = "truefalse";
		o["icon"] = "T";
		o["label"] = tr("Igaz-hamis");
		o["storages"] = QStringList { "questionpair", "image", "video", "sound" };

		m_objectiveModules << o;
	}

	{
		QVariantMap o;

		o["type"] = "multichoice";
		o["icon"] = "M";
		o["label"] = tr("Többszörös választás");
		o["storages"] = QStringList { "questionpair" };

		m_objectiveModules << o;
	}

	{
		QVariantMap o;

		o["type"] = "ordering";
		o["icon"] = "O";
		o["label"] = tr("Sorbarendezés");
		o["storages"] = QStringList { "order" };

		m_objectiveModules << o;
	}

	{
		QVariantMap o;

		o["type"] = "fill";
		o["icon"] = "F";
		o["label"] = tr("Kiegészítés");
		o["storages"] = QStringList { "text" };

		m_objectiveModules << o;
	}
}





/**
 * @brief Map::loadFromJson
 * @param data
 * @param binaryFormat
 * @return
 */

QJsonObject MapData::loadFromJson(const QByteArray &data, const bool &binaryFormat, double *steps, double *currentStep)
{
	Q_ASSERT (m_client);

	if (steps) {
		*steps += m_tableNames.count()
				  +1		// open
				  +1		// jsonDoc
				  +1		// generateUuids
				  +1		// storage
				  +1;		// objective
	}

	if (!databaseOpen()) {
		return QJsonObject();
	}

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	QJsonDocument doc = binaryFormat ? QJsonDocument::fromBinaryData(data) : QJsonDocument::fromJson(data);

	if (doc.isNull()) {
		return QJsonObject();
	}

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	QJsonObject root = doc.object();



	if (!JsonToTable(root.value("storage").toArray(), "storage", true)) {
		return QJsonObject();
	}

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	if (!JsonToTable(root.value("objective").toArray(), "objective", true)) {
		return QJsonObject();
	}

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	foreach (QString t, m_tableNames) {
		if (!JsonToTable(root[t].toArray(), t, false)) {
			return QJsonObject();
		}

		if (steps && currentStep) {
			emit mapLoadingProgress(++(*currentStep)/(*steps));
			QCoreApplication::processEvents();
		}
	}

	generateMissionUuids();

	emit mapLoadingProgress(++(*currentStep)/(*steps));
	QCoreApplication::processEvents();

	return root;
}


