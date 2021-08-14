/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
 *
 * Created on: 2021. 05. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditor
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include "abstractactivity.h"
#include <QObject>
#include "variantmapmodel.h"
#include "variantmapdata.h"

class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)
	Q_PROPERTY(QString readableFilename READ readableFilename NOTIFY filenameChanged)
	Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
	Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)

	Q_PROPERTY(qreal loadProgress READ loadProgress WRITE setLoadProgress NOTIFY loadProgressChanged)
	Q_PROPERTY(QPair<qreal, qreal> loadProgressFraction READ loadProgressFraction WRITE setLoadProgressFraction NOTIFY loadProgressFractionChanged)

	Q_PROPERTY(bool isWithGraphviz READ isWithGraphviz NOTIFY isWithGraphvizChanged)

	// Missions

	Q_PROPERTY(QString currentMission READ currentMission WRITE setCurrentMission NOTIFY currentMissionChanged)
	Q_PROPERTY(VariantMapModel * modelMissionList READ modelMissionList WRITE setModelMissionList NOTIFY modelMissionListChanged)
	Q_PROPERTY(VariantMapModel * modelTerrainList READ modelTerrainList WRITE setModelTerrainList NOTIFY modelTerrainListChanged)
	Q_PROPERTY(VariantMapModel * modelLevelChapterList READ modelLevelChapterList WRITE setModelLevelChapterList NOTIFY modelLevelChapterListChanged)
	Q_PROPERTY(VariantMapModel * modelObjectiveList READ modelObjectiveList WRITE setModelObjectiveList NOTIFY modelObjectiveListChanged)
	Q_PROPERTY(VariantMapModel * modelInventoryList READ modelInventoryList WRITE setModelInventoryList NOTIFY modelInventoryListChanged)
	Q_PROPERTY(VariantMapModel * modelInventoryModules READ modelInventoryModules WRITE setModelInventoryModules NOTIFY modelInventoryModulesChanged)
	Q_PROPERTY(VariantMapModel * modelLockList READ modelLockList WRITE setModelLockList NOTIFY modelLockListChanged)
	Q_PROPERTY(VariantMapModel * modelDialogMissionList READ modelDialogMissionList WRITE setModelDialogMissionList NOTIFY modelDialogMissionListChanged)
	Q_PROPERTY(VariantMapModel * modelDialogChapterList READ modelDialogChapterList WRITE setModelDialogChapterList NOTIFY modelDialogChapterListChanged)


	// Chapters

	Q_PROPERTY(VariantMapModel * modelObjectiveModules READ modelObjectiveModules WRITE setModelObjectiveModules NOTIFY modelObjectiveModulesChanged)
	Q_PROPERTY(VariantMapModel * modelStorageList READ modelStorageList WRITE setModelStorageList NOTIFY modelStorageListChanged)
	Q_PROPERTY(VariantMapModel * modelChapterList READ modelChapterList WRITE setModelChapterList NOTIFY modelChapterListChanged)
	Q_PROPERTY(VariantMapModel * modelDialogChapterMissionList READ modelDialogChapterMissionList WRITE setModelDialogChapterMissionList
			   NOTIFY modelDialogChapterMissionListChanged)


