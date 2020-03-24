/*
 * ---- Call of Suli ----
 *
 * servers.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Servers
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

#include "servers.h"

Servers::Servers(QObject *parent)
	: AbstractActivity(parent)
{
	m_databaseFile = Client::standardPath("servers.db");
}



/**
 * @brief Servers::serverListReload
 */

void Servers::serverListReload()
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return;

	QVariantMap r2 = m_db->runSimpleQuery("SELECT id, name as label, false as disabled, false as selected, '' as image, "
										  "EXISTS(SELECT * FROM autoconnect WHERE autoconnect.serverid=server.id) as autoconnect "
										  "FROM server "
										  "ORDER BY name");
	if (r2["error"].toBool()) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Adatbázis hiba!"), databaseFile());
		return;
	}

	QVariantList list = r2["records"].toList();
	QVariantList list2;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		if (m["autoconnect"].toBool())
			m["icon"] = "M\ue838";
		else
			m["icon"] = "M\ue83a";
		list2 << m;
	}

	emit serverListLoaded(list2);

}


/**
 * @brief Servers::serverGet
 * @param serverId
 */

QVariantMap Servers::serverInfoGet(const int &serverId)
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return QVariantMap();

	QVariantList p;
	p << serverId;

	QVariantMap r = m_db->runSimpleQuery("SELECT name, host, port, ssl, user, password, cert FROM server WHERE id=?", p);

	QVariantList rl = r["records"].toList();
	if (r["error"].toBool() || rl.count() != 1) {
		m_client->sendMessageError(tr("Internal error"), QString(tr("Érvénytelen azonosító: %1")).arg(serverId));
		return QVariantMap();
	}

	QVariantMap server = rl.value(0).toMap();

	emit serverInfoLoaded(server);

	return server;
}



/**
 * @brief Servers::serverInfoInsertOrUpdate
 * @param map
 */

void Servers::serverInfoInsertOrUpdate(const int &id, const QVariantMap &map)
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return;

	QSqlQuery q;
	if (id == -1) {
		q = m_db->insertQuery("INSERT INTO server (?k?) VALUES (?)", map);
	} else {
		q = m_db->updateQuery("UPDATE server SET ? WHERE id=:id", map);
		q.bindValue(":id", id);
	}

	QVariantMap r = m_db->runQuery(q);
	if (r["error"].toBool()) {
		emit databaseError(r["errorString"].toString());
		return;
	}

	emit serverInfoUpdated( id == -1 ? r["lastInsertId"].toInt() : id);
}


/**
 * @brief Servers::serverInfoDelete
 * @param id
 */

void Servers::serverInfoDelete(const int &id)
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return;

	QVariantList l;
	l << id;
	QVariantMap r = m_db->runSimpleQuery("DELETE FROM server WHERE id=?", l);
	if (r["error"].toBool()) {
		emit databaseError(r["errorString"].toString());
		return;
	}

	emit serverInfoUpdated(id);
}


/**
 * @brief Servers::serverConnect
 * @param serverId
 */

void Servers::serverConnect(const int &serverId)
{
	Q_ASSERT (m_client);

	QVariantMap m = serverInfoGet(serverId);

	if (m.empty())
		return;

	QUrl url;
	url.setHost(m["host"].toString());
	url.setPort(m["port"].toInt());
	url.setScheme(m["ssl"].toBool() ? "wss" : "ws");


	QByteArray cert = m["cert"].toByteArray();
	if (!cert.isEmpty()) {
		QSslCertificate c(cert, QSsl::Pem);
		if (!c.isNull()) {
			QList<QSslError> eList;
			eList.append(QSslError(QSslError::SelfSignedCertificate, c));
			eList.append(QSslError(QSslError::HostNameMismatch, c));

			m_client->socket()->ignoreSslErrors(eList);
		}
	}

	m_client->socket()->open(url);
}




/**
 * @brief Servers::serverSetAutoConnect
 * @param serverId
 */

void Servers::serverSetAutoConnect(const int &serverId)
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return;

	QVariantMap r = m_db->runInsertQuery("DELETE FROM autoconnect");
	if (r["error"].toBool()) {
		emit databaseError(r["errorString"].toString());
		return;
	}

	QVariantMap m;
	m["serverid"] = serverId;
	QSqlQuery q;
	q = m_db->insertQuery("INSERT INTO autoconnect (?k?) VALUES (?)", m);
	QVariantMap r2 = m_db->runQuery(q);
	if (r2["error"].toBool()) {
		emit databaseError(r2["errorString"].toString());
		return;
	}

	emit serverInfoUpdated(serverId);
}




/**
 * @brief Servers::databaseInit
 * @return
 */


bool Servers::databaseInit()
{
	Q_ASSERT(m_client);

	if (!m_db->batchQueryFromFile(":/sql/servers.sql")) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Nem sikerült előkészíteni az adatbázist!"), databaseFile());
		return false;
	}

	return true;
}
