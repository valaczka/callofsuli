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

#define GAMEMAP_CURRENT_VERSION 10




#define XP_FACTOR_TARGET_BASE	0.1
#define XP_FACTOR_LEVEL			0.5
#define XP_FACTOR_SOLVED_FIRST	2.5
#define XP_FACTOR_DEATHMATCH	2.3
#define XP_FACTOR_STREAK		0.5
#define XP_FACTOR_NEW_STREAK	1.0
#define XP_FACTOR_DURATION_SEC	0.05

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
		void setUuid(const QByteArray &uuid) { m_uuid = uuid; }
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
		BlockChapterMap();

		QVector<qint32> blocks() const { return m_blocks; };
		QVector<Chapter *> chapters() const { return m_chapters; };

		qint32 addBlock(const qint32 &block) { m_blocks.append(block); return block; }
		Chapter* addChapter(Chapter *chapter) { Q_ASSERT(chapter); m_chapters.append(chapter); return chapter;}

	private:
		QVector<qint32> m_blocks;
		QVector<Chapter *> m_chapters;
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
					 const bool &canDeathmatch, const qreal &questions,
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
		qreal questions() const { return m_questions; }

		void setMission(Mission *mission) { m_mission = mission; }
		Mission *mission() const { return m_mission; }

		bool isSolvedNormal() const { return m_solvedNormal; }
		void setIsSolvedNormal(const bool &solved) { m_solvedNormal = solved; }
		bool isSolvedDeathmatch() const { return m_solvedDeathmatch; }
		void setIsSolvedDeathmatch(const bool &solved) { m_solvedDeathmatch = solved; }

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
		qreal m_questions;

		bool m_solvedNormal;
		bool m_solvedDeathmatch;
	};


	typedef QPair<Mission *, qint32> MissionLock;

	class Mission {
	public:
		Mission(const QByteArray &uuid, const QString &name, const QString &description, const QString &medalImage);
		~Mission();

		QByteArray uuid() const { return m_uuid;};
		void setUuid(const QByteArray &uuid) { m_uuid = uuid; }

		QString name() const { return m_name;};
		QString description() const { return m_description; };
		QVector<MissionLevel *> levels() const { return m_levels; };
		QVector<MissionLock> locks() const { return m_locks; };


		QString medalImage() const { return m_medalImage; };
		void setMedalImage(const QString &image) { m_medalImage = image; }

		MissionLevel* missionLevel(const qint32 &level) const;
		MissionLevel* addMissionLevel(MissionLevel *level) { Q_ASSERT(level); m_levels.append(level); level->setMission(this); return level;}
		void addLock(Mission *mission, const qint32 &level) { Q_ASSERT(mission); m_locks.append(qMakePair<Mission *, qint32>(mission, level)); }

		bool getLockTree(QVector<MissionLock> *listPtr, Mission *rootMission = nullptr) const;

		int lockDepth() const { return m_lockDepth; }
		void setLockDepth(const int &lockDepth) { m_lockDepth = lockDepth; }

		int solvedLevel() const;


	private:
		QByteArray m_uuid;
		QString m_name;
		QString m_description;
		QVector<MissionLevel *> m_levels;
		QVector<MissionLock> m_locks;
		QString m_medalImage;
		int m_lockDepth;
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






	struct SolverInfo {
		int t1, t2, t3, d1, d2, d3;

		SolverInfo() :
			t1(0),
			t2(0),
			t3(0),
			d1(0),
			d2(0),
			d3(0)
		{}


		SolverInfo(const QVariantMap &sqlRow) {
			t1 = sqlRow.value("t1", 0).toInt();
			t2 = sqlRow.value("t2", 0).toInt();
			t3 = sqlRow.value("t3", 0).toInt();
			d1 = sqlRow.value("d1", 0).toInt();
			d2 = sqlRow.value("d2", 0).toInt();
			d3 = sqlRow.value("d3", 0).toInt();
		}


		SolverInfo(const int &_t1,
				   const int &_t2,
				   const int &_t3,
				   const int &_d1,
				   const int &_d2,
				   const int &_d3) :
			t1(_t1),
			t2(_t2),
			t3(_t3),
			d1(_d1),
			d2(_d2),
			d3(_d3)
		{}


		SolverInfo(const SolverInfo &s) {
			t1 = s.t1;
			t2 = s.t2;
			t3 = s.t3;
			d1 = s.d1;
			d2 = s.d2;
			d3 = s.d3;
		}

		inline int solved(const QString &field) const {
			if (field == "t1")
				return t1;
			else if (field == "t2")
				return t2;
			else if (field == "t3")
				return t3;
			else if (field == "d1")
				return d1;
			else if (field == "d2")
				return d2;
			else if (field == "d3")
				return d3;

			return -1;
		};

		inline int solved(const int &level, const bool &deathmatch) const {
			if (level == 1 && !deathmatch)
				return t1;
			else if (level == 2 && !deathmatch)
				return t2;
			else if (level == 3 && !deathmatch)
				return t3;
			else if (level == 1 && deathmatch)
				return d1;
			else if (level == 2 && deathmatch)
				return d2;
			else if (level == 3 && deathmatch)
				return d3;

			return -1;
		};

		inline bool hasSolved(const QString &field) const {
			return solved(field) > 0;
		}

		inline bool hasSolved(const int &level, const bool &deathmatch) const {
			return solved(level, deathmatch) > 0;
		}


		inline bool isSolvedNew(const SolverInfo &baseSolverInfo, const QString &field) const {
			return (baseSolverInfo.solved(field) != -1 && this->solved(field) > baseSolverInfo.solved(field));
		};

		inline bool isSolvedNew(const SolverInfo &baseSolverInfo, const int &level, const bool &deathmatch) const {
			return (baseSolverInfo.solved(level, deathmatch) != -1 && this->solved(level, deathmatch) > baseSolverInfo.solved(level, deathmatch));
		};

		inline bool isSolvedFirst(const SolverInfo &baseSolverInfo, const QString &field) const {
			return (baseSolverInfo.solved(field) == 0 && this->solved(field) > baseSolverInfo.solved(field));
		};

		inline bool isSolvedFirst(const SolverInfo &baseSolverInfo, const int &level, const bool &deathmatch) const {
			return (baseSolverInfo.solved(level, deathmatch) == 0 &&
					this->solved(level, deathmatch) > baseSolverInfo.solved(level, deathmatch));
		};


		inline SolverInfo solve(const int &level, const bool &deathmatch) const {
			SolverInfo p(*this);

			if (level == 1 && !deathmatch)
				p.t1++;
			else if (level == 2 && !deathmatch)
				p.t2++;
			else if (level == 3 && !deathmatch)
				p.t3++;
			else if (level == 1 && deathmatch)
				p.d1++;
			else if (level == 2 && deathmatch)
				p.d2++;
			else if (level == 3 && deathmatch)
				p.d3++;

			return p;
		}

		inline SolverInfo solve(const QString &field) const {
			SolverInfo p(*this);

			if (field == "t1")
				p.t1++;
			else if (field == "t2")
				p.t2++;
			else if (field == "t3")
				p.t3++;
			else if (field == "d1")
				p.d1++;
			else if (field == "d2")
				p.d2++;
			else if (field == "d3")
				p.d3++;

			return p;
		}



		inline QJsonObject toJsonObject() const {
			QJsonObject o;
			o["t1"] = t1;
			o["t2"] = t2;
			o["t3"] = t3;
			o["d1"] = d1;
			o["d2"] = d2;
			o["d3"] = d3;
			return o;
		};

		inline QVariantMap toVariantMap() const {
			QVariantMap o;
			o["t1"] = t1;
			o["t2"] = t2;
			o["t3"] = t3;
			o["d1"] = d1;
			o["d2"] = d2;
			o["d3"] = d3;
			return o;
		};
	};


	static qreal computeSolvedXpFactor(const SolverInfo &baseSolver, const int &level, const bool &deathmatch);


	GameMap(const QByteArray &uuid = QByteArray());
	~GameMap();

	static GameMap* fromBinaryData(const QByteArray &data, QObject *target = nullptr, const QString &func = QString());
	QByteArray toBinaryData() const;

	static GameMap* fromDb(CosDb *db, QObject *target = nullptr, const QString &func = QString(), const bool &withImages = true);
	bool toDb(CosDb *db) const;

	bool imagesToDb(CosDb *db) const;
	void deleteImages();

	void setUuid(const QByteArray &uuid) { m_uuid = uuid; }

	void regenerateUuids();

	QByteArray uuid() const { return m_uuid; }
	QVector<Mission *> missions() const { return m_missions; }
	QVector<Chapter *> chapters() const { return m_chapters; }
	QVector<Image *> images() const { return m_images; }
	QVector<Storage *> storages() const { return m_storages; };
	QVector<MissionLevelData> missionLevelsData() const;

	Chapter *chapter(const qint32 &id) const;
	Mission *mission(const QByteArray &uuid) const;
	MissionLevel *missionLevel(const QByteArray &uuid, const qint32 &level) const;
	Objective *objective(const QByteArray &uuid) const;
	Storage *storage(const qint32 &id) const;

	typedef QHash<Mission *, QVector<MissionLock>> MissionLockHash;
	MissionLockHash missionLockTree(Mission **errMission = nullptr) const;

	Chapter* addChapter(Chapter *chapter) { Q_ASSERT(chapter); m_chapters.append(chapter); return chapter; }
	Image* addImage(Image *image) { Q_ASSERT(image); m_images.append(image); return image; }
	Storage* addStorage(Storage *storage) { Q_ASSERT(storage); m_storages.append(storage); return storage;}

	typedef QPair<MissionLevel *, bool> MissionLevelDeathmatch;

	void setSolver(const QVariantList &list);
	QVector<MissionLevelDeathmatch> getUnlocks(const QByteArray &uuid, const qint32 &level, const bool &deathmatch) const;
	MissionLevelDeathmatch getNextMissionLevel(const QByteArray &uuid, const qint32 &level, const bool &deathmatch) const;

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

	bool missionsToStream(QDataStream &stream) const ;
	bool missionsFromStream(QDataStream &stream, const qint32 &version);
	bool missionsToDb(CosDb *db) const;
	bool missionsFromDb(CosDb *db);
	bool missionLocksToDb(CosDb *db) const;

	void missionLevelsToStream(Mission *mission, QDataStream &stream) const;
	bool missionLevelsFromStream(Mission* mission, QDataStream &stream, const qint32 &version);
	bool missionLevelsToDb(CosDb *db) const;
	bool missionLevelsFromDb(CosDb *db);

	void blockChapterMapsToStream(MissionLevel *level, QDataStream &stream) const;
	bool blockChapterMapsFromStream(MissionLevel* level, QDataStream &stream, const qint32 &version);
	bool blockChapterMapsToDb(CosDb *db) const;
	bool blockChapterMapsFromDb(CosDb *db);

	void inventoriesToStream(MissionLevel *level, QDataStream &stream) const;
	bool inventoriesFromStream(MissionLevel* level, QDataStream &stream);
	bool inventoriesToDb(CosDb *db) const;
	bool inventoriesFromDb(CosDb *db);

	void imagesToStream(QDataStream &stream) const ;
	bool imagesFromStream(QDataStream &stream);
	void imagesFromDb(CosDb *db);

	bool _deprecated_campaignsFromStream(QDataStream &stream, const qint32 &);

	inline bool sendProgress(const qreal &progress) const;
	inline static bool sendProgress(QObject *target, const QString &func, const qreal &progress);

	QByteArray m_uuid;
	QVector<Chapter *> m_chapters;
	QVector<Image *> m_images;
	QVector<Storage *> m_storages;
	QVector<Mission *> m_missions;

	QObject *m_progressObject;
	QString m_progressFunc;





};

#endif // GAMEMAP_H
