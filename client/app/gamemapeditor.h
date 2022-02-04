/*
 * ---- Call of Suli ----
 *
 * gamemapeditor.h
 *
 * Created on: 2022. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapEditorEditor
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

#ifndef GAMEMAPEDITOR_H
#define GAMEMAPEDITOR_H


#include <QObject>
#include <QList>
#include "objectlistmodelobject.h"
#include "objectlistmodel.h"
#include "gamemapreaderiface.h"


class GameMapEditor;
class GameMapEditorMission;


/**
 * @brief The GameMapEditorStorage class
 */


class GameMapEditorStorage : public ObjectListModelObject, public GameMapStorageIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 id READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)

public:
	explicit GameMapEditorStorage(const qint32 &id, const QString &module, const QVariantMap &data, QObject *parent = nullptr);
	virtual ~GameMapEditorStorage() {}

	qint32 id() const;
	void setId(qint32 newId);

	const QString &module() const;
	void setModule(const QString &newModule);

	const QVariantMap &data() const;
	void setData(const QVariantMap &newData);

signals:
	void idChanged();
	void moduleChanged();
	void dataChanged();

};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorStorage>*);







/**
 * @brief The GameMapEditorImage class
 */

class GameMapEditorImage : public ObjectListModelObject, public GameMapImageIface
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QByteArray data READ data WRITE setData NOTIFY dataChanged)

public:
	explicit GameMapEditorImage(const QString &file, const QByteArray &data, QObject *parent = nullptr);
	virtual ~GameMapEditorImage() {}

	const QString &name() const;
	void setName(const QString &newName);

	const QByteArray &data() const;
	void setData(const QByteArray &newData);

signals:
	void nameChanged();
	void dataChanged();
};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorImage>*);







/**
 * @brief The GameMapEditorObjective class
 */

class GameMapEditorObjective : public ObjectListModelObject, public GameMapObjectiveIface
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(qint32 storageId READ storageId WRITE setStorageId NOTIFY storageIdChanged)
	Q_PROPERTY(qint32 storageCount READ storageCount WRITE setStorageCount NOTIFY storageCountChanged)
	Q_PROPERTY(QVariantMap data READ data WRITE setData NOTIFY dataChanged)

public:
	explicit GameMapEditorObjective(const QString &uuid, const QString &module,
							  const qint32 &storageId, const qint32 &storageCount,
							  const QVariantMap &data, GameMapEditor *map, QObject *parent = nullptr);
	virtual ~GameMapEditorObjective() { qDebug() << "DESTROYED" << this; }

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &module() const;
	void setModule(const QString &newModule);

	qint32 storageId() const;
	void setStorageId(qint32 newStorageId);

	qint32 storageCount() const;
	void setStorageCount(qint32 newStorageCount);

	const QVariantMap &data() const;
	void setData(const QVariantMap &newData);

signals:
	void uuidChanged();
	void moduleChanged();
	void storageIdChanged();
	void storageCountChanged();
	void dataChanged();

private:
	GameMapEditor *m_map;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorObjective>*);








/**
 * @brief The GameMapEditorChapter class
 */

class GameMapEditorChapter : public ObjectListModelObject, public GameMapChapterIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 id READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorObjective> *objectives READ objectives CONSTANT)

public:
	explicit GameMapEditorChapter(const qint32 &id, const QString &name, GameMapEditor *map, QObject *parent = nullptr);
	virtual ~GameMapEditorChapter() {}

	ObjectGenericListModel<GameMapEditorObjective> *objectives() const;

	qint32 id() const;
	void setId(qint32 newId);

	const QString &name() const;
	void setName(const QString &newName);

signals:
	void idChanged();
	void nameChanged();

protected:
	QList<GameMapObjectiveIface*> ifaceObjectives() const override
	{ return GameMapReaderIface::ifaceListConvert<GameMapObjectiveIface, GameMapEditorObjective>(m_objectives->objects());	}

	GameMapObjectiveIface* ifaceAddObjective(const QString &uuid,
													 const QString &module,
													 const qint32 &storageId,
													 const qint32 &storageCount,
													 const QVariantMap &data) override;

private:
	GameMapEditor *m_map;
	ObjectGenericListModel<GameMapEditorObjective> *m_objectives;

	friend class MapEditorAction;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorChapter>*);



/**
 * @brief The GameMapEditorInventory class
 */

