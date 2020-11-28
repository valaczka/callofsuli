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

MapEditor::MapEditor(QQuickItem *parent)
	: AbstractActivity(parent)
	, m_game(nullptr)
	, m_loadProgress(0.0)
{
	m_db = new ActivityDB("editorDB", "", this);
}


/**
 * @brief MapEditor::~MapEditor
 */

MapEditor::~MapEditor()
{
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
	setLoadProgress(1);
	QFile f(filename);

	if (!f.exists()) {
		m_client->sendMessageWarning(tr("A fájl nem található"), filename);
		return;
	}

	if (!m_db->isOpen()) {
		if (!m_db->open())
			return;
	}

	f.open(QIODevice::ReadOnly);
	QByteArray d = f.readAll();
	f.close();

	m_game = GameMap::fromBinaryData(d, this, "setLoadProgress");

	if (!m_game) {
		m_client->sendMessageError(tr("Hibás fájl"), filename);
		m_db->close();
		return;
	}

	qDebug() << "PREPARE START---------------------------------------------------";
	QFutureWatcher<void> www;
	connect(&www, &QFutureWatcher<void>::finished, this, &MapEditor::databaseLoaded);
	QFuture<void> future = QtConcurrent::run(this, &MapEditor::databasePrepare);
	www.setFuture(future);
	qDebug() << "PREPARE END-------------------------------------------------------";

}



/**
 * @brief MapEditor::loadFromBackup
 */

void MapEditor::loadFromBackup()
{
	if (!m_db->databaseExists()) {
		m_client->sendMessageError(tr("Backup hiba"), tr("A backup nem létezik"));
		return;
	}

	if (!m_db->isOpen()) {
		if (!m_db->open())
			return;
	}

	_prepare();
}





/**
 * @brief MapEditor::checkBackup
 */

void MapEditor::checkBackup()
{
	if (m_db->databaseExists()) {
		emit backupReady("BACKUP", m_db->databaseName());
		return;
	}

	emit backupUnavailable();
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

void MapEditor::setLoadProgress(qreal loadProgress)
{
	if (qFuzzyCompare(m_loadProgress, loadProgress))
		return;

	m_loadProgress = loadProgress;
	emit loadProgressChanged(m_loadProgress);
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

void MapEditor::databasePrepare()
{
	m_game->setProgressFunc(this, "setLoadProgress");

	if (!m_game->toDb(m_db)) {
		m_client->sendMessageError(tr("Adatfájl hiba"), m_db->databaseName());
		m_db->close();
		return;
	}

	setLoadProgress(2);

	_prepare();
}



void MapEditor::_prepare()
{
	if (!m_db->createUndoTables())	return;

	setLoadProgress(3);

	m_db->createTrigger("map");

	setLoadProgress(4);

	m_db->createTrigger("chapters");

	setLoadProgress(5);

	m_db->createTrigger("storages");

	setLoadProgress(6);

	m_db->createTrigger("objectives");
	m_db->createTrigger("missions");
	m_db->createTrigger("inventories");

	setLoadProgress(100);
}

