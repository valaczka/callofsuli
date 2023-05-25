/*
 * ---- Call of Suli ----
 *
 * mapeditormap.h
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

#ifndef MAPEDITORMAP_H
#define MAPEDITORMAP_H

#include <QObject>
#include "Logger.h"
#include "gamemapreaderiface.h"
#include "gamemap.h"
#include <selectableobject.h>
#include <QPointer>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"


class MapEditorStorage;
using MapEditorStorageList = qolm::QOlm<MapEditorStorage>;
Q_DECLARE_METATYPE(MapEditorStorageList*)

class MapEditorImage;
using MapEditorImageList = qolm::QOlm<MapEditorImage>;
Q_DECLARE_METATYPE(MapEditorImageList*)

class MapEditorChapter;
using MapEditorChapterList = qolm::QOlm<MapEditorChapter>;
Q_DECLARE_METATYPE(MapEditorChapterList*)

class MapEditorObjective;
using MapEditorObjectiveList = qolm::QOlm<MapEditorObjective>;
Q_DECLARE_METATYPE(MapEditorObjectiveList*)

class MapEditorMission;
using MapEditorMissionList = qolm::QOlm<MapEditorMission>;
Q_DECLARE_METATYPE(MapEditorMissionList*)

class MapEditorMissionLevel;
using MapEditorMissionLevelList = qolm::QOlm<MapEditorMissionLevel>;
Q_DECLARE_METATYPE(MapEditorMissionLevelList*)

class MapEditorInventory;
using MapEditorInventoryList = qolm::QOlm<MapEditorInventory>;
Q_DECLARE_METATYPE(MapEditorInventoryList*)

class MapEditor;


/**
 * @brief The MapEditorMap class
 */

class MapEditorMap : public QObject, public GameMapReaderIface
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(MapEditor *mapEditor READ mapEditor CONSTANT)
	Q_PROPERTY(MapEditorStorageList *storageList READ storageList CONSTANT)
	Q_PROPERTY(MapEditorImageList *imageList READ imageList CONSTANT)
	Q_PROPERTY(MapEditorChapterList *chapterList READ chapterList CONSTANT)
	Q_PROPERTY(MapEditorMissionList *missionList READ missionList CONSTANT)

public:
	explicit MapEditorMap(MapEditor *editor);
	virtual ~MapEditorMap();

	template<typename T>
	static QList<T*> olmListConvert(const qolm::QOlmBase *olm)
	{
		Q_ASSERT(olm);
		QList<T*> list;
		list.reserve(olm->size());
		for (int i=0; i<olm->size(); ++i) {
			QObject *o = olm->get(i);
			T* t = dynamic_cast<T*>(o);
			if (t)
				list.append(t);
			else
				LOG_CERROR("client") << "Dynamic cast error";
		}
		return list;
	}

	MapEditor *mapEditor() const;
	MapEditorStorageList *storageList() const;
	MapEditorImageList *imageList() const;
	MapEditorChapterList *chapterList() const;
	MapEditorMissionList *missionList() const;

	Q_INVOKABLE bool loadFromBinaryData(const QByteArray &data) {
		return readBinaryData(data);
	}

	Q_INVOKABLE MapEditorMission *mission(const QString &uuid) const;
	Q_INVOKABLE MapEditorMissionLevel *missionLevel(const QString &uuid, const int &level) const;
	Q_INVOKABLE MapEditorStorage *storage(const int &id) const;
	Q_INVOKABLE MapEditorChapter *chapter(const int &id) const;
	Q_INVOKABLE MapEditorChapter *chapter(MapEditorObjective *objective) const;
	Q_INVOKABLE MapEditorObjective *objective(const QString &uuid) const;
	Q_INVOKABLE MapEditorImage *image(const int &id) const;

	const QString &uuid() const;
	void setUuid(const QString &newUuid);


	int nextIndexStorage() { return ++m_indexStorage; }
	int nextIndexImage() { return ++m_indexImage; }
	int nextIndexChapter() { return ++m_indexChapter; }
	int nextIndexInventory() { return ++m_indexInventory; }

