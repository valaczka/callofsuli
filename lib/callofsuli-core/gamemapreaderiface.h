/*
 * ---- Call of Suli ----
 *
 * gamemapreaderiface.h
 *
 * Created on: 2022. 01. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapReaderIface
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

#ifndef GAMEMAPREADERIFACE_H
#define GAMEMAPREADERIFACE_H


#include "qloggingcategory.h"
#include <QDataStream>
#include <QVariantMap>

#define GAMEMAP_CURRENT_VERSION 13


class GameMapChapterIface;
class GameMapStorageIface;
class GameMapObjectiveIface;
class GameMapMissionIface;
class GameMapMissionLevelIface;
class GameMapImageIface;
class GameMapInventoryIface;


/**
 * @brief The GameMapReaderIface class
 */

class GameMapReaderIface
{

public:
	explicit GameMapReaderIface();
	virtual ~GameMapReaderIface();

	void regenerateUuids();
	GameMapMissionIface *checkLockTree(QList<GameMapMissionIface *> *listPtr = nullptr) const;
	QVector<GameMapMissionLevelIface*> missionLockTree(GameMapMissionIface *mission) const;

	QByteArray toBinaryData() const;

	template<typename TIface, typename TFrom>
	static QList<TIface*> ifaceListConvert(const QList<TFrom *> &from)
	{
		QList<TIface *> list;
		list.reserve(from.size());
		foreach (TFrom *s, from)
			list.append(s);
		return list;
	}

	quint32 appVersion() const;

protected:
	bool readBinaryData(const QByteArray &data);

	virtual QList<GameMapStorageIface*> ifaceStorages() const = 0;
	virtual QList<GameMapChapterIface*> ifaceChapters() const = 0;
	virtual QList<GameMapMissionIface*> ifaceMissions() const = 0;
	virtual QList<GameMapImageIface*> ifaceImages() const = 0;

	virtual GameMapStorageIface* ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data) = 0;
	virtual GameMapChapterIface* ifaceAddChapter(const qint32 &id, const QString &name) = 0;
	virtual GameMapMissionIface* ifaceAddMission(const QByteArray &uuid, const QString &name,
												 const QString &description, const QString &medalImage,
												 const quint32 &gameModes) = 0;
	virtual GameMapImageIface* ifaceAddImage(const qint32 &id, const QByteArray &data) = 0;


	QString m_uuid;


	friend class MapImage;

private:
	bool storagesFromStream(QDataStream &stream);
	void storagesToStream(QDataStream &stream, const QList<GameMapStorageIface*> &storages) const;

	bool chaptersFromStream(QDataStream &stream);
	void chaptersToStream(QDataStream &stream, const QList<GameMapChapterIface *> &chapters) const;

	bool missionsFromStream(QDataStream &stream);
	void missionsToStream(QDataStream &stream, const QList<GameMapMissionIface*> &missions) const;

	bool imagesFromStream(QDataStream &stream);
	void imagesToStream(QDataStream &stream, const QList<GameMapImageIface*> &images) const;

	bool objectivesFromStream(QDataStream &stream, GameMapChapterIface *chapter);
	bool missionLevelsFromStream(QDataStream &stream, GameMapMissionIface *mission);
	bool inventoriesFromStream(QDataStream &stream, GameMapMissionLevelIface *chapter);

	qint32 m_version;
	quint32 m_appVersion;
};





/**
 * @brief The GameMapStorageIface class
 */

class GameMapStorageIface
{

public:
	explicit GameMapStorageIface() : m_id(-1) {}
	virtual ~GameMapStorageIface() {}

protected:
	friend class GameMapReaderIface;
	qint32 m_id;
	QString m_module;
	QVariantMap m_data;
};




/**
 * @brief The GameMapObjectiveIface class
 */


