/*
 * ---- Call of Suli ----
 *
 * gamemap.cpp
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

#include "gamemap.h"
#include "cosdb.h"

#include <QDataStream>
#include <QMetaObject>

GameMap::GameMap(const QByteArray &uuid)
	: m_uuid(uuid)
	, m_campaigns()
	, m_chapters()
	, m_storages()
	, m_progressObject(nullptr)
	, m_progressFunc()
{
	qDebug() << this << "GAME MAP CREATED";
}


/**
 * @brief GameMap::~GameMap
 */

GameMap::~GameMap()
{
	qDeleteAll(m_campaigns.begin(), m_campaigns.end());
	m_campaigns.clear();

	qDeleteAll(m_storages.begin(), m_storages.end());
	m_storages.clear();

	qDeleteAll(m_chapters.begin(), m_chapters.end());
	m_chapters.clear();

	qDeleteAll(m_images.begin(), m_images.end());
	m_images.clear();

	qDebug() << this << "GAME MAP DESTROYED";
}


/**
 * @brief GameMap::fromBinaryData
 * @param data
 * @return
 */

GameMap *GameMap::fromBinaryData(const QByteArray &data, QObject *target, const QString &func)
{
	if (!sendProgress(target, func, 0.0))
		return nullptr;

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


	if (version < 3) {
		qWarning() << "Invalid map version";
		return nullptr;
	}

	if (version < GAMEMAP_CURRENT_VERSION) {
		qInfo() << "Old version found:" << version;
	}

	stream.setVersion(QDataStream::Qt_5_11);


	qreal step = 0.0;
	qreal maxStep = 5.0;

	QByteArray uuid;

	stream >> uuid;

	GameMap *map = new GameMap(uuid);

	qDebug() << "Load map" << uuid;

	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->storagesFromStream(stream);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->chaptersFromStream(stream);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->campaignsFromStream(stream);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->imagesFromStream(stream);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	return map;
}


/**
 * @brief GameMap::toBinaryData
 * @return
 */

QByteArray GameMap::toBinaryData() const
{
	qreal step = 0.0;
	qreal maxStep = 5.0;


	if (!sendProgress(0.0))
		return QByteArray();

	QByteArray s;
	QDataStream stream(&s, QIODevice::WriteOnly);

	qint32 version = GAMEMAP_CURRENT_VERSION;

	stream << (quint32) 0x434F53;			// COS
	stream << QByteArray("MAP");
	stream << version;

	stream.setVersion(QDataStream::Qt_5_11);

	stream << m_uuid;

	if (!sendProgress(++step/maxStep))
		return QByteArray();

	storagesToStream(stream);

	if (!sendProgress(++step/maxStep))
		return QByteArray();

	chaptersToStream(stream);

	if (!sendProgress(++step/maxStep))
		return QByteArray();

	campaignsToStream(stream);

	if (!sendProgress(++step/maxStep))
		return QByteArray();

	imagesToStream(stream);

	if (!sendProgress(++step/maxStep))
		return QByteArray();

	return s;
}



/**
 * @brief GameMap::fromDb
 * @return
 */

GameMap *GameMap::example(const QByteArray &uuid)
{
	GameMap *map = new GameMap(uuid);

	GameMap::Chapter *ch1 = new GameMap::Chapter(19, "Fejezet 19");
	GameMap::Chapter *ch2 = new GameMap::Chapter(20, "Fejezet 20");
	GameMap::Chapter *ch3 = new GameMap::Chapter(21, "Fejezet 21");

	QVariantMap m;
	m["szia"] = "lkjklj";
	m["másik"] = 5;
	m["valami"] = true;
	GameMap::Objective *o1 = new GameMap::Objective("xxx1", "MODULE1", nullptr, QVariantMap());
	GameMap::Objective *o2 = new GameMap::Objective("xxx2", "MODULE2", nullptr, m);

	GameMap::Storage *s1 = new GameMap::Storage(299, "stoMod1", m);
	GameMap::Storage *s2 = new GameMap::Storage(297, "stoMod2", QVariantMap());
	GameMap::Objective *o3 = new GameMap::Objective("xxx3", "MODULE3", s1, QVariantMap());

	ch1->addObjective(o1);
	ch2->addObjective(o2);
	map->addStorage(s1);
	map->addStorage(s2);
	ch2->addObjective(o3);

	map->addChapter(new GameMap::Chapter(5, "Fejezet 1"));
	map->addChapter(new GameMap::Chapter(15, "Fejezet 12"));
	map->addChapter(new GameMap::Chapter(4, "Fejezet 3"));
	map->addChapter(ch1);
	map->addChapter(ch2);
	map->addChapter(ch3);


	Campaign *c1 = new Campaign(1, "campaign1");
	Campaign *c2 = new Campaign(2, "campaign2");
	Campaign *c3 = new Campaign(3, "campaign3");
	Campaign *c4 = new Campaign(4, "campaign4");

	map->addCampaign(c1);
	map->addCampaign(c2);
	map->addCampaign(c3);
	map->addCampaign(c4);

	c1->addLock(c2);
	c1->addLock(c3);
	c1->addLock(c4);
	c2->addLock(c3);
	c4->addLock(c2);

	Mission *m1 = new Mission("{miss1}", false, "MIssion1");
	Mission *m2 = new Mission("{miss2}", false, "MIssion2");
	Mission *m3 = new Mission("{miss3}", true, "MIssion3");
	Mission *m4 = new Mission("{miss4}", true, "MIssion4");
	Mission *m5 = new Mission("{miss5}", false, "MIssion5");
	Mission *m6 = new Mission("{miss6}", false, "MIssion6");

	c1->addMission(m1);
	c1->addMission(m2);
	m2->addLock(m1, 1);
	c2->addMission(m3);
	c2->addMission(m4);
	m4->addLock(m2, 2);
	m4->addLock(m1, -1);
	m4->addLock(m3, -1);
	c3->addMission(m5);
	c3->addMission(m6);


	MissionLevel *ml1 = m1->addMissionLevel(new MissionLevel(1, "oiuiuoiuouo", 12, 650, 3, "", ""));
	MissionLevel *ml2 = m1->addMissionLevel(new MissionLevel(2, "oiuiuoiuouo", 11, 1650, 23, "", ""));
	m1->addMissionLevel(new MissionLevel(3, "terrain1", 1, 60, 1, "", ""));

	m2->addMissionLevel(new MissionLevel(2, "terrain2", 4, 600, 1, "", ""));

	{
		BlockChapterMap *b1 = new BlockChapterMap(5);
		b1->addBlock(2);
		b1->addBlock(4);
		b1->addChapter(ch1);
		b1->addChapter(ch2);
		b1->addChapter(ch3);
		b1->addFavorite(o1);
		ml1->addBlockChapterMap(b1);
	}

	{
		BlockChapterMap *b1 = new BlockChapterMap(5);
		b1->addBlock(2);
		b1->addBlock(4);
		b1->addChapter(ch1);
		b1->addChapter(ch2);
		b1->addChapter(ch3);
		b1->addFavorite(o1);
		ml2->addBlockChapterMap(b1);
	}

	{
		BlockChapterMap *b2 = new BlockChapterMap(15);
		b2->addBlock(1);
		b2->addBlock(2);
		b2->addBlock(3);
		b2->addBlock(4);
		b2->addChapter(ch3);
		b2->addFavorite(o1);
		b2->addFavorite(o2);
		b2->addFavorite(o3);
		ml2->addBlockChapterMap(b2);
	}


	m5->addMissionLevel(new MissionLevel(1, "oiuiuoiuouo", 12, 650, 3, "", ""));
	m5->addMissionLevel(new MissionLevel(2, "oiuiuoiuouo", 11, 1650, 23, "", ""));
	m5->addMissionLevel(new MissionLevel(3, "werweriuouo", 1, 60, 1, "", ""));

	m6->addMissionLevel(new MissionLevel(11, "1oiuiuoiuouo", 112, 1650, 13, "", ""));
	m6->addMissionLevel(new MissionLevel(12, "1oiuiuoiuouo", 111, 10, 123, "", ""));
	m6->addMissionLevel(new MissionLevel(13, "1werweriuouo", 11, 160, 11, "", ""));

	ml1->addInvetory(new Inventory(1, "gun", 3));
	ml1->addInvetory(new Inventory(0, "bullet", 13));
	ml1->addInvetory(new Inventory(23, "weapon", 53));

	ml2->addInvetory(new Inventory(1, "time", 3));
	ml2->addInvetory(new Inventory(0, "bullet", 3));
	ml2->addInvetory(new Inventory(1, "weapon", 3));
	ml2->addInvetory(new Inventory(1, "gun", 3));



	return map;
}


