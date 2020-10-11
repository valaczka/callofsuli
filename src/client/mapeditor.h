/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
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

#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <QObject>
#include "mapdata.h"

class MapEditor : public MapData
{
	Q_OBJECT

public:

	Q_PROPERTY(QString mapUuid READ mapUuid WRITE setMapUuid NOTIFY mapUuidChanged)
	Q_PROPERTY(QString mapTimeCreated READ mapTimeCreated WRITE setMapTimeCreated NOTIFY mapTimeCreatedChanged)
	Q_PROPERTY(QString mapOriginalFile READ mapOriginalFile WRITE setMapOriginalFile NOTIFY mapOriginalFileChanged)
	Q_PROPERTY(bool mapModified READ mapModified WRITE setMapModified NOTIFY mapModifiedChanged)


	MapEditor(QObject *parent = nullptr);

	QString mapUuid() const { return m_mapUuid; }
	QString mapTimeCreated() const { return m_mapTimeCreated; }
	QString mapOriginalFile() const { return m_mapOriginalFile; }
	bool mapModified() const { return m_mapModified; }

public slots:
	void save(const int &mapId = -1, const bool &binaryFormat = true);
	bool loadFromJson(const QByteArray &data, const bool &binaryFormat = true);
	bool loadFromFile(const QString &filename, const bool &binaryFormat = true);
	bool loadFromBackup();

	static QByteArray create(const bool &binaryFormat = true);

	QByteArray saveToJson(const bool &binaryFormat = true);
	bool saveToFile(const QString &filename, const QByteArray &data = QByteArray());
	bool saveToFile(const QUrl &url, const QByteArray &data = QByteArray()) { return saveToFile(url.toLocalFile(), data); }

	void updateMapOriginalFile(const QString &filename);
	void updateMapServerId(const int &serverId, const int &mapId);
	void setMapUuid(QString mapUuid);
	void setMapTimeCreated(QString mapTimeCreated);
	void setMapOriginalFile(QString mapOriginalFile);
	void setMapModified(bool mapModified);

	QVariantMap infoGet() override;
	void infoUpdate(const QVariantMap &map);

	bool campaignUpdate(const int &id, const QVariantMap &params);
	int campaignAdd(const QVariantMap &params);
	bool campaignMissionAdd(const int &id, const int &missionId, const int &num = -1);
	int campaignSummaryAdd(const int &id);
	bool campaignIntroAdd(const int &id, const int &introId, const bool &isOutro = false);
	bool campaignLockSet(const int &id, const QVariantList &lockIdList);
	bool campaignRemove(const int &id);

	bool missionUpdate(const int &id, const QVariantMap &params);
	int missionAdd(const QVariantMap &params);
	int missionLevelAdd(const QVariantMap &params);
	bool missionLevelUpdate(const int &id, const int &missionId, const QVariantMap &params);
	bool missionLevelRemove(const int &id, const int &missionId);
	int missionStorageAdd(const QVariantMap &params);
	bool missionStorageUpdate(const int &id, const int &missionId, const QVariantMap &params);
	bool missionStorageRemove(const int &missionId, const int &storageId);
	bool missionIntroAdd(const int &id, const int &introId, const bool &isOutro = false);
	bool missionCampaignListSet(const int &id, const QVariantList &campaignIdList);
	bool missionRemove(const int &id);

	int summaryAdd(const int &campaignId);
	int summaryLevelAdd(const QVariantMap &params);
	bool summaryLevelUpdate(const int &id, const int &summaryId, const QVariantMap &params);
	bool summaryLevelRemove(const int &id, const int &summaryId);
	int summaryStorageAdd(const QVariantMap &params);
	bool summaryStorageRemove(const int &summaryId, const int &storageId);
	bool summaryIntroAdd(const int &id, const int &introId, const bool &isOutro = false);
	bool summaryRemove(const int &id);

	int storageAdd(const QVariantMap &params, const int &missionId = -1, const int &summaryId = -1);
	bool storageUpdate(const int &id, const QVariantMap &params, const QJsonObject &jsonData = QJsonObject(), const int &missionId = -1, const int &summaryId = -1);
	bool storageMissionListSet(const int &id, const QVariantList &missionIdList);
	bool storageSummaryListSet(const int &id, const QVariantList &summaryIdList);
	bool storageRemove(const int &id, const int &missionId = -1, const int &summaryId = -1);

	int introAdd(const QVariantMap &params);
	bool introUpdate(const int &id, const QVariantMap &params, const int &parentId = -1, const IntroType &type = IntroUndefined);
	bool introRemove(const int &id);

	int objectiveAdd(const QVariantMap &params, const int &missionId = -1, const int &summaryId = -1);
	bool objectiveRemove(const int &id, const int &missionId = -1, const int &summaryId = -1);
	bool objectiveUpdate(const int &id, const QVariantMap &params, const QJsonObject &jsonData = QJsonObject(), const int &missionId = -1, const int &summaryId = -1);


signals:
	void mapBackupExists(const QString &originalFile, const QString &uuid, const int &serverid, const int &mapid);
	void mapLoaded();
	void mapLoadedFromBackup();
	void mapSaved(const QByteArray &data, const QString &uuid, const int &mapId);

	void mapTitleChanged(QString mapTitle);
	void mapUuidChanged(QString mapUuid);
	void mapTimeCreatedChanged(QString mapTimeCreated);
	void mapOriginalFileChanged(QString mapOriginalFile);
	void mapModifiedChanged(bool mapModified);


	void campaignListUpdated();
	void campaignUpdated(const int &id);
	void missionListUpdated(const int &id);
	void missionUpdated(const int &id);
	void summaryUpdated(const int &id);
	void storageListUpdated(const int &mId, const int &sId);
	void storageUpdated(const int &id);
	void introListUpdated(const int &parentId, const IntroType &type);
	void introUpdated(const int &id);
	void objectiveUpdated(const int &id);

private:
	QString m_mapUuid;
	QString m_mapTimeCreated;
	QString m_mapOriginalFile;
	bool m_mapModified;
};

#endif // MAPEDITOR_H
