/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
 *
 * Created on: 2022. 01. 16.
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
#include "abstractactivity.h"
#include "gamemapeditor.h"
#include "mapeditoraction.h"
#include "gamematch.h"
#include "editorundostack.h"


/**
 * @brief The MapEditorMissionLevelObject class
 */

class MapEditorMissionLevelObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(QString uuid MEMBER m_uuid)
	Q_PROPERTY(QString name MEMBER m_name)
	Q_PROPERTY(int level MEMBER m_level)
	Q_PROPERTY(GameMapEditorMissionLevel *missionLevel MEMBER m_missionLevel)

public:
	explicit MapEditorMissionLevelObject(QObject *parent = nullptr);
	explicit MapEditorMissionLevelObject(const QString &name, QObject *parent = nullptr);
	explicit MapEditorMissionLevelObject(const QString &uuid, const QString &name, QObject *parent = nullptr);
	explicit MapEditorMissionLevelObject(const QString &uuid, const QString &name, GameMapEditorMissionLevel *missionLevel, QObject *parent = nullptr);
	virtual ~MapEditorMissionLevelObject();

	const QString &uuid() const { return m_uuid; }
	const int &level() const { return m_level; }

	GameMapEditorMissionLevel *missionLevel() const;

private:
	QString m_uuid;
	QString m_name;
	int m_level;
	GameMapEditorMissionLevel *m_missionLevel;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<MapEditorMissionLevelObject>*);



/**
 * @brief The MapEditor class
 */

class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(GameMapEditor* editor READ editor WRITE setEditor NOTIFY editorChanged)
	Q_PROPERTY(EditorUndoStack *undoStack READ undoStack WRITE setUndoStack NOTIFY undoStackChanged)
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)

	Q_PROPERTY(QVariantList availableObjectives READ availableObjectives CONSTANT)
	Q_PROPERTY(QVariantList availableTerrains READ availableTerrains CONSTANT)
	Q_PROPERTY(QVariantList availableInventories READ availableInventories CONSTANT)
	Q_PROPERTY(ObjectGenericListModel<MapEditorMissionLevelObject>* missionLevelModel READ missionLevelModel CONSTANT)

public:
	explicit MapEditor(QQuickItem *parent = nullptr);
	virtual ~MapEditor();

	GameMapEditor *editor() const;
	void setEditor(GameMapEditor *newEditor);

	EditorUndoStack *undoStack() const;
	void setUndoStack(EditorUndoStack *newUndoStack);

	const QUrl &url() const;
	void setUrl(const QUrl &newUrl);

	const QString &displayName() const;
	void setDisplayName(const QString &newDisplayName);

	const QVariantList &availableObjectives() const;
	const QVariantList &availableTerrains() const;
	const QVariantList &availableInventories() const;

	Q_INVOKABLE QVariantList getStorages() const;
	Q_INVOKABLE QString objectiveQml(const QString &module) const;
	Q_INVOKABLE QString storageQml(const QString &module) const;

	Q_INVOKABLE QVariantMap inventoryInfo(const QString &module) const;
	Q_INVOKABLE QVariantMap storageInfo(GameMapEditorStorage *storage) const;

	ObjectGenericListModel<MapEditorMissionLevelObject> *missionLevelModel() const;
	Q_INVOKABLE void updateMissionLevelModelChapter(GameMapEditorChapter *chapter);
	Q_INVOKABLE void updateMissionLevelModelMission(GameMapEditorMission *mission);
	Q_INVOKABLE void updateMissionLevelModelLock(GameMapEditorMissionLevel *lock);

	Q_INVOKABLE void updateChapterModelMissionLevel(GameMapEditorMissionLevel *missionLevel);

	Q_INVOKABLE QString checkMap() const;


