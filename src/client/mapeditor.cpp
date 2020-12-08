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
	m_map["missionAdd"] = &MapEditor::missionAdd;
	m_map["missionRemove"] = &MapEditor::missionRemove;
	m_map["missionModify"] = &MapEditor::missionModify;
	m_map["missionLoad"] = &MapEditor::missionLoad;



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
	QString name = data.value("name").toString();
	QByteArray uuid = data.value("uuid").toByteArray();

	db()->open();

	if (uuid.isEmpty())
		uuid = QUuid::createUuid().toByteArray();

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

	m_game = GameMap::example(name, uuid);

	if (_createDatabase())
		emit loadFinished();
	else
		emit loadFailed();
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

}


