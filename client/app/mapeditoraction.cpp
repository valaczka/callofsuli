/*
 * ---- Call of Suli ----
 *
 * mapeditoraction.cpp
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

#include "mapeditoraction.h"
#include "question.h"
#include "cosclient.h"


MapEditorAction::MapEditorAction(GameMapEditor *editor, const MapEditorActionType &type, const QVariant &contextId)
	: EditorAction(editor)
	, m_type(type)
	, m_contextId(contextId)
	, m_editor(editor)
{
	Q_ASSERT(editor);
}



const QVariant &MapEditorAction::contextId() const
{
	return m_contextId;
}

void MapEditorAction::setContextId(const QVariant &newContextId)
{
	if (m_contextId == newContextId)
		return;
	m_contextId = newContextId;
	emit contextIdChanged();
}


void MapEditorAction::setType(const MapEditorActionType &newType)
{
	if (m_type == newType)
		return;
	m_type = newType;
	emit typeChanged();
}

const MapEditorAction::MapEditorActionType &MapEditorAction::type() const
{
	return m_type;
}



/**
 * @brief MapEditorAction::variantMapSave
 * @param source
 * @param keys
 * @return
 */


QVariantMap MapEditorAction::variantMapSave(QObject *source, const QStringList &keys) const
{
	if (!source)
		return QVariantMap();

	QVariantMap ret;

	const QMetaObject *metaobject = source->metaObject();

	int count = metaobject->propertyCount();
	for (int i=0; i<count; ++i) {
		QMetaProperty metaproperty = metaobject->property(i);
		QString name = QString::fromLatin1(metaproperty.name());
		if (keys.contains(name) && metaproperty.isReadable()) {
			ret.insert(name, metaproperty.read(source));
		}
	}

	return ret;
}

/**
 * @brief MapEditorAction::variantMapSave
 * @param source
 * @param map
 * @return
 */

QVariantMap MapEditorAction::variantMapSave(QObject *source, const QVariantMap &map) const
{
	return variantMapSave(source, map.keys());
}


/**
 * @brief MapEditorAction::variantMapSet
 * @param target
 * @param map
 */

void MapEditorAction::variantMapSet(QObject *target, const QVariantMap &map) const
{
	if (!target)
		return;

	const QMetaObject *metaobject = target->metaObject();

	int count = metaobject->propertyCount();
	for (int i=0; i<count; ++i) {
		QMetaProperty metaproperty = metaobject->property(i);
		QString name = QString::fromLatin1(metaproperty.name());
		if (map.contains(name) && metaproperty.isWritable())
			metaproperty.write(target, map.value(name));
	}
}
















/**
 * @brief MapEditorAction::chapterAdd
 * @param chapter
 */

void MapEditorAction::chapterAdd(GameMapEditorChapter *chapter)
{
	m_editor->m_chapters->addObject(chapter);
	chapter->setParent(m_editor);
}


/**
 * @brief GameMapEditor::chapterRemove
 * @param chapter
 */

void MapEditorAction::chapterRemove(GameMapEditorChapter *chapter)
{
	int index = m_editor->m_chapters->index(chapter);
	Q_ASSERT(index != -1);
	m_editor->m_chapters->unselectAll();
	chapter->setParent(this);
	m_editor->m_chapters->removeObject(index);
}


/**
 * @brief MapEditorAction::objectiveAdd
 * @param objective
 */

void MapEditorAction::objectiveAdd(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	chapter->m_objectives->addObject(objective);
	objective->setParent(chapter);
	chapter->recalculateCounts();
}


/**
 * @brief GameMapEditorChapter::objectiveRemove
 * @param objective
 */

void MapEditorAction::objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	int index = chapter->m_objectives->index(objective);
	Q_ASSERT(index != -1);
	chapter->m_objectives->unselectAll();
	objective->setParent(this);
	chapter->m_objectives->removeObject(index);
	chapter->recalculateCounts();
}


/**
 * @brief MapEditorAction::storageAdd
 * @param storage
 */

void MapEditorAction::storageAdd(GameMapEditorStorage *storage)
{
	m_editor->m_storages->addObject(storage);
	storage->setParent(m_editor);
}


/**
 * @brief MapEditorAction::storageRemove
 * @param storage
 */

void MapEditorAction::storageRemove(GameMapEditorStorage *storage)
{
	int index = m_editor->m_storages->index(storage);
	Q_ASSERT(index != -1);
	m_editor->m_storages->unselectAll();
	storage->setParent(this);
	m_editor->m_storages->removeObject(index);
}


