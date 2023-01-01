/*
 * ---- Call of Suli ----
 *
 * mapplaymission.cpp
 *
 * Created on: 2023. 01. 01.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapPlayMission
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

#include "mapplaymission.h"

MapPlayMission::MapPlayMission(QObject *parent)
	: QObject{parent}
{

}

const QString &MapPlayMission::name() const
{
	return m_name;
}

void MapPlayMission::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

int MapPlayMissionLevel::level() const
{
	return m_level;
}

void MapPlayMissionLevel::setLevel(int newLevel)
{
	if (m_level == newLevel)
		return;
	m_level = newLevel;
	emit levelChanged();
}

bool MapPlayMissionLevel::deathmatch() const
{
	return m_deathmatch;
}

void MapPlayMissionLevel::setDeathmatch(bool newDeathmatch)
{
	if (m_deathmatch == newDeathmatch)
		return;
	m_deathmatch = newDeathmatch;
	emit deathmatchChanged();
}

MapPlayMissionLevelList *MapPlayMission::levels() const
{
	return m_levels;
}

void MapPlayMission::setLevels(MapPlayMissionLevelList *newLevels)
{
	if (m_levels == newLevels)
		return;
	m_levels = newLevels;
	emit levelsChanged();
}

GameMapMissionLevel *MapPlayMissionLevel::mapLevel() const
{
	return m_mapLevel;
}

void MapPlayMissionLevel::setMapLevel(GameMapMissionLevel *newMapLevel)
{
	if (m_mapLevel == newMapLevel)
		return;
	m_mapLevel = newMapLevel;
	emit mapLevelChanged();
}
