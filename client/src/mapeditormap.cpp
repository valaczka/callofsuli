/*
 * ---- Call of Suli ----
 *
 * mapeditormap.cpp
 *
 * Created on: 2023. 05. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditorMap
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

#include "mapeditormap.h"
#include "Logger.h"
#include "clientcache.h"
#include "gameterrain.h"
#include "utils.h"
#include "abstractlevelgame.h"

/**
 * @brief MapEditorMap::MapEditorMap
 * @param editor
 */

MapEditorMap::MapEditorMap(MapEditor *editor)
	: m_mapEditor(editor)
	, m_storageList(new MapEditorStorageList(this))
	, m_imageList(new MapEditorImageList(this))
	, m_chapterList(new MapEditorChapterList(this))
	, m_missionList(new MapEditorMissionList(this))
{
	Q_ASSERT(m_mapEditor);

	LOG_CTRACE("client") << "MapEditorMap created" << this;
}


/**
 * @brief MapEditorMap::~MapEditorMap
 */

MapEditorMap::~MapEditorMap()
{
	// Különben a destructoroknál összeomolhat

	for (MapEditorMission *m : *m_missionList) {
		m->setMap(nullptr);
		for (MapEditorMissionLevel *ml : *m->levelList()) {
			ml->setMap(nullptr);
			for (MapEditorInventory *i : *ml->inventoryList())
				i->setMap(nullptr);
		}
	}

	for (MapEditorChapter *ch : *m_chapterList) {
		ch->setMap(nullptr);
		for (MapEditorObjective *o : *ch->objectiveList())
			o->setMap(nullptr);
	}

	for (MapEditorImage *i : *m_imageList)
		i->setMap(nullptr);

	for (MapEditorStorage *s : *m_storageList)
		s->setMap(nullptr);

	delete m_missionList;
	delete m_chapterList;
	delete m_imageList;
	delete m_storageList;

	LOG_CTRACE("client") << "MapEditorMap destroyed" << this;
}


/**
 * @brief MapEditorMap::mapEditor
 * @return
 */

MapEditor *MapEditorMap::mapEditor() const
{
	return m_mapEditor;
}


/**
 * @brief MapEditorMap::ifaceAddStorage
 * @param id
 * @param module
 * @param data
 * @return
 */

GameMapStorageIface *MapEditorMap::ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data)
{
	MapEditorStorage *d = new MapEditorStorage(this);
	d->setId(id);
	d->setModule(module);
	d->setData(data);
	m_storageList->append(d);
	m_indexStorage = qMax(id, m_indexStorage);
	return d;
}


/**
 * @brief MapEditorMap::ifaceAddChapter
 * @param id
 * @param name
 * @return
 */

GameMapChapterIface *MapEditorMap::ifaceAddChapter(const qint32 &id, const QString &name)
{
	MapEditorChapter *d = new MapEditorChapter(this);
	d->setId(id);
	d->setName(name);
	m_chapterList->append(d);
	m_indexChapter = qMax(id, m_indexChapter);
	return d;
}


/**
 * @brief MapEditorMap::ifaceAddMission
 * @param uuid
 * @param name
 * @param description
 * @param medalImage
 * @param gameModes
 * @return
 */

GameMapMissionIface *MapEditorMap::ifaceAddMission(const QByteArray &uuid, const QString &name, const QString &description,
												   const QString &medalImage, const quint32 &gameModes)
{
	MapEditorMission *d = new MapEditorMission(this);
	d->setUuid(uuid);
	d->setName(name);
	d->setDescription(description);
	d->setMedalImage(medalImage);
	d->setModes((GameMap::GameModes) gameModes);
	m_missionList->append(d);
	return d;
}


/**
 * @brief MapEditorMap::ifaceAddImage
 * @param id
 * @param data
 * @return
 */

GameMapImageIface *MapEditorMap::ifaceAddImage(const qint32 &id, const QByteArray &data)
{
	MapEditorImage *d = new MapEditorImage(this);
	d->setId(id);
	d->setData(data);
	m_imageList->append(d);
	m_indexImage = qMax(id, m_indexImage);
	return d;
}

const QString &MapEditorMap::uuid() const
{
	return m_uuid;
}

void MapEditorMap::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}


/**
 * @brief MapEditorMap::missionList
 * @return
 */

MapEditorMissionList *MapEditorMap::missionList() const
{
	return m_missionList;
}


/**
 * @brief MapEditorMap::mission
 * @param uuid
 * @return
 */

MapEditorMission *MapEditorMap::mission(const QString &uuid) const
{
	return OlmLoader::find<MapEditorMission>(m_missionList, "uuid", uuid);
}


/**
 * @brief MapEditorMap::missionLevel
 * @param uuid
 * @param level
 * @return
 */

