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
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_loadProgress(0.0)
	, m_loadProgressFraction(qMakePair<qreal, qreal>(0.0, 1.0))
	, m_loadAbortRequest(false)
	, m_modified(false)
	, m_fileName()
	, m_database(nullptr)
	, m_databaseUuid()
	, m_databaseTable()
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
	m_map["chapterAdd"] = &MapEditor::chapterAdd;
	m_map["chapterModify"] = &MapEditor::chapterModify;
	m_map["chapterRemove"] = &MapEditor::chapterRemove;
	m_map["chapterListReload"] = &MapEditor::chapterListReload;
	m_map["objectiveAdd"] = &MapEditor::objectiveAdd;
	m_map["objectiveRemove"] = &MapEditor::objectiveRemove;
	m_map["objectiveModify"] = &MapEditor::objectiveModify;
	m_map["objectiveLoad"] = &MapEditor::objectiveLoad;
	m_map["levelLoad"] = &MapEditor::levelLoad;
	m_map["levelModify"] = &MapEditor::levelModify;
	m_map["blockChapterMapAdd"] = &MapEditor::blockChapterMapAdd;
	m_map["blockChapterMapRemove"] = &MapEditor::blockChapterMapRemove;
	m_map["blockChapterMapLoad"] = &MapEditor::blockChapterMapLoad;
	m_map["blockChapterMapBlockGetList"] = &MapEditor::blockChapterMapBlockGetList;
	m_map["blockChapterMapBlockAdd"] = &MapEditor::blockChapterMapBlockAdd;
	m_map["blockChapterMapBlockRemove"] = &MapEditor::blockChapterMapBlockRemove;
	m_map["blockChapterMapChapterGetList"] = &MapEditor::blockChapterMapChapterGetList;
	m_map["blockChapterMapChapterAdd"] = &MapEditor::blockChapterMapChapterAdd;
	m_map["blockChapterMapChapterRemove"] = &MapEditor::blockChapterMapChapterRemove;


	CosDb *db = new CosDb("editorDb", this);
	db->setDatabaseName(Client::standardPath("tmpmapeditor.db"));
	addDb(db, true);

	connect(db, &CosDb::undone, this, [=]() {
		setModified(true);
		run("campaignListReload");
		run("chapterListReload");
	});

}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{

}


/**
 * @brief MapEditor::loadDefault
 * @return
 */

bool MapEditor::loadDefault()
{
	if (!m_fileName.isEmpty()) {
		qDebug() << "Load from file" << m_fileName;
		loadFromFile(m_fileName);
		return true;
	}

	if (m_database && !m_databaseUuid.isEmpty() && !m_databaseTable.isEmpty()) {
		if (!db()->isOpen()) {
			if (!db()->open()) {
				m_client->sendMessageError(tr("Belső hiba"), tr("Nem lehet előkészíteni az adatbázist!"));
				emit loadFailed();
				return false;
			}
		}

		qDebug() << "load from database" << m_database << m_databaseUuid;
		AbstractActivity::run(&MapEditor::loadFromDbPrivate, QVariantMap());
		return true;
	}

	return false;
}


/**
 * @brief MapEditor::saveDefault
 * @return
 */

bool MapEditor::saveDefault()
{
	if (!m_fileName.isEmpty()) {
		qDebug() << "Save to file" << m_fileName;
		saveToFile(m_fileName);
		return true;
	}

	if (m_database && !m_databaseUuid.isEmpty() && !m_databaseTable.isEmpty()) {
		qDebug() << "save to database" << m_database << m_databaseUuid;
		AbstractActivity::run(&MapEditor::saveToDbPrivate, QVariantMap());
		return true;
	}

	return false;
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

	GameMap *game = GameMap::fromBinaryData(d, this, "setLoadProgress");

	if (!game) {
		m_client->sendMessageError(tr("Hibás fájl"), filename);
		db()->close();
		emit loadFailed();
		return;
	}

	QStringList unavailableTerrains = checkTerrains(game);
	if (!unavailableTerrains.isEmpty()) {
		m_client->sendMessageWarning(tr("Érvénytelen harcmező"),
									 tr("Érvénytelen harcmezőválasztás a következő küldetésekben: ")
									 .append(unavailableTerrains.join(", ")));
	}

	if (_createDatabase(game)) {
		emit loadFinished();
		QFileInfo f(filename);
		setMapName(f.baseName()+" ["+f.path()+"]");
	} else
		emit loadFailed();

	delete game;
}




/**
 * @brief MapEditor::loadFromDbPrivate
 */