/**
 * @brief MapEditorAction::missionAdd
 * @param mission
 */

void MapEditorAction::missionAdd(GameMapEditorMission *mission)
{
	m_editor->m_missions->addObject(mission);
	mission->setParent(m_editor);
}


/**
 * @brief MapEditorAction::missionRemove
 * @param mission
 */

void MapEditorAction::missionRemove(GameMapEditorMission *mission)
{
	int index = m_editor->m_missions->index(mission);
	Q_ASSERT(index != -1);
	m_editor->m_missions->unselectAll();
	mission->setParent(this);
	m_editor->m_missions->removeObject(index);
}


/**
 * @brief MapEditorAction::missionLevelAdd
 * @param mission
 * @param level
 */

void MapEditorAction::missionLevelAdd(GameMapEditorMission *mission, GameMapEditorMissionLevel *level)
{
	mission->levels()->addObject(level);
	level->setParent(mission);
}


/**
 * @brief MapEditorAction::missionLevelRemove
 * @param mission
 * @param level
 */

void MapEditorAction::missionLevelRemove(GameMapEditorMission *mission, GameMapEditorMissionLevel *level)
{
	int index = mission->levels()->index(level);
	Q_ASSERT(index != -1);
	mission->levels()->unselectAll();
	level->setParent(this);
	mission->levels()->removeObject(index);
}













/**
 * @brief MapEditorActionChapterNew::MapEditorActionChapterNew
 * @param name
 */

MapEditorActionChapterNew::MapEditorActionChapterNew(GameMapEditor *editor, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeChapterList)
{
	m_chapter = new GameMapEditorChapter(data.value("id").toInt(),
										 data.value("name").toString(),
										 m_editor, this);

	setDescription(QObject::tr("Új szakasz hozzáadása: %1").arg(m_chapter->name()));

	setUndoFunc([this](){
		chapterRemove(m_chapter);
	});

	setRedoFunc([this](){
		chapterAdd(m_chapter);
	});

}


/**
 * @brief MapEditorActionChapterNew::~MapEditorActionChapterNew
 */

MapEditorActionChapterNew::~MapEditorActionChapterNew()
{
	if (m_chapter && m_chapter->parent() == this) {
		m_chapter->deleteLater();
		m_chapter = nullptr;
	}
}



/**
 * @brief MapEditorActionChapterRemove::MapEditorActionChapterRemove
 * @param editor
 * @param name
 */


MapEditorActionChapterRemove::MapEditorActionChapterRemove(GameMapEditor *editor, GameMapEditorChapter *chapter)
	: MapEditorAction(editor, ActionTypeChapterList)
	, m_list()
{
	setDescription(QObject::tr("Szakasz törlése: %1").arg(chapter->name()));

	addChapter(chapter);

	m_undoFunc = std::bind(&MapEditorActionChapterRemove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionChapterRemove::_redo, this);
}


/**
 * @brief MapEditorActionChapterRemove::MapEditorActionChapterRemove
 * @param editor
 * @param list
 */

MapEditorActionChapterRemove::MapEditorActionChapterRemove(GameMapEditor *editor, const QList<GameMapEditorChapter *> &list)
	: MapEditorAction(editor, ActionTypeChapterList)
	, m_list()
{
	setDescription(QObject::tr("%1 szakasz törlése").arg(list.size()));

	foreach (GameMapEditorChapter *ch, list)
		addChapter(ch);

	m_undoFunc = std::bind(&MapEditorActionChapterRemove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionChapterRemove::_redo, this);
}


/**
 * @brief MapEditorActionChapterRemove::~MapEditorActionChapterRemove
 */

MapEditorActionChapterRemove::~MapEditorActionChapterRemove()
{

}


/**
 * @brief MapEditorActionChapterRemove::addChapter
 * @param chapter
 */

void MapEditorActionChapterRemove::addChapter(GameMapEditorChapter *chapter)
{
	ChapterList l(chapter);

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *ml, m->levels()->objects()) {
			if (ml->chapters()->objects().contains(chapter)) {
				qDebug() << "*** FOUND" << m->name() << ml->level();
				l.append(ml);
			}
		}
	}

	m_list.append(l);
}

/**
 * @brief MapEditorActionChapterRemove::_undo
 */

void MapEditorActionChapterRemove::_undo()
{
	foreach (const ChapterList &l, m_list) {
		chapterAdd(l.chapter);

		foreach (GameMapEditorMissionLevel *ml, l.levels)
			if (ml)
				ml->chapters()->objects().append(l.chapter);
	}
}


/**
 * @brief MapEditorActionChapterRemove::_redo
 */

