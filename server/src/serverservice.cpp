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
#include "terminalhandler.h"
#include "utils.h"
#include "googleoauth2authenticator.h"
#include "microsoftoauth2authenticator.h"
#include <QOAuthHttpServerReplyHandler>



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
	, m_networkManager(new QNetworkAccessManager(this))
{
	m_consoleAppender = new ColorConsoleAppender;

#ifndef QT_NO_DEBUG
	m_consoleAppender->setFormat(QString::fromStdString(
									 "%{time}{hh:mm:ss} %{category:-10} [%{TypeOne}] %{message} "+
									 ColorConsoleAppender::reset+ColorConsoleAppender::green+"<%{function} "+
									 ColorConsoleAppender::magenta+"%{file}:%{line}"+
									 ColorConsoleAppender::green+">\n"));
#else
	m_consoleAppender->setFormat(QString::fromStdString("%{time}{hh:mm:ss} %{category:-10} [%{TypeOne}] %{message}\n"));
#endif

	cuteLogger->registerAppender(m_consoleAppender);

	for (int i=0; i<argc; ++i) {
		m_arguments.append(argv[i]);
	}

	setTerminalActive(true);
	setTerminalMode(TerminalMode::ReadWriteActive);

	connect(&m_mainTimer, &QTimer::timeout, this, &ServerService::onMainTimerTimeout);
	m_mainTimer.setInterval(500);

	LOG_CTRACE("service") << "Server service created";
}


/**
 * @brief ServerService::~ServerService
 */

ServerService::~ServerService()
{
	delete m_settings;
	delete m_networkManager;

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
	cuteLogger->logToGlobalInstance(QStringLiteral("app"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("db"), true);
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

	// Load WASM

	if (!wasmLoad()) {
		quit();
		return CommandResult::Failed;
	}


	// Load main DB

	m_databaseMain = new DatabaseMain(this);
	m_databaseMain->setDbFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("main.db")));
	m_databaseMain->setDbMapsFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("maps.db")));
	m_databaseMain->setDbStatFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("stat.db")));


	if (QFile::exists(m_databaseMain->dbFile()) && !m_importDb.isEmpty()) {
		LOG_CERROR("service") << "Import error, main database already exists";
		quit();
		return CommandResult::Failed;
	}

	m_databaseMain->databaseOpen(m_databaseMain->dbFile());

	if (!m_databaseMain->databasePrepare(m_importDb)) {
		LOG_CERROR("service") << "Main database prepare error";
		quit();
		return CommandResult::Failed;
	}

	if (!m_databaseMain->databaseAttach()) {
		LOG_CERROR("service") << "Main database attach error";
		quit();
		return CommandResult::Failed;
	}

	m_config.m_service = this;
	m_config.loadFromDb(m_databaseMain);

	m_webSocketServer = new WebServer(this);
	m_webSocketServer->setRedirectHost(m_settings->redirectHost());
	m_webSocketServer->start();


	// Create authenticators

	for (auto it=m_settings->oauthMap().constBegin(); it != m_settings->oauthMap().constEnd(); ++it) {
		OAuth2Authenticator *authenticator = nullptr;

		if (it.key() == QStringLiteral("google"))
			authenticator = new GoogleOAuth2Authenticator(this);
		else if (it.key() == QStringLiteral("microsoft"))
			authenticator = new MicrosoftOAuth2Authenticator(this);

		if (authenticator) {
			authenticator->setOAuth(it.value());
			m_authenticators.append(authenticator);
		}
	}


	m_mainTimer.start();

	LOG_CINFO("service") << "Server service started successful";

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

	m_mainTimer.stop();

	qDeleteAll(m_authenticators);

	if (m_webSocketServer) {
		m_webSocketServer->server()->close();
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

	m_mainTimer.stop();

	if (m_webSocketServer)
		m_webSocketServer->server()->pauseAccepting();

	m_databaseMain->databaseClose();

	if (!wasmUnload()) {
		LOG_CINFO("invalid wasm unload");

		quit();
		return CommandResult::Failed;
	}

	m_databaseMain->databaseOpen(m_databaseMain->dbFile());

	if (!m_databaseMain->databaseAttach()) {
		quit();
		return CommandResult::Failed;
	}

	if (!wasmLoad()) {
		quit();
		return CommandResult::Failed;
	}

	if (m_webSocketServer)
		m_webSocketServer->server()->resumeAccepting();

	m_mainTimer.start();

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
		m_webSocketServer->server()->pauseAccepting();

	m_mainTimer.stop();

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
		m_webSocketServer->server()->resumeAccepting();

	m_mainTimer.start();

	return CommandResult::Completed;
}


