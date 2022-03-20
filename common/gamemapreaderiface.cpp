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

#include <QDebug>
#include <QUuid>

GameMapReaderIface::GameMapReaderIface()
	: m_uuid()
	, m_version(0)
{

}


/**
 * @brief GameMapReaderIface::~GameMapReaderIface
 */

GameMapReaderIface::~GameMapReaderIface()
{

}


/**
 * @brief GameMapReaderIface::imagePixmap
 * @param name
 * @return
 */

QPixmap GameMapReaderIface::imagePixmap(const QString &name) const
{
	foreach (GameMapImageIface *i, ifaceImages()) {
		if (i->m_name == name) {
			return i->toPixmap();
		}
	}

	return QPixmap();
}


/**
 * @brief GameMapReaderIface::regenerateUuids
 */

void GameMapReaderIface::regenerateUuids()
{
	m_uuid = QUuid::createUuid().toString();

	foreach(GameMapMissionIface *m, ifaceMissions()) {
		m->m_uuid = QUuid::createUuid().toString();
	}

	foreach (GameMapChapterIface *ch, ifaceChapters()) {
		foreach (GameMapObjectiveIface *o, ch->ifaceObjectives()) {
			o->m_uuid = QUuid::createUuid().toString();
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
				qWarning() << QObject::tr("Redundant locks") << m->m_uuid << m->m_name << m;
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
		qWarning() << QObject::tr("Redundant locks") << mission->m_uuid << mission->m_name;
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

	quint32 magic;
	QByteArray str;

	stream >> magic >> str;

	if (magic != 0x434F53 || str != "MAP") {			// COS
		qWarning() << "Invalid map data";
		return false;
	}

	qint32 version = -1;

	stream >> version;

	qDebug() << "Load map data version" << version;

	if (version < 10) {
		qWarning() << "Invalid map version";
		return false;
	}

	if (version < GAMEMAP_CURRENT_VERSION) {
		qInfo() << "Old version found:" << version;
	}

	stream.setVersion(QDataStream::Qt_5_11);

	QByteArray uuid;

	stream >> uuid;

	m_version = version;
	m_uuid = uuid;

	qDebug() << "Load map" << m_uuid;

	try {
		if (!storagesFromStream(stream))
			throw 1;

		if (!chaptersFromStream(stream))
			throw 1;

		if (!missionsFromStream(stream))
			throw 1;

		if (!imagesFromStream(stream))
			throw 1;

	} catch (...) {
		qWarning() << "Invalid map";
		return false;
	}

	return true;
}




/**
 * @brief GameMapReaderIface::writeBinaryData
 * @return
 */


QByteArray GameMapReaderIface::toBinaryData() const
{
	QByteArray s;
	QDataStream stream(&s, QIODevice::WriteOnly);

	qint32 version = GAMEMAP_CURRENT_VERSION;

	stream << (quint32) 0x434F53;			// COS
	stream << QByteArray("MAP");
	stream << version;

	stream.setVersion(QDataStream::Qt_5_11);

	stream << m_uuid.toLatin1();

	storagesToStream(stream, ifaceStorages());
	chaptersToStream(stream, ifaceChapters());
	missionsToStream(stream, ifaceMissions());
	imagesToStream(stream, ifaceImages());

	return s;
}




/**
 * @brief GameMapReaderIface::storagesFromStream
 * @param stream
 * @return
 */

bool GameMapReaderIface::storagesFromStream(QDataStream &stream)
{
	qDebug() << "Load storages";

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
	qDebug() << "Load chapters";

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

	qDebug() << "Chapters loaded";

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

	qDebug() << "Load missions" << size;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		bool mandatory;		// DEPRECATED
		QString name;
		QString description;
		QString medalImage;
		quint32 lockSize = 0;

		stream >> uuid;

		if (m_version < 10)
			stream >> mandatory;

		stream >> name >> description;

		if (m_version > 7)
			stream >> medalImage;

		stream >> lockSize;

		qDebug() << "Load" << uuid;

		if (uuid.isEmpty())
			return false;

		GameMapMissionIface *m = ifaceAddMission(uuid, name, description, medalImage);

		if (!m)
			return false;

		for (quint32 j=0; j<lockSize; j++) {
			QByteArray uuid;
			qint32 level = -1;
			stream >> uuid >> level;

			if (!m->ifaceAddLock(uuid, level))
				return false;
		}


		missionLevelsFromStream(stream, m);
	}


	qDebug() << "Locks loaded";

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
	qDebug() << "Load images";

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		QString file;
		QByteArray data;

		if (m_version < 11) {
			QString folder;
			stream >> folder;
		}

		stream >> file >> data;

		if (file.isEmpty() || data.isEmpty())
			return false;

		if (!ifaceAddImage(file, data))
			return false;
	}

	qDebug() << "Images loaded";

	return true;
}


/**
 * @brief GameMapReaderIface::imagesToStream
 * @param stream
 * @param images
 */

void GameMapReaderIface::imagesToStream(QDataStream &stream, const QList<GameMapImageIface *> &images) const
{
	stream << (quint32) images.size();

	foreach (GameMapImageIface *i, images) {
		stream << i->m_name;
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

	qDebug() << "Load levels" << size;

	for (quint32 i=0; i<size; i++) {
		qint32 level = -1;
		QByteArray terrain;
		qint32 startHP = -1;
		qint32 duration = -1;
		bool canDeathmatch = false;
		qreal questions = 1.0;
		QString image;

		stream >> level >> terrain >> startHP >> duration;

		if (m_version < 11) {
			qint32 startBlock = -1;
			QString imageFolder;
			stream >> startBlock >> imageFolder;
		}

		stream >> image >> canDeathmatch >> questions;

		if (level == -1 || startHP == -1 || duration == -1)
			return false;

		GameMapMissionLevelIface *m = mission->ifaceAddLevel(level, terrain, startHP, duration, canDeathmatch, questions, image);

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
							qWarning() << "Invalid chapter id";
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
					qWarning() << "Invalid chapter id";
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

bool GameMapReaderIface::inventoriesFromStream(QDataStream &stream, GameMapMissionLevelIface *chapter)
{
	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 block = -1;
		QByteArray module;
		qint32 count = -1;

		stream >> block >> module >> count;

		if (block < 0 || module.isEmpty() || count <= 0)
			return false;

		if (!chapter->ifaceAddInventory(block, module, count))
			return false;
	}

	return true;
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
				if (l->m_level > it->m_level)
					it->m_level = l->m_level;
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
		stream << m->m_image;
		stream << m->m_canDeathmatch;
		stream << m->m_questions;

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
 * @brief GameMapImageIface::toPixmap
 * @return
 */

QPixmap GameMapImageIface::toPixmap() const
{
	QPixmap outPixmap = QPixmap();
	outPixmap.loadFromData(m_data);
	return outPixmap;
}



/**
 * @brief GameMapReaderIface::ifaceListConvert
 * @param from
 * @return
 */


