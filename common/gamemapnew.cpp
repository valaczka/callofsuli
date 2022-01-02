/*
 * ---- Call of Suli ----
 *
 * gamemapnew.cpp
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

#include <QDataStream>
#include <QDebug>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "gamemapnew.h"

GameMapNew::GameMapNew(QObject *parent)
	: QObject(parent)
	, m_version(GAMEMAP_CURRENT_VERSION)
	, m_uuid(QUuid::createUuid().toString())
	, m_storages()
	, m_chapters()
	, m_missions()
{

}


/**
 * @brief GameMapNew::~GameMapNew
 */

GameMapNew::~GameMapNew()
{
	foreach (GameMapStorage *s, m_storages)
		s->setGameMap(nullptr);

	qDeleteAll(m_storages.begin(), m_storages.end());
	m_storages.clear();

	foreach (GameMapChapter *s, m_chapters)
		s->setGameMap(nullptr);

	qDeleteAll(m_chapters.begin(), m_chapters.end());
	m_chapters.clear();

	foreach (GameMapMission *s, m_missions)
		s->setGameMap(nullptr);

	qDeleteAll(m_missions.begin(), m_missions.end());
	m_missions.clear();
}



/**
 * @brief GameMapNew::fromBinaryData
 * @param data
 * @return
 */

GameMapNew *GameMapNew::fromBinaryData(const QByteArray &data, QObject *parent)
{
	QDataStream stream(data);

	quint32 magic;
	QByteArray str;

	stream >> magic >> str;

	if (magic != 0x434F53 || str != "MAP") {			// COS
		qWarning() << "Invalid map data";
		return nullptr;
	}

	qint32 version = -1;

	stream >> version;

	qDebug() << "Load map data version" << version;

	if (version < 10) {
		qWarning() << "Invalid map version";
		return nullptr;
	}

	if (version < GAMEMAP_CURRENT_VERSION) {
		qInfo() << "Old version found:" << version;
	}

	stream.setVersion(QDataStream::Qt_5_11);


	QByteArray uuid;

	stream >> uuid;

	GameMapNew *map = new GameMapNew(parent);
	map->m_version = version;
	map->setUuid(uuid);

	qDebug() << "Load map" << uuid;

	try {
		if (!map->storagesFromStream(stream))
			throw;

		if (!map->chaptersFromStream(stream))
			throw;

	/*map->missionsFromStream(stream);

	map->imagesFromStream(stream);*/
	} catch (...) {
		qWarning() << "Invalid map";
		delete map;
		map = nullptr;
	}

	return map;
}


/**
 * @brief GameMapNew::toBinaryData
 * @return
 */

QByteArray GameMapNew::toBinaryData() const
{
	QByteArray s;
	QDataStream stream(&s, QIODevice::WriteOnly);

	qint32 version = GAMEMAP_CURRENT_VERSION;

	stream << (quint32) 0x434F53;			// COS
	stream << QByteArray("MAP");
	stream << version;

	stream.setVersion(QDataStream::Qt_5_11);

	stream << m_uuid.toLatin1();

	storagesToStream(stream);
	chaptersToStream(stream);
	//missionsToStream(stream);
	//imagesToStream(stream);

	return s;
}



/**
 * @brief GameMapNew::toJson
 * @return
 */

QJsonDocument GameMapNew::toJsonDocument() const
{
	QJsonObject root;

	root["uuid"] = m_uuid;
	root["version"] = m_version;

	QJsonArray storageArray;

	foreach (GameMapStorage *s, m_storages)
		storageArray.append(s->toJsonObject());

	root["storages"] = storageArray;


	QJsonArray chapterArray;

	foreach (GameMapChapter *s, m_chapters)
		chapterArray.append(s->toJsonObject());

	root["chapters"] = chapterArray;

	return QJsonDocument(root);
}




/**
 * @brief GameMapNew::storagesFromStream
 * @param stream
 * @return
 */

bool GameMapNew::storagesFromStream(QDataStream &stream)
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

		GameMapStorage *s = new GameMapStorage(this);
		s->setId(id);
		s->setModule(module);
		s->setData(data);

		storageAdd(s);
	}

	if (m_storages.size() != (int) size)
		return false;

	return true;
}


/**
 * @brief GameMapNew::storagesToStream
 * @param stream
 */

void GameMapNew::storagesToStream(QDataStream &stream) const
{
	stream << (quint32) m_storages.size();

	foreach (GameMapStorage *s, m_storages) {
		stream << s->id();
		stream << s->module().toLatin1();
		stream << s->data();
	}
}


/**
 * @brief GameMapNew::chaptersFromStream
 * @param stream
 * @return
 */

bool GameMapNew::chaptersFromStream(QDataStream &stream)
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

		GameMapChapter *ch = new GameMapChapter(this);
		ch->setId(id);
		ch->setName(name);

		ch->objectivesFromStream(stream, this);

		chapterAdd(ch);
	}

	if (m_chapters.size() != (int) size)
		return false;

	qDebug() << "Chapters loaded";

	return true;
}


/**
 * @brief GameMapNew::chaptersToStream
 * @param stream
 */

void GameMapNew::chaptersToStream(QDataStream &stream) const
{
	stream << (quint32) m_chapters.size();

	foreach (GameMapChapter *ch, m_chapters) {
		stream << ch->id();
		stream << ch->name();

		ch->objectivesToStream(stream);
	}
}


