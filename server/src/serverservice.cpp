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
#include <QCoreApplication>
#include <RollingFileAppender.h>
#include <ColorConsoleAppender.h>
#include "qconsole.h"
#include "utils.h"
#include "googleoauth2authenticator.h"
#include "microsoftoauth2authenticator.h"
#include <QOAuthHttpServerReplyHandler>
#include <csignal>
#include <QResource>



const int ServerService::m_versionMajor = VERSION_MAJOR;
const int ServerService::m_versionMinor = VERSION_MINOR;
const int ServerService::m_versionBuild = VERSION_BUILD;
const char *ServerService::m_version = VERSION_FULL;
ServerService *ServerService::m_instance = nullptr;



/**
 * @brief ServerService::ServerService
 * @param argc
 * @param argv
 */

ServerService::ServerService(int &argc, char **argv)
	: QObject()
	, m_consoleAppender(new ColorConsoleAppender)
	, m_application(new QCoreApplication(argc, argv))
	, m_settings(new ServerSettings)
{
	Q_ASSERT(!m_instance);

	m_instance = this;

	m_consoleAppender->setDetailsLevel(Logger::Info);

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

	QObject::connect(&m_mainTimer, &QTimer::timeout, this, &ServerService::onMainTimerTimeout);
	m_mainTimer.setInterval(500);
}


/**
 * @brief ServerService::~ServerService
 */

ServerService::~ServerService()
{
	m_instance = nullptr;
}


/**
 * @brief ServerService::initialize
 */

