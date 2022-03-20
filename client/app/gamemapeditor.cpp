/*
 * ---- Call of Suli ----
 *
 * gamemapeditor.cpp
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapEditor
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

#include "gamemapeditor.h"
#include "question.h"
#include "cosclient.h"

GameMapEditor::GameMapEditor(QObject *parent)
	: QObject{parent}
	, GameMapReaderIface()
	, m_storages(new ObjectGenericListModel<GameMapEditorStorage>(this))
	, m_images(new ObjectGenericListModel<GameMapEditorImage>(this))
	, m_chapters(new ObjectGenericListModel<GameMapEditorChapter>(this))
	, m_missions(new ObjectGenericListModel<GameMapEditorMission>(this))
	, m_gameData()
{
	QVariant v = Client::readJsonFile(QString("qrc:/internal/game/parameters.json"));

	if (v.isValid())
		m_gameData = v.toMap();

}



GameMapEditor::~GameMapEditor()
{

}


/**
 * @brief GameMapEditor::chapter
 * @param id
 * @return
 */

GameMapEditorChapter *GameMapEditor::chapter(const qint32 &id) const
{
	foreach (GameMapEditorChapter *ch, m_chapters->objects()) {
		if (ch->id() == id)
			return ch;
	}

	return nullptr;
}

/**
 * @brief GameMapEditor::mission
 * @param uuid
 * @return
 */

GameMapEditorMission *GameMapEditor::mission(const QString &uuid) const
{
	foreach (GameMapEditorMission *m, m_missions->objects()) {
		if (m->uuid() == uuid)
			return m;
	}

	return nullptr;
}


/**
 * @brief GameMapEditor::missionLevel
 * @param uuid
 * @param level
 * @return
 */

GameMapEditorMissionLevel *GameMapEditor::missionLevel(const QString &uuid, const qint32 &level) const
{
	foreach (GameMapEditorMission *m, m_missions->objects()) {
		if (m->uuid() == uuid) {
			foreach (GameMapEditorMissionLevel *l, m->levels()->objects()) {
				if (l->level() == level)
					return l;
			}
		}
	}

	return nullptr;
}


/**
 * @brief GameMapEditor::storage
 * @param id
 * @return
 */

GameMapEditorStorage *GameMapEditor::storage(const qint32 &id) const
{
	foreach (GameMapEditorStorage *ch, m_storages->objects()) {
		if (ch->id() == id)
			return ch;
	}

	return nullptr;
}


/**
 * @brief GameMapEditor::fromBinaryData
 * @param data
 * @return
 */

GameMapEditor *GameMapEditor::fromBinaryData(const QByteArray &data, QObject *parent)
{
	GameMapEditor *map = new GameMapEditor(parent);

	if (map->readBinaryData(data))
		return map;

	delete map;

	return nullptr;
}




/**
 * @brief GameMapEditor::ifaceAddStorage
 * @param id
 * @param module
 * @param data
 * @return
 */

GameMapStorageIface *GameMapEditor::ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data)
{
	GameMapEditorStorage *s = new GameMapEditorStorage(id, module, data, this, this);
	m_storages->addObject(s);
	return s;
}


/**
 * @brief GameMapEditor::ifaceAddChapter
 * @param id
 * @param name
 * @return
 */

GameMapChapterIface *GameMapEditor::ifaceAddChapter(const qint32 &id, const QString &name)
{
	GameMapEditorChapter *s = new GameMapEditorChapter(id, name, this, this);
	m_chapters->addObject(s);
	return s;
}


/**
 * @brief GameMapEditor::ifaceAddMission
 * @param uuid
 * @param name
 * @param description
 * @param medalImage
 * @return
 */

GameMapMissionIface *GameMapEditor::ifaceAddMission(const QByteArray &uuid, const QString &name,
													const QString &description, const QString &medalImage)
{
	GameMapEditorMission *s = new GameMapEditorMission(uuid, name, description, medalImage, this, this);
	m_missions->addObject(s);
	return s;
}


/**
 * @brief GameMapEditor::ifaceAddImage
 * @param file
 * @param data
 * @return
 */

GameMapImageIface *GameMapEditor::ifaceAddImage(const QString &file, const QByteArray &data)
{
	GameMapEditorImage *s = new GameMapEditorImage(file, data, this);
	m_images->addObject(s);
	return s;
}