protected:
	QList<GameMapStorageIface *> ifaceStorages() const override
	{ return olmListConvert<GameMapStorageIface>(m_storageList); }

	QList<GameMapChapterIface*> ifaceChapters() const override
	{ return olmListConvert<GameMapChapterIface>(m_chapterList); }

	QList<GameMapMissionIface*> ifaceMissions() const override
	{ return olmListConvert<GameMapMissionIface>(m_missionList); }

	QList<GameMapImageIface*> ifaceImages() const override
	{ return olmListConvert<GameMapImageIface>(m_imageList); }

	GameMapStorageIface* ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data) override;
	GameMapChapterIface* ifaceAddChapter(const qint32 &id, const QString &name) override;
	GameMapMissionIface* ifaceAddMission(const QByteArray &uuid, const QString &name,
										 const QString &description, const QString &medalImage,
										 const quint32 &gameModes) override;
	GameMapImageIface* ifaceAddImage(const qint32 &id, const QByteArray &data) override;


signals:

	void uuidChanged();

private:
	MapEditor *const m_mapEditor;
	MapEditorStorageList *const m_storageList;
	MapEditorImageList *const m_imageList;
	MapEditorChapterList *const m_chapterList;
	MapEditorMissionList *const m_missionList;

	int m_indexStorage = 0;
	int m_indexImage = 0;
	int m_indexChapter = 0;
	int m_indexInventory = 0;
};




/**
 * @brief The MapEditorObject class
 */


class MapEditorObject : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(MapEditorMap *map READ map WRITE setMap NOTIFY mapChanged)

public:
	MapEditorObject(QObject *parent = nullptr) : SelectableObject(nullptr) {
		if (parent)
			LOG_CWARNING("client") << "Invalid parent of MapEditorObject";
	}
	virtual ~MapEditorObject() {}

	MapEditorMap * map() const { return m_map; }
	void setMap(MapEditorMap *newMap)
	{
		if (m_map == newMap)
			return;
		m_map = newMap;
		emit mapChanged();
	}

	virtual void fromVariantMap(const QVariantMap &map, const bool &onlyUpdate = false) = 0;
	virtual QVariantMap toVariantMap(const bool &onlyUpdate = false) const = 0;

signals:
	void mapChanged();

protected:
	MapEditorMap *m_map = nullptr;
};





/**
 * @brief The MapEditorStorage class
 */

class MapEditorStorage : public MapEditorObject, public GameMapStorageIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 storageid READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)
	Q_PROPERTY(int objectiveCount READ objectiveCount NOTIFY objectiveCountChanged)

public:
	explicit MapEditorStorage() : MapEditorObject(), GameMapStorageIface() {  }
	explicit MapEditorStorage(MapEditorMap *map) : MapEditorStorage() { m_map = map; }
	virtual ~MapEditorStorage() {}

	virtual void fromVariantMap(const QVariantMap &map, const bool & = false) override;
	virtual QVariantMap toVariantMap(const bool & = false) const override;

	qint32 id() const;
	void setId(qint32 newId);

	const QString &module() const;
	void setModule(const QString &newModule);

	const QVariantMap &data() const;
	void setData(const QVariantMap &newData);

	int objectiveCount() const;
	void setObjectiveCount(int newObjectiveCount);

	Q_INVOKABLE void recalculateObjectives();

signals:
	void idChanged();
	void moduleChanged();
	void dataChanged();
	void objectiveCountChanged();

private:
	int m_objectiveCount = 0;
};





/**
 * @brief The MapEditorImage class
 */

class MapEditorImage : public MapEditorObject, public GameMapImageIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 imageid READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QByteArray data READ data WRITE setData NOTIFY dataChanged)

public:
	explicit MapEditorImage() : MapEditorObject(), GameMapImageIface() {  }
	explicit MapEditorImage(MapEditorMap *map) : MapEditorImage() { m_map = map; }
	virtual ~MapEditorImage() {}

	virtual void fromVariantMap(const QVariantMap &map, const bool & = false) override;
	virtual QVariantMap toVariantMap(const bool & = false) const override;

	qint32 id() const;
	void setId(qint32 newId);

	const QByteArray &data() const;
	void setData(const QByteArray &newData);

