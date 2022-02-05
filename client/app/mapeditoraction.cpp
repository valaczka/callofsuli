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

const MapEditorAction::MapEditorActionType &MapEditorAction::type() const
{
	return m_type;
}

void MapEditorAction::setType(const MapEditorActionType &newType)
{
	if (m_type == newType)
		return;
	m_type = newType;
	emit typeChanged();
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
	m_editor->m_chapters->removeObject(index);
	chapter->setParent(this);
}


/**
 * @brief MapEditorAction::objectiveAdd
 * @param objective
 */

void MapEditorAction::objectiveAdd(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	chapter->m_objectives->addObject(objective);
	objective->setParent(chapter);
}


/**
 * @brief GameMapEditorChapter::objectiveRemove
 * @param objective
 */

void MapEditorAction::objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	int index = chapter->m_objectives->index(objective);
	Q_ASSERT(index != -1);
	chapter->m_objectives->removeObject(index);
	objective->setParent(this);
}













/**
 * @brief MapEditorActionChapterNew::MapEditorActionChapterNew
 * @param name
 */

MapEditorActionChapterNew::MapEditorActionChapterNew(GameMapEditor *editor, const qint32 &id, const QString &name)
	: MapEditorAction(editor, ActionTypeChapterList)
{
	m_chapter = new GameMapEditorChapter(id, name, m_editor, this);

	setDescription(QObject::tr("Új szakasz hozzáadása: %1").arg(name));

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
		qDebug() << "DESTORY chapter" << m_chapter << "PARENT" << this;
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
	, m_levels()
{
	m_chapter = chapter;
	setDescription(QObject::tr("Szakasz törlése: %1").arg(m_chapter->name()));

	foreach (GameMapEditorMission *m, m_editor->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *ml, m->levels()->objects()) {
			if (ml->chapters()->objects().contains(m_chapter)) {
				qDebug() << "*** FOUND" << m->name() << ml->level();
				m_levels.append(ml);
			}
		}
	}

	setUndoFunc([this](){
		chapterAdd(m_chapter);

		foreach (GameMapEditorMissionLevel *ml, m_levels)
			if (ml)
				ml->chapters()->objects().append(m_chapter);
	});

	setRedoFunc([this](){
		foreach (GameMapEditorMissionLevel *ml, m_levels)
			if (ml)
				ml->chapters()->objects().removeAll(m_chapter);

		chapterRemove(m_chapter);
	});

}


/**
 * @brief MapEditorActionChapterRemove::~MapEditorActionChapterRemove
 */

MapEditorActionChapterRemove::~MapEditorActionChapterRemove()
{

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
														 const QString &uuid,
														 const QString &module, const qint32 &storageId,
														 const qint32 &storageCount, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeChapter, parentChapter->id())
{
	m_parentChapter = parentChapter;
	m_objective = new GameMapEditorObjective(uuid, module, storageId, storageCount, data, m_editor, this);

	setDescription(QObject::tr("Új célpont hozzáadása: %1").arg(module));

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

