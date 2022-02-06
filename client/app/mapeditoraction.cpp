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
		qDebug() << "DESTROY chapter" << m_chapter << "PARENT" << this;
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

MapEditorActionObjectiveNew::MapEditorActionObjectiveNew(GameMapEditor *editor, GameMapEditorChapter *parentChapter, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeChapter, parentChapter->id())
{
	m_parentChapter = parentChapter;
	//m_objective = new GameMapEditorObjective(uuid, module, storageId, storageCount, data, m_editor, this);

	//setDescription(QObject::tr("Új célpont hozzáadása: %1").arg(module));

	setUndoFunc([this](){
		objectiveRemove(m_parentChapter, m_objective);
	});

	setRedoFunc([this](){
		objectiveAdd(m_parentChapter, m_objective);
	});
}


/**
 * @brief MapEditorActionObjectiveNew::~MapEditorActionObjectiveNew
 */

MapEditorActionObjectiveNew::~MapEditorActionObjectiveNew()
{
	if (m_objective && m_objective->parent() == this) {
		qDebug() << "DESTORY objective" << m_objective << "PARENT" << this;
		m_objective->deleteLater();
		m_objective = nullptr;
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

	setDescription(QObject::tr("Célpont módosítása: %1").arg(chapter->name()));

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
