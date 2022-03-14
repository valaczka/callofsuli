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
