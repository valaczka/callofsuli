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

#define GAMEMAP_CURRENT_VERSION 8


/**
 * @brief The GameMap class
 */


class GameMap
{

public:
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
		Objective(const QByteArray &uuid, const QByteArray &module, Storage *storage, const int &storageCount, const QVariantMap &data);

		QByteArray uuid() const { return m_uuid; };
		QByteArray module() const { return m_module; };
		Storage *storage() const { return m_storage; };
		int storageCount() const { return m_storageCount; };
		QVariantMap data() const { return m_data; };

	private:
		QByteArray m_uuid;
		QByteArray m_module;
		Storage *m_storage;
		int m_storageCount;
		QVariantMap m_data;
	};



	class Chapter {
	public:
		Chapter(const qint32 &id, const QString &name);
		~Chapter();

		qint32 id() const { return m_id; };
		QString name() const { return m_name; };
		QVector<Objective *> objectives() const { return m_objectives; };

		Objective* addObjective(Objective *objective) { Q_ASSERT(objective); m_objectives.append(objective); return objective;}

	private:
		qint32 m_id;
		QString m_name;
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


	class Mission;

	class MissionLevel {
	public:
		MissionLevel(const qint32 &level, const QByteArray &terrain,
					 const qint32 &startHP, const qint32 &duration, const qint32 &startBlock,
					 const bool &canDeathmatch,
					 const QString &imageFolder, const QString &imageFile);
		~MissionLevel();

		qint32 level() const { return m_level; };
		QByteArray terrain() const { return m_terrain; };
		qint32 startHP() const { return m_startHP; };
		qint32 duration() const { return m_duration; };
		qint32 startBlock() const { return m_startBlock; };
		QVector<BlockChapterMap *> blockChapterMaps() const { return m_blockChapterMaps; };
		QVector<Inventory *> inventories() const { return m_inventories; };
		QString imageFolder() const { return m_imageFolder; }
		QString imageFile() const { return m_imageFile; }
		bool canDeathmatch() const { return m_canDeathmatch; }

		void setMission(Mission *mission) { m_mission = mission; }
		Mission *mission() const { return m_mission; }


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
		QString m_imageFolder;
		QString m_imageFile;
		Mission *m_mission;
		bool m_canDeathmatch;
	};


	typedef QPair<Mission *, qint32> MissionLock;

	class Mission {
	public:
		Mission(const QByteArray &uuid, const bool &mandatory, const QString &name, const QString &description, const QString &medalImage);
		~Mission();

		QByteArray uuid() const { return m_uuid;};
		bool mandatory() const { return m_mandatory;};
		QString name() const { return m_name;};
		QString description() const { return m_description; };
		QVector<MissionLevel *> levels() const { return m_levels; };
		QVector<MissionLock> locks() const { return m_locks; };

		QString medalImage() const { return m_medalImage; };
		void setMedalImage(const QString &image) { m_medalImage = image; }

		MissionLevel* addMissionLevel(MissionLevel *level) { Q_ASSERT(level); m_levels.append(level); level->setMission(this); return level;}
		void addLock(Mission *mission, const qint32 &level) { Q_ASSERT(mission); m_locks.append(qMakePair<Mission *, qint32>(mission, level)); }

		bool getLockTree(QVector<MissionLock> *listPtr, Mission *rootMission = nullptr) const;

		qint32 getSolvedLevel() const { return m_solvedLevel; }
		void setSolvedLevel(const qint32 &solvedLevel) { m_solvedLevel = solvedLevel; }

		bool getTried() const { return m_tried; }
		void setTried(bool tried) { m_tried = tried; }

		qint32 getLockDepth() const { return m_lockDepth; }
		void setLockDepth(const qint32 &lockDepth) { m_lockDepth = lockDepth; }

	private:
		QByteArray m_uuid;
		bool m_mandatory;
		QString m_name;
		QString m_description;
		QVector<MissionLevel *> m_levels;
		QVector<MissionLock> m_locks;
		qint32 m_solvedLevel;
		bool m_tried;
		qint32 m_lockDepth;
		QString m_medalImage;
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

		bool getLockTree(QVector<Campaign *> *listPtr, Campaign *rootCampaign = nullptr) const;

		bool getSolved() const { return m_solved; }
		void setSolved(bool solved) { m_solved = solved; }

		bool getTried() const { return m_tried; }
		void setTried(bool tried) { m_tried = tried; }

		bool getLocked() const { return m_locked; }
		void setLocked(bool locked) { m_locked = locked; }

	private:
		qint32 m_id;
		QString m_name;
		QVector<Campaign *> m_locks;
		QVector<Mission *> m_missions;
		bool m_solved;
		bool m_tried;
		bool m_locked;
	};



	class MissionLevelData {
	public:

		MissionLevel *m_missionLevel;
		bool m_deathmatch;

		MissionLevelData(MissionLevel *missionLevel = nullptr, const bool &deatchmatch = false)
			: m_missionLevel(missionLevel)
			, m_deathmatch(deatchmatch)
		{}

		MissionLevel *missionLevel() const { return m_missionLevel; }
		bool deathmatch() const { return m_deathmatch; }

		Mission *mission() const { return (m_missionLevel ? m_missionLevel->mission() : nullptr); }

		friend inline bool operator== (const MissionLevelData &b1, const MissionLevelData &b2) {
			return b1.m_missionLevel == b2.m_missionLevel
					&& b1.m_deathmatch == b2.m_deathmatch;
		}
	};



	GameMap(const QByteArray &uuid = QByteArray());
	~GameMap();

	static GameMap* fromBinaryData(const QByteArray &data, QObject *target = nullptr, const QString &func = QString());
	QByteArray toBinaryData() const;

	static GameMap* fromDb(CosDb *db, QObject *target = nullptr, const QString &func = QString(), const bool &withImages = true);
	bool toDb(CosDb *db) const;

	bool imagesToDb(CosDb *db) const;
	void deleteImages();

	QByteArray uuid() const { return m_uuid; }
	QVector<Campaign *> campaigns() const { return m_campaigns; }
	QVector<Mission *> orphanMissions() const { return m_orphanMissions; }
	QVector<Chapter *> chapters() const { return m_chapters; }
	QVector<Image *> images() const { return m_images; }
	QVector<Storage *> storages() const { return m_storages; };
	QVector<MissionLevelData> missionLevelsData() const;

	Campaign *campaign(const qint32 &id) const;
	Chapter *chapter(const qint32 &id) const;
	Mission *mission(const QByteArray &uuid) const;
	MissionLevel *missionLevel(const QByteArray &uuid, const qint32 &level) const;
	Objective *objective(const QByteArray &uuid) const;
	Storage *storage(const qint32 &id) const;

	typedef QHash<Campaign *, QVector<Campaign *>> CampaignLockHash;
	typedef QHash<Mission *, QVector<MissionLock>> MissionLockHash;
	MissionLockHash missionLockTree(Mission **errMission = nullptr) const;
	CampaignLockHash campaignLockTree(Campaign **errCampaign = nullptr) const;

	Chapter* addChapter(Chapter *chapter) { Q_ASSERT(chapter); m_chapters.append(chapter); return chapter; }
	Campaign* addCampaign(Campaign *campaign) { Q_ASSERT(campaign); m_campaigns.append(campaign); return campaign; }
	Image* addImage(Image *image) { Q_ASSERT(image); m_images.append(image); return image; }
	Storage* addStorage(Storage *storage) { Q_ASSERT(storage); m_storages.append(storage); return storage;}

	void setSolver(const QVariantList &list);

	void setProgressFunc(QObject *target, const QString &func);

private:
	void chaptersToStream(QDataStream &stream) const ;
	bool chaptersFromStream(QDataStream &stream, const qint32 &version);
	bool chaptersToDb(CosDb *db) const;
	void chaptersFromDb(CosDb *db);

	void storagesToStream(QDataStream &stream) const ;
	bool storagesFromStream(QDataStream &stream);
	bool storagesToDb(CosDb *db) const;
	void storagesFromDb(CosDb *db);

	void objectivesToStream(Chapter* chapter, QDataStream &stream) const ;
	bool objectivesFromStream(Chapter* chapter, QDataStream &stream, const qint32 &version);
	bool objectivesToDb(CosDb *db) const;
	bool objectivesFromDb(CosDb *db);

	void campaignsToStream(QDataStream &stream) const ;
	bool campaignsFromStream(QDataStream &stream, const qint32 &version);
	bool campaignsToDb(CosDb *db) const;
	bool campaingsFromDb(CosDb *db);

	bool missionsToStream(Campaign* campaign, QDataStream &stream) const ;
	QHash<QByteArray, QList<QPair<QByteArray, qint32>>> missionsFromStream(Campaign* campaign, QDataStream &stream, const qint32 &version);
	bool missionsToDb(CosDb *db) const;
	bool missionsFromDb(CosDb *db);
	bool missionLocksToDb(CosDb *db) const;

	void missionLevelsToStream(Mission *mission, QDataStream &stream) const;
	bool missionLevelsFromStream(Mission* mission, QDataStream &stream, const qint32 &version);
	bool missionLevelsToDb(CosDb *db) const;
	bool missionLevelsFromDb(CosDb *db);

	void blockChapterMapsToStream(MissionLevel *level, QDataStream &stream) const;
	bool blockChapterMapsFromStream(MissionLevel* level, QDataStream &stream);
	bool blockChapterMapsToDb(CosDb *db) const;
	bool blockChapterMapsFromDb(CosDb *db);

	void inventoriesToStream(MissionLevel *level, QDataStream &stream) const;
	bool inventoriesFromStream(MissionLevel* level, QDataStream &stream);
	bool inventoriesToDb(CosDb *db) const;
	bool inventoriesFromDb(CosDb *db);

	void imagesToStream(QDataStream &stream) const ;
	bool imagesFromStream(QDataStream &stream);
	void imagesFromDb(CosDb *db);

	inline bool sendProgress(const qreal &progress) const;
	inline static bool sendProgress(QObject *target, const QString &func, const qreal &progress);

	QByteArray m_uuid;
	QVector<Campaign *> m_campaigns;
	QVector<Chapter *> m_chapters;
	QVector<Image *> m_images;
	QVector<Storage *> m_storages;
	QVector<Mission *> m_orphanMissions;

	QObject *m_progressObject;
	QString m_progressFunc;





};

#endif // GAMEMAP_H