/**
 * @brief GameMap::fromDb
 * @param db
 * @return
 */

GameMap *GameMap::fromDb(CosDb *db, QObject *target, const QString &func, const bool &withImages)
{
	qreal step = 0.0;
	qreal maxStep = withImages ? 10.0 : 9.0;

	if (!sendProgress(target, func, 0.0))
		return nullptr;

	Q_ASSERT(db);
	Q_ASSERT(db->isValid());
	Q_ASSERT(db->isOpen());

	QVariantMap m = db->execSelectQueryOneRow("SELECT uuid FROM map");
	GameMap *map = new GameMap(m.value("uuid").toByteArray());

	sendProgress(target, func, ++step/maxStep);

	map->chaptersFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->storagesFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->objectivesFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->campaingsFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->missionsFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->missionLevelsFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->blockChapterMapsFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	map->inventoriesFromDb(db);
	if (!sendProgress(target, func, ++step/maxStep)) {
		delete map;
		return nullptr;
	}

	if (withImages) {
		map->imagesFromDb(db);
		if (!sendProgress(target, func, ++step/maxStep)) {
			delete map;
			return nullptr;
		}
	}

	return map;

}


/**
 * @brief GameMap::toDb
 * @param db
 * @return
 */

bool GameMap::toDb(CosDb *db) const
{
	qreal step = 0.0;
	qreal maxStep = 11.0;

	Q_ASSERT(db);
	Q_ASSERT(db->isValid());
	Q_ASSERT(db->isOpen());

	if (!sendProgress(step/maxStep))
		return false;

	if (!db->execSimpleQuery("PRAGMA foreign_keys = ON"))
		return false;

	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS map ("
							 "uuid TEXT"
							 ")"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM map"))		return false;

	QVariantMap l;
	l["uuid"] = QString(m_uuid);

	if (db->execInsertQuery("INSERT INTO map (?k?) VALUES (?)", l) == -1)
		return false;

	if (!sendProgress(++step/maxStep))
		return false;

	if (!imagesToDb(db))
		return false;

	if (!sendProgress(++step/maxStep))
		return false;

	chaptersToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	storagesToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	objectivesToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	campaignsToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	missionsToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	if (!missionLevelsToDb(db))
		return false;

	if (!sendProgress(++step/maxStep))
		return false;

	missionLocksToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	blockChapterMapsToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;

	inventoriesToDb(db);
	if (!sendProgress(++step/maxStep))
		return false;


	return true;

}




/**
 * @brief GameMap::campaign
 * @param id
 * @return
 */

GameMap::Campaign *GameMap::campaign(const qint32 &id) const
{
	if (id == -1)
		return nullptr;

	foreach(GameMap::Campaign *c, m_campaigns) {
		if (c->id() == id)
			return c;
	}

	return nullptr;
}




/**
 * @brief GameMap::chapter
 * @param id
 * @return
 */

GameMap::Chapter *GameMap::chapter(const qint32 &id) const
{
	if (id == -1)
		return nullptr;

	foreach(Chapter *c, m_chapters) {
		if (c->id() == id)
			return c;
	}

	return nullptr;
}


/**
 * @brief GameMap::mission
 * @param uuid
 * @return
 */

GameMap::Mission *GameMap::mission(const QByteArray &uuid) const
{
	if (uuid.isEmpty())
		return nullptr;

	foreach(Campaign *c, m_campaigns) {
		foreach(Mission *m, c->missions()) {
			if (m->uuid() == uuid)
				return m;
		}
	}

	return nullptr;
}


/**
 * @brief GameMap::missionLevel
 * @param uuid
 * @param level
 * @return
 */

GameMap::MissionLevel *GameMap::missionLevel(const QByteArray &uuid, const qint32 &level) const
{
	if (uuid.isEmpty() || level == -1)
		return nullptr;

	foreach(Campaign *c, m_campaigns) {
		foreach(Mission *m, c->missions()) {
			if (m->uuid() == uuid) {
				foreach(MissionLevel *l, m->levels()) {
					if (l->level() == level) {
						return l;
					}
				}
			}
		}
	}

	return nullptr;
}


/**
 * @brief GameMap::objective
 * @param uuid
 * @return
 */

GameMap::Objective *GameMap::objective(const QByteArray &uuid) const
{
	if (uuid.isEmpty())
		return nullptr;

	foreach(Chapter *c, m_chapters) {
		foreach(Objective *m, c->objectives()) {
			if (m->uuid() == uuid)
				return m;
		}
	}

	return nullptr;
}


/**
 * @brief GameMap::lockTree
 * @return
 */

GameMap::MissionLockHash GameMap::missionLockTree(Mission **errMission) const
{
	MissionLockHash hash;

	foreach (Campaign *c, m_campaigns) {
		foreach (Mission *m, c->missions()) {
			QVector<GameMap::MissionLock> list;
			if (!m->getLockTree(&list, m)) {
				qWarning() << QObject::tr("Redundant locks");
				if (errMission)
					*errMission = m;

				return MissionLockHash();
			}
			hash[m] = list;
		}
	}

	return hash;
}


/**
 * @brief GameMap::campaignLockTree
 * @param errCampaign
 * @return
 */

GameMap::CampaignLockHash GameMap::campaignLockTree(GameMap::Campaign **errCampaign) const
{
	CampaignLockHash hash;

	foreach (Campaign *c, m_campaigns) {
		QVector<Campaign *> list;

		if (!c->getLockTree(&list, c)) {
			qWarning() << QObject::tr("Redundant locks");
			if (errCampaign)
				*errCampaign = c;

			return CampaignLockHash();
		}
		hash[c] = list;
	}

	return hash;
}


/**
 * @brief GameMap::setProgressFunc
 * @param target
 * @param func
 */

void GameMap::setProgressFunc(QObject *target, const QString &func)
{
	m_progressObject = target;
	m_progressFunc = func;
}





/**
 * @brief GameMap::chaptersToStream
 * @param stream
 */

void GameMap::chaptersToStream(QDataStream &stream) const
{
	stream << (quint32) m_chapters.size();

	foreach (GameMap::Chapter *ch, m_chapters) {
		stream << ch->id();
		stream << ch->name();

		objectivesToStream(ch, stream);
	}
}




/**
 * @brief GameMap::chaptersFromStream
 * @param stream
 */

bool GameMap::chaptersFromStream(QDataStream &stream)
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

		Chapter *ch = new Chapter(id, name);
		objectivesFromStream(ch, stream);

		addChapter(ch);
	}

	if (m_chapters.size() != (int) size)
		return false;

	qDebug() << "Chapters loaded";

	return true;
}



