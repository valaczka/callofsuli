/*
 * ---- Call of Suli ----
 *
 * gamemap.h
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

#ifndef GAMEMAP_H
#define GAMEMAP_H


#include <QObject>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include "gamemapreaderiface.h"


#define XP_FACTOR_TARGET_BASE	0.1
#define XP_FACTOR_LEVEL			0.5
#define XP_FACTOR_SOLVED_FIRST	2.5
#define XP_FACTOR_DEATHMATCH	2.3
#define XP_FACTOR_STREAK		0.5
#define XP_FACTOR_NEW_STREAK	1.0
#define XP_FACTOR_DURATION_SEC	0.05


class GameMapStorage;
class GameMapImage;
class GameMapChapter;
class GameMapObjective;
class GameMapMission;
class GameMapMissionLevel;
class GameMapInventory;



/**
 * @brief The GameMap class
 */


class GameMap : public GameMapReaderIface
{
	Q_GADGET

	Q_PROPERTY(QString uuid MEMBER m_uuid)
	Q_PROPERTY(QList<GameMapStorage *> storages MEMBER m_storages)
	Q_PROPERTY(QList<GameMapImage *> images MEMBER m_images)
	Q_PROPERTY(QList<GameMapChapter *> chapters MEMBER m_chapters)
	Q_PROPERTY(QList<GameMapMission *> missions MEMBER m_missions)

public:
	explicit GameMap();
	virtual ~GameMap();


	const QString &uuid() const;
	const QList<GameMapStorage *> &storages() const;
	const QList<GameMapImage *> &images() const;
	const QList<GameMapChapter *> &chapters() const;
	const QList<GameMapMission *> &missions() const;

	GameMapStorage *storage(const qint32 &id) const;
	GameMapImage *image(const qint32 &id) const;
	GameMapChapter *chapter(const qint32 &id) const;
	GameMapMission *mission(const QString &uuid) const;
	GameMapMissionLevel *missionLevel(const QString &uuid, const qint32 &level) const;
	GameMapObjective *objective(const QString &uuid) const;

	static GameMap *fromBinaryData(const QByteArray &data);


	// Solver methods

	struct SolverInfo;


	typedef QPair<GameMapMissionLevel *, bool> MissionLevelDeathmatch;

	void setSolver(const QVariantList &list);
	QVector<MissionLevelDeathmatch> getUnlocks(const QString &uuid,
											   const qint32 &level,
											   const bool &deathmatch) const;
	MissionLevelDeathmatch getNextMissionLevel(const QString &uuid,
											   const qint32 &level,
											   const bool &deathmatch,
											   const bool &lite) const;
	static qreal computeSolvedXpFactor(const SolverInfo &baseSolver,
									   const int &level,
									   const bool &deathmatch, const bool &isLite);


protected:
	QList<GameMapStorageIface *> ifaceStorages() const override
	{ return ifaceListConvert<GameMapStorageIface, GameMapStorage>(m_storages);	}

	QList<GameMapChapterIface*> ifaceChapters() const override
	{ return ifaceListConvert<GameMapChapterIface, GameMapChapter>(m_chapters); }

	QList<GameMapMissionIface*> ifaceMissions() const override
	{ return ifaceListConvert<GameMapMissionIface, GameMapMission>(m_missions);	}

	QList<GameMapImageIface*> ifaceImages() const override
	{ return ifaceListConvert<GameMapImageIface, GameMapImage>(m_images);	}

	GameMapStorageIface* ifaceAddStorage(const qint32 &id, const QString &module, const QVariantMap &data) override;
	GameMapChapterIface* ifaceAddChapter(const qint32 &id, const QString &name) override;
	GameMapMissionIface* ifaceAddMission(const QByteArray &uuid, const QString &name,
												 const QString &description, const QString &medalImage) override;
	GameMapImageIface* ifaceAddImage(const qint32 &id, const QByteArray &data) override;

private:
	QList<GameMapStorage *> m_storages;
	QList<GameMapImage *> m_images;
	QList<GameMapChapter *> m_chapters;
	QList<GameMapMission *> m_missions;
};




/**
 * @brief The GameMapStorage class
 */


class GameMapStorage : public GameMapStorageIface
{
	Q_GADGET

