/*
 * ---- Call of Suli ----
 *
 * mapeditoraction.h
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditorAction
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

#ifndef MAPEDITORACTION_H
#define MAPEDITORACTION_H

#include <QObject>
#include <QList>
#include <QPointer>
#include "editorundostack.h"
#include "editoraction.h"
#include "gamemapeditor.h"
#include "gamemapreaderiface.h"

class MapEditorAction : public EditorAction
{
	Q_OBJECT

	Q_PROPERTY(MapEditorActionType type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(QVariant contextId READ contextId WRITE setContextId NOTIFY contextIdChanged)

public:
	enum MapEditorActionType {
		ActionTypeInvalid		= 0,
		ActionTypeChapterList	= 0x01,
		ActionTypeChapter		= 0x02,
		ActionTypeMissionList	= 0x04,
		ActionTypeMission		= 0x08,
		ActionTypeMissionLevel	= 0x10,
		ActionTypeInventory		= 0x20,
		ActionTypeImageList		= 0x40,
		ActionTypeImage			= 0x80,
		ActionTypeObjective		= 0x100,
		ActionTypeStorage		= 0x200,
		ActionTypeStorageList	= 0x400
	};

	Q_ENUM(MapEditorActionType);
	Q_DECLARE_FLAGS(MapEditorActionTypes, MapEditorActionType)
	Q_FLAG(MapEditorActionTypes)

	explicit MapEditorAction(GameMapEditor *editor, const MapEditorActionType &type, const QVariant &contextId = QVariant::Invalid);

	const MapEditorActionType &type() const;
	void setType(const MapEditorActionType &newType);

	const QVariant &contextId() const;
	void setContextId(const QVariant &newContextId);

signals:
	void typeChanged();
	void contextIdChanged();

protected:
	QVariantMap variantMapSave(QObject *source, const QStringList &keys) const;
	QVariantMap variantMapSave(QObject *source, const QVariantMap &map) const;
	void variantMapSet(QObject *target, const QVariantMap &map) const;

	void chapterAdd(GameMapEditorChapter *chapter);
	void chapterRemove(GameMapEditorChapter *chapter);

	void objectiveAdd(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);
	void objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);
	void objectiveRecalculate(GameMapEditorObjective *objective);

	void storageAdd(GameMapEditorStorage *storage);
	void storageRemove(GameMapEditorStorage *storage);

	void missionAdd(GameMapEditorMission *mission);
	void missionRemove(GameMapEditorMission *mission);

	void missionLevelAdd(GameMapEditorMission *mission, GameMapEditorMissionLevel *level);
	void missionLevelRemove(GameMapEditorMission *mission, GameMapEditorMissionLevel *level);

	void missionLevelAddChapter(GameMapEditorMissionLevel *level, GameMapEditorChapter *chapter);
	void missionLevelRemoveChapter(GameMapEditorMissionLevel *level, GameMapEditorChapter *chapter);

	void missionLockAdd(GameMapEditorMission *mission, GameMapEditorMissionLevel *level);
	void missionLockRemove(GameMapEditorMission *mission, GameMapEditorMissionLevel *level);

	void inventoryAdd(GameMapEditorMissionLevel *level, GameMapEditorInventory *inventory);
	void inventoryRemove(GameMapEditorMissionLevel *level, GameMapEditorInventory *inventory);
	void addDefaultInventories(GameMapEditorMissionLevel *level, const QVariantList &list);

	MapEditorActionType m_type;
	QVariant m_contextId;
	GameMapEditor *m_editor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MapEditorAction::MapEditorActionTypes)



/**
 * @brief The MapEditorActionObjectiveNew class
 */


class MapEditorActionObjectiveNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionObjectiveNew(GameMapEditor *editor, GameMapEditorChapter *parentChapter,
										 const QVariantMap &data, const QVariantMap &storageData = QVariantMap());
	virtual ~MapEditorActionObjectiveNew();

private:
	GameMapEditorObjective *m_objective;
	GameMapEditorChapter *m_parentChapter;
	GameMapEditorStorage *m_storage;

	GameMapEditorStorage *m_storageEdited;
	QVariantMap m_storageDataSource;
	QVariantMap m_storageDataTarget;

};