/**
 * @brief GameMap::chaptersToDb
 * @param db
 */

bool GameMap::chaptersToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS chapters ("
							 "id INTEGER PRIMARY KEY,"
							 "name TEXT"
							 ");"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM chapters"))		return false;


	foreach (GameMap::Chapter *ch, m_chapters) {
		QVariantMap l;
		l["id"] = ch->id();
		l["name"] = ch->name();

		if (db->execInsertQuery("INSERT INTO chapters (?k?) VALUES (?)", l) == -1)
			return false;
	}

return true;
}


/**
 * @brief GameMap::chaptersFromDb
 * @param db
 */

void GameMap::chaptersFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT id, name FROM chapters");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();
		int chapterId = m.value("id").toInt();

		Chapter *ch = new Chapter(chapterId, m.value("name").toString());

		QVariantList ll;
		ll.append(chapterId);

		addChapter(ch);
	}
}




/**
 * @brief GameMap::storagesToStream
 * @param chapter
 * @param stream
 */

void GameMap::storagesToStream(QDataStream &stream) const
{
	stream << (quint32) m_storages.size();

	foreach (GameMap::Storage *s, m_storages) {
		stream << s->id();
		stream << s->module();
		stream << s->data();
	}
}



/**
 * @brief GameMap::storagesFromStream
 * @param chapter
 * @param stream
 */

bool GameMap::storagesFromStream(QDataStream &stream)
{
	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 id = -1;
		QByteArray module;
		QVariantMap data;
		stream >> id >> module >> data;

		if (id == -1)
			return false;

		Storage *s = new Storage(id, module, data);

		addStorage(s);
	}

	if (m_storages.size() != (int) size)
		return false;

	return true;
}



/**
 * @brief GameMap::storagesToDb
 * @param db
 */

bool GameMap::storagesToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS storages ("
							 "id INTEGER PRIMARY KEY,"
							 "module TEXT NOT NULL,"
							 "data TEXT"
							 ");"))
		return false;

	if (!db->execSimpleQuery("DELETE FROM storages"))		return false;


	foreach (GameMap::Storage *s, m_storages) {
		QJsonDocument doc(QJsonObject::fromVariantMap(s->data()));

		QVariantMap l;
		l["id"] = s->id();
		l["module"] = QString(s->module());
		l["data"] = QString(doc.toJson(QJsonDocument::Compact));

		if (db->execInsertQuery("INSERT INTO storages (?k?) VALUES (?)", l) == -1)
			return false;
	}

	return true;
}


/**
 * @brief GameMap::storagesFromDb
 * @param db
 */

void GameMap::storagesFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT id, module, data FROM storages");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();

		QJsonDocument doc = QJsonDocument::fromJson(m.value("data").toByteArray());
		Storage *s = new Storage(m.value("id").toInt(), m.value("module").toByteArray(), doc.object().toVariantMap());

		addStorage(s);
	}
}




/**
 * @brief GameMap::objectivesToStream
 * @param chapter
 * @param stream
 */

void GameMap::objectivesToStream(Chapter *chapter, QDataStream &stream) const
{
	Q_ASSERT(chapter);

	stream << (quint32) chapter->objectives().size();

	foreach (GameMap::Objective *o, chapter->objectives()) {
		stream << o->uuid();
		stream << o->module();
		stream << (o->storage() ? o->storage()->id() : (qint32) -1);
		stream << o->data();
	}
}



/**
 * @brief GameMap::objectivesFromStream
 * @param chapter
 * @param stream
 */

bool GameMap::objectivesFromStream(Chapter *chapter, QDataStream &stream)
{
	Q_ASSERT(chapter);

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		QByteArray module;
		qint32 storageId;
		QVariantMap data;
		stream >> uuid >> module >> storageId >> data;

		Objective *o = new Objective(uuid, module, storage(storageId), data);

		chapter->addObjective(o);
	}

	if (chapter->objectives().size() != (int) size)
		return false;

	return true;
}


/**
 * @brief GameMap::objectivesToDb
 * @param db
 */

bool GameMap::objectivesToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS objectives ("
							 "uuid TEXT NOT NULL PRIMARY KEY,"
							 "chapter INTEGER REFERENCES chapters(id) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "module TEXT NOT NULL,"
							 "storage INTEGER REFERENCES storages(id) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "data TEXT,"
							 "UNIQUE (uuid)"
							 ");"))
		return false;

	if (!db->execSimpleQuery("DELETE FROM objectives"))		return false;


	foreach (GameMap::Chapter *ch, m_chapters) {
		foreach (GameMap::Objective *o, ch->objectives()) {
			QJsonDocument doc(QJsonObject::fromVariantMap(o->data()));

			QVariantMap l;
			l["uuid"] = QString(o->uuid());
			l["chapter"] = ch->id();
			l["module"] = QString(o->module());
			if (o->storage())
				l["storage"] = o->storage()->id();
			else
				l["storage"] = QVariant::Invalid;
			l["data"] = QString(doc.toJson(QJsonDocument::Compact));

			if (db->execInsertQuery("INSERT INTO objectives (?k?) VALUES (?)", l) == -1)
				return false;
		}
	}

	return true;
}