MapEditorMissionLevel *MapEditorMap::missionLevel(const QString &uuid, const int &level) const
{
	MapEditorMission *m = OlmLoader::find<MapEditorMission>(m_missionList, "uuid", uuid);

	if (!m)
		return nullptr;

	return m->level(level);
}


/**
 * @brief MapEditorMap::storage
 * @param id
 * @return
 */

MapEditorStorage *MapEditorMap::storage(const int &id) const
{
	return OlmLoader::find<MapEditorStorage>(m_storageList, "storageid", id);
}


/**
 * @brief MapEditorMap::chapter
 * @param id
 * @return
 */

MapEditorChapter *MapEditorMap::chapter(const int &id) const
{
	return OlmLoader::find<MapEditorChapter>(m_chapterList, "chapterid", id);
}



/**
 * @brief MapEditorMap::chapter
 * @param objective
 * @return
 */

MapEditorChapter *MapEditorMap::chapter(MapEditorObjective *objective) const
{
	for (MapEditorChapter *ch : *m_chapterList) {
		if (ch->objective(objective->uuid()))
			return ch;
	}

	return nullptr;
}


/**
 * @brief MapEditorMap::objective
 * @param uuid
 * @return
 */

MapEditorObjective *MapEditorMap::objective(const QString &uuid) const
{
	for (MapEditorChapter *ch : *m_chapterList) {
		MapEditorObjective *o = ch->objective(uuid); //OlmLoader::find<MapEditorObjective>(ch->objectiveList(), "uuid", uuid);
		if (o)
			return o;
	}

	return nullptr;
}


/**
 * @brief MapEditorMap::image
 * @param id
 * @return
 */

MapEditorImage *MapEditorMap::image(const int &id) const
{
	return OlmLoader::find<MapEditorImage>(m_imageList, "imageid", id);
}



MapEditorChapterList *MapEditorMap::chapterList() const
{
	return m_chapterList;
}

MapEditorImageList *MapEditorMap::imageList() const
{
	return m_imageList;
}

MapEditorStorageList *MapEditorMap::storageList() const
{
	return m_storageList;
}








/**
 * @brief MapEditorStorage::fromVariantMap
 * @param map
 */

void MapEditorStorage::fromVariantMap(const QVariantMap &map, const bool &)
{
	setId(map.value(QStringLiteral("id")).toInt());
	setModule(map.value(QStringLiteral("module")).toString());
	setData(map.value(QStringLiteral("data")).toMap());
}


/**
 * @brief MapEditorStorage::toVariantMap
 * @return
 */

QVariantMap MapEditorStorage::toVariantMap(const bool &) const
{
	QVariantMap m;
	m.insert(QStringLiteral("id"), m_id);
	m.insert(QStringLiteral("module"), m_module);
	m.insert(QStringLiteral("data"), m_data);
	return m;
}



const QVariantMap &MapEditorStorage::data() const
{
	return m_data;
}

void MapEditorStorage::setData(const QVariantMap &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}

int MapEditorStorage::objectiveCount() const
{
	return m_objectiveCount;
}

void MapEditorStorage::setObjectiveCount(int newObjectiveCount)
{
	if (m_objectiveCount == newObjectiveCount)
		return;
	m_objectiveCount = newObjectiveCount;
	emit objectiveCountChanged();
}


/**
 * @brief MapEditorStorage::recalculateObjectives
 */

void MapEditorStorage::recalculateObjectives()
{
	int count = 0;

	if (m_map) {
		for (const MapEditorChapter *ch : *m_map->chapterList()) {
			for (const MapEditorObjective *o : *ch->objectiveList()) {
				if (o->storageId() == m_id)
					++count;
			}
		}
	}

	setObjectiveCount(count);
}



const QString &MapEditorStorage::module() const
{
	return m_module;
}

void MapEditorStorage::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

qint32 MapEditorStorage::id() const
{
	return m_id;
}

void MapEditorStorage::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}



/**
 * @brief MapEditorImage::fromVariantMap
 * @param map
 */

void MapEditorImage::fromVariantMap(const QVariantMap &map, const bool &)
{
	setId(map.value(QStringLiteral("id")).toInt());
	setData(map.value(QStringLiteral("data")).toByteArray());
}


/**
 * @brief MapEditorImage::toVariantMap
 * @return
 */

QVariantMap MapEditorImage::toVariantMap(const bool &) const
{
	QVariantMap m;
	m.insert(QStringLiteral("id"), m_id);
	m.insert(QStringLiteral("data"), m_data);
	return m;
}


qint32 MapEditorImage::id() const
{
	return m_id;
}

void MapEditorImage::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}

const QByteArray &MapEditorImage::data() const
{
	return m_data;
}

void MapEditorImage::setData(const QByteArray &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}


/**
 * @brief MapEditorChapter::fromVariantMap
 * @param map
 */

