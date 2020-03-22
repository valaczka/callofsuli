/*
 * ---- Call of Suli ----
 *
 * mapobject.cpp
 *
 * Created on: 2021. 12. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapObject
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

#include "maplistobject.h"

MapListObject::MapListObject(QObject *parent)
	: ObjectListModelObject{parent}
	, m_active(false)
	, m_dataSize(0)
	, m_downloaded(false)
	, m_version(0)
	, m_binded()
	, m_used(false)
	, m_lastModified()
{

}

const QString &MapListObject::uuid() const
{
	return m_uuid;
}

void MapListObject::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &MapListObject::name() const
{
	return m_name;
}

void MapListObject::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

bool MapListObject::active() const
{
	return m_active;
}

void MapListObject::setActive(bool newActive)
{
	if (m_active == newActive)
		return;
	m_active = newActive;
	emit activeChanged();
}

int MapListObject::dataSize() const
{
	return m_dataSize;
}

void MapListObject::setDataSize(int newDataSize)
{
	if (m_dataSize == newDataSize)
		return;
	m_dataSize = newDataSize;
	emit dataSizeChanged();
}

bool MapListObject::downloaded() const
{
	return m_downloaded;
}

void MapListObject::setDownloaded(bool newDownloaded)
{
	if (m_downloaded == newDownloaded)
		return;
	m_downloaded = newDownloaded;
	emit downloadedChanged();
}

const QString &MapListObject::md5() const
{
	return m_md5;
}

void MapListObject::setMd5(const QString &newMd5)
{
	if (m_md5 == newMd5)
		return;
	m_md5 = newMd5;
	emit md5Changed();
}

int MapListObject::version() const
{
	return m_version;
}

void MapListObject::setVersion(int newVersion)
{
	if (m_version == newVersion)
		return;
	m_version = newVersion;
	emit versionChanged();
}

const QVariantList &MapListObject::binded() const
{
	return m_binded;
}

void MapListObject::setBinded(const QVariantList &newBinded)
{
	if (m_binded == newBinded)
		return;
	m_binded = newBinded;
	emit bindedChanged();
}

bool MapListObject::used() const
{
	return m_used;
}

void MapListObject::setUsed(bool newUsed)
{
	if (m_used == newUsed)
		return;
	m_used = newUsed;
	emit usedChanged();
}

const QDateTime &MapListObject::lastModified() const
{
	return m_lastModified;
}

void MapListObject::setLastModified(const QDateTime &newLastModified)
{
	if (m_lastModified == newLastModified)
		return;
	m_lastModified = newLastModified;
	emit lastModifiedChanged();
}