signals:
	void idChanged();
	void dataChanged();
};





/**
 * @brief The MapEditorChapter class
 */


class MapEditorChapter : public MapEditorObject, public GameMapChapterIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 chapterid READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(MapEditorObjectiveList *objectiveList READ objectiveList CONSTANT)
	Q_PROPERTY(int objectiveCount READ objectiveCount NOTIFY objectiveCountChanged)

public:
	explicit MapEditorChapter() : MapEditorObject(), GameMapChapterIface(),
		m_objectiveList(new MapEditorObjectiveList(this)) {  }
	explicit MapEditorChapter(MapEditorMap *map) : MapEditorChapter() { m_map = map; }
	virtual ~MapEditorChapter() {
		delete m_objectiveList;
	}

	virtual void fromVariantMap(const QVariantMap &map, const bool &onlyUpdate = false) override;
	virtual QVariantMap toVariantMap(const bool &onlyUpdate = false) const override;

	qint32 id() const;
	void setId(qint32 newId);

	const QString &name() const;
	void setName(const QString &newName);

	MapEditorObjectiveList *objectiveList() const;

	MapEditorObjective *objective(const QString &uuid) const;

	int objectiveCount() const;
	void setObjectiveCount(int newObjectiveCount);

	Q_INVOKABLE void recalculateObjectiveCount();
	Q_INVOKABLE void recalculateStorageCount() const;

signals:
	void idChanged();
	void nameChanged();
	void objectiveCountChanged();

protected:
	QList<GameMapObjectiveIface*> ifaceObjectives() const override
	{ return MapEditorMap::olmListConvert<GameMapObjectiveIface>(m_objectiveList); }

	GameMapObjectiveIface* ifaceAddObjective(const QString &uuid,
											 const QString &module,
											 const qint32 &storageId,
											 const qint32 &storageCount,
											 const QVariantMap &data) override;

private:
	MapEditorObjectiveList *const m_objectiveList;
	int m_objectiveCount = 0;
};






/**
 * @brief The MapEditorObjective class
 */

class MapEditorObjective : public MapEditorObject, public GameMapObjectiveIface
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(qint32 storageId READ storageId WRITE setStorageId NOTIFY storageIdChanged)
	Q_PROPERTY(qint32 storageCount READ storageCount WRITE setStorageCount NOTIFY storageCountChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)
	Q_PROPERTY(MapEditorStorage* storage READ storage NOTIFY storageChanged)

public:
	explicit MapEditorObjective();
	explicit MapEditorObjective(MapEditorMap *map) : MapEditorObjective() { m_map = map; }
	virtual ~MapEditorObjective();

	virtual void fromVariantMap(const QVariantMap &map, const bool & = false) override;
	virtual QVariantMap toVariantMap(const bool & = false) const override;

	void recalculateStorage() const;

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &module() const;
	void setModule(const QString &newModule);

	qint32 storageId() const;
	void setStorageId(qint32 newStorageId);

	qint32 storageCount() const;
	void setStorageCount(qint32 newStorageCount);

	MapEditorStorage *storage() const;

	const QVariantMap &data() const;
	void setData(const QVariantMap &newData);

signals:
	void uuidChanged();
	void moduleChanged();
	void storageIdChanged();
	void storageCountChanged();
	void storageChanged();
	void dataChanged();
};







/**
 * @brief The MapEditorMission class
 */

class MapEditorMission : public MapEditorObject, public GameMapMissionIface
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(QString medalImage READ medalImage WRITE setMedalImage NOTIFY medalImageChanged)
	Q_PROPERTY(GameMap::GameModes modes READ modes WRITE setModes NOTIFY modesChanged)
	Q_PROPERTY(MapEditorMissionLevelList *levelList READ levelList CONSTANT)
	Q_PROPERTY(QString fullMedalImage READ fullMedalImage NOTIFY fullMedalImageChanged)
	Q_PROPERTY(QList<MapEditorMissionLevel *> lockList READ lockList NOTIFY lockListChanged)