void MapEditorChapter::fromVariantMap(const QVariantMap &map, const bool &onlyUpdate)
{
	setId(map.value(QStringLiteral("id")).toInt());
	setName(map.value(QStringLiteral("name")).toString());

	if (onlyUpdate)
		return;

	foreach (const QVariant &v, map.value(QStringLiteral("objectives")).toList()) {
		MapEditorObjective *o = new MapEditorObjective(m_map);
		o->fromVariantMap(v.toMap());
		m_objectiveList->append(o);
	}

	recalculateObjectiveCount();
}



/**
 * @brief MapEditorChapter::toVariantMap
 * @return
 */

QVariantMap MapEditorChapter::toVariantMap(const bool &onlyUpdate) const
{
	QVariantMap m;
	m.insert(QStringLiteral("id"), m_id);
	m.insert(QStringLiteral("name"), m_name);

	if (!onlyUpdate) {
		QVariantList list;

		for (MapEditorObjective *o : *m_objectiveList)
			list.append(o->toVariantMap());

		m.insert(QStringLiteral("objectives"), list);
	}

	return m;
}



/**
 * @brief MapEditorChapter::ifaceAddObjective
 * @param uuid
 * @param module
 * @param storageId
 * @param storageCount
 * @param data
 * @return
 */

GameMapObjectiveIface *MapEditorChapter::ifaceAddObjective(const QString &uuid, const QString &module,
														   const qint32 &storageId, const qint32 &storageCount,
														   const QVariantMap &data)
{
	MapEditorObjective *d = new MapEditorObjective(m_map);
	d->setUuid(uuid);
	d->setModule(module);
	d->setStorageId(storageId);
	d->setStorageCount(storageCount);
	d->setData(data);
	m_objectiveList->append(d);
	recalculateObjectiveCount();
	d->recalculateStorage();
	return d;
}


/**
 * @brief MapEditorChapter::objectiveCount
 * @return
 */

int MapEditorChapter::objectiveCount() const
{
	return m_objectiveCount;
}

void MapEditorChapter::setObjectiveCount(int newObjectiveCount)
{
	if (m_objectiveCount == newObjectiveCount)
		return;
	m_objectiveCount = newObjectiveCount;
	emit objectiveCountChanged();
}


/**
 * @brief MapEditorChapter::recalculateObjectiveCount
 */

void MapEditorChapter::recalculateObjectiveCount()
{
	int n = 0;

	for (const MapEditorObjective *o : *m_objectiveList)
		n += qMax(1, o->storageCount());

	setObjectiveCount(n);
}

void MapEditorChapter::recalculateStorageCount() const
{
	for (const MapEditorObjective *o : *m_objectiveList)
		o->recalculateStorage();
}


/**
 * @brief MapEditorChapter::objectiveList
 * @return
 */

MapEditorObjectiveList *MapEditorChapter::objectiveList() const
{
	return m_objectiveList;
}


/**
 * @brief MapEditorChapter::objective
 * @param uuid
 * @return
 */

MapEditorObjective *MapEditorChapter::objective(const QString &uuid) const
{
	return OlmLoader::find<MapEditorObjective>(m_objectiveList, "uuid", uuid);
}


/**
 * @brief MapEditorChapter::name
 * @return
 */

const QString &MapEditorChapter::name() const
{
	return m_name;
}

void MapEditorChapter::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

qint32 MapEditorChapter::id() const
{
	return m_id;
}

void MapEditorChapter::setId(qint32 newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}


/**
 * @brief MapEditorObjective::fromVariantMap
 * @param map
 */

MapEditorObjective::MapEditorObjective()
	: MapEditorObject(nullptr)
	, GameMapObjectiveIface()
{
	LOG_CTRACE("client") << "OBJECTIVE CREATED" << this;
	connect(this, &MapEditorObjective::mapChanged, this, &MapEditorObjective::storageChanged);
}


/**
 * @brief MapEditorObjective::~MapEditorObjective
 */

MapEditorObjective::~MapEditorObjective()
{
	recalculateStorage();
	LOG_CTRACE("client") << "OBJECTIVE DESTROYED" << this;
}


/**
 * @brief MapEditorObjective::fromVariantMap
 * @param map
 */

void MapEditorObjective::fromVariantMap(const QVariantMap &map, const bool &)
{
	setUuid(map.value(QStringLiteral("uuid")).toString());
	setModule(map.value(QStringLiteral("module")).toString());
	setStorageId(map.value(QStringLiteral("storageId")).toInt());
	setStorageCount(map.value(QStringLiteral("storageCount")).toInt());
	setData(map.value(QStringLiteral("data")).toMap());
}


/**
 * @brief MapEditorObjective::toVariantMap
 * @return
 */

QVariantMap MapEditorObjective::toVariantMap(const bool &) const
{
	QVariantMap m;
	m.insert(QStringLiteral("uuid"), m_uuid);
	m.insert(QStringLiteral("module"), m_module);
	m.insert(QStringLiteral("storageId"), m_storageId);
	m.insert(QStringLiteral("storageCount"), m_storageCount);
	m.insert(QStringLiteral("data"), m_data);
	return m;
}


