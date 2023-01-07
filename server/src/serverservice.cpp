/*
 * ---- Call of Suli ----
 *
 * serverservice.cpp
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerService
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "serverservice.h"
#include "../../version/version.h"

#include "qcommandlineparser.h"
#include <QtService/Terminal>
#include <QCoreApplication>
#include <RollingFileAppender.h>
#include <ColorConsoleAppender.h>
#include "qconsole.h"
#include "serverhandler.h"
#include "utils.h"


const int ServerService::m_versionMajor = VERSION_MAJOR;
const int ServerService::m_versionMinor = VERSION_MINOR;
const int ServerService::m_versionBuild = VERSION_BUILD;
const char *ServerService::m_version = VERSION_FULL;



using namespace QtService;



/**
 * @brief ServerService::ServerService
 * @param argc
 * @param argv
 */

ServerService::ServerService(int &argc, char **argv)
	: Service(argc, argv)
	, m_settings(new ServerSettings())
{
	m_consoleAppender = new ColorConsoleAppender;

#ifndef QT_NO_DEBUG
	m_consoleAppender->setFormat(QString::fromStdString(
									 "%{time}{hh:mm:ss} %{category} [%{TypeOne}] %{message} "+
									 ColorConsoleAppender::reset+ColorConsoleAppender::green+"<%{function} "+
									 ColorConsoleAppender::magenta+"%{file}:%{line}"+
									 ColorConsoleAppender::green+">\n"));
#else
	m_consoleAppender->setFormat(QString::fromStdString("%{time}{hh:mm:ss} %{category} [%{TypeOne}] %{message}\n"));
#endif

	cuteLogger->registerAppender(m_consoleAppender);

	for (int i=0; i<argc; ++i) {
		m_arguments.append(argv[i]);
	}

	LOG_CTRACE("service") << "Server service created";
}


/**
 * @brief ServerService::~ServerService
 */

ServerService::~ServerService()
{
	delete m_settings;

	LOG_CTRACE("service") << "Server service destroyed";
}


/**
 * @brief ServerService::initialize
 */