const QVariantMap &GameMapEditor::gameData() const
{
	return m_gameData;
}

const QString &GameMapEditor::uuid() const
{
	return m_uuid;
}

void GameMapEditor::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}






/**
 * @brief GameMapEditor::storages
 * @return
 */

ObjectGenericListModel<GameMapEditorStorage> *GameMapEditor::storages() const
{
	return m_storages;
}





/**
 * @brief GameMapEditorStorage::id
 * @return
 */

GameMapEditorStorage::GameMapEditorStorage(const qint32 &id, const QString &module, const QVariantMap &data, GameMapEditor *editor, QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapStorageIface()
	, m_editor(editor)
{
	m_id = id;
	m_module = module;
	m_data = data;
}

qint32 GameMapEditorStorage::id() const
{
	return m_id;
}

void GameMapEditorStorage::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}

const QString &GameMapEditorStorage::module() const
{
	return m_module;
}

void GameMapEditorStorage::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

const QVariantMap &GameMapEditorStorage::data() const
{
	return m_data;
}

void GameMapEditorStorage::setData(const QVariantMap &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}




/**
 * @brief GameMapEditorStorage::objectiveCount
 * @return
 */

int GameMapEditorStorage::objectiveCount() const
{
	if (!m_editor)
		return 0;

	int ret = 0;

	foreach (GameMapEditorChapter *ch, m_editor->chapters()->objects()) {
		foreach (GameMapEditorObjective *o, ch->objectives()->objects()) {
			if (o->storageId() == m_id)
				ret++;
		}
	}

	return ret;
}


/**
 * @brief GameMapEditorStorage::recalculateCounts
 */

void GameMapEditorStorage::recalculateCounts()
{
	emit objectiveCountChanged();
}



/**
 * @brief GameMapEditorImage::GameMapEditorImage
 * @param file
 * @param data
 * @param parent
 */