/**
 * @brief GameMap::objectivesFromDb
 * @param db
 */

bool GameMap::objectivesFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT uuid, module, data, storage, chapters.id as chapterid FROM objectives "
										 "LEFT JOIN chapters ON (chapters.id=objectives.chapter)");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();

		Chapter *c = chapter(m.value("chapterid").toInt());

		if (!c)
			return false;

		QJsonDocument doc = QJsonDocument::fromJson(m.value("data").toByteArray());
		Objective *o = new Objective(m.value("uuid").toByteArray(), m.value("module").toByteArray(),
									 storage(m.value("storage").toInt()),
									 doc.object().toVariantMap());

		c->addObjective(o);
	}

	return true;
}







/**
 * @brief GameMap::campaignsToStream
 * @param stream
 */

void GameMap::campaignsToStream(QDataStream &stream) const
{
	stream << (quint32) m_campaigns.size();

	foreach (GameMap::Campaign *c, m_campaigns) {
		stream << c->id();
		stream << c->name();

		stream << (quint32) c->locks().size();

		foreach (GameMap::Campaign *cl, c->locks()) {
			stream << cl->id();
		}

		missionsToStream(c, stream);

	}
}


/**
 * @brief GameMap::campaignsFromStream
 * @param stream
 */

bool GameMap::campaignsFromStream(QDataStream &stream)
{
	QHash<qint32, QList<qint32>> lockList;
	QHash<QByteArray, QList<QPair<QByteArray, qint32>>> mLockList;

	quint32 size = 0;
	stream >> size;

	qDebug() << "Load campaigns" << size;

	for (quint32 i=0; i<size; i++) {
		qint32 id = -1;
		QString name;
		quint32 lockSize = 0;

		stream >> id >> name >> lockSize;

		qDebug() << "Load campaign" << id << name << lockSize;

		if (id == -1)
			return false;

		Campaign *c = new Campaign(id, name);


		QList<qint32> ll;

		for (quint32 j=0; j<lockSize; j++) {
			qint32 id = -1;
			stream >> id;

			if (id == -1) {
				delete c;
				return false;
			}

			ll.append(id);
		}

		if (!ll.isEmpty())
			lockList[id] = ll;

		qDebug() << "Load missions";

		mLockList.insert(missionsFromStream(c, stream));

		addCampaign(c);

		qDebug() << "Campaign loaded" << id << name;
	}

	if (m_campaigns.size() != (int) size)
		return false;

	qDebug() << "Campaigns loaded";

	// Campaign locks

	qDebug() << "Load campaign locks";

	{
		QHash<qint32, QList<qint32>>::const_iterator it;

		for (it=lockList.constBegin(); it != lockList.constEnd(); ++it) {
			Campaign *c = campaign(it.key());
			if (!c)
				return false;

			foreach (qint32 cid, it.value()) {
				Campaign *cc = campaign(cid);

				if (!cc)
					return false;

				c->addLock(cc);
			}
		}
	}

	qDebug() << "Load missions locks";

	// Mission locks

	{
		QHash<QByteArray, QList<QPair<QByteArray, qint32>>>::const_iterator mit;

		for (mit=mLockList.constBegin(); mit != mLockList.constEnd(); ++mit) {
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

	return true;
}


/**
 * @brief GameMap::campaignsToDb
 * @param db
 */

bool GameMap::campaignsToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS campaigns ("
							 "id INTEGER PRIMARY KEY,"
							 "name TEXT"
							 ");"))
		return false;

	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS campaignLocks ("
							 "campaign INTEGER NOT NULL REFERENCES campaigns(id) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "lock INTEGER NOT NULL REFERENCES campaigns(id) ON DELETE CASCADE ON UPDATE CASCADE"
							 ");"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM campaignLocks"))		return false;
	if (!db->execSimpleQuery("DELETE FROM campaigns"))		return false;


	foreach (Campaign *c, m_campaigns) {
		QVariantMap l;
		l["id"] = c->id();
		l["name"] = c->name();

		if (db->execInsertQuery("INSERT INTO campaigns (?k?) VALUES (?)", l) == -1)
			return false;
	}


	foreach (Campaign *c, m_campaigns) {
		foreach (Campaign *l, c->locks()) {
			QVariantMap m;
			m["campaign"] = c->id();
			m["lock"] = l->id();

			if (db->execInsertQuery("INSERT INTO campaignLocks (?k?) VALUES (?)", m) == -1)
				return false;
		}
	}

	return true;
}


/**
 * @brief GameMap::campaingsFromDb
 * @param db
 */

bool GameMap::campaingsFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT id, name FROM campaigns");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();
		int campaignId = m.value("id").toInt();

		Campaign *c = new Campaign(campaignId, m.value("name").toString());

		addCampaign(c);
	}

	QVariantList ll = db->execSelectQuery("SELECT campaign, lock FROM campaignLocks");

	foreach (QVariant v, ll) {
		QVariantMap m = v.toMap();

		Campaign *c = campaign(m.value("campaign").toInt());
		Campaign *l = campaign(m.value("lock").toInt());

		if (!c || !l)
			return false;

		c->addLock(l);
	}

	return true;
}




/**
 * @brief GameMap::missionsToStream
 * @param campaign
 * @param stream
 */

bool GameMap::missionsToStream(GameMap::Campaign *campaign, QDataStream &stream) const
{
	Q_ASSERT(campaign);

	stream << (quint32) campaign->missions().size();

	foreach (Mission *m, campaign->missions()) {
		stream << m->uuid();
		stream << m->mandatory();
		stream << m->name();

		stream << (quint32) m->locks().size();

		foreach (MissionLock p, m->locks()) {
			Mission *mm = p.first;
			if (!mm)
				return false;

			stream << mm->uuid();
			stream << (qint32) p.second;
		}

		missionLevelsToStream(m, stream);
	}

	return true;
}


/**
 * @brief GameMap::missionsFromStream
 * @param campaign
 * @param stream
 */