void ServerService::initialize()
{
	QCoreApplication::setApplicationName(QStringLiteral("callofsuli-server"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("callofsuli-server"));
	QCoreApplication::setApplicationVersion(m_version);

	QCoreApplication::addLibraryPath(QStringLiteral("lib/QtService/plugins"));

	cuteLogger->logToGlobalInstance(QStringLiteral("service"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("db"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("websocket"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("credential"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("logger"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("oauth2"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("client"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("qt.service.plugin.standard.backend"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("qt.service.service"), true);
}







/*
 * @brief ServerService::onStart
 * @return
 */

Service::CommandResult ServerService::onStart()
{
	LOG_CTRACE("service") << "Server service started";

	m_settings->printConfig();


	m_databaseMain = new DatabaseMain(this);
	m_databaseMain->setDbFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("main.db")));
	m_databaseMain->databaseOpen(m_databaseMain->dbFile());

	if (!m_databaseMain->databasePrepare()) {
		LOG_CERROR("service") << "Main database prepare error";
		quit();
		return CommandResult::Failed;
	}

	m_config.m_service = this;
	m_config.loadFromDb(m_databaseMain);

	m_webSocketServer = new WebSocketServer(m_settings->ssl() ? QWebSocketServer::SecureMode : QWebSocketServer::NonSecureMode, this);
	m_webSocketServer->start();


	m_googleOAuth2Authenticator = new GoogleOAuth2Authenticator(this);

	if (!m_googleOAuth2Authenticator->listen()) {
		LOG_CERROR("service") << "OAuth2Authenticator listening error";
		quit();
		return CommandResult::Failed;
	}

	m_verifier = jwt::verify()
			.allow_algorithm(jwt::algorithm::hs256{m_settings->jwtSecret().toStdString()})
			.with_issuer(JWT_ISSUER);


	LOG_CINFO("service") << "Server service started successfull";

	return CommandResult::Completed;
}


/**
 * @brief ServerService::onStop
 * @param exitCode
 * @return
 */

QtService::Service::CommandResult ServerService::onStop(int &exitCode)
{
	LOG_CINFO("service") << "Server service stopped with code:" << exitCode;

	qDeleteAll(m_clients);

	if (m_googleOAuth2Authenticator)
		delete m_googleOAuth2Authenticator;

	if (m_webSocketServer) {
		m_webSocketServer->close();
		delete m_webSocketServer;
	}

	if (m_databaseMain) {
		m_databaseMain->databaseClose();
		delete m_databaseMain;
	}


	return CommandResult::Completed;
}


/**
 * @brief ServerService::onReload
 * @return
 */

Service::CommandResult ServerService::onReload()
{
	LOG_CINFO("service") << "Server service reloaded";

	if (m_webSocketServer) {
		m_webSocketServer->close();
	}

	m_databaseMain->databaseClose();

	m_databaseMain->databaseOpen(m_databaseMain->dbFile());

	return CommandResult::Completed;
}


/**
 * @brief ServerService::onPause
 * @return
 */

Service::CommandResult ServerService::onPause()
{
	LOG_CINFO("service") << "Server service paused";

	if (m_webSocketServer)
		m_webSocketServer->pauseAccepting();

	return CommandResult::Completed;
}


/**
 * @brief ServerService::onResume
 * @return
 */

Service::CommandResult ServerService::onResume()
{
	LOG_CINFO("service") << "Server service resumed";

	if (m_webSocketServer)
		m_webSocketServer->resumeAccepting();

	return CommandResult::Completed;
}


/**
 * @brief ServerService::onConfigChanged
 */

void ServerService::onConfigChanged()
{
	sendToClients(WebSocketMessage::createEvent(ServerHandler::_getConfig(this)));
}


/**
 * @brief ServerService::config
 * @return
 */

ServerConfig &ServerService::config()
{
	return m_config;
}


/**
 * @brief ServerService::sendToClients
 * @param message
 */

void ServerService::sendToClients(const WebSocketMessage &message) const
{
	LOG_CTRACE("service") << "Send to all clients:" << message;
	foreach (Client *c, m_clients) {
		if (c)
			c->send(message);
	}
}


/**
 * @brief ServerService::sendToClients
 * @param roles
 * @param message
 */

void ServerService::sendToClients(const Credential::Roles &roles, const WebSocketMessage &message) const
{
	LOG_CTRACE("service") << "Send to clients with roles" << roles << "message:" << message;

	foreach (Client *c, m_clients) {
		if (c && ((c->credential().roles() & roles) != 0))
			c->send(message);
	}
}


/**
 * @brief ServerService::verifier
 * @return
 */

JwtVerifier &ServerService::verifier()
{
	return m_verifier;
}

const QString &ServerService::serverName() const
{
	return m_serverName;
}

void ServerService::setServerName(const QString &newServerName)
{
	if (m_serverName == newServerName)
		return;
	m_serverName = newServerName;
	emit serverNameChanged();
}


/**
 * @brief ServerService::googleOAuth2Authenticator
 * @return
 */

GoogleOAuth2Authenticator *ServerService::googleOAuth2Authenticator() const
{
	return m_googleOAuth2Authenticator.data();
}


/**
 * @brief ServerService::clients
 * @return
 */

const QVector<QPointer<Client> > &ServerService::clients() const
{
	return m_clients;
}


/**
 * @brief ServerService::webSocketServer
 * @return
 */

WebSocketServer *ServerService::webSocketServer() const
{
	return m_webSocketServer.data();
}


/**
 * @brief ServerService::addWebSocketClient
 * @param client
 */

void ServerService::clientAdd(Client *client)
{
	if (!client)
		return;

	m_clients.append(client);
}


/**
 * @brief ServerService::webSocketClientRemove
 * @param client
 */

void ServerService::clientRemove(Client *client)
{
	m_clients.removeAll(client);
	client->deleteLater();
}


/**
 * @brief ServerService::databaseMain
 * @return
 */

DatabaseMain *ServerService::databaseMain() const
{
	return m_databaseMain.data();
}



/**
 * @brief ServerService::preStart
 * @return
 */

bool ServerService::preStart()
{
	m_settings->setDataDir(QCoreApplication::applicationDirPath());

	QCommandLineParser parser;
	parser.setApplicationDescription(QString::fromUtf8(QByteArrayLiteral("Call of Suli szerver – Copyright © 2012-2023 Valaczka János Pál")));

	auto helpOption = parser.addHelpOption();
	auto versionOption = parser.addVersionOption();

	parser.addOption({QStringLiteral("license"), QObject::tr("Licensz")});
	//parser.addOption({{QStringLiteral("l"), QStringLiteral("log")}, QObject::tr("Naplózás <file> fájlba"), QStringLiteral("file")});
	//parser.addOption({{QStringLiteral("n"), QStringLiteral("log-limit")}, QObject::tr("Maximum <db> log fájl tárolása"), QStringLiteral("db")});
	parser.addOption({{QStringLiteral("d"), QStringLiteral("dir")}, QObject::tr("Adatbázis könyvtár"), QStringLiteral("database-directory")});


#ifdef QT_DEBUG
	parser.addOption({QStringLiteral("trace"), QObject::tr("Trace üzenetek megjelenítése")});
#else
	parser.addOption({QStringLiteral("debug"), QObject::tr("Debug üzenetek megjelenítése")});
#endif

	parser.parse(m_arguments);


	if (parser.isSet(helpOption)) {
		parser.showHelp(0);
		return false;
	}

	if (parser.isSet(versionOption)) {
		parser.showVersion();
		return false;
	}


	if (parser.isSet(QStringLiteral("license"))) {
		QByteArray b = Utils::fileContent(QStringLiteral(":/license.txt"));

		QConsole::qStdOut()->write(b);

		std::exit(0);
		return false;
	}

	if (parser.isSet(QStringLiteral("dir"))) {
		m_settings->setDataDir(parser.value(QStringLiteral("dir")));
	} else {
		LOG_CERROR("service") << "You must specify main data directory";

		std::exit(1);
		return false;
	}

#ifdef QT_DEBUG
	if (parser.isSet(QStringLiteral("trace")))
		m_consoleAppender->setDetailsLevel(Logger::Trace);
	else
		m_consoleAppender->setDetailsLevel(Logger::Debug);

#else
	if (parser.isSet(QStringLiteral("debug")))
		m_consoleAppender->setDetailsLevel(Logger::Debug);
	else
		m_consoleAppender->setDetailsLevel(Logger::Info);
#endif


	/*QString logFile;

	if (parser.isSet(QStringLiteral("log"))) logFile = parser.value(QStringLiteral("log"));

	int logLimit = 12;

	if (parser.isSet(QStringLiteral("log-limit"))) logLimit = parser.value(QStringLiteral("log-limit")).toInt();

	if (!logFile.isEmpty()) {
		RollingFileAppender* appender = new RollingFileAppender(logFile);
		appender->setFormat(QStringLiteral("%{time}{hh:mm:ss} [%{TypeOne}] %{category} %{message}\n"));
		appender->setDatePattern(RollingFileAppender::WeeklyRollover);
		appender->setLogFilesLimit(logLimit);
		cuteLogger->registerAppender(appender);
	}*/


	m_settings->loadFromFile();

	if (m_settings->generateJwtSecret())
		m_settings->saveToFile(true);

	return true;
}


/**
 * @brief ServerService::settings
 * @return
 */

ServerSettings *ServerService::settings() const
{
	return m_settings;
}

/**
 * @brief ServerService::version
 * @return
 */

const char *ServerService::version() const
{
	return m_version;
}

int ServerService::versionBuild()
{
	return m_versionBuild;
}

int ServerService::versionMinor()
{
	return m_versionMinor;
}

int ServerService::versionMajor()
{
	return m_versionMajor;
}


/**
 * @brief ServerConfig::set
 * @param key
 * @param value
 */


void ServerConfig::set(const char *key, const QJsonValue &value)
{
	m_data.insert(key, value);
	if (m_db) m_db->saveConfig(m_data);
	if (m_service) emit m_service->configChanged();
}



/**
 * @brief ServerConfig::loadFromDb
 * @param db
 */

void ServerConfig::loadFromDb(DatabaseMain *db)
{
	Q_ASSERT(db);

	LOG_CTRACE("db") << "Load config from database";

	m_db = db;

	QDefer ret;

	m_db->worker()->execInThread([ret, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(m_db->dbName());

		QMutexLocker(m_db->mutex());

		QSqlQuery q(db);
		q.prepare("SELECT config FROM system");

		if (!q.exec() || !q.first()) {
			ret.reject();
			return;
		}

		const QString &s = q.value(QStringLiteral("config")).toString();

		if (s.isEmpty() || !s.startsWith('{'))
			m_data = QJsonObject();
		else
			m_data = Utils::byteArrayToJsonObject(s.toUtf8());

		if (m_service) emit m_service->configChanged();

		ret.resolve();
	});

	QDefer::await(ret);
}
