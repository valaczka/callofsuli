/*
 * ---- Call of Suli ----
 *
 * server.cpp
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

#include <QCoreApplication>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QCommandLineParser>

#include "server.h"
#include "../version/buildnumber.h"










Server::Server(QObject *parent) : QObject(parent)
{
	m_serverVersionMajor = _VERSION_MAJOR;
	m_serverVersionMinor = _VERSION_MINOR;

	m_isHostForced = false;
	m_isPortForced = false;

	m_socketServer = nullptr;

	m_sqlDbFile = "";
	m_sqlDbCreate = false;
	m_readyToStart = true;
	m_db = QSqlDatabase::addDatabase("QSQLITE");
	m_dbVersionMajor = 0;
	m_dbVersionMinor = 0;
	m_dbSocketPort = 0;
	m_socketPendingConnections = 30;
}


/**
 * @brief Server::~Server
 */

Server::~Server()
{
	if (m_socketServer) {
		m_socketServer->close();
		//qDeleteAll(m_clients.begin(), m_clients.end());
		delete m_socketServer;
	}
}



/**
 * @brief Server::start
 * @return
 */

void Server::start()
{
	databaseOpen();
	if (readyToStart())	databaseLoad();
	if (readyToStart()) websocketServerStart();
}

void Server::stop()
{
	m_db.close();
}



/**
 * @brief Server::parseCommandLine
 */

