/*
 * ---- Call of Suli ----
 *
 * map.h
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

#ifndef MAP_H
#define MAP_H

#include <QObject>
#include "abstractdbactivity.h"


class Map : public AbstractDbActivity
{
	Q_OBJECT

public:

	enum MapType { MapInvalid, MapEditor, MapGame, MapCustom };
	Q_ENUM(MapType)

	enum IntroType { IntroUndefined, IntroCampaign, IntroMission, IntroSummary, IntroChapter };
	Q_ENUM(IntroType)

	Q_PROPERTY(QString mapUuid READ mapUuid WRITE setMapUuid NOTIFY mapUuidChanged)
	Q_PROPERTY(QString mapTimeCreated READ mapTimeCreated WRITE setMapTimeCreated NOTIFY mapTimeCreatedChanged)
	Q_PROPERTY(QString mapOriginalFile READ mapOriginalFile WRITE setMapOriginalFile NOTIFY mapOriginalFileChanged)
	Q_PROPERTY(MapType mapType READ mapType WRITE setMapType NOTIFY mapTypeChanged)
	Q_PROPERTY(bool mapModified READ mapModified WRITE setMapModified NOTIFY mapModifiedChanged)


	Map(QObject *parent = nullptr);
	virtual ~Map() override;


	QString mapUuid() const { return m_mapUuid; }
	QString mapTimeCreated() const { return m_mapTimeCreated; }
	QString mapOriginalFile() const { return m_mapOriginalFile; }
	MapType mapType() const { return m_mapType; }
	bool mapModified() const { return m_mapModified; }

public slots:
	void save(const int &mapId = -1, const bool &binaryFormat = true);
	bool loadFromJson(const QByteArray &data, const bool &binaryFormat = true);
	bool loadFromFile(const QString &filename, const bool &binaryFormat = true);
	bool loadFromBackup();
	QByteArray saveToJson(const bool &binaryFormat = true);
	static QByteArray create(const bool &binaryFormat = true);
	bool saveToFile(const QString &filename, const QByteArray &data = QByteArray());
	bool saveToFile(const QUrl &url, const QByteArray &data = QByteArray()) { return saveToFile(url.toLocalFile(), data); }
	void updateMapOriginalFile(const QString &filename);
	void updateMapServerId(const int &serverId, const int &mapId);
	bool databaseCheck();
	bool databasePrepare();

	void setMapType(MapType mapType);
	void setMapUuid(QString mapUuid);
	void setMapTimeCreated(QString mapTimeCreated);
	void setMapOriginalFile(QString mapOriginalFile);
	void setMapModified(bool mapModified);

	QVariantMap infoGet();
	void infoUpdate(const QVariantMap &map);

	QVariantMap campaignGet(const int &id);
	QVariantList campaignListGet();
	bool campaignUpdate(const int &id, const QVariantMap &params);
	int campaignAdd(const QVariantMap &params);
	bool campaignMissionAdd(const int &id, const int &missionId, const int &num = -1);
	int campaignSummaryAdd(const int &id);
	bool campaignIntroAdd(const int &id, const int &introId, const bool &isOutro = false);
	bool campaignLockSet(const int &id, const QVariantList &lockIdList);

	QVariantMap missionGet(const int &id, const bool &isSummary = false);
	QVariantList missionListGet(const int &campaignId = -1);
	bool missionUpdate(const int &id, const QVariantMap &params, const int &campaignId = -1);
	int missionAdd(const QVariantMap &params);
	int missionLevelAdd(const QVariantMap &params);
	bool missionLevelUpdate(const int &id, const int &missionId, const QVariantMap &params);
	bool missionLevelRemove(const int &id, const int &missionId);
	int missionChapterAdd(const QVariantMap &params);
	bool missionChapterUpdate(const int &id, const int &missionId, const QVariantMap &params);
	bool missionChapterRemove(const int &missionId, const int &chapterId);
	bool missionIntroAdd(const int &id, const int &introId, const bool &isOutro = false);
	bool missionCampaignListSet(const int &id, const QVariantList &campaignIdList);

	QVariantMap summaryGet(const int &id) { return missionGet(id, true); }
	int summaryAdd(const int &campaignId);
	int summaryLevelAdd(const QVariantMap &params);
	bool summaryLevelUpdate(const int &id, const int &summaryId, const QVariantMap &params);
	bool summaryLevelRemove(const int &id, const int &summaryId);
	int summaryChapterAdd(const QVariantMap &params);
	bool summaryChapterRemove(const int &summaryId, const int &chapterId);
	bool summaryIntroAdd(const int &id, const int &introId, const bool &isOutro = false);

	QVariantMap chapterGet(const int &id);
	QVariantList chapterListGet(const int &missionId = -1, const int &summaryId = -1);
	int chapterAdd(const QVariantMap &params);
	bool chapterUpdate(const int &id, const QVariantMap &params, const int &missionId = -1, const int &summaryId = -1);
	bool chapterIntroAdd(const int &id, const int &introId);
	bool chapterMissionListSet(const int &id, const QVariantList &missionIdList);
	bool chapterSummaryListSet(const int &id, const QVariantList &summaryIdList);

	QVariantMap introGet(const int &id);
	QVariantList introListGet(const int &parentId = -1, const IntroType &type = IntroUndefined);
	int introAdd(const QVariantMap &params);
	bool introUpdate(const int &id, const QVariantMap &params, const int &parentId = -1, const IntroType &type = IntroUndefined);

signals:
	void mapBackupExists(const QString &originalFile, const QString &uuid, const int &serverid, const int &mapid);
	void mapLoaded();
	void mapLoadedFromBackup();
	void mapSaved(const QByteArray &data, const QString &uuid, const int &mapId);

	void mapTitleChanged(QString mapTitle);
	void mapUuidChanged(QString mapUuid);
	void mapTimeCreatedChanged(QString mapTimeCreated);
	void mapOriginalFileChanged(QString mapOriginalFile);
	void mapTypeChanged(MapType mapType);
	void mapModifiedChanged(bool mapModified);


	void campaignListUpdated();
	void campaignUpdated(const int &id);
	void missionListUpdated(const int &id);
	void missionUpdated(const int &id);
	void summaryUpdated(const int &id);
	void chapterListUpdated(const int &mId, const int &sId);
	void chapterUpdated(const int &id);
	void introListUpdated(const int &parentId, const IntroType &type);
	void introUpdated(const int &id);

private:
	QJsonArray tableToJson(const QString &table, const bool &convertData = false);
	bool JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData = false);

	QStringList m_tableNames;
	QString m_mapUuid;
	QString m_mapTimeCreated;
	QString m_mapOriginalFile;
	MapType m_mapType;
	bool m_mapModified;
};

#endif // MAP_H
