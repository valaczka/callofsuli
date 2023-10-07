/*
 * ---- Call of Suli ----
 *
 * gamemapreaderiface.cpp
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

#include "gamemapreaderiface.h"
#include "utils.h"
#include "Logger.h"

#include <QDebug>
#include <QUuid>


GameMapReaderIface::GameMapReaderIface()
	: m_uuid()
	, m_version(0)
	, m_appVersion(0)
{

}


/**
 * @brief GameMapReaderIface::~GameMapReaderIface
 */

GameMapReaderIface::~GameMapReaderIface()
{

}



/**
 * @brief GameMapReaderIface::regenerateUuids
 */

void GameMapReaderIface::regenerateUuids()
{
	m_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

	foreach(GameMapMissionIface *m, ifaceMissions()) {
		m->m_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
	}

	foreach (GameMapChapterIface *ch, ifaceChapters()) {
		foreach (GameMapObjectiveIface *o, ch->ifaceObjectives()) {
			o->m_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
		}
	}
}



/**
 * @brief GameMapReaderIface::checkLockTree
 * @return
 */

GameMapMissionIface *GameMapReaderIface::checkLockTree(QList<GameMapMissionIface*> *listPtr) const
{
	foreach(GameMapMissionIface *m, ifaceMissions()) {
		QVector<GameMapMissionLevelIface*> list;
		if (!m->getLockTree(&list, m)) {
			if (listPtr) {
				listPtr->append(m);
			} else {
				LOG_CWARNING("app") << "Redundant locks" << qPrintable(m->m_uuid) << qPrintable(m->m_name) << m;
				return m;
			}
		}
	}

	if (listPtr && !listPtr->isEmpty())
		return listPtr->at(0);
	else
		return nullptr;
}


/**
 * @brief GameMapReaderIface::missionLockTree
 * @param mission
 * @return
 */

QVector<GameMapMissionLevelIface *> GameMapReaderIface::missionLockTree(GameMapMissionIface *mission) const
{
	QVector<GameMapMissionLevelIface*> list;
	if (!mission->getLockTree(&list, mission)) {
		LOG_CWARNING("app") << "Redundant locks:" << qPrintable(mission->m_uuid) << qPrintable(mission->m_name);
		return QVector<GameMapMissionLevelIface*>();
	}

	return list;
}






/**
 * @brief GameMapReaderIface::readBinaryData
 * @param data
 */

bool GameMapReaderIface::readBinaryData(const QByteArray &data)
{
	QDataStream stream(data);

	quint32 magic = 0;
	QByteArray str;

	stream >> magic >> str;

	if (magic != 0x434F53 || str != "MAP") {			// COS
		LOG_CWARNING("app") << "Invalid map data";
		return false;
	}

	qint32 version = -1;

	stream >> version;

	LOG_CTRACE("app") << "Load map data version:" << version;

	if (version < 7) {
		LOG_CWARNING("app") << "Invalid map version";
		return false;
	}

	if (version < GAMEMAP_CURRENT_VERSION) {
		LOG_CINFO("app") << "Old version found:" << version;
	}

	stream.setVersion(QDataStream::Qt_5_11);

	QByteArray uuid;

	stream >> uuid;

	m_version = version;
	m_uuid = uuid;

	LOG_CTRACE("app") << "Load map" << m_uuid;

	try {
		if (version > 11)
			stream >> m_appVersion;

		if (!storagesFromStream(stream))
			throw 1;

		if (!chaptersFromStream(stream))
			throw 1;

		if (!missionsFromStream(stream))
			throw 1;

		if (!imagesFromStream(stream))
			throw 1;

	} catch (...) {
		LOG_CWARNING("app") << "Invalid map";
		return false;
	}

	return true;
}




/**
 * @brief GameMapReaderIface::writeBinaryData
 * @return
 */


QByteArray GameMapReaderIface::toBinaryData(const bool &imageCheck) const
{
	QByteArray s;
	QDataStream stream(&s, QIODevice::WriteOnly);

	qint32 version = GAMEMAP_CURRENT_VERSION;

	stream << (quint32) 0x434F53;			// COS
	stream << QByteArray("MAP");
	stream << version;

	stream.setVersion(QDataStream::Qt_5_11);

	stream << m_uuid.toLatin1();
	stream << Utils::versionCode();

	storagesToStream(stream, ifaceStorages());
	chaptersToStream(stream, ifaceChapters());
	missionsToStream(stream, ifaceMissions());
	imagesToStream(stream, ifaceImages(), imageCheck, ifaceMissions(), ifaceStorages());

	return s;
}




