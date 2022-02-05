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

/**
 * @brief MapEditor::loadTest
 */

void MapEditor::loadTest()
{
	GameMapEditor *e = GameMapEditor::fromBinaryData(Client::fileContent("/home/valaczka/ddd.map"), this);
	setEditor(e);

	{
		qDebug() << "-------";
		foreach (GameMapEditorChapter *ch, e->chapters()->objects()) {
			qDebug() << ch->id() << ch->name();
		}
		qDebug() << "-------";
	}

	m_undoStack->call(new MapEditorActionChapterNew(e, 199, "nulladik chapter"));

	for (int i=0; i<2; ++i) {
		auto ch = e->chapters()->objects();
		if (!ch.isEmpty()) {
			m_undoStack->call(new MapEditorActionChapterRemove(e, ch.first()));
		}
	}

	m_undoStack->call(new MapEditorActionChapterNew(e, 195, "első chapter"));
	m_undoStack->call(new MapEditorActionChapterNew(e, 196, "második chapter"));
	m_undoStack->call(new MapEditorActionChapterNew(e, 197, "harmadik chapter"));

	{
		qDebug() << "-------";
		foreach (GameMapEditorChapter *ch, e->chapters()->objects()) {
			qDebug() << ch->id() << ch->name();
		}
		qDebug() << "-------";
	}
}

void MapEditor::unloadTest()
{
	int i=3;

	while (m_undoStack->canUndo() && i-->0) {
		m_undoStack->undo();
	}

	if (m_editor) {
		{
			qDebug() << "-------";
			foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
				qDebug() << ch->id() << ch->name();
			}
			qDebug() << "-------";
		}


		m_editor->deleteLater();
		setEditor(nullptr);
	}
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

void MapEditor::addTest()
{
	m_undoStack->call(new MapEditorActionChapterNew(m_editor, m_editor->chapters()->objects().size()+100, "nulladik chapter"));

	{
		qDebug() << "-------";
		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
			qDebug() << ch->id() << ch->name();
		}
		qDebug() << "-------";
	}


}

void MapEditor::removeTest()
{
	auto ch = m_editor->chapters()->objects();
	if (!ch.isEmpty()) {
		m_undoStack->call(new MapEditorActionChapterRemove(m_editor, ch.first()));
	}

	{
		qDebug() << "-------";
		foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
			qDebug() << ch->id() << ch->name();
		}
		qDebug() << "-------";
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
 * @brief MapEditor::onStepChanged
 */


void MapEditor::onUndoRedoCompleted()
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

	int s = qMin(qMax(m_undoStack->step(),0), m_undoStack->actions().size());

	EditorAction *a = m_undoStack->actions().at(s);
	MapEditorAction *ma = qobject_cast<MapEditorAction*>(a);

	if (!ma) {
		emit actionContextUpdated(MapEditorAction::ActionTypeInvalid, QVariant::Invalid);
		return;
	}

	emit actionContextUpdated(ma->type(), ma->contextId());
}
