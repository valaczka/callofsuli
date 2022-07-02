/*
 * ---- Call of Suli ----
 *
 * teachergroups.cpp
 *
 * Created on: 2021. 02. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherGroups
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

#include "teachergroups.h"
#include "teachermaps.h"

TeacherGroups::TeacherGroups(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassTeacher, parent)
	, m_modelMapList(new ObjectGenericListModel<MapListObject>(this))
	, m_selectedGroupId(-1)
	, m_missionNameMap()
	, m_selectedGroupName()
	, m_selectedGroupFullName()
{
	connect(this, &TeacherGroups::groupGet, this, &TeacherGroups::onGroupGet);
	connect(this, &TeacherGroups::groupMapActivate, this, &TeacherGroups::groupReload);
	connect(this, &TeacherGroups::groupMapRemove, this, &TeacherGroups::groupReload);
	connect(this, &TeacherGroups::groupMapAdd, this, &TeacherGroups::groupReload);
	connect(this, &TeacherGroups::gameListUserGet, this, &TeacherGroups::onGameListUserGet);
	connect(this, &TeacherGroups::gameListGroupGet, this, &TeacherGroups::onGameListGroupGet);

	CosDb *db = TeacherMaps::teacherMapsDb(Client::clientInstance(), this);

	if (db) {
		addDb(db, false);
		mapDownloadInfoReload();
	}
}

/**
 * @brief TeacherGroups::~TeacherGroups
 */

TeacherGroups::~TeacherGroups()
{
	delete m_modelMapList;
}


/**
 * @brief TeacherGroups::modelMapList
 * @return
 */

ObjectGenericListModel<MapListObject> *TeacherGroups::modelMapList() const
{
	return m_modelMapList;
}




/**
 * @brief TeacherGroups::groupReload
 */

void TeacherGroups::groupReload(QJsonObject, QByteArray)
{
	if (m_selectedGroupId != -1)
		send("groupGet", {{"id", m_selectedGroupId}});
}


void TeacherGroups::setSelectedGroupId(int selectedGroupId)
{
	if (m_selectedGroupId == selectedGroupId)
		return;

	m_selectedGroupId = selectedGroupId;
	emit selectedGroupIdChanged(m_selectedGroupId);
}


/**
 * @brief TeacherGroups::mapDownload
 * @param data
 */

void TeacherGroups::mapDownload(QVariantMap data)
{
	if (!m_downloader) {
		CosDownloader *dl = new CosDownloader(this, CosMessage::ClassUserInfo, "downloadMap", this);
		dl->setJsonKeyFileName("uuid");
		setDownloader(dl);

		connect(m_downloader, &CosDownloader::oneDownloadFinished, this, &TeacherGroups::onOneDownloadFinished);
		connect(m_downloader, &CosDownloader::downloadFinished, this, [=](){
			mapDownloadInfoReload();
			groupReload();
		});
	}

	m_downloader->clear();

	TeacherMaps::mapDownloadPrivate(data, m_downloader, m_modelMapList);

	if (m_downloader->hasDownloadable()) {
		emit mapDownloadRequest(Client::formattedDataSize(m_downloader->fullSize()));
	} else {
		groupReload();
	}
}


/**
 * @brief TeacherGroups::mapDownloadInfoReload
 */

void TeacherGroups::mapDownloadInfoReload()
{
	if (db())
		m_mapDownloadInfo = TeacherMaps::mapDownloadInfo(db());
	else
		m_mapDownloadInfo.clear();
}


/**
 * @brief TeacherGroups::loadMapDataToModel
 * @param uuid
 * @param model
 */

bool TeacherGroups::loadMapDataToModel(const QString &uuid, GameMapModel *model)
{
	if (!model) {
		qWarning() << "Missing game map model";
		return false;
	}

	if (uuid.isEmpty()) {
		model->clear();
		return false;
	}

	QVariantMap r = db()->execSelectQueryOneRow("SELECT data FROM maps WHERE uuid=?", {uuid});

	if (r.isEmpty()) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen pályaazonosító!"), uuid);
		return false;
	}

	QByteArray b = r.value("data").toByteArray();

	GameMap *map = GameMap::fromBinaryData(b);

	model->setGameMap(map);

	delete map;

	return true;
}






/**
 * @brief TeacherGroups::onGroupGet
 * @param jsonData
 */

void TeacherGroups::onGroupGet(QJsonObject jsonData, QByteArray)
{
	onGroupMapListGet(jsonData.value("mapList").toArray());

	QString name = jsonData.value("name").toString();

	setSelectedGroupName(name);
	QJsonArray classList = jsonData.value("classList").toArray();

	if (classList.isEmpty()) {
		setSelectedGroupFullName(name);
	} else {
		QStringList l;
		foreach (QJsonValue v, classList) {
			QJsonObject o = v.toObject();
			QString s = o.value("name").toString();
			if (!s.isEmpty())
				l.append(s);
		}

		setSelectedGroupFullName(name+" | "+l.join(", "));
	}
}




