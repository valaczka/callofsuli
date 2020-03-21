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
#include <QtSql>
#include <QtWebSockets/QtWebSockets>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslSocket>


class Server : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString sqlDbFile READ sqlDbFile WRITE setSqlDbFile NOTIFY sqlDbFileChanged)
	Q_PROPERTY(bool sqlDbCreate READ sqlDbCreate WRITE setSqlDbCreate NOTIFY sqlDbCreateChanged)
	Q_PROPERTY(bool readyToStart READ readyToStart WRITE setReadyToStart NOTIFY readyToStartChanged)

	Q_PROPERTY(int dbVersionMajor READ dbVersionMajor WRITE setDbVersionMajor NOTIFY dbVersionMajorChanged)
	Q_PROPERTY(int dbVersionMinor READ dbVersionMinor WRITE setDbVersionMinor NOTIFY dbVersionMinorChanged)
	Q_PROPERTY(QString dbSocketHost READ dbSocketHost WRITE setDbSocketHost NOTIFY dbSocketHostChanged)
	Q_PROPERTY(int dbSocketPort READ dbSocketPort WRITE setDbSocketPort NOTIFY dbSocketPortChanged)
	Q_PROPERTY(QString dbServerName READ dbServerName WRITE setDbServerName NOTIFY dbServerNameChanged)
	Q_PROPERTY(QString socketCertFile READ socketCertFile WRITE setSocketCertFile NOTIFY socketCertFileChanged)
	Q_PROPERTY(QString socketCertKey READ socketCertKey WRITE setSocketCertKey NOTIFY socketCertKeyChanged)
	Q_PROPERTY(int socketPendingConnections READ socketPendingConnections WRITE setSocketPendingConnections NOTIFY socketPendingConnectionsChanged)

	mutable int m_serverVersionMajor;
	mutable int m_serverVersionMinor;
	bool m_isHostForced;
	bool m_isPortForced;

	QWebSocketServer *m_socketServer;

	QString m_sqlDbFile;
	bool m_readyToStart;
	QSqlDatabase m_db;
	bool m_sqlDbCreate;
	int m_dbVersionMajor;
	int m_dbVersionMinor;
	QString m_dbSocketHost;
	int m_dbSocketPort;
	QString m_dbServerName;
	QString m_socketCertFile;
	QString m_socketCertKey;
	int m_socketPendingConnections;

public:
	explicit Server(QObject *parent = nullptr);
	~Server();

	void start();
	void stop();
	void commandLineParse(QCoreApplication &app);
	void databaseOpen();
	void databaseLoad();
	void databaseInit();

	void websocketServerStart();



	int serverVersionMajor() const { return m_serverVersionMajor; }
	int serverVersionMinor() const { return m_serverVersionMinor; }

	QString sqlDbFile() const { return m_sqlDbFile; }
	bool readyToStart() const { return m_readyToStart; }
	QSqlDatabase db() const { return m_db; }
	bool sqlDbCreate() const { return m_sqlDbCreate; }
	int dbVersionMajor() const { return m_dbVersionMajor; }
	int dbVersionMinor() const { return m_dbVersionMinor; }
	QString dbSocketHost() const { return m_dbSocketHost; }
	int dbSocketPort() const { return m_dbSocketPort; }
	QString dbServerName() const { return m_dbServerName; }
	QString socketCertFile() const { return m_socketCertFile; }
	QString socketCertKey() const { return m_socketCertKey; }
	int socketPendingConnections() const { return m_socketPendingConnections; }

	QWebSocketServer *socketServer() const { return m_socketServer; }

private slots:
	void onSslErrors(const QList<QSslError> &errors);
	void onNewConnection();

public slots:
	void setSqlDbFile(QString sqlDbFile);
	void setReadyToStart(bool readyToStart);
	void setSqlDbCreate(bool sqlDbCreate);
	void setDbVersionMajor(int dbVersionMajor);
	void setDbVersionMinor(int dbVersionMinor);
	void setDbSocketHost(QString dbSocketHost);
	void setDbSocketPort(int dbSocketPort);
	void setDbServerName(QString dbServerName);
	void setSocketCertFile(QString socketCertFile);
	void setSocketCertKey(QString socketCertKey);
	void setSocketPendingConnections(int socketPendingConnections);

signals:
	void sqlDbFileChanged(QString sqlDbFile);
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
};

#endif // SERVER_H
