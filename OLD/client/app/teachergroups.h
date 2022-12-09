/*
 * ---- Call of Suli ----
 *
 * teachergroups.h
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

#ifndef TEACHERGROUPS_H
#define TEACHERGROUPS_H

#include "abstractactivity.h"
#include "maplistobject.h"
#include "objectlistmodel.h"


class TeacherGroupsUserObject;
class TeacherGroupsClassObject;


/**
 * @brief The TeacherGroups class
 */


class TeacherGroups : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(ObjectGenericListModel<MapListObject> * modelMapList READ modelMapList NOTIFY modelMapListChanged)

	Q_PROPERTY(int selectedGroupId READ selectedGroupId WRITE setSelectedGroupId NOTIFY selectedGroupIdChanged)
	Q_PROPERTY(QString selectedGroupName READ selectedGroupName WRITE setSelectedGroupName NOTIFY selectedGroupNameChanged)
	Q_PROPERTY(QString selectedGroupFullName READ selectedGroupFullName WRITE setSelectedGroupFullName NOTIFY selectedGroupFullNameChanged)

	Q_PROPERTY(QVariantList mapList READ mapList CONSTANT)
	Q_PROPERTY(QVariantList gradeList READ gradeList CONSTANT)


public:
	explicit TeacherGroups(QQuickItem *parent = nullptr);
	~TeacherGroups();

	ObjectGenericListModel<MapListObject> * modelMapList() const;

	int selectedGroupId() const { return m_selectedGroupId; }

	const QString &selectedGroupName() const;
	void setSelectedGroupName(const QString &newSelectedGroupName);

	const QString &selectedGroupFullName() const;
	void setSelectedGroupFullName(const QString &newSelectedGroupFullName);

	Q_INVOKABLE ObjectListModel* newClassModel(QObject *parent) const;
	Q_INVOKABLE ObjectListModel* newUserModel(QObject *parent) const;
	Q_INVOKABLE ObjectListModel* newMapModel(QObject *parent) const;

	Q_INVOKABLE QVariantMap grade(const int &id) const;
	Q_INVOKABLE QVariantMap mapMission(const QString &mapUuid, const QString &missionUuid) const;
	Q_INVOKABLE QVariantList missionList(const QString &mapUuid = "") const;

	const QVariantList &mapList() const;
	QVariantList gradeList() const;

public slots:
	void groupReload(QJsonObject = QJsonObject(), QByteArray = QByteArray());
	void setSelectedGroupId(int selectedGroupId);
	void mapDownload(QVariantMap data);
	void mapDownloadInfoReload();

protected slots:
	virtual void onMessageReceived(const CosMessage &message) override;

private slots:
	void onGroupGet(QJsonObject jsonData, QByteArray);
	void onGroupMapListGet(const QJsonArray &list);
	void onGameListUserGet(QJsonObject jsonData, QByteArray);
	void onGameListGroupGet(QJsonObject jsonData, QByteArray);
	void onCampaignGet(QJsonObject jsonData, QByteArray);
	void onCampaignListGet(QJsonObject jsonData, QByteArray);
	void onCampaignResultGet(QJsonObject jsonData, QByteArray);

	void onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &);

