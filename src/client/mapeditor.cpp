/*
 * ---- Call of Suli ----
 *
 * mapeditor.cpp
 *
 * Created on: 2020. 11. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapEditor
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

#include "mapeditor.h"
#include "sqlimage.h"
#include <QtConcurrent/QtConcurrent>
#include <QUuid>

MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(parent)
	, m_game(nullptr)
	, m_loadProgress(0.0)
	, m_loadProgressFraction(qMakePair<qreal, qreal>(0.0, 1.0))
	, m_loadAbortRequest(false)
	, m_modified(false)
{
	m_map["campaignAdd"] = &MapEditor::campaignAdd;
	m_map["campaignModify"] = &MapEditor::campaignModify;
	m_map["campaignRemove"] = &MapEditor::campaignRemove;
	m_map["campaignListReload"] = &MapEditor::campaignListReload;
	m_map["campaignLoad"] = &MapEditor::campaignLoad;
	m_map["campaignLockAdd"] = &MapEditor::campaignLockAdd;
	m_map["campaignLockRemove"] = &MapEditor::campaignLockRemove;
	m_map["campaignLockGetList"] = &MapEditor::campaignLockGetList;
	m_map["missionAdd"] = &MapEditor::missionAdd;
	m_map["missionRemove"] = &MapEditor::missionRemove;
	m_map["missionModify"] = &MapEditor::missionModify;
	m_map["missionLoad"] = &MapEditor::missionLoad;
	m_map["missionLockAdd"] = &MapEditor::missionLockAdd;
	m_map["missionLockRemove"] = &MapEditor::missionLockRemove;
	m_map["missionLockGetList"] = &MapEditor::missionLockGetList;
	m_map["missionLockGetLevelList"] = &MapEditor::missionLockGetLevelList;
	m_map["missionLockModify"] = &MapEditor::missionLockModify;
	m_map["missionLevelAdd"] = &MapEditor::missionLevelAdd;
	m_map["missionLevelRemove"] = &MapEditor::missionLevelRemove;
	m_map["missionLevelNormalize"] = &MapEditor::missionLevelNormalize;


	CosDb *db = new CosDb("editorDb", this);
	db->setDatabaseName(Client::standardPath("tmpmapeditor.db"));
	addDb(db);

	connect(db, &CosDb::undone, this, [=]() {
		setModified(true);
		run("campaignListReload");
	});

}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	if (m_game)
		delete m_game;
}


/**
 * @brief MapEditor::loadFromFile
 * @param filename
 */

void MapEditor::loadFromFilePrivate(QVariantMap data)
{
	QString filename = data.value("filename").toString();

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.2));
	setLoadProgress(0.0);

	QFile f(filename);
	f.open(QIODevice::ReadOnly);
	QByteArray d = f.readAll();
	f.close();

	m_game = GameMap::fromBinaryData(d, this, "setLoadProgress");

	if (!m_game) {
		m_client->sendMessageError(tr("Hibás fájl"), filename);
		db()->close();
		emit loadFailed();
		return;
	}

	QStringList unavailableTerrains = checkTerrains();
	if (!unavailableTerrains.isEmpty()) {
		m_client->sendMessageWarning(tr("Hiba"), tr("Érvénytelen terephivatkozás!"), unavailableTerrains.join(", "));
	}

	if (_createDatabase())
		emit loadFinished();
	else
		emit loadFailed();
}


/**
 * @brief MapEditor::saveToFile
 * @param data
 */

void MapEditor::saveToFilePrivate(QVariantMap data)
{
	QString filename = data.value("filename").toString();

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.9));
	setLoadProgress(0.0);

	m_loadAbortRequest = false;
	m_game = GameMap::fromDb(db(), this, "setLoadProgress");

	if (!m_game) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Adatbázis hiba"));
		emit saveFailed();
		return;
	}

	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Írási hiba"), filename);
		emit saveFailed();
		return;
	}

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.9, 1.0));
	QByteArray d = m_game->toBinaryData();

	f.write(d);
	f.close();

	emit saveFinished();
}









