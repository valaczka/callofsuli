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

#include "teachermap.h"
#include "application.h"

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
		setLastModified(QDateTime::fromSecsSinceEpoch(JSON_TO_INTEGER(object.value(QStringLiteral("lastModified")))));

	if (object.contains(QStringLiteral("lastEditor")) || allField)
		setLastEditor(object.value(QStringLiteral("lastEditor")).toString());

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
