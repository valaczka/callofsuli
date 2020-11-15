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
#include "abstractactivity.h"
#include "qobjectmodel.h"
#include "qobjectdatalist.h"

/**
 * @brief The ServerData class
 */

class ServerData : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
	Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
	Q_PROPERTY(bool ssl READ ssl WRITE setSsl NOTIFY sslChanged)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(QString session READ session WRITE setSession NOTIFY sessionChanged)
	Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect NOTIFY autoconnectChanged)
	Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)

public:
	ServerData(QObject *parent = nullptr);
	ServerData(const QJsonObject &object, QObject *parent = nullptr);
	ServerData(const QVariantMap &map, QObject *parent = nullptr);
	~ServerData();

	QString name() const { return m_name; }
	QString host() const { return m_host; }
	int port() const { return m_port; }
	bool ssl() const { return m_ssl; }
	QString username() const { return m_username; }
	QString session() const { return m_session; }
	bool autoconnect() const { return m_autoconnect; }
	int id() const { return m_id; }

	QJsonObject asJsonObject() const;
	void set(const QVariantMap &map);

public slots:
	void setName(QString name);
	void setHost(QString host);
	void setPort(int port);
	void setSsl(bool ssl);
	void setUsername(QString username);
	void setSession(QString session);
	void setAutoconnect(bool autoconnect);
	void setId(int id);

signals:
	void nameChanged(QString name);
	void hostChanged(QString host);
	void portChanged(int port);
	void sslChanged(bool ssl);
	void usernameChanged(QString username);
	void sessionChanged(QString session);
	void autoconnectChanged(bool autoconnect);
	void idChanged(int id);

private:
	QString m_name;
	QString m_host;
	int m_port;
	bool m_ssl;
	QString m_username;
	QString m_session;
	bool m_autoconnect;
	int m_id;
};


/**
 * @brief The Servers class
 */


class Servers : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QVariantMap resources READ resources NOTIFY resourcesChanged)
	Q_PROPERTY(bool readyResources READ readyResources NOTIFY readyResourcesChanged)
	Q_PROPERTY(QObjectModel* serversModel READ serversModel NOTIFY serversModelChanged)
	Q_PROPERTY(ServerData* connectedServer READ connectedServer WRITE setConnectedServer NOTIFY connectedServerChanged)


public:
	Servers(QQuickItem *parent = nullptr);
	~Servers();

	QVariantMap resources() const { return m_resources; }
	void reloadResources(QVariantMap resources);
	void getDownloadedResource(const CosMessage &message);
	void registerResource(const QString &filename);
	void unregisterResources();
	void checkResources();
	bool readyResources() const { return m_readyResources; }

	QObjectModel *serversModel() const { return m_serversModel; }
	int nextId();
	ServerData* connectedServer() const { return m_connectedServer; }

	void saveServerList();


public slots:
	void serverListReload();
	void serverConnect(const int &index);
	int serverInsertOrUpdate(const int &index, const QVariantMap &map);
	void serverDelete(const int &index);
	void serverDeleteSelected(QObjectModel *model);
	void serverSetAutoConnect(const int &index);
	void serverTryLogin(ServerData *d);
	void serverLogOut();
	void doAutoConnect();

	void setReadyResources(bool readyResources);
	void setConnectedServer(ServerData* connectedServer);

protected slots:
	void clientSetup() override;
	void onMessageReceived(const CosMessage &message) override;
	void onMessageFrameReceived(const CosMessage &message) override;

	void removeServerDir(const int &serverId);

	void onSessionTokenChanged(QString sessionToken);
	void onConnectionStateChanged(Client::ConnectionState state);
	void onAuthInvalid();
	void onUserRolesChanged(CosMessage::ClientRoles userRoles);
	void onUserNameChanged(QString username);

signals:
	void serverListLoaded();

	void resourcesChanged(QVariantMap resources);
	void readyResourcesChanged(bool readyResources);
	void serversModelChanged(QObjectModel* serversModel);
	void connectedServerChanged(ServerData* connectedServer);

private:
	QVariantMap m_resources;
	bool m_readyResources;
	QObjectDataList m_serverList;
	QObjectModel* m_serversModel;
	QString m_dataFileName;
	ServerData* m_connectedServer;
	ServerData* m_serverTryConnect;
};



#endif // SERVERS_H
