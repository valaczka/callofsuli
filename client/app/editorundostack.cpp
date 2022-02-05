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
	, m_actions()
	, m_step(-1)
	, m_savedStep(-1)
{

}

/**
 * @brief EditorUndoStack::~EditorUndoStack
 */

EditorUndoStack::~EditorUndoStack()
{
	qDeleteAll(m_actions);
	m_actions.clear();
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
	qDebug() << "ADD ACTION" << m_actions.size() << m_step;

	if (m_savedStep > m_step)
		m_savedStep = -1;

	while (m_step < m_actions.size()-1) {
		EditorAction *a = m_actions.takeLast();
		a->deleteLater();
		qDebug() << "TRUNCATED" << m_actions.size();
	}

	m_actions.append(action);
	action->redo();

	m_step = m_actions.size()-1;

	emit sizeChanged();
	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();

	return action;
}


/**
 * @brief EditorUndoStack::undo
 */

bool EditorUndoStack::undo(const int &steps)
{
	if (!canUndo())
		return false;

	for (int i=0; i<steps && m_step >= 0; ++i) {
		EditorAction *a = m_actions.at(m_step);

		qDebug() << "UNDO STACK UNDO" << a << m_step;
		a->undo();

		--m_step;
	}

	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();
	emit undoCompleted();

	return true;
}


/**
 * @brief EditorUndoStack::redo
 * @return
 */

bool EditorUndoStack::redo(const int &steps)
{
	if (!canRedo())
		return false;

	for (int i=0; i<steps && m_step<m_actions.size()-1; ++i) {
		EditorAction *a = m_actions.at(m_step+1);

		qDebug() << "REDO STACK REDO" << a << m_step;
		a->redo();

		++m_step;
	}

	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();
	emit redoCompleted();

	return true;
}


/**
 * @brief EditorUndoStack::clear
 */

void EditorUndoStack::clear()
{
	qDeleteAll(m_actions);
	m_actions.clear();

	setSavedStep(-1);
	m_step = -1;

	emit sizeChanged();
	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();
}


int EditorUndoStack::savedStep() const
{
	return m_savedStep;
}

void EditorUndoStack::setSavedStep(int newSavedStep)
{
	if (m_savedStep == newSavedStep)
		return;
	m_savedStep = newSavedStep;
	emit savedStepChanged();
}


/**
 * @brief EditorUndoStack::canUndo
 * @return
 */

bool EditorUndoStack::canUndo() const
{
	return !m_actions.isEmpty() && m_step >= 0;
}

int EditorUndoStack::size() const
{
	return m_actions.size();
}

bool EditorUndoStack::canRedo() const
{
	return !m_actions.isEmpty() && m_step < m_actions.size()-1;
}

int EditorUndoStack::step() const
{
	return m_step;
}
