/*
 * ---- Call of Suli ----
 *
 * gamemapstorage.cpp
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapStorage
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

#include "gamemapnew.h"
#include "gamemapstorage.h"

GameMapStorage::GameMapStorage(QObject *parent)
	: QObject{parent}
	, m_id(0)
	, m_module()
	, m_data()
	, m_gameMap(nullptr)
{

}


/**
 * @brief GameMapStorage::~GameMapStorage
 */

GameMapStorage::~GameMapStorage()
{
	if (m_gameMap)
		m_gameMap->storageRemove(this);
}


/**
 * @brief GameMapStorage::toJsonObject
 * @return
 */

QJsonObject GameMapStorage::toJsonObject() const
{
	QJsonObject obj;

	obj["id"] = m_id;
	obj["module"] = m_module;
	obj["data"] = QJsonObject::fromVariantMap(m_data);

	return obj;
}


qint32 GameMapStorage::id() const
{
	return m_id;
}

void GameMapStorage::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}

const QString &GameMapStorage::module() const
{
	return m_module;
}

void GameMapStorage::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

const QVariantMap &GameMapStorage::data() const
{
	return m_data;
}

void GameMapStorage::setData(const QVariantMap &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}

GameMapNew *GameMapStorage::gameMap() const
{
	return m_gameMap;
}

void GameMapStorage::setGameMap(GameMapNew *newGameMap)
{
	if (m_gameMap == newGameMap)
		return;
	m_gameMap = newGameMap;
	emit gameMapChanged();
}