void Server::commandLineParse(QCoreApplication &app)
{
	QCoreApplication::instance()->setApplicationName(QString::fromUtf8("Call of Suli szerver"));
	QCoreApplication::instance()->setOrganizationDomain("server.callofsuli.vjp.piarista.hu");
	QCoreApplication::instance()->setOrganizationName("Call of Suli");
	QCoreApplication::instance()->setApplicationVersion(_VERSION_FULL);

	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::instance()->applicationName()+
									 QStringLiteral(" – Copyright © 2012-2020 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();


	parser.addPositionalArgument("db", "Az adatbázis fájl");


	QCommandLineOption dbCreate(QStringList() << "i" << "initialize",
								QStringLiteral("Új adatbázis előkészítése"));
	parser.addOption(dbCreate);

	QCommandLineOption certFile(QStringList() << "c" << "certificate",
								QStringLiteral("Tanúsítvány az SSL kapcsolathoz"),
								QStringLiteral("cert"));
	parser.addOption(certFile);

	QCommandLineOption keyFile(QStringList() << "k" << "key",
							   QStringLiteral("Kulcs az SSL kapcsolathoz"),
							   QStringLiteral("key"));
	parser.addOption(keyFile);

	QCommandLineOption host(QStringList() << "H" << "host",
							QStringLiteral("Host"),
							QStringLiteral("host"));
	parser.addOption(host);

	QCommandLineOption port(QStringList() << "p" << "port",
							QStringLiteral("Port"),
							QStringLiteral("port"));
	parser.addOption(port);

	QCommandLineOption pend(QStringList() << "P" << "pending-connections",
							QStringLiteral("Max. pending connections"),
							QStringLiteral("num"));
	parser.addOption(pend);




	parser.process(app);

	QStringList args = parser.positionalArguments();

	setSqlDbFile(args.value(0));

	if (parser.isSet(dbCreate)) setSqlDbCreate(parser.isSet(dbCreate));
	if (parser.isSet(certFile))	setSocketCertFile(parser.value(certFile));
	if (parser.isSet(keyFile))	setSocketCertKey(parser.value(keyFile));

	if (parser.isSet(host)) {
		setDbSocketHost(parser.value(host));
		m_isHostForced = true;
	}
	if (parser.isSet(port)) {
		setDbSocketPort(parser.value(port).toInt());
		m_isPortForced = true;
	}
	if (parser.isSet(pend)) setSocketPendingConnections(parser.value(pend).toInt());
}



/**
 * @brief Server::databaseOpen
 */

void Server::databaseOpen()
{
	if (sqlDbFile().isEmpty()) {
		qCritical() << "Nincs megadva adatbázis!";
		setReadyToStart(false);
	}


	m_db.setDatabaseName(sqlDbFile());

	if (!QFile::exists(sqlDbFile()) && !sqlDbCreate()) {
		qCritical() << "Nem létezik az adatbázis:" << sqlDbFile();
		setReadyToStart(false);
		return;
	}

	if (!m_db.open()) {
		qCritical() << "Nem sikerült megnyitni az adatbázist:" << m_db.databaseName();
		setReadyToStart(false);
		return;
	}

	QSqlQuery q;
	if (!q.exec("PRAGMA foreign_keys = ON")) {
		qCritical() << "Nem sikerült megnyitni az adatbázist:" << m_db.databaseName();
		setReadyToStart(false);
		return;
	}

	qInfo() << "Adatbázis megnyitva:" << m_db.databaseName();
}


/**
 * @brief Server::databaseLoad
 */

void Server::databaseLoad()
{
	QSqlQuery q;
	bool isFirst = true;

tryAgain:
	q.clear();
	q.exec("SELECT versionMajor, versionMinor, socketHost, socketPort, serverName text from system");
	if (q.first()) {
		setDbVersionMajor(q.value(0).toInt());
		setDbVersionMinor(q.value(1).toInt());
		if (!m_isHostForced) setDbSocketHost(q.value(2).toString());
		if (!m_isPortForced) setDbSocketPort(q.value(3).toInt());
		setDbServerName(q.value(4).toString());
	} else if (!sqlDbCreate() || !isFirst) {
		qCritical() << "Az adatbázis üres vagy hibás!";
		setReadyToStart(false);
		return;
	} else {
		qInfo() << "Az adatbázis üres vagy hibás, előkészítem.";

		isFirst = false;

		QStringList commands;
		commands << "CREATE TABLE system(versionMajor integer, versionMinor integer, socketHost text, socketPort integer, serverName text)";
		commands << QString("INSERT INTO system(versionMajor, versionMinor, socketHost, socketPort, serverName) values (%1, %2, '%3', %4, '%5')")
					.arg(serverVersionMajor())
					.arg(serverVersionMinor())
					.arg("")
					.arg(10101)
					.arg("-- my Call of Suli server --");

		foreach (QString c, commands) {
			QSqlQuery q;
			if (!q.exec(c)) {
				qWarning() << "DB error:" << q.lastError().text() << c;
				setReadyToStart(false);
				return;
			}
		}

		qInfo() << "Adatbázis előkészítve.";

		goto tryAgain;
	}

	qInfo() << "Adatbázis betöltve:" << dbServerName();
}



/**
 * @brief Server::databaseInit
 */

void Server::databaseInit()
{
	/*
	foreach (QString f, files) {
					qDebug() << "load "+f;
					QFile file(f);
					file.open(QFile::ReadOnly);
					qExec(QString::fromUtf8(file.readAll()));
					q.clear();
			}
*/
}





/**
 * @brief Server::websocketServerStart
 */

void Server::websocketServerStart()
{
	if (!socketCertFile().isEmpty()) {
		if (!QSslSocket::supportsSsl()) {
			qCritical("Platform doesn't support SSL");
			setReadyToStart(false);
			return;
		}


		m_socketServer = new QWebSocketServer("CallOfSuli server", QWebSocketServer::SecureMode);

		QString base;

		if (QDir::isAbsolutePath(socketCertKey()))
			base = "";
		else {
			base = QDir::currentPath()+"/";
		}


		QFile certFile(base+socketCertFile());
		QFile keyFile(base+socketCertKey());

		certFile.open(QIODevice::ReadOnly);
		keyFile.open(QIODevice::ReadOnly);

		QSslConfiguration config;
		config.setLocalCertificate(QSslCertificate(&certFile, QSsl::Pem));
		config.setPrivateKey(QSslKey(&keyFile, QSsl::Rsa, QSsl::Pem));
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		m_socketServer->setSslConfiguration(config);

		certFile.close();
		keyFile.close();

	}
	else {
		m_socketServer = new QWebSocketServer("CallOfSuli server", QWebSocketServer::NonSecureMode);
	}


	m_socketServer->setMaxPendingConnections(socketPendingConnections());



	connect(m_socketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	connect(m_socketServer, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)));

	quint16 port = (dbSocketPort()<=0) ? 1024 : quint16(dbSocketPort());
	if (m_socketServer->listen(dbSocketHost().isEmpty() ? QHostAddress::Any : QHostAddress(dbSocketHost()), port))
	{
		qInfo() << "Listening on port" << port
				<< (m_socketServer->secureMode() == QWebSocketServer::SecureMode ? "SSL" : "");
	} else {
		qCritical("Cannot listen on host %s and port %d", dbSocketHost().toStdString().data(), port);
		setReadyToStart(false);
		return;
	}

	qInfo("Maximum pending connections: %d", m_socketServer->maxPendingConnections());

}