/**
 * @brief MapEditor::checkBackup
 */

void MapEditor::checkBackup()
{
	/*if (db()->databaseExists()) {
		emit backupReady("BACKUP", db()->databaseName());
		return;
	}

	emit backupUnavailable();*/

	removeBackup();
}


/**
 * @brief MapEditor::removeBackup
 */

void MapEditor::removeBackup()
{
	if (QFile::exists(db()->databaseName())) {
		if (!QFile::remove(db()->databaseName())) {
			m_client->sendMessageError(tr("Backup törlése"), tr("Nem sikerült törölni a fájlt"), db()->databaseName());
			return;
		}
	}

	emit backupUnavailable();
}


/**
 * @brief MapEditor::removeDatabase
 */

void MapEditor::removeDatabase()
{
	qDebug() << "Remove image provider";
	QQmlEngine *engine = qmlEngine(this);
	engine->removeImageProvider("mapdb");

	QString dbName = db()->databaseName();

	if (!db()->isOpen())
		db()->close();

	QFile::remove(dbName);
}


/**
 * @brief MapEditor::loadAbort
 */

void MapEditor::loadAbort()
{
	m_loadAbortRequest = true;
}


/**
 * @brief MapEditor::setMapName
 * @param mapName
 */


void MapEditor::setMapName(QString mapName)
{
	if (m_mapName == mapName)
		return;

	m_mapName = mapName;
	emit mapNameChanged(m_mapName);
}



/**
 * @brief MapEditor::setLoadProgress
 * @param loadProgress
 */

bool MapEditor::setLoadProgress(qreal loadProgress)
{
	m_loadProgress = m_loadProgressFraction.first+(m_loadProgressFraction.second-m_loadProgressFraction.first)*loadProgress;
	emit loadProgressChanged(m_loadProgress);

	return m_loadAbortRequest;
}



/**
 * @brief MapEditor::setLoadProgressFraction
 * @param loadProgressFraction
 */

void MapEditor::setLoadProgressFraction(QPair<qreal, qreal> loadProgressFraction)
{
	if (m_loadProgressFraction == loadProgressFraction)
		return;

	m_loadProgressFraction = loadProgressFraction;
	emit loadProgressFractionChanged(m_loadProgressFraction);
}




/**
 * @brief MapEditor::campaignAdd
 * @param data
 */

void MapEditor::campaignAdd(QVariantMap data)
{
	db()->undoLogBegin(tr("Új hadjárat hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO campaigns(?k?) values (?)", data);

	db()->undoLogEnd();

	campaignListReload();

	if (ret != -1) {
		setModified(true);
		emit campaignAdded(ret);
	}
}


/**
 * @brief MapEditor::campaignModify
 * @param data
 */

void MapEditor::campaignModify(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	QVariantMap d = data.value("data").toMap();

	if (id == -1 || d.isEmpty())
		return;

	db()->undoLogBegin(tr("Hadjárat módosítása"));

	QVariantMap bind;
	bind[":id"] = id;

	bool ret = db()->execUpdateQuery("UPDATE campaigns SET ? WHERE id=:id", d, bind);

	db()->undoLogEnd();

	if (ret) {
		campaignListReload();
		setModified(true);
		emit campaignModified(id);
	}
}


/**
 * @brief MapEditor::campaignRemove
 * @param data
 */

void MapEditor::campaignRemove(QVariantMap data)
{
	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		db()->undoLogBegin(tr("%1 hadjárat törlése").arg(list.count()));
		bool ret = db()->execListQuery("DELETE FROM campaigns WHERE id IN (?l?)", list);

		db()->undoLogEnd();

		if (ret) {
			campaignListReload();
			setModified(true);

			foreach (QVariant v, list)
				emit campaignRemoved(v.toInt());
		}

		return;
	}

	int id = data.value("id", -1).toInt();

	if (id == -1)
		return;

	db()->undoLogBegin(tr("Hadjárat törlése"));

	QVariantList l;
	l << id;

	bool ret = db()->execSimpleQuery("DELETE FROM campaigns WHERE id=?", l);

	db()->undoLogEnd();

	if (ret) {
		campaignListReload();
		setModified(true);
		emit campaignRemoved(id);
	}
}







