/*
 * ---- Call of Suli ----
 *
 * basemap.cpp
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * BaseMap
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

#include "basemap.h"
#include "qjsonobject.h"

BaseMap::BaseMap(QObject *parent)
	: SelectableObject{parent}
{

}


/**
 * @brief BaseMap::loadFromJson
 * @param object
 * @param allField
 */

void BaseMap::loadFromJson(const QJsonObject &object, const bool &allField)
{
	if (object.contains(QStringLiteral("uuid")) || allField)
		setUuid(object.value(QStringLiteral("uuid")).toString());

	if (object.contains(QStringLiteral("name")) || allField)
		setName(object.value(QStringLiteral("name")).toString());

	if (object.contains(QStringLiteral("size")) || allField)
		setSize(object.value(QStringLiteral("size")).toInt());

	if (object.contains(QStringLiteral("md5")) || allField)
		setMd5(object.value(QStringLiteral("md5")).toString());
}




bool BaseMap::downloaded() const
{
	return m_downloaded;
}

void BaseMap::setDownloaded(bool newDownloaded)
{
	if (m_downloaded == newDownloaded)
		return;
	m_downloaded = newDownloaded;
	emit downloadedChanged();
}

qreal BaseMap::downloadProgress() const
{
	return m_downloadProgress;
}

void BaseMap::setDownloadProgress(qreal newDownloadProgress)
{
	if (qFuzzyCompare(m_downloadProgress, newDownloadProgress))
		return;
	m_downloadProgress = newDownloadProgress;
	emit downloadProgressChanged();
}

const QString &BaseMap::uuid() const
{
	return m_uuid;
}

void BaseMap::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &BaseMap::md5() const
{
	return m_md5;
}

void BaseMap::setMd5(const QString &newMd5)
{
	if (m_md5 == newMd5)
		return;
	m_md5 = newMd5;
	emit md5Changed();
}

int BaseMap::size() const
{
	return m_size;
}

void BaseMap::setSize(int newSize)
{
	if (m_size == newSize)
		return;
	m_size = newSize;
	emit sizeChanged();
}

const QString &BaseMap::name() const
{
	return m_name;
}

void BaseMap::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}
