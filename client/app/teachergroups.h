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
#include "gamemapmodel.h"

class TeacherGroups : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(VariantMapModel * modelGroupList READ modelGroupList WRITE setModelGroupList NOTIFY modelGroupListChanged)
	Q_PROPERTY(VariantMapModel * modelUserList READ modelUserList WRITE setModelUserList NOTIFY modelUserListChanged)
	Q_PROPERTY(VariantMapModel * modelMapList READ modelMapList WRITE setModelMapList NOTIFY modelMapListChanged)

	Q_PROPERTY(int selectedGroupId READ selectedGroupId WRITE setSelectedGroupId NOTIFY selectedGroupIdChanged)


public:
	explicit TeacherGroups(QQuickItem *parent = nullptr);
	~TeacherGroups();

	VariantMapModel * modelGroupList() const { return m_modelGroupList; }
	VariantMapModel * modelUserList() const { return m_modelUserList; }
	VariantMapModel * modelMapList() const { return m_modelMapList; }

	int selectedGroupId() const { return m_selectedGroupId; }

public slots:
	void groupSelect(const int &groupId);
	void groupReload(QJsonObject = QJsonObject(), QByteArray = QByteArray());
	void setModelGroupList(VariantMapModel * modelGroupList);
	void setModelUserList(VariantMapModel * modelUserList);
	void setModelMapList(VariantMapModel * modelMapList);
	void setSelectedGroupId(int selectedGroupId);
	void mapDownload(QVariantMap data);
	void mapDownloadInfoReload();
	bool loadMapDataToModel(const QString &uuid, GameMapModel *model);

protected slots:
	void clientSetup() override;

private slots:
	void onGroupListGet(QJsonObject jsonData, QByteArray);
	void onGroupGet(QJsonObject jsonData, QByteArray);
	void onGroupMapListGet(const QJsonArray &list);
	void onGameListUserGet(QJsonObject jsonData, QByteArray);
	void onGameListMapGet(QJsonObject jsonData, QByteArray);

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

	void gameListUserGet(QJsonObject jsonData, QByteArray binaryData);
	void gameListUserReady(const QVariantList &list, const QString &username);
	void gameListMapGet(QJsonObject jsonData, QByteArray binaryData);
	void gameListMapReady(const QVariantList &list, const QString &mapid, const QString &username);

	void modelGroupListChanged(VariantMapModel * modelGroupList);
	void modelUserListChanged(VariantMapModel * modelUserList);
	void modelMapListChanged(VariantMapModel * modelMapList);
	void selectedGroupIdChanged(int selectedGroupId);

private:
	VariantMapModel * m_modelGroupList;
	VariantMapModel * m_modelUserList;
	VariantMapModel * m_modelMapList;
	int m_selectedGroupId;
	QVariantMap m_mapDownloadInfo;
	QVariantMap m_missionNameMap;
};

#endif // TEACHERGROUPS_H