GameMapEditorImage::GameMapEditorImage(const QString &file, const QByteArray &data, QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapImageIface()
{
	m_name = file;
	m_data = data;
}


const QString &GameMapEditorImage::name() const
{
	return m_name;
}

void GameMapEditorImage::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

const QByteArray &GameMapEditorImage::data() const
{
	return m_data;
}

void GameMapEditorImage::setData(const QByteArray &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}

ObjectGenericListModel<GameMapEditorImage> *GameMapEditor::images() const
{
	return m_images;
}


/**
 * @brief GameMapEditorObjective::GameMapEditorObjective
 * @param uuid
 * @param module
 * @param storageId
 * @param storageCount
 * @param data
 * @param map
 * @param parent
 */

GameMapEditorObjective::GameMapEditorObjective(const QString &uuid, const QString &module,
											   const qint32 &storageId, const qint32 &storageCount,
											   const QVariantMap &data, GameMapEditor *map,
											   QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapObjectiveIface()
{
	m_uuid = uuid;
	m_module = module;
	m_storageId = storageId;
	m_storageCount = storageCount;
	m_data = data;
	m_map = map;

	connect(this, &GameMapEditorObjective::storageIdChanged, this, &GameMapEditorObjective::storageDataChanged);
	connect(this, &GameMapEditorObjective::storageIdChanged, this, &GameMapEditorObjective::storageModuleChanged);
	connect(this, &GameMapEditorObjective::storageIdChanged, this, &GameMapEditorObjective::infoChanged);
	connect(this, &GameMapEditorObjective::dataChanged, this, &GameMapEditorObjective::infoChanged);
}


const QString &GameMapEditorObjective::uuid() const
{
	return m_uuid;
}

void GameMapEditorObjective::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &GameMapEditorObjective::module() const
{
	return m_module;
}

void GameMapEditorObjective::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

qint32 GameMapEditorObjective::storageId() const
{
	return m_storageId;
}

void GameMapEditorObjective::setStorageId(qint32 newStorageId)
{
	if (m_storageId == newStorageId)
		return;
	m_storageId = newStorageId;
	emit storageIdChanged();
}

qint32 GameMapEditorObjective::storageCount() const
{
	return m_storageCount;
}

void GameMapEditorObjective::setStorageCount(qint32 newStorageCount)
{
	if (m_storageCount == newStorageCount)
		return;
	m_storageCount = newStorageCount;
	emit storageCountChanged();
}

const QVariantMap &GameMapEditorObjective::data() const
{
	return m_data;
}

void GameMapEditorObjective::setData(const QVariantMap &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}



/**
 * @brief GameMapEditorChapter::GameMapEditorChapter
 * @param id
 * @param name
 * @param map
 * @param parent
 */

GameMapEditorChapter::GameMapEditorChapter(const qint32 &id, const QString &name, GameMapEditor *map, QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapChapterIface()
	, m_objectives(new ObjectGenericListModel<GameMapEditorObjective>(this))
{
	m_id = id;
	m_name = name;
	m_map = map;
}

ObjectGenericListModel<GameMapEditorObjective> *GameMapEditorChapter::objectives() const
{
	return m_objectives;
}

qint32 GameMapEditorChapter::id() const
{
	return m_id;
}

void GameMapEditorChapter::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}

const QString &GameMapEditorChapter::name() const
{
	return m_name;
}

void GameMapEditorChapter::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}



/**
 * @brief GameMapEditorChapter::ifaceAddObjective
 * @param uuid
 * @param module
 * @param storageId
 * @param storageCount
 * @param data
 * @return
 */

GameMapObjectiveIface *GameMapEditorChapter::ifaceAddObjective(const QString &uuid, const QString &module,
															   const qint32 &storageId, const qint32 &storageCount,
															   const QVariantMap &data)
{
	GameMapEditorObjective *s = new GameMapEditorObjective(uuid, module, storageId, storageCount, data, m_map, this);
	m_objectives->addObject(s);
	return s;
}


ObjectGenericListModel<GameMapEditorChapter> *GameMapEditor::chapters() const
{
	return m_chapters;
}



/**
 * @brief GameMapEditorInventory::GameMapEditorInventory
 * @param block
 * @param module
 * @param count
 * @param parent
 */

GameMapEditorInventory::GameMapEditorInventory(const qint32 &block, const QString &module, const qint32 &count, QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapInventoryIface()
{
	m_block = block;
	m_module = module;
	m_count = count;
}

qint32 GameMapEditorInventory::block() const
{
	return m_block;
}

void GameMapEditorInventory::setBlock(qint32 newBlock)
{
	if (m_block == newBlock)
		return;
	m_block = newBlock;
	emit blockChanged();
}

const QString &GameMapEditorInventory::module() const
{
	return m_module;
}

void GameMapEditorInventory::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

qint32 GameMapEditorInventory::count() const
{
	return m_count;
}

void GameMapEditorInventory::setCount(qint32 newCount)
{
	if (m_count == newCount)
		return;
	m_count = newCount;
	emit countChanged();
}


/**
 * @brief GameMapEditorMissionLevel::GameMapEditorMissionLevel
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param canDeathmatch
 * @param questions
 * @param image
 * @param mission
 * @param map
 * @param parent
 */

GameMapEditorMissionLevel::GameMapEditorMissionLevel(const qint32 &level, const QByteArray &terrain,
													 const qint32 &startHP, const qint32 &duration,
													 const bool &canDeathmatch, const qreal &questions,
													 const QString &image, GameMapEditorMission *mission,
													 GameMapEditor *map, QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapMissionLevelIface()
	, m_chapters(new ObjectGenericListModel<GameMapEditorChapter>(false, this))
	, m_inventories(new ObjectGenericListModel<GameMapEditorInventory>(this))
	, m_mission(mission)
	, m_map(map)
{
	m_level = level;
	m_terrain = terrain;
	m_startHP = startHP;
	m_duration = duration;
	m_canDeathmatch = canDeathmatch;
	m_questions = questions;
	m_image = image;
}


ObjectGenericListModel<GameMapEditorChapter> *GameMapEditorMissionLevel::chapters() const
{
	return m_chapters;
}

ObjectGenericListModel<GameMapEditorInventory> *GameMapEditorMissionLevel::inventories() const
{
	return m_inventories;
}

GameMapMissionIface *GameMapEditorMissionLevel::mission() const
{
	return m_mission;
}

GameMapEditorMission *GameMapEditorMissionLevel::editorMission() const
{
	return m_mission;
}

qint32 GameMapEditorMissionLevel::level() const
{
	return m_level;
}

void GameMapEditorMissionLevel::setLevel(qint32 newLevel)
{
	if (m_level == newLevel)
		return;
	m_level = newLevel;
	emit levelChanged();
}

const QString &GameMapEditorMissionLevel::terrain() const
{
	return m_terrain;
}

void GameMapEditorMissionLevel::setTerrain(const QString &newTerrain)
{
	if (m_terrain == newTerrain)
		return;
	m_terrain = newTerrain;
	emit terrainChanged();
}

qint32 GameMapEditorMissionLevel::startHP() const
{
	return m_startHP;
}

void GameMapEditorMissionLevel::setStartHP(qint32 newStartHP)
{
	if (m_startHP == newStartHP)
		return;
	m_startHP = newStartHP;
	emit startHPChanged();
}

qint32 GameMapEditorMissionLevel::duration() const
{
	return m_duration;
}

void GameMapEditorMissionLevel::setDuration(qint32 newDuration)
{
	if (m_duration == newDuration)
		return;
	m_duration = newDuration;
	emit durationChanged();
}

const QString &GameMapEditorMissionLevel::image() const
{
	return m_image;
}

void GameMapEditorMissionLevel::setImage(const QString &newImage)
{
	if (m_image == newImage)
		return;
	m_image = newImage;
	emit imageChanged();
}

bool GameMapEditorMissionLevel::canDeathmatch() const
{
	return m_canDeathmatch;
}

void GameMapEditorMissionLevel::setCanDeathmatch(bool newCanDeathmatch)
{
	if (m_canDeathmatch == newCanDeathmatch)
		return;
	m_canDeathmatch = newCanDeathmatch;
	emit canDeathmatchChanged();
}

qreal GameMapEditorMissionLevel::questions() const
{
	return m_questions;
}

void GameMapEditorMissionLevel::setQuestions(qreal newQuestions)
{
	if (qFuzzyCompare(m_questions, newQuestions))
		return;
	m_questions = newQuestions;
	emit questionsChanged();
}


/**
 * @brief GameMapEditorMissionLevel::isLastLevel
 * @return
 */

bool GameMapEditorMissionLevel::isLastLevel() const
{
	if (!m_mission)
		return false;

	int maxLevel = -1;

	foreach (GameMapEditorMissionLevel *l, m_mission->levels()->objects())
		maxLevel = qMax(maxLevel, l->level());

	return (m_level == maxLevel);
}


/**
 * @brief GameMapEditorMissionLevel::ifaceAddChapter
 * @param chapterId
 * @return
 */

bool GameMapEditorMissionLevel::ifaceAddChapter(const qint32 &chapterId)
{
	if (!m_map)
		return false;

	GameMapEditorChapter *ch = m_map->chapter(chapterId);

	if (!ch)
		return false;

	m_chapters->addObject(ch);

	return true;
}



/**
 * @brief GameMapEditorMissionLevel::ifaceAddInventory
 * @param block
 * @param module
 * @param count
 * @return
 */

GameMapInventoryIface *GameMapEditorMissionLevel::ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count)
{
	GameMapEditorInventory *s = new GameMapEditorInventory(block, module, count, this);
	m_inventories->addObject(s);
	return s;
}



/**
 * @brief GameMapEditorMission::GameMapEditorMission
 * @param uuid
 * @param name
 * @param description
 * @param medalImage
 * @param map
 * @param parent
 */

GameMapEditorMission::GameMapEditorMission(const QByteArray &uuid, const QString &name,
										   const QString &description, const QString &medalImage,
										   GameMapEditor *map, QObject *parent)
	: ObjectListModelObject(parent)
	, GameMapMissionIface()
	, m_map(map)
	, m_levels(new ObjectGenericListModel<GameMapEditorMissionLevel>(this))
	, m_locks(new ObjectGenericListModel<GameMapEditorMissionLevel>(false, this))
{
	m_uuid = uuid;
	m_name = name;
	m_description = description;
	m_medalImage = medalImage;
}

const QString &GameMapEditorMission::uuid() const
{
	return m_uuid;
}

void GameMapEditorMission::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &GameMapEditorMission::name() const
{
	return m_name;
}

void GameMapEditorMission::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

const QString &GameMapEditorMission::description() const
{
	return m_description;
}

void GameMapEditorMission::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

const QString &GameMapEditorMission::medalImage() const
{
	return m_medalImage;
}

void GameMapEditorMission::setMedalImage(const QString &newMedalImage)
{
	if (m_medalImage == newMedalImage)
		return;
	m_medalImage = newMedalImage;
	emit medalImageChanged();
}

ObjectGenericListModel<GameMapEditorMissionLevel> *GameMapEditorMission::levels() const
{
	return m_levels;
}

ObjectGenericListModel<GameMapEditorMissionLevel> *GameMapEditorMission::locks() const
{
	return m_locks;
}


/**
 * @brief GameMapEditorMission::level
 * @param num
 * @return
 */

GameMapEditorMissionLevel *GameMapEditorMission::level(const qint32 &num) const
{
	foreach (GameMapEditorMissionLevel *l, m_levels->objects()) {
		if (l->level() == num)
			return l;
	}

	return nullptr;
}


/**
 * @brief GameMapEditorMission::lastLevel
 * @return
 */

GameMapEditorMissionLevel *GameMapEditorMission::lastLevel() const
{
	GameMapEditorMissionLevel *ret = nullptr;

	int maxLevel = -1;

	foreach (GameMapEditorMissionLevel *l, m_levels->objects()) {
		if (l->level() > maxLevel) {
			maxLevel = l->level();
			ret = l;
		}
	}

	return ret;
}



/**
 * @brief GameMapEditorMission::ifaceAddLevel
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param canDeathmatch
 * @param questions
 * @param image
 * @return
 */

GameMapMissionLevelIface *GameMapEditorMission::ifaceAddLevel(const qint32 &level, const QByteArray &terrain,
															  const qint32 &startHP, const qint32 &duration,
															  const bool &canDeathmatch, const qreal &questions,
															  const QString &image)
{
	GameMapEditorMissionLevel *s = new GameMapEditorMissionLevel(level, terrain, startHP, duration, canDeathmatch,
																 questions, image, this, m_map, this);
	m_levels->addObject(s);
	return s;
}


GameMapMissionLevelIface *GameMapEditorMission::ifaceAddLock(const QString &uuid, const qint32 &level)
{
	if (!m_map)
		return nullptr;

	GameMapEditorMission *m = m_map->mission(uuid);

	if (!m)
		return nullptr;

	GameMapEditorMissionLevel *s = m->level(level);
	m_locks->addObject(s);
	return s;
}

ObjectGenericListModel<GameMapEditorMission> *GameMapEditor::missions() const
{
	return m_missions;
}


/**
 * @brief GameMapEditorObjective::storageModule
 * @return
 */

const QString GameMapEditorObjective::storageModule() const
{
	if (m_map) {
		GameMapEditorStorage *s = m_map->storage(m_storageId);
		if (s)
			return s->module();
	}

	return "";
}


/**
 * @brief GameMapEditorObjective::storageData
 * @return
 */

const QVariantMap GameMapEditorObjective::storageData() const
{
	if (m_map) {
		GameMapEditorStorage *s = m_map->storage(m_storageId);
		if (s)
			return s->data();
	}

	return QVariantMap();
}


/**
 * @brief GameMapEditorObjective::info
 * @return
 */

const QStringList GameMapEditorObjective::info() const
{
	QVariantMap m = Question::objectiveInfo(m_module, m_data, storageModule(), storageData());

	QStringList list;
	list << m.value("name").toString()
		 << m.value("icon").toString()
		 << m.value("title").toString()
		 << m.value("details").toString()
		 << m.value("image").toString();

	return list;
}


/**
 * @brief GameMapEditorObjective::storageDataChanged
 */

void GameMapEditorObjective::storageDataReload()
{
	emit infoChanged();
}



/**
 * @brief GameMapEditorChapter::missionCount
 * @return
 */

int GameMapEditorChapter::missionCount()
{
	if (!m_map)
		return 0;

	int ret = 0;

	foreach (GameMapEditorMission *m, m_map->missions()->objects()) {
		foreach (GameMapEditorMissionLevel *ml, m->levels()->objects()) {
			if (ml->chapters()->objects().contains(this))
				++ret;
		}
	}

	return ret;
}


/**
 * @brief GameMapEditorChapter::objectivesCount
 * @return
 */

int GameMapEditorChapter::objectiveCount() const
{
	int ret = 0;

	foreach (GameMapEditorObjective *o, m_objectives->objects()) {
		if (o->storageId() != -1)
			ret += o->storageCount();
		else
			++ret;
	}

	return ret;
}


/**
 * @brief GameMapEditorChapter::recalculateCounts
 */

void GameMapEditorChapter::recalculateCounts()
{
	emit missionCountChanged();
	emit objectiveCountChanged();
}