/**
 * @brief MapEditor::setModified
 * @param modified
 */

void MapEditor::setModified(bool modified)
{
	if (m_modified == modified)
		return;

	m_modified = modified;
	emit modifiedChanged(m_modified);
}


/**
 * @brief MapEditor::loadFromFile
 * @param data
 */

void MapEditor::loadFromFile(QVariantMap data)
{
	QString filename = data.value("filename").toString();

	if (filename.isEmpty() || !QFile::exists(filename)) {
		m_client->sendMessageWarning(tr("A fájl nem található"), filename);
		//emit loadFailed();
		createNew(data);
		return;
	}

	if (!db()->isOpen()) {
		if (!db()->open()) {
			m_client->sendMessageError(tr("Belső hiba"), tr("Nem lehet előkészíteni az adatbázist!"));
			emit loadFailed();
			return;
		}
	}

	emit loadStarted();

	AbstractActivity::run(&MapEditor::loadFromFilePrivate, data);
}


/**
 * @brief MapEditor::saveToFile
 * @param data
 */

void MapEditor::saveToFile(QVariantMap data)
{
	QString filename = data.value("filename").toString();

	if (filename.isEmpty())
		return;

	if (!db()->isOpen()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Nincs megnyitva az adatbázis!"));
		emit saveFailed();
		return;
	}

	emit saveStarted();

	AbstractActivity::run(&MapEditor::saveToFilePrivate, data);
}


/**
 * @brief MapEditor::createNew
 * @param data
 */

void MapEditor::createNew(QVariantMap data)
{
	db()->open();

	emit loadStarted();

	AbstractActivity::run(&MapEditor::createNewPrivate, data);
}



/**
 * @brief MapEditor::onMessageReceived
 * @param message
 */

void MapEditor::onMessageReceived(const CosMessage &message)
{

}





/**
 * @brief MapEditor::databasePrepares
 * @return
 */

bool MapEditor::_createDatabase()
{
	setLoadProgressFraction(qMakePair<qreal, qreal>(0.2, 0.6));
	m_game->setProgressFunc(this, "setLoadProgress");

	if (!m_game->toDb(db())) {
		m_client->sendMessageError(tr("Adatfájl hiba"), db()->databaseName());
		db()->close();
		return false;
	}

	qDebug() << "Add sqlimage provider";
	QQmlEngine *engine = qmlEngine(this);
	SqlImage *sqlImage = new SqlImage(m_client, db(), "images");
	engine->addImageProvider("mapdb", sqlImage);

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.6, 1.0));
	return _createTriggers();
}





/**
 * @brief MapEditor::_createTriggers
 */

bool MapEditor::_createTriggers()
{
	QStringList tableList;
	tableList << "map";
	tableList << "chapters";
	tableList << "storages";
	tableList << "campaigns";
	tableList << "campaignLocks";
	tableList << "missions";
	tableList << "missionLocks";
	tableList << "missionLevels";
	tableList << "blockChapterMaps";
	tableList << "blockChapterMapBlocks";
	tableList << "blockChapterMapChapters";
	tableList << "blockChapterMapFavorites";
	tableList << "inventories";
	tableList << "images";


	qreal step = 0.0;
	qreal maxStep = (qreal) tableList.count()+1;

	setLoadProgress(step/maxStep);

	if (!db()->createUndoTables())	return false;

	setLoadProgress(++step/maxStep);


	foreach (QString k, tableList) {
		db()->createTrigger(k);
		setLoadProgress(++step/maxStep);
	}

	return true;
}



/**
 * @brief MapEditor::_loadFromNew
 */