/**
 * @brief ServerService::wasmLoad
 * @return
 */

bool ServerService::wasmLoad()
{
	QStringList wasmList;
	wasmList.append(QCoreApplication::applicationDirPath()+QStringLiteral("/share/wasm.rcc"));
	wasmList.append(QCoreApplication::applicationDirPath()+QStringLiteral("/../share/wasm.rcc"));
	wasmList.append(QCoreApplication::applicationDirPath()+QStringLiteral("/../../share/wasm.rcc"));

	auto it = wasmList.constBegin();

	for (; it != wasmList.constEnd(); ++it) {
		if (QFile::exists(*it)) {
			if (QResource::registerResource(*it, QStringLiteral("/wasm"))) {
				LOG_CDEBUG("service") << "Registering wasm resource:" << *it;
				m_loadedWasmResource = *it;
			} else {
				LOG_CERROR("service") << "Wasm resource register error:" << *it;
				return false;
			}
			break;
		}
	}

	if (it == wasmList.constEnd()) {
		LOG_CERROR("service") << "Wasm resource not found";
		return false;
	} else {
		return true;
	}
}


/**
 * @brief ServerService::wasmUnload
 * @return
 */

bool ServerService::wasmUnload()
{
	if (m_loadedWasmResource.isEmpty()) {
		LOG_CERROR("service") << "Invalid wasm resource";
		return false;
	}

	if (QResource::unregisterResource(m_loadedWasmResource, QStringLiteral("/wasm"))) {
		LOG_CDEBUG("service") << "Wasm resource unregistered:" << m_loadedWasmResource;
		return true;
	} else {
		LOG_CERROR("service") << "Wasm resource unregister failed:" << m_loadedWasmResource;
		return false;
	}

}


/**
 * @brief ServerService::terminalConnected
 * @param terminal
 */

void ServerService::terminalConnected(QtService::Terminal *terminal)
{
	LOG_CINFO("service") << "Terminal connected" << terminal;

	new TerminalHandler(this, terminal);
}



/**
 * @brief ServerService::imitateLatency
 * @return
 */

int ServerService::imitateLatency() const
{
	return m_imitateLatency;
}

void ServerService::setImitateLatency(int newImitateLatency)
{
	m_imitateLatency = newImitateLatency;
}


/**
 * @brief ServerService::importDb
 * @return
 */

const QString &ServerService::importDb() const
{
	return m_importDb;
}


/**
 * @brief ServerService::panels
 * @return
 */

QVector<Panel*> ServerService::panels() const
{
	QVector<Panel *> list;

	list.reserve(m_panels.size());

	foreach (Panel *p, m_panels)
		if (p)
			list.append(p);

	list.squeeze();

	return list;
}


/**
 * @brief ServerService::addPanel
 * @param panel
 */

void ServerService::addPanel(Panel *panel)
{
	if (!panel)
		return;

	int id = 1;

	foreach (Panel *p, m_panels)
		if (p && p->id() >= id)
			id = p->id()+1;

	panel->setId(id);

	m_panels.append(panel);
	connect(panel, &Panel::destroyed, this, [this, panel](){
		removePanel(panel, false);
	});
}


/**
 * @brief ServerService::removePanel
 * @param panel
 */

void ServerService::removePanel(Panel *panel, const bool &_delete)
{
	if (!panel)
		return;

	m_panels.removeAll(panel);

	if (_delete)
		panel->deleteLater();
}


/**
 * @brief ServerService::panel
 * @param uuid
 * @return
 */

Panel *ServerService::panel(const int &id) const
{
	foreach (Panel *p, m_panels)
		if (p && p->id() == id)
			return p;

	return nullptr;
}


/**
 * @brief ServerService::eventStreams
 * @return
 */

