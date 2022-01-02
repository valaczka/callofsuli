/*
 * ---- Call of Suli ----
 *
 * gamemapmission.cpp
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapMission
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

#include <QUuid>
#include "gamemap.h"
#include "gamemapmission.h"
#include "gamemapmissionlevel.h"

GameMapMission::GameMapMission(QObject *parent)
	: QObject{parent}
	, m_uuid(QUuid::createUuid().toString())
	, m_name()
	, m_description()
	, m_levels()
	, m_locks()
	, m_medalImage()
	, m_gameMap(nullptr)
{

}

GameMapMission::~GameMapMission()
{
	/*foreach (GameMapMissionLevel *s, m_levels)
		s->setMission(nullptr);*/

	qDeleteAll(m_levels.begin(), m_levels.end());
	m_levels.clear();

	foreach (GameMapMissionLock *s, m_locks)
		s->setParentMission(nullptr);

	qDeleteAll(m_locks.begin(), m_locks.end());
	m_locks.clear();

	//if (m_gameMap)
	//	m_gameMap->missionremove
}



const QString &GameMapMission::uuid() const
{
	return m_uuid;
}

void GameMapMission::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &GameMapMission::name() const
{
	return m_name;
}

void GameMapMission::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

const QString &GameMapMission::description() const
{
	return m_description;
}

void GameMapMission::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

const QList<GameMapMissionLevel *> &GameMapMission::levels() const
{
	return m_levels;
}

void GameMapMission::setLevels(const QList<GameMapMissionLevel *> &newLevels)
{
	if (m_levels == newLevels)
		return;
	m_levels = newLevels;
	emit levelsChanged();
}

const QList<GameMapMissionLock *> &GameMapMission::locks() const
{
	return m_locks;
}

void GameMapMission::setLocks(const QList<GameMapMissionLock *> &newLocks)
{
	if (m_locks == newLocks)
		return;
	m_locks = newLocks;
	emit locksChanged();
}

const QString &GameMapMission::medalImage() const
{
	return m_medalImage;
}

void GameMapMission::setMedalImage(const QString &newMedalImage)
{
	if (m_medalImage == newMedalImage)
		return;
	m_medalImage = newMedalImage;
	emit medalImageChanged();
}

GameMapNew *GameMapMission::gameMap() const
{
	return m_gameMap;
}

void GameMapMission::setGameMap(GameMapNew *newGameMap)
{
	if (m_gameMap == newGameMap)
		return;
	m_gameMap = newGameMap;
	emit gameMapChanged();
}



/**
 * @brief GameMapMissionLock::GameMapMissionLock
 * @param parent
 */

GameMapMissionLock::GameMapMissionLock(QObject *parent)
	: QObject{parent}
	, m_mission(nullptr)
	, m_level(-1)
	, m_parentMission(nullptr)
{

}


/**
 * @brief GameMapMissionLock::~GameMapMissionLock
 */

GameMapMissionLock::~GameMapMissionLock()
{
	//if (m_parentMission)
	//	m_parentMission->lockremove
}

GameMapMission *GameMapMissionLock::mission() const
{
	return m_mission;
}

void GameMapMissionLock::setMission(GameMapMission *newMission)
{
	if (m_mission == newMission)
		return;
	m_mission = newMission;
	emit missionChanged();
}

qint32 GameMapMissionLock::level() const
{
	return m_level;
}

void GameMapMissionLock::setLevel(qint32 newLevel)
{
	if (m_level == newLevel)
		return;
	m_level = newLevel;
	emit levelChanged();
}

GameMapMission *GameMapMissionLock::parentMission() const
{
	return m_parentMission;
}

void GameMapMissionLock::setParentMission(GameMapMission *newParentMission)
{
	if (m_parentMission == newParentMission)
		return;
	m_parentMission = newParentMission;
	emit parentMissionChanged();
}