/**
 * @brief GameMapReaderIface::storagesFromStream
 * @param stream
 * @return
 */

bool GameMapReaderIface::storagesFromStream(QDataStream &stream)
{
	LOG_CTRACE("app") << "Load storages";

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 id = -1;
		QByteArray module;
		QVariantMap data;
		stream >> id >> module >> data;

		if (id == -1)
			return false;

		if (!ifaceAddStorage(id, module, data))
			return false;
	}

	return true;
}



/**
 * @brief GameMapReaderIface::storagesToStream
 * @param stream
 */

void GameMapReaderIface::storagesToStream(QDataStream &stream, const QList<GameMapStorageIface *> &storages) const
{
	stream << (quint32) storages.size();

	foreach (GameMapStorageIface *s, storages) {
		stream << s->m_id;
		stream << s->m_module.toLatin1();
		stream << s->m_data;
	}
}





/**
 * @brief GameMapReaderIface::chaptersFromStream
 * @param stream
 * @return
 */

bool GameMapReaderIface::chaptersFromStream(QDataStream &stream)
{
	LOG_CTRACE("app") << "Load chapters";

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 id = -1;
		QString name;
		stream >> id >> name;

		if (id == -1)
			return false;

		GameMapChapterIface *ch = ifaceAddChapter(id, name);

		if (!ch)
			return false;

		objectivesFromStream(stream, ch);
	}

	LOG_CTRACE("app") << "Chapters loaded";

	return true;
}





/**
 * @brief GameMapReaderIface::chaptersToStream
 * @param stream
 * @param chapters
 */

void GameMapReaderIface::chaptersToStream(QDataStream &stream, const QList<GameMapChapterIface *> &chapters) const
{
	stream << (quint32) chapters.size();

	foreach (GameMapChapterIface *ch, chapters) {
		stream << ch->m_id;
		stream << ch->m_name;

		ch->objectivesToStream(stream);
	}
}





/**
 * @brief GameMapReaderIface::missionsFromStream
 * @param stream
 * @return
 */

bool GameMapReaderIface::missionsFromStream(QDataStream &stream)
{
	quint32 size = 0;
	stream >> size;

	LOG_CTRACE("app") << "Load missions:" << size;

	struct lockStruct {
		GameMapMissionIface *mission;
		QByteArray lockMission;
		qint32 lockLevel;

		lockStruct(GameMapMissionIface *m, const QByteArray &lm, const qint32 &ll) :
			mission(m), lockMission(lm), lockLevel(ll) {}
	};

	QList<lockStruct> lockList;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		bool mandatory;		// DEPRECATED
		QString name;
		QString description;
		QString medalImage;
		quint32 lockSize = 0;
		quint32 gameModes = 0;

		stream >> uuid;

		if (m_version < 10)
			stream >> mandatory;

		stream >> name >> description;

		if (m_version > 7)
			stream >> medalImage;

		if (m_version > 12)
			stream >> gameModes;

		stream >> lockSize;

		LOG_CTRACE("app") << "Load mission:" << uuid;

		if (uuid.isEmpty())
			return false;

		GameMapMissionIface *m = ifaceAddMission(uuid, name, description, medalImage, gameModes);

		if (!m)
			return false;

		for (quint32 j=0; j<lockSize; j++) {
			QByteArray uuid;
			qint32 level = -1;
			stream >> uuid >> level;

			lockList.append(lockStruct(m, uuid, level));
		}


		missionLevelsFromStream(stream, m);
	}


	LOG_CTRACE("app") << "Load locks";

	foreach (const lockStruct &s, lockList) {
		if (!s.mission || !s.mission->ifaceAddLock(s.lockMission, s.lockLevel)) {
			return false;
		}
	}

	LOG_CTRACE("app") << "Locks loaded";

	return true;
}





/**
 * @brief GameMapReaderIface::missionsToStream
 * @param stream
 * @param missions
 */

void GameMapReaderIface::missionsToStream(QDataStream &stream, const QList<GameMapMissionIface *> &missions) const
{
	stream << (quint32) missions.size();

	foreach (GameMapMissionIface *m, missions) {
		stream << m->m_uuid.toLatin1();
		stream << m->m_name;
		stream << m->m_description;
		stream << m->m_medalImage;
		stream << m->m_gameModes;

		const QList<GameMapMissionLevelIface *> locks = m->ifaceLocks();

		stream << (quint32) locks.size();

		foreach (GameMapMissionLevelIface *p, locks) {
			stream << p->mission()->m_uuid.toLatin1();
			stream << p->m_level;
		}

		m->levelsToStream(stream);
	}
}



