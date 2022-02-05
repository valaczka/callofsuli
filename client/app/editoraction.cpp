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

EditorAction::EditorAction(QObject *parent)
	: QObject(parent)
	, m_undoFunc(nullptr)
	, m_redoFunc(nullptr)
	, m_canUndo(false)
	, m_canRedo(true)
{

}


/**
 * @brief EditorAction::EditorAction
 * @param undoFunc
 * @param data
 */

EditorAction::EditorAction(const std::function<void ()> &undoFunc, QObject *parent)
	: QObject(parent)
	, m_undoFunc(undoFunc)
	, m_redoFunc(nullptr)
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

EditorAction::EditorAction(const std::function<void ()> &undoFunc, const std::function<void ()> &redoFunc, QObject *parent)
	: QObject(parent)
	, m_undoFunc(undoFunc)
	, m_redoFunc(redoFunc)
	, m_canUndo(false)
	, m_canRedo(true)
{

}


EditorAction::~EditorAction()
{

}


const QString &EditorAction::description() const
{
	return m_description;
}

void EditorAction::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

bool EditorAction::canUndo() const
{
	return m_canUndo;
}

void EditorAction::setCanUndo(bool newCanUndo)
{
	if (m_canUndo == newCanUndo)
		return;
	m_canUndo = newCanUndo;
	emit canUndoChanged();
}

bool EditorAction::canRedo() const
{
	return m_canRedo;
}

void EditorAction::setCanRedo(bool newCanRedo)
{
	if (m_canRedo == newCanRedo)
		return;
	m_canRedo = newCanRedo;
	emit canRedoChanged();
}




void EditorAction::setUndoFunc(const std::function<void ()> &newUndoFunc)
{
	m_undoFunc = newUndoFunc;
}

void EditorAction::setRedoFunc(const std::function<void ()> &newRedoFunc)
{
	m_redoFunc = newRedoFunc;
}




/**
 * @brief EditorAction::undo
 * @param parent
 */

void EditorAction::undo()
{
	if (!m_canUndo) {
		qWarning() << "Can't undo" << m_description;
		return;
	}

	qDebug() << "UNDO" << this;

	if (m_undoFunc != nullptr) {
		qDebug() << "CALL UNDO FUNC" << this;
		m_undoFunc();
	}

	setCanUndo(false);
	setCanRedo(true);
}


/**
 * @brief EditorAction::redo
 * @param parent
 */

void EditorAction::redo()
{
	if (!m_canRedo) {
		qWarning() << "Can't redo" << m_description;
		return;
	}

	qDebug() << "REDO" << this;

	if (m_redoFunc != nullptr) {
		qDebug() << "CALL REDO FUNC" << this;
		m_redoFunc();
	}

	setCanUndo(true);
	setCanRedo(false);
}


