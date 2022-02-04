/*
 * ---- Call of Suli ----
 *
 * editoraction.cpp
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * EditorAction
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

#include <QDebug>
#include "editoraction.h"

/**
 * @brief EditorAction::EditorAction
 * @param data
 */

EditorAction::EditorAction(void *data)
	: m_undoFunc(nullptr)
	, m_redoFunc(nullptr)
	, m_subUndoActions()
	, m_subRedoActions()
	, m_data(data)
	, m_canUndo(false)
	, m_canRedo(true)
{

}


/**
 * @brief EditorAction::EditorAction
 * @param undoFunc
 * @param data
 */

EditorAction::EditorAction(const std::function<void (EditorAction *)> &undoFunc, void *data)
	: m_undoFunc(undoFunc)
	, m_redoFunc(nullptr)
	, m_subUndoActions()
	, m_subRedoActions()
	, m_data(data)
	, m_canUndo(false)
	, m_canRedo(true)
{

}


/**
 * @brief EditorAction::EditorAction
 * @param undoFunc
 * @param redoFunc
 * @param data
 */

EditorAction::EditorAction(const std::function<void (EditorAction *)> &undoFunc, const std::function<void (EditorAction *)> &redoFunc, void *data)
	: m_undoFunc(undoFunc)
	, m_redoFunc(redoFunc)
	, m_subUndoActions()
	, m_subRedoActions()
	, m_data(data)
	, m_canUndo(false)
	, m_canRedo(true)
{

}


EditorAction::~EditorAction()
{
	qDeleteAll(m_subUndoActions);
	qDeleteAll(m_subRedoActions);

	m_subUndoActions.clear();
	m_subRedoActions.clear();
}

/**
 * @brief EditorAction::addSubUndoAction
 * @param action
 */

void EditorAction::addSubUndoAction(EditorAction *action)
{
	m_subUndoActions.append(action);
}

/**
 * @brief EditorAction::addSubRedoAction
 * @param action
 */

void EditorAction::addSubRedoAction(EditorAction *action)
{
	m_subRedoActions.append(action);
}

void *EditorAction::data() const
{
	return m_data;
}

void EditorAction::setData(void *newData)
{
	m_data = newData;
}


const QString &EditorAction::description() const
{
	return m_description;
}

void EditorAction::setDescription(const QString &newDescription)
{
	m_description = newDescription;
}

bool EditorAction::canUndo() const
{
	return m_canUndo;
}

bool EditorAction::canRedo() const
{
	return m_canRedo;
}

void EditorAction::setUndoFunc(const std::function<void (EditorAction *)> &newUndoFunc)
{
	m_undoFunc = newUndoFunc;
}

void EditorAction::setRedoFunc(const std::function<void (EditorAction *)> &newRedoFunc)
{
	m_redoFunc = newRedoFunc;
}




/**
 * @brief EditorAction::undo
 * @param parent
 */

void EditorAction::undo(EditorAction *parent)
{
	if (!m_canUndo) {
		qWarning() << "Can't undo" << m_description;
		return;
	}

	qDebug() << "UNDO" << this << "PARENT" << parent;

	if (m_undoFunc != nullptr) {
		qDebug() << "CALL UNDO FUNC" << this;
		m_undoFunc(parent);
	}

	foreach (EditorAction *action, m_subUndoActions) {
		action->undo(this);
	}

	m_canUndo = false;
	m_canRedo = true;
}


/**
 * @brief EditorAction::redo
 * @param parent
 */

void EditorAction::redo(EditorAction *parent)
{
	if (!m_canRedo) {
		qWarning() << "Can't redo" << m_description;
		return;
	}

	qDebug() << "REDO" << this << "PARENT" <<  parent << m_data;

	if (m_redoFunc != nullptr) {
		qDebug() << "CALL REDO FUNC" << this;
		m_redoFunc(parent);
	}

	foreach (EditorAction *action, m_subRedoActions) {
		action->redo(this);
	}

	m_canRedo = false;
	m_canUndo = true;
}


