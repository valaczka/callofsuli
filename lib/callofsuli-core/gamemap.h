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
#include "qjsonarray.h"


#define SOLVED_MAX				3

#define XP_FACTOR_TARGET_BASE	0.1
#define XP_FACTOR_LEVEL			0.5
#define XP_FACTOR_PRACTICE		0.5
#define XP_FACTOR_SOLVED_FIRST	2.5
#define XP_FACTOR_SOLVED_OVER	-0.5				// Afert SOLVED_MAX
#define XP_FACTOR_DEATHMATCH	1.5
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


	// Játékmód

	enum GameMode {
		Invalid = 0,
		Action [[deprecated]] = 1,
		Lite = 1 << 2,
		Test = 1 << 3,
		Quiz = 1 << 4,
		Exam = 1 << 5,
		Practice = 1 << 6,
		MultiPlayer [[deprecated]] = 1 << 7,
		Conquest = 1 << 8,
		Rpg = 1 << 9
	};

	Q_ENUM(GameMode);

	Q_DECLARE_FLAGS(GameModes, GameMode)
	Q_FLAG(GameModes)



	// Solver methods

	struct SolverInfo;


	static qreal computeSolvedXpFactor(const SolverInfo &baseSolver,
									   const int &level,
									   const GameMode &mode);

	static qreal computeSolvedXpFactor(const int &level,
									   const int &solved,
									   const GameMode &mode);


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
										 const QString &description, const QString &medalImage,
										 const quint32 &gameModes) override;
	GameMapImageIface* ifaceAddImage(const qint32 &id, const QByteArray &data) override;

private:
	QList<GameMapStorage *> m_storages;
	QList<GameMapImage *> m_images;
	QList<GameMapChapter *> m_chapters;
	QList<GameMapMission *> m_missions;
};

Q_DECLARE_METATYPE(GameMap)
Q_DECLARE_METATYPE(GameMap::GameMode)
Q_DECLARE_METATYPE(GameMap::GameModes)
Q_DECLARE_OPERATORS_FOR_FLAGS(GameMap::GameModes)



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
											 const QVariantMap &data,
											 const qint32 &examPoint) override;

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
	Q_PROPERTY(qint32 examPoint MEMBER m_examPoint)

public:
	explicit GameMapObjective(const QString &uuid, const QString &module,
							  const qint32 &storageId, const qint32 &storageCount,
							  const QVariantMap &data, const qint32 &examPoint,
							  GameMap *map);
	virtual ~GameMapObjective() {}

	const QString &uuid() const;
	const QString &module() const;
	qint32 storageId() const;
	qint32 storageCount() const;
	const QVariantMap &data() const;
	qint32 examPoint() const { return m_examPoint; };

	GameMapStorage *storage() const;

	QVariantList &generatedQuestions();
	QVariantMap &commonData() { return m_commonData; }

private:
	GameMap *m_map;
	QVariantList m_generatedQuestions;
	QVariantMap m_commonData;
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
	Q_PROPERTY(GameMap::GameModes modes READ modes CONSTANT)

public:
	explicit GameMapMission() {}
	explicit GameMapMission(const QByteArray &uuid, const QString &name, const QString &description,
							const QString &medalImage, const GameMap::GameModes &modes,
							GameMap *map);
	virtual ~GameMapMission();

	const QString &uuid() const;
	const QString &name() const;
	const QString &description() const;
	const QString &medalImage() const;
	const QList<GameMapMissionLevel *> &levels() const;
	const QList<GameMapMissionLevel *> &locks() const;
	GameMap::GameModes modes() const;

	GameMapMissionLevel *level(const qint32 &num) const;


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
													const qreal &passed,
													const quint32 &gameModes,
													const qint32 &image) override;
	virtual GameMapMissionLevelIface* ifaceAddLock(const QString &uuid, const qint32 &level) override;

private:
	QList<GameMapMissionLevel *> m_levels;
	QList<GameMapMissionLevel *> m_locks;
	GameMap *m_map = nullptr;
};