QHash<QByteArray, QList<QPair<QByteArray, qint32>>>
GameMap::missionsFromStream(GameMap::Campaign *campaign, QDataStream &stream)
{
	Q_ASSERT(campaign);

	QHash<QByteArray, QList<QPair<QByteArray, qint32>>> lockList;

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		QByteArray uuid;
		bool mandatory;
		QString name;
		quint32 lockSize = 0;

		stream >> uuid >> mandatory >> name >> lockSize;

		if (uuid.isEmpty())
			return QHash<QByteArray, QList<QPair<QByteArray, qint32>>>();

		Mission *m = new Mission(uuid, mandatory, name);

		QList<QPair<QByteArray, qint32>> ll;

		for (quint32 j=0; j<lockSize; j++) {
			QByteArray uuid;
			qint32 level = -1;
			stream >> uuid >> level;

			if (uuid.isEmpty()) {
				delete m;
				return QHash<QByteArray, QList<QPair<QByteArray, qint32>>>();
			}

			ll.append(qMakePair<QByteArray, qint32>(uuid, level));
		}

		if (!ll.isEmpty())
			lockList[uuid] = ll;



		missionLevelsFromStream(m, stream);

		campaign->addMission(m);
	}

	if (campaign->missions().size() != (int) size)
		return QHash<QByteArray, QList<QPair<QByteArray, qint32>>>();

	return lockList;
}


/**
 * @brief GameMap::missionsToDb
 * @param db
 */

bool GameMap::missionsToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS missions ("
							 "uuid TEXT PRIMARY KEY,"
							 "campaign INTEGER NOT NULL REFERENCES campaigns(id) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "mandatory BOOL,"
							 "name TEXT"
							 ");"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM missions"))		return false;


	foreach (Campaign *c, m_campaigns) {
		foreach (Mission *m, c->missions()) {
			QVariantMap l;
			l["campaign"] = c->id();
			l["name"] = m->name();
			l["mandatory"] = m->mandatory();
			l["uuid"] = QString(m->uuid());

			if (db->execInsertQuery("INSERT INTO missions (?k?) VALUES (?)", l) == -1)
				return false;
		}
	}

	return true;
}




/**
 * @brief GameMap::missionsFromDb
 * @param db
 */

bool GameMap::missionsFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT uuid, campaign, mandatory, name FROM missions");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();

		Campaign *c = campaign(m.value("campaign").toInt());

		if (!c)
			return false;

		Mission *mis = new Mission(m.value("uuid").toByteArray(), m.value("mandatory").toBool(), m.value("name").toString());

		c->addMission(mis);
	}


	QVariantList ll = db->execSelectQuery("SELECT mission, lock, level FROM missionLocks");

	foreach (QVariant v, ll) {
		QVariantMap m = v.toMap();

		Mission *mis = mission(m.value("mission").toByteArray());
		Mission *l = mission(m.value("lock").toByteArray());
		qint32 level = m.value("level").isNull() ? -1 : m.value("level").toInt();

		if (!mis || !l)
			return false;

		mis->addLock(l, level);
	}

	return true;
}


/**
 * @brief GameMap::missionLocksToDb
 * @param db
 */

bool GameMap::missionLocksToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS missionLocks ("
							 "mission TEXT NOT NULL REFERENCES missions(uuid) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "lock TEXT NOT NULL REFERENCES missions(uuid) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "level INTEGER,"
							 "FOREIGN KEY(lock, level) REFERENCES missionLevels(mission, level) ON DELETE CASCADE ON UPDATE CASCADE"
							 ");"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM missionLocks"))		return false;

	foreach (Campaign *c, m_campaigns) {
		foreach (Mission *mis, c->missions()) {
			foreach (MissionLock p, mis->locks()) {
				QVariantMap m;
				m["mission"] = QString(mis->uuid());
				m["lock"] = QString(p.first->uuid());
				if (p.second == -1)
					m["level"] = QVariant::Invalid;
				else
					m["level"] = p.second;

				if (db->execInsertQuery("INSERT INTO missionLocks (?k?) VALUES (?)", m) == -1)
					return false;
			}
		}
	}

	return true;
}


/**
 * @brief GameMap::missionLevelsToStream
 * @param stream
 */

void GameMap::missionLevelsToStream(Mission *mission, QDataStream &stream) const
{
	Q_ASSERT(mission);

	stream << (quint32) mission->levels().size();

	foreach (MissionLevel *m, mission->levels()) {
		stream << m->level();
		stream << m->terrain();
		stream << m->startHP();
		stream << m->duration();
		stream << m->startBlock();
		stream << m->imageFolder();
		stream << m->imageFile();

		blockChapterMapsToStream(m, stream);
		inventoriesToStream(m, stream);
	}
}


/**
 * @brief GameMap::missionLevelsFromStream
 * @param mission
 * @param stream
 */

bool GameMap::missionLevelsFromStream(GameMap::Mission *mission, QDataStream &stream)
{
	Q_ASSERT(mission);

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 level = -1;
		QByteArray terrain;
		qint32 startHP = -1;
		qint32 duration = -1;
		qint32 startBlock = -1;
		QString imageFolder, imageFile;

		stream >> level >> terrain >> startHP >> duration >> startBlock >> imageFolder >> imageFile;

		if (level == -1 || startHP == -1 || duration == -1 || startBlock == -1)
			return false;

		MissionLevel *m = new MissionLevel(level, terrain, startHP, duration, startBlock, imageFolder, imageFile);

		blockChapterMapsFromStream(m, stream);
		inventoriesFromStream(m, stream);

		mission->addMissionLevel(m);
	}

	if (mission->levels().size() != (int) size)
		return false;

	return true;
}


/**
 * @brief GameMap::missionLevelsToDb
 * @param db
 */

bool GameMap::missionLevelsToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS missionLevels ("
							 "mission TEXT NOT NULL REFERENCES missions(uuid) ON DELETE CASCADE ON UPDATE CASCADE,"
							 "level INTEGER NOT NULL CHECK (level>0),"
							 "terrain TEXT,"
							 "startHP INTEGER NOT NULL DEFAULT 5 CHECK (startHP>0),"
							 "duration INTEGER NOT NULL DEFAULT 600 CHECK (duration>0),"
							 "startBlock INTEGER NOT NULL DEFAULT 1 CHECK (startBlock>0),"
							 "imageFolder TEXT,"
							 "imageFile TEXT,"
							 "PRIMARY KEY (mission, level),"
							 "FOREIGN KEY (imageFolder, imageFile) REFERENCES images(folder, file) ON DELETE SET NULL ON UPDATE CASCADE"
							 ")"))
		return false;

	if (!db->execSimpleQuery("DELETE FROM missionLevels"))		return false;


	foreach (Campaign *c, m_campaigns) {
		foreach (Mission *m, c->missions()) {
			foreach (MissionLevel *ml, m->levels()) {
				QVariantMap l;
				l["mission"] = QString(m->uuid());
				l["level"] = ml->level();
				l["terrain"] = QString(ml->terrain());
				l["startHP"] = ml->startHP();
				l["duration"] = ml->duration();
				l["startBlock"] = ml->startBlock();

				if (ml->imageFolder().isEmpty())
					l["imageFolder"] = QVariant::Invalid;
				else
					l["imageFolder"] = ml->imageFolder();

				if (ml->imageFile().isEmpty())
					l["imageFile"] = QVariant::Invalid;
				else
					l["imageFile"] = ml->imageFile();

				if (db->execInsertQuery("INSERT INTO missionLevels (?k?) VALUES (?)", l) == -1) {
					return false;
				}
			}
		}
	}

	return true;
}


