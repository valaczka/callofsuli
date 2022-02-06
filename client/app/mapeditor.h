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
#include "mapeditoraction.h"

class MapEditor : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(GameMapEditor* editor READ editor WRITE setEditor NOTIFY editorChanged)
	Q_PROPERTY(EditorUndoStack *undoStack READ undoStack WRITE setUndoStack NOTIFY undoStackChanged)
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)

public:
	explicit MapEditor(QQuickItem *parent = nullptr);

	GameMapEditor *editor() const;
	void setEditor(GameMapEditor *newEditor);

	EditorUndoStack *undoStack() const;
	void setUndoStack(EditorUndoStack *newUndoStack);

	const QUrl &url() const;
	void setUrl(const QUrl &newUrl);

	const QString &displayName() const;
	void setDisplayName(const QString &newDisplayName);


public slots:
	void open(const QUrl &url);
	void create();
	void close();

	void chapterAdd(QVariantMap data);
	void chapterRemove(GameMapEditorChapter *chapter);
	void chapterRemoveList(const QList<GameMapEditorChapter*> &list);
	void chapterModify(GameMapEditorChapter *chapter, const QVariantMap &data);
	bool chapterModelUnselectObjectives(ObjectGenericListModel<GameMapEditorChapter> *model);

	void objectiveAdd(GameMapEditorChapter *chapter, QVariantMap data);
	void objectiveRemove(GameMapEditorChapter *chapter, GameMapEditorObjective *objective);
	void objectiveRemoveList(GameMapEditorChapter *chapter, const QList<GameMapEditorObjective*> &list);
	void objectiveModify(GameMapEditorObjective *objective, const QVariantMap &data);

private slots:
	void onUndoRedoCompleted(const int &lastStep);

signals:
	void actionContextUpdated(const MapEditorAction::MapEditorActionType &type, const QVariant &contextId);
	void editorChanged();
	void undoStackChanged();
	void urlChanged();
	void displayNameChanged();

private:
	GameMapEditor *m_editor;
	EditorUndoStack *m_undoStack;
	QUrl m_url;
	QString m_displayName;

};

#endif // MAPEDITOR_H
