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
{
	m_db = new ActivityDB("editorDB", "", this);


	QStringList campaignRoles;
	campaignRoles << "type";
	campaignRoles << "cid";
	campaignRoles << "uuid";
	campaignRoles << "name";
	campaignRoles << "mandatory";
	campaignRoles << "ordNumC";
	campaignRoles << "ordNumM";

	m_campaignModel = new VariantMapModel(&m_campaignData, campaignRoles, this);

	connect(this, &MapEditor::tableCampaignsChanged, this, &MapEditor::campaignListReload);
	connect(this, &MapEditor::tableMissionsChanged, this, &MapEditor::campaignListReload);
}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
	if (m_campaignModel)
		delete m_campaignModel;

	if (m_db)
		delete m_db;

	if (m_game)
		delete m_game;
}


/**
 * @brief MapEditor::loadFromFile
 * @param filename
 */

void MapEditor::loadFromFile(const QString &filename)
{
	if (!QFile::exists(filename)) {
		m_client->sendMessageWarning(tr("A fájl nem található"), filename);
		emit loadFailed();
		return;
	}

	if (!m_db->isOpen()) {
		if (!m_db->open()) {
			m_client->sendMessageError(tr("Belső hiba"), tr("Nem lehet előkészíteni az adatbázist!"));
			emit loadFailed();
			return;
		}
	}

	QFutureWatcher<void> www;
	QFuture<void> future = QtConcurrent::run(this, &MapEditor::_loadFromFile, filename);
	www.setFuture(future);
}




/**
 * @brief MapEditor::createNew
 */

void MapEditor::createNew(const QString &name, const QString &uuid)
{
	m_db->open();

	QByteArray u;

	if (uuid.isEmpty()) {
		u = QUuid::createUuid().toByteArray();
	} else {
		u = uuid.toUtf8();
	}

	m_game = GameMap::example(name, u);

	QFutureWatcher<void> watcher;
	//connect(&watcher, &QFutureWatcher<void>::finished, this, &MapEditor::loadFinished);
	QFuture<void> future = QtConcurrent::run(this, &MapEditor::_loadFromNew);
	watcher.setFuture(future);
}



/**
 * @brief MapEditor::loadFromBackup
 */

void MapEditor::loadFromBackup()
{
	/*if (!m_db->databaseExists()) {
		m_client->sendMessageError(tr("Backup hiba"), tr("A backup nem létezik"));
		return;
	}

	if (!m_db->isOpen()) {
		if (!m_db->open())
			return;
	}

	_prepare(); */
}





/**
 * @brief MapEditor::checkBackup
 */

void MapEditor::checkBackup()
{
	/*if (m_db->databaseExists()) {
		emit backupReady("BACKUP", m_db->databaseName());
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
	if (QFile::exists(m_db->databaseName())) {
		if (!QFile::remove(m_db->databaseName())) {
			m_client->sendMessageError(tr("Backup törlése"), tr("Nem sikerült törölni a fájlt"), m_db->databaseName());
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
	QString dbName = m_db->databaseName();

	if (!m_db->isOpen())
		m_db->close();

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
 * @brief MapEditor::campaignListReload
 */

void MapEditor::campaignListReload()
{
	if (m_campaignData.size()) {
		m_campaignData.clear();
	}

	QVariantList list = m_db->execSelectQuery("SELECT 0 as type, id as cid, CAST(id as TEXT) as uuid, name, false as mandatory, "
											  "ordNum as ordNumC, 0 as ordNumM FROM campaigns "
											  "UNION "
											  "SELECT 1 as type, campaign as cid, uuid, missions.name, mandatory, "
											  "campaigns.ordNum as ordNumC, missions.ordNum as ordNumM FROM missions "
											  "LEFT JOIN campaigns ON (campaigns.id=missions.campaign)");
	m_campaignData.fromMapList(list, "uuid");
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


/**
 * @brief MapEditor::clientSetup
 */

void MapEditor::clientSetup()
{
	m_db->setDatabaseName(m_client->standardPath("tmpmapeditor.db"));
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

	if (!m_game->toDb(m_db)) {
		m_client->sendMessageError(tr("Adatfájl hiba"), m_db->databaseName());
		m_db->close();
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

	if (!m_db->createUndoTables())	return false;

	setLoadProgress(++step/maxStep);


	QSqlDriver *driver = m_db->db().driver();

	connect(driver, QOverload<const QString &, QSqlDriver::NotificationSource, const QVariant &>::of(&QSqlDriver::notification),
			[=](const QString &name, QSqlDriver::NotificationSource, const QVariant &){

		if (tableList.contains(name)) {
			QMetaObject::invokeMethod(this, QString("table"+name.left(1).toUpper()+name.mid(1)+"Changed").toLatin1().constData(), Qt::DirectConnection);
		}
	});


	foreach (QString k, tableList) {
		driver->subscribeToNotification(k);
		m_db->createTrigger(k);
		setLoadProgress(++step/maxStep);
	}

	return true;
}


/**
 * @brief MapEditor::_loadFromFile
 */

void MapEditor::_loadFromFile(QString filename)
{
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
		m_db->close();
		emit loadFailed();
		return;
	}

	if (_createDatabase())
		emit loadFinished();
	else
		emit loadFailed();
}


/**
 * @brief MapEditor::_loadFromNew
 */

void MapEditor::_loadFromNew()
{
	emit loadStarted();

	if (_createDatabase())
		emit loadFinished();
	else
		emit loadFailed();
}

