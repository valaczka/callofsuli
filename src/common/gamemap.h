/*
 * ---- Call of Suli ----
 *
 * gamemap.h
 *
 * Created on: 2020. 11. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMap
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <QVariant>
#include <QDebug>
#include "cosdb.h"

class GameMap
{

public:

	class Storage {
	public:
		Storage(const qint32 &id, const QByteArray &module, const QVariantMap &data);

		qint32 id() const { return m_id; };
		QByteArray module() const { return m_module; };
		QVariantMap data() const { return m_data; };

	private:
		qint32 m_id;
		QByteArray m_module;
		QVariantMap m_data;
	};



	class Objective {
	public:
		Objective(const QByteArray &uuid, const QByteArray &module, Storage *storage, const QVariantMap &data);

		QByteArray uuid() const { return m_uuid; };
		QByteArray module() const { return m_module; };
		Storage *storage() const { return m_storage; };
		QVariantMap data() const { return m_data; };

	private:
		QByteArray m_uuid;
		QByteArray m_module;
		Storage *m_storage;
		QVariantMap m_data;
	};



	class Chapter {
	public:
		Chapter(const qint32 &id, const QString &name);
		~Chapter();

		qint32 id() const { return m_id; };
		QString name() const { return m_name; };
		QVector<Storage *> storages() const { return m_storages; };
		QVector<Objective *> objectives() const { return m_objectives; };

		Storage *storage(const qint32 &id) const;

		Storage* addStorage(Storage *storage) { Q_ASSERT(storage); m_storages.append(storage); return storage;}
		Objective* addObjective(Objective *objective) { Q_ASSERT(objective); m_objectives.append(objective); return objective;}

	private:
		qint32 m_id;
		QString m_name;
		QVector<Storage *> m_storages;
		QVector<Objective *> m_objectives;
	};



	class BlockChapterMap {
	public:
		BlockChapterMap(const qint32 &maxObjective);

		QVector<qint32> blocks() const { return m_blocks; };
		QVector<Chapter *> chapters() const { return m_chapters; };
		QVector<Objective *> favorites() const { return m_favorites; };
		qint32 maxObjective() const { return m_maxObjective; };

		qint32 addBlock(const qint32 &block) { m_blocks.append(block); return block; }
		Chapter* addChapter(Chapter *chapter) { Q_ASSERT(chapter); m_chapters.append(chapter); return chapter;}
		Objective* addFavorite(Objective *objective) { Q_ASSERT(objective); m_favorites.append(objective); return objective;}

	private:
		QVector<qint32> m_blocks;
		QVector<Chapter *> m_chapters;
		QVector<Objective *> m_favorites;
		qint32 m_maxObjective;
	};



	class Inventory {
	public:
		Inventory(const qint32 &block, const QByteArray &module, const qint32 &count);

		qint32 block() const { return m_block; };
		QByteArray module() const { return m_module; };
		qint32 count() const { return m_count; };

	private:
		qint32 m_block;
		QByteArray m_module;
		qint32 m_count;
	};



	class MissionLevel {
	public:
		MissionLevel(const qint32 &level, const QByteArray &terrain,
					 const qint32 &startHP, const qint32 &duration, const qint32 &startBlock);
		~MissionLevel();

		qint32 level() const { return m_level; };
		QByteArray terrain() const { return m_terrain; };
		qint32 startHP() const { return m_startHP; };
		qint32 duration() const { return m_duration; };
		qint32 startBlock() const { return m_startBlock; };
		QVector<BlockChapterMap *> blockChapterMaps() const { return m_blockChapterMaps; };
		QVector<Inventory *> inventories() const { return m_inventories; };

		BlockChapterMap* addBlockChapterMap(BlockChapterMap *map) { Q_ASSERT(map); m_blockChapterMaps.append(map); return map;}
		Inventory* addInvetory(Inventory *inventory) { Q_ASSERT(inventory); m_inventories.append(inventory); return inventory;}

	private:
		qint32 m_level;
		QByteArray m_terrain;
		qint32 m_startHP;
		qint32 m_duration;
		qint32 m_startBlock;
		QVector<BlockChapterMap *> m_blockChapterMaps;
		QVector<Inventory *> m_inventories;
	};


	class Mission;
	typedef QPair<Mission *, qint32> MissionLock;

	class Mission {
	public:
		Mission(const QByteArray &uuid, const bool &mandatory, const QString &name);
		~Mission();

		QByteArray uuid() const { return m_uuid;};
		bool mandatory() const { return m_mandatory;};
		QString name() const { return m_name;};
		QVector<MissionLevel *> levels() const { return m_levels; };
		QVector<MissionLock> locks() const { return m_locks; };

		MissionLevel* addMissionLevel(MissionLevel *level) { Q_ASSERT(level); m_levels.append(level); return level;}
		void addLock(Mission *mission, const qint32 &level) { Q_ASSERT(mission); m_locks.append(qMakePair<Mission *, qint32>(mission, level)); }

		bool getLockTree(QVector<MissionLock> *listPtr, Mission *rootMission = nullptr);

	private:
		QByteArray m_uuid;
		bool m_mandatory;
		QString m_name;
		QVector<MissionLevel *> m_levels;
		QVector<MissionLock> m_locks;
	};



	class Campaign {
	public:
		Campaign(const qint32 &id, const QString &name);
		~Campaign();

		qint32 id() const { return m_id;};
		QString name() const { return m_name;};
		QVector<Campaign *> locks() const { return m_locks;};
		QVector<Mission *> missions() const { return m_missions;};

		Mission* addMission(Mission *mission) { Q_ASSERT(mission); m_missions.append(mission); return mission;}
		void addLock(Campaign *campaign) { Q_ASSERT(campaign); m_locks.append(campaign); }

	private:
		qint32 m_id;
		QString m_name;
		QVector<Campaign *> m_locks;
		QVector<Mission *> m_missions;
	};



	class Image {
	public:
		Image(const QString &folder, const QString &file, const QByteArray &data);

		QString folder() const { return m_folder;};
		QString file() const { return  m_file; };
		QByteArray data() const { return m_data; };

	private:
		QString m_folder;
		QString m_file;
		QByteArray m_data;
	};



	GameMap(const QByteArray &uuid = QByteArray(), const QString &name = QString());
	~GameMap();

	static GameMap* fromBinaryData(const QByteArray &data, QObject *target = nullptr, const QString &func = QString());
	QByteArray toBinaryData() const;

	static GameMap* fromDb();

	static GameMap* fromDb(CosDb *db, QObject *target = nullptr, const QString &func = QString());
	bool toDb(CosDb *db) const;

	bool imagesToDb(CosDb *db) const;
	void deleteImages();

	QByteArray uuid() const { return m_uuid; }
	QString name() const { return m_name; }
	QVector<Campaign *> campaigns() const { return m_campaigns; }
	QVector<Chapter *> chapters() const { return m_chapters; }
	QVector<Image *> images() const { return m_images; }

	Campaign *campaign(const qint32 &id) const;
	Chapter *chapter(const qint32 &id) const;
	Mission *mission(const QByteArray &uuid) const;
	MissionLevel *missionLevel(const QByteArray &uuid, const qint32 &level) const;
	Objective *objective(const QByteArray &uuid) const;

	typedef QHash<QByteArray, QVector<MissionLock>> MissionLockHash;
	MissionLockHash lockTree(bool *errPtr = nullptr) const;

	Chapter* addChapter(Chapter *chapter) { Q_ASSERT(chapter); m_chapters.append(chapter); return chapter; }
	Campaign* addCampaign(Campaign *campaign) { Q_ASSERT(campaign); m_campaigns.append(campaign); return campaign; }
	Image* addImage(Image *image) { Q_ASSERT(image); m_images.append(image); return image; }

	void setProgressFunc(QObject *target, const QString &func);

private:
	void chaptersToStream(QDataStream &stream) const ;
	void chaptersFromStream(QDataStream &stream);
	void chaptersToDb(CosDb *db) const;
	void chaptersFromDb(CosDb *db);

	void storagesToStream(Chapter* chapter, QDataStream &stream) const ;
	void storagesFromStream(Chapter* chapter, QDataStream &stream);
	void storagesToDb(CosDb *db) const;
	void storagesFromDb(CosDb *db);

	void objectivesToStream(Chapter* chapter, QDataStream &stream) const ;
	void objectivesFromStream(Chapter* chapter, QDataStream &stream);
	void objectivesToDb(CosDb *db) const;
	void objectivesFromDb(CosDb *db);

	void campaignsToStream(QDataStream &stream) const ;
	void campaignsFromStream(QDataStream &stream);
	void campaignsToDb(CosDb *db) const;
	void campaingsFromDb(CosDb *db);

	void missionsToStream(Campaign* campaign, QDataStream &stream) const ;
	QHash<QByteArray, QList<QPair<QByteArray, qint32>>> missionsFromStream(Campaign* campaign, QDataStream &stream);
	void missionsToDb(CosDb *db) const;
	void missionsFromDb(CosDb *db);

	void missionLevelsToStream(Mission *mission, QDataStream &stream) const;
	void missionLevelsFromStream(Mission* mission, QDataStream &stream);
	void missionLevelsToDb(CosDb *db) const;
	void missionLevelsFromDb(CosDb *db);

	void blockChapterMapsToStream(MissionLevel *level, QDataStream &stream) const;
	void blockChapterMapsFromStream(MissionLevel* level, QDataStream &stream);
	void blockChapterMapsToDb(CosDb *db) const;
	void blockChapterMapsFromDb(CosDb *db);

	void inventoriesToStream(MissionLevel *level, QDataStream &stream) const;
	void inventoriesFromStream(MissionLevel* level, QDataStream &stream);
	void inventoriesToDb(CosDb *db) const;
	void inventoriesFromDb(CosDb *db);

	void imagesToStream(QDataStream &stream) const ;
	void imagesFromStream(QDataStream &stream);
	void imagesFromDb(CosDb *db);

	inline void sendProgress(const qreal &progress) const;
	inline static void sendProgress(QObject *target, const QString &func, const qreal &progress);

	QByteArray m_uuid;
	QString m_name;
	QVector<Campaign *> m_campaigns;
	QVector<Chapter *> m_chapters;
	QVector<Image *> m_images;

	QObject *m_progressObject;
	QString m_progressFunc;
};




#endif // GAMEMAP_H
