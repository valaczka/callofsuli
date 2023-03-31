/*
 * ---- Call of Suli ----
 *
 * teachermap.h
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMap
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

#ifndef TEACHERMAP_H
#define TEACHERMAP_H

#include <QObject>
#include "QOlm/QOlm.hpp"
#include "qjsonobject.h"
#include "basemap.h"

class TeacherMap;
using TeacherMapList = qolm::QOlm<TeacherMap>;
Q_DECLARE_METATYPE(TeacherMapList*)

/**
 * @brief The TeacherMap class
 */

class TeacherMap : public BaseMap
{
	Q_OBJECT

	Q_PROPERTY(int version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(int draftVersion READ draftVersion WRITE setDraftVersion NOTIFY draftVersionChanged)
	Q_PROPERTY(QJsonObject cache READ cache WRITE setCache NOTIFY cacheChanged)
	Q_PROPERTY(QDateTime lastModified READ lastModified WRITE setLastModified NOTIFY lastModifiedChanged)
	Q_PROPERTY(QString lastEditor READ lastEditor WRITE setLastEditor NOTIFY lastEditorChanged)

public:
	explicit TeacherMap(QObject *parent = nullptr);

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int version() const;
	void setVersion(int newVersion);

	int draftVersion() const;
	void setDraftVersion(int newDraftVersion);

	const QJsonObject &cache() const;
	void setCache(const QJsonObject &newCache);

	const QDateTime &lastModified() const;
	void setLastModified(const QDateTime &newLastModified);

	const QString &lastEditor() const;
	void setLastEditor(const QString &newLastEditor);

signals:
	void versionChanged();
	void draftVersionChanged();
	void cacheChanged();
	void lastModifiedChanged();
	void lastEditorChanged();

private:
	int m_version = 0;
	int m_draftVersion = -2;
	QJsonObject m_cache;
	QDateTime m_lastModified;
	QString m_lastEditor;
};

#endif // TEACHERMAP_H
