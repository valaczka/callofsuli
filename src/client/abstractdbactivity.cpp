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

#include "abstractdbactivity.h"

AbstractDbActivity::AbstractDbActivity(const QString &connectionName, QObject *parent)
	: COSdb(connectionName, parent)
{
	m_client = nullptr;

	m_canUndo = -1;

	connect(m_db, &CosSql::canUndoChanged, this, &AbstractDbActivity::setCanUndo);
}





void AbstractDbActivity::setClient(Client *client)
{
	if (m_client == client)
		return;

	m_client = client;
	emit clientChanged(m_client);

	if (m_client) {
		connect(this, &COSdb::databaseError, m_client, &Client::sendDatabaseError);
		clientSetup();
	}
}


/**
 * @brief AbstractDbActivity::execSelectQuery
 * @param query
 * @param params
 * @return
 */

QVariantList AbstractDbActivity::execSelectQuery(const QString &query, const QVariantList &params)
{
	QVariantList ret;

	if (!m_db->execSelectQuery(query, params, &ret)) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Lekérdezési hiba"));
	}

	return ret;
}

void AbstractDbActivity::setCanUndo(int canUndo)
{
	if (m_canUndo == canUndo)
		return;

	m_canUndo = canUndo;
	emit canUndoChanged(m_canUndo);
}

