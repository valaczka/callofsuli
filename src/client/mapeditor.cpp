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
	, m_campaignData()
	, m_modified(false)
{

	QStringList campaignRoles;
	campaignRoles << "type";
	campaignRoles << "cid";
	campaignRoles << "uuid";
	campaignRoles << "cname";
	campaignRoles << "mname";
	campaignRoles << "mandatory";
	campaignRoles << "locked";

	m_campaignModel = new VariantMapModel(&m_campaignData, campaignRoles, this);



	m_map["loadFromFile"] = &MapEditor::loadFromFile;
	m_map["createNew"] = &MapEditor::createNew;
	m_map["saveToFile"] = &MapEditor::saveToFile;

	m_map["campaignAdd"] = &MapEditor::campaignAdd;
	m_map["campaignModify"] = &MapEditor::campaignModify;
	m_map["campaignListReload"] = &MapEditor::campaignListReload;
	m_map["missionAdd"] = &MapEditor::missionAdd;



	CosDb *db = new CosDb("editorDb", this);
	db->setDatabaseName(Client::standardPath("tmpmapeditor.db"));
	addDb(db);

	connect(db, &CosDb::undone, this, [=]() {
		setModified(true);
		run("campaignListReload");
	});


	connect(this, &MapEditor::campaignListReloaded, this, &MapEditor::setCampaignList);
}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	if (m_campaignModel)
		delete m_campaignModel;

	if (m_game)
		delete m_game;
}


/**
 * @brief MapEditor::loadFromFile
 * @param filename
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
 * @brief MapEditor::setCampaignModel
 * @param campaignModel
 */

void MapEditor::setCampaignModel(VariantMapModel *campaignModel)
{
	if (m_campaignModel == campaignModel)
		return;

	m_campaignModel = campaignModel;
	emit campaignModelChanged(m_campaignModel);
}

void MapEditor::setCampaignModelKey(int campaignModelKey)
{
	if (m_campaignModelKey == campaignModelKey)
		return;

	m_campaignModelKey = campaignModelKey;
	emit campaignModelKeyChanged(m_campaignModelKey);
}

void MapEditor::setModified(bool modified)
{
	if (m_modified == modified)
		return;

	m_modified = modified;
	emit modifiedChanged(m_modified);
}




/**
 * @brief MapEditor::onMessageReceived
 * @param message
 */

void MapEditor::onMessageReceived(const CosMessage &message)
{

}



/**
 * @brief MapEditor::setCampaignList
 * @param list
 */

void MapEditor::setCampaignList(const QVariantList &list)
{
	m_campaignData.fromMapList(list, "uuid");
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


	/*QSqlDriver *driver = db()->db().driver();

	connect(driver, QOverload<const QString &, QSqlDriver::NotificationSource, const QVariant &>::of(&QSqlDriver::notification),
			[=](const QString &name, QSqlDriver::NotificationSource, const QVariant &){

		if (tableList.contains(name)) {
			QMetaObject::invokeMethod(this, QString("table"+name.left(1).toUpper()+name.mid(1)+"Changed").toLatin1().constData(), Qt::DirectConnection);
		}
	});*/


	foreach (QString k, tableList) {
		//driver->subscribeToNotification(k);
		db()->createTrigger(k);
		setLoadProgress(++step/maxStep);
	}

	return true;
}



/**
 * @brief MapEditor::_loadFromNew
 */

void MapEditor::createNew(QVariantMap data)
{
	QString name = data.value("name").toString();
	QByteArray uuid = data.value("uuid").toByteArray();

	db()->open();

	if (uuid.isEmpty())
		uuid = QUuid::createUuid().toByteArray();

	m_game = GameMap::example(name, uuid);

	emit loadStarted();

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