public:
	explicit MapEditor(QQuickItem *parent = nullptr);
	virtual ~MapEditor();

	//Q_INVOKABLE virtual void run(const QString &func, QVariantMap data = QVariantMap()) override { AbstractActivity::run(m_map, func, data); };

	qreal loadProgress() const { return m_loadProgress; }
	QPair<qreal, qreal> loadProgressFraction() const { return m_loadProgressFraction; }

	QString filename() const { return m_filename; }
	QString readableFilename() const;
	bool isWithGraphviz() const;
	bool modified() const { return m_modified; }
	bool loaded() const { return m_loaded; }

	TerrainData defaultTerrain() const;

	Q_INVOKABLE QString objectiveQml(const QString &module);
	Q_INVOKABLE QString storageQml(const QString &module);

	QString currentMission() const { return m_currentMission; }
	VariantMapModel * modelMissionList() const { return m_modelMissionList; }
	VariantMapModel * modelTerrainList() const { return m_modelTerrainList; }
	VariantMapModel * modelLevelChapterList() const { return m_modelLevelChapterList; }
	VariantMapModel * modelObjectiveList() const { return m_modelObjectiveList; }
	VariantMapModel * modelInventoryList() const { return m_modelInventoryList; }
	VariantMapModel * modelInventoryModules() const { return m_modelInventoryModules; }
	VariantMapModel * modelLockList() const { return m_modelLockList; }
	VariantMapModel * modelDialogMissionList() const { return m_modelDialogMissionList; }
	VariantMapModel * modelDialogChapterList() const { return m_modelDialogChapterList; }
	VariantMapModel * modelDialogChapterMissionList() const { return m_modelDialogChapterMissionList; }
	VariantMapModel * modelChapterList() const { return m_modelChapterList; }
	VariantMapModel * modelObjectiveModules() const { return m_modelObjectiveModules; }
	VariantMapModel * modelStorageList() const { return m_modelStorageList; }

public slots:
	void createTargets(const QString &filename);
	void create(const QString &filename = "");
	void save(const QString &filename = "");
	void saveCopy(const QString &filename);
	void openUrl(const QUrl &url) { createTargets(url.toLocalFile()); }
	void saveUrl(const QUrl &url) { save(url.toLocalFile()); }
	void saveCopyUrl(const QUrl &url) { saveCopy(url.toLocalFile()); }
	void loadAbort();
	bool setLoadProgress(qreal loadProgress);
	void setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction);
	void setFilename(QString filename);
	void setModified(bool modified);
	void setLoaded(bool loaded);

	void checkPermissions();

	void setCurrentMission(QString currentMission);
	void setModelMissionList(VariantMapModel * modelMissionList);
	void setModelTerrainList(VariantMapModel * modelTerrainList);
	void setModelLevelChapterList(VariantMapModel * modelLevelChapterList);
	void setModelObjectiveList(VariantMapModel * modelObjectiveList);
	void setModelInventoryList(VariantMapModel * modelInventoryList);
	void setModelInventoryModules(VariantMapModel * modelInventoryModules);
	void setModelLockList(VariantMapModel * modelLockList);
	void setModelDialogMissionList(VariantMapModel * modelDialogMissionList);
	void setModelDialogChapterList(VariantMapModel * modelDialogChapterList);
	void setModelDialogChapterMissionList(VariantMapModel * modelDialogChapterMissionList);
	void setModelChapterList(VariantMapModel * modelChapterList);
	void setModelObjectiveModules(VariantMapModel * modelObjectiveModules);
	void setModelStorageList(VariantMapModel * modelStorageModules);

	void getMissionList();
	void getCurrentMissionData();
	void getFirstMission();
	void getObjectiveList();
	void getStorageList();
	void getChapterList();

	void play(QVariantMap data);

	void missionAdd(QVariantMap data);
	void missionModify(QVariantMap data);
	void missionRemove();

	void missionLevelAdd(QVariantMap data);
	void missionLevelRemove(QVariantMap data);
	void missionLevelModify(QVariantMap data);
	void missionLevelGetChapterList(int level);
	void missionLevelChapterAdd(QVariantMap data);
	void missionLevelChapterRemove(QVariantMap data);

	void missionLockGetList(QVariantMap data);
	void missionLockAdd(QVariantMap data);
	void missionLockModify(QVariantMap data);
	void missionLockRemove(QVariantMap data);
	void missionLockGraphUpdate();

	void inventoryAdd(QVariantMap data);
	void inventoryModify(QVariantMap data);
	void inventoryRemove(QVariantMap data);

	void chapterAdd(QVariantMap data);
	void chapterModify(QVariantMap data);
	void chapterRemove(QVariantMap data);
	void chapterGetMissionList(QVariantMap data);
	void chapterMissionListModify(const int &chapter, VariantMapModel *model, const QString &selectField = "used");
	void chapterImport(QVariantMap data);

	void objectiveAdd(QVariantMap data);
	void objectiveModify(QVariantMap data);
	void objectiveRemove(QVariantMap data);
	void objectiveCopy(QVariantMap data);
	void objectiveImport(QVariantMap data);
	void objectiveAddOrModify(QVariantMap data);



