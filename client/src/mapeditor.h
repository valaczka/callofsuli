/*
 * ---- Call of Suli ----
 *
 * mapeditor.h
 *
 * Created on: 2023. 05. 06.
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
#include <QTimer>
#include "client.h"
#include "mapeditormap.h"
#include "editorundostack.h"


/**
 * @brief The MapEditor class
 */

class MapEditor : public QObject
{
	Q_OBJECT

	Q_PROPERTY(MapEditorMap *map READ map WRITE setMap NOTIFY mapChanged)
	Q_PROPERTY(EditorUndoStack *undoStack READ undoStack CONSTANT)
	Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
	Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged)
	Q_PROPERTY(bool autoSaved READ autoSaved WRITE setAutoSaved NOTIFY autoSavedChanged)

public:
	explicit MapEditor(QObject *parent = nullptr);
	virtual ~MapEditor();

	MapEditorMap *map() const;
	void setMap(MapEditorMap *newMap);


	Q_INVOKABLE bool loadMapTest();

	EditorUndoStack *undoStack() const;

	const QString &displayName() const;
	void setDisplayName(const QString &newDisplayName);


	Q_INVOKABLE MapEditorMission* missionAdd(const QString &name = QString());
	Q_INVOKABLE void missionRemove(MapEditorMission *mission);
	Q_INVOKABLE void missionModify(MapEditorMission *mission, QJSValue modifyFunc);

	Q_INVOKABLE MapEditorMissionLevel* missionLevelAdd(MapEditorMission *mission);

	bool modified() const;
	void setModified(bool newModified);

	bool autoSaved() const;
	void setAutoSaved(bool newAutoSaved);

public slots:
	virtual void save();
	virtual void saveAuto();

protected:
	bool loadFromBinaryData(const QByteArray &data);
	void unloadMap();

protected slots:
	void onModified();

signals:
	void mapChanged();
	void displayNameChanged();
	void modifiedChanged();
	void autoSavedChanged();

private:
	Client *const m_client;
	MapEditorMap *m_map = nullptr;
	EditorUndoStack *const m_undoStack;
	QTimer m_saveTimer;
	QString m_displayName;
	bool m_modified = false;
	bool m_autoSaved = false;
};



#endif // MAPEDITOR_H