void MapEditorActionChapterRemove::_redo()
{
	foreach (const ChapterList &l, m_list) {
		foreach (GameMapEditorMissionLevel *ml, l.levels)
			if (ml)
				ml->chapters()->objects().removeAll(l.chapter);

		chapterRemove(l.chapter);
	}
}



/**
 * @brief MapEditorActionObjectiveNew::MapEditorActionObjectiveNew
 * @param editor
 * @param uuid
 * @param module
 * @param storageId
 * @param storageCount
 * @param data
 */

MapEditorActionObjectiveNew::MapEditorActionObjectiveNew(GameMapEditor *editor, GameMapEditorChapter *parentChapter,
														 const QVariantMap &data, const QVariantMap &storageData)
	: MapEditorAction(editor, ActionTypeChapter, parentChapter->id())
	, m_storageDataSource()
	, m_storageDataTarget()
{
	m_storage = nullptr;
	m_storageEdited = nullptr;

	m_parentChapter = parentChapter;


	int storageId = storageData.value("id", -1).toInt();
	QString storageModule = storageData.value("module").toString();

	QString module = data.value("module").toString();
	QVariantMap objectiveData = data.value("data").toMap();

	QString readableName = Question::objectiveInfo(module, objectiveData).value("name").toString();

	if (storageId == -1 && !storageModule.isEmpty()) {
		int id = 1;

		foreach (GameMapEditorStorage *s, m_editor->storages()->objects())
			id = qMax(s->id()+1, id);

		m_storage = new GameMapEditorStorage(id, storageModule, storageData.value("data").toMap(), this);

		storageId = id;

		setDescription(QObject::tr("Új feladat+előállító hozzáadása: %1").arg(readableName));
	} else {
		setDescription(QObject::tr("Új feladat hozzáadása: %1").arg(readableName));

		if (storageId != -1) {
			m_storageEdited = m_editor->storage(storageId);
			m_storageDataSource = variantMapSave(m_storageEdited, storageData.value("data").toMap());
			m_storageDataTarget = storageData.value("data").toMap();
		}
	}

	m_objective = new GameMapEditorObjective(QUuid::createUuid().toString(), module, storageId,
											 data.value("storageCount", 0).toInt(),
											 objectiveData,
											 m_editor, this);


	setUndoFunc([this](){
		objectiveRemove(m_parentChapter, m_objective);

		if (m_storageEdited)
			variantMapSet(m_storageEdited, m_storageDataSource);

		if (m_storage)
			storageRemove(m_storage);
	});

	setRedoFunc([this](){
		if (m_storage)
			storageAdd(m_storage);

		if (m_storageEdited)
			variantMapSet(m_storageEdited, m_storageDataTarget);

		objectiveAdd(m_parentChapter, m_objective);
	});

}


/**
 * @brief MapEditorActionObjectiveNew::~MapEditorActionObjectiveNew
 */

MapEditorActionObjectiveNew::~MapEditorActionObjectiveNew()
{
	if (m_objective && m_objective->parent() == this) {
		m_objective->deleteLater();
		m_objective = nullptr;
	}

	if (m_storage && m_storage->parent() == this) {
		m_storage->deleteLater();
		m_storage = nullptr;
	}
}



/**
 * @brief MapEditorActionChapterModify::MapEditorActionChapterModify
 * @param editor
 * @param chapter
 * @param data
 */

MapEditorActionChapterModify::MapEditorActionChapterModify(GameMapEditor *editor, GameMapEditorChapter *chapter, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeChapter, chapter->id())
{
	m_chapter = chapter;
	m_dataSource = variantMapSave(chapter, data);
	m_dataTarget = data;

	setDescription(QObject::tr("Szakasz módosítása: %1").arg(chapter->name()));

	setUndoFunc([this](){
		variantMapSet(m_chapter, m_dataSource);
	});

	setRedoFunc([this](){
		variantMapSet(m_chapter, m_dataTarget);
	});
}



/**
 * @brief MapEditorActionChapterModify::~MapEditorActionChapterModify
 */

MapEditorActionChapterModify::~MapEditorActionChapterModify()
{

}


/**
 * @brief MapEditorActionObjectiveRemove::MapEditorActionObjectiveRemove
 * @param editor
 * @param parentChapter
 * @param objective
 */