QVector<EventStream *> ServerService::eventStreams() const
{
	QVector<EventStream *> list;

	list.reserve(m_eventStreams.size());

	foreach (EventStream *s, m_eventStreams)
		if (s)
			list.append(s);

	list.squeeze();

	return list;
}



/**
 * @brief ServerService::addEventStream
 * @param stream
 */

void ServerService::addEventStream(EventStream *stream)
{
	if (!stream)
		return;

	m_eventStreams.append(stream);

	stream->setService(this);

	LOG_CTRACE("service") << "Event stream added:" << stream << stream->streamTypes();

	connect(stream, &EventStream::destroyed, this, [this, stream](){
		m_eventStreams.removeAll(stream);
		LOG_CTRACE("service") << "Event stream removed:" << stream << stream->streamTypes();
	});

	stream->HttpEventStream::write(QByteArrayLiteral("hello"), QByteArrayLiteral("hello message"));
}



/**
 * @brief ServerService::triggerEventStreams
 * @param type
 */

void ServerService::triggerEventStreams(const EventStream::EventStreamType &type)
{
	foreach (EventStream *s, m_eventStreams)
		if (s) s->trigger(type);
}


/**
 * @brief ServerService::triggerEventStreams
 * @param type
 * @param data
 */

void ServerService::triggerEventStreams(const EventStream::EventStreamType &type, const QVariant &data)
{
	foreach (EventStream *s, m_eventStreams)
		if (s) s->trigger(type, data);
}




/**
 * @brief ServerService::networkManager
 * @return
 */

QNetworkAccessManager *ServerService::networkManager() const
{
	return m_networkManager;
}


/**
 * @brief ServerService::authenticators
 * @return
 */

QVector<OAuth2Authenticator *> ServerService::authenticators() const
{
	QVector<OAuth2Authenticator *> list;

	list.reserve(m_authenticators.size());

	foreach (OAuth2Authenticator *a, m_authenticators)
		if (a)
			list.append(a);

	list.squeeze();

	return list;
}


/**
 * @brief ServerService::config
 * @return
 */