/**
 * @brief The MapEditorActionObjectiveRemove class
 */

class MapEditorActionObjectiveRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionObjectiveRemove(GameMapEditor *editor, GameMapEditorChapter *parentChapter, GameMapEditorObjective *objective);
	explicit MapEditorActionObjectiveRemove(GameMapEditor *editor, GameMapEditorChapter *parentChapter, const QList<GameMapEditorObjective *> &list);
	virtual ~MapEditorActionObjectiveRemove();

private:
	void _undo();
	void _redo();

	GameMapEditorChapter *m_parentChapter;
	QList<QPointer<GameMapEditorObjective>> m_list;
};



/**
 * @brief The MapEditorActionObjectiveModify class
 */

class MapEditorActionObjectiveModify : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionObjectiveModify(GameMapEditor *editor, GameMapEditorChapter *chapter,
											GameMapEditorObjective *objective, const QVariantMap &data,
											GameMapEditorStorage *storage = nullptr, const QVariantMap &storageData = QVariantMap());
	virtual ~MapEditorActionObjectiveModify();

private:
	GameMapEditorChapter *m_parentChapter;
	GameMapEditorObjective *m_objective;
	QVariantMap m_dataSource;
	QVariantMap m_dataTarget;

	GameMapEditorStorage *m_storage;
	QVariantMap m_storageDataSource;
	QVariantMap m_storageDataTarget;
};




/**
 * @brief The MapEditorActionObjectiveMove class
 */

class MapEditorActionObjectiveMove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionObjectiveMove(GameMapEditor *editor, GameMapEditorChapter *parentChapter, GameMapEditorObjective *objective,
										  const bool &isCopy, const QVariantMap &targetChapterData);
	explicit MapEditorActionObjectiveMove(GameMapEditor *editor, GameMapEditorChapter *parentChapter, const QList<GameMapEditorObjective *> &list,
										  const bool &isCopy, const QVariantMap &targetChapterData);
	virtual ~MapEditorActionObjectiveMove();

private:
	void _undo();
	void _redo();
	void _setTarget(const QVariantMap &targetChapterData);

	GameMapEditorChapter *m_parentChapter;
	QList<QPointer<GameMapEditorObjective>> m_list;
	bool m_isCopy;

	GameMapEditorChapter *m_targetChapter;
	bool m_isNewChapter;
};







/**
 * @brief The MapEditorActionChapterNew class
 */


class MapEditorActionChapterNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionChapterNew(GameMapEditor *editor, const QVariantMap &data, GameMapEditorMissionLevel *missionLevel = nullptr);
	virtual ~MapEditorActionChapterNew();

private:
	GameMapEditorChapter *m_chapter;
	GameMapEditorMissionLevel *m_missionLevel;

};


/**
 * @brief The MapEditorActionChapterRemove class
 */

class MapEditorActionChapterRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionChapterRemove(GameMapEditor *editor, GameMapEditorChapter *chapter);
	explicit MapEditorActionChapterRemove(GameMapEditor *editor, const QList<GameMapEditorChapter *> &list);
	virtual ~MapEditorActionChapterRemove();

private:
	void addChapter(GameMapEditorChapter *chapter);
	void _undo();
	void _redo();

	struct ChapterList {
		ChapterList(GameMapEditorChapter *_chapter) :
			chapter(_chapter), levels()
		{}

		ChapterList(GameMapEditorChapter *_chapter, QList<QPointer<GameMapEditorMissionLevel>> _levels) :
			chapter(_chapter), levels(_levels)
		{}

		void append(GameMapEditorMissionLevel *level) {
			levels.append(level);
		}

		GameMapEditorChapter *chapter;
		QList<QPointer<GameMapEditorMissionLevel>> levels;
	};

	QList<ChapterList> m_list;

};



/**
 * @brief The MapEditorActionChapterModify class
 */