/**
 * @brief MapEditorObjective::recalculateStorage
 */

void MapEditorObjective::recalculateStorage() const
{
	if (m_storageId > 0 && m_map) {
		MapEditorStorage *s = m_map->storage(m_storageId);
		if (s)
			s->recalculateObjectives();
	}
}


/**
 * @brief MapEditorObjective::uuid
 * @return
 */

const QString &MapEditorObjective::uuid() const
{
	return m_uuid;
}

void MapEditorObjective::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &MapEditorObjective::module() const
{
	return m_module;
}

void MapEditorObjective::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

qint32 MapEditorObjective::storageId() const
{
	return m_storageId;
}

void MapEditorObjective::setStorageId(qint32 newStorageId)
{
	if (m_storageId == newStorageId)
		return;
	m_storageId = newStorageId;
	emit storageIdChanged();
	emit storageChanged();
}

qint32 MapEditorObjective::storageCount() const
{
	return m_storageCount;
}

void MapEditorObjective::setStorageCount(qint32 newStorageCount)
{
	if (m_storageCount == newStorageCount)
		return;
	m_storageCount = newStorageCount;
	emit storageCountChanged();
	recalculateStorage();
}


/**
 * @brief MapEditorObjective::storage
 * @return
 */

MapEditorStorage *MapEditorObjective::storage() const
{
	if (!m_map)
		return nullptr;

	return m_map->storage(m_storageId);
}


const QVariantMap &MapEditorObjective::data() const
{
	return m_data;
}

void MapEditorObjective::setData(const QVariantMap &newData)
{
	if (m_data == newData)
		return;
	m_data = newData;
	emit dataChanged();
}


/**
 * @brief MapEditorMission::fromVariantMap
 * @param map
 */

void MapEditorMission::fromVariantMap(const QVariantMap &map, const bool &onlyUpdate)
{
	setUuid(map.value(QStringLiteral("uuid")).toString());
	setName(map.value(QStringLiteral("name")).toString());
	setDescription(map.value(QStringLiteral("description")).toString());
	setMedalImage(map.value(QStringLiteral("medalImage")).toString());
	setModes((GameMap::GameModes) map.value(QStringLiteral("modes")).toInt());

	if (onlyUpdate)
		return;

	foreach (const QVariant &v, map.value(QStringLiteral("levels")).toList()) {
		MapEditorMissionLevel *o = new MapEditorMissionLevel(m_map, this);
		o->fromVariantMap(v.toMap());
		m_levelList->append(o);
	}

	foreach (const QVariant &v, map.value(QStringLiteral("locks")).toList()) {
		const QVariantMap &m = v.toMap();
		lockAdd(m.value(QStringLiteral("uuid")).toString(),
				m.value(QStringLiteral("level")).toInt());

	}
}

/**
 * @brief MapEditorMission::toVariantMap
 * @return
 */

QVariantMap MapEditorMission::toVariantMap(const bool &onlyUpdate) const
{

	QVariantMap m;
	m.insert(QStringLiteral("uuid"), m_uuid);
	m.insert(QStringLiteral("name"), m_name);
	m.insert(QStringLiteral("description"), m_description);
	m.insert(QStringLiteral("medalImage"), m_medalImage);
	m.insert(QStringLiteral("modes"), GameMap::GameModes::Int(m_modes));

	if (!onlyUpdate) {
		QVariantList list;
		QVariantList locks;

		for (MapEditorMissionLevel *o : *m_levelList)
			list.append(o->toVariantMap());

		foreach (MapEditorMissionLevel *l, m_lockList) {
			if (!l || !l->editorMission())
				continue;

			QVariantMap ml;
			ml.insert(QStringLiteral("uuid"), l->editorMission()->uuid());
			ml.insert(QStringLiteral("level"), l->level());
			locks.append(ml);
		}
		m.insert(QStringLiteral("locks"), locks);
		m.insert(QStringLiteral("levels"), list);
	}

	return m;
}


/**
 * @brief MapEditorMission::lockAdd
 * @param uuid
 * @param level
 */

void MapEditorMission::lockAdd(const QString &uuid, const int &level)
{
	if (!m_map)
		return;


	MapEditorMissionLevel *ml = m_map->missionLevel(uuid, level);

	if (ml)
		lockAdd(ml);
}


/**
 * @brief MapEditorMission::lockAdd
 * @param level
 */

void MapEditorMission::lockAdd(MapEditorMissionLevel *level)
{
	if (!level)
		return;
	m_lockList.append(level);
	emit lockListChanged();
}


/**
 * @brief MapEditorMission::lockRemove
 * @param uuid
 * @param level
 */

void MapEditorMission::lockRemove(const QString &uuid, const int &level)
{
	if (!m_map)
		return;

	MapEditorMissionLevel *ml = m_map->missionLevel(uuid, level);

	if (ml)
		lockRemove(ml);
}