MapEditorActionObjectiveRemove::MapEditorActionObjectiveRemove(GameMapEditor *editor,
															   GameMapEditorChapter *parentChapter,
															   GameMapEditorObjective *objective)
	: MapEditorAction(editor, ActionTypeChapter, parentChapter->id())
	, m_parentChapter(parentChapter)
	, m_list()
{
	setDescription(QObject::tr("Feladat törlése: %1").arg(objective->info()[0]));

	m_list.append(objective);

	m_undoFunc = std::bind(&MapEditorActionObjectiveRemove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionObjectiveRemove::_redo, this);
}


/**
 * @brief MapEditorActionObjectiveRemove::MapEditorActionObjectiveRemove
 * @param editor
 * @param parentChapter
 * @param list
 */

MapEditorActionObjectiveRemove::MapEditorActionObjectiveRemove(GameMapEditor *editor,
															   GameMapEditorChapter *parentChapter,
															   const QList<GameMapEditorObjective *> &list)
	: MapEditorAction(editor, ActionTypeChapter, parentChapter->id())
	, m_parentChapter(parentChapter)
	, m_list()
{
	setDescription(QObject::tr("%1 feladat törlése").arg(list.size()));

	foreach (GameMapEditorObjective *o, list)
		m_list.append(o);

	m_undoFunc = std::bind(&MapEditorActionObjectiveRemove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionObjectiveRemove::_redo, this);
}


/**
 * @brief MapEditorActionObjectiveRemove::~MapEditorActionObjectiveRemove
 */

MapEditorActionObjectiveRemove::~MapEditorActionObjectiveRemove()
{

}

/**
 * @brief MapEditorActionObjectiveRemove::_undo
 */

void MapEditorActionObjectiveRemove::_undo()
{
	foreach (GameMapEditorObjective *o, m_list)
		objectiveAdd(m_parentChapter, o);
}

/**
 * @brief MapEditorActionObjectiveRemove::_redo
 */

void MapEditorActionObjectiveRemove::_redo()
{
	foreach (GameMapEditorObjective *o, m_list)
		objectiveRemove(m_parentChapter, o);
}



/**
 * @brief MapEditorActionObjectiveModify::MapEditorActionObjectiveModify
 * @param editor
 * @param objective
 * @param data
 * @param storage
 * @param storageData
 */

MapEditorActionObjectiveModify::MapEditorActionObjectiveModify(GameMapEditor *editor, GameMapEditorChapter *chapter,
															   GameMapEditorObjective *objective, const QVariantMap &data,
															   GameMapEditorStorage *storage, const QVariantMap &storageData)
	: MapEditorAction(editor, ActionTypeChapter, chapter->id())
{
	m_parentChapter = chapter;
	m_objective = objective;
	m_dataSource = variantMapSave(objective, data);
	m_dataTarget = data;

	m_storage = storage;

	QString readableName = Question::objectiveInfo(objective->module(), objective->data()).value("name").toString();

	if (m_storage) {
		m_storageDataSource = variantMapSave(m_storage, storageData);
		m_storageDataTarget = storageData;

		setDescription(QObject::tr("Feladat+előállító módosítása: %1").arg(readableName));
	} else {
		setDescription(QObject::tr("Feladat módosítása: %1").arg(readableName));
	}


	setUndoFunc([this](){
		variantMapSet(m_objective, m_dataSource);

		if (m_storage)
			variantMapSet(m_storage, m_storageDataSource);

		m_parentChapter->recalculateCounts();
	});

	setRedoFunc([this](){
		if (m_storage)
			variantMapSet(m_storage, m_storageDataTarget);

		variantMapSet(m_objective, m_dataTarget);

		m_parentChapter->recalculateCounts();
	});
}


/**
 * @brief MapEditorActionObjectiveModify::~MapEditorActionObjectiveModify
 */

MapEditorActionObjectiveModify::~MapEditorActionObjectiveModify()
{

}





/**
 * @brief MapEditorActionObjectiveMove::MapEditorActionObjectiveMove
 * @param editor
 * @param parentChapter
 * @param objective
 * @param targetChapterData
 */

MapEditorActionObjectiveMove::MapEditorActionObjectiveMove(GameMapEditor *editor, GameMapEditorChapter *parentChapter,
														   GameMapEditorObjective *objective, const bool &isCopy, const QVariantMap &targetChapterData)
	: MapEditorAction(editor, ActionTypeChapter)
	, m_parentChapter(parentChapter)
	, m_list()
	, m_isCopy(isCopy)
	, m_targetChapter(nullptr)
	, m_isNewChapter(false)
{
	QString readableName = Question::objectiveInfo(objective->module(), objective->data()).value("name").toString();

	if (m_isCopy)
		setDescription(QObject::tr("Feladat másolása: %1").arg(readableName));
	else
		setDescription(QObject::tr("Feladat áthelyezése: %1").arg(readableName));

	_setTarget(targetChapterData);

	if (m_isCopy) {
		GameMapEditorObjective *obj = new GameMapEditorObjective(QUuid::createUuid().toString(),
																 objective->module(), objective->storageId(),
																 objective->storageCount(),
																 objective->data(),
																 m_editor, this);
		m_list.append(obj);
	} else {
		m_list.append(objective);
	}

	m_undoFunc = std::bind(&MapEditorActionObjectiveMove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionObjectiveMove::_redo, this);
}


