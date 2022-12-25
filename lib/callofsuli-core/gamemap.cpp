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
#include <QRandomGenerator>
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
	qDebug() << "++++++";
	qDebug() << "+++" << data.size();
	GameMap *map = new GameMap();

	qDebug() << "****";

	if (map->readBinaryData(data))
		return map;

	delete map;

	return nullptr;
}




/**
 * @brief GameMap::setSolver
 * @param list
 */

void GameMap::setSolver(const QVariantList &list)
{
	// Clear
	foreach (GameMapMission *m, m_missions) {
		m->setLockDepth(0);
		foreach (GameMapMissionLevel *ml, m->levels()) {
			ml->setSolvedNormal(false);
			ml->setSolvedDeathmatch(false);
		}
	}

	// Load solver
	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		QByteArray id = m.value("missionid").toByteArray();

		GameMapMission *mis = mission(id);

		if (!mis) {
//			qCDebug(lcMapReader).noquote() << QObject::tr("Invalid mission id:") << id;
			continue;
		}

		SolverInfo info(m);

		foreach (GameMapMissionLevel *ml, mis->levels()) {
			ml->setSolvedNormal(info.hasSolved(ml->level(), false));
			ml->setSolvedDeathmatch(info.hasSolved(ml->level(), true));
		}

	}


	// Calculate locks

	foreach (GameMapMission *m, m_missions) {
		qint32 lockDepth = 0;

		QVector<GameMapMissionLevelIface*> locks = missionLockTree(m);
		foreach (GameMapMissionLevelIface *mli, locks) {
			GameMapMissionLevel *ml = dynamic_cast<GameMapMissionLevel*>(mli);

			Q_ASSERT(ml);

			GameMapMission *lockerMission = dynamic_cast<GameMapMission*>(ml->mission());

			Q_ASSERT(lockerMission);

			qint32 lockerLevel = ml->level();

			if ((lockerLevel == -1 && lockerMission->solvedLevel() < 1) || lockerLevel > lockerMission->solvedLevel()) {
				QVector<GameMapMissionLevelIface*> lockerLocks = missionLockTree(lockerMission);
				if (lockerLocks.size() > 0) {
					foreach (GameMapMissionLevelIface* mli2, lockerLocks) {
						GameMapMissionLevel *ml2 = dynamic_cast<GameMapMissionLevel*>(mli2);

						Q_ASSERT(ml2);

						GameMapMission *locker2Mission = dynamic_cast<GameMapMission*>(ml2->mission());

						Q_ASSERT(locker2Mission);

						if ((ml2->level() == -1 && locker2Mission->solvedLevel() < 1) || ml2->level() > locker2Mission->solvedLevel()) {
							lockDepth = 2;
							break;
						}
					}
				}

				if (lockDepth < 1)
					lockDepth = 1;
			}
		}

		m->setLockDepth(lockDepth);
	}

}




/**
 * @brief GameMap::getUnlocks
 * @param uuid
 * @param level
 * @param deathmatch
 * @return
 */

QVector<GameMap::MissionLevelDeathmatch> GameMap::getUnlocks(const QString &uuid, const qint32 &level, const bool &deathmatch) const
{
	GameMapMission *mis = mission(uuid);
	GameMapMissionLevel *ml = missionLevel(uuid, level);
	QVector<MissionLevelDeathmatch> ret;

	if (!mis || !ml) {
//		qCDebug(lcMapReader).noquote() << QObject::tr("Invalid mission uuid:") << uuid;
		return ret;
	}

	if (deathmatch)
		return ret;

	if (ml->solvedNormal())
		return ret;


	// Unlock deathmatch

	if (!deathmatch && ml->canDeathmatch()) {
		ret.append(qMakePair(ml, true));
	}

	// Unlock next level

	GameMapMissionLevel *nextlevel = missionLevel(uuid, level+1);
	if (nextlevel) {
		ret.append(qMakePair(nextlevel, false));
	}


	// Locks

	QVector<GameMapMission *> lockedMissions;

	foreach(GameMapMission *lm, m_missions) {
		QVector<GameMapMissionLevelIface*> locks = missionLockTree(lm);

		if (locks.isEmpty())
			continue;

		foreach (GameMapMissionLevelIface *mli, locks) {
			GameMapMissionLevel *l = dynamic_cast<GameMapMissionLevel*>(mli);

			Q_ASSERT(l);

			if (l->mission() == mis && (l->level() == level || l->level() == -1))
				if (!lockedMissions.contains(lm))
					lockedMissions.append(lm);
		}
	}



	if (lockedMissions.isEmpty())
		return ret;



	// Calculate unlocks

	foreach (GameMapMission *m, lockedMissions) {
		qint32 oldLockDepth = m->lockDepth();

		if (oldLockDepth < 1)
			continue;

		bool locked = false;

		QVector<GameMapMissionLevelIface*> locks = missionLockTree(m);
		foreach (GameMapMissionLevelIface *mli, locks) {
			GameMapMissionLevel *ml = dynamic_cast<GameMapMissionLevel*>(mli);

			Q_ASSERT(ml);

			GameMapMission *lockerMission = dynamic_cast<GameMapMission*>(ml->mission());

			Q_ASSERT(lockerMission);

			qint32 lockerLevel = ml->level();
			qint32 solvedLevel = lockerMission->solvedLevel();


			// Emulate solved level

			if (lockerMission == mis && solvedLevel < level)
				solvedLevel = level;


			if ((lockerLevel == -1 && solvedLevel < 1) || lockerLevel > solvedLevel) {
				locked = true;
			}
		}

		if (oldLockDepth > 0 && !locked) {
			ret.append(qMakePair(m->level(1), false));
		}
	}

	return ret;
}




