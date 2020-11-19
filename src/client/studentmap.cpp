/*
 * ---- Call of Suli ----
 *
 * studentmap.cpp
 *
 * Created on: 2020. 06. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMap
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

#include "studentmap.h"

StudentMap::StudentMap(QQuickItem *parent)
	: MapData("mapStudentDB", parent)
{
	m_databaseInitSql << ":/sql/studentmap.sql";

	//setDatabaseFile(Client::standardPath("tmpmap.db"));

	m_uuid = "";
	m_student = nullptr;

	connect(this, &StudentMap::studentChanged, this, &StudentMap::onStudentChanged);
}


/**
 * @brief StudentMap::loadFromRepository
 * @param uuid
 * @return
 */

bool StudentMap::loadFromRepository(const QString &uuid, const QString &md5)
{
	Q_ASSERT(m_client);
	Q_ASSERT(m_student);

	QVariantList l;
	l << uuid;

	QVariantMap r;
	//m_student->mapRepository()->db()->execSelectQueryOneRow("SELECT data FROM mapdata WHERE uuid=?", l, &r);

	QByteArray mapdata = r.value("data").toByteArray();

	if (mapdata.isEmpty() || (!md5.isEmpty() && QCryptographicHash::hash(mapdata, QCryptographicHash::Md5).toHex() != md5)) {
		mapDownloadRequest(uuid);
		return false;
	}

	emit mapLoadingStarted(uuid);

	double steps = 0;
	double currentStep = 0.0;

	QJsonObject root = MapData::loadFromJson(mapdata, true, &steps, &currentStep);

	if (root.isEmpty()) {
		qWarning() << "JSON error" << uuid;
		emit mapDownloadError();
		return false;
	}

	emit mapLoaded(uuid);

	return true;
}


/**
 * @brief StudentMap::onMapDataReceived
 * @param jsonData
 * @param mapData
 */

void StudentMap::onMapDataReceived(const QJsonObject &jsonData, const QByteArray &mapData)
{
	if (jsonData.isEmpty() || mapData.isEmpty()) {
		qWarning() << tr("Invalid parameters");
		return;
	}

	QString uuid = jsonData.value("uuid").toString();

	if (m_student->mapRepository()->add(uuid, mapData, jsonData.value("md5").toString()) == -1) {
		emit mapDownloadError();
		return;
	}

	emit mapDownloaded(uuid);

	loadFromRepository(uuid);
}




/**
 * @brief StudentMap::onMapResultListLoaded
 * @param list
 */

void StudentMap::onMapResultListLoaded(const QJsonArray &list)
{
	m_db->execSimpleQuery("DELETE FROM result");

	foreach (QJsonValue v, list) {
		QJsonObject o = v.toObject();

		QVariantMap params;
		params["uuid"] = o.value("uuid").toString();
		params["level"] = o.value("level").toInt();
		params["attempt"] = o.value("attempt").toInt();
		params["success"] = o.value("success").toInt();

		if (m_db->execInsertQuery("INSERT INTO result(?k?) VALUES (?)", params) == -1) {
			emit mapDownloadError();
			return;
		}
	}

	emit mapResultUpdated();
}


/**
 * @brief StudentMap::getMissionList
 * @return
 */

