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
#include "editorundostack.h"

class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(GameMapEditor* editor READ editor WRITE setEditor NOTIFY editorChanged)
	Q_PROPERTY(EditorUndoStack *undoStack READ undoStack WRITE setUndoStack NOTIFY undoStackChanged)

public:
	explicit MapEditor(QQuickItem *parent = nullptr);

	GameMapEditor *editor() const;
	void setEditor(GameMapEditor *newEditor);

	Q_INVOKABLE void loadTest();
	Q_INVOKABLE void unloadTest();

	EditorUndoStack *undoStack() const;
	void setUndoStack(EditorUndoStack *newUndoStack);

signals:
	void editorChanged();

	void undoStackChanged();

private:
	GameMapEditor *m_editor;
	EditorUndoStack *m_undoStack;
};

#endif // MAPEDITOR_H
