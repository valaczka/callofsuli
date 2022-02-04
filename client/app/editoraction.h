/*
 * ---- Call of Suli ----
 *
 * editoraction.h
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

#ifndef EDITORACTION_H
#define EDITORACTION_H

#include <QList>


class EditorAction
{

public:
	EditorAction(void *data = nullptr);
	EditorAction(const std::function<void(EditorAction *)> &undoFunc, void *data = nullptr);
	EditorAction(const std::function<void(EditorAction *)> &undoFunc,
				 const std::function<void(EditorAction *)> &redoFunc,
				 void *data = nullptr);
	virtual ~EditorAction();

	const QString &description() const;
	void setDescription(const QString &newDescription);

	void setUndoFunc(const std::function<void(EditorAction *)> &newUndoFunc);
	void setRedoFunc(const std::function<void(EditorAction *)> &newRedoFunc);

	void addSubUndoAction(EditorAction *action);
	void addSubRedoAction(EditorAction *action);

	void *data() const;
	void setData(void *newData);

	void undo(EditorAction *parent = nullptr);
	void redo(EditorAction *parent = nullptr);


	bool operator==(const EditorAction &other) const;

	bool canUndo() const;
	bool canRedo() const;

protected:
	std::function<void(EditorAction *)> m_undoFunc;
	std::function<void(EditorAction *)> m_redoFunc;

private:
	QString m_description;
	QList<EditorAction*> m_subUndoActions;
	QList<EditorAction*> m_subRedoActions;
	void *m_data;
	bool m_canUndo;
	bool m_canRedo;
};


/**
 * @brief EditorAction::operator ==
 * @param other
 * @return
 */

inline bool EditorAction::operator==(const EditorAction &other) const
{
	return m_subUndoActions == other.m_subUndoActions && m_subRedoActions == other.m_subRedoActions
			&& m_description == other.m_description;
}


#endif // EDITORACTION_H
