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
#include "variantmapmodel.h"
#include "variantmapdata.h"

class StudentMaps : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(bool demoMode READ demoMode WRITE setDemoMode NOTIFY demoModeChanged)
	Q_PROPERTY(VariantMapModel * modelMapList READ modelMapList NOTIFY modelMapListChanged)
	Q_PROPERTY(VariantMapModel * modelMissionList READ modelMissionList NOTIFY modelMissionListChanged)
	Q_PROPERTY(VariantMapModel * modelGroupList READ modelGroupList WRITE setModelGroupList NOTIFY modelGroupListChanged)
	Q_PROPERTY(VariantMapModel * modelCharacterList READ modelCharacterList WRITE setModelCharacterList NOTIFY modelCharacterListChanged)
	Q_PROPERTY(int selectedGroupId READ selectedGroupId WRITE setSelectedGroupId NOTIFY selectedGroupIdChanged)

	Q_PROPERTY(int baseXP READ baseXP WRITE setBaseXP NOTIFY baseXPChanged)


public:
	explicit StudentMaps(QQuickItem *parent = nullptr);
	~StudentMaps();

	static CosDb *studentMapsDb(Client *client, QObject *parent = nullptr, const QString &connectionName = "studentMapsDb");

	//Q_INVOKABLE virtual void run(const QString &func, QVariantMap data = QVariantMap()) override { AbstractActivity::run(m_map, func, data); };

	VariantMapModel * modelMapList() const { return m_modelMapList; }
	VariantMapModel * modelMissionList() const { return m_modelMissionList; }
	VariantMapModel * modelGroupList() const { return m_modelGroupList; }
	VariantMapModel * modelCharacterList() const { return m_modelCharacterList; }
	GameMap * currentMap() const { return m_currentMap; }
	bool demoMode() const { return m_demoMode; }
	int baseXP() const { return m_baseXP; }
	int selectedGroupId() const { return m_selectedGroupId; }


public slots:
	void groupSelect(const int &groupId);
	void mapDownload(QVariantMap data);
	void mapLoad(QVariantMap data);
	void demoMapLoad();
	void getMissionList();
	void playGame(QVariantMap data);
	void setDemoMode(bool demoMode);
	void setBaseXP(int baseXP);
	void setModelGroupList(VariantMapModel * modelGroupList);
	void setSelectedGroupId(int selectedGroupId);
	void setModelCharacterList(VariantMapModel * modelCharacterList);

protected slots:
	void clientSetup() override;

private slots:
	void onGroupListGet(QJsonObject jsonData, QByteArray);
	void onMapListGet(QJsonObject jsonData, QByteArray);
	void onOneDownloadFinished(const CosDownloaderItem &item, const QByteArray &data, const QJsonObject &);
	void loadGameMap(GameMap *map, const QString &mapName = "");
	void unloadGameMap();

	void onDemoGameWin(const QString &uuid, const int level);
	void onGameEnd(GameMatch *match, const bool &win = false);

	void onMissionListGet(QJsonObject jsonData, QByteArray);
	void onGameCreate(QJsonObject jsonData, QByteArray);
	void onGameFinish(QJsonObject jsonData, QByteArray);

signals:
	void groupListGet(QJsonObject jsonData, QByteArray binaryData);

	void mapListLoaded(QVariantList list);
	void mapListGet(QJsonObject jsonData, QByteArray binaryData);
	void mapDownloadRequest(QString formattedDataSize);

	void missionListGet(QJsonObject jsonData, QByteArray binaryData);
	void missionSelected(const int &index);
	void missionListChanged();

	void gameMapLoaded(const QString &mapName);
	void gameMapUnloaded();

	void gamePlayReady(GameMatch *gameMatch);

	void gameCreate(QJsonObject jsonData, QByteArray binaryData);
	void gameFinish(QJsonObject jsonData, QByteArray binaryData);
	void gameUpdate(QJsonObject jsonData, QByteArray binaryData);

	void modelMapListChanged(VariantMapModel * modelMapList);
	void modelMissionListChanged(VariantMapModel * modelMissionList);
	void demoModeChanged(bool demoMode);
	void baseXPChanged(int baseXP);
	void modelGroupListChanged(VariantMapModel * modelGroupList);
	void selectedGroupIdChanged(int selectedGroupId);
	void modelCharacterListChanged(VariantMapModel * modelCharacterList);

private:
	//QHash<QString, void (StudentMaps::*)(QVariantMap)> m_map;
	VariantMapModel *m_modelMapList;
	GameMap * m_currentMap;
	CosDb *m_imageDb;
	VariantMapModel * m_modelMissionList;
	bool m_demoMode;
	QVariantMap m_demoSolverMap;
	int m_baseXP;
	VariantMapModel * m_modelGroupList;
	int m_selectedGroupId;
	VariantMapModel * m_modelCharacterList;
};

#endif // STUDENTMAPS_H
