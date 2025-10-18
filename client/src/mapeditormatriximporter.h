/*
 * ---- Call of Suli ----
 *
 * mapeditormatriximporter.h
 *
 * Created on: 2025. 10. 18.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditorMatrixImporter
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

#ifndef MAPEDITORMATRIXIMPORTER_H
#define MAPEDITORMATRIXIMPORTER_H

#include <QObject>
#include "mapeditor.h"

class MapEditorMatrixImporter : public QObject
{
	Q_OBJECT

	Q_PROPERTY(MapEditor* mapEditor READ mapEditor WRITE setMapEditor NOTIFY mapEditorChanged FINAL)
	Q_PROPERTY(bool downloaded READ downloaded WRITE setDownloaded NOTIFY downloadedChanged FINAL)

public:
	explicit MapEditorMatrixImporter(QObject *parent = nullptr);

	MapEditor *mapEditor() const;
	void setMapEditor(MapEditor *newMapEditor);

	Q_INVOKABLE void downloadTemplate(const QUrl &file);
	Q_INVOKABLE void upload(const QUrl &file);

	bool downloaded() const;
	void setDownloaded(bool newDownloaded);

signals:
	void imported();
	void mapEditorChanged();
	void downloadedChanged();

private:
	QByteArray createContent() const;
	bool loadContent(const QByteArray &data);

	MapEditor *m_mapEditor = nullptr;
	const QString m_hash;
	bool m_downloaded = false;
};

#endif // MAPEDITORMATRIXIMPORTER_H
