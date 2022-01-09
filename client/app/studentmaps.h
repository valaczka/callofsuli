/*
 * ---- Call of Suli ----
 *
 * studentmaps.h
 *
 * Created on: 2020. 12. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMaps
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

#ifndef STUDENTMAPS_H
#define STUDENTMAPS_H

#include "abstractactivity.h"
#include "maplistobject.h"
#include "objectlistmodelobject.h"

class StudentMaps : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(ObjectGenericListModel<MapListObject> *modelMapList READ modelMapList NOTIFY modelMapListChanged)
	Q_PROPERTY(bool demoMode READ demoMode NOTIFY demoModeChanged)
	Q_PROPERTY(int selectedGroupId READ selectedGroupId WRITE setSelectedGroupId NOTIFY selectedGroupIdChanged)
	Q_PROPERTY(int baseXP READ baseXP WRITE setBaseXP NOTIFY baseXPChanged)


public:
	explicit StudentMaps(QQuickItem *parent = nullptr);
	virtual ~StudentMaps();

	static CosDb *studentMapsDb(Client *client, QObject *parent = nullptr, const QString &connectionName = "studentMapsDb");

	GameMap * currentMap() const { return m_currentMap; }
	bool demoMode() const { return m_demoMode; }
	int baseXP() const { return m_baseXP; }
	int selectedGroupId() const { return m_selectedGroupId; }

	Q_INVOKABLE void init(const bool &demoMode);

	ObjectGenericListModel<MapListObject> *modelMapList() const;

public slots:
	void mapDownload(MapListObject *map);
	void mapDownload(QList<QObject *> list);
	void mapLoad(MapListObject *map);
	void demoMapLoad();
	void getMissionList();
	void getLevelInfo(const QString &uuid, const int &level, const bool &deathmatch);
	void playGame(const QString &uuid, const int &level, const bool &deathmatch);
	void setBaseXP(int baseXP);
	void setSelectedGroupId(int selectedGroupId);

private slots:
	void onMapListGet(QJsonObject jsonData, QByteArray);
	void onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &);
	bool loadGameMap(GameMap *map, MapListObject *mapObject = nullptr);
	void unloadGameMap();

	void onDemoGameWin();
	void onGameEnd(GameMatch *match, const bool &win = false);

	void onMissionListGet(QJsonObject jsonData, QByteArray);
	void onGameCreate(QJsonObject jsonData, QByteArray);
	void onGameFinish(QJsonObject jsonData, QByteArray);
	void onGameListUserGet(QJsonObject jsonData, QByteArray);

signals:
	void groupListGet(QJsonObject jsonData, QByteArray binaryData);

	void mapListLoaded(QVariantList list);
	void mapListGet(QJsonObject jsonData, QByteArray binaryData);
	void mapDownloadRequest(QString formattedDataSize);
	void mapDownloadFinished(const QList<MapListObject*> list);

	void missionListGet(QJsonObject jsonData, QByteArray binaryData);
	void missionSelected(const int &index);
	void solvedMissionListReady(const QVariantList &list);

	void levelInfoReady(const QVariantMap &info);

	void userListGet(QJsonObject jsonData, QByteArray binaryData);

	void gameMapLoaded(MapListObject *map);
	void gameMapUnloaded();

	void gamePlayReady(GameMatch *gameMatch);
	void gameStarted();
	void gameFinishDialogReady(QVariantMap data);

	void gameCreate(QJsonObject jsonData, QByteArray binaryData);
	void gameFinish(QJsonObject jsonData, QByteArray binaryData);
	void gameUpdate(QJsonObject jsonData, QByteArray binaryData);

	void gameListUserGet(QJsonObject jsonData, QByteArray binaryData);
	void gameListUserReady(const QVariantList &list, const QString &username, const int &offset);
	void gameListUserMissionGet(QJsonObject jsonData, QByteArray binaryData);

	void demoModeChanged(bool demoMode);
	void baseXPChanged(int baseXP);
	void selectedGroupIdChanged(int selectedGroupId);
	void modelMapListChanged();

private:
	void _createDownloader();

	const QString m_demoMapFile;
	ObjectGenericListModel<MapListObject> *m_modelMapList;
	GameMap * m_currentMap;
	bool m_demoMode;
	QVariantMap m_demoSolverMap;
	int m_baseXP;
	int m_selectedGroupId;
	QVariantMap m_missionNameMap;
};

#endif // STUDENTMAPS_H