/**
 * @brief MapEditorActionObjectiveMove::MapEditorActionObjectiveMove
 * @param editor
 * @param parentChapter
 * @param list
 * @param targetChapterData
 */

MapEditorActionObjectiveMove::MapEditorActionObjectiveMove(GameMapEditor *editor, GameMapEditorChapter *parentChapter,
														   const QList<GameMapEditorObjective *> &list, const bool &isCopy, const QVariantMap &targetChapterData)
	: MapEditorAction(editor, ActionTypeChapter)
	, m_parentChapter(parentChapter)
	, m_list()
	, m_isCopy(isCopy)
	, m_targetChapter(nullptr)
	, m_isNewChapter(false)
{
	if (m_isCopy)
		setDescription(QObject::tr("%1 feladat másolása").arg(list.size()));
	else
		setDescription(QObject::tr("%1 feladat áthelyezése").arg(list.size()));

	_setTarget(targetChapterData);

	if (m_isCopy) {
		foreach (GameMapEditorObjective *o, list) {
			GameMapEditorObjective *obj = new GameMapEditorObjective(QUuid::createUuid().toString(),
																	 o->module(), o->storageId(),
																	 o->storageCount(),
																	 o->data(),
																	 m_editor, this);
			m_list.append(obj);
		}
	} else {
		foreach (GameMapEditorObjective *o, list)
			m_list.append(o);
	}

	m_undoFunc = std::bind(&MapEditorActionObjectiveMove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionObjectiveMove::_redo, this);
}



/**
 * @brief MapEditorActionObjectiveMove::~MapEditorActionObjectiveMove
 */

MapEditorActionObjectiveMove::~MapEditorActionObjectiveMove()
{
	if (m_isCopy) {
		foreach (GameMapEditorObjective *o, m_list) {
			if (o && o->parent() == this)
				o->deleteLater();
		}

		m_list.clear();
	}

	if (m_isNewChapter && m_targetChapter && m_targetChapter->parent() == this) {
		m_targetChapter->deleteLater();
		m_targetChapter = nullptr;
	}
}


/**
 * @brief MapEditorActionObjectiveMove::_undo
 */

void MapEditorActionObjectiveMove::_undo()
{
	if (m_isCopy) {
		foreach (GameMapEditorObjective *o, m_list) {
			objectiveRemove(m_targetChapter, o);
		}
	} else {
		foreach (GameMapEditorObjective *o, m_list) {
			objectiveRemove(m_targetChapter, o);
			objectiveAdd(m_parentChapter, o);
		}
	}

	if (m_isNewChapter)
		chapterRemove(m_targetChapter);
}


/**
 * @brief MapEditorActionObjectiveMove::_redo
 */

void MapEditorActionObjectiveMove::_redo()
{
	if (m_isNewChapter)
		chapterAdd(m_targetChapter);

	if (m_isCopy) {
		foreach (GameMapEditorObjective *o, m_list) {
			objectiveAdd(m_targetChapter, o);
		}
	} else {
		foreach (GameMapEditorObjective *o, m_list) {
			objectiveRemove(m_parentChapter, o);
			objectiveAdd(m_targetChapter, o);
		}
	}
}




/**
 * @brief MapEditorActionObjectiveMove::_setTarget
 */

void MapEditorActionObjectiveMove::_setTarget(const QVariantMap &targetChapterData)
{
	int chapterId = targetChapterData.value("id", -1).toInt();

	if (chapterId == -1) {
		int id = 1;

		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects())
			id = qMax(ch->id()+1, id);

		m_targetChapter = new GameMapEditorChapter(id,
												   targetChapterData.value("name").toString(),
												   m_editor, this);
		m_isNewChapter = true;

		setContextId(id);

	} else {
		m_targetChapter = m_editor->chapter(chapterId);

		setContextId(chapterId);
	}
}


/**
 * @brief MapEditorActionChapterMissionLevels::MapEditorActionChapterMissionLevels
 * @param editor
 * @param chapter
 * @param levels
 */

