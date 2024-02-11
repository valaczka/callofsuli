/*
 * ---- Call of Suli ----
 *
 * desktopapplication.cpp
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * DesktopApplication
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

#include <RollingFileAppender.h>
#include "ColorConsoleAppender.h"
#include "desktopapplication.h"
#include "standaloneclient.h"
#include "utils_.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#if QT_VERSION >= 0x060000
#include <private/qguiapplication_p.h>
#else
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif
#endif


/**
 * @brief DesktopApplication::DesktopApplication
 * @param argc
 * @param argv
 */

DesktopApplication::DesktopApplication(QApplication *app)
	: MobileApplication(app)
	, m_appender(new ColorConsoleAppender)
{
#ifdef Q_OS_WIN
	SetConsoleOutputCP(CP_UTF8);

#if QT_VERSION >= 0x060000
	if(auto inf = qApp->nativeInterface<QNativeInterface::Private::QWindowsApplication>()) {
		inf->setWindowActivationBehavior(QNativeInterface::Private::QWindowsApplication::AlwaysActivateWindow);
	}
#else
	QWindowsWindowFunctions::setWindowActivationBehavior(QWindowsWindowFunctions::AlwaysActivateWindow);
#endif

#endif

	m_appender->setDetailsLevel(Logger::Info);

#ifndef QT_NO_DEBUG
	m_appender->setFormat(QString::fromStdString(
							  "%{time}{hh:mm:ss} %{category:-10} [%{TypeOne}] %{message} "+
							  ColorConsoleAppender::reset+ColorConsoleAppender::green+"<%{function} "+
							  ColorConsoleAppender::magenta+"%{file}:%{line}"+
							  ColorConsoleAppender::green+">\n"));
#else
	m_appender->setFormat(QString::fromStdString("%{time}{hh:mm:ss} %{category:-10} [%{TypeOne}] %{message}\n"));
#endif

	cuteLogger->registerAppender(m_appender);

	cuteLogger->logToGlobalInstance(QStringLiteral("app"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("oauth2"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("client"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("credential"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("game"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("scene"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("sound"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("utils"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("http"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("utils"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("updater"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("qml"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("logger"), true);
	cuteLogger->logToGlobalInstance(QStringLiteral("qaterial.utils"), true);


	// QSingleInstance

	QObject::connect(&m_singleInstance, &QSingleInstance::instanceMessage, &m_singleInstance, [this](const QStringList &args){
		performInstanceArguments(args);
	});

	m_singleInstance.setStartupFunction([this]() -> int {
		return run();
	});
}


/**
 * @brief DesktopApplication::~DesktopApplication
 */

DesktopApplication::~DesktopApplication()
{

}



/**
 * @brief DesktopApplication::commandLineParse
 */

void DesktopApplication::commandLineParse()
{
	QCommandLineParser parser;
	parser.setApplicationDescription(QString::fromUtf8("Call of Suli – Copyright © 2012-2024 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOption({QStringLiteral("license"), QObject::tr("Licensz")});
	parser.addOption({{QStringLiteral("l"), QStringLiteral("log")}, QObject::tr("Naplózás <file> fájlba"), QStringLiteral("file")});
	parser.addOption({{QStringLiteral("n"), QStringLiteral("log-limit")}, QObject::tr("Maximum <db> log fájl tárolása"), QStringLiteral("db")});
	parser.addOption({{QStringLiteral("e"), QStringLiteral("editor")}, QObject::tr("Pályszerkesztő indítása")});
	parser.addOption({{QStringLiteral("m"), QStringLiteral("map")}, QObject::tr("Pálya szerkesztése"), QStringLiteral("file")});
	parser.addOption({{QStringLiteral("p"), QStringLiteral("play")}, QObject::tr("Pálya lejátszása"), QStringLiteral("file")});
	parser.addOption({{QStringLiteral("d"), QStringLiteral("demo")}, QObject::tr("Demo pálya lejátszása")});


#ifndef QT_NO_DEBUG
	parser.addOption({QStringLiteral("trace"), QObject::tr("Trace üzenetek megjelenítése")});
	parser.addOption({QStringLiteral("dev-page"), QObject::tr("_PageDev.qml betöltése")});
	parser.addOption({QStringLiteral("adjacency"), QObject::tr("PageConquest adjacency setup"), QStringLiteral("world")});
#else
	parser.addOption({QStringLiteral("debug"), QObject::tr("Debug üzenetek megjelenítése")});
#endif

	parser.process(*m_application);

	if (parser.isSet(QStringLiteral("license"))) {
		m_commandLine = License;
		return;
	}


	QString logFile;

	if (parser.isSet(QStringLiteral("log"))) logFile = parser.value(QStringLiteral("log"));

	int logLimit = 12;

	if (parser.isSet(QStringLiteral("log-limit"))) logLimit = parser.value(QStringLiteral("log-limit")).toInt();

	if (!logFile.isEmpty()) {
		RollingFileAppender* appender = new RollingFileAppender(logFile);
		appender->setFormat(QStringLiteral("%{time}{hh:mm:ss} [%{TypeOne}] %{category} %{message}\n"));
		appender->setDatePattern(RollingFileAppender::WeeklyRollover);
		appender->setLogFilesLimit(logLimit);
		cuteLogger->registerAppender(appender);
	}


	if (parser.isSet(QStringLiteral("editor")))
		m_commandLine = Editor;
	else if (parser.isSet(QStringLiteral("demo")))
		m_commandLine = Demo;
	else if (parser.isSet(QStringLiteral("map"))) {
		m_commandLine = Map;
		m_commandLineData = parser.value(QStringLiteral("map"));
	} else if (parser.isSet(QStringLiteral("play"))) {
		m_commandLine = Play;
		m_commandLineData = parser.value(QStringLiteral("play"));
	}
#ifndef QT_NO_DEBUG
	else if (parser.isSet(QStringLiteral("dev-page")))
		m_commandLine = DevPage;
	else if (parser.isSet(QStringLiteral("adjacency"))) {
		m_commandLine = Adjacency;
		m_commandLineData = parser.value(QStringLiteral("adjacency"));
	}
#endif

	m_arguments = parser.positionalArguments();


#ifndef QT_NO_DEBUG
#ifdef Q_OS_ANDROID
	m_appender->setDetailsLevel(Logger::Trace);
#else
	if (parser.isSet(QStringLiteral("trace")))
		m_appender->setDetailsLevel(Logger::Trace);
	else
		m_appender->setDetailsLevel(Logger::Debug);
#endif
#else
	if (parser.isSet(QStringLiteral("debug")))
		m_appender->setDetailsLevel(Logger::Debug);
	else
		m_appender->setDetailsLevel(Logger::Info);
#endif


}



/**
 * @brief DesktopApplication::performCommandLine
 */

bool DesktopApplication::performCommandLine()
{
	if (m_commandLine == License)
	{
		const auto &b = Utils::fileContent(QStringLiteral(":/license.txt"));

		if (b) {
		QTextStream out(stdout);
		out << *b << Qt::endl;
		}

		return false;
	}

	return true;
}



/**
 * @brief DesktopApplication::performInstanceArguments
 * @param arguments
 */

void DesktopApplication::performInstanceArguments(const QStringList &arguments)
{
	if (arguments.size() > 1 )
		selectUrl(arguments.at(1));
	else
		selectUrl(QUrl());
}



/**
 * @brief DesktopApplication::runSingleInstance
 * @return
 */

int DesktopApplication::runSingleInstance()
{
#ifndef QT_NO_DEBUG
	LOG_CERROR("app") << "DISABLED SINGLE INSTANCE";
#else
	if (m_singleInstance.process()) {
		if (!m_singleInstance.isMaster()) {
			LOG_CINFO("app") << QObject::tr("Már fut az alkalmazás egy példánya");
			return 1;
		}
	} else {
		LOG_CINFO("app") << QObject::tr("Már fut az alkalmazás egy példánya");
		return 1;
	}
#endif

	return run();
}



/**
 * @brief DesktopApplication::createClient
 * @return
 */

Client *DesktopApplication::createClient()
{
	Client *c = new StandaloneClient(this);

	if (!m_arguments.isEmpty()) {
		QUrl u(m_arguments.at(0));
		if (u.isValid())
			c->setParseUrl(u);
	}

	return c;
}



