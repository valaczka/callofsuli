/*
 * ---- Call of Suli ----
 *
 * gamemapnew.cpp
 *
 * Created on: 2022. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapNew
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

#include <QDebug>
#include "gamemap.h"


/**
 * @brief GameMapXX::computeSolvedXpFactor
 * @param baseSolver
 * @param level
 * @param deathmatch
 * @return
 */


GameMap::GameMap()
	: GameMapReaderIface()
{

}


/**
 * @brief GameMap::~GameMap
 */

GameMap::~GameMap()
{
	qDeleteAll(m_storages);
	qDeleteAll(m_images);
	qDeleteAll(m_chapters);
	qDeleteAll(m_missions);

	m_storages.clear();
	m_images.clear();
	m_chapters.clear();
	m_missions.clear();
}


const QList<GameMapImage *> &GameMap::images() const
{
	return m_images;
}

const QList<GameMapChapter *> &GameMap::chapters() const
{
	return m_chapters;
}

const QList<GameMapMission *> &GameMap::missions() const
{
	return m_missions;
}

GameMapStorage *GameMap::storage(const qint32 &id) const
{
	foreach (GameMapStorage *s, m_storages)
		if (s->id() == id)
			return s;

	return nullptr;
}

GameMapImage *GameMap::image(const qint32 &id) const
{
	foreach (GameMapImage *s, m_images)
		if (s->id() == id)
			return s;

	return nullptr;
}

GameMapChapter *GameMap::chapter(const qint32 &id) const
{
	foreach (GameMapChapter *s, m_chapters)
		if (s->id() == id)
			return s;

	return nullptr;
}

GameMapMission *GameMap::mission(const QString &uuid) const
{
	foreach (GameMapMission *s, m_missions)
		if (s->uuid() == uuid)
			return s;

	return nullptr;
}

GameMapMissionLevel *GameMap::missionLevel(const QString &uuid, const qint32 &level) const
{
	GameMapMission *m = mission(uuid);

	if (m) {
		foreach (GameMapMissionLevel *s, m->levels())
			if (s->level() == level)
				return s;
	}

	return nullptr;
}

GameMapObjective *GameMap::objective(const QString &uuid) const
{
	foreach (GameMapChapter *ch, m_chapters) {
		foreach (GameMapObjective *s, ch->objectives())
			if (s->uuid() == uuid)
				return s;
	}

	return nullptr;
}


/**
 * @brief GameMap::fromBinaryData
 * @param data
 * @return
 */

GameMap *GameMap::fromBinaryData(const QByteArray &data)
{
	std::unique_ptr<GameMap> map(new GameMap);

	if (map->readBinaryData(data))
		return map.release();

	return nullptr;
}



/**
 * @brief GameMap::computeSolvedXpFactor
 * @param baseSolver
 * @param level
 * @param deathmatch
 * @return
 */

qreal GameMap::computeSolvedXpFactor(const SolverInfo &baseSolver, const int &level, const GameMode &mode)
{
	return computeSolvedXpFactor(level, baseSolver.solved(level), mode);
}


/**
 * @brief GameMap::computeSolvedXpFactor
 * @param level
 * @param deathmatch
 * @param solved
 * @param mode
 * @return
 */

qreal GameMap::computeSolvedXpFactor(const int &level, const int &solved, const GameMode &mode)
{
	qreal factor = XP_FACTOR_LEVEL*level;

	if (mode == Practice) {
		factor = XP_FACTOR_PRACTICE;
	} else {
		if (solved < 1)
			factor *= XP_FACTOR_SOLVED_FIRST;
		else if (solved > SOLVED_MAX) {
			factor += XP_FACTOR_SOLVED_OVER * (solved - SOLVED_MAX);
			factor = qMax(factor, XP_FACTOR_LEVEL);
		}
	}

	return factor;
}




/**
 * @brief GameMap::ifaceAddStorage
 * @param id
 * @param module
 * @param data
 * @return
 */

GameMapStorageIface *GameMap::ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data)
{
	GameMapStorage *s = new GameMapStorage(id, module, data);
	m_storages.append(s);
	return s;
}


/**
 * @brief GameMap::ifaceAddChapter
 * @param id
 * @param name
 * @return
 */

GameMapChapterIface *GameMap::ifaceAddChapter(const qint32 &id, const QString &name)
{
	GameMapChapter *s = new GameMapChapter(id, name, this);
	m_chapters.append(s);
	return s;
}


/**
 * @brief GameMap::ifaceAddMission
 * @param uuid
 * @param name
 * @param description
 * @param medalImage
 * @return
 */

GameMapMissionIface *GameMap::ifaceAddMission(const QByteArray &uuid, const QString &name,
											  const QString &description, const QString &medalImage, const quint32 &gameModes)
{
	GameMapMission *s = new GameMapMission(uuid, name, description, medalImage,
										   QVariant(gameModes).value<GameMap::GameModes>(),
										   this);

	m_missions.append(s);
	return s;
}


/**
 * @brief GameMap::ifaceAddImage
 * @param file
 * @param data
 * @return
 */

