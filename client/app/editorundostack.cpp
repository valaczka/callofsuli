/*
 * ---- Call of Suli ----
 *
 * editorundostack.cpp
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

#include "editorundostack.h"
#include <QtDebug>

EditorUndoStack::EditorUndoStack(QObject *parent)
	: QObject{parent}
{

}

const QList<EditorAction *> &EditorUndoStack::actions() const
{
	return m_actions;
}


/**
 * @brief EditorUndoStack::append
 * @param action
 */

EditorAction* EditorUndoStack::call(EditorAction *action)
{
	qDebug() << "ADD ACTION" << m_actions.size();
	m_actions.append(action);
	action->redo();
	emit canUndoChanged();

	return action;
}


/**
 * @brief EditorUndoStack::undo
 */

bool EditorUndoStack::undo()
{
	if (m_actions.isEmpty())
		return false;

	EditorAction *a = m_actions.takeLast();
	a->undo();

	qDebug() << "UNDO STACK UNDO" << a << m_actions.size();

	emit canUndoChanged();

	return true;
}


/**
 * @brief EditorUndoStack::canUndo
 * @return
 */

bool EditorUndoStack::canUndo() const
{
	return m_actions.size() > 0;
}