/**
 * @brief GameMapReaderIface::imagesFromStream
 * @param stream
 * @return
 */

bool GameMapReaderIface::imagesFromStream(QDataStream &stream)
{
	LOG_CTRACE("app") << "Load images";

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 id = -1;
		QByteArray data;

		if (m_version < 11) {
			QString folder;
			stream >> folder;
		}

		stream >> id >> data;

		if (id == -1 || data.isEmpty())
			return false;

		if (!ifaceAddImage(id, data))
			return false;
	}

	LOG_CTRACE("app") << "Images loaded";

	return true;
}


/**
 * @brief GameMapReaderIface::imagesToStream
 * @param stream
 * @param images
 * @param check
 * @param missions
 * @param storages
 */

void GameMapReaderIface::imagesToStream(QDataStream &stream, const QList<GameMapImageIface *> &images,
										const bool &check, const QList<GameMapMissionIface *> &missions,
										const QList<GameMapStorageIface *> &storages) const
{
	if (check) {
		LOG_CTRACE("app") << "Check images...";

		QVector<GameMapImageIface *> list;
		list.reserve(images.size());

		foreach(GameMapImageIface *i, images) {
			const int &id = i->m_id;

			bool found = false;

			foreach (GameMapMissionIface *m, missions) {
				foreach (GameMapMissionLevelIface *ml, m->ifaceLevels()) {
					if (ml->m_image == id) {
						found = true;
						break;
					}
				}

				if (found)
					break;
			}

			if (found) {
				list.append(i);
				continue;
			}

			foreach (GameMapStorageIface *s, storages) {
				if (s->m_module != QLatin1String("images"))
					continue;

				const QVariantList &l = s->m_data.value(QStringLiteral("images")).toList();
				foreach (const QVariant &v, l) {
					const QVariantMap &m = v.toMap();
					if (m.contains(QStringLiteral("second")) && m.value(QStringLiteral("second"), -1).toInt() == id) {
						found = true;
						break;
					}
				}

				if (found)
					break;
			}

			if (found) {
				list.append(i);
				continue;
			}


			LOG_CTRACE("app") << "- image skipped:" << id;
		}


		stream << (quint32) list.size();

		foreach (GameMapImageIface *i, list) {
			stream << i->m_id;
			stream << i->m_data;
		}


		LOG_CTRACE("app") << "...check images finished";
		return;
	}

	stream << (quint32) images.size();

	foreach (GameMapImageIface *i, images) {
		stream << i->m_id;
		stream << i->m_data;
	}
}






/**
 * @brief GameMapReaderIface::objectivesFromStream
 * @param stream
 * @return
 */
bool GameMapReaderIface::objectivesFromStream(QDataStream &stream, GameMapChapterIface *chapter)
{
	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		QByteArray module;
		qint32 storageId = -1;
		qint32 storageCount = 0;
		QVariantMap data;
		stream >> uuid >> module >> storageId >> data >> storageCount;

		if (uuid.isEmpty())
			return false;

		if (!chapter->ifaceAddObjective(uuid, module, storageId, storageCount, data))
			return false;
	}

	return true;
}


/**
 * @brief GameMapReaderIface::missionLevelsFromStream
 * @param stream
 * @param mission
 * @return
 */

bool GameMapReaderIface::missionLevelsFromStream(QDataStream &stream, GameMapMissionIface *mission)
{
	quint32 size = 0;
	stream >> size;

	LOG_CTRACE("app") << "Load levels:" << size;

	for (quint32 i=0; i<size; i++) {
		qint32 level = -1;
		QByteArray terrain;
		qint32 startHP = -1;
		qint32 duration = -1;
		bool canDeathmatch = false;
		qreal questions = 1.0;
		qreal passed = 0.8;
		QString image;

		stream >> level >> terrain >> startHP >> duration;

		if (m_version < 11) {
			qint32 startBlock = -1;
			QString imageFolder;
			stream >> startBlock >> imageFolder;
		}

		if (m_version < 15)
			stream >> image ;

		stream >> canDeathmatch >> questions;

		if (m_version > 13)
			stream >> passed;

		if (level == -1 || startHP == -1 || duration == -1)
			return false;

		qint32 imageId = -1;

		if (m_version > 14)
			stream >> imageId;

		GameMapMissionLevelIface *m = mission->ifaceAddLevel(level, terrain, startHP, duration, canDeathmatch, questions, passed, imageId);

		if (!m)
			return false;


		// Load chapters

		if (m_version < 11) {
			quint32 size = 0;
			stream >> size;

			for (quint32 i=0; i<size; i++) {
				{
					quint32 size = 0;
					stream >> size;
					for (quint32 i=0; i<size; i++) {
						qint32 block = -1;
						stream >> block;
					}
				}


				{
					quint32 size = 0;
					stream >> size;
					for (quint32 i=0; i<size; i++) {
						qint32 chId = -1;
						stream >> chId;

						if (chId == -1) {
							LOG_CWARNING("app") << "Invalid chapter id";
							return false;
						}

						if (!m->ifaceAddChapter(chId))
							return false;
					}
				}
			}
		} else {
			quint32 size = 0;
			stream >> size;
			for (quint32 i=0; i<size; i++) {
				qint32 chId = -1;
				stream >> chId;

				if (chId == -1) {
					LOG_CWARNING("app") << "Invalid chapter id";
					return false;
				}

				if (!m->ifaceAddChapter(chId))
					return false;
			}
		}

		inventoriesFromStream(stream, m);
	}


	return true;
}





