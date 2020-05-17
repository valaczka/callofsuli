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

QVariantList Map::m_storageModules = QVariantList();
QVariantList Map::m_objectiveModules = QVariantList();

Map::Map(const QString &connectionName, QObject *parent)
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

Map::~Map()
{
	if (m_db->isOpen())
		m_db->close();

	if (QFile::exists(m_databaseFile)) {
		qDebug() << tr("Remove temporary map file ")+m_databaseFile << QFile::remove(m_databaseFile);
	}
}


/**
 * @brief Map::storageModules
 * @return
 */


QVariantList Map::storageModules()
{
	if (m_storageModules.isEmpty())
		setStorageModules();

	return m_storageModules;
}


/**
 * @brief Map::objectiveModules
 * @return
 */

QVariantList Map::objectiveModules()
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


QVariantMap Map::storageModule(const QString &type)
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

QVariantList Map::storageObjectiveModules(const QString &type)
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

QVariantMap Map::objectiveModule(const QString &type)
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

QVariantMap Map::infoGet()
{
	QVariantMap ret;
	m_db->execSelectQueryOneRow("SELECT title FROM info", QVariantList(), &ret);

	return ret;
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
	map["id"] = -1;

	m_db->execSelectQueryOneRow("SELECT id, num, name FROM campaign WHERE id=?", l, &map);

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
	m_db->execSelectQuery("SELECT id, COALESCE(num, 0) as num, name FROM campaign ORDER BY num", QVariantList(), &list);
	return list;
}



/**
 * @brief Map::missionGet
 * @param id
 * @return
 */

