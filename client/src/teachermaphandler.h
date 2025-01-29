/*
 * ---- Call of Suli ----
 *
 * teachermaphandler.h
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMapHandler
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

#ifndef TEACHERMAPHANDLER_H
#define TEACHERMAPHANDLER_H

#include "basemaphandler.h"
#include "teachermap.h"
#include "mapeditor.h"
#include <QObject>

class TeacherMapEditor;

/**
 * @brief The TeacherMapHandler class
 */

class TeacherMapHandler : public BaseMapHandler
{
	Q_OBJECT

	Q_PROPERTY(TeacherMapList *mapList READ mapList CONSTANT)
	Q_PROPERTY(TeacherMapEditor *mapEditor READ mapEditor WRITE setMapEditor NOTIFY mapEditorChanged)

public:
	explicit TeacherMapHandler(QObject *parent = nullptr);
	virtual ~TeacherMapHandler();

	Q_INVOKABLE void mapCreate(const QString &name);
	Q_INVOKABLE void mapImport(const QUrl &file);
	Q_INVOKABLE void mapDownload(TeacherMap *map);
	Q_INVOKABLE void mapEdit(TeacherMap *map);
	Q_INVOKABLE void checkDownloads();

	Q_INVOKABLE bool mapExport(const QUrl &file, const QList<TeacherMap*> &list) const;

	TeacherMapList *mapList() const;

	TeacherMapEditor *mapEditor() const;
	void setMapEditor(TeacherMapEditor *newMapEditor);

	Q_INVOKABLE TeacherMap *findMap(const QString &uuid) const;


#ifdef Q_OS_WASM
	Q_INVOKABLE void mapImportWasm();
#endif


signals:
	void mapEditorChanged();

private slots:
	void unsetMapEditor();

protected:
	void reloadList() override;

private:
	void loadEditorPage();
	void _mapImportContent(const QString &name, const QByteArray &content);

private:
	TeacherMapList *const m_mapList;
	TeacherMapEditor *m_mapEditor = nullptr;

};





/**
 * @brief The TeacherMapEditor class
 */

class TeacherMapEditor : public MapEditor
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(int draftVersion READ draftVersion WRITE setDraftVersion NOTIFY draftVersionChanged)

public:
	TeacherMapEditor(QObject *parent = nullptr);
	virtual ~TeacherMapEditor();

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	int draftVersion() const;
	void setDraftVersion(int newDraftVersion);

signals:
	void uuidChanged();
	void draftVersionChanged();

private slots:
	void onSaveRequest();

private:
	int m_draftVersion = 0;
	QString m_uuid;

};

#endif // TEACHERMAPHANDLER_H