class GameMapEditorInventory : public ObjectListModelObject, public GameMapInventoryIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 block READ block WRITE setBlock NOTIFY blockChanged)
	Q_PROPERTY(QString module READ module WRITE setModule NOTIFY moduleChanged)
	Q_PROPERTY(qint32 count READ count WRITE setCount NOTIFY countChanged)

public:
	explicit GameMapEditorInventory(const qint32 &block, const QString &module, const qint32 &count, QObject *parent = nullptr);
	virtual ~GameMapEditorInventory() {}

	qint32 block() const;
	void setBlock(qint32 newBlock);

	const QString &module() const;
	void setModule(const QString &newModule);

	qint32 count() const;
	void setCount(qint32 newCount);

signals:
	void blockChanged();
	void moduleChanged();
	void countChanged();

};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorInventory>*);


/**
 * @brief The GameMapEditorMissionLevel class
 */

class GameMapEditorMissionLevel : public ObjectListModelObject, public GameMapMissionLevelIface
{
	Q_OBJECT

	Q_PROPERTY(qint32 level READ level WRITE setLevel NOTIFY levelChanged)
	Q_PROPERTY(QString terrain READ terrain WRITE setTerrain NOTIFY terrainChanged)
	Q_PROPERTY(qint32 startHP READ startHP WRITE setStartHP NOTIFY startHPChanged)
	Q_PROPERTY(qint32 duration READ duration WRITE setDuration NOTIFY durationChanged)
	Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)
	Q_PROPERTY(bool canDeathmatch READ canDeathmatch WRITE setCanDeathmatch NOTIFY canDeathmatchChanged)
	Q_PROPERTY(qreal questions READ questions WRITE setQuestions NOTIFY questionsChanged)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorChapter> *chapters READ chapters CONSTANT)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorInventory> *inventories READ inventories CONSTANT)
	Q_PROPERTY(GameMapEditorMission *mission READ mission CONSTANT)

public:
	explicit GameMapEditorMissionLevel(const qint32 &level, const QByteArray &terrain, const qint32 &startHP,
								 const qint32 &duration, const bool &canDeathmatch, const qreal &questions,
								 const QString &image, GameMapEditorMission *mission, GameMapEditor *map, QObject *parent = nullptr);
	virtual ~GameMapEditorMissionLevel() {}

	ObjectGenericListModel<GameMapEditorChapter> *chapters() const;

	ObjectGenericListModel<GameMapEditorInventory> *inventories() const;

	GameMapEditorMission *mission() const;

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

signals:
	void levelChanged();
	void terrainChanged();
	void startHPChanged();
	void durationChanged();
	void imageChanged();
	void canDeathmatchChanged();
	void questionsChanged();

protected:
	QList<GameMapChapterIface*> ifaceChapters() const
	{ return GameMapReaderIface::ifaceListConvert<GameMapChapterIface, GameMapEditorChapter>(m_chapters->objects());	}
	QList<GameMapInventoryIface*> ifaceInventories() const
	{ return GameMapReaderIface::ifaceListConvert<GameMapInventoryIface, GameMapEditorInventory>(m_inventories->objects());	}

	bool ifaceAddChapter(const qint32 &chapterId);
	GameMapInventoryIface* ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count);

private:
	ObjectGenericListModel<GameMapEditorChapter> *m_chapters;
	ObjectGenericListModel<GameMapEditorInventory> *m_inventories;
	GameMapEditorMission *m_mission;
	GameMapEditor *m_map;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorMissionLevel>*);



/**
 * @brief The GameMapEditorMissionLock class
 */

class GameMapEditorMissionLock : public ObjectListModelObject, public GameMapMissionLockIface
{
	Q_OBJECT

	Q_PROPERTY(GameMapEditorMission *mission READ mission WRITE setMission NOTIFY missionChanged)
	Q_PROPERTY(qint32 level READ level WRITE setLevel NOTIFY levelChanged)

public:
	explicit GameMapEditorMissionLock(GameMapEditorMission *mission, const qint32 &level, GameMapEditor *map, QObject *parent = nullptr);
	virtual ~GameMapEditorMissionLock() {}

	GameMapMissionIface *mission() const override;
	GameMapEditorMission *mission();
	void setMission(GameMapEditorMission *newMission);

	qint32 level() const;
	void setLevel(qint32 newLevel);

signals:
	void levelChanged();
	void missionChanged();

private:
	GameMapEditor *m_map;
	GameMapEditorMission *m_mission;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorMissionLock>*);



/**
 * @brief The GameMapEditorMission class
 */