GameMapImageIface *GameMap::ifaceAddImage(const qint32 &id, const QByteArray &data)
{
	GameMapImage *s = new GameMapImage(id, data);
	m_images.append(s);
	return s;
}

const QString &GameMap::uuid() const
{
	return m_uuid;
}

const QList<GameMapStorage *> &GameMap::storages() const
{
	return m_storages;
}

GameMapStorage::GameMapStorage(const qint32 &id, const QString &module, const QVariantMap &data)
	: GameMapStorageIface()
{
	m_id = id;
	m_module = module;
	m_data = data;
}

qint32 GameMapStorage::id() const
{
	return m_id;
}

const QString &GameMapStorage::module() const
{
	return m_module;
}

const QVariantMap &GameMapStorage::data() const
{
	return m_data;
}

GameMapImage::GameMapImage(const qint32 &id, const QByteArray &data)
	: GameMapImageIface()
{
	m_id = id;
	m_data = data;
}

const qint32 &GameMapImage::id() const
{
	return m_id;
}

const QByteArray &GameMapImage::data() const
{
	return m_data;
}

GameMapChapter::GameMapChapter(const qint32 &id, const QString &name, GameMap *map)
	: GameMapChapterIface()
	, m_map(map)
{
	m_id = id;
	m_name = name;
}

GameMapChapter::~GameMapChapter()
{
	qDeleteAll(m_objectives);
	m_objectives.clear();
}

qint32 GameMapChapter::id() const
{
	return m_id;
}

const QString &GameMapChapter::name() const
{
	return m_name;
}

const QList<GameMapObjective *> &GameMapChapter::objectives() const
{
	return m_objectives;
}


/**
 * @brief GameMapChapter::ifaceAddObjective
 * @param m_uuid
 * @param m_module
 * @param m_storageId
 * @param m_storageCount
 * @param m_data
 * @return
 */

GameMapObjectiveIface *GameMapChapter::ifaceAddObjective(const QString &uuid, const QString &module,
														 const qint32 &storageId, const qint32 &storageCount,
														 const QVariantMap &data, const qint32 &examPoint)
{
	GameMapObjective *o = new GameMapObjective(uuid, module, storageId, storageCount, data, examPoint, m_map);
	m_objectives.append(o);
	return o;
}




/**
 * @brief GameMapMission::GameMapMission
 * @param uuid
 * @param name
 * @param description
 * @param medalImage
 */


GameMapMission::GameMapMission(const QByteArray &uuid, const QString &name, const QString &description,
							   const QString &medalImage, const GameMap::GameModes &modes,
							   GameMap *map)
	: GameMapMissionIface()
	, m_map(map)
{
	m_uuid = uuid;
	m_name = name;
	m_description = description;
	m_medalImage = medalImage;
	m_gameModes = modes;
}

GameMapMission::~GameMapMission()
{

}

const QString &GameMapMission::uuid() const
{
	return m_uuid;
}

const QString &GameMapMission::name() const
{
	return m_name;
}

const QString &GameMapMission::description() const
{
	return m_description;
}

const QString &GameMapMission::medalImage() const
{
	return m_medalImage;
}

const QList<GameMapMissionLevel *> &GameMapMission::levels() const
{
	return m_levels;
}

const QList<GameMapMissionLevel *> &GameMapMission::locks() const
{
	return m_locks;
}


/**
 * @brief GameMapMission::level
 * @param num
 * @return
 */

GameMapMissionLevel *GameMapMission::level(const qint32 &num) const
{
	foreach (GameMapMissionLevel *l, m_levels) {
		if (l->level() == num)
			return l;
	}

	return nullptr;
}




/**
 * @brief GameMapMission::ifaceAddLevel
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param canDeathmatch
 * @param questions
 * @param image
 * @return
 */

GameMapMissionLevelIface *GameMapMission::ifaceAddLevel(const qint32 &level, const QByteArray &terrain,
														const qint32 &startHP, const qint32 &duration,
														const bool &canDeathmatch, const qreal &questions,
														const qreal &passed, const quint32 &gameModes,
														const qint32 &image)
{
	GameMapMissionLevel *s = new GameMapMissionLevel(level, terrain, startHP,
													 duration, canDeathmatch, questions,
													 passed, QVariant(gameModes).value<GameMap::GameModes>(),
													 image, this, m_map);
	m_levels.append(s);
	return s;
}


/**
 * @brief GameMapMission::ifaceAddLock
 * @param uuid
 * @param level
 * @return
 */

GameMapMissionLevelIface *GameMapMission::ifaceAddLock(const QString &uuid, const qint32 &level)
{
	if (!m_map)
		return nullptr;

	GameMapMission *m = m_map->mission(uuid);

	if (!m)
		return nullptr;

	GameMapMissionLevel *l = m->level(level > 0 ? level : 1);
	m_locks.append(l);
	return l;
}



/**
 * @brief GameMapMission::modes
 * @return
 */