void ServerService::initialize()
{
	QCoreApplication::setApplicationName(QStringLiteral("callofsuli-server"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("callofsuli-server"));
	QCoreApplication::setApplicationVersion(m_version);

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






/**
 * @brief ServerService::wasmLoad
 * @return
 */

bool ServerService::wasmLoad()
{
	QStringList wasmList;

	wasmList.append(m_settings->dataDir().absoluteFilePath(QStringLiteral("wasm.rcc")));
	wasmList.append(QCoreApplication::applicationDirPath()+QStringLiteral("/share/wasm.rcc"));
	wasmList.append(QCoreApplication::applicationDirPath()+QStringLiteral("/../share/wasm.rcc"));
	wasmList.append(QCoreApplication::applicationDirPath()+QStringLiteral("/../../share/wasm.rcc"));


	auto it = std::find_if(wasmList.constBegin(), wasmList.constEnd(), [](const QString &s){
		return QFile::exists(s);
	});


	if (it == wasmList.constEnd()) {
		LOG_CWARNING("service") << "Wasm resource not found";
		return false;
	} else {
		if (QResource::registerResource(*it, QStringLiteral("/wasm"))) {
			LOG_CDEBUG("service") << "Registering wasm resource:" << *it;
			m_loadedWasmResource = *it;
		} else {
			LOG_CERROR("service") << "Wasm resource register error:" << *it;
			return false;
		}
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
		LOG_CWARNING("service") << "Missing wasm resource";
		return true;
	}

	if (QResource::unregisterResource(m_loadedWasmResource, QStringLiteral("/wasm"))) {
		LOG_CDEBUG("service") << "Wasm resource unregistered:" << m_loadedWasmResource;
		return true;
	} else {
		LOG_CERROR("service") << "Wasm resource unregister failed:" << m_loadedWasmResource;
		return false;
	}


	return true;
}



/**
 * @brief ServerService::processSignal
 * @param sig
 */

void ServerService::processSignal(int sig)
{
	if (!m_instance) {
		LOG_CWARNING("service") << "Service instance missing";
		return;
	}

	if (sig == SIGTERM || sig == SIGINT)
		m_instance->stop();
	else if (sig == SIGHUP)
		m_instance->reload();
	else if (sig == SIGUSR1)
		m_instance->pause();
	else if (sig == SIGUSR2)
		m_instance->resume();
	else
		LOG_CWARNING("service") << "Unknown signal" << sig;
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
 * @brief ServerService::logPeerUser
 * @param user
 */

bool ServerService::logPeerUser(const PeerUser &user)
{
	bool ret = PeerUser::addOrUpdate(&m_peerUser, user);
	//if (ret)
	//	triggerEventStreams(EventStream::EventStreamPeerUsers);
	return ret;
}


/**
 * @brief ServerService::importDb
 * @return
 */

const QString &ServerService::importDb() const
{
	return m_importDb;
}


#ifdef _COMPAT
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

#endif


/**
 * @brief ServerService::networkManager
 * @return
 */

QNetworkAccessManager *ServerService::networkManager() const
{
	return m_networkManager.get();
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
 * @brief ServerService::webSocketServer
 * @return
 */

std::weak_ptr<WebServer> ServerService::webServer() const
{
	return m_webServer;
}



/**
 * @brief ServerService::databaseMain
 * @return
 */

DatabaseMain *ServerService::databaseMain() const
{
	return m_databaseMain.get();
}


/**
 * @brief ServerService::databaseMainWorker
 * @return
 */

QLambdaThreadWorker *ServerService::databaseMainWorker() const
{
	if (m_databaseMain)
		return m_databaseMain->worker();
	else
		return nullptr;
}


/**
 * @brief ServerService::preStart
 * @return
 */

std::optional<int> ServerService::preStart()
{
	initialize();

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
		return 0;
	}

	if (parser.isSet(versionOption)) {
		parser.showVersion();
		return 0;
	}


	if (parser.isSet(QStringLiteral("license"))) {
		const std::optional<QByteArray> &b = Utils::fileContent(QStringLiteral(":/license.txt"));

		if (!b)
			return 1;

		QConsole::qStdOut()->write(*b);

		return 0;
	}

	const QByteArray &envDir = qgetenv("SERVER_DIR");

	if (parser.isSet(QStringLiteral("dir")))
		m_settings->setDataDir(parser.value(QStringLiteral("dir")));
	else if (!envDir.isEmpty())
		m_settings->setDataDir(QString::fromUtf8(envDir));
	else {
		LOG_CERROR("service") << qPrintable(QCoreApplication::tr("You must specify main data directory"));

		return 1;
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

#ifdef QT_DEBUG
	if (parser.isSet(QStringLiteral("latency"))) {
		setImitateLatency(parser.value(QStringLiteral("latency")).toInt());
		LOG_CDEBUG("service") << "Imitate latency:" << m_imitateLatency;
	}
#endif

	if (parser.isSet(QStringLiteral("log")))
		m_settings->setLogLimit(parser.value(QStringLiteral("log")).toInt());

	int logLimit = m_settings->logLimit();

	if (logLimit > 0) {
		const QString &logDir = QStringLiteral("log");

		if (!m_settings->dataDir().exists(logDir)) {
			if (!m_settings->dataDir().mkdir(logDir)) {
				LOG_CERROR("service") << "Directory create error:" << qPrintable(m_settings->dataDir().absoluteFilePath(logDir));
				return 2;
			}
		}

		RollingFileAppender* appender = new RollingFileAppender(m_settings->dataDir().absoluteFilePath(logDir+QStringLiteral("/server.log")));
		appender->setFormat(QString::fromStdString("%{time}{yyyy-MM-dd hh:mm:ss} %{category:-10} [%{Type}] %{message}\n"));
		appender->setDatePattern(RollingFileAppender::DailyRollover);
		appender->setLogFilesLimit(logLimit);
		appender->setDetailsLevel(Logger::Debug);
		cuteLogger->registerAppender(appender);
	}

	return std::nullopt;
}


/**
 * @brief ServerService::settings
 * @return
 */

ServerSettings *ServerService::settings() const
{
	return m_settings.get();
}

/**
 * @brief ServerService::version
 * @return
 */

const char *ServerService::version() const
{
	return m_version;
}


/**
 * @brief ServerService::exec
 * @return
 */

int ServerService::exec()
{
	for (const int &s : QVector<int>{SIGINT, SIGTERM, SIGHUP, SIGUSR1, SIGUSR2})
		signal(s, &ServerService::processSignal);

	m_state = ServerPrepared;


#ifdef Q_OS_LINUX
	pid_t pid = getpid();
	LOG_CDEBUG("service") << "Server service PID:" << pid;
#endif

	m_settings->printConfig();

	wasmLoad();

	// Load main DB

	m_databaseMain = std::make_unique<DatabaseMain>(this);
	m_databaseMain->setDbFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("main.db")));
	m_databaseMain->setDbMapsFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("maps.db")));
	m_databaseMain->setDbStatFile(m_settings->dataDir().absoluteFilePath(QStringLiteral("stat.db")));


	if (QFile::exists(m_databaseMain->dbFile()) && !m_importDb.isEmpty()) {
		LOG_CERROR("service") << "Import error, main database already exists";
		return 4;
	}

	m_databaseMain->databaseOpen(m_databaseMain->dbFile());

	if (!m_databaseMain->databasePrepare(m_importDb)) {
		LOG_CERROR("service") << "Main database prepare error";
		return 5;
	}

	if (!m_databaseMain->databaseAttach()) {
		LOG_CERROR("service") << "Main database attach error";
		return 6;
	}

	m_config.m_service = this;


	// Create authenticators

	for (auto it=m_settings->oauthMap().constBegin(); it != m_settings->oauthMap().constEnd(); ++it) {
		std::shared_ptr<OAuth2Authenticator> authenticator;

		if (it.key() == QStringLiteral("google"))
			authenticator = std::make_shared<GoogleOAuth2Authenticator>(this);
		else if (it.key() == QStringLiteral("microsoft"))
			authenticator = std::make_shared<MicrosoftOAuth2Authenticator>(this);

		if (authenticator) {
			authenticator->setOAuth(it.value());
			m_authenticators.append(std::move(authenticator));
		}
	}


	if (!start())
		return 3;

	int r = m_application->exec();

	LOG_CDEBUG("service") << "Exit with code" << r;

	return r;
}



