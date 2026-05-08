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
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "qjsonobject.h"
#include "basemap.h"

class TeacherMap;
using TeacherMapList = qolm::QOlm<TeacherMap>;
Q_DECLARE_METATYPE(TeacherMapList*)


class TeacherMapTag;
using TeacherMapTagList = qolm::QOlm<TeacherMapTag>;
Q_DECLARE_METATYPE(TeacherMapTagList*)




/**
 * @brief The TeacherMapTag class
 */

class TeacherMapTag : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(int tagId READ tagId WRITE setTagId NOTIFY tagIdChanged FINAL)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
	Q_PROPERTY(int parentId READ parentId WRITE setParentId NOTIFY parentIdChanged FINAL)

public:
	explicit TeacherMapTag(QObject *parent = nullptr);

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int tagId() const;
	void setTagId(int newTagId);

	QString name() const;
	void setName(const QString &newName);

	int parentId() const;
	void setParentId(int newParentId);

signals:
	void tagIdChanged();
	void nameChanged();
	void parentIdChanged();

private:
	int m_tagId;
	QString m_name;
	int m_parentId = -1;
};




/**
 * @brief The TeacherMap class
 */

class TeacherMap : public BaseMap
{
	Q_OBJECT

	Q_PROPERTY(int version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(int draftVersion READ draftVersion WRITE setDraftVersion NOTIFY draftVersionChanged)
	Q_PROPERTY(QDateTime lastModified READ lastModified WRITE setLastModified NOTIFY lastModifiedChanged)
	Q_PROPERTY(QString lastEditor READ lastEditor WRITE setLastEditor NOTIFY lastEditorChanged)
	Q_PROPERTY(QVariantList tags READ tags WRITE setTags NOTIFY tagsChanged FINAL)

public:
	explicit TeacherMap(QObject *parent = nullptr);

	void loadFromJson(const QJsonObject &object, const bool &allField = true);

	int version() const;
	void setVersion(int newVersion);

	int draftVersion() const;
	void setDraftVersion(int newDraftVersion);

	const QDateTime &lastModified() const;
	void setLastModified(const QDateTime &newLastModified);

	const QString &lastEditor() const;
	void setLastEditor(const QString &newLastEditor);

	QVariantList tags() const;
	void setTags(const QVariantList &newTags);

signals:
	void versionChanged();
	void draftVersionChanged();
	void lastModifiedChanged();
	void lastEditorChanged();
	void tagsChanged();

private:
	int m_version = 0;
	int m_draftVersion = -2;
	QDateTime m_lastModified;
	QString m_lastEditor;
	QVariantList m_tags;
};

#endif // TEACHERMAP_H