	Q_PROPERTY(qint32 id MEMBER m_id)
	Q_PROPERTY(QString module MEMBER m_module)
	Q_PROPERTY(QVariantMap data MEMBER m_data)

public:
	explicit GameMapStorage(const qint32 &id, const QString &module, const QVariantMap &data);
	virtual ~GameMapStorage() {}

	qint32 id() const;
	const QString &module() const;
	const QVariantMap &data() const;

};




/**
 * @brief The GameMapImage class
 */

class GameMapImage : public GameMapImageIface
{
	Q_GADGET

	Q_PROPERTY(qint32 id MEMBER m_id)
	Q_PROPERTY(QByteArray data MEMBER m_data STORED false)

public:
	explicit GameMapImage(const qint32 &id, const QByteArray &data);
	virtual ~GameMapImage() {}

	const qint32 &id() const;
	const QByteArray &data() const;
};







/**
 * @brief The GameMapChapter class
 */


class GameMapChapter : public GameMapChapterIface
{
	Q_GADGET

	Q_PROPERTY(qint32 id MEMBER m_id)
	Q_PROPERTY(QString name MEMBER m_name)
	Q_PROPERTY(QList<GameMapObjective *> objectives MEMBER m_objectives)

public:
	explicit GameMapChapter(const qint32 &id, const QString &name, GameMap *map);
	virtual ~GameMapChapter();

	qint32 id() const;
	const QString &name() const;
	const QList<GameMapObjective *> &objectives() const;

protected:
	QList<GameMapObjectiveIface*> ifaceObjectives() const override
	{ return GameMapReaderIface::ifaceListConvert<GameMapObjectiveIface, GameMapObjective>(m_objectives);	}

	GameMapObjectiveIface* ifaceAddObjective(const QString &uuid,
													 const QString &module,
													 const qint32 &storageId,
													 const qint32 &storageCount,
													 const QVariantMap &data) override;

private:
	QList<GameMapObjective *> m_objectives;
	GameMap *m_map;
};





/**
 * @brief The GameMapObjective class
 */

class GameMapObjective : public GameMapObjectiveIface
{
	Q_GADGET

	Q_PROPERTY(QString uuid MEMBER m_uuid)
	Q_PROPERTY(QString module MEMBER m_module)
	Q_PROPERTY(qint32 storageId MEMBER m_storageId)
	Q_PROPERTY(qint32 storageCount MEMBER m_storageCount)
	Q_PROPERTY(QVariantMap data MEMBER m_data)

public:
	explicit GameMapObjective(const QString &uuid, const QString &module,
							  const qint32 &storageId, const qint32 &storageCount,
							  const QVariantMap &data, GameMap *map);
	virtual ~GameMapObjective() {}

	const QString &uuid() const;
	const QString &module() const;
	qint32 storageId() const;
	qint32 storageCount() const;
	const QVariantMap &data() const;

	GameMapStorage *storage() const;

	void setGeneratedQuestions(const QVariantList &newGeneratedQuestions);
	bool hasGeneratedQuestion() const;
	QVariantMap takeQuestion();

private:
	GameMap *m_map;
	QVariantList m_generatedQuestions;
};





/**
 * @brief The GameMapMission class
 */


class GameMapMission : public GameMapMissionIface
{
	Q_GADGET

	Q_PROPERTY(QString uuid MEMBER m_uuid)
	Q_PROPERTY(QString name MEMBER m_name)
	Q_PROPERTY(QString description MEMBER m_description)
	Q_PROPERTY(QString medalImage MEMBER m_medalImage)
	Q_PROPERTY(QList<GameMapMissionLevel *> levels MEMBER m_levels)
	Q_PROPERTY(QList<GameMapMissionLevel *> locks MEMBER m_locks)

public:
	explicit GameMapMission(const QByteArray &uuid, const QString &name, const QString &description, const QString &medalImage, GameMap *map);
	virtual ~GameMapMission() {}

	const QString &uuid() const;
	const QString &name() const;
	const QString &description() const;
	const QString &medalImage() const;
	const QList<GameMapMissionLevel *> &levels() const;
	const QList<GameMapMissionLevel *> &locks() const;

	GameMapMissionLevel *level(const qint32 &num) const;

	qint32 solvedLevel() const;

	int lockDepth() const;
	void setLockDepth(int newLockDepth);

protected:
	QList<GameMapMissionLevelIface*> ifaceLevels() const override
	{ return GameMapReaderIface::ifaceListConvert<GameMapMissionLevelIface, GameMapMissionLevel>(m_levels);	}

