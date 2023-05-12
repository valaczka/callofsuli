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
#include "mapplay.h"


class MapPlayEditor;

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

	Q_INVOKABLE QVariantMap objectiveInfo(MapEditorObjective *objective) const;
	Q_INVOKABLE QVariantMap storageInfo(MapEditorStorage *storage) const;
	Q_INVOKABLE QVariantMap inventoryInfo(MapEditorInventory *inventory) const;


	Q_INVOKABLE MapEditorMission* missionAdd(const QString &name = QString());
	Q_INVOKABLE void missionRemove(MapEditorMission *mission);
	Q_INVOKABLE void missionModify(MapEditorMission *mission, QJSValue modifyFunc);
	Q_INVOKABLE void missionLockAdd(MapEditorMission *mission, MapEditorMissionLevel *lock);
	Q_INVOKABLE void missionLockAdd(MapEditorMission *mission, const QString &uuid, const int &level);
	Q_INVOKABLE void missionLockRemove(MapEditorMission *mission, MapEditorMissionLevel *lock);

	Q_INVOKABLE MapEditorMissionLevel* missionLevelAdd(MapEditorMission *mission);
	Q_INVOKABLE void missionLevelRemove(MapEditorMissionLevel *missionLevel);
	Q_INVOKABLE void missionLevelModify(MapEditorMissionLevel *missionLevel, QJSValue modifyFunc);
	Q_INVOKABLE void missionLevelChapterAdd(MapEditorMissionLevel *missionLevel, MapEditorChapter *chapter) {
		missionLevelChapterAdd(missionLevel, QList<MapEditorChapter *>{chapter});
	}
	Q_INVOKABLE void missionLevelChapterAdd(MapEditorMissionLevel *missionLevel, const QList<MapEditorChapter *> &chapterList);
	Q_INVOKABLE void missionLevelChapterRemove(MapEditorMissionLevel *missionLevel, MapEditorChapter *chapter) {
		missionLevelChapterRemove(missionLevel, QList<MapEditorChapter *>{chapter});
	}
	Q_INVOKABLE void missionLevelChapterRemove(MapEditorMissionLevel *missionLevel, const QList<MapEditorChapter *> &chapterList);

	Q_INVOKABLE void missionLevelPlay(MapEditorMissionLevel *missionLevel, int mode);

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
	MapPlayEditor *m_mapPlay = nullptr;
};




/**
 * @brief The MapPlayEditor class
 */


class MapPlayEditor : public MapPlay
{
	Q_OBJECT

public:
	explicit MapPlayEditor(Client *client, QObject *parent = nullptr)
		: MapPlay(client, parent)
	{
		m_online = false;
	}

	virtual ~MapPlayEditor() {}


	bool reloadMap(MapEditorMap *map);
	bool play(MapEditorMissionLevel *missionLevel, const GameMap::GameMode &mode);


protected:
	virtual void onCurrentGamePrepared() override;
	virtual void onCurrentGameFinished() override;

};


#endif // MAPEDITOR_H