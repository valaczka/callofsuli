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
#include <QFile>

#include "server.h"
#include "../version/buildnumber.h"







Server::Server(QObject *parent)
	: QObject(parent)
	, m_serverVersionMajor(_VERSION_MAJOR)
	, m_serverVersionMinor(_VERSION_MINOR)
{
	m_socketServer = nullptr;
	m_serverDir = "";
	m_db = new CosDb("serverDb", this);
	m_mapsDb = new CosDb("mapsDb", this);

	m_versionMajor = 0;
	m_versionMinor = 0;
	m_port = 10101;
	m_pendingConnections = 30;
	m_createDb = false;
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

	delete m_mapsDb;
	delete m_db;
}



/**
 * @brief Server::start
 * @return
 */

bool Server::start()
{
	if (!serverDirCheck())
		return false;

	m_db->setDatabaseName(m_serverDir+"/main.db");
	m_mapsDb->setDatabaseName(m_serverDir+"/maps.db");

	if (!databaseLoad())
		return false;

	if (!resourcesInit())
		return false;

	reloadResources();

	if (!websocketServerStart())
		return false;

	return true;
}


/**
 * @brief Server::stop
 */

void Server::stop()
{
	m_db->close();
}



/**
 * @brief Server::parseCommandLine
 */

bool Server::commandLineParse(QCoreApplication &app)
{
	QCoreApplication::instance()->setApplicationName("callofsuli-server");
	QCoreApplication::instance()->setOrganizationDomain("server.callofsuli.vjp.piarista.hu");
	QCoreApplication::instance()->setApplicationVersion(_VERSION_FULL);

	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::instance()->applicationName()+
									 QString::fromUtf8(" – Copyright © 2012-2021 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addPositionalArgument("dir", "Az adatbázisokat tartalmazó könyvtár");

	parser.addOption({{"i", "initialize"}, QString::fromUtf8("Új adatbázis előkészítése")});
	parser.addOption({{"c", "certificate"}, QString::fromUtf8("Tanúsítvány az SSL kapcsolathoz <file>"), "file"});
	parser.addOption({{"k", "key"}, QString::fromUtf8("Titkos kulcs az SSL kapcsolathoz <file>"), "file"});
	parser.addOption({{"H", "host"}, QString::fromUtf8("A szerver <ip> címe"), "ip"});
	parser.addOption({{"P", "port"}, QString::fromUtf8("A szerver <num> portja"), "num"});
	parser.addOption({{"p", "pending"}, QString::fromUtf8("Maximum <num> pending-connections"), "num"});
	parser.addOption({{"w", "write"}, QString::fromUtf8("Beállítások mentése")});

	parser.process(app);

	QStringList args = parser.positionalArguments();

	setServerDir(args.value(0));

	if (parser.isSet("initialize")) m_createDb = true;

	if (!serverDirCheck())
		return false;

	QSettings settings(m_serverDir+"/settings.ini", QSettings::IniFormat);
	if (settings.contains("host")) m_host = settings.value("host").toString();
	if (settings.contains("port")) m_port = settings.value("port").toInt();
	if (settings.contains("certificate")) m_socketCert = settings.value("certificate").toString();
	if (settings.contains("key")) m_socketKey = settings.value("key").toString();
	if (settings.contains("pendingConnections")) m_pendingConnections = settings.value("pendingConnections").toInt();

	if (parser.isSet("certificate")) m_socketCert = parser.value("certificate");
	if (parser.isSet("key")) m_socketKey = parser.value("key");
	if (parser.isSet("host")) m_host = parser.value("host");
	if (parser.isSet("port")) m_port = parser.value("port").toInt();
	if (parser.isSet("pending")) m_pendingConnections = parser.value("pending").toInt();

	if (parser.isSet("write")) {
		qInfo().noquote() << tr("Beállítások mentése");
		settings.setValue("host", m_host);
		settings.setValue("port", m_port);
		settings.setValue("certificate", m_socketCert);
		settings.setValue("key", m_socketKey);
		settings.setValue("pendingConnections", m_pendingConnections);
	}

	return true;
}


/**
 * @brief Server::dbDirCheck
 * @return
 */

bool Server::serverDirCheck()
{
	if (m_serverDir.isEmpty()) {
		qInfo().noquote() << tr("Nincs megadva adatbáziskönyvtár, az alapértelmezett lesz használva.");

		QString dir = QStandardPaths::standardLocations(QStandardPaths::DataLocation).value(0);

		setServerDir(dir);
		m_createDb = true;
	}

	QFileInfo f(m_serverDir);

	if (!f.isDir() && m_createDb) {
		QDir d;
		if (d.mkdir(m_serverDir)) {
			qDebug().noquote() << tr("Az adatbáziskönyvtár létrehozva: ")+m_serverDir;
			return true;
		} else {
			qWarning().noquote() << tr("Nem sikerült létrehozni az adatbáziskönyvtárt: ")+m_serverDir;
		}
	}

	if (f.isDir())
		return true;

	qWarning().noquote() << tr("A megadott könyvtár nem létezik vagy nem olvasható: ")+m_serverDir;
	return false;
}





/**
 * @brief Server::databaseLoad
 */

bool Server::databaseLoad()
{
	if (!m_db->open())
		return false;

	if (!m_mapsDb->open())
		return false;

	QVariantMap m = m_db->execSelectQueryOneRow("SELECT versionMajor, versionMinor, serverName from system");

	if (m.isEmpty()) {
		qWarning() << tr("Az adatbázis üres, előkészítem.");

		if (!m_db->batchQueryFromFile(":/sql/init.sql")) {
			qWarning().noquote() << tr("Nem sikerült előkészíteni az adatbázist: %1").arg(m_db->databaseName());
			return false;
		}

		QVariantMap params;
		params["versionMajor"] = m_serverVersionMajor;
		params["versionMinor"] = m_serverVersionMinor;
		params["serverName"] = tr("Call of Suli szerver");

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
		QString pwd = CosDb::hashPassword("admin", &salt);

		params.clear();
		params["username"] = "admin";
		params["password"] = pwd;
		params["salt"] = salt;

		if (m_db->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", params)==-1)
			return false;



#ifdef QT_DEBUG
		for (int i=0; i<5; i++) {
			QString username = QString("test%1").arg(i+1);
			qDebug() << "Create test user" << username;
			QVariantMap p;
			p["username"] = username;
			p["firstname"] = QString("Teszt %1 felhasználó").arg(i+1);
			p["active"] = true;
			p["isTeacher"] = false;
			p["isAdmin"] = false;

			if (m_db->execInsertQuery("INSERT INTO user(?k?) values (?)", p)==-1)
				return false;


			QString salt;
			QString pwd = CosDb::hashPassword(username, &salt);

			p.clear();
			p["username"] = username;
			p["password"] = pwd;
			p["salt"] = salt;

			if (m_db->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", p)==-1)
				return false;
		}
#endif


		if (!rankInit())
			return false;
	} else {
		setServerName(m.value("serverName").toString());
		m_versionMajor = m.value("versionMajor").toInt();
		m_versionMinor = m.value("versionMinor").toInt();
	}


	QVariantList tables = m_mapsDb->execSelectQuery("SELECT name FROM sqlite_master WHERE type ='table' AND name='maps'");

	if (tables.isEmpty()) {
		qWarning() << tr("A pályaadatbázis üres, előkészítem.");

		if (!m_mapsDb->batchQueryFromFile(":/sql/maps.sql")) {
			qWarning().noquote() << tr("Nem sikerült előkészíteni az adatbázist: %1").arg(m_mapsDb->databaseName());
			return false;
		}
	}

	m_db->subscribeToNotification("ranklog");

	qInfo().noquote() << tr("Az adatbázis betöltve: ")+m_serverName;

	return true;
}


/**
 * @brief Server::databaseInit
 * @return
 */

bool Server::rankInit()
{
	QStringList ranks;

	ranks << tr("közkatona")
		  << tr("őrvezető")
		  << tr("tizedes")
		  << tr("szakaszvezető")
		  << tr("őrmester")
		  << tr("törzsőrmester")
		  << tr("főtörzsőrmester")
		  << tr("zászlós")
		  << tr("törzszászlós")
		  << tr("főtörzszászlós")
		  << tr("alhadnagy")
		  << tr("hadnagy")
		  << tr("főhadnagy")
		  << tr("százados")
		  << tr("őrnagy")
		  << tr("alezredes")
		  << tr("ezredes")
		  << tr("dandártábornok")
		  << tr("vezérőrnagy")
		  << tr("altábornagy");


	const int max_level = 3;
	const int base_xp = 500;
	const qreal rank_factor_step = 0.15;
	qreal rank_factor = 1.0;
	int multiply = 0;

	for (int n=0; n<ranks.size(); n++) {
		QString rank = ranks.at(n);
		for (int i=1; i<=max_level; i++) {
			QVariantMap m;
			m["name"] = rank;
			m["level"] = i;
			m["image"] = QString("rank/%1.svg").arg(n);
			m["xp"] = (int) round(base_xp*rank_factor*multiply);

			m_db->execInsertQuery("INSERT INTO rank (?k?) VALUES (?)", m);

			multiply += 1+rank_factor_step;
		}
		rank_factor += rank_factor_step;
	}

	QVariantMap m;
	m["name"] = tr("vezérezredes");
	m["level"] = 1;
	m["image"] = "rank/100.svg";
	m["xp"] = QVariant::Invalid;

	m_db->execInsertQuery("INSERT INTO rank (?k?) VALUES (?)", m);

	return true;
}


/**
 * @brief Server::resourcesInit
 * @return
 */

bool Server::resourcesInit()
{
	m_resourceMap["images.db"] = ":/images/default.db";

	QMapIterator<QString, QVariant> i(m_resourceMap);
	while (i.hasNext()) {
		i.next();
		QString newName = m_serverDir+"/"+i.key();

		if (QFile::exists(newName))
			continue;

		qDebug().noquote() << tr("Adatbázis létrehozása:") << newName;
		if (!QFile::copy(i.value().toString(), newName)) {
			qWarning().noquote() << tr("Nem sikerült létrehozni az adatbázist:") << newName;
			return false;
		}
	}

	return true;
}











/**
 * @brief Server::websocketServerStart
 * @return
 */


bool Server::websocketServerStart()
{
	if (!m_socketCert.isEmpty() && !m_socketKey.isEmpty()) {
		if (!QSslSocket::supportsSsl()) {
			qCritical().noquote() << tr("Platform doesn't support SSL");
			return false;
		}

		m_socketServer = new QWebSocketServer("CallOfSuli server", QWebSocketServer::SecureMode);

		QString base;

		if (QDir::isAbsolutePath(m_socketKey))
			base = "";
		else {
			base = m_serverDir+"/";
		}


		QFile certFile(base+m_socketCert);
		QFile keyFile(base+m_socketKey);

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


	m_socketServer->setMaxPendingConnections(m_pendingConnections);


	connect(m_socketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	connect(m_socketServer, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)));

	quint16 port = (m_port<=0) ? 10101 : quint16(m_port);
	if (m_socketServer->listen(m_host.isEmpty() ? QHostAddress::Any : QHostAddress(m_host), port))
	{
		qInfo().noquote() << tr("Szerver elindult: ")+m_socketServer->serverUrl().toString();
	} else {
		qCritical().noquote() << QString(tr("Cannot listen on host %1 and port %2")).arg(m_host.toStdString().data()).arg(port);
		return false;
	}

	qDebug("Maximum pending connections: %d", m_socketServer->maxPendingConnections());

	return true;
}


/**
 * @brief Server::reloadResources
 */

void Server::reloadResources()
{
	QStringList files;

	QMapIterator<QString, QVariant> i(m_resourceMap);
	while (i.hasNext()) {
		i.next();
		files << i.key();
	}

	QDirIterator it(m_serverDir+"/", QStringList() << "*.cres");

	while (it.hasNext()) {
		it.next();
		if (!it.fileName().isEmpty())
			files << it.fileName();
	}

	QVariantMap map;

	foreach (QString f, files) {
		QFile ff(m_serverDir+"/"+f);
		if (!ff.open(QIODevice::ReadOnly)) {
			qWarning().noquote() << tr("Nem sikerült megnyitni a fájlt:") << m_serverDir+"/"+f;
			continue;
		}
		QByteArray data = ff.readAll();
		ff.close();

		QString md5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
		int size = data.size();

		qDebug().noquote() << tr("Fájl hozzáadva:") << f << md5 << size;

		QVariantMap d;
		d["size"] = size;
		d["md5"] = md5;

		map[f] = d;
	}

	setResources(map);
}


/**
 * @brief Server::resourceContent
 * @param filename
 * @return
 */

QByteArray Server::resourceContent(const QString &filename, QString *md5) const
{
	QByteArray ret;

	if (!m_resources.contains(filename)) {
		qWarning().noquote() << tr("Érvénytelen fájl:") << filename;
		return ret;
	}

	QFile f(m_serverDir+"/"+filename);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning().noquote() << tr("Nem sikerült megnyitni a fájlt:") << filename;
		return ret;
	}

	ret = f.readAll();

	f.close();

	if (md5) {
		*md5 = QCryptographicHash::hash(ret, QCryptographicHash::Md5).toHex();
	}

	return ret;
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

	Client *handler = new Client(pSocket, this, this);

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

void Server::setServerDir(QString serverDir)
{
	if (m_serverDir == serverDir)
		return;

	m_serverDir = serverDir;
	emit serverDirChanged(m_serverDir);
}

void Server::setServerName(QString serverName)
{
	if (m_serverName == serverName)
		return;

	m_serverName = serverName;
	emit serverNameChanged(m_serverName);
}

void Server::setResources(QVariantMap resources)
{
	if (m_resources == resources)
		return;

	m_resources = resources;
	emit resourcesChanged(m_resources);
}