/**
 * @brief MapEditorMission::lockRemove
 * @param level
 */

void MapEditorMission::lockRemove(MapEditorMissionLevel *level)
{
	m_lockList.removeAll(level);
	emit lockListChanged();
}




const QString &MapEditorMission::uuid() const
{
	return m_uuid;
}

void MapEditorMission::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QString &MapEditorMission::name() const
{
	return m_name;
}

void MapEditorMission::setName(const QString &newName)
{
	if (m_name == newName)
		return;
	m_name = newName;
	emit nameChanged();
}

const QString &MapEditorMission::description() const
{
	return m_description;
}

void MapEditorMission::setDescription(const QString &newDescription)
{
	if (m_description == newDescription)
		return;
	m_description = newDescription;
	emit descriptionChanged();
}

const QString &MapEditorMission::medalImage() const
{
	return m_medalImage;
}

void MapEditorMission::setMedalImage(const QString &newMedalImage)
{
	if (m_medalImage == newMedalImage)
		return;
	m_medalImage = newMedalImage;
	emit medalImageChanged();
	emit fullMedalImageChanged();
}

const GameMap::GameModes &MapEditorMission::modes() const
{
	return m_modes;
}

void MapEditorMission::setModes(const GameMap::GameModes &newModes)
{
	if (m_modes == newModes)
		return;
	m_modes = newModes;
	m_gameModes = m_modes;
	emit modesChanged();
}


/**
 * @brief MapEditorMission::ifaceLocks
 * @return
 */

QList<GameMapMissionLevelIface *> MapEditorMission::ifaceLocks() const
{
	QList<GameMapMissionLevelIface *> list;

	if (!m_map)
		return list;

	list.reserve(m_lockList.size());

	foreach (MapEditorMissionLevel *ml, m_lockList) {
		if (ml)
			list.append(ml);
	}



	return list;
}



/**
 * @brief MapEditorMission::ifaceAddLevel
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param canDeathmatch
 * @param questions
 * @param passed
 * @param image
 * @return
 */

GameMapMissionLevelIface *MapEditorMission::ifaceAddLevel(const qint32 &level, const QByteArray &terrain,
														  const qint32 &startHP, const qint32 &duration,
														  const bool &canDeathmatch, const qreal &questions,
														  const qreal &passed, const qint32 &image)
{
	MapEditorMissionLevel *d = new MapEditorMissionLevel(m_map, this);
	d->setLevel(level);
	d->setTerrain(terrain);
	d->setStartHP(startHP);
	d->setDuration(duration);
	d->setCanDeathmatch(canDeathmatch);
	d->setQuestions(questions);
	d->setPassed(passed);
	d->setImage(image);
	m_levelList->append(d);
	return d;
}




/**
 * @brief MapEditorMission::ifaceAddLock
 * @param uuid
 * @param level
 * @return
 */

GameMapMissionLevelIface *MapEditorMission::ifaceAddLock(const QString &uuid, const qint32 &level)
{
	if (!m_map)
		return nullptr;

	MapEditorMissionLevel *lock = m_map->missionLevel(uuid, level);

	if (lock)
		lockAdd(lock);

	return lock;
}

/**
 * @brief MapEditorMission::lockList
 * @return
 */

QList<MapEditorMissionLevel *> MapEditorMission::lockList() const
{
	QList<MapEditorMissionLevel *> list;

	list.reserve(m_lockList.size());

	foreach (MapEditorMissionLevel *ml, m_lockList)
		if (ml)
			list.append(ml);

	return list;
}




/**
 * @brief MapEditorMission::fullMedalImage
 * @return
 */

QString MapEditorMission::fullMedalImage() const
{
	return AbstractLevelGame::medalImagePath(m_medalImage);
}







/**
 * @brief MapEditorMission::levelList
 * @return
 */

MapEditorMissionLevelList *MapEditorMission::levelList() const
{
	return m_levelList;
}


/**
 * @brief MapEditorMission::level
 * @param level
 * @return
 */

MapEditorMissionLevel *MapEditorMission::level(const int &level) const
{
	return OlmLoader::find<MapEditorMissionLevel>(m_levelList, "level", level);
}


/**
 * @brief MapEditorMission::createNextLevel
 * @return
 */

