/*
 * ---- Call of Suli ----
 *
 * abstractactivity.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractActivity
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

#include "abstractactivity.h"

AbstractActivity::AbstractActivity(QObject *parent) : QObject(parent)
{
	m_client = nullptr;
	m_db = nullptr;
}

/**
 * @brief AbstractActivity::~AbstractActivity
 */

AbstractActivity::~AbstractActivity()
{
	// Az m_client-et nem töröljük, az a QML-ben lesz megadva.
	if (m_db)
		delete m_db;
}


/**
 * @brief AbstractActivity::databaseInit
 * @param filename
 * @return
 */

bool AbstractActivity::databaseOpen()
{
	Q_ASSERT (m_client);

	if (m_databaseFile.isEmpty()) {
		qWarning().noquote() << tr("Nincs megadva adatbázis!");
		m_client->sendMessageError(tr("Internal error"), tr("Nincs megadva adatbázis!"));
		return false;
	}

	if (m_db) {
		qInfo().noquote() << tr("Az adatbázis már meg van nyitva: ")+m_db->db().databaseName();
	} else {
		bool isOwnCreated = false;

		if (!QFile::exists(m_databaseFile)) {
			qDebug() << tr("Új adatbázis létrehozása ")+m_databaseFile;
			isOwnCreated = true;
		}

		m_db=new CosSql(this);
		if (!m_db->open(m_databaseFile, true)) {
			qWarning().noquote() << tr("Nem lehet megnyitni az adatbázist: ")+m_databaseFile;
			m_client->sendMessageError(tr("Internal error"), tr("Nem lehet megnyitni az adatbázist!"), m_databaseFile);
			return false;
		}

		if (!databaseInit()) {
			qWarning().noquote() << tr("Nem lehet létrehozni az adatbázist: ")+m_databaseFile;
			m_client->sendMessageError(tr("Internal error"), tr("Nem lehet létrehozni az adatbázist!"), m_databaseFile);


			if (isOwnCreated) {
				qDebug().noquote() << tr("Az adatbázis félkész, törlöm: ")+m_databaseFile;
				m_db->close();
				if (!QFile::remove(m_databaseFile)) {
					qWarning().noquote() << tr("Nem sikerült törölni a hibás adatbázist: ")+m_databaseFile;
				}
			}

			return false;
		}
	}

	return true;
}





void AbstractActivity::setClient(Client *client)
{
	if (m_client == client)
		return;

	m_client = client;
	emit clientChanged(m_client);

	qDebug() << "setClient" << m_client;

	if (m_client) {
		connect(this, &AbstractActivity::databaseError, m_client, &Client::sendDatabaseError);
		clientSetup();
	}
}


void AbstractActivity::setDb(CosSql *db)
{
	if (m_db == db)
		return;

	m_db = db;
	emit dbChanged(m_db);
}

void AbstractActivity::setDatabaseFile(QString databaseFile)
{
	if (m_databaseFile == databaseFile)
		return;

	m_databaseFile = databaseFile;
	emit databaseFileChanged(m_databaseFile);
}