/**
 * @brief ServerService::versionBuild
 * @return
 */

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
			m_data = Utils::byteArrayToJsonObject(s.toUtf8()).value_or(QJsonObject());


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

	//if (PeerUser::clear(&m_peerUser))
	//	triggerEventStreams(EventStream::EventStreamPeerUsers);


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
			AdminAPI::campaignFinish(m_databaseMain.get(), id);
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
			AdminAPI::campaignStart(m_databaseMain.get(), id);
		}

	});

}



/**
 * @brief ServerService::start
 * @return
 */

bool ServerService::start()
{
	if (m_state != ServerPrepared && m_state != ServerPaused) {
		LOG_CERROR("service") << "Invalid state to start:" << m_state;
		return false;
	}

	m_config.loadFromDb(m_databaseMain.get());

	m_webServer.reset(new WebServer(this));

	LOG_CDEBUG("service") << "Server service started";

	m_webServer->setRedirectHost(m_settings->redirectHost());

	m_mainTimer.start();

	LOG_CINFO("service") << "Server service started successful";

	if (m_webServer->start()) {
		m_state = ServerRunning;
		return true;
	} else {
		return false;
	}
}



/**
 * @brief ServerService::stop
 */

void ServerService::stop()
{
	LOG_CINFO("service") << "Server service stopping";

	m_state = ServerFinished;

	if (m_webServer)
		m_webServer.reset();

	m_mainTimer.stop();
	m_databaseMain->databaseClose();

	m_application->quit();
}


/**
 * @brief ServerService::pause
 */

void ServerService::pause()
{
	if (m_state != ServerRunning) {
		LOG_CERROR("service") << "Invalid state to pause:" << m_state;
		return;
	}

	LOG_CINFO("service") << "Server service pause";

	if (m_webServer)
		m_webServer.reset();

	m_state = ServerPaused;

	m_mainTimer.stop();
	m_databaseMain->databaseClose();
}


/**
 * @brief ServerService::resume
 */

void ServerService::resume()
{
	if (m_state != ServerPaused) {
		LOG_CERROR("service") << "Invalid state to resume:" << m_state;
		return;
	}

	LOG_CINFO("service") << "Server service resume";

	if (!m_databaseMain->databaseOpen(m_databaseMain->dbFile()))
		return std::exit(10);

	if (!m_databaseMain->databaseAttach())
		return std::exit(10);

	m_mainTimer.start();

	if (!start())
		m_application->quit();
}



/**
 * @brief ServerService::reload
 */

void ServerService::reload()
{
	if (m_state != ServerRunning && m_state != ServerPaused) {
		LOG_CERROR("service") << "Invalid state to reload:" << m_state;
		return;
	}

	LOG_CINFO("service") << "Server service reload";

	if (m_state == ServerRunning) {
		pause();
	}

	wasmUnload();

	wasmLoad();

	resume();
}



const QVector<std::shared_ptr<OAuth2Authenticator> > &ServerService::authenticators() const
{
	return m_authenticators;
}


