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
#include "utils_.h"
#include "googleoauth2authenticator.h"
#include "microsoftoauth2authenticator.h"
#include "conquestengine.h"
#include <QOAuthHttpServerReplyHandler>
#include <csignal>
#include <QResource>
#include "querybuilder.hpp"
#include "teacherapi.h"
#include "authapi.h"


const int ServerService::m_versionMajor = VERSION_MAJOR;
const int ServerService::m_versionMinor = VERSION_MINOR;
const int ServerService::m_versionBuild = VERSION_BUILD;
const char *ServerService::m_version = VERSION_FULL;
ServerService *ServerService::m_instance = nullptr;


#ifndef QT_NO_DEBUG
#	define _MAIN_TIMER_TEST_MODE
#endif

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
	, m_networkManager(new QNetworkAccessManager(this))
	//, m_engineHandler(new EngineHandler(this))
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

#ifdef _MAIN_TIMER_TEST_MODE
	LOG_CERROR("service") << "_MAIN_TIMER_TEST_MODE defined";
	m_mainTimerInterval = 2000;
#else
	m_mainTimerInterval = 100;
#endif

	connect(m_application.get(), &QCoreApplication::aboutToQuit, this, [this](){
		m_mainTimer.stop();
		m_engineHandler.reset();
		m_webServer.reset();
		m_networkManager.reset();
		m_authenticators.clear();
		m_databaseMain->databaseClose();
		m_databaseMain.reset();
		m_settings.reset();
	});
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

	QLocale::setDefault(QLocale(QLocale::Hungarian, QLocale::Hungary));

	cuteLogger->logToGlobalInstance(QStringLiteral("service"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("app"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("db"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("credential"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("logger"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("oauth2"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("client"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("engine"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("utils"), true);
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
 * @brief ServerService::agentSignLoad
 */

void ServerService::agentSignLoad()
{
	QDirIterator it(m_settings->dataDir().absoluteFilePath(QStringLiteral("agentSign")), QDir::Files);

	while (it.hasNext()) {
		const QString &file = it.next();
		const auto &ptr = Utils::fileContent(file);

		if (!ptr) {
			LOG_CERROR("service") << "Read error:" << file;
			continue;
		}

		LOG_CDEBUG("service") << "Add agent signature" << file;
		m_agentSignatures.append(ptr->data());
	}
}


/**
 * @brief ServerService::agentSignUnload
 */

void ServerService::agentSignUnload()
{
	LOG_CDEBUG("service") << "Unload agent signatures";
	m_agentSignatures.clear();
}



/**
 * @brief ServerService::timerEvent
 * @param event
 */

void ServerService::timerEvent(QTimerEvent *)
{
	if (m_state != ServerRunning)
		return;

	m_engineHandler->timerEvent();

	const QDateTime &current = QDateTime::currentDateTime();
	const QDateTime dtMinute(current.date(), QTime(current.time().hour(), current.time().minute()));

#ifndef _MAIN_TIMER_TEST_MODE
	if (!m_mainTimerLastTick.isNull() && dtMinute <= m_mainTimerLastTick)
		return;
#endif

	m_engineHandler->timerMinuteEvent();

#ifdef _MAIN_TIMER_TEST_MODE
	if (!m_mainTimerLastTick.isNull() && dtMinute <= m_mainTimerLastTick)
		return;
#endif

	m_mainTimerLastTick = dtMinute;

	if (!m_databaseMain) {
		LOG_CWARNING("service") << "Main database unavailable";
		return;
	}

	m_databaseMain->worker()->execInThread([this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(m_databaseMain->dbName());

		QMutexLocker _locker(m_databaseMain->mutex());

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

		// Clear wallet

		TeacherAPI::_clearWallet(m_databaseMain.get(), this);
	});

	// Send notifications

	AdminAPI::sendNotifications(m_databaseMain.get(), this);

	LOG_CTRACE("service") << "Timer check finished";
}



/**
 * @brief ServerService::loadDynamicDictFromRcc
 * @param path
 */

void ServerService::loadDynamicDictFromRcc(const QString &filename, const QString &path)
{
	QDirIterator it2(path, { QStringLiteral("*tsx") }, QDir::Files, QDirIterator::Subdirectories);

	while (it2.hasNext()) {
		QString file = it2.next();
		file.replace(path, QStringLiteral(""));

		if (m_dynamicContentDict.contains(file)) {
			LOG_CERROR("service") << "Dynamic content registry already exists:" << file;
		} else {
			m_dynamicContentDict.insert(file, filename);
		}
	}
}



/**
 * @brief ServerService::loadMarket
 * @return
 */

RpgMarketList ServerService::loadMarket() const
{
	LOG_CDEBUG("service") << "Load market";

	const QString &dir = m_settings->dataDir().absoluteFilePath(QStringLiteral("content"));

	if (!QFile::exists(dir)) {
		LOG_CTRACE("service") << "Dynamic content directory missing";
		return {};
	}

	RpgMarketList ret;

	QDirIterator it(dir, {
						QStringLiteral("*.cres"), QStringLiteral("*.dres")
					}, QDir::Files);

	while (it.hasNext()) {
		const QString &file = it.next();

		LOG_CTRACE("service") << "Load:" << qPrintable(file);

		if (!QResource::registerResource(file, QStringLiteral("/tmp"))) {
			LOG_CERROR("service") << "Invalid resource:" << qPrintable(file);
			continue;
		}

		{
			QDirIterator it2(QStringLiteral(":/tmp"), { QStringLiteral("market.json") }, QDir::Files, QDirIterator::Subdirectories);

			while (it2.hasNext()) {
				const QString &file = it2.next();
				const QString &name = file.section('/', -2, -2);

				LOG_CTRACE("service") << "Load market data from" << file;

				const auto &ptr = Utils::fileToJsonObject(file);

				if (!ptr) {
					LOG_CERROR("service") << "Invalid market data:" << qPrintable(file);
					continue;
				}

				RpgMarket market;
				market.fromJson(*ptr);
				if (market.name.isEmpty())
					market.name = name;

				ret.list.append(market);
			}
		}

		if (!QResource::unregisterResource(file, QStringLiteral("/tmp"))) {
			LOG_CERROR("service") << "Unregister resource error:" << file;
		}
	}

	return ret;
}


/**
 * @brief ServerService::loadMarket
 * @param filename
 * @return
 */

RpgMarketList ServerService::loadMarket(const QString &filename) const
{
	LOG_CDEBUG("service") << "Load market from file:" << qPrintable(filename);

	const auto &ptr = Utils::fileToJsonObject(filename);

	if (!ptr) {
		LOG_CERROR("service") << "Invalid market data:" << qPrintable(filename);
		return {};
	}

	RpgMarketList list;
	list.fromJson(*ptr);
	return list;
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
 * @brief ServerService::loadSmtpServer
 */

void ServerService::loadSmtpServer()
{
	if (m_settings->smtpHost().isEmpty() || m_settings->smtpUser().isEmpty())
		return;

	m_smtpServer.reset(new SimpleMail::Server);

	m_smtpServer->setHost(m_settings->smtpHost());
	m_smtpServer->setPort(m_settings->smtpPort());
	m_smtpServer->setConnectionType(m_settings->smtpSsl() ? SimpleMail::Server::SslConnection : SimpleMail::Server::TcpConnection);
	m_smtpServer->setUsername(m_settings->smtpUser());
	m_smtpServer->setPassword(m_settings->smtpPassword());

	LOG_CDEBUG("service") << "SMTP client started";
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
	m_settings->setDataDir(QCoreApplication::applicationDirPath());

	QCommandLineParser parser;
	parser.setApplicationDescription(QString::fromUtf8(QByteArrayLiteral("Call of Suli szerver – Copyright © 2012-2024 Valaczka János Pál")));
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);

	auto helpOption = parser.addHelpOption();
	auto versionOption = parser.addVersionOption();

	parser.addOption({QStringLiteral("license"), QObject::tr("Licensz")});
	parser.addOption({QStringLiteral("log"), QObject::tr("Naplózási limit (0 = nincs naplózás)"), QStringLiteral("num")});
	parser.addOption({{QStringLiteral("d"), QStringLiteral("dir")}, QObject::tr("Adatbázis könyvtár"), QStringLiteral("database-directory")});
	parser.addOption({{QStringLiteral("i"), QStringLiteral("import")}, QObject::tr("Adatbázis importálása"), QStringLiteral("database")});


	parser.addOption({{QStringLiteral("q"), QStringLiteral("quiet")}, QObject::tr("Csendes üzemmód")});
	parser.addOption({QStringLiteral("trace"), QObject::tr("Trace üzenetek megjelenítése")});

#ifdef QT_NO_DEBUG
	parser.addOption({QStringLiteral("debug"), QObject::tr("Debug üzenetek megjelenítése")});
#else
	parser.addOption({{QStringLiteral("l"), QStringLiteral("latency")}, QObject::tr("Késleltett válaszadás"), QStringLiteral("msec")});
	parser.addOption({{QStringLiteral("t"), QStringLiteral("token")}, QObject::tr("Create token"), QStringLiteral("username")});
#endif

	parser.addOption({{QStringLiteral("u"), QStringLiteral("upgrade")}, QObject::tr("Adatbázis frissítésének kényszerítése"), QStringLiteral("version")});

	parser.addOption({{QStringLiteral("m"), QStringLiteral("market")}, QObject::tr("Market adatbázis készítése")});

	parser.addOption({{QStringLiteral("z"), QStringLiteral("zap")}, QObject::tr("Felhasználói adatok TÖRLÉSE (hadjárat, dolgozat)")});

	parser.addPositionalArgument(QStringLiteral("dir"), QObject::tr("Adatbázis könyvtár"), QStringLiteral("[dir]"));

	parser.parse(m_arguments);

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
	else if (const QStringList &list = parser.positionalArguments(); !list.isEmpty())
		m_settings->setDataDir(list.first());
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
#ifndef QT_NO_DEBUG
		else
			m_consoleAppender->setDetailsLevel(Logger::Debug);
#else
		else if (parser.isSet(QStringLiteral("debug")))
			m_consoleAppender->setDetailsLevel(Logger::Debug);
		else
			m_consoleAppender->setDetailsLevel(Logger::Info);
#endif
	}


	if (parser.isSet(QStringLiteral("market"))) {
		QJsonDocument doc(loadMarket().toJson());
		QConsole::qStdOut()->write(doc.toJson());
		return 0;
	}

	m_zap = parser.isSet(QStringLiteral("zap"));

	if (parser.isSet(QStringLiteral("import")))
		m_importDb = parser.value(QStringLiteral("import"));

	if (parser.isSet(QStringLiteral("upgrade")))
		m_forceUpgrade = parser.value(QStringLiteral("upgrade"));


	m_engineHandler.reset(new EngineHandler(this));


	m_settings->loadFromFile();

	if (m_settings->generateJwtSecret())
		m_settings->saveToFile(true);

#ifndef QT_NO_DEBUG
	if (parser.isSet(QStringLiteral("latency"))) {
		setImitateLatency(parser.value(QStringLiteral("latency")).toInt());
		LOG_CDEBUG("service") << "Imitate latency:" << m_imitateLatency;
	}

	if (parser.isSet(QStringLiteral("token"))) {
		m_createToken = parser.value(QStringLiteral("token"));
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

	loadSmtpServer();

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
	agentSignLoad();

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


	// Force upgrade

	if (!m_forceUpgrade.isEmpty()) {
		const QStringList &sList = m_forceUpgrade.split('.');
		const int vMajor = sList.size() > 1 ? sList.at(0).toInt() : 0;
		const int vMinor = sList.size() > 1 ? sList.at(1).toInt() : 0;

		if (vMajor > 0) {
			LOG_CINFO("service") << "Upgrade database to version" << qPrintable(QString::number(vMajor).append('.').append(QString::number(vMinor)));
			if (!m_databaseMain->databaseUpgrade(vMajor, vMinor)) {
				LOG_CERROR("service") << "Upgrade error";
				return 7;
			}
		} else {
			LOG_CERROR("service") << "Invalid version code:" << m_forceUpgrade;
		}
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


	// Restore backed up engines

	const int &count = ConquestEngine::restoreEngines(this);

	if (count > 0)
		LOG_CINFO("service") << count << "ConquestEngines restored";

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

		QMutexLocker _locker(m_db->mutex());

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

	m_webServer->setRedirectHost(m_settings->redirectHost());

	m_mainTimer.start(m_mainTimerInterval, Qt::PreciseTimer, this);

	reloadDynamicContent();


	const QString &marketFile = m_settings->dataDir().absoluteFilePath(QStringLiteral("market.json"));

	if (QFile::exists(marketFile))
		setMarket(loadMarket(marketFile));
	else
		setMarket(loadMarket());


	AdminAPI::zapWallet(m_databaseMain.get());
	//AdminAPI::fillCurrency(m_databaseMain.get());

	if (!m_createToken.isEmpty()) {
		if (const auto &cred = AuthAPI::getCredential(m_databaseMain.get(), m_createToken)) {
			LOG_CDEBUG("service") << "Create token for:" << qPrintable(m_createToken);
			const QString &token = cred->createJWT(m_settings->jwtSecret());
			QConsole::qStdOut()->write(token.toLatin1());
		} else {
			LOG_CERROR("service") << "Invalid username:" << qPrintable(m_createToken);
		}

		return false;
	}


	if (m_zap) {
		if (!AdminAPI::zapUserData(m_databaseMain.get()))
			LOG_CERROR("service") << "Zap data error";
		else
			LOG_CINFO("service") << "Database zapped";

		return false;
	}

	LOG_CINFO("service") << "Server service started";

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

	if (m_webServer) {
		m_engineHandler->websocketCloseAll();
		m_webServer.reset();
	}

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

	if (m_webServer) {
		m_engineHandler->websocketCloseAll();
		m_webServer.reset();
	}

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

	m_mainTimer.start(m_mainTimerInterval, Qt::PreciseTimer, this);

	if (!start())
		m_application->quit();
}


/**
 * @brief ServerService::agentSignatures
 * @return
 */

const QList<QByteArray> &ServerService::agentSignatures() const
{
	return m_agentSignatures;
}


/**
 * @brief ServerService::market
 * @return
 */

const RpgMarketList &ServerService::market() const
{
	return m_market;
}

void ServerService::setMarket(const RpgMarketList &newMarket)
{
	m_market = newMarket;
}


/**
 * @brief ServerService::loadableDynamicContent
 * @return
 */

const QJsonArray &ServerService::loadableDynamicContent() const
{
	return m_loadableDynamicContent;
}


/**
 * @brief ServerService::dynamicContentDict
 * @return
 */

const QJsonObject &ServerService::dynamicContentDict() const
{
	return m_dynamicContentDict;
}



/**
 * @brief ServerService::dynamicContent
 * @return
 */

const QJsonArray &ServerService::dynamicContent() const
{
	return m_dynamicContent;
}


/**
 * @brief ServerService::mainTimerInterval
 * @return
 */

int ServerService::mainTimerInterval() const
{
	return m_mainTimerInterval;
}



/**
 * @brief ServerService::reloadDynamicContent
 */

void ServerService::reloadDynamicContent()
{
	m_dynamicContent = {};
	m_loadableDynamicContent = {};
	m_dynamicContentDict = {};
	ConquestEngine::m_helper.clear();

	const QString &dir = m_settings->dataDir().absoluteFilePath(QStringLiteral("content"));

	if (!QFile::exists(dir)) {
		LOG_CDEBUG("service") << "Dynamic content directory missing";
		return;
	}

	LOG_CINFO("service") << "Reload dynamic content:" << qPrintable(dir);

	for (const bool isStatic : std::vector<bool>{true, false}) {
		QDirIterator it(dir, {
							isStatic ? QStringLiteral("*.cres") : QStringLiteral("*.dres")
						}, QDir::Files);

		while (it.hasNext()) {
			const QString &file = it.next();

			LOG_CTRACE("service") << "Load:" << qPrintable(file);

			const auto &content = Utils::fileContent(file);

			if (!content)
				continue;

			const QString &md5 = QString::fromLatin1(QCryptographicHash::hash(*content, QCryptographicHash::Md5).toHex());
			const qint64 size = content->size();

			if (!QResource::registerResource(file, QStringLiteral("/tmp"))) {
				LOG_CERROR("service") << "Invalid resource:" << qPrintable(file);
				continue;
			}


			if (isStatic)
				m_dynamicContent.append(QJsonObject{
											{ QStringLiteral("file"), it.fileName() },
											{ QStringLiteral("md5"), md5 },
											{ QStringLiteral("size"), size }
										});
			else
				m_loadableDynamicContent.append(QJsonObject{
													{ QStringLiteral("file"), it.fileName() },
													{ QStringLiteral("md5"), md5 },
													{ QStringLiteral("size"), size }
												});



			LOG_CDEBUG("service") << "Dynamic content registered:" << qPrintable(it.fileName());

			if (isStatic)
				ConquestEngine::loadWorldDataFromResource(QStringLiteral(":/tmp"));
			else
				loadDynamicDictFromRcc(it.fileName(), QStringLiteral(":/tmp/"));

			if (!QResource::unregisterResource(file, QStringLiteral("/tmp"))) {
				LOG_CERROR("service") << "Unregister resource error:" << file;
			}
		}
	}
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
	agentSignUnload();

	wasmLoad();
	agentSignLoad();

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

std::optional<std::weak_ptr<OAuth2Authenticator>> ServerService::oauth2Authenticator(const char *type) const
{
	const auto &it = std::find_if(m_authenticators.constBegin(), m_authenticators.constEnd(),
								  [&type](const std::shared_ptr<OAuth2Authenticator> &auth) {
		return strcmp(auth->type(), type) == 0;
	});

	if (it != m_authenticators.constEnd())
		return *it;
	else
		return std::nullopt;
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