GameMap::GameModes GameMapMission::modes() const
{
	return QVariant(m_gameModes).value<GameMap::GameModes>();
}




/**
 * @brief GameMapObjective::GameMapObjective
 * @param m_uuid
 * @param m_module
 * @param m_storageId
 * @param m_storageCount
 * @param m_data
 */

GameMapObjective::GameMapObjective(const QString &uuid, const QString &module,
								   const qint32 &storageId, const qint32 &storageCount,
								   const QVariantMap &data, const qint32 &examPoint, GameMap *map)
	: GameMapObjectiveIface()
	, m_map(map)
{
	m_uuid = uuid;
	m_module = module;
	m_storageId = storageId;
	m_storageCount = storageCount;
	m_data = data;
	m_examPoint = examPoint;
	m_map = map;
}





/**
 * @brief GameMapObjective::uuid
 * @return
 */

const QString &GameMapObjective::uuid() const
{
	return m_uuid;
}

const QString &GameMapObjective::module() const
{
	return m_module;
}

qint32 GameMapObjective::storageId() const
{
	return m_storageId;
}

qint32 GameMapObjective::storageCount() const
{
	return m_storageCount;
}

const QVariantMap &GameMapObjective::data() const
{
	return m_data;
}


/**
 * @brief GameMapObjective::storage
 * @return
 */

GameMapStorage *GameMapObjective::storage() const
{
	if (!m_map)
		return nullptr;

	return m_map->storage(m_storageId);
}


/**
 * @brief GameMapObjective::generatedQuestions
 * @return
 */


QVariantList &GameMapObjective::generatedQuestions()
{
	return m_generatedQuestions;
}






/**
 * @brief GameMapMissionLevel::GameMapMissionLevel
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param canDeathmatch
 * @param questions
 * @param image
 */

GameMapMissionLevel::GameMapMissionLevel(const qint32 &level, const QByteArray &terrain, const qint32 &startHP,
										 const qint32 &duration, const bool &canDeathmatch, const qreal &questions,
										 const qreal &passed, const GameMap::GameModes &modes,
										 const qint32 &image, GameMapMission *mission, GameMap *map)
	: GameMapMissionLevelIface()
	, m_map(map)
	, m_mission(mission)
{
	m_level = level;
	m_terrain = terrain;
	m_startHP = startHP;
	m_duration = duration;
	m_canDeathmatch = canDeathmatch;
	m_questions = questions;
	m_passed = passed;
	m_image = image;
	m_gameModes = modes;
}

qint32 GameMapMissionLevel::level() const
{
	return m_level;
}

const QString &GameMapMissionLevel::terrain() const
{
	return m_terrain;
}

qint32 GameMapMissionLevel::startHP() const
{
	return m_startHP;
}

qint32 GameMapMissionLevel::duration() const
{
	return m_duration;
}

const qint32 &GameMapMissionLevel::image() const
{
	return m_image;
}

bool GameMapMissionLevel::canDeathmatch() const
{
	return m_canDeathmatch;
}

qreal GameMapMissionLevel::questions() const
{
	return m_questions;
}

qreal GameMapMissionLevel::passed() const
{
	return m_passed;
}

const QList<GameMapInventory *> &GameMapMissionLevel::inventories() const
{
	return m_inventories;
}

const QList<qint32> &GameMapMissionLevel::chapterIds() const
{
	return m_chapterIds;
}


/**
 * @brief GameMapMissionLevel::mission
 * @return
 */

GameMapMission *GameMapMissionLevel::mission() const
{
	return m_mission;
}


/**
 * @brief GameMapMissionLevel::chapters
 * @return
 */

QList<GameMapChapter *> GameMapMissionLevel::chapters() const
{
	QList<GameMapChapter *> list;

	if (m_map) {
		list.reserve(m_chapterIds.size());

		foreach (qint32 id, m_chapterIds)
			list.append(m_map->chapter(id));

	}
	return list;
}

bool GameMapMissionLevel::ifaceAddChapter(const qint32 &chapterId)
{
	m_chapterIds.append(chapterId);
	return true;
}


/**
 * @brief GameMapMissionLevel::ifaceAddInventory
 * @param block
 * @param module
 * @param count
 * @return
 */

GameMapInventoryIface *GameMapMissionLevel::ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count)
{
	GameMapInventory *i = new GameMapInventory(block, module, count);
	m_inventories.append(i);
	return i;
}

GameMap *GameMapMissionLevel::map() const
{
	return m_map;
}

GameMap::GameModes GameMapMissionLevel::modes() const
{
	return QVariant(m_gameModes).value<GameMap::GameModes>();
}


/**
 * @brief GameMapInventory::GameMapInventory
 * @param block
 * @param module
 * @param count
 */

GameMapInventory::GameMapInventory(const qint32 &block, const QString &module, const qint32 &count)
{
	m_block = block;
	m_module = module;
	m_count = count;
}

qint32 GameMapInventory::block() const
{
	return m_block;
}

const QString &GameMapInventory::module() const
{
	return m_module;
}

qint32 GameMapInventory::count() const
{
	return m_count;
}
