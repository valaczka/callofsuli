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
{

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

	QtJsonSerializer::JsonSerializer serializer;

	{
		QJsonValue ch = serializer.serialize(e);
		qDebug() << ch;
		QByteArray b = e->toBinaryData();
		QFile f("/tmp/out1.map");
		f.open(QIODevice::WriteOnly);
		f.write(b);
		f.close();
	}

	for (int i=0; i<3; ++i) {
		auto ch = e->chapters()->objects();
		if (!ch.isEmpty()) {
			m_undoStack->call(new MapEditorActionChapterRemove(e, ch.last()));
		}
	}

	m_undoStack->call(new MapEditorActionChapterNew(e, "első chapter"));
	m_undoStack->call(new MapEditorActionChapterNew(e, "második chapter"));
	m_undoStack->call(new MapEditorActionChapterNew(e, "harmadik chapter"));

	{
		QJsonValue ch = serializer.serialize(e);
		qDebug() << ch;
		QByteArray b = e->toBinaryData();
		QFile f("/tmp/out2.map");
		f.open(QIODevice::WriteOnly);
		f.write(b);
		f.close();
	}
}

void MapEditor::unloadTest()
{
	while (m_undoStack->canUndo()) {
		m_undoStack->undo();
	}

	if (m_editor) {
	QtJsonSerializer::JsonSerializer serializer;
	QJsonValue ch = serializer.serialize(m_editor);
	qDebug() << ch;
	QByteArray b = m_editor->toBinaryData();
	QFile f("/tmp/out3.map");
	f.open(QIODevice::WriteOnly);
	f.write(b);
	f.close();


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