class GameMapObjectiveIface
{

public:
	explicit GameMapObjectiveIface() : m_storageId(-1), m_storageCount(0) {}
	virtual ~GameMapObjectiveIface() {}

protected:
	friend class GameMapReaderIface;
	friend class GameMapChapterIface;
	QString m_uuid;
	QString m_module;
	qint32 m_storageId;
	qint32 m_storageCount;
	QVariantMap m_data;
};




/**
 * @brief The GameMapChapterIface class
 */


class GameMapChapterIface
{

public:
	explicit GameMapChapterIface() : m_id(-1) {}
	virtual ~GameMapChapterIface() {}

protected:
	friend class GameMapReaderIface;
	friend class GameMapMissionIface;

	virtual QList<GameMapObjectiveIface*> ifaceObjectives() const = 0;
	virtual GameMapObjectiveIface* ifaceAddObjective(const QString &m_uuid,
													 const QString &m_module,
													 const qint32 &m_storageId,
													 const qint32 &m_storageCount,
													 const QVariantMap &m_data) = 0;

	qint32 m_id;
	QString m_name;

private:
	friend class GameMapReaderIface;
	void objectivesToStream(QDataStream &stream) const;
};



/**
 * @brief The GameMapMissionIface class
 */


class GameMapMissionIface
{

public:
	explicit GameMapMissionIface() {}
	virtual ~GameMapMissionIface() {}

	bool getLockTree(QVector<GameMapMissionLevelIface *> *listPtr, GameMapMissionIface *rootMission) const;

protected:
	friend class GameMapReaderIface;

	virtual QList<GameMapMissionLevelIface*> ifaceLevels() const = 0;
	virtual QList<GameMapMissionLevelIface*> ifaceLocks() const = 0;

	virtual GameMapMissionLevelIface* ifaceAddLevel(const qint32 &level,
													const QByteArray &terrain,
													const qint32 &startHP,
													const qint32 &duration,
													const bool &canDeathmatch,
													const qreal &questions,
													const QString &image) = 0;
	virtual GameMapMissionLevelIface* ifaceAddLock(const QString &uuid, const qint32 &level) = 0;


	QString m_uuid;
	QString m_name;
	QString m_description;
	QString m_medalImage;
	quint32 m_gameModes = 0;

private:
	friend class GameMapReaderIface;
	void levelsToStream(QDataStream &stream) const;
};





/**
 * @brief The GameMapMissionLevel class
 */

class GameMapMissionLevelIface
{

public:
	explicit GameMapMissionLevelIface() : m_level(-1), m_startHP(0), m_duration(0), m_canDeathmatch(false), m_questions(0) {}
	virtual ~GameMapMissionLevelIface() {}


protected:
	friend class GameMapReaderIface;
	friend class GameMapMissionIface;

	virtual GameMapMissionIface *mission() const = 0;

	virtual QList<GameMapChapterIface*> ifaceChapters() const = 0;
	virtual QList<GameMapInventoryIface*> ifaceInventories() const = 0;

	virtual bool ifaceAddChapter(const qint32 &chapterId) = 0;
	virtual GameMapInventoryIface* ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count) = 0;

	qint32 m_level;
	QString m_terrain;
	qint32 m_startHP;
	qint32 m_duration;
	QString m_image;
	bool m_canDeathmatch;
	qreal m_questions;

private:
	void inventoriesToStream(QDataStream &stream) const;
};



/**
 * @brief The GameMapInventoryIface class
 */


class GameMapInventoryIface
{

public:
	explicit GameMapInventoryIface() : m_block(-1), m_count(0) {}
	virtual ~GameMapInventoryIface() {}

protected:
	friend class GameMapMissionLevelIface;
	qint32 m_block;
	QString m_module;
	qint32 m_count;
};




/**
 * @brief The GameMapImageIface class
 */


class GameMapImageIface
{

public:
	explicit GameMapImageIface() {}
	virtual ~GameMapImageIface() {}

protected:
	friend class GameMapReaderIface;
	friend class MapImage;
	friend class GameMapEditor;
	qint32 m_id;
	QByteArray m_data;

};

#endif // GAMEMAPREADERIFACE_H