Q_DECLARE_METATYPE(GameMapMission)




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
	Q_PROPERTY(qint32 image MEMBER m_image)
	Q_PROPERTY(bool canDeathmatch MEMBER m_canDeathmatch)
	Q_PROPERTY(qreal questions MEMBER m_questions)
	Q_PROPERTY(qreal passed MEMBER m_passed)
	Q_PROPERTY(GameMap::GameModes modes READ modes CONSTANT)
	Q_PROPERTY(QList<GameMapInventory *> inventories MEMBER m_inventories)
	Q_PROPERTY(QList<qint32> chapterIds MEMBER m_chapterIds)
	Q_PROPERTY(GameMapMission* mission MEMBER m_mission)

public:
	explicit GameMapMissionLevel() {}
	explicit GameMapMissionLevel(const qint32 &level, const QByteArray &terrain, const qint32 &startHP,
								 const qint32 &duration, const bool &canDeathmatch, const qreal &questions,
								 const qreal &passed, const GameMap::GameModes &modes,
								 const qint32 &image, GameMapMission *mission, GameMap *map);
	virtual ~GameMapMissionLevel() {}

	qint32 level() const;
	const QString &terrain() const;
	qint32 startHP() const;
	qint32 duration() const;
	const qint32 &image() const;
	bool canDeathmatch() const;
	qreal questions() const;
	qreal passed() const;
	const QList<GameMapInventory *> &inventories() const;
	const QList<qint32> &chapterIds() const;

	GameMapMission *mission() const;
	QList<GameMapChapter *> chapters() const;

	GameMap *map() const;

	GameMap::GameModes modes() const;

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
	GameMap *m_map = nullptr;
	GameMapMission *m_mission = nullptr;
};

Q_DECLARE_METATYPE(GameMapMissionLevel)





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
	QHash<int, int> levels;

	SolverInfo() = default;

	SolverInfo(const QJsonArray &array) {
		for (const QJsonValue &v : array) {
			const QJsonObject o = v.toObject();
			levels[o.value(QStringLiteral("level")).toInt()] = o.value(QStringLiteral("num")).toInt();
		}
	}

	SolverInfo(const QJsonObject &object) {
		// Old version

		const int level1 = std::max(object.value(QStringLiteral("t1")).toInt(0),
									object.value(QStringLiteral("d1")).toInt(0));
		const int level2 = std::max(object.value(QStringLiteral("t2")).toInt(0),
									object.value(QStringLiteral("d2")).toInt(0));
		const int level3 = std::max(object.value(QStringLiteral("t3")).toInt(0),
									object.value(QStringLiteral("d3")).toInt(0));

		if (level1 > 0)
			setSolved(1, level1);

		if (level2 > 0)
			setSolved(2, level2);

		if (level3 > 0)
			setSolved(3, level3);
	}

	void setSolved(const int &level, const int &num) {
		levels[level] = num;
	}


	int solved(const int &level) const {
		return levels.value(level, -1);
	};

	bool isSolvedNew(const SolverInfo &baseSolverInfo, const int &level) const {
		return (baseSolverInfo.solved(level) != -1 && this->solved(level) > baseSolverInfo.solved(level));
	};

	bool isSolvedFirst(const SolverInfo &baseSolverInfo, const int &level) const {
		return (baseSolverInfo.solved(level) == 0 &&
				this->solved(level) > baseSolverInfo.solved(level));
	};


	SolverInfo solve(const int &level) const {
		SolverInfo p(*this);

		int n = p.levels.value(level, -1);

		if (n > 0)
			++n;
		else
			n = 1;

		p.levels[level] = n;

		return p;
	}


	QJsonArray toJsonArray() const {
		QJsonArray a;

		for (const auto &[l, n] : levels.asKeyValueRange()) {
			QJsonObject o({
							  { QStringLiteral("level"), l },
							  { QStringLiteral("num"), n },
						  });
			a.append(o);
		}

		return a;
	}

};



#endif // GAMEMAPNEW_H
