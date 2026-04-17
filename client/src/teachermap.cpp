/*
 * ---- Call of Suli ----
 *
 * teachermap.cpp
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

#include <QJsonArray>
#include "teachermap.h"


TeacherMap::TeacherMap(QObject *parent)
	: BaseMap{parent}
{

}


/**
 * @brief TeacherMap::loadFromJson
 * @param object
 * @param allField
 */

void TeacherMap::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("version")) || allField)
		setVersion(object.value(QStringLiteral("version")).toInt());

	if (object.contains(QStringLiteral("draftVersion")) || allField)
		setDraftVersion(object.value(QStringLiteral("draftVersion")).toInt());

	if (object.contains(QStringLiteral("lastModified")) || allField)
		setLastModified(QDateTime::fromSecsSinceEpoch(object.value(QStringLiteral("lastModified")).toInteger()));

	if (object.contains(QStringLiteral("lastEditor")) || allField)
		setLastEditor(object.value(QStringLiteral("lastEditor")).toString());

	if (object.contains(QStringLiteral("tags")) || allField)
		setTags(object.value(QStringLiteral("tags")).toArray().toVariantList());

	BaseMap::loadFromJson(object, allField);
}






int TeacherMap::version() const
{
	return m_version;
}

void TeacherMap::setVersion(int newVersion)
{
	if (m_version == newVersion)
		return;
	m_version = newVersion;
	emit versionChanged();
}

int TeacherMap::draftVersion() const
{
	return m_draftVersion;
}

void TeacherMap::setDraftVersion(int newDraftVersion)
{
	if (m_draftVersion == newDraftVersion)
		return;
	m_draftVersion = newDraftVersion;
	emit draftVersionChanged();
}


const QDateTime &TeacherMap::lastModified() const
{
	return m_lastModified;
}

void TeacherMap::setLastModified(const QDateTime &newLastModified)
{
	if (m_lastModified == newLastModified)
		return;
	m_lastModified = newLastModified;
	emit lastModifiedChanged();
}

const QString &TeacherMap::lastEditor() const
{
	return m_lastEditor;
}

void TeacherMap::setLastEditor(const QString &newLastEditor)
{
	if (m_lastEditor == newLastEditor)
		return;
	m_lastEditor = newLastEditor;
	emit lastEditorChanged();
}



/**
 * @brief TeacherMapTag::TeacherMapTag
 * @param parent
 */

TeacherMapTag::TeacherMapTag(QObject *parent)
	: SelectableObject{parent}
{

}

void TeacherMapTag::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("id")) || allField)
		setTagId(object.value(QStringLiteral("id")).toInt());

	if (object.contains(QStringLiteral("tag")) || allField)
		setName(object.value(QStringLiteral("tag")).toString());

	if (object.contains(QStringLiteral("parent")) || allField)
		setParentId(std::max(0, object.value(QStringLiteral("parent")).toInt()));
}


/**
 * @brief TeacherMapTag::tagId
 * @return
 */

int TeacherMapTag::tagId() const
{
	return m_tagId;
}

void TeacherMapTag::setTagId(int newTagId)
{
	if (m_tagId == newTagId)
		return;
	m_tagId = newTagId;
	emit tagIdChanged();
}

QString TeacherMapTag::name() const
{
	return m_name;
}

void TeacherMapTag::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}


int TeacherMapTag::parentId() const
{
	return m_parentId;
}

void TeacherMapTag::setParentId(int newParentId)
{
	if (m_parentId == newParentId)
		return;
	m_parentId = newParentId;
	emit parentIdChanged();
}

QVariantList TeacherMap::tags() const
{
	return m_tags;
}

void TeacherMap::setTags(const QVariantList &newTags)
{
	if (m_tags == newTags)
		return;
	m_tags = newTags;
	emit tagsChanged();
}