QVariantMap Map::missionGet(const int &id, const bool &isSummary, const bool &fullStorage, const int &filterLevel)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	if (isSummary) {
		m_db->execSelectQueryOneRow("SELECT summary.id, campaign.name as name FROM summary "
									"LEFT JOIN campaign ON (summary.campaignid=campaign.id) "
									"WHERE summary.id=?", l, &map);

		m_db->execSelectQueryOneRow("SELECT intro.id as introId, ttext as introText, img as introImg, media as introMedia, sec as introSec, "
									"levelMin as introLevelMin, levelMax as introLevelMax FROM bindIntroSummary "
									"LEFT JOIN intro ON (bindIntroSummary.introid=intro.id) "
									"WHERE summaryid=? AND outro=false", l, &map);

		m_db->execSelectQueryOneRow("SELECT intro.id as outroId, ttext as outroText, img as outroImg, media as outroMedia, sec as outroSec, "
									"levelMin as outroLevelMin, levelMax as outroLevelMax FROM bindIntroSummary "
									"LEFT JOIN intro ON (bindIntroSummary.introid=intro.id) "
									"WHERE summaryid=? AND outro=true", l, &map);
	} else {
		m_db->execSelectQueryOneRow("SELECT id, name FROM mission WHERE id=?", l, &map);

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
 * @brief Map::summaryFullStorageListGet
 * @param id
 * @return
 */

QVariantList Map::summaryStorageListGet(const int &id, const int &filterLevel)
{
	QVariantList list;
	QVariantList l;
	l << id;

	m_db->execSelectQuery("SELECT storageid as id, name, module, data FROM "
						  "(SELECT summary.id as summaryid, storageid FROM summary "
						  "LEFT JOIN bindCampaignMission ON (bindCampaignMission.campaignid=summary.campaignid) "
						  "INNER JOIN bindMissionStorage ON (bindMissionStorage.missionid=bindCampaignMission.missionid) "
						  "UNION SELECT summaryid, storageid FROM bindSummaryStorage) r "
						  "LEFT JOIN storage ON (storage.id=r.storageid) "
						  "WHERE summaryid=? ORDER BY storageid", l, &list);

	QVariantList ret;

	foreach(QVariant v, list) {
		QVariantMap m = v.toMap();

		QVariantList objs = objectiveListGet(m["id"].toInt(), filterLevel);
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

QVariantMap Map::storageGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	m_db->execSelectQueryOneRow("SELECT id, name, module, data FROM storage where id=?", l, &map);


	QVariantList missions;

	m_db->execSelectQuery("SELECT mission.id as id, mission.name as name FROM bindMissionStorage "
						  "LEFT JOIN mission ON (mission.id=bindMissionStorage.missionid) "
						  "WHERE storageid=? ORDER BY mission.name", l, &missions);

	map["missions"] = missions;


	QVariantList campaigns;

	m_db->execSelectQuery("SELECT campaign.id as id, campaign.name as name FROM bindSummaryStorage "
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

QVariantList Map::storageListGet(const int &missionId, const int &summaryId, const int &filterLevel)
{
	QVariantList list;

	if (summaryId != -1) {
		QVariantList l;
		l << summaryId;
		m_db->execSelectQuery("SELECT storageid as id, 0 as num, name, module, data FROM bindSummaryStorage "
							  "LEFT JOIN storage ON (storage.id=bindSummaryStorage.storageid) "
							  "WHERE summaryid=? "
							  "ORDER BY storageid", l, &list);
	} else if (missionId != -1) {
		QVariantList l;
		l << missionId;
		m_db->execSelectQuery("SELECT storageid as id, num, name, module, data FROM bindMissionStorage "
							  "LEFT JOIN storage ON (storage.id=bindMissionStorage.storageid) "
							  "WHERE missionid=? "
							  "ORDER BY num", l, &list);
	} else {
		m_db->execSelectQuery("SELECT id, 0 as num, name, module, data FROM storage ORDER BY id", QVariantList(), &list);
	}


	QVariantList ret;

	foreach(QVariant v, list) {
		QVariantMap m = v.toMap();

		QVariantList objs = objectiveListGet(m["id"].toInt(), filterLevel);
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

QVariantMap Map::introGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;
	map["id"] = -1;

	m_db->execSelectQueryOneRow("SELECT id, ttext, img, media, sec, levelMin, levelMax FROM intro where id=?", l, &map);


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
	}

	return list;
}





/**
 * @brief Map::storageInfo
 * @param module
 * @return
 */

QVariantMap Map::storageInfo(const QString &type) const
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


QVariantMap Map::objectiveGet(const int &id)
{
	QVariantList l;
	l << id;
	QVariantMap map;

	m_db->execSelectQueryOneRow("SELECT objective.id as id, objective.storageid as storageid, objective.module as module, objective.data as data, "
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

QVariantList Map::objectiveListGet(const int &storageId, const int filterLevel)
{
	QVariantList list;

	if (storageId != -1) {
		QVariantList l;
		l << storageId;

		if (filterLevel != -1) {
			l << filterLevel;
			m_db->execSelectQuery("SELECT id, storageid, module, data, level FROM objective WHERE storageid=? AND level=? ORDER BY id", l, &list);
		} else {
			m_db->execSelectQuery("SELECT id, storageid, module, data, level FROM objective WHERE storageid=? ORDER BY id", l, &list);
		}
	} else {
		if (filterLevel != -1) {
			QVariantList l;
			l << filterLevel;
			m_db->execSelectQuery("SELECT id, storageid, module, data, level FROM objective WHERE level=? ORDER BY id", l, &list);
		} else {
			m_db->execSelectQuery("SELECT id, storageid, module, data, level FROM objective ORDER BY id", QVariantList(), &list);
		}
	}

	return list;
}




/**
 * @brief Map::objectiveInfo
 * @param type
 * @return
 */

QVariantMap Map::objectiveInfo(const QString &type) const
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

bool Map::databaseInit()
{
	Q_ASSERT(m_client);

	QStringList l;
	l << ":/sql/map.sql";
	l << m_databaseInitSql;

	foreach (QString s, l) {
		if (!m_db->batchQueryFromFile(s)) {
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
 * @brief Map::setStorageModules
 */

void Map::setStorageModules()
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

void Map::setObjectiveModules()
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

QJsonObject Map::loadFromJson(const QByteArray &data, const bool &binaryFormat, double *steps, double *currentStep)
{
	Q_ASSERT (m_client);


	if (steps) {
		*steps += m_tableNames.count()
				  +1		// open
				  +1		// jsonDoc
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

	if (doc.isNull())
		return QJsonObject();

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	QJsonObject root = doc.object();



	if (!JsonToTable(root["storage"].toArray(), "storage", true))
		return QJsonObject();

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	if (!JsonToTable(root["objective"].toArray(), "objective", true))
		return QJsonObject();

	if (steps && currentStep) {
		emit mapLoadingProgress(++(*currentStep)/(*steps));
		QCoreApplication::processEvents();
	}

	foreach (QString t, m_tableNames) {
		if (!JsonToTable(root[t].toArray(), t, false))
			return QJsonObject();

		if (steps && currentStep) {
			emit mapLoadingProgress(++(*currentStep)/(*steps));
			QCoreApplication::processEvents();
		}
	}


	return root;
}


