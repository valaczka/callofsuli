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
	: AbstractDbActivity("serversDb", parent)
{
	setDatabaseFile(Client::standardPath("servers.db"));
	m_connectedServerId = -1;
	m_tryToConnectServerId = -1;
}





/**
 * @brief Servers::serverListReload
 */

void Servers::serverListReload()
{
	if (!databaseOpen())
		return;

	QVariantList list;
	if (!execSelectQuery("SELECT id, name, "
							   "EXISTS(SELECT * FROM autoconnect WHERE autoconnect.serverid=server.id) as autoconnect "
							   "FROM server "
							   "ORDER BY name", QVariantList(), &list)) {
		return;
	}

	emit serverListLoaded(list);
}


/**
 * @brief Servers::serverGet
 * @param serverId
 */

QVariantMap Servers::serverInfoGet(const int &serverId)
{
	if (!databaseOpen())
		return QVariantMap();

	QVariantMap server;

	QVariantList p;
	p << serverId;

	if (!execSelectQueryOneRow("SELECT name, host, port, ssl, cert, username, session FROM server WHERE id=?", p, &server)) {
		return QVariantMap();
	}

	emit serverInfoLoaded(server);

	return server;
}



/**
 * @brief Servers::serverInfoInsertOrUpdate
 * @param map
 */

void Servers::serverInfoInsertOrUpdate(const int &id, const QVariantMap &map)
{
	if (!databaseOpen())
		return;

	if (id == -1) {
		int newId = execInsertQuery("INSERT INTO server (?k?) VALUES (?)", map);
		if (newId == -1)
			return;
		emit serverInfoUpdated(newId);
	} else {
		QVariantMap bindValues;
		bindValues[":id"] = id;
		if (!execUpdateQuery("UPDATE server SET ? WHERE id=:id", map, bindValues))
			return;
		emit serverInfoUpdated(id);
	}
}


/**
 * @brief Servers::serverInfoDelete
 * @param id
 */

void Servers::serverInfoDelete(const int &id)
{
	if (!databaseOpen())
		return;

	QVariantList l;
	l << id;
	if (!execSimpleQuery("DELETE FROM server WHERE id=?", l))
		return;

	emit serverInfoUpdated(id);

	removeServerDir(id);
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
	url.setHost(m.value("host").toString());
	url.setPort(m.value("port").toInt());
	url.setScheme(m.value("ssl").toBool() ? "wss" : "ws");


	QByteArray cert = m.value("cert").toByteArray();
	if (!cert.isEmpty()) {
		QSslCertificate c(cert, QSsl::Pem);
		if (!c.isNull()) {
			QList<QSslError> eList;
			eList.append(QSslError(QSslError::SelfSignedCertificate, c));
			eList.append(QSslError(QSslError::HostNameMismatch, c));

			m_client->socket()->ignoreSslErrors(eList);
		}
	}

	QString dir = QString("%1").arg(serverId);
	QString serverDir = Client::standardPath(dir);
	if (!QFileInfo::exists(serverDir)) {
		QDir d(Client::standardPath());
		if (!d.mkdir(dir)) {
			m_client->sendMessageError(tr("Programhiba"), tr("Nem sikerült létrehozni a könyvtárt:"), serverDir);
			return;
		}
	}

	/*QString defaultResourceDb = serverDir+"/resources.db";
	if (!QFile::exists(defaultResourceDb)) {
		qInfo() << tr("Adatbázis létrehozása:") << defaultResourceDb;
		QFile from(":/db/default.db");
		if (!from.open(QIODevice::ReadOnly)) {
			m_client->sendMessageError(tr("Programhiba"), tr("Nem sikerült megnyitni az adatbázist!"));
			return;
		}
		QByteArray b = from.readAll();
		from.close();
		QFile f(defaultResourceDb);
		if (f.open(QIODevice::WriteOnly)) {
			f.write(b);
			f.close();
		}
	}*/

	m_client->setServerDataDir(serverDir);

	m_client->socket()->open(url);
}




/**
 * @brief Servers::serverSetAutoConnect
 * @param serverId
 */

void Servers::serverSetAutoConnect(const int &serverId, const bool &value)
{
	if (!databaseOpen())
		return;

	QVariantList l;
	l << serverId;

	execSimpleQuery("DELETE FROM autoconnect");

	if (value) {
		QVariantMap m;
		m["serverid"] = serverId;
		execInsertQuery("INSERT INTO autoconnect (?k?) VALUES (?)", m);
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

	QVariantMap r;

	if (!execSelectQueryOneRow("SELECT username, session FROM server WHERE id=?", p, &r))
		return;

	QString username = r.value("username").toString();
	QString session = r.value("session").toString();

	if (!username.isEmpty() && !session.isEmpty())
		m_client->login(username, session);
}


/**
 * @brief Servers::serverLogOut
 */

void Servers::serverLogOut()
{
	if (m_connectedServerId != -1) {
		QVariantList l;
		l << m_connectedServerId;
		execSimpleQuery("UPDATE server SET session=null WHERE id=?", l);
	}
}


/**
 * @brief Servers::doAutoConnect
 */

void Servers::doAutoConnect()
{
	QVariantMap m;

	qDebug() << "Auto connect";

	if (execSelectQueryOneRow("SELECT serverid FROM autoconnect", QVariantList(), &m)) {
		serverConnect(m.value("serverid").toInt());
	}
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

	if (!batchQueryFromFile(":/sql/servers.sql")) {
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
	connect(m_client, &Client::connectionStateChanged, this, &Servers::onConnectionStateChanged);
	connect(m_client, &Client::sessionTokenChanged, this, &Servers::onSessionTokenChanged);
	connect(m_client, &Client::userNameChanged, this, &Servers::onUserNameChanged);
	connect(m_client, &Client::authInvalid, this, &Servers::onAuthInvalid);
}


/**
 * @brief Servers::removeServerDir
 */

void Servers::removeServerDir(const int &serverId)
{
	QString dir = QString("%1").arg(serverId);
	QString serverDir = Client::standardPath(dir);

	QDir d(serverDir);
	if (d.exists())
		qDebug() << tr("Remove server files") << serverDir << d.removeRecursively();
}


/**
 * @brief Servers::onSessionTokenChanged
 * @param sessionToken
 */

void Servers::onSessionTokenChanged(QString sessionToken)
{
	if (m_connectedServerId != -1) {
		QVariantList l;
		l << sessionToken;
		l << m_connectedServerId;
		execSimpleQuery("UPDATE server SET session=? WHERE id=?", l);
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
 * @brief Servers::onInvalidAuth
 */

void Servers::onAuthInvalid()
{
	serverLogOut();
}

/**
 * @brief Servers::onUserRolesChanged
 * @param userRoles
 */

void Servers::onUserRolesChanged(Client::Roles userRoles)
{
	if (!userRoles.testFlag(Client::RoleStudent) &&
		!userRoles.testFlag(Client::RoleTeacher) &&
		!userRoles.testFlag(Client::RoleAdmin))
	{
		serverLogOut();
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
		execSimpleQuery("UPDATE server SET username=? WHERE id=?", l);
	}
}
