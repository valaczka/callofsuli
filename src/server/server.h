/*
 * ---- Call of Suli ----
 *
 * server.h
 *
 * Created on: 2020. 03. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Server
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QtWebSockets/QtWebSockets>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslSocket>

#include "serverdb.h"
#include "client.h"

class Server : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString serverDir READ serverDir WRITE setServerDir NOTIFY serverDirChanged)
	Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)
	Q_PROPERTY(ServerDb * db READ db)
	Q_PROPERTY(QVariantMap resources READ resources WRITE setResources NOTIFY resourcesChanged)

	const int m_serverVersionMajor;
	const int m_serverVersionMinor;

private:
	QWebSocketServer *m_socketServer;

	QString m_serverDir;
	int m_versionMajor;
	int m_versionMinor;
	QString m_host;
	int m_port;
	int m_pendingConnections;
	QString m_serverName;
	QString m_socketCert;
	QString m_socketKey;
	bool m_createDb;

	QList<Client *> m_clients;
	ServerDb * m_db;
	QVariantMap m_resourceMap;
	QVariantMap m_resources;

public:
	explicit Server(QObject *parent = nullptr);
	~Server();

	bool start();
	void stop();
	bool commandLineParse(QCoreApplication &app);
	bool serverDirCheck();
	bool databaseLoad();
	bool rankInit();
	bool resourcesInit();

	bool websocketServerStart();
	void reloadResources();

	QString serverDir() const { return m_serverDir; }
	QString serverName() const { return m_serverName; }
	QByteArray resourceContent(const QString &filename, QString *md5 = nullptr) const;

	QWebSocketServer *socketServer() const { return m_socketServer; }
	ServerDb * db() const { return m_db; }
	QList<Client *> clients() const { return m_clients; }
	QVariantMap resourceMap() const { return m_resourceMap; }
	QVariantMap resources() const { return m_resources; }

private slots:
	void onSslErrors(const QList<QSslError> &errors);
	void onNewConnection();
	void onSocketDisconnected();

public slots:
	void setServerDir(QString serverDir);
	void setServerName(QString serverName);
	void setResources(QVariantMap resources);

signals:
	void serverDirChanged(QString serverDir);
	void serverNameChanged(QString serverName);
	void resourcesChanged(QVariantMap resources);
	void serverInfoChanged();
};

#endif // SERVER_H