MapEditorActionChapterMissionLevels::MapEditorActionChapterMissionLevels(GameMapEditor *editor, GameMapEditorChapter *chapter,
																		 const QList<GameMapEditorMissionLevel *> &levels)
	: MapEditorAction(editor, ActionTypeChapter, chapter->id())
	, m_chapter(chapter)
	, m_listSource()
	, m_listTarget()
{
	foreach (GameMapEditorMission *m, m_editor->missions()->objects()){
		foreach (GameMapEditorMissionLevel *ml, m->levels()->objects()) {
			if (ml->chapters()->objects().contains(chapter))
				m_listSource.append(ml);
		}
	}

	m_listTarget.reserve(levels.size());
	foreach (GameMapEditorMissionLevel *ml, levels) {
		m_listTarget.append(ml);
	}

	setDescription(QObject::tr("Szakasz küldetéseinek módosítása: %1").arg(chapter->name()));

	setUndoFunc([this](){
		updateMissionLevels(m_listSource);
		m_chapter->recalculateCounts();
	});

	setRedoFunc([this](){
		updateMissionLevels(m_listTarget);
		m_chapter->recalculateCounts();
	});
}


/**
 * @brief MapEditorActionChapterMissionLevels::~MapEditorActionChapterMissionLevels
 */

MapEditorActionChapterMissionLevels::~MapEditorActionChapterMissionLevels()
{

}


/**
 * @brief MapEditorActionChapterMissionLevels::updateMissionLevels
 * @param list
 */

void MapEditorActionChapterMissionLevels::updateMissionLevels(const QList<QPointer<GameMapEditorMissionLevel> > &list)
{
	if (!m_chapter)
		return;

	// Remove

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *ml, m->levels()->objects()) {
			if (!ml)
				continue;

			if (list.contains(ml))
				continue;

			for (int i=ml->chapters()->index(m_chapter); i != -1; i=ml->chapters()->index(m_chapter)) {
				ml->chapters()->removeObject(i);
			}
		}
	}


	// Append

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *ml, m->levels()->objects()) {
			if (list.contains(ml) && !ml->chapters()->objects().contains(m_chapter))
				ml->chapters()->addObject(m_chapter);
		}
	}

}



/**
 * @brief MapEditorActionMissionNew::MapEditorActionMissionNew
 * @param editor
 * @param data
 * @param terrain
 */

MapEditorActionMissionNew::MapEditorActionMissionNew(GameMapEditor *editor, const QVariantMap &data, const QString &terrain)
	: MapEditorAction(editor, ActionTypeMissionList)
	, m_mission(nullptr)
	, m_missionLevel(nullptr)
{
	QString medalImage = data.value("medalImage",
									Client::medalIcons().at(QRandomGenerator::global()->bounded(Client::medalIcons().size()))).toString();
	QByteArray uuid = data.value("uuid", QUuid::createUuid().toByteArray()).toByteArray();

	m_mission = new GameMapEditorMission(uuid,
										 data.value("name").toString(),
										 data.value("description").toString(),
										 medalImage,
										 m_editor, this);

	setDescription(QObject::tr("Új küldetés hozzáadása: %1").arg(m_mission->name()));

	QVariantMap defaults = m_editor->gameData()
						   .value("level").toMap()
						   .value("1").toMap()
						   .value("defaults").toMap();

	m_missionLevel = new GameMapEditorMissionLevel(1,
												   terrain.isEmpty() ?
													   defaults.value("terrain", "West_Louische/1").toByteArray() :
													   terrain.toLatin1(),
												   defaults.value("startHP", 5).toInt(),
												   defaults.value("duration", 300).toInt(),
												   defaults.value("deathmatch", true).toBool(),
												   defaults.value("questions", 0.5).toReal(),
												   defaults.value("image", "").toString(),
												   m_mission,
												   m_editor, this);

	missionLevelAdd(m_mission, m_missionLevel);

	setUndoFunc([this](){
		missionRemove(m_mission);
	});

	setRedoFunc([this](){
		missionAdd(m_mission);
	});
}

/**
 * @brief MapEditorActionMissionNew::~MapEditorActionMissionNew
 */

MapEditorActionMissionNew::~MapEditorActionMissionNew()
{
	if (m_mission && m_mission->parent() == this) {
		m_mission->deleteLater();
		m_mission = nullptr;
	}

	if (m_missionLevel && m_missionLevel->parent() == this) {
		m_missionLevel->deleteLater();
		m_missionLevel = nullptr;
	}
}

/**
 * @brief MapEditorActionMissionNew::missionLevel
 * @return
 */