class MapEditorActionChapterModify : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionChapterModify(GameMapEditor *editor, GameMapEditorChapter *chapter, const QVariantMap &data);
	virtual ~MapEditorActionChapterModify();

private:
	GameMapEditorChapter *m_chapter;
	QVariantMap m_dataSource;
	QVariantMap m_dataTarget;
};






/**
 * @brief The MapEditorActionChapterModify class
 */

class MapEditorActionChapterMissionLevels : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionChapterMissionLevels(GameMapEditor *editor, GameMapEditorChapter *chapter,
												 const QList<GameMapEditorMissionLevel*> &levels);
	virtual ~MapEditorActionChapterMissionLevels();

private:
	void updateMissionLevels(const QList<QPointer<GameMapEditorMissionLevel>> &list);

	GameMapEditorChapter *m_chapter;
	QList<QPointer<GameMapEditorMissionLevel>> m_listSource;
	QList<QPointer<GameMapEditorMissionLevel>> m_listTarget;
};



/**
 * @brief The MapEditorActionMissionNew class
 */

class MapEditorActionMissionNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionNew(GameMapEditor *editor, const QVariantMap &data, const QString &terrain = "");
	virtual ~MapEditorActionMissionNew();

	GameMapEditorMission *mission() const;
	GameMapEditorMissionLevel *missionLevel() const;


private:
	GameMapEditorMission *m_mission;
	GameMapEditorMissionLevel *m_missionLevel;
};




/**
 * @brief The MapEditorActionMissonRemove class
 */

class MapEditorActionMissionRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionRemove(GameMapEditor *editor, GameMapEditorMission *mission);
	explicit MapEditorActionMissionRemove(GameMapEditor *editor, const QList<GameMapEditorMission *> &list);
	virtual ~MapEditorActionMissionRemove();

private:
	void addMission(GameMapEditorMission *mission);
	void _undo();
	void _redo();

	struct MissionList {
		MissionList(GameMapEditorMission *_mission) :
			mission(_mission), locks()
		{}

		MissionList(GameMapEditorMission *_mission, QList<QPair<QPointer<GameMapEditorMission>, QPointer<GameMapEditorMissionLevel>>> _locks) :
			mission(_mission), locks(_locks)
		{}

		void append(GameMapEditorMission *mission, GameMapEditorMissionLevel *lock) {
			locks.append(qMakePair<GameMapEditorMission *, GameMapEditorMissionLevel*>(mission, lock));
		}

		GameMapEditorMission *mission;
		QList<QPair<QPointer<GameMapEditorMission>, QPointer<GameMapEditorMissionLevel>>> locks;
	};

	QList<MissionList> m_list;

};





/**
 * @brief The MapEditorActionMissionModify class
 */

class MapEditorActionMissionModify : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionModify(GameMapEditor *editor, GameMapEditorMission *mission, const QVariantMap &data);
	virtual ~MapEditorActionMissionModify();

private:
	GameMapEditorMission *m_mission;
	QVariantMap m_dataSource;
	QVariantMap m_dataTarget;
};











/**
 * @brief The MapEditorActionMissionLockNew class
 */

class MapEditorActionMissionLockNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLockNew(GameMapEditor *editor, GameMapEditorMission *parentMission, GameMapEditorMissionLevel *level);
	virtual ~MapEditorActionMissionLockNew();

private:
	GameMapEditorMission *m_mission;
	GameMapEditorMissionLevel *m_missionLevel;
};




/**
 * @brief The MapEditorActionMissionLockRemove class
 */

class MapEditorActionMissionLockRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLockRemove(GameMapEditor *editor, GameMapEditorMission *parentMission, GameMapEditorMissionLevel *level);
	virtual ~MapEditorActionMissionLockRemove();

private:
	GameMapEditorMission *m_mission;
	GameMapEditorMissionLevel *m_missionLevel;
};







/**
 * @brief The MapEditorActionMissionLockReplace class
 */

class MapEditorActionMissionLockReplace : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLockReplace(GameMapEditor *editor, GameMapEditorMission *parentMission,
											   GameMapEditorMissionLevel *oldLevel,
											   GameMapEditorMissionLevel *newLevel);
	virtual ~MapEditorActionMissionLockReplace();

