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
#include "Logger.h"

EditorUndoStack::EditorUndoStack(QObject *parent)
	: QObject{parent}
	, m_actions()
	, m_step(-1)
	, m_savedStep(-1)
{
	connect(this, &EditorUndoStack::stepChanged, this, &EditorUndoStack::onStepChanged);
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
	if (m_savedStep > m_step)
		m_savedStep = -1;

	while (m_step < m_actions.size()-1) {
		EditorAction *a = m_actions.takeLast();
		delete a;
	}

	if (!m_actions.isEmpty() && m_maximumAction > 0 && m_actions.size() >= m_maximumAction) {
		EditorAction *a = m_actions.takeFirst();
		delete a;
	}

	m_actions.append(action);
	action->redo();

	m_step = m_actions.size()-1;

	emit sizeChanged();
	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();
	emit callCompleted();

	return action;
}


/**
 * @brief EditorUndoStack::undo
 */

bool EditorUndoStack::undo(const int &steps)
{
	if (!canUndo())
		return false;

	int lastUndoStep = -1;

	for (int i=0; i<steps && m_step >= 0; ++i) {
		EditorAction *a = m_actions.at(m_step);

		a->undo();

		lastUndoStep = m_step--;
	}

	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();
	emit undoCompleted(lastUndoStep);

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

	int lastRedoStep = -1;

	for (int i=0; i<steps && m_step<m_actions.size()-1; ++i) {
		EditorAction *a = m_actions.at(m_step+1);

		a->redo();

		lastRedoStep = m_step+1;

		++m_step;
	}

	emit stepChanged();
	emit canUndoChanged();
	emit canRedoChanged();
	emit redoCompleted(lastRedoStep);

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


/**
 * @brief EditorUndoStack::onStepChanged
 */

void EditorUndoStack::onStepChanged()
{
	if (m_step>=0 && m_step<m_actions.size())
		setUndoText(m_actions.at(m_step)->description());
	else
		setUndoText("");

	if (m_step>=-1 && m_step<m_actions.size()-1)
		setRedoText(m_actions.at(m_step+1)->description());
	else
		setRedoText("");
}

int EditorUndoStack::maximumAction() const
{
	return m_maximumAction;
}

void EditorUndoStack::setMaximumAction(int newMaximumAction)
{
	if (m_maximumAction == newMaximumAction)
		return;
	m_maximumAction = newMaximumAction;
	emit maximumActionChanged();
}


const QString &EditorUndoStack::redoText() const
{
	return m_redoText;
}

void EditorUndoStack::setRedoText(const QString &newRedoText)
{
	if (m_redoText == newRedoText)
		return;
	m_redoText = newRedoText;
	emit redoTextChanged();
}

const QString &EditorUndoStack::undoText() const
{
	return m_undoText;
}

void EditorUndoStack::setUndoText(const QString &newUndoText)
{
	if (m_undoText == newUndoText)
		return;
	m_undoText = newUndoText;
	emit undoTextChanged();
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


/**
 * @brief EditorAction::undo
 */

EditorAction::EditorAction(MapEditorMap *map, const QString &description)
	: m_description(description), m_map(map)
{
	LOG_CTRACE("client") << "Editor action created" << this;
}


/**
 * @brief EditorAction::EditorAction
 * @param undoFunc
 * @param map
 * @param description
 */

EditorAction::EditorAction(const std::function<void ()> &undoFunc, MapEditorMap *map, const QString &description)
	: m_undoFunc(undoFunc), m_description(description), m_map(map)
{
	LOG_CTRACE("client") << "Editor action created" << this;
}



/**
 * @brief EditorAction::EditorAction
 * @param undoFunc
 * @param redoFunc
 * @param map
 * @param description
 */

EditorAction::EditorAction(const std::function<void ()> &undoFunc, const std::function<void ()> &redoFunc, MapEditorMap *map, const QString &description)
	: m_undoFunc(undoFunc), m_redoFunc(redoFunc), m_description(description), m_map(map)
{
	LOG_CTRACE("client") << "Editor action created" << this;
}



/**
 * @brief EditorAction::EditorAction
 * @param undoFunc
 * @param redoFunc
 * @param data
 * @param map
 * @param description
 */

EditorAction::EditorAction(const std::function<void ()> &undoFunc, const std::function<void ()> &redoFunc, const QVariantMap &data,
						   MapEditorMap *map, const QString &description)
	: m_undoFunc(undoFunc), m_redoFunc(redoFunc), m_description(description), m_data(data), m_map(map)
{
	LOG_CTRACE("client") << "Editor action created" << this;
}



/**
 * @brief EditorAction::~EditorAction
 */

EditorAction::~EditorAction()
{
	LOG_CTRACE("client") << "Editor action destroyed" << this;
}



/**
 * @brief EditorAction::undo
 */

void EditorAction::undo()
{
	if (!m_canUndo) {
		LOG_CWARNING("client") << "Can't undo:" << qPrintable(m_description);
		return;
	}

	if (m_undoFunc)
		m_undoFunc();

	m_canUndo = false;
	m_canRedo = true;
}


/**
 * @brief EditorAction::redo
 */

void EditorAction::redo()
{
	if (!m_canRedo) {
		LOG_CWARNING("client") << "Can't undo:" << qPrintable(m_description);
		return;
	}

	if (m_redoFunc)
		m_redoFunc();

	m_canUndo = true;
	m_canRedo = false;
}

const std::function<void ()> &EditorAction::undoFunc() const
{
	return m_undoFunc;
}

void EditorAction::setUndoFunc(const std::function<void ()> &newUndoFunc)
{
	m_undoFunc = newUndoFunc;
}

const std::function<void ()> &EditorAction::redoFunc() const
{
	return m_redoFunc;
}

void EditorAction::setRedoFunc(const std::function<void ()> &newRedoFunc)
{
	m_redoFunc = newRedoFunc;
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

const QVariantMap &EditorAction::data() const
{
	return m_data;
}

void EditorAction::setData(const QVariantMap &newData)
{
	m_data = newData;
}

MapEditorMap* EditorAction::map() const
{
	return m_map;
}

void EditorAction::setMap(MapEditorMap *newMap)
{
	m_map = newMap;
}
