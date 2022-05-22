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
#include <QObject>


class EditorAction : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
	Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)

public:
	EditorAction(QObject *parent = nullptr);
	EditorAction(const std::function<void(void)> &undoFunc,
				 QObject *parent = nullptr);
	EditorAction(const std::function<void(void)> &undoFunc,
				 const std::function<void(void)> &redoFunc,
				 QObject *parent = nullptr);
	virtual ~EditorAction();

	void setUndoFunc(const std::function<void(void)> &newUndoFunc);
	void setRedoFunc(const std::function<void(void)> &newRedoFunc);

	void undo();
	void redo();

	const QString &description() const;
	void setDescription(const QString &newDescription);

	bool canUndo() const;
	void setCanUndo(bool newCanUndo);

	bool canRedo() const;
	void setCanRedo(bool newCanRedo);

signals:
	void descriptionChanged();
	void canUndoChanged();
	void canRedoChanged();

protected:
	std::function<void(void)> m_undoFunc;
	std::function<void(void)> m_redoFunc;

private:
	QString m_description;
	bool m_canUndo;
	bool m_canRedo;
};




#endif // EDITORACTION_H