/**
 * @brief GameMap::getNextMissionLevel
 * @param uuid
 * @param level
 * @param deathmatch
 * @return
 */

GameMap::MissionLevelDeathmatch GameMap::getNextMissionLevel(const QString &uuid, const qint32 &level, const bool &deathmatch, const bool &lite) const
{
	GameMapMission *mis = mission(uuid);
	GameMapMissionLevel *ml = missionLevel(uuid, level);

	if (!mis || !ml) {
//		qCDebug(lcMapReader).noquote() << QObject::tr("Invalid mission uuid:") << uuid;
		return qMakePair(nullptr, false);
	}


	// Check next level

	GameMapMissionLevel *nextlevel = mis->level(level+1);
	if (nextlevel && !nextlevel->solvedNormal())
		return qMakePair(nextlevel, false);


	// Check deathmatch

	if (!deathmatch && ml->canDeathmatch() && !ml->solvedDeathmatch() && !lite)
		return qMakePair(ml, true);


	// Check current mission

	for (int i=1; GameMapMissionLevel *l = mis->level(i); i++) {
		if (l == ml)
			continue;

		if (!l->solvedNormal())
			return qMakePair(l, false);

		if (l->canDeathmatch() && !l->solvedDeathmatch() && !lite)
			return qMakePair(l, true);
	}


	// Check other missions for normal levels

	foreach (GameMapMission *m, missions()) {
		if (m == mis)
			continue;

		if (m->lockDepth() > 0)
			continue;

		for (int i=1; GameMapMissionLevel *l = m->level(i); i++) {
			if (!l->solvedNormal())
				return qMakePair(l, false);
		}
	}


	// Check all missions for deathmatch levels

	if (lite)
		return qMakePair(nullptr, false);

	foreach (GameMapMission *m, missions()) {
		if (m == mis)
			continue;

		if (m->lockDepth() > 0)
			continue;

		for (int i=1; GameMapMissionLevel *l = m->level(i); i++) {
			if (l->canDeathmatch() && !l->solvedDeathmatch())
				return qMakePair(l, true);
		}
	}

	return qMakePair(nullptr, false);
}




/**
 * @brief GameMap::computeSolvedXpFactor
 * @param baseSolver
 * @param level
 * @param deathmatch
 * @return
 */

qreal GameMap::computeSolvedXpFactor(const SolverInfo &baseSolver, const int &level, const bool &deathmatch, const bool &isLite)
{
	qreal xp = XP_FACTOR_LEVEL*level;

	if (!isLite) {
		if (deathmatch)
			xp *= XP_FACTOR_DEATHMATCH;

		if (!baseSolver.hasSolved(level, deathmatch))
			xp *= XP_FACTOR_SOLVED_FIRST;
	}

	return xp;
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
											  const QString &description, const QString &medalImage)
{
	GameMapMission *s = new GameMapMission(uuid, name, description, medalImage, this);
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
														 const QVariantMap &data)
{
	GameMapObjective *o = new GameMapObjective(uuid, module, storageId, storageCount, data, m_map);
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


GameMapMission::GameMapMission(const QByteArray &uuid, const QString &name, const QString &description, const QString &medalImage, GameMap *map)
	: GameMapMissionIface()
	, m_map(map)
	, m_lockDepth(0)
{
	m_uuid = uuid;
	m_name = name;
	m_description = description;
	m_medalImage = medalImage;
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
 * @brief GameMapMission::solvedLevel
 * @return
 */

qint32 GameMapMission::solvedLevel() const
{
	qint32 level = 0;

	foreach (GameMapMissionLevel *ml, m_levels) {
		int n = ml->level();
		if (ml->solvedNormal()) {
			bool hasSolvedAll = true;
			for (int i=n-1; i>0; --i) {
				if (!ml->mission()->level(i) || !ml->mission()->level(i)->solvedNormal())
					hasSolvedAll = false;
			}

			if (hasSolvedAll && level < n)
				level = n;
		}
	}

	return level;
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
														const QString &image)
{
	GameMapMissionLevel *s = new GameMapMissionLevel(level, terrain, startHP,
													 duration, canDeathmatch, questions,
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



int GameMapMission::lockDepth() const
{
	return m_lockDepth;
}

void GameMapMission::setLockDepth(int newLockDepth)
{
	m_lockDepth = newLockDepth;
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
								   const QVariantMap &data, GameMap *map)
	: GameMapObjectiveIface()
	, m_map(map)
{
	m_uuid = uuid;
	m_module = module;
	m_storageId = storageId;
	m_storageCount = storageCount;
	m_data = data;
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
										 const QString &image, GameMapMission *mission, GameMap *map)
	: GameMapMissionLevelIface()
	, m_map(map)
	, m_mission(mission)
	, m_solvedNormal(false)
	, m_solvedDeathmatch(false)
{
	m_level = level;
	m_terrain = terrain;
	m_startHP = startHP;
	m_duration = duration;
	m_canDeathmatch = canDeathmatch;
	m_questions = questions;
	m_image = image;
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

const QString &GameMapMissionLevel::image() const
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

bool GameMapMissionLevel::solvedDeathmatch() const
{
	return m_solvedDeathmatch;
}

void GameMapMissionLevel::setSolvedDeathmatch(bool newSolvedDeathmatch)
{
	m_solvedDeathmatch = newSolvedDeathmatch;
}

bool GameMapMissionLevel::solvedNormal() const
{
	return m_solvedNormal;
}

void GameMapMissionLevel::setSolvedNormal(bool newSolvedNormal)
{
	m_solvedNormal = newSolvedNormal;
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
