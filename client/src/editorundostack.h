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

#include "mapeditormap.h"
#include <QMap>
#include <QVariant>
#include <QList>
#include <QPointer>

class EditorAction;


/**
 * @brief The EditorUndoStack class
 */

class EditorUndoStack : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QList<EditorAction*> actions READ actions NOTIFY actionsChanged)
	Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
	Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
	Q_PROPERTY(int size READ size NOTIFY sizeChanged)
	Q_PROPERTY(int step READ step NOTIFY stepChanged)
	Q_PROPERTY(int savedStep READ savedStep WRITE setSavedStep NOTIFY savedStepChanged)
	Q_PROPERTY(QString undoText READ undoText NOTIFY undoTextChanged)
	Q_PROPERTY(QString redoText READ redoText NOTIFY redoTextChanged)
	Q_PROPERTY(int maximumAction READ maximumAction WRITE setMaximumAction NOTIFY maximumActionChanged)

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

	const QString &undoText() const;
	void setUndoText(const QString &newUndoText);

	const QString &redoText() const;
	void setRedoText(const QString &newRedoText);

	int maximumAction() const;
	void setMaximumAction(int newMaximumAction);

public slots:
	EditorAction *call(EditorAction *action);
	bool undo(const int &steps = 1);
	bool redo(const int &steps = 1);
	void clear();

private slots:
	void onStepChanged();

signals:
	void undoCompleted(const int &lastStep);
	void redoCompleted(const int &lastStep);
	void callCompleted();

	void actionsChanged();
	void sizeChanged();
	void canUndoChanged();
	void canRedoChanged();
	void stepChanged();
	void savedStepChanged();
	void undoTextChanged();
	void redoTextChanged();
	void maximumActionChanged();

protected:
	QList<EditorAction *> m_actions;
	int m_step;
	int m_savedStep;

private:
	QString m_undoText;
	QString m_redoText;
	int m_maximumAction = 100;
};




/**
 * @brief The EditorAction class
 */

class EditorAction
{

public:
	EditorAction(MapEditorMap *map = nullptr, const QString &description = QString());
	EditorAction(const std::function<void(void)> &undoFunc, MapEditorMap *map = nullptr, const QString &description = QString());
	EditorAction(const std::function<void(void)> &undoFunc, const std::function<void(void)> &redoFunc,
				 MapEditorMap *map = nullptr, const QString &description = QString());
	EditorAction(const std::function<void(void)> &undoFunc, const std::function<void(void)> &redoFunc,
				 const QVariantMap &data, MapEditorMap *map = nullptr, const QString &description = QString());

	virtual ~EditorAction();

	void undo();
	void redo();

	const std::function<void ()> &undoFunc() const;
	void setUndoFunc(const std::function<void ()> &newUndoFunc);

	const std::function<void ()> &redoFunc() const;
	void setRedoFunc(const std::function<void ()> &newRedoFunc);

	const QString &description() const;
	void setDescription(const QString &newDescription);

	bool canUndo() const;
	bool canRedo() const;

	const QVariantMap &data() const;
	void setData(const QVariantMap &newData);

	MapEditorMap *map() const;
	void setMap(MapEditorMap *newMap);

protected:
	std::function<void(void)> m_undoFunc;
	std::function<void(void)> m_redoFunc;
	QString m_description;
	QVariantMap m_data;
	QPointer<MapEditorMap> m_map;

private:
	bool m_canUndo = false;
	bool m_canRedo = true;
};

#endif // EDITORUNDOSTACK_H
