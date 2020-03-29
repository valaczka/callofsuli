/*
 * ---- Call of Suli ----
 *
 * servers.h
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

#ifndef SERVERS_H
#define SERVERS_H

#include <QObject>
#include "abstractdbactivity.h"

class Servers : public AbstractDbActivity
{
	Q_OBJECT

	Q_PROPERTY(int connectedServerId READ connectedServerId WRITE setConnectedServerId NOTIFY connectedServerIdChanged)


public:
	Servers(QObject *parent = nullptr);

	int connectedServerId() const { return m_connectedServerId; }

public slots:
	int serverListReload();
	QVariantMap serverInfoGet(const int &serverId);
	void serverInfoInsertOrUpdate(const int &id, const QVariantMap &map);
	void serverInfoDelete(const int &id);
	void serverConnect(const int &serverId);
	void serverSetAutoConnect(const int &serverId, const bool &value = true);
	void serverTryLogin(const int &serverId);
	void serverLogOut();

	void setConnectedServerId(int connectedServerId);

protected slots:
	bool databaseInit() override;
	void clientSetup() override;


	void onSessionTokenChanged(QString sessionToken);
	void onConnectionStateChanged(Client::ConnectionState state);
	void onAuthInvalid();
	void onUserRolesChanged(Client::Roles userRoles);
	void onUserNameChanged(QString username);

signals:
	void serverListLoaded(const QVariantList &serverList);
	void serverInfoLoaded(const QVariantMap &server);
	void serverInfoUpdated(const int &serverId);

	void connectedServerIdChanged(int connectedServerId);

private:
	int m_connectedServerId;
	int m_tryToConnectServerId;
};

#endif // SERVERS_H