GameMapEditorMissionLevel *MapEditorActionMissionNew::missionLevel() const
{
	return m_missionLevel;
}

GameMapEditorMission *MapEditorActionMissionNew::mission() const
{
	return m_mission;
}


/**
 * @brief MapEditorActionMissionRemove::MapEditorActionMissionRemove
 * @param editor
 * @param mission
 */

MapEditorActionMissionRemove::MapEditorActionMissionRemove(GameMapEditor *editor, GameMapEditorMission *mission)
	: MapEditorAction(editor, ActionTypeMissionList)
	, m_list()
{
	setDescription(QObject::tr("Küldetés törlése: %1").arg(mission->name()));

	addMission(mission);

	m_undoFunc = std::bind(&MapEditorActionMissionRemove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionMissionRemove::_redo, this);

}


/**
 * @brief MapEditorActionMissionRemove::MapEditorActionMissionRemove
 * @param editor
 * @param list
 */

MapEditorActionMissionRemove::MapEditorActionMissionRemove(GameMapEditor *editor, const QList<GameMapEditorMission *> &list)
	: MapEditorAction(editor, ActionTypeMissionList)
	, m_list()
{
	setDescription(QObject::tr("%1 küldetés törlése").arg(list.size()));

	foreach (GameMapEditorMission *ch, list)
		addMission(ch);

	m_undoFunc = std::bind(&MapEditorActionMissionRemove::_undo, this);
	m_redoFunc = std::bind(&MapEditorActionMissionRemove::_redo, this);

}


/**
 * @brief MapEditorActionMissionRemove::~MapEditorActionMissionRemove
 */

MapEditorActionMissionRemove::~MapEditorActionMissionRemove()
{

}


/**
 * @brief MapEditorActionMissionRemove::addMission
 * @param mission
 */

void MapEditorActionMissionRemove::addMission(GameMapEditorMission *mission)
{
	MissionList l(mission);

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *ml, m->locks()->objects()) {
			if (mission->levels()->objects().contains(ml))
				l.append(m, ml);
		}
	}

	m_list.append(l);
}


/**
 * @brief MapEditorActionMissionRemove::_undo
 */

void MapEditorActionMissionRemove::_undo()
{
	foreach (const MissionList &l, m_list) {
		missionAdd(l.mission);

		for (int i=0; i<l.locks.size(); i++) {
			const QPair<GameMapEditorMission*, GameMapEditorMissionLevel*> &p = l.locks.value(i);
			if (p.first && p.second)
				p.first->locks()->addObject(p.second);
		}
	}
}


/**
 * @brief MapEditorActionMissionRemove::_redo
 */

void MapEditorActionMissionRemove::_redo()
{
	foreach (const MissionList &l, m_list) {
		for (int i=0; i<l.locks.size(); i++) {
			const QPair<GameMapEditorMission*, GameMapEditorMissionLevel*> &p = l.locks.value(i);

			if (p.first && p.second) {
				auto list = p.first->locks();

				for (int j=list->index(p.second); j != -1; j=list->index(p.second)) {
					list->removeObject(j);
				}
			}
		}

		missionRemove(l.mission);
	}
}





/**
 * @brief MapEditorActionMissionModify::MapEditorActionMissionModify
 * @param editor
 * @param mission
 * @param data
 */

MapEditorActionMissionModify::MapEditorActionMissionModify(GameMapEditor *editor, GameMapEditorMission *mission, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeMission, mission->uuid())
{
	m_mission = mission;
	m_dataSource = variantMapSave(mission, data);
	m_dataTarget = data;

	setDescription(QObject::tr("Küldetés módosítása: %1").arg(mission->name()));

	setUndoFunc([this](){
		variantMapSet(m_mission, m_dataSource);
	});

	setRedoFunc([this](){
		variantMapSet(m_mission, m_dataTarget);
	});
}



/**
 * @brief MapEditorActionMissionModify::~MapEditorActionMissionModify
 */

MapEditorActionMissionModify::~MapEditorActionMissionModify()
{

}



/**
 * @brief MapEditorActionMissionLevelNew::MapEditorActionMissionLevelNew
 * @param editor
 * @param parentMission
 * @param data
 */

