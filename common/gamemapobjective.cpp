/*
 * ---- Call of Suli ----
 *
 * gamemapobjective.cpp
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapObjective
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

#include "gamemapobjective.h"
#include "gamemapchapter.h"

GameMapObjective::GameMapObjective(QObject *parent)
	: QObject{parent}
	, m_uuid()
	, m_module()
	, m_storage(nullptr)
	, m_storageCount(0)
	, m_data()
	, m_chapter(nullptr)
{

}


/**
 * @brief GameMapObjective::~GameMapObjective
 */

GameMapObjective::~GameMapObjective()
{
	if (m_chapter)
		m_chapter->objectiveRemove(this);
}


/**
 * @brief GameMapObjective::toJsonObject
 * @return
 */

QJsonObject GameMapObjective::toJsonObject() const
{
	QJsonObject obj;

	obj["uuid"] = m_uuid;
	obj["module"] = m_module;
	if (m_storage)
		obj["storage"] =  m_storage->id();
	obj["storageCount"] = m_storageCount;
	obj["data"] = QJsonObject::fromVariantMap(m_data);

	return obj;
}


/**
 * @brief GameMapObjective::uuid
 * @return
 */

const QString &GameMapObjective::uuid() const
{
	return m_uuid;
}

void GameMapObjective::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &GameMapObjective::module() const
{
	return m_module;
}

void GameMapObjective::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

GameMapStorage *GameMapObjective::storage() const
{
	return m_storage;
}

void GameMapObjective::setStorage(GameMapStorage *newStorage)
{
	if (m_storage == newStorage)
		return;
	m_storage = newStorage;
	emit storageChanged();
}

qint32 GameMapObjective::storageCount() const
{
	return m_storageCount;
}

void GameMapObjective::setStorageCount(qint32 newStorageCount)
{
	if (m_storageCount == newStorageCount)
		return;
	m_storageCount = newStorageCount;
	emit storageCountChanged();
}

const QVariantMap &GameMapObjective::data() const
{
	return m_data;
}

void GameMapObjective::setData(const QVariantMap &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}

GameMapChapter *GameMapObjective::chapter() const
{
	return m_chapter;
}

void GameMapObjective::setChapter(GameMapChapter *newChapter)
{
	if (m_chapter == newChapter)
		return;
	m_chapter = newChapter;
	emit chapterChanged();
}