void StudentMap::campaignListUpdate()
{
	QVariantList ret;

	QVariantList campaigns;
//	m_db->execSelectQuery("SELECT id, name FROM campaign ORDER BY num", QVariantList(), &campaigns);

	foreach (QVariant v, campaigns) {
		QVariantMap m = v.toMap();
		int cid = m.value("id").toInt();
		QString cname = m.value("name").toString();

		bool locked = false;

		QVariantList l;
		l << cid;
		QVariantList locks;
	//	m_db->execSelectQuery("SELECT lockId FROM campaignLock WHERE campaignId=?", l, &locks);

		foreach (QVariant v, locks) {
			QVariantMap m = v.toMap();
			int lockCid = m.value("lockId").toInt();

			if (!isCampaignCompleted(lockCid)) {
				locked = true;
				break;
			}
		}


		if (locked)
			continue;


		QVariantList missionList;

		bool prevMissionCompleted = true;

		QVariantList missions;
	/*	m_db->execSelectQuery("SELECT mission.id as id, uuid, mission.name FROM bindCampaignMission "
							  "LEFT JOIN mission ON (mission.id=bindCampaignMission.missionid) "
							  "WHERE campaignId=? ORDER BY num",
							  l, &missions);*/

		foreach (QVariant v, missions) {
			QVariantMap m = v.toMap();
			int mid = m.value("id").toInt();
			QString mUuid = m.value("uuid").toString();
			QString mname = m.value("name").toString();

			QVariantList availableLevels;

			bool hasCompleteLevel = false;

			if (prevMissionCompleted) {
				QVariantList p;
				p << mUuid;

				QVariantList levels;
	/*			m_db->execSelectQuery("SELECT missionlevel.level, success "
									  "FROM mission "
									  "LEFT JOIN missionLevel ON (missionLevel.missionid=mission.id) "
									  "LEFT JOIN result ON (result.uuid=mission.uuid AND result.level=missionLevel.level) "
									  "WHERE mission.uuid=? ORDER BY missionLevel.level",
									  p, &levels);*/


				for (int i=0; i<levels.count(); ++i) {
					QVariantMap m = levels.at(i).toMap();
					int level = m.value("level").toInt();
					int success = m.value("success", 0).toInt();

					if (!success && i>0)
						break;

					QVariantMap lm;
					lm["level"] = level;
					availableLevels << lm;

					if (success)
						hasCompleteLevel = true;
				}
			}

			QVariantMap record;
			record["type"] = "mission";
			record["id"] = mid;
			record["uuid"] = mUuid;
			record["locked"] = !prevMissionCompleted;
			record["name"] = mname;
			record["levels"] = availableLevels;

			missionList << record;


			if (!prevMissionCompleted)
				break;

			if (!hasCompleteLevel)
				prevMissionCompleted = false;
		}



		QVariantList summaryLevels;
/*		m_db->execSelectQuery("SELECT summary.id as id, summary.uuid as uuid, summaryLevel.level as level, success, "
							  "EXISTS(SELECT * FROM bindCampaignMission "
							  "LEFT JOIN mission ON (mission.id=bindCampaignMission.missionid) "
							  "LEFT JOIN result ON (result.uuid=mission.uuid) "
							  "WHERE campaignId=summary.campaignId AND "
							  "((result.level=summaryLevel.level AND (success<1 OR success IS null)) OR result.level=0 OR result.level IS NULL)"
							  ") as incomplete "
							  "FROM summary "
							  "LEFT JOIN summaryLevel ON (summaryLevel.summaryid=summary.id) "
							  "LEFT JOIN result ON (result.uuid=summary.uuid AND result.level=summaryLevel.level) "
							  "WHERE campaignId=? ORDER BY summaryLevel.level",
							  l, &summaryLevels);*/


		QVariantList availableSummaryLevels;
		int sid = -1;
		QString sUuid;

		foreach (QVariant v, summaryLevels) {
			QVariantMap m = v.toMap();
			int level = m.value("level").toInt();
			bool success = m.value("success", false).toBool();
			bool incomplete = m.value("incomplete", true).toBool();
			sid = m.value("id").toInt();
			sUuid = m.value("uuid").toString();

			if (!incomplete) {
				QVariantMap lm;
				lm["level"] = level;
				availableSummaryLevels << lm;
			}

			if (incomplete || !success)
				break;
		}

		if (!availableSummaryLevels.isEmpty()) {
			QVariantMap record;
			record["type"] = "summary";
			record["id"] = sid;
			record["uuid"] = sUuid;
			record["locked"] = false;
			record["name"] = cname;
			record["levels"] = availableSummaryLevels;

			missionList << record;
		}


		QVariantMap record;
		record["type"] = "campaign";
		record["id"] = cid;
		record["uuid"] = "";
		record["locked"] = locked;
		record["name"] = cname;
		record["levels"] = QVariantList();
		record["missions"] = missionList;

		ret << record;
	}

	setCampaignList(ret);
}


/**
 * @brief StudentMap::missionListGet
 * @param campaignId
 * @return
 */

QVariantList StudentMap::missionListGet(const int &campaignId)
{
	foreach (QVariant v, m_campaignList) {
		QVariantMap m = v.toMap();
		if (m.value("id", -1).toInt() == campaignId)
			return m.value("missions").toList();
	}

	return QVariantList();
}






/**
 * @brief StudentMap::setStudent
 * @param student
 */

void StudentMap::setStudent(Student *student)
{
	if (m_student == student)
		return;

	m_student = student;
	emit studentChanged(m_student);
}

void StudentMap::setCampaignList(QVariantList campaignList)
{
	m_campaignList = campaignList;
	emit campaignListChanged(m_campaignList);
}


/**
 * @brief StudentMap::onStudentChanged
 * @param student
 */

void StudentMap::onStudentChanged(Student *)
{
	if (!m_student)
		return;

	connect(m_student, &Student::mapDataReceived, this, &StudentMap::onMapDataReceived);
	connect(this, &StudentMap::mapDownloadRequest, m_student, &Student::mapDownload);
	connect(m_student, &Student::mapResultListLoaded, this, &StudentMap::onMapResultListLoaded);
}


/**
 * @brief StudentMap::isCampaignUnlocked
 * @param campaignId
 * @return
 */

bool StudentMap::isCampaignCompleted(const int &campaignId)
{
	QVariantList uuids;

	QVariantList params;
	params << campaignId;
	params << campaignId;

/*	m_db->execSelectQuery("SELECT r.uuid, EXISTS(SELECT * FROM result WHERE result.uuid=r.uuid AND success>0) as success "
						  "FROM (SELECT uuid FROM bindCampaignMission "
						  "LEFT JOIN  mission ON (mission.id=bindCampaignMission.missionid) "
						  "WHERE campaignId=? "
						  "UNION SELECT uuid FROM summary WHERE campaignId=?) r",
						  params, &uuids);*/

	bool ret = true;

	foreach (QVariant v, uuids) {
		if (!v.toMap().value("success", false).toBool()) {
			ret = false;
			break;
		}
	}

	return ret;
}