MapEditorMissionLevel *MapEditorMission::createNextLevel(MapEditorMap *map) const
{
	if (!map)
		map = m_map;

	if (!map)
		return nullptr;

	if (m_levelList->size() >= 3)
		return nullptr;

	int l = 1;

	for (MapEditorMissionLevel *ml : *m_levelList)
		l = qMax(l, ml->level()+1);

	MapEditorMissionLevel *level = new MapEditorMissionLevel(const_cast<MapEditorMission*>(this));
	level->setLevel(l);
	level->setCanDeathmatch(m_modes.testFlag(GameMap::Action));

	const QJsonObject &parameters = Utils::fileToJsonObject(QStringLiteral(":/internal/game/parameters.json")).value_or(QJsonObject{});

	const QJsonObject &levelData = parameters.value(QStringLiteral("level")).toObject()
			.value(QString::number(l)).toObject()
			.value(QStringLiteral("defaults")).toObject();

	if (levelData.contains(QStringLiteral("duration")))
		level->setDuration(levelData.value(QStringLiteral("duration")).toInt());

	if (levelData.contains(QStringLiteral("startHP")))
		level->setStartHP(levelData.value(QStringLiteral("startHP")).toInt());

	if (levelData.contains(QStringLiteral("questions")))
		level->setQuestions(levelData.value(QStringLiteral("questions")).toDouble());

	if (levelData.contains(QStringLiteral("passed")))
		level->setPassed(levelData.value(QStringLiteral("passed")).toDouble());

	if (levelData.contains(QStringLiteral("deathmatch")))
		level->setCanDeathmatch(levelData.value(QStringLiteral("deathmatch")).toBool());

	if (levelData.contains(QStringLiteral("inventory"))) {
		const QJsonArray &list = levelData.value(QStringLiteral("inventory")).toArray();

		foreach (const QJsonValue &v, list) {
			const QVariantMap &data = v.toObject().toVariantMap();

			MapEditorInventory *inventory = new MapEditorInventory(map);
			inventory->fromVariantMap(data);
			inventory->setInventoryid(map->nextIndexInventory());

			level->inventoryAdd(inventory);
		}
	}

	return level;
}


/**
 * @brief MapEditorMission::levelAdd
 * @param level
 */

void MapEditorMission::levelAdd(MapEditorMissionLevel *level)
{
	if (!m_levelList->contains(level))
		m_levelList->append(level);
}



/**
 * @brief MapEditorMission::removeLevel
 * @param level
 */

void MapEditorMission::levelRemove(MapEditorMissionLevel *level)
{
	if (m_levelList->contains(level))
		m_levelList->remove(level);
}


/**
 * @brief MapEditorMissionLevel::fromVariantMap
 * @param map
 */

void MapEditorMissionLevel::fromVariantMap(const QVariantMap &map, const bool &onlyUpdate)
{
	setLevel(map.value(QStringLiteral("level")).toInt());
	setTerrain(map.value(QStringLiteral("terrain")).toString());
	setStartHP(map.value(QStringLiteral("startHP")).toInt());
	setDuration(map.value(QStringLiteral("duration")).toInt());
	setImage(map.value(QStringLiteral("image")).toInt());
	setCanDeathmatch(map.value(QStringLiteral("canDeathmatch")).toBool());
	setQuestions(map.value(QStringLiteral("questions")).toReal());
	setPassed(map.value(QStringLiteral("passed")).toReal());


	if (onlyUpdate)
		return;

	foreach (const QVariant &v, map.value(QStringLiteral("inventories")).toList()) {
		MapEditorInventory *o = new MapEditorInventory(m_map);
		o->fromVariantMap(v.toMap());
		m_inventoryList->append(o);
	}

	foreach (const QVariant &v, map.value(QStringLiteral("chapters")).toList())
		chapterAdd(v.toInt());

	emit chapterListChanged();
}


/**
 * @brief MapEditorMissionLevel::toVariantMap
 * @return
 */

QVariantMap MapEditorMissionLevel::toVariantMap(const bool &onlyUpdate) const
{

	QVariantMap m;
	m.insert(QStringLiteral("level"), m_level);
	m.insert(QStringLiteral("terrain"), m_terrain);
	m.insert(QStringLiteral("startHP"), m_startHP);
	m.insert(QStringLiteral("duration"), m_duration);
	m.insert(QStringLiteral("image"), m_image);
	m.insert(QStringLiteral("canDeathmatch"), m_canDeathmatch);
	m.insert(QStringLiteral("questions"), m_questions);
	m.insert(QStringLiteral("passed"), m_passed);

	if (!onlyUpdate) {
		QVariantList iList;
		QVariantList chList;

		for (MapEditorInventory *o : *m_inventoryList)
			iList.append(o->toVariantMap());

		foreach (MapEditorChapter *ch, m_chapterList)
			if (ch)
				chList.append(ch->id());

		m.insert(QStringLiteral("chapters"), chList);
		m.insert(QStringLiteral("inventories"), iList);
	}
	return m;
}


/**
 * @brief MapEditorMissionLevel::level
 * @return
 */

qint32 MapEditorMissionLevel::level() const
{
	return m_level;
}

void MapEditorMissionLevel::setLevel(qint32 newLevel)
{
	if (m_level == newLevel)
		return;
	m_level = newLevel;
	emit levelChanged();
}

const QString &MapEditorMissionLevel::terrain() const
{
	return m_terrain;
}

