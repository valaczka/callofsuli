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
#include <QUdpSocket>
#include "abstractactivity.h"
#include "serverobject.h"
#include "sqlimage.h"
#include "gamematch.h"
#include "googleoauth2.h"

class Servers : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(GoogleOAuth2 * googleOAuth2 READ googleOAuth2 WRITE setGoogleOAuth2 NOTIFY googleOAuth2Changed)
	Q_PROPERTY(ObjectGenericListModel<ServerObject>* serversModel READ serversModel NOTIFY serversModelChanged)

public:
	Servers(QQuickItem *parent = nullptr);
	virtual ~Servers();

	ServerObject * connectedServerKey() const { return m_connectedServer; }

	void saveServerList();
	QVariantMap createFullMap(const QVariantMap &newData = QVariantMap(), const QVariantMap &from = QVariantMap());

	ServerObject *findServer(const QString &host, const int &port, const bool &ssl);
	ServerObject *findServer(const QString &username, const QString &host, const int &port, const bool &ssl);


	GoogleOAuth2 *googleOAuth2() const;
	void setGoogleOAuth2(GoogleOAuth2 *newGoogleOAuth2);
	void setGoogleOAuth2(const QString &id, const QString &key, const qint16 &port);

	ObjectGenericListModel<ServerObject> *serversModel() const;

public slots:
	void reloadResources(QVariantMap resources);
	void registerResource(const QString &filename);
	void unregisterResources();

	void serverListReload();
	void serverConnect(ServerObject *server);
	ServerObject* serverCreate(const QJsonObject &json);
	void serverDelete(ServerObject *server);
	void serverDeleteList(const QList<ServerObject *> list);
	void serverDeleteList(const QObjectList &list);
	void serverSetAutoConnect(ServerObject *server);
	bool serverTryLogin();
	void serverLogOut();
	void doAutoConnect(const QStringList &arguments = QStringList());
	bool parseUrls(const QStringList &urls);
	bool parseUrl(const QString &url);

	void sendBroadcast();

	void acceptCertificate(ServerObject *server, const QSslCertificate &cert, const QList<int> &errorList);

protected slots:
	void onMessageReceived(const CosMessage &message) override;
	void onOneResourceDownloaded(const CosDownloaderItem &item, const QByteArray &, const QJsonObject &);

	void removeServerDir(const int &serverId);

	void onSessionTokenChanged(QString sessionToken);
	void onConnectionStateChanged(Client::ConnectionState state);
	void onAuthInvalid();
	void onUserRolesChanged(CosMessage::ClientRoles userRoles);
	void onUserNameChanged(QString username);

private slots:
	void onUdpDatagramReceived();
	void onSocketSslErrors(QList<QSslError> errors);

signals:
	void serverListLoaded();
	void serverCreated(ServerObject *server);
	void resourceDownloadRequest(QString formattedDataSize);
	void resourceRegisterRequest(QString filename);
	void resourceReady();

	void serversModelChanged();
	void googleOAuth2Changed();

	void certificateError(ServerObject *server, const QSslCertificate &certificate, const QVariantMap &data);


private:
	void _reloadResources(QVariantMap resources);

	QString m_dataFileName;
	QPointer<ServerObject> m_connectedServer;
	QPointer<ServerObject> m_serverTryToConnect;
	QStringList m_sqlImageProviders;
	QUdpSocket* m_udpSocket;
	QStringList m_urlsToProcess;
	GoogleOAuth2 *m_googleOAuth2;
	ObjectGenericListModel<ServerObject> *m_serversModel;
};



#endif // SERVERS_H