/**
 * @brief GameMap::missionLevelsFromDb
 * @param db
 */

bool GameMap::missionLevelsFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT mission, level, terrain, startHP, duration, startBlock, imageFolder, imageFile FROM missionLevels");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();

		Mission *mis = mission(m.value("mission").toByteArray());

		if (!mis)
			return false;

		mis->addMissionLevel(new MissionLevel(m.value("level").toInt(),
											  m.value("terrain").toByteArray(),
											  m.value("startHP").toInt(),
											  m.value("duration").toInt(),
											  m.value("startBlock").toInt(),
											  m.value("imageFolder").toString(),
											  m.value("imageFile").toString()
											  ));
	}

	return true;
}


/**
 * @brief GameMap::blockChapterMapsToStream
 * @param level
 * @param stream
 */

void GameMap::blockChapterMapsToStream(GameMap::MissionLevel *level, QDataStream &stream) const
{
	Q_ASSERT(level);

	stream << (quint32) level->blockChapterMaps().size();

	foreach (BlockChapterMap *m, level->blockChapterMaps()) {
		stream << m->maxObjective();

		stream << (quint32) m->blocks().size();

		foreach (qint32 b, m->blocks())
			stream << b;


		stream << (quint32) m->chapters().size();

		foreach (Chapter *ch, m->chapters())
			stream << ch->id();


		stream << (quint32) m->favorites().size();

		foreach (Objective *o, m->favorites())
			stream << o->uuid();
	}
}


/**
 * @brief GameMap::blockChapterMapsFromStream
 * @param level
 * @param stream
 */

bool GameMap::blockChapterMapsFromStream(GameMap::MissionLevel *level, QDataStream &stream)
{
	Q_ASSERT(level);

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 maxObj = -1;

		stream >> maxObj;

		if (maxObj < 0)
			return false;

		BlockChapterMap *b = new BlockChapterMap(maxObj);

		{
			quint32 size = 0;
			stream >> size;
			for (quint32 i=0; i<size; i++) {
				qint32 block = -1;
				stream >> block;

				if (block < 0)
					return false;

				b->addBlock(block);
			}

			if (b->blocks().size() != (int) size)
				return false;
		}


		{
			quint32 size = 0;
			stream >> size;
			for (quint32 i=0; i<size; i++) {
				qint32 chId = -1;
				stream >> chId;

				if (chId == -1)
					return false;

				Chapter *ch = chapter(chId);

				if (!ch)
					return false;

				b->addChapter(ch);
			}
			if (b->chapters().size() != (int) size)
				return false;
		}


		{
			quint32 size = 0;
			stream >> size;
			for (quint32 i=0; i<size; i++) {
				QByteArray f;
				stream >> f;

				if (f.isEmpty())
					return false;

				Objective *ch = objective(f);

				if (!ch)
					return false;

				b->addFavorite(ch);
			}
			if (b->favorites().size() != (int) size)
				return false;
		}


		level->addBlockChapterMap(b);
	}

	if (level->blockChapterMaps().size() != (int) size)
		return false;

	return true;
}



/**
 * @brief GameMap::blockChapterMapsToDb
 * @param db
 */

bool GameMap::blockChapterMapsToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS blockChapterMaps ("
							 "id INTEGER PRIMARY KEY,"
							 "mission TEXT NOT NULL,"
							 "level INTEGER NOT NULL,"
							 "maxObjective INTEGER NOT NULL DEFAULT 0 CHECK (maxObjective>=0),"
							 "FOREIGN KEY (mission, level) REFERENCES missionLevels(mission, level) ON UPDATE CASCADE ON DELETE CASCADE"
							 ")"))
		return false;

	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS blockChapterMapBlocks ("
							 "blockid INTEGER NOT NULL REFERENCES blockChapterMaps(id) ON UPDATE CASCADE ON DELETE CASCADE,"
							 "block INTEGER NOT NULL CHECK (block>0)"
							 ")"))
		return false;

	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS blockChapterMapChapters ("
							 "blockid INTEGER NOT NULL REFERENCES blockChapterMaps(id) ON UPDATE CASCADE ON DELETE CASCADE,"
							 "chapter INTEGER NOT NULL REFERENCES chapters(id) ON UPDATE CASCADE ON DELETE CASCADE"
							 ")"))
		return false;

	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS blockChapterMapFavorites ("
							 "blockid INTEGER NOT NULL REFERENCES blockChapterMaps(id) ON UPDATE CASCADE ON DELETE CASCADE,"
							 "objective TEXT NOT NULL REFERENCES objectives(uuid) ON UPDATE CASCADE ON DELETE CASCADE"
							 ")"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM blockChapterMapFavorites"))		return false;
	if (!db->execSimpleQuery("DELETE FROM blockChapterMapChapters"))		return false;
	if (!db->execSimpleQuery("DELETE FROM blockChapterMapBlocks"))		return false;
	if (!db->execSimpleQuery("DELETE FROM blockChapterMaps"))		return false;


	foreach (Campaign *c, m_campaigns) {
		foreach (Mission *m, c->missions()) {
			foreach (MissionLevel *ml, m->levels()) {
				foreach (BlockChapterMap *b, ml->blockChapterMaps()) {
					QVariantMap l;
					l["mission"] = QString(m->uuid());
					l["level"] = ml->level();
					l["maxObjective"] = b->maxObjective();

					int id = db->execInsertQuery("INSERT INTO blockChapterMaps (?k?) VALUES (?)", l);

					if (id == -1)
						return false;

					foreach (qint32 bl, b->blocks()) {
						QVariantMap l;
						l["blockid"] = id;
						l["block"] = bl;

						if (db->execInsertQuery("INSERT INTO blockChapterMapBlocks (?k?) VALUES (?)", l) == -1)
							return false;
					}


					foreach (Chapter *ch, b->chapters()) {
						QVariantMap l;
						l["blockid"] = id;
						l["chapter"] = ch->id();

						if (db->execInsertQuery("INSERT INTO blockChapterMapChapters (?k?) VALUES (?)", l) == -1)
							return false;
					}

					foreach (Objective *o, b->favorites()) {
						QVariantMap l;
						l["blockid"] = id;
						l["objective"] = QString(o->uuid());

						if (db->execInsertQuery("INSERT INTO blockChapterMapFavorites (?k?) VALUES (?)", l) == -1)
							return false;
					}
				}
			}
		}
	}

	return true;
}


