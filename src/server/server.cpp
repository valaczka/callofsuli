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
#include <QCryptographicHash>

#include "server.h"
#include "../version/buildnumber.h"










Server::Server(QObject *parent) : QObject(parent)
{
	m_serverVersionMajor = _VERSION_MAJOR;
	m_serverVersionMinor = _VERSION_MINOR;

	m_isHostForced = false;
	m_isPortForced = false;
	m_isConnectionForced = false;

	m_socketServer = nullptr;

	m_sqlDbDir = "";
	m_db = new CosSql("mainDB", this);
	m_mapDb = new MapRepository("mapDB", this);


	m_sqlDbCreate = false;
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
		delete m_socketServer;
	}

	qDeleteAll(m_clients.begin(), m_clients.end());

	delete m_mapDb;
	delete m_db;
}



/**
 * @brief Server::start
 * @return
 */

bool Server::start()
{
	if (!dbDirCheck(sqlDbDir(), sqlDbCreate()))
		return false;

	QString file = QDir::toNativeSeparators(sqlDbDir()+"/main.cosdb");

	if (!m_db->open(file, sqlDbCreate()))
		return false;

	if (!databaseLoad())
		return false;

	resourcesLoad();


	m_mapDb->setDatabaseFile(QDir::toNativeSeparators(sqlDbDir()+"/maps.cosdb"));

	if (!m_mapDb->databaseOpen())
		return false;


	if (!websocketServerStart())
		return false;

	return true;
}

void Server::stop()
{
	m_db->close();
}



/**
 * @brief Server::parseCommandLine
 */