public slots:
	void open(const QUrl &url);
	void create();
	void close();
	void save(const QUrl &newUrl = QUrl());

	void chapterAdd(QVariantMap data, GameMapEditorMissionLevel *missionLevel = nullptr);
	void chapterRemove(GameMapEditorChapter *chapter);
	void chapterRemoveList(const QList<GameMapEditorChapter*> &list);
	void chapterModify(GameMapEditorChapter *chapter, const QVariantMap &data);
	bool chapterModelUnselectObjectives(ObjectGenericListModel<GameMapEditorChapter> *model);
	void chapterModifyMissionLevels(GameMapEditorChapter *chapter, const QList<MapEditorMissionLevelObject*> &list);

	void objectiveAdd(GameMapEditorChapter *chapter, const QVariantMap &data, const QVariantMap &storageData = QVariantMap());
	void objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);
	void objectiveRemoveList(GameMapEditorChapter *chapter, const QList<GameMapEditorObjective*> &list);
	void objectiveModify(GameMapEditorChapter *chapter, GameMapEditorObjective *objective,
						 const QVariantMap &data, const QVariantMap &storageData = QVariantMap());
	void objectiveMoveCopy(GameMapEditorChapter *chapter, const bool &isCopy, GameMapEditorObjective *objective,
						   const int &targetChapterId, const QString &newChapterName = "");
	void objectiveMoveCopyList(GameMapEditorChapter *chapter, const bool &isCopy, const QList<GameMapEditorObjective*> &list,
							   const int &targetChapterId, const QString &newChapterName = "");

	QVariantMap objectiveGeneratePreview(const QString &objectiveModule,
										 const QVariantMap &objectiveData,
										 const QString &storageModule,
										 const QVariantMap &storageData);

	void missionAdd(const QVariantMap &data, const QString &terrain = "");
	void missionRemove(GameMapEditorMission *mission);
	void missionRemoveList(const QList<GameMapEditorMission*> &list);
	void missionModify(GameMapEditorMission *mission, const QVariantMap &data);

	void missionLockAdd(GameMapEditorMission *mission, GameMapEditorMissionLevel *level);
	void missionLockRemove(GameMapEditorMission *mission, GameMapEditorMissionLevel *level);
	void missionLockReplace(GameMapEditorMission *mission, GameMapEditorMissionLevel *levelOld, GameMapEditorMissionLevel *levelNew);

	void missionLevelAdd(GameMapEditorMission *mission, QVariantMap data);
	void missionLevelRemove(GameMapEditorMissionLevel *missionLevel);
	void missionLevelModify(GameMapEditorMissionLevel *missionLevel, const QVariantMap &data);
	void missionLevelPlay(GameMapEditorMissionLevel *missionLevel, const GameMatch::GameMode &mode = GameMatch::ModeNormal);
	void missionLevelRemoveChapter(GameMapEditorMissionLevel *missionLevel, GameMapEditorChapter* chapter);
	void missionLevelRemoveChapterList(GameMapEditorMissionLevel *missionLevel, const QList<GameMapEditorChapter*> &chapterList);
	void missionLevelModifyChapters(GameMapEditorMissionLevel *missionLevel, const QList<GameMapEditorChapter*> &list);

	void inventoryAdd(GameMapEditorMissionLevel *missionLevel, const QVariantMap &data);
	void inventoryRemove(GameMapEditorMissionLevel *missionLevel, GameMapEditorInventory *inventory);
	void inventoryModify(GameMapEditorInventory *inventory, const QVariantMap &data);

	void storageRemove(GameMapEditorStorage *storage);

	int imageAdd(const QUrl &url);


private slots:
	void onUndoRedoCompleted(const int &lastStep);

signals:
	void actionContextUpdated(const MapEditorAction::MapEditorActionType &type, const QVariant &contextId);
	void editorChanged();
	void undoStackChanged();
	void urlChanged();
	void displayNameChanged();

	void missionOpenRequest(GameMapEditorMission *mission);
	void missionLevelOpenRequest(GameMapEditorMissionLevel *missionLevel);
	void missionLevelRemoved(GameMapEditorMissionLevel *missionLevel);

	void gamePlayReady(GameMatch *gameMatch);

private:
	QList<GameMapEditorMissionLevel*> toMissionLevelList(const QList<MapEditorMissionLevelObject*> &list);

	GameMapEditor *m_editor;
	EditorUndoStack *m_undoStack;
	QUrl m_url;
	QString m_displayName;
	QVariantList m_availableObjectives;
	QVariantList m_availableTerrains;
	QVariantList m_availableInventories;
	ObjectGenericListModel<MapEditorMissionLevelObject> *m_missionLevelModel;
};




#endif // MAPEDITOR_H