/**
 * @brief GameMap::blockChapterMapsFromDb
 * @param db
 */

bool GameMap::blockChapterMapsFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT id, mission, level, maxObjective FROM blockChapterMaps");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();

		MissionLevel *mis = missionLevel(m.value("mission").toByteArray(), m.value("level").toInt());

		if (!mis)
			return false;

		BlockChapterMap *b = mis->addBlockChapterMap(new BlockChapterMap(m.value("maxObjective").toInt()));

		QVariantList bId;
		bId.append(m.value("id").toInt());

		{
			QVariantList l = db->execSelectQuery("SELECT block FROM blockChapterMapBlocks WHERE blockid=?", bId);

			foreach (QVariant v, l) {
				QVariantMap m = v.toMap();

				b->addBlock(m.value("block").toInt());
			}
		}

		{
			QVariantList l = db->execSelectQuery("SELECT chapter FROM blockChapterMapChapters WHERE blockid=?", bId);

			foreach (QVariant v, l) {
				QVariantMap m = v.toMap();

				Chapter *ch = chapter(m.value("chapter").toInt());

				if (!ch)
					return false;

				b->addChapter(ch);
			}
		}

		{
			QVariantList l = db->execSelectQuery("SELECT objective FROM blockChapterMapFavorites WHERE blockid=?", bId);

			foreach (QVariant v, l) {
				QVariantMap m = v.toMap();

				Objective *ch = objective(m.value("objective").toByteArray());

				if (!ch)
					return false;

				b->addFavorite(ch);
			}
		}
	}

	return true;
}


/**
 * @brief GameMap::inventoriesToStream
 * @param level
 * @param stream
 */

void GameMap::inventoriesToStream(GameMap::MissionLevel *level, QDataStream &stream) const
{
	Q_ASSERT(level);

	stream << (quint32) level->inventories().size();

	foreach (Inventory *m, level->inventories()) {
		stream << m->block();
		stream << m->module();
		stream << m->count();
	}
}


/**
 * @brief GameMap::inventoriesFromStream
 * @param level
 * @param stream
 */

bool GameMap::inventoriesFromStream(GameMap::MissionLevel *level, QDataStream &stream)
{
	Q_ASSERT(level);

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		qint32 block = -1;
		QByteArray module;
		qint32 count = -1;

		stream >> block >> module >> count;

		if (block < 0 || module.isEmpty() || count <= 0)
			return false;

		level->addInvetory(new Inventory(block, module, count));
	}

	if (level->inventories().size() != (int) size)
		return false;

	return true;
}


/**
 * @brief GameMap::inventoriesToDb
 * @param db
 */

bool GameMap::inventoriesToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS inventories ("
							 "mission TEXT NOT NULL,"
							 "level INTEGER NOT NULL,"
							 "block INTEGER NOT NULL DEFAULT 0 CHECK (block>=0),"
							 "module TEXT NOT NULL,"
							 "count INTEGER NOT NULL DEFAULT 1 CHECK (count>0),"
							 "FOREIGN KEY (mission, level) REFERENCES missionLevels(mission, level) ON UPDATE CASCADE ON DELETE CASCADE"
							 ")"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM inventories"))		return false;


	foreach (Campaign *c, m_campaigns) {
		foreach (Mission *m, c->missions()) {
			foreach (MissionLevel *ml, m->levels()) {
				foreach (Inventory *b, ml->inventories()) {
					QVariantMap l;
					l["mission"] = QString(m->uuid());
					l["level"] = ml->level();
					l["block"] = b->block();
					l["module"] = QString(b->module());
					l["count"] = b->count();

					int id = db->execInsertQuery("INSERT INTO inventories (?k?) VALUES (?)", l);

					if (id == -1)
						return false;
				}
			}
		}
	}

	return true;
}


/**
 * @brief GameMap::inventoriesFromDb
 * @param db
 */

bool GameMap::inventoriesFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT mission, level, block, module, count FROM inventories");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();

		MissionLevel *mis = missionLevel(m.value("mission").toByteArray(), m.value("level").toInt());

		if (!mis)
			return false;

		mis->addInvetory(new Inventory(m.value("block").toInt(), m.value("module").toByteArray(), m.value("count").toInt()));
	}

	return true;
}


/**
 * @brief GameMap::imagesToStream
 * @param stream
 */

void GameMap::imagesToStream(QDataStream &stream) const
{
	stream << (quint32) m_images.size();

	foreach (Image *ch, m_images) {
		stream << ch->folder();
		stream << ch->file();
		stream << ch->data();
	}
}


/**
 * @brief GameMap::imagesFromStream
 * @param stream
 */

bool GameMap::imagesFromStream(QDataStream &stream)
{
	qDebug() << "Load images";

	quint32 size = 0;
	stream >> size;

	for (quint32 i=0; i<size; i++) {
		QString folder;
		QString file;
		QByteArray data;
		stream >> folder >> file >> data;

		if (folder.isEmpty() || file.isEmpty() || data.isEmpty())
			return false;

		addImage(new Image(folder, file, data));
	}

	if (m_images.size() != (int) size)
		return false;

	qDebug() << "Images loaded";

	return true;
}


/**
 * @brief GameMap::imagesToDb
 * @param db
 */

bool GameMap::imagesToDb(CosDb *db) const
{
	if (!db->execSimpleQuery("CREATE TABLE IF NOT EXISTS images ("
							 "folder TEXT NOT NULL,"
							 "file TEXT NOT NULL,"
							 "content BLOB NOT NULL,"
							 "PRIMARY KEY (folder, file)"
							 ");"))
		return false;


	if (!db->execSimpleQuery("DELETE FROM images"))		return false;


	foreach (Image *ch, m_images) {
		QVariantMap l;
		l["folder"] = ch->folder();
		l["file"] = ch->file();
		l["content"] = ch->data();

		if (db->execInsertQuery("INSERT INTO images (?k?) VALUES (?)", l) == -1)
			return false;
	}

	return true;
}




/**
 * @brief GameMap::deleteImages
 */

void GameMap::deleteImages()
{
	qInfo() << QObject::tr("Delete images");
	qDeleteAll(m_images.begin(), m_images.end());
	m_images.clear();
}




/**
 * @brief GameMap::imagesFromDb
 * @param db
 */


void GameMap::imagesFromDb(CosDb *db)
{
	QVariantList l = db->execSelectQuery("SELECT folder, file, content FROM images");

	foreach (QVariant v, l) {
		QVariantMap m = v.toMap();
		addImage(new Image(m.value("folder").toString(), m.value("file").toString(), m.value("content").toByteArray()));
	}
}