ServerConfig &ServerService::config()
{
	return m_config;
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
 * @brief ServerService::webSocketServer
 * @return
 */

WebServer *ServerService::webServer() const
{
	return m_webSocketServer.data();
}


/**
 * @brief ServerService::oauth2Authenticator
 * @param type
 * @return
 */

OAuth2Authenticator *ServerService::oauth2Authenticator(const char *type) const
{
	for (OAuth2Authenticator *a : m_authenticators) {
		if (strcmp(a->type(), type) == 0)
			return a;
	}

	return nullptr;
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
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);

	auto helpOption = parser.addHelpOption();
	auto versionOption = parser.addVersionOption();

	parser.addOption({QStringLiteral("license"), QObject::tr("Licensz")});
	parser.addOption({QStringLiteral("log"), QObject::tr("Naplózási limit (0 = nincs naplózás)"), QStringLiteral("num")});
	parser.addOption({{QStringLiteral("d"), QStringLiteral("dir")}, QObject::tr("Adatbázis könyvtár"), QStringLiteral("database-directory")});
	parser.addOption({{QStringLiteral("i"), QStringLiteral("import")}, QObject::tr("Adatbázis importálása"), QStringLiteral("database")});

	parser.addOption({QStringLiteral("terminal"), QObject::tr("Terminál indítása")});

	parser.addOption({{QStringLiteral("q"), QStringLiteral("quiet")}, QObject::tr("Csendes üzemmód")});
	parser.addOption({QStringLiteral("trace"), QObject::tr("Trace üzenetek megjelenítése")});

#ifndef QT_DEBUG
	parser.addOption({QStringLiteral("debug"), QObject::tr("Debug üzenetek megjelenítése")});
#else
	parser.addOption({{QStringLiteral("l"), QStringLiteral("latency")}, QObject::tr("Késleltett válaszadás"), QStringLiteral("msec")});
#endif

	parser.parse(m_arguments);

	parser.clearPositionalArguments();


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


	if (parser.isSet(QStringLiteral("quiet")))
		m_consoleAppender->setDetailsLevel(Logger::Fatal);
	else {
		if (parser.isSet(QStringLiteral("trace")))
			m_consoleAppender->setDetailsLevel(Logger::Trace);
#ifdef QT_DEBUG
		else
			m_consoleAppender->setDetailsLevel(Logger::Debug);
#else
		else if (parser.isSet(QStringLiteral("debug")))
			m_consoleAppender->setDetailsLevel(Logger::Debug);
		else
			m_consoleAppender->setDetailsLevel(Logger::Info);
#endif
	}


	if (parser.isSet(QStringLiteral("import")))
		m_importDb = parser.value(QStringLiteral("import"));




	m_settings->loadFromFile();

	if (m_settings->generateJwtSecret())
		m_settings->saveToFile(true);

	if (parser.isSet(QStringLiteral("latency"))) {
		setImitateLatency(parser.value(QStringLiteral("latency")).toInt());
		LOG_CDEBUG("service") << "Imitate latency:" << m_imitateLatency;
	}


	if (parser.isSet(QStringLiteral("log")))
		m_settings->setLogLimit(parser.value(QStringLiteral("log")).toInt());

	int logLimit = m_settings->logLimit();

	if (logLimit > 0) {
		const QString &logDir = QStringLiteral("log");

		if (!m_settings->dataDir().exists(logDir)) {
			if (!m_settings->dataDir().mkdir(logDir)) {
				LOG_CERROR("service") << "Directory create error:" << qPrintable(m_settings->dataDir().absoluteFilePath(logDir));

				std::exit(1);
				return false;
			}
		}

		RollingFileAppender* appender = new RollingFileAppender(m_settings->dataDir().absoluteFilePath(logDir+QStringLiteral("/server.log")));
		appender->setFormat(QString::fromStdString("%{time}{yyyy-MM-dd hh:mm:ss}: %{category:-10} [%{Type}] %{message}\n"));
		appender->setDatePattern(RollingFileAppender::DailyRollover);
		appender->setLogFilesLimit(logLimit);
		appender->setDetailsLevel(Logger::Debug);
		cuteLogger->registerAppender(appender);
	}

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
 * @brief ServerConfig::set
 * @param object
 */

void ServerConfig::set(const QJsonObject &object)
{
	for (auto it=object.constBegin(); it != object.constEnd(); ++it)
		m_data.insert(it.key(), it.value());
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








/**
 * @brief ServerService::onMainTimerTimeout
 */

void ServerService::onMainTimerTimeout()
{
	const QDateTime &current = QDateTime::currentDateTime();
	const QDateTime dtMinute(current.date(), QTime(current.time().hour(), current.time().minute()));

	if (!m_mainTimerLastTick.isNull() && dtMinute <= m_mainTimerLastTick)
		return;

	LOG_CTRACE("service") << "Timer check";
	m_mainTimerLastTick = dtMinute;

	if (!m_databaseMain) {
		LOG_CWARNING("service") << "Main database unavailable";
		return;
	}

	m_databaseMain->worker()->execInThread([this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(m_databaseMain->dbName());

		QMutexLocker(m_databaseMain->mutex());

		// Finish campaigns

		LOG_CTRACE("service") << "Finish campaigns";

		QueryBuilder q(db);
		q.addQuery("SELECT id FROM campaign WHERE started=true AND finished=false "
				   "AND endTime IS NOT NULL AND endTime<").addValue(QDateTime::currentDateTimeUtc());

		if (!q.exec()) {
			LOG_CERROR("service") << "Finish campaigns failed";
			return;
		}

		while (q.sqlQuery().next()) {
			const int id = q.value("id").toInt();
			QDefer::await(AdminAPI::campaignFinish(m_databaseMain, id));
		}


		// Start campaigns

		LOG_CTRACE("service") << "Start campaigns";

		QueryBuilder qq(db);
		qq.addQuery("SELECT id FROM campaign WHERE started=false "
				   "AND startTime IS NOT NULL AND startTime<").addValue(QDateTime::currentDateTimeUtc());

		if (!qq.exec()) {
			LOG_CERROR("service") << "Start campaigns failed";
			return;
		}

		while (qq.sqlQuery().next()) {
			const int id = qq.value("id").toInt();
			QDefer::await(AdminAPI::campaignStart(m_databaseMain, id));
		}

	});

}