void MapEditor::createNewPrivate(QVariantMap data)
{
	QString name = data.value("name").toString();
	QByteArray uuid = data.value("uuid").toByteArray();

	if (uuid.isEmpty())
		uuid = QUuid::createUuid().toByteArray();

	m_game = GameMap::example(name, uuid);

	if (_createDatabase())
		emit loadFinished();
	else
		emit loadFailed();
}


/**
 * @brief MapEditor::checkTerrains
 */

QStringList MapEditor::checkTerrains() const
{
	if (!m_client || !m_game)
		return QStringList();

	QStringList list;

	foreach (GameMap::Campaign *c, m_game->campaigns()) {
		foreach (GameMap::Mission *m, c->missions()) {
			foreach (GameMap::MissionLevel *l, m->levels()) {
				if (m_client->terrain(l->terrain()).name.isEmpty())
					list.append(l->terrain());
			}
		}
	}

	return list;
}





/**
 * @brief MapEditor::_campaignListReload
 */

void MapEditor::campaignListReload(QVariantMap)
{
	QVariantList list = db()->execSelectQuery("SELECT 0 as type, id as cid, CAST(id as TEXT) as uuid, name as cname, "
											  "false as mandatory, '' as mname, "
											  "EXISTS(SELECT * FROM campaignLocks WHERE campaign=campaigns.id) as locked "
											  "FROM campaigns "
											  "UNION "
											  "SELECT 1 as type, campaign as cid, uuid, campaigns.name as cname, mandatory, "
											  "missions.name as mname, "
											  "EXISTS(SELECT * FROM missionLocks WHERE mission=missions.uuid) as locked "
											  "FROM missions "
											  "LEFT JOIN campaigns ON (campaigns.id=missions.campaign)");
	emit campaignListReloaded(list);
}


/**
 * @brief MapEditor::campaignLoad
 * @param data
 */

void MapEditor::campaignLoad(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	if (id == -1) {
		emit campaignLoaded(QVariantMap());
		return;
	}

	QVariantList l;
	l.append(id);

	QVariantMap map = db()->execSelectQueryOneRow("SELECT id, name from campaigns WHERE id=?", l);

	if (map.isEmpty()) {
		emit campaignLoaded(QVariantMap());
		return;
	}

	QVariantList locks = db()->execSelectQuery("SELECT lock, name from campaignLocks "
											   "LEFT JOIN campaigns ON (campaigns.id=campaignLocks.lock) "
											   "WHERE campaign=?", l);

	map["locks"] = locks;

	emit campaignLoaded(map);
}


/**
 * @brief MapEditor::campaignLockAdd
 * @param data
 */

void MapEditor::campaignLockAdd(QVariantMap data)
{
	int id = data.value("id", -1).toInt();

	if (id == -1)
		return;

	QVariantMap bind;
	bind["campaign"] = id;

	QVariantList queryList;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		foreach (QVariant v, list) {
			QVariantMap m = bind;
			m["lock"] = v.toInt();
			queryList.append(m);
		}
	} else if (data.contains("lock")) {
		bind["lock"] = data.value("lock").toInt();
		queryList.append(bind);

	}

	db()->undoLogBegin(tr("%1 hadjárat zárolás hozzáadása").arg(queryList.count()));

	foreach (QVariant v, queryList)
		db()->execInsertQuery("INSERT INTO campaignLocks (?k?) VALUES (?)", v.toMap());

	db()->undoLogEnd();


	campaignListReload();
	setModified(true);

	emit campaignModified(id);
}





/**
 * @brief MapEditor::campaignLockRemove
 * @param data
 */