/**
 * @brief TeacherGroups::onGroupMapListGet
 * @param list
 */

void TeacherGroups::onGroupMapListGet(const QJsonArray &list)
{
	m_modelMapList->unselectAll();

	QJsonArray ret;

	foreach (QJsonValue v, list) {
		QJsonObject m = v.toObject();
		QString uuid = m.value("uuid").toString();

		m["downloaded"] = false;

		if (m_mapDownloadInfo.contains(uuid)) {
			QVariantMap dm = m_mapDownloadInfo.value(uuid).toMap();
			if (m.value("md5") == dm.value("md5") && m.value("dataSize") == dm.value("dataSize"))
				m["downloaded"] = true;
		}

		ret.append(m);
	}

	m_modelMapList->updateJsonArray(ret, "uuid");
}



/**
 * @brief TeacherGroups::onGameListUserGet
 * @param jsonData
 * @param binaryData
 */

void TeacherGroups::onGameListUserGet(QJsonObject jsonData, QByteArray)
{
	if (jsonData.value("groupid").toInt() != m_selectedGroupId) {
		qDebug() << "Invalid groupid";
		return;
	}

	if (jsonData.contains("error")) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Lekérdezési hiba"), jsonData.value("error").toString());
		return;
	}

	QJsonArray list = jsonData.value("list").toArray();
	QVariantList ret;

	if (m_missionNameMap.isEmpty()) {
		m_missionNameMap = TeacherMaps::missionNames(db());
	}

	foreach (QJsonValue v, list) {
		QVariantMap m = v.toObject().toVariantMap();
		QString missionname = m_missionNameMap.value(m.value("mapid").toString()).toMap()
							  .value(m.value("missionid").toString()).toString();

		m["missionname"] = missionname;
		m["duration"] = QTime(0,0).addSecs(m.value("duration").toInt()).toString("mm:ss");
		ret.append(m);
	}

	emit gameListUserReady(ret, jsonData.value("username").toString());
}



/**
 * @brief TeacherGroups::onGameListGroupGet
 * @param jsonData
 */

void TeacherGroups::onGameListGroupGet(QJsonObject jsonData, QByteArray)
{
	if (jsonData.contains("error")) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Lekérdezési hiba"), jsonData.value("error").toString());
		return;
	}

	int groupid = jsonData.value("groupid").toInt();
	QJsonArray list = jsonData.value("list").toArray();
	QVariantList ret;

	if (m_missionNameMap.isEmpty()) {
		m_missionNameMap = TeacherMaps::missionNames(db());
	}

	foreach (QJsonValue v, list) {
		QVariantMap m = v.toObject().toVariantMap();
		QString missionname = m_missionNameMap.value(m.value("mapid").toString()).toMap()
							  .value(m.value("missionid").toString()).toString();

		m["missionname"] = missionname;
		m["duration"] = QTime(0,0).addSecs(m.value("duration").toInt()).toString("mm:ss");
		ret.append(m);
	}

	emit gameListGroupReady(ret, groupid, jsonData.value("username").toString(), jsonData.value("offset").toInt());
}



/**
 * @brief TeacherGroups::onOneDownloadFinished
 * @param item
 * @param data
 */

void TeacherGroups::onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &)
{
	TeacherMaps::mapDownloadFinished(db(), item, data);
	m_missionNameMap = TeacherMaps::missionNames(db());
}

const QString &TeacherGroups::selectedGroupName() const
{
	return m_selectedGroupName;
}

void TeacherGroups::setSelectedGroupName(const QString &newSelectedGroupName)
{
	if (m_selectedGroupName == newSelectedGroupName)
		return;
	m_selectedGroupName = newSelectedGroupName;
	emit selectedGroupNameChanged();
}

const QString &TeacherGroups::selectedGroupFullName() const
{
	return m_selectedGroupFullName;
}

void TeacherGroups::setSelectedGroupFullName(const QString &newSelectedGroupFullName)
{
	if (m_selectedGroupFullName == newSelectedGroupFullName)
		return;
	m_selectedGroupFullName = newSelectedGroupFullName;
	emit selectedGroupFullNameChanged();
}


/**
 * @brief TeacherGroups::newClassModel
 * @return
 */

ObjectListModel *TeacherGroups::newClassModel(QObject *parent) const
{
	return new ObjectGenericListModel<TeacherGroupsClassObject>(parent);
}


/**
 * @brief TeacherGroups::newUserModel
 * @return
 */

ObjectListModel *TeacherGroups::newUserModel(QObject *parent) const
{
	return new ObjectGenericListModel<TeacherGroupsUserObject>(parent);
}


/**
 * @brief TeacherGroups::newMapModel
 * @param parent
 * @return
 */

ObjectListModel *TeacherGroups::newMapModel(QObject *parent) const
{
return new ObjectGenericListModel<MapListObject>(parent);
}