void MapEditorMissionLevel::setTerrain(const QString &newTerrain)
{
	if (m_terrain == newTerrain)
		return;
	m_terrain = newTerrain;
	emit terrainChanged();
	emit terrainDataChanged();
}

qint32 MapEditorMissionLevel::startHP() const
{
	return m_startHP;
}

void MapEditorMissionLevel::setStartHP(qint32 newStartHP)
{
	if (m_startHP == newStartHP)
		return;
	m_startHP = newStartHP;
	emit startHPChanged();
}

qint32 MapEditorMissionLevel::duration() const
{
	return m_duration;
}

void MapEditorMissionLevel::setDuration(qint32 newDuration)
{
	if (m_duration == newDuration)
		return;
	m_duration = newDuration;
	emit durationChanged();
}


bool MapEditorMissionLevel::canDeathmatch() const
{
	return m_canDeathmatch;
}

void MapEditorMissionLevel::setCanDeathmatch(bool newCanDeathmatch)
{
	if (m_canDeathmatch == newCanDeathmatch)
		return;
	m_canDeathmatch = newCanDeathmatch;
	emit canDeathmatchChanged();
}

qreal MapEditorMissionLevel::questions() const
{
	return m_questions;
}

void MapEditorMissionLevel::setQuestions(qreal newQuestions)
{
	if (qFuzzyCompare(m_questions, newQuestions))
		return;
	m_questions = newQuestions;
	emit questionsChanged();
}

qreal MapEditorMissionLevel::passed() const
{
	return m_passed;
}

void MapEditorMissionLevel::setPassed(qreal newPassed)
{
	if (qFuzzyCompare(m_passed, newPassed))
		return;
	m_passed = newPassed;
	emit passedChanged();
}

MapEditorInventoryList *MapEditorMissionLevel::inventoryList() const
{
	return m_inventoryList;
}



/**
 * @brief MapEditorMissionLevel::inventory
 * @param inventoryId
 * @return
 */

MapEditorInventory *MapEditorMissionLevel::inventory(const int &inventoryId) const
{
	for (MapEditorInventory *i : *m_inventoryList) {
		if (i->inventoryid() == inventoryId)
			return i;
	}

	return nullptr;
}



/**
 * @brief MapEditorMissionLevel::ifaceChapters
 * @return
 */

QList<GameMapChapterIface *> MapEditorMissionLevel::ifaceChapters() const
{
	QList<GameMapChapterIface *> list;

	if (!m_map)
		return list;

	list.reserve(m_chapterList.size());

	foreach (MapEditorChapter *ch, m_chapterList) {
		if (ch)
			list.append(ch);
	}


	return list;
}

/**
 * @brief MapEditorMissionLevel::ifaceAddChapter
 * @param chapterId
 * @return
 */

bool MapEditorMissionLevel::ifaceAddChapter(const qint32 &chapterId)
{
	chapterAdd(chapterId);
	return true;
}



/**
 * @brief MapEditorMissionLevel::ifaceAddInventory
 * @param block
 * @param module
 * @param count
 * @return
 */

GameMapInventoryIface *MapEditorMissionLevel::ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count)
{
	MapEditorInventory *d = new MapEditorInventory(m_map);
	d->setInventoryid(m_map->nextIndexInventory());
	d->setBlock(block);
	d->setModule(module);
	d->setCount(count);
	m_inventoryList->append(d);
	return d;
}



/**
 * @brief MapEditorMissionLevel::terrainData
 * @return
 */


QVariantMap MapEditorMissionLevel::terrainData() const
{
	const GameTerrain &t = GameTerrain::terrain(m_terrain);

	QVariantMap m;

	if (t.isValid()) {
		m.insert(QStringLiteral("name"), t.name());
		m.insert(QStringLiteral("displayName"), t.displayName());
		m.insert(QStringLiteral("level"), t.level());
		m.insert(QStringLiteral("thumbnail"), t.thumbnail());
	} else {
		m.insert(QStringLiteral("name"), QLatin1String(""));
		m.insert(QStringLiteral("displayName"), tr("-- érvénytelen --"));
		m.insert(QStringLiteral("level"), 0);
		m.insert(QStringLiteral("thumbnail"), QLatin1String(""));
	}

	return m;
}



/**
 * @brief MapEditorMissionLevel::chapterList
 * @return
 */

QList<MapEditorChapter *> MapEditorMissionLevel::chapterList() const
{
	QList<MapEditorChapter *> list;

	list.reserve(m_chapterList.size());

	foreach (MapEditorChapter *ml, m_chapterList)
		if (ml)
			list.append(ml);

	return list;
}


/**
 * @brief MapEditorMissionLevel::chapterAdd
 * @param id
 */

void MapEditorMissionLevel::chapterAdd(const int &id)
{
	chapterAdd(m_map->chapter(id));
}


/**
 * @brief MapEditorMissionLevel::chapterAdd
 * @param chapter
 */