class GameMapEditorMission : public ObjectListModelObject, public GameMapMissionIface
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(QString medalImage READ medalImage WRITE setMedalImage NOTIFY medalImageChanged)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorMissionLevel> *levels READ levels CONSTANT)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorMissionLock> *locks READ locks CONSTANT)

public:
	explicit GameMapEditorMission(const QByteArray &uuid, const QString &name, const QString &description,
								  const QString &medalImage, GameMapEditor *map, QObject *parent = nullptr);
	virtual ~GameMapEditorMission() {}


	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &name() const;
	void setName(const QString &newName);

	const QString &description() const;
	void setDescription(const QString &newDescription);

	const QString &medalImage() const;
	void setMedalImage(const QString &newMedalImage);

	ObjectGenericListModel<GameMapEditorMissionLevel> *levels() const;
	ObjectGenericListModel<GameMapEditorMissionLock> *locks() const;

signals:
	void uuidChanged();
	void nameChanged();
	void descriptionChanged();
	void medalImageChanged();

protected:
	QList<GameMapMissionLevelIface*> ifaceLevels() const override
	{ return GameMapReaderIface::ifaceListConvert<GameMapMissionLevelIface, GameMapEditorMissionLevel>(m_levels->objects());	}

	QList<GameMapMissionLockIface*> ifaceLocks() const override
	{ return GameMapReaderIface::ifaceListConvert<GameMapMissionLockIface, GameMapEditorMissionLock>(m_locks->objects()); }

	virtual GameMapMissionLevelIface* ifaceAddLevel(const qint32 &level,
													const QByteArray &terrain,
													const qint32 &startHP,
													const qint32 &duration,
													const bool &canDeathmatch,
													const qreal &questions,
													const QString &image) override;
	virtual GameMapMissionLockIface* ifaceAddLock(const QString &uuid, const qint32 &level) override;

private:
	GameMapEditor *m_map;
	ObjectGenericListModel<GameMapEditorMissionLevel> *m_levels;
	ObjectGenericListModel<GameMapEditorMissionLock> *m_locks;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<GameMapEditorMission>*);










/**
 * @brief The GameMapEditor class
 */


class GameMapEditor : public QObject, public GameMapReaderIface
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorStorage> *storages READ storages CONSTANT)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorImage> *images READ images CONSTANT)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorChapter> *chapters READ chapters CONSTANT)
	Q_PROPERTY(ObjectGenericListModel<GameMapEditorMission> *missions READ missions CONSTANT)

public:
	explicit GameMapEditor(QObject *parent = nullptr);
	virtual ~GameMapEditor();

	GameMapEditorChapter *chapter(const qint32 &id) const;
	GameMapEditorMission *mission(const QString &uuid) const;

	static GameMapEditor *fromBinaryData(const QByteArray &data, QObject *parent = nullptr);

	ObjectGenericListModel<GameMapEditorStorage> *storages() const;
	ObjectGenericListModel<GameMapEditorImage> *images() const;
	ObjectGenericListModel<GameMapEditorChapter> *chapters() const;
	ObjectGenericListModel<GameMapEditorMission> *missions() const;

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

signals:
	void uuidChanged();

protected:
	QList<GameMapStorageIface *> ifaceStorages() const override
	{ return ifaceListConvert<GameMapStorageIface, GameMapEditorStorage>(m_storages->objects()); }

	QList<GameMapChapterIface*> ifaceChapters() const override
	{ return ifaceListConvert<GameMapChapterIface, GameMapEditorChapter>(m_chapters->objects()); }

	QList<GameMapMissionIface*> ifaceMissions() const override
	{ return ifaceListConvert<GameMapMissionIface, GameMapEditorMission>(m_missions->objects()); }

	QList<GameMapImageIface*> ifaceImages() const override
	{ return ifaceListConvert<GameMapImageIface, GameMapEditorImage>(m_images->objects());	}

	GameMapStorageIface* ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data) override;
	GameMapChapterIface* ifaceAddChapter(const qint32 &id, const QString &name) override;
	GameMapMissionIface* ifaceAddMission(const QByteArray &uuid, const QString &name,
												 const QString &description, const QString &medalImage) override;
	GameMapImageIface* ifaceAddImage(const QString &file, const QByteArray &data) override;

private:
	ObjectGenericListModel<GameMapEditorStorage> *m_storages;
	ObjectGenericListModel<GameMapEditorImage> *m_images;
	ObjectGenericListModel<GameMapEditorChapter> *m_chapters;
	ObjectGenericListModel<GameMapEditorMission> *m_missions;

	friend class MapEditorAction;
};




#endif // GAMEMAPEDITOR_H
