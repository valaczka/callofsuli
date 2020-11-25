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
#include "variantmapmodel.h"
#include "variantmapdata.h"
#include "sqlimage.h"

class Servers : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QVariantMap resources READ resources NOTIFY resourcesChanged)
	Q_PROPERTY(bool readyResources READ readyResources NOTIFY readyResourcesChanged)
	Q_PROPERTY(VariantMapModel* serversModel READ serversModel NOTIFY serversModelChanged)
	Q_PROPERTY(int connectedServerKey READ connectedServerKey WRITE setConnectedServerKey NOTIFY connectedServerKeyChanged)


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

	VariantMapModel *serversModel() const { return m_serversModel; }
	int connectedServerKey() const { return m_connectedServerKey; }

	void saveServerList();
	QVariantMap createFullMap(const QVariantMap &newData = QVariantMap(), const QVariantMap &from = QVariantMap());


public slots:
	void serverListReload();
	void serverConnect(const int &index);
	int serverInsertOrUpdate(const int &key, const QVariantMap &map);
	void serverDelete(const int &index);
	void serverDeleteKey(const int &key);
	void serverDeleteSelected(VariantMapModel *model);
	void serverSetAutoConnect(const int &index);
	void serverTryLogin(const int &key);
	void serverLogOut();
	void doAutoConnect();

	void setReadyResources(bool readyResources);
	void setConnectedServerKey(int connectedServerKey);

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
	void serversModelChanged(VariantMapModel* serversModel);

	void connectedServerKeyChanged(int connectedServerKey);

private:
	QVariantMap m_resources;
	bool m_readyResources;
	VariantMapData m_serverList;
	VariantMapModel* m_serversModel;
	QString m_dataFileName;
	int m_connectedServerKey;
	int m_serverTryConnectKey;
	QStringList m_sqlImageProviders;
};



#endif // SERVERS_H