void MapEditorMissionLevel::chapterAdd(MapEditorChapter *chapter)
{
	if (chapter) {
		m_chapterList.append(chapter);
		emit chapterListChanged();
	}
}


/**
 * @brief MapEditorMissionLevel::chapterRemove
 * @param id
 */

void MapEditorMissionLevel::chapterRemove(const int &id)
{
	chapterRemove(m_map->chapter(id));
}


/**
 * @brief MapEditorMissionLevel::chapterRemove
 * @param chapter
 */

void MapEditorMissionLevel::chapterRemove(MapEditorChapter *chapter)
{
	if (chapter) {
		m_chapterList.removeAll(chapter);
		emit chapterListChanged();
	}
}



/**
 * @brief MapEditorMissionLevel::canDelete
 * @return
 */

bool MapEditorMissionLevel::canDelete() const
{
	if (!m_mission)
		return false;

	int maxLevel = 0;

	for (const MapEditorMissionLevel *level : *m_mission->levelList())
		maxLevel = qMax(maxLevel, level->level());

	return (m_level > 1 && m_level == maxLevel);
}





/**
 * @brief MapEditorMissionLevel::unlinkedChapterList
 * @return
 */

QList<MapEditorChapter *> MapEditorMissionLevel::unlinkedChapterList() const
{
	QList<MapEditorChapter *> list;

	if (!m_map)
		return list;

	for (MapEditorChapter *chapter : *m_map->chapterList()) {
		if (chapter && !m_chapterList.contains(chapter))
			list.append(chapter);
	}

	return list;
}




/**
 * @brief MapEditorMissionLevel::inventoryAdd
 * @param inventory
 */

void MapEditorMissionLevel::inventoryAdd(MapEditorInventory *inventory)
{
	if (!m_inventoryList->contains(inventory))
		m_inventoryList->append(inventory);
}


/**
 * @brief MapEditorMissionLevel::inventoryRemove
 * @param inventory
 */

void MapEditorMissionLevel::inventoryRemove(MapEditorInventory *inventory)
{
	if (m_inventoryList->contains(inventory))
		m_inventoryList->remove(inventory);
}



/**
 * @brief MapEditorMissionLevel::inventoryRemove
 * @param id
 */

void MapEditorMissionLevel::inventoryRemove(const int &id)
{
	for (MapEditorInventory *inventory : *m_inventoryList) {
		if (inventory->inventoryid() == id)
			m_inventoryList->remove(inventory);
	}
}



/**
 * @brief MapEditorInventory::fromVariantMap
 * @param map
 */

void MapEditorInventory::fromVariantMap(const QVariantMap &map, const bool &)
{
	setInventoryid(map.value(QStringLiteral("id")).toInt());
	setBlock(map.value(QStringLiteral("block")).toInt());
	setModule(map.value(QStringLiteral("module")).toString());
	setCount(map.value(QStringLiteral("count")).toInt());
}


/**
 * @brief MapEditorInventory::toVariantMap
 * @return
 */

QVariantMap MapEditorInventory::toVariantMap(const bool &) const
{
	QVariantMap m;
	m.insert(QStringLiteral("id"), m_inventoryid);
	m.insert(QStringLiteral("block"), m_block);
	m.insert(QStringLiteral("module"), m_module);
	m.insert(QStringLiteral("count"), m_count);
	return m;
}


/**
 * @brief MapEditorInventory::block
 * @return
 */

qint32 MapEditorInventory::block() const
{
	return m_block;
}

void MapEditorInventory::setBlock(qint32 newBlock)
{
	if (m_block == newBlock)
		return;
	m_block = newBlock;
	emit blockChanged();
}

const QString &MapEditorInventory::module() const
{
	return m_module;
}

void MapEditorInventory::setModule(const QString &newModule)
{
	if (m_module == newModule)
		return;
	m_module = newModule;
	emit moduleChanged();
}

qint32 MapEditorInventory::count() const
{
	return m_count;
}

void MapEditorInventory::setCount(qint32 newCount)
{
	if (m_count == newCount)
		return;
	m_count = newCount;
	emit countChanged();
}

int MapEditorInventory::inventoryid() const
{
	return m_inventoryid;
}

void MapEditorInventory::setInventoryid(int newInventoryid)
{
	if (m_inventoryid == newInventoryid)
		return;
	m_inventoryid = newInventoryid;
	emit inventoryidChanged();
}


qint32 MapEditorMissionLevel::image() const
{
	return m_image;
}

void MapEditorMissionLevel::setImage(qint32 newImage)
{
	if (m_image == newImage)
		return;
	m_image = newImage;
	emit imageChanged();
	emit editorImageChanged();
}


/**
 * @brief MapEditorMissionLevel::editorImage
 * @return
 */

MapEditorImage *MapEditorMissionLevel::editorImage() const
{
	return m_map ? m_map->image(m_image) : nullptr;
}