MapEditorActionMissionLevelNew::MapEditorActionMissionLevelNew(GameMapEditor *editor, GameMapEditorMission *parentMission, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeMission, parentMission->uuid())
	, m_mission(parentMission)
{
	int level = data.value("level", 1).toInt();

	setDescription(QObject::tr("Új szint hozzáadása: %1").arg(m_mission->name()));


	QVariantMap defaults = m_editor->gameData()
						   .value("level").toMap()
						   .value(QString("%1").arg(level)).toMap()
						   .value("defaults").toMap();

	m_missionLevel = new GameMapEditorMissionLevel(level,
												   data.contains("terrain") ?
													   data.value("terrain").toByteArray() :
													   defaults.value("terrain", "West_Louische/1").toByteArray(),
												   data.contains("startHP") ?
													   data.value("startHP").toInt() :
													   defaults.value("startHP", 5).toInt(),
												   data.contains("duration") ?
													   data.value("duration").toInt() :
													   defaults.value("duration", 300).toInt(),
												   data.contains("deathmatch") ?
													   data.value("deathmatch").toBool() :
													   defaults.value("deathmatch", true).toBool(),
												   data.contains("questions") ?
													   data.value("questions").toReal() :
													   defaults.value("questions", 0.5).toReal(),
												   data.contains("image") ?
													   data.value("image").toString() :
													   defaults.value("image", "").toString(),
												   m_mission,
												   m_editor, this);

	setUndoFunc([this](){
		missionLevelRemove(m_mission, m_missionLevel);
	});

	setRedoFunc([this](){
		missionLevelAdd(m_mission, m_missionLevel);
	});

}

/**
 * @brief MapEditorActionMissionLevelNew::~MapEditorActionMissionLevelNew
 */

MapEditorActionMissionLevelNew::~MapEditorActionMissionLevelNew()
{
	if (m_missionLevel && m_missionLevel->parent() == this) {
		m_missionLevel->deleteLater();
		m_missionLevel = nullptr;
	}
}

GameMapEditorMissionLevel *MapEditorActionMissionLevelNew::missionLevel() const
{
	return m_missionLevel;
}



/**
 * @brief MapEditorActionMissionLevelRemove::MapEditorActionMissionLevelRemove
 * @param editor
 * @param missionLevel
 */

MapEditorActionMissionLevelRemove::MapEditorActionMissionLevelRemove(GameMapEditor *editor,
																	 GameMapEditorMission *parentMission,
																	 GameMapEditorMissionLevel *missionLevel)
	: MapEditorAction(editor, ActionTypeMission, parentMission->uuid())
	, m_mission(parentMission)
	, m_missionLevel(missionLevel)
	, m_locks()
{
	setDescription(QObject::tr("Szint törlése: %1 (%2)").arg(m_mission->name()).arg(m_missionLevel->level()));

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		if (m->locks()->objects().contains(m_missionLevel))
			m_locks.append(m);
	}


	setUndoFunc([this](){
		missionLevelAdd(m_mission, m_missionLevel);

		foreach (GameMapEditorMission *m, m_locks)
			m->locks()->addObject(m_missionLevel);
	});

	setRedoFunc([this]() {
		foreach (GameMapEditorMission *m, m_locks) {
			for (int j=m->locks()->index(m_missionLevel); j != -1; j=m->locks()->index(m_missionLevel))
				m->locks()->removeObject(j);
		}

		missionLevelRemove(m_mission, m_missionLevel);
	});


}


/**
 * @brief MapEditorActionMissionLevelRemove::~MapEditorActionMissionLevelRemove
 */

MapEditorActionMissionLevelRemove::~MapEditorActionMissionLevelRemove()
{
	if (m_missionLevel && m_missionLevel->parent() == this) {
		m_missionLevel->deleteLater();
		m_missionLevel = nullptr;
	}
}



/**
 * @brief MapEditorActionMissionLevelModify::MapEditorActionMissionLevelModify
 * @param editor
 * @param missionLevel
 * @param data
 */

MapEditorActionMissionLevelModify::MapEditorActionMissionLevelModify(GameMapEditor *editor,
																	 GameMapEditorMissionLevel *missionLevel,
																	 const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeMissionLevel)
	, m_missionLevel(missionLevel)
{
	m_dataSource = variantMapSave(missionLevel, data);
	m_dataTarget = data;

	setDescription(QObject::tr("Szint módosítása: %1 (%2)").arg(missionLevel->editorMission()->name()).arg(missionLevel->level()));

	setUndoFunc([this](){
		variantMapSet(m_missionLevel, m_dataSource);
	});

	setRedoFunc([this](){
		variantMapSet(m_missionLevel, m_dataTarget);
	});
}


/**
 * @brief MapEditorActionMissionLevelModify::~MapEditorActionMissionLevelModify
 */

MapEditorActionMissionLevelModify::~MapEditorActionMissionLevelModify()
{

}
