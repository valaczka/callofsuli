/*
 * ---- Call of Suli ----
 *
 * mapeditor.cpp
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

#include <QtJsonSerializer>

#include "mapeditor.h"
#include "editoraction.h"
#include "mapeditoraction.h"

MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_editor(nullptr)
	, m_undoStack(new EditorUndoStack(this))
	, m_url()
{
	connect(m_undoStack, &EditorUndoStack::undoCompleted, this, &MapEditor::onUndoRedoCompleted);
	connect(m_undoStack, &EditorUndoStack::redoCompleted, this, &MapEditor::onUndoRedoCompleted);
}


/**
 * @brief MapEditor::editor
 * @return
 */

GameMapEditor *MapEditor::editor() const
{
	return m_editor;
}


void MapEditor::setEditor(GameMapEditor *newEditor)
{
	if (m_editor == newEditor)
		return;
	m_editor = newEditor;
	emit editorChanged();
}


EditorUndoStack *MapEditor::undoStack() const
{
	return m_undoStack;
}

void MapEditor::setUndoStack(EditorUndoStack *newUndoStack)
{
	if (m_undoStack == newUndoStack)
		return;
	m_undoStack = newUndoStack;
	emit undoStackChanged();
}



/**
 * @brief MapEditor::open
 * @param url
 */

void MapEditor::open(const QUrl &url)
{
	if (m_editor) {
		Client::clientInstance()->sendMessageWarning(tr("Pálya megnyitás"), tr("Már meg van nyitva egy pálya!"), m_url.toLocalFile());
		return;
	}

	GameMapEditor *e = GameMapEditor::fromBinaryData(Client::fileContent(url.toLocalFile()), this);

	if (!e) {
		Client::clientInstance()->sendMessageWarning(tr("Hibás pálya"), tr("Nem lehet megnyitni a fájlt!"), url.toLocalFile());
		return;
	}

	setEditor(e);
	setUrl(url);
	m_undoStack->clear();
	setDisplayName(url.toLocalFile());
}


/**
 * @brief MapEditor::create
 */

void MapEditor::create()
{
	if (m_editor) {
		Client::clientInstance()->sendMessageWarning(tr("Új pálya"), tr("Már meg van nyitva egy pálya!"), m_url.toLocalFile());
		return;
	}

	GameMapEditor *e = new GameMapEditor(this);
	setEditor(e);
	setUrl(QUrl());
	m_undoStack->clear();
	setDisplayName(tr("-- Új pálya --"));
}



/**
 * @brief MapEditor::close
 */

void MapEditor::close()
{
	setUrl(QUrl());
	setDisplayName("");
	m_undoStack->clear();

	if (m_editor) {
		m_editor->deleteLater();
		setEditor(nullptr);
	}
}


/**
 * @brief MapEditor::url
 * @return
 */

const QUrl &MapEditor::url() const
{
	return m_url;
}

void MapEditor::setUrl(const QUrl &newUrl)
{
	if (m_url == newUrl)
		return;
	m_url = newUrl;
	emit urlChanged();
}

const QString &MapEditor::displayName() const
{
	return m_displayName;
}

void MapEditor::setDisplayName(const QString &newDisplayName)
{
	if (m_displayName == newDisplayName)
		return;
	m_displayName = newDisplayName;
	emit displayNameChanged();
}



/**
 * @brief MapEditor::chapterAdd
 * @param data
 */

void MapEditor::chapterAdd(QVariantMap data)
{
	int id = 1;

	foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects())
		id = qMax(ch->id()+1, id);

	data.insert("id", id);

	m_undoStack->call(new MapEditorActionChapterNew(m_editor, data));

}


/**
 * @brief MapEditor::chapterRemove
 * @param chapter
 */

void MapEditor::chapterRemove(GameMapEditorChapter *chapter)
{
	if (!chapter)
		return;

	m_undoStack->call(new MapEditorActionChapterRemove(m_editor, chapter));
}


/**
 * @brief MapEditor::chapterRemove
 * @param list
 */

void MapEditor::chapterRemoveList(const QList<GameMapEditorChapter *> &list)
{
	if (list.isEmpty())
		return;

	m_undoStack->call(new MapEditorActionChapterRemove(m_editor, list));
}

/**
 * @brief MapEditor::chapterModify
 * @param chapter
 * @param data
 */

void MapEditor::chapterModify(GameMapEditorChapter *chapter, const QVariantMap &data)
{
	if (!chapter)
		return;

	m_undoStack->call(new MapEditorActionChapterModify(m_editor, chapter, data));
}



/**
 * @brief MapEditor::chapterModelUnselectAll
 * @param model
 * @return
 */

bool MapEditor::chapterModelUnselectObjectives(ObjectGenericListModel<GameMapEditorChapter> *model)
{
	if (!model)
		return false;

	bool hasUnselect = false;

	foreach (GameMapEditorChapter *ch, model->objects()) {
		if (ch->objectives()->selectedCount()) {
			hasUnselect = true;
			ch->objectives()->unselectAll();
		}
	}

	return hasUnselect;
}


/**
 * @brief MapEditor::objectiveAdd
 * @param chapter
 * @param data
 */

void MapEditor::objectiveAdd(GameMapEditorChapter *chapter, QVariantMap data)
{

}


/**
 * @brief MapEditor::objectiveRemove
 * @param chapter
 * @param objective
 */

void MapEditor::objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective)
{
	if (!chapter || !objective)
		return;

	m_undoStack->call(new MapEditorActionObjectiveRemove(m_editor, chapter, objective));
}


/**
 * @brief MapEditor::objectiveRemoveList
 * @param chapter
 * @param list
 */

void MapEditor::objectiveRemoveList(GameMapEditorChapter *chapter, const QList<GameMapEditorObjective *> &list)
{
	if (!chapter || list.isEmpty())
		return;

	m_undoStack->call(new MapEditorActionObjectiveRemove(m_editor, chapter, list));
}


/**
 * @brief MapEditor::objectiveModify
 * @param objective
 * @param data
 */

void MapEditor::objectiveModify(GameMapEditorObjective *objective, const QVariantMap &data)
{

}




/**
 * @brief MapEditor::onStepChanged
 */


void MapEditor::onUndoRedoCompleted(const int &lastStep)
{
	if (m_editor) {
		{
			qDebug() << "=========";
			foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
				qDebug() << ch->id() << ch->name();
			}
			qDebug() << "=========";
		}
	}

	if (m_undoStack->actions().isEmpty()) {
		emit actionContextUpdated(MapEditorAction::ActionTypeInvalid, QVariant::Invalid);
		return;
	}

	int s = qMin(qMax(lastStep, 0), m_undoStack->actions().size());

	EditorAction *a = m_undoStack->actions().at(s);
	MapEditorAction *ma = qobject_cast<MapEditorAction*>(a);

	if (!ma) {
		emit actionContextUpdated(MapEditorAction::ActionTypeInvalid, QVariant::Invalid);
		return;
	}

	emit actionContextUpdated(ma->type(), ma->contextId());
}
