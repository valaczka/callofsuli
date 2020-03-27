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

#include "../common/cossql.h"
#include "client.h"

class Server : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString sqlDbDir READ sqlDbDir WRITE setSqlDbDir NOTIFY sqlDbDirChanged)
	Q_PROPERTY(bool sqlDbCreate READ sqlDbCreate WRITE setSqlDbCreate NOTIFY sqlDbCreateChanged)

	Q_PROPERTY(int dbVersionMajor READ dbVersionMajor WRITE setDbVersionMajor NOTIFY dbVersionMajorChanged)
	Q_PROPERTY(int dbVersionMinor READ dbVersionMinor WRITE setDbVersionMinor NOTIFY dbVersionMinorChanged)
	Q_PROPERTY(QString dbSocketHost READ dbSocketHost WRITE setDbSocketHost NOTIFY dbSocketHostChanged)
	Q_PROPERTY(int dbSocketPort READ dbSocketPort WRITE setDbSocketPort NOTIFY dbSocketPortChanged)
	Q_PROPERTY(QString dbServerName READ dbServerName WRITE setDbServerName NOTIFY dbServerNameChanged)
	Q_PROPERTY(QString socketCertFile READ socketCertFile WRITE setSocketCertFile NOTIFY socketCertFileChanged)
	Q_PROPERTY(QString socketCertKey READ socketCertKey WRITE setSocketCertKey NOTIFY socketCertKeyChanged)
	Q_PROPERTY(int socketPendingConnections READ socketPendingConnections WRITE setSocketPendingConnections NOTIFY socketPendingConnectionsChanged)
	Q_PROPERTY(QString dbResources READ dbResources WRITE setDbResources NOTIFY dbResourcesChanged)
	Q_PROPERTY(QByteArray dbResourcesHash READ dbResourcesHash WRITE setDbResourcesHash NOTIFY dbResourcesHashChanged)

	mutable int m_serverVersionMajor;
	mutable int m_serverVersionMinor;
	bool m_isHostForced;
	bool m_isPortForced;
	bool m_isConnectionForced;

	QWebSocketServer *m_socketServer;

	QString m_sqlDbDir;
	CosSql* m_db;
	bool m_sqlDbCreate;
	int m_dbVersionMajor;
	int m_dbVersionMinor;
	QString m_dbSocketHost;
	int m_dbSocketPort;
	QString m_dbServerName;
	QString m_socketCertFile;
	QString m_socketCertKey;
	int m_socketPendingConnections;
	QString m_dbResources;
	QByteArray m_dbResourcesHash;

private:
	QList<Client *> m_clients;

public:
	explicit Server(QObject *parent = nullptr);
	~Server();

	bool start();
	void stop();
	void commandLineParse(QCoreApplication &app);
	bool dbDirCheck(const QString &dirname, bool create);
	bool databaseLoad();
	bool databaseInit();
	bool resourcesLoad();

	bool websocketServerStart();

	int serverVersionMajor() const { return m_serverVersionMajor; }
	int serverVersionMinor() const { return m_serverVersionMinor; }

	QString sqlDbDir() const { return m_sqlDbDir; }
	CosSql* db() { return m_db; }
	bool sqlDbCreate() const { return m_sqlDbCreate; }
	int dbVersionMajor() const { return m_dbVersionMajor; }
	int dbVersionMinor() const { return m_dbVersionMinor; }
	QString dbSocketHost() const { return m_dbSocketHost; }
	int dbSocketPort() const { return m_dbSocketPort; }
	QString dbServerName() const { return m_dbServerName; }
	QString socketCertFile() const { return m_socketCertFile; }
	QString socketCertKey() const { return m_socketCertKey; }
	int socketPendingConnections() const { return m_socketPendingConnections; }
	QString dbResources() const { return m_dbResources; }
	QByteArray dbResourcesHash() const { return m_dbResourcesHash; }

	QWebSocketServer *socketServer() const { return m_socketServer; }


private slots:
	void onSslErrors(const QList<QSslError> &errors);
	void onNewConnection();
	void onSocketDisconnected();

public slots:
	void setSqlDbDir(QString sqlDbDir);
	void setSqlDbCreate(bool sqlDbCreate);
	void setDbVersionMajor(int dbVersionMajor);
	void setDbVersionMinor(int dbVersionMinor);
	void setDbSocketHost(QString dbSocketHost);
	void setDbSocketPort(int dbSocketPort);
	void setDbServerName(QString dbServerName);
	void setSocketCertFile(QString socketCertFile);
	void setSocketCertKey(QString socketCertKey);
	void setSocketPendingConnections(int socketPendingConnections);
	void setDbResources(QString dbResources);
	void setDbResourcesHash(QByteArray dbResourcesHash);

signals:
	void sqlDbDirChanged(QString sqlDbDir);
	void readyToStartChanged(bool readyToStart);
	void sqlDbCreateChanged(bool sqlDbCreate);
	void dbVersionMajorChanged(int dbVersionMajor);
	void dbVersionMinorChanged(int dbVersionMinor);
	void dbSocketHostChanged(QString dbSocketHost);
	void dbSocketPortChanged(int dbSocketPort);
	void dbServerNameChanged(QString dbServerName);
	void socketCertFileChanged(QString socketCertFile);
	void socketCertKeyChanged(QString socketCertKey);
	void socketPendingConnectionsChanged(int socketPendingConnections);
	void dbResourcesChanged(QString dbResources);
	void dbResourcesHashChanged(QByteArray dbResourcesHash);
};

#endif // SERVER_H