/**
 * @brief ServerService::oauth2Authenticator
 * @param type
 * @return
 */

std::weak_ptr<OAuth2Authenticator> ServerService::oauth2Authenticator(const char *type) const
{
	const auto &it = std::find_if(m_authenticators.constBegin(), m_authenticators.constEnd(), [&type](const std::shared_ptr<OAuth2Authenticator> &auth) {
		return strcmp(auth->type(), type) == 0;
	});

	return *it;
}



/**
 * @brief ServerService::serverName
 * @return
 */

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
 * @brief ServerService::peerUser
 * @return
 */

const QVector<PeerUser> &ServerService::peerUser() const
{
	return m_peerUser;
}




/**
 * @brief PeerUser::add
 * @param list
 * @param user
 */

QVector<PeerUser>::iterator PeerUser::find(QVector<PeerUser> *list, const PeerUser &user)
{
	Q_ASSERT(list);

	QVector<PeerUser>::iterator it = list->begin();

	for (; it != list->end(); ++it) {
		if (*it == user)
			break;
	}

	return it;
}


/**
 * @brief PeerUser::add
 * @param list
 * @param user
 */

bool PeerUser::addOrUpdate(QVector<PeerUser> *list, const PeerUser &user)
{
	Q_ASSERT(list);

	if (user.m_username.isEmpty())
		return false;

	auto it = find(list, user);

	if (it == list->end()) {
		list->append(user);
		return true;
	} else
		it->setTimestamp(QDateTime::currentDateTime());

	return false;
}


/**
 * @brief PeerUser::remove
 * @param list
 * @param user
 * @return
 */

bool PeerUser::remove(QVector<PeerUser> *list, const PeerUser &user)
{
	Q_ASSERT(list);

	if (user.m_username.isEmpty())
		return false;

	bool ret = false;

	for (auto it = list->constBegin(); it != list->constEnd(); ) {
		if (*it == user) {
			list->erase(it);
			ret = true;
		} else
			++it;
	}

	return ret;
}



/**
 * @brief PeerUser::clear
 * @param list
 */

bool PeerUser::clear(QVector<PeerUser> *list, const qint64 &sec)
{
	Q_ASSERT(list);

	const QDateTime &dt = QDateTime::currentDateTime();

	bool ret = false;

	for (auto it = list->constBegin(); it != list->constEnd(); ) {
		if (!(it->m_timestamp).isValid() || it->m_timestamp.secsTo(dt) > sec) {
			list->erase(it);
			ret = true;
		} else
			++it;
	}

	return ret;
}


/**
 * @brief PeerUser::toJson
 * @param list
 * @return
 */

QJsonArray PeerUser::toJson(const QVector<PeerUser> *list)
{
	Q_ASSERT(list);

	QJsonArray ret;

	foreach (const PeerUser &user, *list) {
		QJsonObject o;
		o.insert(QStringLiteral("username"), user.m_username);
		o.insert(QStringLiteral("host"), user.m_host.toString());
		o.insert(QStringLiteral("agent"), user.m_agent);
		o.insert(QStringLiteral("timestamp"), user.m_timestamp.toSecsSinceEpoch());
		ret.append(o);
	}

	return ret;
}




const QString &PeerUser::username() const
{
	return m_username;
}

void PeerUser::setUsername(const QString &newUsername)
{
	m_username = newUsername;
}

const QString &PeerUser::familyName() const
{
	return m_familyName;
}

void PeerUser::setFamilyName(const QString &newFamilyName)
{
	m_familyName = newFamilyName;
}

const QString &PeerUser::givenName() const
{
	return m_givenName;
}

void PeerUser::setGivenName(const QString &newGivenName)
{
	m_givenName = newGivenName;
}

const QHostAddress &PeerUser::host() const
{
	return m_host;
}

void PeerUser::setHost(const QHostAddress &newHost)
{
	m_host = newHost;
}

const QDateTime &PeerUser::timestamp() const
{
	return m_timestamp;
}

void PeerUser::setTimestamp(const QDateTime &newTimestamp)
{
	m_timestamp = newTimestamp;
}

const QString &PeerUser::agent() const
{
	return m_agent;
}

void PeerUser::setAgent(const QString &newAgent)
{
	m_agent = newAgent;
}
