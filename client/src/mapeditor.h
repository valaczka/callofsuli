/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
 *
 * Created on: 2023. 05. 06.
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

#include <QObject>
#include <QTimer>
#include "client.h"
#include "mapeditormap.h"
#include "editorundostack.h"
#include "mapplay.h"
#include <QFileInfo>


class MapPlayEditor;

/**
 * @brief The MapEditor class
 */

class MapEditor : public QObject
{
	Q_OBJECT

	Q_PROPERTY(MapEditorMap *map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(EditorUndoStack *undoStack READ undoStack CONSTANT)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
	Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)
	Q_PROPERTY(bool autoSaved READ autoSaved WRITE setAutoSaved NOTIFY autoSavedChanged)
	Q_PROPERTY(QVariantList availableMedals READ availableMedals CONSTANT)

public:
	explicit MapEditor(QObject *parent = nullptr);
	virtual ~MapEditor();

	enum ExportType {
		ExportInvalid,
		ExportExam
	};

	Q_ENUM(ExportType);

	MapEditorMap *map() const;
	void setMap(MapEditorMap *newMap);

	bool loadFromBinaryData(const QByteArray &data);

	Q_INVOKABLE void createFile();
	Q_INVOKABLE void saveAs(const QUrl &file, const bool &createNew = false);
	Q_INVOKABLE void exportData(const MapEditor::ExportType &type, const QUrl &file, const QVariantMap &data = {});

#ifdef Q_OS_WASM
	Q_INVOKABLE void wasmSaveAs(const bool &createNew = false);
	Q_INVOKABLE void wasmUploadImage(QJSValue uploadFunc);
#endif

	Q_INVOKABLE void openFile(const QUrl &file, const bool &fromBackup = false);
	Q_INVOKABLE bool hasBackup(const QUrl &file) const;
	Q_INVOKABLE QUrl currentFolder() const;
	Q_INVOKABLE QString currentFileName() const { return m_currentFileName; }

	Q_INVOKABLE QStringList checkMap() const;

	EditorUndoStack *undoStack() const;

	QString displayName() const;
	void setDisplayName(const QString &newDisplayName);

	Q_INVOKABLE QVariantMap objectiveInfo(MapEditorObjective *objective) const;
	Q_INVOKABLE QVariantMap storageInfo(MapEditorStorage *storage) const;
	Q_INVOKABLE QString objectiveQml(MapEditorObjective *objective) const;
	Q_INVOKABLE QString storageQml(MapEditorStorage *storage) const;
	Q_INVOKABLE QVariantMap inventoryInfo(MapEditorInventory *inventory) const;

	Q_INVOKABLE QVariantList pickableListModel() const;
	Q_INVOKABLE QVariantList objectiveListModel() const;
	Q_INVOKABLE QVariantList storageListModel(const QString &objectiveModule) const;
	Q_INVOKABLE QVariantList storageModel(const QString &storageModule) const;
	Q_INVOKABLE QVariantList storageListAllModel() const;
	Q_INVOKABLE QVariantList terrainListModel() const;

	Q_INVOKABLE MapEditorImage *uploadImage(const QUrl &url);
	Q_INVOKABLE QVariantList uploadImageDirectory(const QUrl &url);

	Q_INVOKABLE MapEditorMission* missionAdd(const QString &name = QString());
	Q_INVOKABLE MapEditorMission* missionAddWithLevels(const QList<QSet<MapEditorChapter*>> &levels, const QString &name = QString());
	Q_INVOKABLE void missionRemove(MapEditorMission *mission);
	Q_INVOKABLE void missionModify(MapEditorMission *mission, QJSValue modifyFunc);
	Q_INVOKABLE void missionLockAdd(MapEditorMission *mission, MapEditorMissionLevel *lock);
	Q_INVOKABLE void missionLockAdd(MapEditorMission *mission, const QString &uuid, const int &level);
	Q_INVOKABLE void missionLockRemove(MapEditorMission *mission, MapEditorMissionLevel *lock);

	Q_INVOKABLE void storageAdd(MapEditorStorage *storage, QJSValue modifyFunc);
	Q_INVOKABLE void storageModify(MapEditorStorage *storage, QJSValue modifyFunc);
	Q_INVOKABLE void storageLoadEditor(const QString &module);
	Q_INVOKABLE void storageRemove(MapEditorStorage *storage);

	Q_INVOKABLE void chapterAdd(const QString &name = QString(), MapEditorMissionLevel *missionLevel = nullptr);
	Q_INVOKABLE void chapterRemove(MapEditorChapter *chapter);
	Q_INVOKABLE void chapterModify(MapEditorChapter *chapter, QJSValue modifyFunc);
	Q_INVOKABLE void chapterImport(const QUrl &url);