/**
 * @brief GameMap::sendProgress
 * @param progress
 */

bool GameMap::sendProgress(const qreal &progress) const
{
	return sendProgress(m_progressObject, m_progressFunc, progress);
}




/**
 * @brief GameMap::sendProgress
 * @param target
 * @param func
 * @param progress
 */

bool GameMap::sendProgress(QObject *target, const QString &func, const qreal &progress)
{
	bool abortRequest = false;
	if (target && !func.isEmpty()) {
		QMetaObject::invokeMethod(target, func.toLatin1(), Qt::DirectConnection, Q_RETURN_ARG(bool, abortRequest), Q_ARG(qreal, progress));
	}

	return !abortRequest;
}







/**
 * @brief GameMap::Storage::Storage
 * @param id
 * @param module
 * @param data
 */

GameMap::Storage::Storage(const qint32 &id, const QByteArray &module, const QVariantMap &data)
	: m_id(id)
	, m_module(module)
	, m_data(data)
{

}


/**
 * @brief GameMap::Objective::Objective
 * @param uuid
 * @param module
 * @param storage
 * @param data
 */

GameMap::Objective::Objective(const QByteArray &uuid, const QByteArray &module, GameMap::Storage *storage, const QVariantMap &data)
	: m_uuid(uuid)
	, m_module(module)
	, m_storage(storage)
	, m_data(data)
{

}



/**
 * @brief GameMap::Chapter::Chapter
 * @param id
 * @param name
 */

GameMap::Chapter::Chapter(const qint32 &id, const QString &name)
	: m_id(id)
	, m_name(name)
	, m_objectives()
{

}


/**
 * @brief GameMap::Chapter::~Chapter
 */

GameMap::Chapter::~Chapter()
{
	qDeleteAll(m_objectives.begin(), m_objectives.end());
	m_objectives.clear();
}


/**
 * @brief GameMap::Chapter::storage
 * @param id
 * @return
 */

GameMap::Storage *GameMap::storage(const qint32 &id) const
{
	if (id == -1)
		return nullptr;

	foreach(GameMap::Storage *s, m_storages) {
		if (s->id() == id)
			return s;
	}

	return nullptr;
}





/**
 * @brief GameMap::BlockChapterMap::BlockChapterMap
 * @param block
 * @param maxObjective
 */

GameMap::BlockChapterMap::BlockChapterMap(const qint32 &maxObjective)
	: m_blocks()
	, m_chapters()
	, m_favorites()
	, m_maxObjective(maxObjective)
{

}


/**
 * @brief GameMap::Inventory::Inventory
 * @param block
 * @param module
 * @param count
 */

GameMap::Inventory::Inventory(const qint32 &block, const QByteArray &module, const qint32 &count)
	: m_block(block)
	, m_module(module)
	, m_count(count)
{

}


/**
 * @brief GameMap::MissionLevel::MissionLevel
 * @param missionUuid
 * @param level
 * @param terrain
 * @param startHP
 * @param duration
 * @param startBlock
 */

GameMap::MissionLevel::MissionLevel(const qint32 &level, const QByteArray &terrain,
									const qint32 &startHP, const qint32 &duration, const qint32 &startBlock,
									const QString &imageFolder, const QString &imageFile)
	: m_level(level)
	, m_terrain(terrain)
	, m_startHP(startHP)
	, m_duration(duration)
	, m_startBlock(startBlock)
	, m_blockChapterMaps()
	, m_inventories()
	, m_imageFolder(imageFolder)
	, m_imageFile(imageFile)
{

}


/**
 * @brief GameMap::MissionLevel::~MissionLevel
 */

GameMap::MissionLevel::~MissionLevel()
{
	qDeleteAll(m_blockChapterMaps.begin(), m_blockChapterMaps.end());
	m_blockChapterMaps.clear();

	qDeleteAll(m_inventories.begin(), m_inventories.end());
	m_inventories.clear();
}


/**
 * @brief GameMap::Mission::Mission
 * @param uuid
 * @param mandatory
 * @param name
 */

GameMap::Mission::Mission(const QByteArray &uuid, const bool &mandatory, const QString &name)
	: m_uuid(uuid)
	, m_mandatory(mandatory)
	, m_name(name)
	, m_levels()
	, m_locks()
{

}


/**
 * @brief GameMap::Mission::~Mission
 */

GameMap::Mission::~Mission()
{
	qDeleteAll(m_levels.begin(), m_levels.end());
	m_levels.clear();
}


/**
 * @brief GameMap::Mission::getLockTree
 * @param listPtr
 * @param rootMission
 * @return
 */

bool GameMap::Mission::getLockTree(QVector<GameMap::MissionLock> *listPtr, GameMap::Mission *rootMission) const
{
	foreach (MissionLock l, m_locks) {
		if (l.first == rootMission)				// redundant
			return false;

		bool contains = false;

		QVector<GameMap::MissionLock>::iterator it;

		for (it = (*listPtr).begin(); it != (*listPtr).end(); ++it) {
			if (it->first == l.first) {
				if (l.second > it->second)
					it->second = l.second;
				contains = true;
				break;
			}
		}

		if (!contains) {
			listPtr->append(l);
			if (!l.first->getLockTree(listPtr, rootMission))
				return false;
		}
	}

	return true;
}







/**
 * @brief GameMap::Campaign::Campaign
 * @param id
 * @param name
 */

GameMap::Campaign::Campaign(const qint32 &id, const QString &name)
	: m_id(id)
	, m_name(name)
	, m_locks()
	, m_missions()
{

}


/**
 * @brief GameMap::Campaign::~Campaign
 */

GameMap::Campaign::~Campaign()
{
	qDeleteAll(m_missions.begin(), m_missions.end());
	m_missions.clear();
}


/**
 * @brief GameMap::Campaign::getLockTree
 * @param listPtr
 * @param rootCampaign
 * @return
 */

bool GameMap::Campaign::getLockTree(QVector<GameMap::Campaign *> *listPtr, GameMap::Campaign *rootCampaign) const
{
	foreach (Campaign *c, m_locks) {
		if (c == rootCampaign)				// redundant
			return false;

		if (!listPtr->contains(c)) {
			listPtr->append(c);
			if (!c->getLockTree(listPtr, rootCampaign))
				return false;
		}
	}

	return true;
}




/**
 * @brief GameMap::Image::Image
 * @param folder
 * @param file
 * @param data
 */

GameMap::Image::Image(const QString &folder, const QString &file, const QByteArray &data)
	: m_folder(folder)
	, m_file(file)
	, m_data(data)
{

}
