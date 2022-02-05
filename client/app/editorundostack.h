/*
 * ---- Call of Suli ----
 *
 * editorundostack.h
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * EditorUndoStack
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

#ifndef EDITORUNDOSTACK_H
#define EDITORUNDOSTACK_H

#include <QObject>
#include "editoraction.h"

class EditorUndoStack : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<EditorAction*> actions READ actions NOTIFY actionsChanged)
	Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
	Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
	Q_PROPERTY(int size READ size NOTIFY sizeChanged)
	Q_PROPERTY(int step READ step NOTIFY stepChanged)
	Q_PROPERTY(int savedStep READ savedStep WRITE setSavedStep NOTIFY savedStepChanged)

public:
	explicit EditorUndoStack(QObject *parent = nullptr);
	virtual ~EditorUndoStack();

	const QList<EditorAction*> &actions() const;

	int size() const;
	int step() const;
	virtual bool canUndo() const;
	virtual bool canRedo() const;

	int savedStep() const;
	void setSavedStep(int newSavedStep);

public slots:
	EditorAction *call(EditorAction *action);
	bool undo(const int &steps = 1);
	bool redo(const int &steps = 1);
	void clear();

signals:
	void undoCompleted();
	void redoCompleted();

	void actionsChanged();
	void sizeChanged();
	void canUndoChanged();
	void canRedoChanged();
	void stepChanged();
	void savedStepChanged();

protected:
	QList<EditorAction *> m_actions;
	int m_step;
	int m_savedStep;
};

#endif // EDITORUNDOSTACK_H