private:
	GameMapEditorMission *m_mission;
	GameMapEditorMissionLevel *m_missionLevelOld;
	GameMapEditorMissionLevel *m_missionLevelNew;
};






/**
 * @brief The MapEditorActionMissionLevelNew class
 */

class MapEditorActionMissionLevelNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLevelNew(GameMapEditor *editor, GameMapEditorMission *parentMission, const QVariantMap &data);
	virtual ~MapEditorActionMissionLevelNew();

	GameMapEditorMissionLevel *missionLevel() const;

private:
	GameMapEditorMission *m_mission;
	GameMapEditorMissionLevel *m_missionLevel;
};




/**
 * @brief The MapEditorActionMissionLevelRemove class
 */

class MapEditorActionMissionLevelRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLevelRemove(GameMapEditor *editor, GameMapEditorMission *parentMission, GameMapEditorMissionLevel *missionLevel);
	virtual ~MapEditorActionMissionLevelRemove();

private:
	GameMapEditorMission *m_mission;
	GameMapEditorMissionLevel *m_missionLevel;
	QList<QPointer<GameMapEditorMission>> m_locks;
};





/**
 * @brief The MapEditorActionMissionLevelModify class
 */

class MapEditorActionMissionLevelModify : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLevelModify(GameMapEditor *editor, GameMapEditorMissionLevel *missionLevel, const QVariantMap &data);
	virtual ~MapEditorActionMissionLevelModify();

private:
	GameMapEditorMissionLevel *m_missionLevel;
	QVariantMap m_dataSource;
	QVariantMap m_dataTarget;
};




/**
 * @brief The MapEditorActionMissionLevelChapters class
 */

class MapEditorActionMissionLevelChapters : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionMissionLevelChapters(GameMapEditor *editor, GameMapEditorMissionLevel *missionLevel,
												 const QList<GameMapEditorChapter*> &chapters);
	virtual ~MapEditorActionMissionLevelChapters();

private:
	void updateChapters(const QList<QPointer<GameMapEditorChapter>> &list);

	GameMapEditorMissionLevel *m_missionLevel;
	QList<QPointer<GameMapEditorChapter>> m_listSource;
	QList<QPointer<GameMapEditorChapter>> m_listTarget;
	QList<QPointer<GameMapEditorChapter>> m_listUpdated;
};






/**
 * @brief The MapEditorActionInventoryNew class
 */

class MapEditorActionInventoryNew : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionInventoryNew(GameMapEditor *editor, GameMapEditorMissionLevel *parentMissionLevel, const QVariantMap &data);
	virtual ~MapEditorActionInventoryNew();

private:
	GameMapEditorMissionLevel *m_missionLevel;
	GameMapEditorInventory *m_inventory;
};



/**
 * @brief The MapEditorActionInventoryRemove class
 */

class MapEditorActionInventoryRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionInventoryRemove(GameMapEditor *editor, GameMapEditorMissionLevel *parentMissionLevel,
											GameMapEditorInventory *inventory);
	virtual ~MapEditorActionInventoryRemove();

private:
	GameMapEditorMissionLevel *m_missionLevel;
	GameMapEditorInventory *m_inventory;
};




/**
 * @brief The MapEditorActionInventoryModify class
 */

class MapEditorActionInventoryModify : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionInventoryModify(GameMapEditor *editor, GameMapEditorInventory *inventory, const QVariantMap &data);
	virtual ~MapEditorActionInventoryModify();

private:
	GameMapEditorInventory *m_inventory;
	QVariantMap m_dataSource;
	QVariantMap m_dataTarget;
};





/**
 * @brief The MapEditorActionStorageRemove class
 */

class MapEditorActionStorageRemove : public MapEditorAction
{
	Q_OBJECT

public:
	explicit MapEditorActionStorageRemove(GameMapEditor *editor, GameMapEditorStorage *storage);
	virtual ~MapEditorActionStorageRemove();

private:
	GameMapEditorStorage *m_storage;
};




#endif // MAPEDITORACTION_H




