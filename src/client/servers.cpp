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
	m_connectedServerId = -1;
	m_tryToConnectServerId = -1;
}



/**
 * @brief Servers::serverListReload
 */

int Servers::serverListReload()
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return -1;

	QVariantMap r2 = m_db->runSimpleQuery("SELECT id, name as label, false as disabled, false as selected, '' as image, "
										  "EXISTS(SELECT * FROM autoconnect WHERE autoconnect.serverid=server.id) as autoconnect "
										  "FROM server "
										  "ORDER BY name");
	if (r2["error"].toBool()) {
		m_client->sendMessageError(tr("Adatbázis"), tr("Adatbázis hiba!"), databaseFile());
		return -1;
	}

	int autoconnectId = -1;

	QVariantList list = r2["records"].toList();
	QVariantList list2;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		if (m["autoconnect"].toBool()) {
			m["icon"] = "M\ue838";
			autoconnectId = m["id"].toInt();
		} else {
			m["icon"] = "M\ue83a";
		}
		list2 << m;
	}

	emit serverListLoaded(list2);

	return autoconnectId;

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

	QVariantMap r = m_db->runSimpleQuery("SELECT name, host, port, ssl, cert, username, session FROM server WHERE id=?", p);

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

	m_tryToConnectServerId = serverId;

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

void Servers::serverSetAutoConnect(const int &serverId, const bool &value)
{
	Q_ASSERT (m_client);

	if (!databaseOpen())
		return;

	QVariantMap r;

	if (value)
		r = m_db->runSimpleQuery("DELETE FROM autoconnect");
	else {
		QVariantList l;
		l << serverId;
		r = m_db->runSimpleQuery("DELETE FROM autoconnect WHERE serverid=", l);
	}

	if (r["error"].toBool()) {
		emit databaseError(r["errorString"].toString());
		return;
	}

	if (value) {
		QVariantMap m;
		m["serverid"] = serverId;
		QSqlQuery q;
		q = m_db->insertQuery("INSERT INTO autoconnect (?k?) VALUES (?)", m);
		QVariantMap r2 = m_db->runQuery(q);
		if (r2["error"].toBool()) {
			emit databaseError(r2["errorString"].toString());
			return;
		}
	}

	emit serverInfoUpdated(serverId);
}


/**
 * @brief Servers::serverTryLogin
 * @param serverId
 */

void Servers::serverTryLogin(const int &serverId)
{
	if (!databaseOpen())
		return;

	QVariantList p;
	p << serverId;

	QVariantMap r = m_db->runSimpleQuery("SELECT username, session FROM server WHERE id=?", p);

	QVariantList rl = r["records"].toList();
	if (r["error"].toBool() || rl.count() != 1) {
		return;
	}

	QString username = rl.value(0).toMap().value("username").toString();
	QString session = rl.value(0).toMap().value("session").toString();

	m_client->login(username, session);
}




void Servers::setConnectedServerId(int connectedServerId)
{
	if (m_connectedServerId == connectedServerId)
		return;

	m_connectedServerId = connectedServerId;
	emit connectedServerIdChanged(m_connectedServerId);
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


/**
 * @brief Servers::clientSetup
 */

void Servers::clientSetup()
{
	connect(m_client, &Client::jsonReceived, this, &Servers::jsonParse);
	connect(m_client, &Client::connectionStateChanged, this, &Servers::onConnectionStateChanged);
	connect(m_client, &Client::sessionTokenChanged, this, &Servers::onSessionTokenChanged);
	connect(m_client, &Client::userNameChanged, this, &Servers::onUserNameChanged);
}


/**
 * @brief Servers::jsonParse
 * @param object
 */

void Servers::jsonParse(const QJsonObject &object)
{

}


/**
 * @brief Servers::onSessionTokenChanged
 * @param sessionToken
 */

void Servers::onSessionTokenChanged(QString sessionToken)
{
	if (m_connectedServerId != -1 && !sessionToken.isEmpty()) {
		QVariantList l;
		l << sessionToken;
		l << m_connectedServerId;
		m_db->runSimpleQuery("UPDATE server SET session=? WHERE id=?", l);
	}
}


/**
 * @brief Servers::onConnectionStateChanged
 * @param state
 */

void Servers::onConnectionStateChanged(Client::ConnectionState state)
{
	if (state == Client::Connected) {
		setConnectedServerId(m_tryToConnectServerId);
		serverTryLogin(m_connectedServerId);
	} else if (state == Client::Standby) {
		setConnectedServerId(-1);
	}
}


/**
 * @brief Servers::onUserNameChanged
 * @param username
 */


void Servers::onUserNameChanged(QString username)
{
	if (m_connectedServerId != -1 && !username.isEmpty()) {
		QVariantList l;
		l << username;
		l << m_connectedServerId;
		m_db->runSimpleQuery("UPDATE server SET username=? WHERE id=?", l);
	}
}