/**
 * @brief Server::onSslErrors
 * @param errors
 */

void Server::onSslErrors(const QList<QSslError> &errors)
{
	qWarning() << "SSL error" << errors;
}



/**
 * @brief Server::onNewConnection
 */

void Server::onNewConnection()
{
	QWebSocket *pSocket = m_socketServer->nextPendingConnection();
/*	COSsocketHandler *socketHandler = new COSsocketHandler(pSocket, this);

	connect(socketHandler, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

	m_clients << socketHandler; */
}



/*void COSsocket::onSocketDisconnected()
{
	COSsocketHandler *pClient = qobject_cast<COSsocketHandler *>(sender());
	if (pClient)
	{
		m_clients.removeAll(pClient);
		pClient->deleteLater();
	}
}*/





/**
 * @brief Server::setSqlDbFile
 * @param sqlDbFile
 */

void Server::setSqlDbFile(QString sqlDbFile)
{
	if (m_sqlDbFile == sqlDbFile)
		return;

	m_sqlDbFile = sqlDbFile;
	emit sqlDbFileChanged(m_sqlDbFile);
}

void Server::setReadyToStart(bool readyToStart)
{
	if (m_readyToStart == readyToStart)
		return;

	m_readyToStart = readyToStart;
	emit readyToStartChanged(m_readyToStart);
}

void Server::setSqlDbCreate(bool sqlDbCreate)
{
	if (m_sqlDbCreate == sqlDbCreate)
		return;

	m_sqlDbCreate = sqlDbCreate;
	emit sqlDbCreateChanged(m_sqlDbCreate);
}

void Server::setDbVersionMajor(int dbVersionMajor)
{
	if (m_dbVersionMajor == dbVersionMajor)
		return;

	m_dbVersionMajor = dbVersionMajor;
	emit dbVersionMajorChanged(m_dbVersionMajor);
}

void Server::setDbVersionMinor(int dbVersionMinor)
{
	if (m_dbVersionMinor == dbVersionMinor)
		return;

	m_dbVersionMinor = dbVersionMinor;
	emit dbVersionMinorChanged(m_dbVersionMinor);
}

void Server::setDbSocketHost(QString dbSocketHost)
{
	if (m_dbSocketHost == dbSocketHost)
		return;

	m_dbSocketHost = dbSocketHost;
	emit dbSocketHostChanged(m_dbSocketHost);
}

void Server::setDbSocketPort(int dbSocketPort)
{
	if (m_dbSocketPort == dbSocketPort)
		return;

	m_dbSocketPort = dbSocketPort;
	emit dbSocketPortChanged(m_dbSocketPort);
}

void Server::setDbServerName(QString dbServerName)
{
	if (m_dbServerName == dbServerName)
		return;

	m_dbServerName = dbServerName;
	emit dbServerNameChanged(m_dbServerName);
}

void Server::setSocketCertFile(QString socketCertFile)
{
	if (m_socketCertFile == socketCertFile)
		return;

	m_socketCertFile = socketCertFile;
	emit socketCertFileChanged(m_socketCertFile);
}

void Server::setSocketCertKey(QString socketCertKey)
{
	if (m_socketCertKey == socketCertKey)
		return;

	m_socketCertKey = socketCertKey;
	emit socketCertKeyChanged(m_socketCertKey);
}

void Server::setSocketPendingConnections(int socketPendingConnections)
{
	if (m_socketPendingConnections == socketPendingConnections)
		return;

	m_socketPendingConnections = socketPendingConnections;
	emit socketPendingConnectionsChanged(m_socketPendingConnections);
}