void MapEditor::campaignLockRemove(QVariantMap data)
{
	int id = data.value("id", -1).toInt();

	if (id == -1)
		return;

	bool ret = false;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		QVariantMap bind;
		bind[":id"] = id;

		db()->undoLogBegin(tr("%1 hadjárat zárolás törlése").arg(list.count()));
		ret = db()->execListQuery("DELETE FROM campaignLocks WHERE campaign=:id AND lock IN (?l?)", list, bind);

		db()->undoLogEnd();
	} else if (data.contains("lock")) {
		db()->undoLogBegin(tr("Hadjárat zárolás törlése"));

		QVariantList l;
		l << id;
		l << data.value("lock", -1).toInt();

		ret = db()->execSimpleQuery("DELETE FROM campaignLocks WHERE campaign=? AND lock=?", l);

		db()->undoLogEnd();
	}


	if (ret) {
		campaignListReload();
		setModified(true);

		emit campaignModified(id);
	}

}


/**
 * @brief MapEditor::campaignLockGetList
 * @param data
 */

void MapEditor::campaignLockGetList(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	if (id == -1) {
		return;
	}

	QVariantList l;
	l.append(id);
	l.append(id);

	QVariantList locks = db()->execSelectQuery("SELECT id, name from campaigns "
											   "LEFT JOIN campaignLocks ON (campaignLocks.lock=campaigns.id AND campaignLocks.campaign=?) "
											   "WHERE lock IS NULL AND id<>?", l);

	emit campaignLockListLoaded(id, locks);
}





/**
 * @brief MapEditor::missionAdd
 * @param data
 */

void MapEditor::missionAdd(QVariantMap data)
{
	if (!data.contains("uuid"))	data["uuid"] = QUuid::createUuid().toString();
	if (!data.contains("mandatory"))	data["mandatory"] = false;

	db()->undoLogBegin(tr("Új küldetés hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO missions(?k?) values (?)", data);

	db()->undoLogEnd();

	campaignListReload();

	if (ret != -1) {
		setModified(true);
		emit missionAdded(ret);
	}
}


/**
 * @brief MapEditor::missionModify
 * @param data
 */

void MapEditor::missionModify(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	QVariantMap d = data.value("data").toMap();

	if (uuid.isEmpty() || d.isEmpty())
		return;

	db()->undoLogBegin(tr("Küldetés módosítása"));

	QVariantMap bind;
	bind[":id"] = uuid;

	bool ret = db()->execUpdateQuery("UPDATE missions SET ? WHERE uuid=:id", d, bind);

	db()->undoLogEnd();

	if (ret) {
		campaignListReload();
		setModified(true);
		emit missionModified(uuid);
	}
}


/**
 * @brief MapEditor::missionRemove
 * @param data
 */

void MapEditor::missionRemove(QVariantMap data)
{
	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		db()->undoLogBegin(tr("%1 küldetés törlése").arg(list.count()));
		bool ret = db()->execListQuery("DELETE FROM missions WHERE uuid IN (?l?)", list);

		db()->undoLogEnd();

		if (ret) {
			campaignListReload();
			setModified(true);

			foreach (QVariant v, list)
				emit missionRemoved(v.toString());
		}

		return;
	}


	QString uuid = data.value("uuid", "").toString();

	if (uuid.isEmpty())
		return;

	db()->undoLogBegin(tr("Küldetés törlése"));

	QVariantList l;
	l << uuid;

	bool ret = db()->execSimpleQuery("DELETE FROM missions WHERE uuid=?", l);

	db()->undoLogEnd();

	if (ret) {
		campaignListReload();
		setModified(true);
		emit missionRemoved(uuid);
	}
}


/**
 * @brief MapEditor::missionLoad
 * @param data
 */