void Server::commandLineParse(QCoreApplication &app)
{
	QCoreApplication::instance()->setApplicationName("callofsuli-server");
	QCoreApplication::instance()->setOrganizationDomain("server.callofsuli.vjp.piarista.hu");
	//QCoreApplication::instance()->setOrganizationName("Call of Suli");
	QCoreApplication::instance()->setApplicationVersion(_VERSION_FULL);

	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::instance()->applicationName()+
									 QStringLiteral(" – Copyright © 2012-2020 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();


	parser.addPositionalArgument("dir", "Az adatbázisokat tartalmazó könyvtár");


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

	QCommandLineOption port(QStringList() << "P" << "port",
							QStringLiteral("Port"),
							QStringLiteral("port"));
	parser.addOption(port);

	QCommandLineOption pend(QStringList() << "p" << "pending-connections",
							QStringLiteral("Max. pending connections"),
							QStringLiteral("num"));
	parser.addOption(pend);




	parser.process(app);

	QStringList args = parser.positionalArguments();

	setSqlDbDir(args.value(0));

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
	if (parser.isSet(pend)) {
		setSocketPendingConnections(parser.value(pend).toInt());
		m_isConnectionForced = true;
	}
}


/**
 * @brief Server::dbDirCheck
 * @return
 */

bool Server::dbDirCheck(const QString &dirname, bool create)
{
	if (dirname.isEmpty()) {
		qWarning().noquote() << tr("Nincs megadva adatbáziskönyvtár!");
		return false;
	}

	QFileInfo f(dirname);

	if (!f.isDir() || !f.isReadable()) {
		if (create) {
			QDir d;
			if (d.mkdir(dirname)) {
				qDebug().noquote() << tr("Az adatbáziskönyvtár létrehozva: ")+dirname;
			} else {
				qWarning().noquote() << tr("Nem sikerült létrehozni az adatbáziskönyvtárt: ")+dirname;
				return false;
			}
		} else {
			qWarning().noquote() << tr("A megadott könyvtár nem létezik vagy nem olvasható: ")+dirname;
			return false;
		}
	}

	return true;
}





/**
 * @brief Server::databaseLoad
 */

bool Server::databaseLoad()
{
	bool isFirst=true;

	while (true) {
		QVariantMap r = m_db->runSimpleQuery("SELECT versionMajor, versionMinor, socketHost, socketPort, serverName, connections from system");

		if (r["error"].toBool() || r["records"].toList().isEmpty()) {
			if (isFirst) {
				qInfo().noquote() << tr("Az adatbázis üres vagy hibás, előkészítem...");

				if (!m_db->batchQueryFromFile(":/sql/init.sql") || !databaseInit()) {
					if (m_db->dbCreated()) {
						QString dbname = m_db->db().databaseName();
						qDebug().noquote() << tr("Az adatbázis félkész, törlöm: ")+dbname;
						m_db->close();
						if (!QFile::remove(dbname)) {
							qWarning().noquote() << tr("Nem sikerült törölni a hibás adatbázist: ")+dbname;
						}
					}
					return false;
				}
			} else {
				qWarning().noquote() << tr("Nem sikerült előkészíteni az adatbázist!");
				return false;
			}
		} else {
			QVariantMap rr = r["records"].toList().value(0).toMap();
			setDbServerName(rr["serverName"].toString());
			setDbVersionMajor(rr["versionMajor"].toInt());
			setDbVersionMinor(rr["versionMinor"].toInt());
			if (!m_isHostForced) setDbSocketHost(rr["socketHost"].toString());
			if (!m_isPortForced) setDbSocketPort(rr["socketPort"].toInt());
			int conn;
			if (!m_isConnectionForced)
				conn = rr["connections"].toInt();
			else
				conn = socketPendingConnections();

			if (conn>0)
				setSocketPendingConnections(conn);
			else
				setSocketPendingConnections(30);


			QVariantMap p;
			if (m_isHostForced)
				p["socketHost"] = m_dbSocketHost;
			if (m_isPortForced)
				p["socketPort"] = m_dbSocketPort;
			if (m_isConnectionForced)
				p["connections"] = m_socketPendingConnections;

			if (p.count()) {
				m_db->execUpdateQuery("UPDATE system SET ?", p);
			}

			break;
		}

		isFirst=false;
	}

	qInfo().noquote() << tr("Az adatbázis betöltve: ")+dbServerName();

	return true;
}



/**
 * @brief Server::databaseInit
 */

bool Server::databaseInit()
{
	QVariantMap params;

	params["versionMajor"] = serverVersionMajor();
	params["versionMinor"] = serverVersionMinor();
	params["socketHost"] = dbSocketHost();
	params["socketPort"] = dbSocketPort();
	params["serverName"] = tr("-- új Call of Suli szerver --");

	if (m_db->execInsertQuery("INSERT INTO system(?k?) values (?)", params)==-1)
		return false;



	params.clear();
	params["username"] = "admin";
	params["firstname"] = tr("Adminisztrátor");
	params["active"] = true;
	params["isTeacher"] = true;
	params["isAdmin"] = true;

	if (m_db->execInsertQuery("INSERT INTO user(?k?) values (?)", params)==-1)
		return false;




	QString salt;
	QString pwd = CosSql::hashPassword("admin", &salt);

	params.clear();
	params["username"] = "admin";
	params["password"] = pwd;
	params["salt"] = salt;

	if (m_db->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", params)==-1)
		return false;

	return true;
}


/**
 * @brief Server::resourcesLoad
 * @return
 */

bool Server::resourcesLoad()
{
	QString f = QDir::toNativeSeparators(sqlDbDir()+"/resources.cosdb");

	if (QFile::exists(f)) {
		QFile file(f);
		if (file.open(QIODevice::ReadOnly)) {
			QCryptographicHash hash(QCryptographicHash::Sha1);
			hash.addData(&file);
			QByteArray d = hash.result();
			file.close();

			setDbResources(f);
			setDbResourcesHash(d);

			qInfo().noquote() << tr("Erőforrásadatbázis betöltve: ")+dbResources();
			qDebug().noquote() << tr("Erőforrásh hash: ")+dbResourcesHash().toHex();
		} else {
			qWarning().noquote() << tr("Az erőforrásadtabázis nem olvasható: ")+f;
			return false;
		}
	} else {
		qDebug().noquote() << tr("Az erőforrásadtabázis nem létezik: ")+f;
		return false;
	}

	return true;
}





/**
 * @brief Server::websocketServerStart
 * @return
 */


bool Server::websocketServerStart()
{
	if (socketCertFile().isEmpty()) {
		QString f = QDir::toNativeSeparators(sqlDbDir()+"/sslcert.pem");

		if (QFile::exists(f)) {
			qInfo().noquote() << tr("SSL tanúsítvány automatikus beállítása: ") << f;
			setSocketCertFile(f);
		}
	}

	if (socketCertKey().isEmpty()) {
		QString f = QDir::toNativeSeparators(sqlDbDir()+"/sslkey.pem");

		if (QFile::exists(f)) {
			qInfo().noquote() << tr("SSL kulcs automatikus beállítása: ") << f;
			setSocketCertKey(f);
		}
	}

	if (!socketCertFile().isEmpty() && !socketCertKey().isEmpty()) {
		if (!QSslSocket::supportsSsl()) {
			qCritical().noquote() << tr("Platform doesn't support SSL");
			return false;
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

	quint16 port = (dbSocketPort()<=0) ? 10101 : quint16(dbSocketPort());
	if (m_socketServer->listen(dbSocketHost().isEmpty() ? QHostAddress::Any : QHostAddress(dbSocketHost()), port))
	{
		qInfo().noquote() << tr("Listening on ")+m_socketServer->serverUrl().toString();
	} else {
		qCritical().noquote() << QString(tr("Cannot listen on host %1 and port %2")).arg(dbSocketHost().toStdString().data()).arg(port);
		return false;
	}

	qDebug("Maximum pending connections: %d", m_socketServer->maxPendingConnections());

	return true;
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

	Client *handler = new Client(m_db, pSocket);

	connect(handler, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));

	m_clients << handler;
}


/**
 * @brief Server::onSocketDisconnected
 */

void Server::onSocketDisconnected()
{
	Client *handler = qobject_cast<Client *>(sender());
	if (handler)
	{
		m_clients.removeAll(handler);
		handler->deleteLater();
	}
}





/**
	 * @brief Server::setSqlDbFile
	 * @param sqlDbFile
	 */

void Server::setSqlDbDir(QString sqlDbFile)
{
	if (m_sqlDbDir == sqlDbFile)
		return;

	m_sqlDbDir = sqlDbFile;
	emit sqlDbDirChanged(m_sqlDbDir);
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

void Server::setDbResources(QString dbResources)
{
	if (m_dbResources == dbResources)
		return;

	m_dbResources = dbResources;
	emit dbResourcesChanged(m_dbResources);
}

void Server::setDbResourcesHash(QByteArray dbResourcesHash)
{
	if (m_dbResourcesHash == dbResourcesHash)
		return;

	m_dbResourcesHash = dbResourcesHash;
	emit dbResourcesHashChanged(m_dbResourcesHash);
}