signals:
	void mapDownloadRequest(QString formattedDataSize);

	void groupListGet(QJsonObject jsonData, QByteArray binaryData);
	void groupGet(QJsonObject jsonData, QByteArray binaryData);
	void groupRemove(QJsonObject jsonData, QByteArray binaryData);
	void groupModify(QJsonObject jsonData, QByteArray binaryData);

	void groupUserGet(QJsonObject jsonData, QByteArray binaryData);
	void groupUserAdd(QJsonObject jsonData, QByteArray binaryData);
	void groupUserRemove(QJsonObject jsonData, QByteArray binaryData);
	void groupExcludedUserListGet(QJsonObject jsonData, QByteArray binaryData);

	void groupClassAdd(QJsonObject jsonData, QByteArray binaryData);
	void groupClassRemove(QJsonObject jsonData, QByteArray binaryData);
	void groupExcludedClassListGet(QJsonObject jsonData, QByteArray binaryData);

	void groupMapAdd(QJsonObject jsonData, QByteArray binaryData);
	void groupMapActivate(QJsonObject jsonData, QByteArray binaryData);
	void groupMapRemove(QJsonObject jsonData, QByteArray binaryData);
	void groupExcludedMapListGet(QJsonObject jsonData, QByteArray binaryData);
	void groupTrophyGet(QJsonObject jsonData, QByteArray binaryData);
	void groupExamListGet(QJsonObject jsonData, QByteArray binaryData);

	void gameListUserGet(QJsonObject jsonData, QByteArray binaryData);
	void gameListUserReady(const QVariantList &list, const QString &username);
	void gameListMapReady(const QVariantList &list, const QString &mapid, const QString &username);
	void gameListGroupGet(QJsonObject jsonData, QByteArray binaryData);
	void gameListGroupReady(const QVariantList &list, const int &groupid, const QString &username, const int &offset);

	void campaignListGet(QJsonObject jsonData, QByteArray binaryData);
	void campaignGet(QJsonObject jsonData, QByteArray binaryData);
	void campaignGetReady(const QJsonObject &jsonData);
	void campaignAdd(QJsonObject jsonData, QByteArray binaryData);
	void campaignRemove(QJsonObject jsonData, QByteArray binaryData);
	void campaignModify(QJsonObject jsonData, QByteArray binaryData);
	void campaignFinish(QJsonObject jsonData, QByteArray binaryData);
	void campaignResultGet(QJsonObject jsonData, QByteArray binaryData);
	void campaignResultGetReady(int id, QJsonArray list);

	void examEngineCreate(QJsonObject jsonData, QByteArray binaryData);
	void examEngineMapGet(QJsonObject jsonData, QByteArray binaryData);
	void examEngineMessage(QString func, QJsonObject jsonData);

	void modelMapListChanged();
	void selectedGroupIdChanged(int selectedGroupId);
	void selectedGroupNameChanged();
	void selectedGroupFullNameChanged();


private:
	ObjectGenericListModel<MapListObject> * m_modelMapList;
	int m_selectedGroupId;
	QVariantMap m_mapDownloadInfo;
	QVariantMap m_missionNameMap;
	QString m_selectedGroupName;
	QString m_selectedGroupFullName;
	QMap<int, QVariantMap> m_gradeMap;
	QVariantList m_mapList;
};




/**
 * @brief The TeacherGroupsUserObject class
 */

class TeacherGroupsUserObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(QString username MEMBER m_username)
	Q_PROPERTY(QString firstname MEMBER m_firstname)
	Q_PROPERTY(QString lastname MEMBER m_lastname)
	Q_PROPERTY(QString classname MEMBER m_classname)
	Q_PROPERTY(int classid MEMBER m_classid)
	Q_PROPERTY(QString rankimage MEMBER m_rankimage)
	Q_PROPERTY(int rankid MEMBER m_rankid)
	Q_PROPERTY(int ranklevel MEMBER m_ranklevel)
	Q_PROPERTY(bool active MEMBER m_active)

public:
	Q_INVOKABLE explicit TeacherGroupsUserObject(QObject *parent = nullptr)
		: ObjectListModelObject(parent)
		, m_rankid(-1)
		, m_ranklevel(-1)
		, m_classid(-1)
	{

	}

private:
	QString m_username;
	QString m_firstname;
	QString m_lastname;
	QString m_classname;
	QString m_rankimage;
	int m_rankid;
	int m_ranklevel;
	bool m_active;
	int m_classid;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<TeacherGroupsUserObject>*);





/**
 * @brief The TeacherGroupsClassObject class
 */


class TeacherGroupsClassObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(int classid MEMBER m_classid)
	Q_PROPERTY(QString name MEMBER m_name)

public:
	Q_INVOKABLE explicit TeacherGroupsClassObject(QObject *parent = nullptr)
		: ObjectListModelObject(parent)
		, m_classid(-1)
	{

	}

private:
	int m_classid;
	QString m_name;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<TeacherGroupsClassObject>*);

#endif // TEACHERGROUPS_H