void MapEditor::missionLoad(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	if (uuid.isEmpty()) {
		emit missionLoaded(QVariantMap());
		return;
	}

	QVariantList l;
	l.append(uuid);

	QVariantMap map = db()->execSelectQueryOneRow("SELECT uuid, campaign, mandatory, missions.name as name, campaigns.name as campaignName "
												  "FROM missions "
												  "LEFT JOIN campaigns ON (campaigns.id=missions.campaign) "
												  "WHERE uuid=?", l);

	if (map.isEmpty()) {
		emit missionLoaded(QVariantMap());
		return;
	}

	QVariantList locks = db()->execSelectQuery("SELECT lock, name, level from missionLocks "
											   "LEFT JOIN missions ON (missions.uuid=missionLocks.lock) "
											   "WHERE mission=?", l);

	map["locks"] = locks;

	QVariantList levels = db()->execSelectQuery("SELECT level, terrain, startHP, duration, startBlock, imageFolder, imageFile from missionLevels "
												"WHERE mission=?", l);

	map["levels"] = levels;

	emit missionLoaded(map);
}


/**
 * @brief MapEditor::missionLockAdd
 * @param data
 */

void MapEditor::missionLockAdd(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	if (uuid.isEmpty()) {
		return;
	}


	QVariantMap bind;
	bind["mission"] = uuid;

	QVariantList queryList;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		foreach (QVariant v, list) {
			QVariantMap m = bind;
			m["lock"] = v.toString();
			queryList.append(m);
		}
	} else if (data.contains("lock")) {
		bind["lock"] = data.value("lock").toString();
		queryList.append(bind);
	}

	db()->undoLogBegin(tr("%1 küldetés zárolás hozzáadása").arg(queryList.count()));

	foreach (QVariant v, queryList)
		db()->execInsertQuery("INSERT INTO missionLocks (?k?) VALUES (?)", v.toMap());

	db()->undoLogEnd();


	campaignListReload();
	setModified(true);

	emit missionModified(uuid);
}


/**
 * @brief MapEditor::missionLockRemove
 * @param data
 */

void MapEditor::missionLockRemove(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	if (uuid.isEmpty()) {
		return;
	}
	bool ret = false;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		QVariantMap bind;
		bind[":id"] = uuid;

		db()->undoLogBegin(tr("%1 küldetés zárolás törlése").arg(list.count()));
		ret = db()->execListQuery("DELETE FROM missionLocks WHERE mission=:id AND lock IN (?l?)", list, bind);

		db()->undoLogEnd();
	} else if (data.contains("lock")) {
		db()->undoLogBegin(tr("Küldetés zárolás törlése"));

		QVariantList l;
		l << uuid;
		l << data.value("lock", "").toString();

		ret = db()->execSimpleQuery("DELETE FROM missionLocks WHERE mission=? AND lock=?", l);

		db()->undoLogEnd();
	}


	if (ret) {
		campaignListReload();
		setModified(true);

		emit missionModified(uuid);
	}
}


/**
 * @brief MapEditor::missionLockGetList
 * @param data
 */

void MapEditor::missionLockGetList(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	if (uuid.isEmpty()) {
		return;
	}

	QVariantList l;
	l.append(uuid);
	l.append(uuid);

	QVariantList locks = db()->execSelectQuery("SELECT uuid, name from missions "
											   "LEFT JOIN missionLocks ON (missionLocks.lock=missions.uuid AND missionLocks.mission=?) "
											   "WHERE lock IS NULL AND uuid<>?", l);

	emit missionLockListLoaded(uuid, locks);
}




/**
 * @brief MapEditor::missionLockGetLevelList
 * @param data
 */

void MapEditor::missionLockGetLevelList(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	QString lock = data.value("lock", "").toString();

	if (uuid.isEmpty() || lock.isEmpty()) {
		emit missionLockLevelListLoaded(uuid, lock, QVariantList());
		return;
	}

	int level = data.value("level", -1).toInt();

	QVariantList l;
	l.append(lock);
	l.append(level);

	QVariantList locks = db()->execSelectQuery("SELECT level from missionLevels "
											   "WHERE mission=? AND level<>?", l);

	if (level > 0) {
		QVariantMap m;
		m["level"] = 0;
		locks.prepend(m);
	}

	if (!locks.size()) {
		m_client->sendMessageInfo(tr("Küldetés zárolása"), tr("A zárolást nincs mire módosítani"));
		return;
	}

	emit missionLockLevelListLoaded(uuid, lock, locks);
}