void MapEditor::loadFromDbPrivate(QVariantMap)
{
	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.2));
	setLoadProgress(0.0);

	QVariantList l;
	l.append(m_databaseUuid);
	QVariantMap m = m_database->execSelectQueryOneRow("SELECT name, data FROM "+m_databaseTable+" WHERE uuid=?", l);

	if (m.isEmpty()) {
		m_client->sendMessageError(tr("Hibás adatbázis"), m_database->databaseName());
		db()->close();
		emit loadFailed();
		return;
	}

	QByteArray d = m.value("data").toByteArray();

	GameMap *game = GameMap::fromBinaryData(d, this, "setLoadProgress");

	if (!game) {
		m_client->sendMessageError(tr("Hibás adatbázis"), m_database->databaseName());
		db()->close();
		emit loadFailed();
		return;
	}

	QStringList unavailableTerrains = checkTerrains(game);
	if (!unavailableTerrains.isEmpty()) {
		m_client->sendMessageWarning(tr("Érvénytelen harcmező"),
									 tr("Érvénytelen harcmezőválasztás a következő küldetésekben: ")
									 .append(unavailableTerrains.join(", ")));
	}

#ifdef QT_DEBUG
	qDebug() << "*** WRITE TO /tmp/ttt.dat";
	QFile f("/tmp/ttt.dat");
	f.open(QIODevice::WriteOnly);
	f.write(game->toBinaryData());
	f.close();
#endif

	if (_createDatabase(game)) {
		emit loadFinished();
		setMapName(m.value("name").toString());
	} else
		emit loadFailed();

	delete game;
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
	GameMap *game = GameMap::fromDb(db(), this, "setLoadProgress");

	if (!game) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Adatbázis hiba"));
		emit saveFailed();
		return;
	}



	if (!checkGame(game)) {
		emit saveFailed();
		delete game;
		return;
	}



	QFile f(filename);
	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Írási hiba"), filename);
		emit saveFailed();
		return;
	}

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.9, 1.0));
	QByteArray d = game->toBinaryData();

	f.write(d);
	f.close();

	emit saveFinished();

	delete game;
}


/**
 * @brief MapEditor::saveToDbPrivate
 */

void MapEditor::saveToDbPrivate(QVariantMap)
{
	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 0.9));
	setLoadProgress(0.0);

	m_loadAbortRequest = false;
	GameMap *game = GameMap::fromDb(db(), this, "setLoadProgress");

	if (!game) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Adatbázis hiba"));
		emit saveFailed();
		return;
	}


	if (!checkGame(game)) {
		emit saveFailed();
		delete game;
		return;
	}


	setLoadProgressFraction(qMakePair<qreal, qreal>(0.9, 1.0));
	QByteArray d = game->toBinaryData();

	QVariantList l;
	l.append(d);
	l.append(QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex()));
	l.append(m_databaseUuid);
	QVariantMap m = m_database->execSelectQueryOneRow("UPDATE "+m_databaseTable+" SET data=?, md5=?, lastModified=datetime('now') WHERE uuid=?", l);

	emit saveFinished();

	delete game;
}







void MapEditor::setFileName(QString fileName)
{
	if (m_fileName == fileName)
		return;

	m_fileName = fileName;
	emit fileNameChanged(m_fileName);
}

void MapEditor::setDatabase(CosDb *database)
{
	if (m_database == database)
		return;

	m_database = database;
	emit databaseChanged(m_database);
}

void MapEditor::setDatabaseUuid(QString databaseUuid)
{
	if (m_databaseUuid == databaseUuid)
		return;

	m_databaseUuid = databaseUuid;
	emit databaseUuidChanged(m_databaseUuid);
}

void MapEditor::setDatabaseTable(QString databaseTable)
{
	if (m_databaseTable == databaseTable)
		return;

	m_databaseTable = databaseTable;
	emit databaseTableChanged(m_databaseTable);
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
	/*if (QFile::exists(db()->databaseName())) {
		if (!QFile::remove(db()->databaseName())) {
			m_client->sendMessageError(tr("Backup törlése"), tr("Nem sikerült törölni a fájlt"), db()->databaseName());
			return;
		}
	}*/

	emit backupUnavailable();
}



/**
 * @brief MapEditor::loadAbort
 */

void MapEditor::loadAbort()
{
	m_loadAbortRequest = true;
}


/**
 * @brief MapEditor::removeImageProvider
 */

void MapEditor::removeImageProvider()
{
	qDebug() << "Remove image provider";
	QQmlEngine *engine = qmlEngine(this);
	if (engine)
		engine->removeImageProvider("mapdb");
}