signals:
	void loadStarted(const QString &filename);
	void loadFailed();
	void loadSucceed();
	void loadProgressChanged(qreal loadProgress);
	void loadProgressFractionChanged(QPair<qreal, qreal> loadProgressFraction);

	void saveFailed();
	void saveSucceed(const QString &filename, const bool &isCopy);
	void saveDialogRequest(const bool &isNew);

	void storagePermissionsGranted();
	void storagePermissionsDenied();


	void filenameChanged(QString filename);
	void isWithGraphvizChanged(bool isWithGraphviz);
	void modifiedChanged(bool modified);
	void loadedChanged(bool loaded);

	void playFailed(QString error);
	void playReady(GameMatch *gamematch);

	void currentMissionDataChanged(QVariantMap data);

	void missionLockListReady(QVariantMap data);
	void missionChapterListReady(QVariantMap data);
	void chapterMissionListReady(QVariantMap data);
	void chapterImportReady(QVariantList records);

	void currentMissionChanged(QString currentMission);
	void modelMissionListChanged(VariantMapModel * modelMissionList);
	void modelTerrainListChanged(VariantMapModel * modelTerrainList);
	void modelLevelChapterListChanged(VariantMapModel * modelLevelChapterList);
	void modelObjectiveListChanged(VariantMapModel * modelObjectiveList);
	void modelInventoryListChanged(VariantMapModel * modelInventoryList);
	void modelInventoryModulesChanged(VariantMapModel * modelInventoryModules);
	void modelLockListChanged(VariantMapModel * modelLockList);
	void modelDialogMissionListChanged(VariantMapModel * modelDialogMissionList);
	void modelDialogChapterListChanged(VariantMapModel * modelDialogChapterList);
	void modelDialogChapterMissionListChanged(VariantMapModel * modelDialogChapterMissionList);
	void modelChapterListChanged(VariantMapModel * modelChapterList);
	void modelObjectiveModulesChanged(VariantMapModel * modelObjectiveModules);
	void modelStorageListChanged(VariantMapModel * modelStorageModules);

protected:
	void openPrivate(QVariantMap data);
	void createPrivate(QVariantMap data);
	void savePrivate(QVariantMap data);
	bool loadDatabasePrivate(GameMap *game, const QString &filename = "");
	bool createTriggersPrivate();

protected slots:
	void clientSetup() override;

private:
	QVariantMap missionLevelDefaults(const int &level);
	int missionLevelAddPrivate(const QString &mission, const int &level);
	int missionLevelChapterAddPrivate(const QString &mission, const int &level, const QVariantList &list);

	mutable QVariantMap m_gameData;
	qreal m_loadProgress;
	QPair<qreal, qreal> m_loadProgressFraction;
	bool m_loadAbortRequest;
	QString m_filename;
	bool m_modified;
	bool m_loaded;
	QString m_currentMission;
	VariantMapModel * m_modelMissionList;
	VariantMapModel * m_modelTerrainList;
	VariantMapModel * m_modelLevelChapterList;
	VariantMapModel * m_modelObjectiveList;
	VariantMapModel * m_modelInventoryList;
	VariantMapModel * m_modelInventoryModules;
	VariantMapModel * m_modelLockList;
	VariantMapModel * m_modelDialogMissionList;
	VariantMapModel * m_modelDialogChapterList;
	VariantMapModel * m_modelDialogChapterMissionList;
	VariantMapModel * m_modelChapterList;
	VariantMapModel * m_modelObjectiveModules;
	VariantMapModel * m_modelStorageList;
};

#endif // MAPEDITOR_H