	QList<GameMapMissionLevelIface*> ifaceLocks() const override
	{ return GameMapReaderIface::ifaceListConvert<GameMapMissionLevelIface, GameMapMissionLevel>(m_locks); }

	virtual GameMapMissionLevelIface* ifaceAddLevel(const qint32 &level,
													const QByteArray &terrain,
													const qint32 &startHP,
													const qint32 &duration,
													const bool &canDeathmatch,
													const qreal &questions,
													const QString &image) override;
	virtual GameMapMissionLevelIface* ifaceAddLock(const QString &uuid, const qint32 &level) override;

private:
	QList<GameMapMissionLevel *> m_levels;
	QList<GameMapMissionLevel *> m_locks;
	GameMap *m_map;
	int m_lockDepth;
};






/**
 * @brief The GameMapMissionLevel class
 */

class GameMapMissionLevel : public GameMapMissionLevelIface
{
	Q_GADGET

	Q_PROPERTY(qint32 level MEMBER m_level)
	Q_PROPERTY(QString terrain MEMBER m_terrain)
	Q_PROPERTY(qint32 startHP MEMBER m_startHP)
	Q_PROPERTY(qint32 duration MEMBER m_duration)
	Q_PROPERTY(QString image MEMBER m_image)
	Q_PROPERTY(bool canDeathmatch MEMBER m_canDeathmatch)
	Q_PROPERTY(qreal questions MEMBER m_questions)
	Q_PROPERTY(QList<GameMapInventory *> inventories MEMBER m_inventories)
	Q_PROPERTY(QList<qint32> chapterIds MEMBER m_chapterIds)

	Q_PROPERTY(bool solvedNormal MEMBER m_solvedNormal STORED false)
	Q_PROPERTY(bool solvedDeathmatch MEMBER m_solvedDeathmatch STORED false)

public:
	explicit GameMapMissionLevel(const qint32 &level, const QByteArray &terrain, const qint32 &startHP,
								 const qint32 &duration, const bool &canDeathmatch, const qreal &questions,
								 const QString &image, GameMapMission *mission, GameMap *map);
	virtual ~GameMapMissionLevel() {}

	qint32 level() const;
	const QString &terrain() const;
	qint32 startHP() const;
	qint32 duration() const;
	const QString &image() const;
	bool canDeathmatch() const;
	qreal questions() const;
	const QList<GameMapInventory *> &inventories() const;
	const QList<qint32> &chapterIds() const;

	GameMapMission *mission() const;
	QList<GameMapChapter *> chapters() const;

	bool solvedNormal() const;
	void setSolvedNormal(bool newSolvedNormal);

	bool solvedDeathmatch() const;
	void setSolvedDeathmatch(bool newSolvedDeathmatch);

protected:
	QList<GameMapChapterIface*> ifaceChapters() const
	{ return GameMapReaderIface::ifaceListConvert<GameMapChapterIface, GameMapChapter>(chapters());	}
	QList<GameMapInventoryIface*> ifaceInventories() const
	{ return GameMapReaderIface::ifaceListConvert<GameMapInventoryIface, GameMapInventory>(m_inventories);	}

	bool ifaceAddChapter(const qint32 &chapterId);
	GameMapInventoryIface* ifaceAddInventory(const qint32 &block, const QString &module, const qint32 &count);

private:
	QList<GameMapInventory *> m_inventories;
	QList<qint32> m_chapterIds;
	GameMap *m_map;
	GameMapMission *m_mission;
	bool m_solvedNormal;
	bool m_solvedDeathmatch;
};







/**
 * @brief The GameMapInventory class
 */


class GameMapInventory : public GameMapInventoryIface
{
	Q_GADGET

	Q_PROPERTY(qint32 block MEMBER m_block)
	Q_PROPERTY(QString module MEMBER m_module)
	Q_PROPERTY(qint32 count MEMBER m_count)

public:
	explicit GameMapInventory(const qint32 &block, const QString &module, const qint32 &count);
	virtual ~GameMapInventory() {}

	qint32 block() const;
	const QString &module() const;
	qint32 count() const;
};



/**
 * @brief The GameMap::SolverInfo struct
 */


struct GameMap::SolverInfo {
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



#endif // GAMEMAPNEW_H