/**
 * @brief MapEditor::missionLockModify
 * @param data
 */

void MapEditor::missionLockModify(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	QString lock = data.value("lock", "").toString();
	int level = data.value("level", -1).toInt();

	qDebug() << "LOCK" << uuid << "TO" << lock << level;

	if (uuid.isEmpty() || lock.isEmpty())
		return;

	db()->undoLogBegin(tr("Küldetés zárolásának módosítása"));

	QVariantList l;
	if (level > 0)
		l.append(level);
	else
		l.append(QVariant::Invalid);

	l.append(uuid);
	l.append(lock);

	bool ret = db()->execSimpleQuery("UPDATE missionLocks SET level=? WHERE mission=? AND lock=?", l);

	db()->undoLogEnd();

	if (ret) {
		setModified(true);
		emit missionModified(uuid);
	}
}



/**
 * @brief MapEditor::missionLevelAdd
 * @param data
 */

void MapEditor::missionLevelAdd(QVariantMap data)
{
	QString uuid = data.value("mission", "").toString();
	QString terrain = data.value("terrain", "").toString();
	if (uuid.isEmpty() || terrain.isEmpty()) {
		return;
	}

	db()->db().transaction();

	QVariantList l;
	l.append(uuid);
	QVariantMap m = db()->execSelectQueryOneRow("SELECT COALESCE(MAX(level),0) AS level FROM missionLevels WHERE mission=?", l);

	data["level"] = m.value("level", 0).toInt()+1;

	db()->undoLogBegin(tr("Új szint hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO missionLevels(?k?) values (?)", data);

	db()->undoLogEnd();

	if (ret != -1) {
		db()->db().commit();
		setModified(true);
		emit missionModified(uuid);
	} else {
		db()->db().rollback();
		m_client->sendMessageError(tr("Adatbázis hiba"), tr("Nem sikerült új szintet hozzáadni!"));
	}
}



/**
 * @brief MapEditor::missionLevelRemove
 * @param data
 */

void MapEditor::missionLevelRemove(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	if (uuid.isEmpty()) {
		return;
	}
	bool ret = false;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		QVariantMap bind;
		bind[":id"] = uuid;

		db()->undoLogBegin(tr("%1 szint törlése").arg(list.count()));
		ret = db()->execListQuery("DELETE FROM missionLevels WHERE mission=:id AND level IN (?l?)", list, bind);

		db()->undoLogEnd();
	} else if (data.contains("level")) {
		db()->undoLogBegin(tr("Szint törlése"));

		QVariantList l;
		l << uuid;
		l << data.value("level", -1).toInt();

		ret = db()->execSimpleQuery("DELETE FROM missionLevels WHERE mission=? AND level=?", l);

		db()->undoLogEnd();
	}


	if (ret) {
		setModified(true);
		emit missionModified(uuid);
	}
}


/**
 * @brief MapEditor::missionLevelNormalize
 * @param uuid
 */

void MapEditor::missionLevelNormalize(QVariantMap)
{
	qInfo() << tr("Küldetés szintek újraszámozása...");

	db()->clearUndo();

	QVariantList uuids = db()->execSelectQuery("SELECT uuid FROM missions");

	foreach (QVariant v, uuids) {
		QString uuid = v.toMap().value("uuid").toString();

		QVariantList l;
		l.append(uuid);
		QVariantList list = db()->execSelectQuery("SELECT rowid, level FROM missionLevels WHERE mission=? ORDER BY level", l);

		for (int i=0; i<list.size(); ++i) {
			QVariantMap m = list.at(i).toMap();
			int level = m.value("level").toInt();
			int norm = i+1;

			if (level != norm) {
				QVariantList l;
				l.append(norm);
				l.append(uuid);
				l.append(m.value("rowid").toInt());

				db()->execSimpleQuery("UPDATE missionLevels SET level=? WHERE mission=? AND rowid=?", l);
			}
		}
	}
}