public:
	explicit MapEditorMission() : MapEditorObject(), GameMapMissionIface()
	  , m_levelList(new MapEditorMissionLevelList(this))
	{ }
	explicit MapEditorMission(MapEditorMap *map) : MapEditorMission() { m_map = map; }
	virtual ~MapEditorMission() {
		delete m_levelList;
	}

	virtual void fromVariantMap(const QVariantMap &map, const bool &onlyUpdate = false) override;
	virtual QVariantMap toVariantMap(const bool &onlyUpdate = false) const override;

	QList<MapEditorMissionLevel *> lockList() const;
	Q_INVOKABLE void lockAdd(const QString &uuid, const int &level);
	Q_INVOKABLE void lockAdd(MapEditorMissionLevel *level);
	Q_INVOKABLE void lockRemove(const QString &uuid, const int &level);
	Q_INVOKABLE void lockRemove(MapEditorMissionLevel *level);

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &name() const;
	void setName(const QString &newName);

	const QString &description() const;
	void setDescription(const QString &newDescription);

	const QString &medalImage() const;
	void setMedalImage(const QString &newMedalImage);

	const GameMap::GameModes &modes() const;
	void setModes(const GameMap::GameModes &newModes);

	MapEditorMissionLevelList *levelList() const;
	MapEditorMissionLevel *level(const int &level) const;

	MapEditorMissionLevel *createNextLevel(MapEditorMap *map = nullptr) const;
	void levelAdd(MapEditorMissionLevel *level);
	void levelRemove(MapEditorMissionLevel *level);

	QString fullMedalImage() const;


signals:
	void uuidChanged();
	void nameChanged();
	void descriptionChanged();
	void medalImageChanged();
	void modesChanged();
	void lockListChanged();
	void fullMedalImageChanged();

protected:
	QList<GameMapMissionLevelIface*> ifaceLevels() const override
	{ return MapEditorMap::olmListConvert<GameMapMissionLevelIface>(m_levelList); }

	QList<GameMapMissionLevelIface*> ifaceLocks() const override;

	virtual GameMapMissionLevelIface* ifaceAddLevel(const qint32 &level,
													const QByteArray &terrain,
													const qint32 &startHP,
													const qint32 &duration,
													const bool &canDeathmatch,
													const qreal &questions,
													const qreal &passed,
													const QString &image) override;
	virtual GameMapMissionLevelIface* ifaceAddLock(const QString &uuid, const qint32 &level) override;


private:
	MapEditorMissionLevelList *const m_levelList;
	QList<QPointer<MapEditorMissionLevel>> m_lockList;
	GameMap::GameModes m_modes = GameMap::Invalid;
};






/**
 * @brief The MapEditorMissionLevel class
 */

class MapEditorMissionLevel : public MapEditorObject, public GameMapMissionLevelIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)
	Q_PROPERTY(qint32 startHP READ startHP WRITE setStartHP NOTIFY startHPChanged)
	Q_PROPERTY(qint32 duration READ duration WRITE setDuration NOTIFY durationChanged)
	Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
	Q_PROPERTY(bool canDeathmatch READ canDeathmatch WRITE setCanDeathmatch NOTIFY canDeathmatchChanged)
	Q_PROPERTY(qreal questions READ questions WRITE setQuestions NOTIFY questionsChanged)
	Q_PROPERTY(qreal passed READ passed WRITE setPassed NOTIFY passedChanged)
	Q_PROPERTY(MapEditorInventoryList *inventoryList READ inventoryList CONSTANT)
	Q_PROPERTY(MapEditorMission *mission READ editorMission CONSTANT)
	Q_PROPERTY(QList<MapEditorChapter*> chapterList READ chapterList NOTIFY chapterListChanged)

