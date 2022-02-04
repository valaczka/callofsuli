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


MapEditorAction::MapEditorAction(GameMapEditor *editor, const MapEditorActionType &type, void *data)
	: EditorAction(data)
	, m_type(type)
	, m_editor(editor)
{
	Q_ASSERT(editor);
}



/**
 * @brief MapEditorAction::type
 * @return
 */

MapEditorAction::MapEditorActionType MapEditorAction::type() const
{
	return m_type;
}

void MapEditorAction::setType(MapEditorActionType newType)
{
	m_type = newType;
}



/**
 * @brief MapEditorAction::chapterAdd
 * @param chapter
 */

void MapEditorAction::chapterAdd(GameMapEditorChapter *chapter)
{
	Q_ASSERT(m_editor);
	Q_ASSERT(chapter);
	qDebug() << "REAL CHAPTER ADD" << chapter->id() << chapter->name();
	m_editor->m_chapters->addObject(chapter);
}


/**
 * @brief GameMapEditor::chapterRemove
 * @param chapter
 */

void MapEditorAction::chapterRemove(GameMapEditorChapter *chapter)
{
	Q_ASSERT(m_editor);
	qDebug() << this << "REAL CHAPTER REMOVE" << chapter << chapter->id() << chapter->name();
	int index = m_editor->m_chapters->index(chapter);
	Q_ASSERT(index != -1);
	m_editor->m_chapters->removeObject(index);
}


/**
 * @brief MapEditorAction::objectiveAdd
 * @param objective
 */

void MapEditorAction::objectiveAdd(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	Q_ASSERT(chapter);
	Q_ASSERT(objective);
	qDebug() << "REAL OBJECTIVE ADD" << objective << objective->uuid() << objective->module();
	chapter->m_objectives->addObject(objective);
}


/**
 * @brief GameMapEditorChapter::objectiveRemove
 * @param objective
 */

void MapEditorAction::objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	Q_ASSERT(chapter);
	Q_ASSERT(objective);
	qDebug() << this << "REAL OBJECTIVE REMOVE" << objective << objective->uuid() << objective->module();
	int index = chapter->m_objectives->index(objective);
	Q_ASSERT(index != -1);
	chapter->m_objectives->removeObject(index);
}


/**
 * @brief MapEditorActionChapterNew::MapEditorActionChapterNew
 * @param name
 */

MapEditorActionChapterNew::MapEditorActionChapterNew(GameMapEditor *editor, const QString &name)
	: MapEditorAction(editor, ActionTypeChapterList)
{
	m_name = name;
	m_chapter = nullptr;

	setDescription(QObject::tr("Új szakasz hozzáadása: %1").arg(name));

	setUndoFunc([this](EditorAction *){
		chapterRemove(m_chapter);
		m_chapter->deleteLater();
		m_chapter = nullptr;
	});

	setRedoFunc([this](EditorAction *){
		int id = 1;
		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
			if (ch->id() >= id)
				id = ch->id()+1;
		}

		m_chapter = new GameMapEditorChapter(id, m_name, m_editor, m_editor);
		chapterAdd(m_chapter);
	});

}



/**
 * @brief MapEditorActionChapterRemove::MapEditorActionChapterRemove
 * @param editor
 * @param name
 */


MapEditorActionChapterRemove::MapEditorActionChapterRemove(GameMapEditor *editor, GameMapEditorChapter *chapter)
	: MapEditorAction(editor, ActionTypeChapterList)
{
	m_name = chapter->name();
	m_chapter = chapter;

	setDescription(QObject::tr("Szakasz törlése: %1").arg(m_name));

	setUndoFunc([this](EditorAction *){
		int id = 1;
		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
			if (ch->id() >= id)
				id = ch->id()+1;
		}

		m_chapter = new GameMapEditorChapter(id, m_name, m_editor, m_editor);
		chapterAdd(m_chapter);
	});

	setRedoFunc([this](EditorAction *){
		chapterRemove(m_chapter);
		m_chapter->deleteLater();
		m_chapter = nullptr;
	});

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

MapEditorActionObjectiveNew::MapEditorActionObjectiveNew(GameMapEditor *editor,
														 const QString &module, const qint32 &storageId,
														 const qint32 &storageCount, const QVariantMap &data)
	: MapEditorAction(editor, ActionTypeChapter)
{
	QString m_uuid = QUuid::createUuid().toString();
	QString m_module = module;
	qint32 m_storageId = storageId;
	qint32 m_storageCount = storageCount;
	QVariantMap m_data = data;

	setDescription(QObject::tr("Új célpont hozzáadása: %1").arg(module));

	setUndoFunc([this](EditorAction *){
		chapterRemove(m_chapter);
		m_chapter->deleteLater();
		m_chapter = nullptr;
	});

	setRedoFunc([this](EditorAction *){
		m_id = 1;

		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
			if (ch->id() >= m_id)
				m_id = ch->id()+1;
		}

		m_chapter = new GameMapEditorChapter(m_id, m_name, m_editor, m_editor);
		chapterAdd(m_chapter);
	});
}