	Q_INVOKABLE void objectiveLoadEditor(MapEditorChapter *chapter, const QString &module, const QString &storageModule, MapEditorStorage *storage);
	Q_INVOKABLE void objectiveAdd(MapEditorChapter *chapter, MapEditorObjective *objective, MapEditorStorage *storage, QJSValue modifyFunc);
	Q_INVOKABLE void objectiveModify(MapEditorObjective *objective, MapEditorStorage *storage, QJSValue modifyFunc);
	Q_INVOKABLE void objectiveRemove(MapEditorChapter *chapter, const QList<MapEditorObjective *> &objectiveList);
	Q_INVOKABLE void objectiveDuplicate(MapEditorChapter *chapter, const QList<MapEditorObjective *> &objectiveList);
	Q_INVOKABLE void objectiveCopyOrMove(MapEditorChapter *chapter, const QList<MapEditorObjective *> &objectiveList,
										 const int &toChapterId, const bool &isCopy, const QString &chapterName = QString());
	Q_INVOKABLE QString objectivePreview(const QString &objectiveModule, const QVariantMap &objectiveData,
										 const QString &storageModule, const QVariantMap &storageData) const;

	Q_INVOKABLE MapEditorMissionLevel* missionLevelAdd(MapEditorMission *mission);
	Q_INVOKABLE void missionLevelRemove(MapEditorMissionLevel *missionLevel);
	Q_INVOKABLE void missionLevelModify(MapEditorMissionLevel *missionLevel, QJSValue modifyFunc);
	Q_INVOKABLE void missionLevelChapterAdd(MapEditorMissionLevel *missionLevel, MapEditorChapter *chapter) {
		missionLevelChapterAdd(missionLevel, QList<MapEditorChapter *>{chapter});
	}
	Q_INVOKABLE void missionLevelChapterAdd(MapEditorMissionLevel *missionLevel, const QList<MapEditorChapter *> &chapterList);
	Q_INVOKABLE void missionLevelChapterRemove(MapEditorMissionLevel *missionLevel, const QList<MapEditorChapter *> &chapterList);
	Q_INVOKABLE void missionLevelInventoryAdd(MapEditorMissionLevel *missionLevel, const QString &type);
	Q_INVOKABLE void missionLevelInventoryRemove(MapEditorMissionLevel *missionLevel, const QList<MapEditorInventory *> &inventoryList);
	Q_INVOKABLE void missionLevelInventoryModify(MapEditorMissionLevel *missionLevel, MapEditorInventory *inventory, QJSValue modifyFunc);

	Q_INVOKABLE void missionLevelPlay(MapEditorMissionLevel *missionLevel, int mode);

	bool modified() const;
	void setModified(bool newModified);

	bool autoSaved() const;
	void setAutoSaved(bool newAutoSaved);

	const QVariantList &availableMedals() const;

public slots:
	void save();
	void saveAuto();
	void onSaved(const bool &success);
	void onAutoSaved(const bool &success);

protected:
	void unloadMap();
	void loadMap();
	void setFileDisplayName() {
		QFileInfo fi(m_currentFileName);
		setDisplayName(tr("%1 (%2)").arg(fi.fileName(), fi.path()));
	}

protected slots:
	void onModified();
	void onSaveRequestFile();
	void onAutoSaveRequestFile();

signals:
	void saveRequest();
	void autoSaveRequest();
	void objectiveDialogRequest(MapEditorChapter *chapter);
	void mapChanged();
	void displayNameChanged();
	void modifiedChanged();
	void autoSavedChanged();

protected:
	QString m_currentFileName;
	QString m_currentBackupName;
	Client *const m_client;
	MapEditorMap *m_map = nullptr;

private:
	void loadAvailableMedals();
	QByteArray exportExam(MapEditorMissionLevel *missionLevel) const;
	QVariantMap exportChapterList(const QList<MapEditorChapter*> &list) const;
	bool importChapterList(const QVariantMap &data);
	bool chapterImportData(const QByteArray &content);

	EditorUndoStack *const m_undoStack;
	QTimer m_saveTimer;
	QString m_displayName;
	bool m_modified = false;
	bool m_autoSaved = false;
	MapPlayEditor *m_mapPlay = nullptr;

	MapEditorObjective *m_tmpObjective = nullptr;
	MapEditorStorage *m_tmpStorage = nullptr;

	static const QString m_backupSuffix;

	QVariantList m_availableMedals;
};




/**
 * @brief The MapPlayEditor class
 */


class MapPlayEditor : public MapPlay
{
	Q_OBJECT

public:
	explicit MapPlayEditor(Client *client, QObject *parent = nullptr);
	virtual ~MapPlayEditor();


	bool reloadMap(MapEditorMap *map);
	bool play(MapEditorMissionLevel *missionLevel, const GameMap::GameMode &mode);


protected:
	virtual void onCurrentGamePrepared() override;
	virtual void onCurrentGameFinished() override;
};


#endif // MAPEDITOR_H