/**
 * @brief GameMapNew::missionsFromStream
 * @param stream
 * @return
 */

bool GameMapNew::missionsFromStream(QDataStream &stream)
{
	quint32 size = 0;
	stream >> size;

	qDebug() << "Load missions" << size;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		bool mandatory;
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

		if (uuid.isEmpty())
			return false;

		GameMapMission *m = new GameMapMission(this);

		m->setUuid(uuid);
		m->setName(name);
		m->setDescription(description);
		m->setMedalImage(medalImage);

		QList<QPair<QByteArray, qint32>> ll;

		for (quint32 j=0; j<lockSize; j++) {
			QByteArray uuid;
			qint32 level = -1;
			stream >> uuid >> level;

			if (uuid.isEmpty()) {
				delete m;
				return false;
			}

			ll.append(qMakePair<QByteArray, qint32>(uuid, level));
		}

		if (!ll.isEmpty())
			lockList[uuid] = ll;



		missionLevelsFromStream(m, stream, version);

		m_missions.append(m);
	}


	if (m_missions.size() != (int) size)
		return false;


	qDebug() << "Load missions locks";

	// Mission locks

	{
		QHash<QByteArray, QList<QPair<QByteArray, qint32>>>::const_iterator mit;

		for (mit=lockList.constBegin(); mit != lockList.constEnd(); ++mit) {
			Mission *m = mission(mit.key());
			if (!m)
				return false;

			for (int i=0; i<mit.value().size(); ++i) {
				Mission *mm = mission(mit.value().at(i).first);
				qint32 level = mit.value().at(i).second;

				if (!mm)
					return false;

				m->addLock(mm, level);
			}
		}
	}

	qDebug() << "Locks loaded";

	return true;	*/
}


/**
 * @brief GameMapNew::missionsToStream
 * @param stream
 */

void GameMapNew::missionsToStream(QDataStream &stream) const
{

}

const QList<GameMapMission *> &GameMapNew::missions() const
{
	return m_missions;
}

void GameMapNew::setMissions(const QList<GameMapMission *> &newMissions)
{
	if (m_missions == newMissions)
		return;
	m_missions = newMissions;
	emit missionsChanged();
}



const QList<GameMapChapter *> &GameMapNew::chapters() const
{
	return m_chapters;
}

void GameMapNew::setChapters(const QList<GameMapChapter *> &newChapters)
{
	if (m_chapters == newChapters)
		return;
	m_chapters = newChapters;
	emit chaptersChanged();
}





/**
 * @brief GameMapNew::version
 * @return
 */

qint32 GameMapNew::version() const
{
	return m_version;
}

const QString &GameMapNew::uuid() const
{
	return m_uuid;
}

void GameMapNew::setUuid(const QString &newUuid)
{
	if (m_uuid == newUuid)
		return;
	m_uuid = newUuid;
	emit uuidChanged();
}

const QList<GameMapStorage *> &GameMapNew::storages() const
{
	return m_storages;
}

void GameMapNew::setStorages(const QList<GameMapStorage *> &newStorages)
{
	if (m_storages == newStorages)
		return;
	m_storages = newStorages;
	emit storagesChanged();
}


/**
 * @brief GameMapNew::storageAdd
 * @param storage
 */

void GameMapNew::storageAdd(GameMapStorage *storage)
{
	m_storages.append(storage);
	storage->setGameMap(this);
}


/**
 * @brief GameMapNew::storageRemove
 * @param storage
 * @return
 */

int GameMapNew::storageRemove(GameMapStorage *storage)
{
	int n = m_storages.removeAll(storage);
	storage->setGameMap(nullptr);
	return n;
}


/**
 * @brief GameMapNew::storage
 * @param id
 * @return
 */

GameMapStorage *GameMapNew::storage(const qint32 &id) const
{
	for (GameMapStorage *s : m_storages)
		if (s->id() == id)
			return s;

	return nullptr;
}


/**
 * @brief GameMapNew::chapterAdd
 * @param chapter
 */

void GameMapNew::chapterAdd(GameMapChapter *chapter)
{
	m_chapters.append(chapter);
	chapter->setGameMap(this);
}

/**
 * @brief GameMapNew::chapterRemove
 * @param chapter
 * @return
 */

int GameMapNew::chapterRemove(GameMapChapter *chapter)
{
	int n = m_chapters.removeAll(chapter);
	chapter->setGameMap(nullptr);
	return n;
}


/**
 * @brief GameMapNew::chapter
 * @param id
 * @return
 */

GameMapChapter *GameMapNew::chapter(const qint32 &id) const
{
	for (GameMapChapter *ch : m_chapters)
		if (ch->id() == id)
			return ch;

	return nullptr;
}


/**
 * @brief GameMapNew::missionAdd
 * @param mission
 */

void GameMapNew::missionAdd(GameMapMission *mission)
{
	m_missions.append(mission);
	mission->setGameMap(this);
}



/**
 * @brief GameMapNew::missionRemove
 * @param mission
 * @return
 */

int GameMapNew::missionRemove(GameMapMission *mission)
{
	int n = m_missions.removeAll(mission);
	mission->setGameMap(nullptr);
	return n;
}