/**
 * @brief GameMapReaderIface::inventoriesFromStream
 * @param stream
 * @param chapter
 * @return
 */

bool GameMapReaderIface::inventoriesFromStream(QDataStream &stream, GameMapMissionLevelIface *missionLevel)
{
	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 block = -1;
		QByteArray module;
		qint32 count = -1;

		stream >> block >> module >> count;

		if (block < 0 || module.isEmpty() || count <= 0) {
			return false;
		}

		if (!missionLevel->ifaceAddInventory(block, module, count))
			return false;
	}

	return true;
}


/**
 * @brief GameMapReaderIface::appVersion
 * @return
 */

quint32 GameMapReaderIface::appVersion() const
{
	return m_appVersion;
}






/**
 * @brief GameMapChapterIface::objectiveToStream
 * @param stream
 * @return
 */

void GameMapChapterIface::objectivesToStream(QDataStream &stream) const
{
	QList<GameMapObjectiveIface*> objectives = ifaceObjectives();
	stream << (quint32) objectives.size();

	foreach (GameMapObjectiveIface *o, objectives) {
		stream << o->m_uuid.toLatin1();
		stream << o->m_module.toLatin1();
		stream << o->m_storageId;
		stream << o->m_data;
		stream << o->m_storageCount;
	}
}



/**
 * @brief GameMapMissionIface::levelsToStream
 * @param stream
 */

bool GameMapMissionIface::getLockTree(QVector<GameMapMissionLevelIface *> *listPtr, GameMapMissionIface *rootMission) const
{
	foreach (GameMapMissionLevelIface *l, ifaceLocks()) {
		if (l->mission() == rootMission)				// redundant
			return false;

		bool contains = false;

		foreach (GameMapMissionLevelIface *it, *listPtr) {
			if (it->mission() == l->mission()) {                
				contains = true;
				break;
			}
		}

		if (!contains) {
			listPtr->append(l);
			if (!l->mission()->getLockTree(listPtr, rootMission))
				return false;
		}
	}

	return true;
}





/**
 * @brief GameMapMissionIface::levelsToStream
 * @param stream
 */

void GameMapMissionIface::levelsToStream(QDataStream &stream) const
{
	QList<GameMapMissionLevelIface*> levels = ifaceLevels();
	stream << (quint32) levels.size();

	foreach (GameMapMissionLevelIface *m, levels) {
		stream << m->m_level;
		stream << m->m_terrain.toLatin1();
		stream << m->m_startHP;
		stream << m->m_duration;
		//stream << m->m_image;
		stream << m->m_canDeathmatch;
		stream << m->m_questions;
		stream << m->m_passed;
		stream << m->m_image;

		QList<GameMapChapterIface*> chapters = m->ifaceChapters();

		stream << (quint32) chapters.size();

		foreach (GameMapChapterIface *ch, chapters)
			stream << ch->m_id;


		m->inventoriesToStream(stream);
	}
}



/**
 * @brief GameMapMissionLevelIface::inventoriesToStream
 * @param stream
 */

void GameMapMissionLevelIface::inventoriesToStream(QDataStream &stream) const
{
	QList<GameMapInventoryIface*> inventories = ifaceInventories();
	stream << (quint32) inventories.size();

	foreach (GameMapInventoryIface *m, inventories) {
		stream << m->m_block;
		stream << m->m_module.toLatin1();
		stream << m->m_count;
	}
}




/**
 * @brief GameMapReaderIface::ifaceListConvert
 * @param from
 * @return
 */