public:
	explicit MapEditorMissionLevel(MapEditorMission *mission)
		: MapEditorObject(), GameMapMissionLevelIface()
	  , m_inventoryList(new MapEditorInventoryList(this))
	  , m_mission(mission)
	{
		if (m_mission)
			m_map = m_mission->map();
	}
	explicit MapEditorMissionLevel(MapEditorMap *map, MapEditorMission *mission) : MapEditorMissionLevel(mission) { m_map = map; }
	virtual ~MapEditorMissionLevel() {
		delete m_inventoryList;
	}

	virtual void fromVariantMap(const QVariantMap &map, const bool &onlyUpdate = false) override;
	virtual QVariantMap toVariantMap(const bool &onlyUpdate = false) const override;


	qint32 level() const;
	void setLevel(qint32 newLevel);

	const QString &terrain() const;
	void setTerrain(const QString &newTerrain);

	qint32 startHP() const;
	void setStartHP(qint32 newStartHP);

	qint32 duration() const;
	void setDuration(qint32 newDuration);

	const QString &image() const;
	void setImage(const QString &newImage);

	bool canDeathmatch() const;
	void setCanDeathmatch(bool newCanDeathmatch);

	qreal questions() const;
	void setQuestions(qreal newQuestions);

	qreal passed() const;
	void setPassed(qreal newPassed);

	MapEditorInventoryList *inventoryList() const;
	MapEditorInventory *inventory(const int &inventoryId) const;
	MapEditorMission *editorMission() const { return m_mission; }

	QList<MapEditorChapter *> chapterList() const;

	Q_INVOKABLE void chapterAdd(const int &id);
	Q_INVOKABLE void chapterAdd(MapEditorChapter *chapter);
	Q_INVOKABLE void chapterRemove(const int &id);
	Q_INVOKABLE void chapterRemove(MapEditorChapter *chapter);

	Q_INVOKABLE bool canDelete() const;
	Q_INVOKABLE QList<MapEditorChapter*> unlinkedChapterList() const;

	Q_INVOKABLE void inventoryAdd(MapEditorInventory *inventory);
	Q_INVOKABLE void inventoryRemove(MapEditorInventory *inventory);
	Q_INVOKABLE void inventoryRemove(const int &id);


signals:
	void levelChanged();
	void terrainChanged();
	void startHPChanged();
	void durationChanged();
	void imageChanged();
	void canDeathmatchChanged();
	void questionsChanged();
	void passedChanged();
	void chapterListChanged();

protected:
	QList<GameMapChapterIface*> ifaceChapters() const override;
	QList<GameMapInventoryIface*> ifaceInventories() const override
	{ return MapEditorMap::olmListConvert<GameMapInventoryIface>(m_inventoryList); }

	bool ifaceAddChapter(const qint32 &chapterId) override;
	GameMapInventoryIface* ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count) override;

	GameMapMissionIface *mission() const override { return m_mission; }

private:
	MapEditorInventoryList *const m_inventoryList;
	MapEditorMission *const m_mission;
	QList<QPointer<MapEditorChapter>> m_chapterList;

};






/**
 * @brief The MapEditorInventory class
 */

class MapEditorInventory : public MapEditorObject, public GameMapInventoryIface
{
	Q_OBJECT

	Q_PROPERTY(int inventoryid READ inventoryid WRITE setInventoryid NOTIFY inventoryidChanged)
	Q_PROPERTY(qint32 block READ block WRITE setBlock NOTIFY blockChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(qint32 count READ count WRITE setCount NOTIFY countChanged)

public:
	explicit MapEditorInventory() : MapEditorObject(), GameMapInventoryIface() {  }
	explicit MapEditorInventory(MapEditorMap *map) : MapEditorInventory() { m_map = map; }
	virtual ~MapEditorInventory() {}

	virtual void fromVariantMap(const QVariantMap &map, const bool & = false) override;
	virtual QVariantMap toVariantMap(const bool & = false) const override;

	qint32 block() const;
	void setBlock(qint32 newBlock);

	const QString &module() const;
	void setModule(const QString &newModule);

	qint32 count() const;
	void setCount(qint32 newCount);

	int inventoryid() const;
	void setInventoryid(int newInventoryid);

signals:
	void blockChanged();
	void moduleChanged();
	void countChanged();
	void inventoryidChanged();

private:
	int m_inventoryid = 0;
};


#endif // MAPEDITORMAP_H