/**
 * @brief MapEditor::moduleData
 * @param module
 * @param isObjective
 * @return
 */

QVariantMap MapEditor::moduleData(const QString &module, const bool &isObjective) const
{
	QVariantMap m = isObjective ? Client::objectiveModuleMap() : Client::storageModuleMap();

	if (!m.contains(module))
		return QVariantMap({
							   { "name", tr("Érvénytelen modul!") },
							   { "icon", "image://font/Material Icons/\ue002" }
						   });

	return m.value(module).toMap();
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

void MapEditor::loadFromFile(const QString &filename)
{
	if (filename.isEmpty() || !QFile::exists(filename)) {
		m_client->sendMessageWarning(tr("A fájl nem található"), filename);
		emit loadFailed();
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

	QVariantMap data;
	data["filename"] = filename;

	AbstractActivity::run(&MapEditor::loadFromFilePrivate, data);
}



/**
 * @brief MapEditor::saveToFile
 * @param data
 */

void MapEditor::saveToFile(const QString &filename)
{
	if (filename.isEmpty())
		return;

	if (!db()->isOpen()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Nincs megnyitva az adatbázis!"));
		emit saveFailed();
		return;
	}

	emit saveStarted();

	QVariantMap data;
	data["filename"] = filename;

	AbstractActivity::run(&MapEditor::saveToFilePrivate, data);
}






/**
 * @brief MapEditor::databasePrepares
 * @return
 */

bool MapEditor::_createDatabase(GameMap *game)
{
	setLoadProgressFraction(qMakePair<qreal, qreal>(0.2, 0.6));
	game->setProgressFunc(this, "setLoadProgress");

	if (!game->toDb(db())) {
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
	tableList << "objectives";
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
 * @brief MapEditor::_blockChapterMapBlockGetListPrivate
 * @param data
 * @return
 */

QList<int> MapEditor::_blockChapterMapBlockGetListPrivate(const QString &mission, const int &level, const QString &terrain)
{
	TerrainData t = Client::terrain(terrain);

	if (t.name.isEmpty()) {
		return QList<int>();
	}

	QList<int> freeBlocks = t.blocks.keys();


	QVariantList ll;
	ll.append(mission);
	ll.append(level);

	QVariantList usedBlocks = db()->execSelectQuery("SELECT DISTINCT block "
													"FROM blockChapterMaps "
													"LEFT JOIN blockChapterMapBlocks ON (blockChapterMapBlocks.blockid=blockChapterMaps.id) "
													"WHERE mission=? AND level=?", ll);

	foreach (QVariant v, usedBlocks) {
		int block = v.toMap().value("block").toInt();

		freeBlocks.removeAll(block);
	}

	return freeBlocks;
}




/**
 * @brief MapEditor::checkTerrains
 */

QStringList MapEditor::checkTerrains(GameMap *game) const
{
	if (!m_client || !game)
		return QStringList();

	QVariantMap terrainMap = m_client->terrainMap();
	QStringList list;

	foreach (GameMap::Campaign *c, game->campaigns()) {
		foreach (GameMap::Mission *m, c->missions()) {
			foreach (GameMap::MissionLevel *l, m->levels()) {
				if (!terrainMap.contains(l->terrain()))
					list.append(m->name());
			}
		}
	}

	return list;
}



/**
 * @brief MapEditor::checkGame
 * @param game
 * @return
 */

bool MapEditor::checkGame(GameMap *game) const
{
	if (!game)
		return false;

	QStringList unavailableTerrains = checkTerrains(game);
	if (!unavailableTerrains.isEmpty()) {
		m_client->sendMessageWarning(tr("Érvénytelen harcmező"),
									 tr("Érvénytelen harcmezőválasztás a következő küldetésekben: ")
									 .append(unavailableTerrains.join(", ")));
		return false;
	}


	GameMap::Mission *mission = nullptr;
	game->missionLockTree(&mission);

	if (mission) {
		m_client->sendMessageWarning(tr("Körkörös zárolás"), tr("A %1 küldetés körkörösen van zárolva, javítás szükséges!").arg(mission->name()));
		return false;
	}

	GameMap::Campaign *campaign = nullptr;
	game->campaignLockTree(&campaign);

	if (campaign) {
		m_client->sendMessageWarning(tr("Körkörös zárolás"), tr("A %1 hadjárat körkörösen van zárolva, javítás szükséges!").arg(campaign->name()));
		return false;
	}

	return true;
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

	QVariantList levels = db()->execSelectQuery("SELECT rowid, level, terrain, startHP, duration, startBlock, imageFolder, imageFile from missionLevels "
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

	db()->transaction();

	QVariantList l;
	l.append(uuid);
	QVariantMap m = db()->execSelectQueryOneRow("SELECT COALESCE(MAX(level),0) AS level FROM missionLevels WHERE mission=?", l);

	int newLevel = m.value("level", 0).toInt()+1;

	data["level"] = newLevel;

	db()->undoLogBegin(tr("Új szint hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO missionLevels(?k?) values (?)", data);

	QVariantMap m2;
	m2["mission"] = uuid;
	m2["level"] = newLevel;

	int ret2 = db()->execInsertQuery("INSERT INTO blockChapterMaps(?k?) values (?)", m2);

	db()->undoLogEnd();

	if (ret != -1 && ret2 != -1) {
		db()->commit();
		setModified(true);
		emit missionModified(uuid);
	} else {
		db()->rollback();
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

	db()->undoLogBegin(tr("Szint törlése"));

	QVariantList l;
	l << uuid;

	qDebug() << l;

	ret = db()->execSimpleQuery("DELETE FROM missionLevels WHERE rowid = (SELECT rowid FROM missionLevels WHERE mission=? ORDER BY level DESC LIMIT 1)", l);

	db()->undoLogEnd();


	if (ret) {
		setModified(true);
		emit missionModified(uuid);
	}
}


/**
 * @brief MapEditor::chapterAdd
 * @param data
 */

void MapEditor::chapterAdd(QVariantMap data)
{
	db()->undoLogBegin(tr("Új szakasz hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO chapters(?k?) values (?)", data);

	db()->undoLogEnd();

	chapterListReload();

	if (ret != -1) {
		setModified(true);
		emit chapterAdded(ret);
	}
}


/**
 * @brief MapEditor::chapterModify
 * @param data
 */


void MapEditor::chapterModify(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	QVariantMap d = data.value("data").toMap();

	if (id == -1 || d.isEmpty())
		return;

	db()->undoLogBegin(tr("Szakasz módosítása"));

	QVariantMap bind;
	bind[":id"] = id;

	bool ret = db()->execUpdateQuery("UPDATE chapters SET ? WHERE id=:id", d, bind);

	db()->undoLogEnd();

	if (ret) {
		chapterListReload();
		setModified(true);
		emit chapterModified(id);
	}
}





/**
 * @brief MapEditor::chapterRemove
 * @param data
 */

void MapEditor::chapterRemove(QVariantMap data)
{
	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		db()->undoLogBegin(tr("%1 szakasz törlése").arg(list.count()));
		bool ret = db()->execListQuery("DELETE FROM chapters WHERE id IN (?l?)", list);

		db()->undoLogEnd();

		if (ret) {
			chapterListReload();
			setModified(true);

			foreach (QVariant v, list)
				emit chapterRemoved(v.toInt());
		}

		return;
	}

	int id = data.value("id", -1).toInt();

	if (id == -1)
		return;

	db()->undoLogBegin(tr("Szakasz törlése"));

	QVariantList l;
	l << id;

	bool ret = db()->execSimpleQuery("DELETE FROM chapters WHERE id=?", l);

	db()->undoLogEnd();

	if (ret) {
		chapterListReload();
		setModified(true);
		emit chapterRemoved(id);
	}
}






/**
 * @brief MapEditor::chapterListReload
 */

void MapEditor::chapterListReload(QVariantMap)
{
	QVariantList list = db()->execSelectQuery("SELECT 0 as type, id, CAST(id as TEXT) as uuid, name, "
											  "NULL as storage, NULL as module, NULL as data "
											  "FROM chapters "
											  "UNION "
											  "SELECT 1 as type, chapter as id, uuid, name, "
											  "storage, module, data "
											  "FROM objectives "
											  "LEFT JOIN chapters ON (chapters.id=objectives.chapter)"
											  );
	emit chapterListReloaded(list);
}



/**
 * @brief MapEditor::objectiveAdd
 * @param data
 */

void MapEditor::objectiveAdd(QVariantMap data)
{
	if (!data.contains("uuid"))	data["uuid"] = QUuid::createUuid().toString();

	db()->undoLogBegin(tr("Új célpont hozzáadása"));

	int ret = db()->execInsertQuery("INSERT INTO objectives(?k?) values (?)", data);

	db()->undoLogEnd();

	chapterListReload();

	if (ret != -1) {
		setModified(true);
		emit objectiveAdded(ret);
	}
}


/**
 * @brief MapEditor::objectiveModify
 * @param data
 */

void MapEditor::objectiveModify(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	QVariantMap d = data.value("data").toMap();

	if (uuid.isEmpty() || d.isEmpty())
		return;

	db()->undoLogBegin(tr("Célpont módosítása"));

	QVariantMap bind;
	bind[":id"] = uuid;

	bool ret = db()->execUpdateQuery("UPDATE objectives SET ? WHERE uuid=:id", d, bind);

	db()->undoLogEnd();

	if (ret) {
		chapterListReload();
		setModified(true);
		emit objectiveModified(uuid);
	}
}


/**
 * @brief MapEditor::objectiveRemove
 * @param data
 */

void MapEditor::objectiveRemove(QVariantMap data)
{
	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		db()->undoLogBegin(tr("%1 célpont törlése").arg(list.count()));
		bool ret = db()->execListQuery("DELETE FROM objectives WHERE uuid IN (?l?)", list);

		db()->undoLogEnd();

		if (ret) {
			chapterListReload();
			setModified(true);

			foreach (QVariant v, list)
				emit objectiveRemoved(v.toString());
		}

		return;
	}


	QString uuid = data.value("uuid", "").toString();

	if (uuid.isEmpty())
		return;

	db()->undoLogBegin(tr("Célpont törlése"));

	QVariantList l;
	l << uuid;

	bool ret = db()->execSimpleQuery("DELETE FROM objectives WHERE uuid=?", l);

	db()->undoLogEnd();

	if (ret) {
		chapterListReload();
		setModified(true);
		emit objectiveRemoved(uuid);
	}
}


/**
 * @brief MapEditor::objectiveLoad
 * @param data
 */

void MapEditor::objectiveLoad(QVariantMap data)
{
	QString uuid = data.value("uuid", "").toString();
	if (uuid.isEmpty()) {
		emit objectiveLoaded(QVariantMap());
		return;
	}

	QVariantList l;
	l.append(uuid);

	QVariantMap map = db()->execSelectQueryOneRow("SELECT uuid, chapter, objectives.module as objectiveModule, storage, objectives.data as objectiveData, "
												  "storages.module as storageModule, storages.data as storageData "
												  "FROM objectives "
												  "LEFT JOIN storages ON (storages.id=objectives.storage) "
												  "WHERE uuid=?", l);

	if (map.isEmpty()) {
		emit objectiveLoaded(QVariantMap());
		return;
	}


	emit objectiveLoaded(map);
}



/**
 * @brief MapEditor::levelLoad
 * @param data
 */

void MapEditor::levelLoad(QVariantMap data)
{
	int id = data.value("rowid", -1).toInt();
	if (id <= 0) {
		emit levelLoaded(QVariantMap());
		return;
	}

	QVariantList l;
	l.append(id);

	QVariantMap map = db()->execSelectQueryOneRow("SELECT missionLevels.rowid as rowid, mission, missions.name as name, level, terrain, startHP, "
												  "duration, startBlock, imageFolder, imageFile "
												  "FROM missionLevels "
												  "LEFT JOIN missions ON (missions.uuid=missionLevels.mission) "
												  "WHERE missionLevels.rowid=?", l);

	if (map.isEmpty()) {
		emit levelLoaded(QVariantMap());
		return;
	}

	TerrainData t = Client::terrain(map.value("terrain").toString());

	if (!t.name.isEmpty())
		map["terrainBlocks"] = t.blocks.count();
	else
		map["terrainBlocks"] = 1;



	QVariantList blockData;
	QList<int> freeBlocks = t.blocks.keys();


	QVariantList ll;
	ll.append(map.value("mission").toString());
	ll.append(map.value("level").toInt());

	QVariantList bcmList = db()->execSelectQuery("SELECT id, maxObjective FROM blockChapterMaps WHERE mission=? AND level=?", ll);

	foreach (QVariant v, bcmList) {
		QVariantMap m = v.toMap();
		int bcmId = m.value("id").toInt();
		int maxObj = m.value("maxObjective").toInt();

		QVariantList ll;
		ll.append(bcmId);
		QVariantList blockList = db()->execSelectQuery("SELECT DISTINCT block FROM blockChapterMapBlocks WHERE blockid=? ORDER BY block", ll);

		int enemies = 0;

		QStringList bl;

		if (blockList.count()) {
			foreach (QVariant v, blockList) {
				int block = v.toMap().value("block").toInt();
				bl.append(v.toMap().value("block").toString());

				freeBlocks.removeAll(block);

				enemies += t.blocks.value(block, 0);
			}
		} else {
			enemies = t.enemies;
			freeBlocks.clear();
		}

		if (maxObj > 0 && enemies > maxObj) {
			enemies = maxObj;
		}

		QVariantMap bd;
		bd["id"] = bcmId;
		bd["enemies"] = enemies;
		bd["blocks"] = bl.join(", ");
		blockData.append(bd);
	}

	if (!freeBlocks.isEmpty() || bcmList.isEmpty())
		map["canAdd"] = true;
	else
		map["canAdd"] = false;

	map["blockDataList"] = blockData;




	QVariantList inventory = db()->execSelectQuery("SELECT rowid, block, module, count FROM inventories WHERE mission=? AND level=?", ll);

	map["inventory"] = inventory;


	emit levelLoaded(map);
}



/**
 * @brief MapEditor::levelModify
 * @param data
 */

void MapEditor::levelModify(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	QVariantMap d = data.value("data").toMap();

	if (rowid <= 0 || d.isEmpty())
		return;

	db()->undoLogBegin(tr("Küldetésszint módosítása"));

	QVariantMap bind;
	bind[":id"] = rowid;

	bool ret = db()->execUpdateQuery("UPDATE missionLevels SET ? WHERE rowid=:id", d, bind);

	db()->undoLogEnd();

	if (ret) {
		levelLoad(data);
		setModified(true);
	}
}




/**
 * @brief MapEditor::blockChapterMapLoad
 * @param data
 */

void MapEditor::blockChapterMapLoad(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	if (id <= 0) {
		emit blockChapterMapLoaded(QVariantMap());
		return;
	}

	QVariantList l;
	l.append(id);

	QVariantMap map = db()->execSelectQueryOneRow("SELECT blockChapterMaps.id as id, terrain, maxObjective "
												  "FROM blockChapterMaps "
												  "LEFT JOIN missionLevels ON (missionLevels.mission=blockChapterMaps.mission "
												  "AND missionLevels.level=blockChapterMaps.level) "
												  "WHERE blockChapterMaps.id=?", l);

	if (map.isEmpty()) {
		emit blockChapterMapLoaded(QVariantMap());
		return;
	}

	QVariantList blockList = db()->execSelectQuery("SELECT DISTINCT block FROM blockChapterMapBlocks WHERE blockid=?", l);

	QVariantList chapters = db()->execSelectQuery("SELECT chapter, name FROM blockChapterMapChapters "
												  "LEFT JOIN chapters ON (chapters.id=blockChapterMapChapters.chapter) "
												  "WHERE blockid=?", l);

	QVariantList favorites = db()->execSelectQuery("SELECT objective FROM blockChapterMapFavorites WHERE blockid=?", l);


	map["blocks"] = blockList;
	map["chapters"] = chapters;
	map["favorites"] = favorites;

	emit blockChapterMapLoaded(map);
}


/**
 * @brief MapEditor::blockChapterMapAdd
 * @param data
 */

void MapEditor::blockChapterMapAdd(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	if (rowid <= 0)
		return;

	QVariantList l;
	l.append(rowid);

	QVariantMap level = db()->execSelectQueryOneRow("SELECT mission, level, terrain FROM missionLevels WHERE rowid=?", l);

	if (level.isEmpty())
		return;

	QList<int> freeBlocks = _blockChapterMapBlockGetListPrivate(level.value("mission").toString(),
																level.value("level").toInt(),
																level.value("terrain").toString());


	db()->undoLogBegin(tr("Elosztás hozzáadása"));

	QVariantMap m;
	m["mission"] = level.value("mission", "").toString();
	m["level"] = level.value("level", -1).toInt();

	int ret = db()->execInsertQuery("INSERT INTO blockChapterMaps(?k?) values (?)", m);

	if (ret != -1 && !freeBlocks.isEmpty()) {
		foreach (int block, freeBlocks) {
			QVariantMap m;
			m["blockid"] = ret;
			m["block"] = block;
			db()->execInsertQuery("INSERT INTO blockChapterMapBlocks(?k?) values (?)", m);
		}
	}

	db()->undoLogEnd();

	campaignListReload();

	if (ret != -1) {
		setModified(true);
		levelLoad(data);
	}
}




/**
 * @brief MapEditor::blockChapterMapRemove
 * @param data
 */

void MapEditor::blockChapterMapRemove(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	if (rowid <= 0)
		return;

	QVariantList l;
	l.append(rowid);

	QVariantMap level = db()->execSelectQueryOneRow("SELECT mission, level FROM missionLevels WHERE rowid=?", l);

	if (level.isEmpty())
		return;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		QVariantMap bind;
		bind[":mission"] = level.value("mission", "").toString();
		bind[":level"] = level.value("level", -1).toInt();

		db()->undoLogBegin(tr("%1 elosztás törlése").arg(list.count()));
		bool ret = db()->execListQuery("DELETE FROM blockChapterMaps WHERE id IN (?l?) AND mission=:mission AND level=:level", list, bind);

		db()->undoLogEnd();

		if (ret) {
			levelLoad(data);
			setModified(true);
			foreach (QVariant v, list)
				emit blockChapterMapRemoved(v.toInt());
		}

		return;
	}


	int id = data.value("id", -1).toInt();

	if (id <= 0)
		return;

	db()->undoLogBegin(tr("Elosztás törlése"));

	QVariantList ll;
	ll << id;
	ll << level.value("mission", "").toString();
	ll << level.value("level", -1).toInt();

	bool ret = db()->execSimpleQuery("DELETE FROM blockChapterMaps WHERE id=? AND mission=? AND level=?", ll);

	db()->undoLogEnd();

	if (ret) {
		levelLoad(data);
		setModified(true);
		emit blockChapterMapRemoved(id);
	}
}



/**
 * @brief MapEditor::blockChapterMapBlockGetList
 * @param data
 */

void MapEditor::blockChapterMapBlockGetList(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	if (id == -1) {
		return;
	}

	QVariantList l;
	l.append(id);

	QVariantMap level = db()->execSelectQueryOneRow("SELECT missionLevels.mission as mission, missionLevels.level as level, terrain "
													"FROM blockChapterMaps "
													"LEFT JOIN missionLevels ON (missionLevels.mission=blockChapterMaps.mission AND "
													"missionLevels.level=blockChapterMaps.level) "
													"WHERE blockChapterMaps.id=?", l);

	if (level.isEmpty()) {
		emit blockChapterMapBlockListLoaded(id, QVariantList());
		return;
	}

	QList<int> freeBlocks = _blockChapterMapBlockGetListPrivate(level.value("mission").toString(),
																level.value("level").toInt(),
																level.value("terrain").toString());

	if (freeBlocks.isEmpty()) {
		emit blockChapterMapBlockListLoaded(id, QVariantList());
		return;
	}

	QVariantList ret;

	foreach (int b, freeBlocks) {
		QVariantMap m;
		m["block"] = b;
		m["details"] = tr("%1. csatatér").arg(b);
		ret.append(m);
	}

	emit blockChapterMapBlockListLoaded(id, ret);
}




/**
 * @brief MapEditor::blockChapterMapBlockAdd
 * @param data
 */

void MapEditor::blockChapterMapBlockAdd(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	int id = data.value("id", -1).toInt();

	if (rowid == -1 || id == -1)
		return;

	QVariantMap bind;
	bind["blockid"] = id;

	QVariantList queryList;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		foreach (QVariant v, list) {
			QVariantMap m = bind;
			m["block"] = v.toInt();
			queryList.append(m);
		}
	} else if (data.contains("block")) {
		bind["block"] = data.value("block").toInt();
		queryList.append(bind);

	}

	db()->undoLogBegin(tr("%1 csatatér hozzáadása").arg(queryList.count()));

	foreach (QVariant v, queryList)
		db()->execInsertQuery("INSERT INTO blockChapterMapBlocks (?k?) VALUES (?)", v.toMap());

	db()->undoLogEnd();

	levelLoad(data);
	setModified(true);
	blockChapterMapLoad(data);
}




/**
 * @brief MapEditor::blockChapterMapBlockRemove
 * @param data
 */

void MapEditor::blockChapterMapBlockRemove(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	int id = data.value("id", -1).toInt();

	if (rowid == -1 || id == -1)
		return;


	QVariantList queryList;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		foreach (QVariant v, list) {
			queryList.append(v);
		}
	} else if (data.contains("block")) {
		queryList.append(data.value("block"));

	}

	db()->undoLogBegin(tr("%1 csatatér eltávolítása").arg(queryList.count()));

	foreach (QVariant v, queryList) {
		QVariantList l;
		l.append(id);
		l.append(v.toInt());
		db()->execSimpleQuery("DELETE FROM blockChapterMapBlocks WHERE blockid=? AND block=?", l);
	}

	db()->undoLogEnd();

	levelLoad(data);
	setModified(true);
	blockChapterMapLoad(data);
}



/**
 * @brief MapEditor::blockChapterMapChapterGetList
 * @param data
 */

void MapEditor::blockChapterMapChapterGetList(QVariantMap data)
{
	int id = data.value("id", -1).toInt();
	if (id == -1) {
		return;
	}

	QVariantList l;
	l.append(id);

	QVariantList list = db()->execSelectQuery("SELECT chapters.id as id, chapters.name as name "
											  "FROM chapters "
											  "WHERE chapters.id NOT IN (SELECT chapter FROM blockChapterMapChapters "
											  "where blockid=?)", l);

	emit blockChapterMapChapterListLoaded(id, list);
}




/**
 * @brief MapEditor::blockChapterMapChapterAdd
 * @param data
 */

void MapEditor::blockChapterMapChapterAdd(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	int id = data.value("id", -1).toInt();

	if (rowid == -1 || id == -1)
		return;

	QVariantMap bind;
	bind["blockid"] = id;

	QVariantList queryList;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		foreach (QVariant v, list) {
			QVariantMap m = bind;
			m["chapter"] = v.toInt();
			queryList.append(m);
		}
	} else if (data.contains("chapter")) {
		bind["chapter"] = data.value("chapter").toInt();
		queryList.append(bind);

	}

	db()->undoLogBegin(tr("%1 szakasz hozzáadása").arg(queryList.count()));

	foreach (QVariant v, queryList)
		db()->execInsertQuery("INSERT INTO blockChapterMapChapters (?k?) VALUES (?)", v.toMap());

	db()->undoLogEnd();

	levelLoad(data);
	setModified(true);
	blockChapterMapLoad(data);
}




/**
 * @brief MapEditor::blockChapterMapChapterRemove
 * @param data
 */

void MapEditor::blockChapterMapChapterRemove(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	int id = data.value("id", -1).toInt();

	if (rowid == -1 || id == -1)
		return;


	QVariantList queryList;

	if (data.contains("list")) {
		QVariantList list = data.value("list").toList();

		foreach (QVariant v, list) {
			queryList.append(v);
		}
	} else if (data.contains("chapter")) {
		queryList.append(data.value("chapter"));

	}

	db()->undoLogBegin(tr("%1 szakasz eltávolítása").arg(queryList.count()));

	foreach (QVariant v, queryList) {
		QVariantList l;
		l.append(id);
		l.append(v.toInt());
		db()->execSimpleQuery("DELETE FROM blockChapterMapChapters WHERE blockid=? AND chapter=?", l);
	}

	db()->undoLogEnd();

	levelLoad(data);
	setModified(true);
	blockChapterMapLoad(data);
}



/**
 * @brief MapEditor::play
 * @param data
 */

void MapEditor::play(QVariantMap data)
{
	int rowid = data.value("rowid", -1).toInt();
	if (rowid <= 0)
		return;

	QVariantList l;
	l.append(rowid);

	QVariantMap level = db()->execSelectQueryOneRow("SELECT mission, name, level, terrain, startHP, duration, startBlock,"
													"imageFolder, imageFile FROM missionLevels "
													"LEFT JOIN missions ON (missions.uuid=missionLevels.mission) "
													"WHERE missionLevels.rowid=?", l);
	if (level.isEmpty()) {
		emit playFailed();
		return;
	}

	setLoadProgressFraction(qMakePair<qreal, qreal>(0.0, 1.0));
	setLoadProgress(0.0);

	m_loadAbortRequest = false;
	GameMap *game = GameMap::fromDb(db(), this, "setLoadProgress", false);

	if (!game) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Adatbázis hiba"));
		emit playFailed();
		return;
	}

	if (!checkGame(game)) {
		emit playFailed();
		delete game;
		return;
	}

	GameMatch *m_gameMatch = new GameMatch(game, this);
	m_gameMatch->setDeleteGameMap(true);
	m_gameMatch->setImageDbName("mapdb");
	m_gameMatch->setMissionUuid(level.value("mission").toByteArray());
	m_gameMatch->setName(level.value("name").toString());
	m_gameMatch->setLevel(level.value("level").toInt());
	m_gameMatch->setTerrain(level.value("terrain").toString());
	m_gameMatch->setStartHp(level.value("startHP").toInt());
	m_gameMatch->setDuration(level.value("duration").toInt());
	m_gameMatch->setStartBlock(level.value("startBlock").toInt());

	QString imageFolder = level.value("imageFolder").toString();
	QString imageFile = level.value("imageFile").toString();

	if (!imageFolder.isEmpty() && !imageFile.isEmpty())
		m_gameMatch->setBgImage(imageFolder+"/"+imageFile);

	emit playReady(m_gameMatch);

}



