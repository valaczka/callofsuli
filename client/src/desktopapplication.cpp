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
#include "desktopapplication.h"
#include "desktopclient.h"

#include "sound.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

/**
 * @brief DesktopApplication::DesktopApplication
 * @param argc
 * @param argv
 */

DesktopApplication::DesktopApplication(int &argc, char **argv)
	: Application(argc, argv)
{
	cuteLogger->logToGlobalInstance("app.application", true);
	cuteLogger->logToGlobalInstance("app.client", true);
	cuteLogger->logToGlobalInstance("app.game", true);
	cuteLogger->logToGlobalInstance("app.scene", true);
	cuteLogger->logToGlobalInstance("app.sound", true);
	cuteLogger->logToGlobalInstance("app.utils", true);
	cuteLogger->logToGlobalInstance("qaterial.utils", true);
	cuteLogger->logToGlobalInstance("qml", true);
	cuteLogger->logToGlobalInstance("logger", true);
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
	parser.setApplicationDescription(QString::fromUtf8("Call of Suli – Copyright © 2012-2023 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOption({QStringLiteral("license"), QObject::tr("Licensz")});
	parser.addOption({{QStringLiteral("l"), QStringLiteral("log")}, QObject::tr("Naplózás <file> fájlba"), QStringLiteral("file")});
	parser.addOption({{QStringLiteral("n"), QStringLiteral("log-limit")}, QObject::tr("Maximum <db> log fájl tárolása"), QStringLiteral("db")});
	parser.addOption({{QStringLiteral("e"), QStringLiteral("editor")}, QObject::tr("Pályszerkesztő indítása")});
	parser.addOption({{QStringLiteral("m"), QStringLiteral("map")}, QObject::tr("Pálya szerkesztése"), QStringLiteral("file")});
	parser.addOption({{QStringLiteral("p"), QStringLiteral("play")}, QObject::tr("Pálya lejátszása"), QStringLiteral("file")});


#ifdef QT_NO_DEBUG
	parser.addOption({QStringLiteral("debug"), QObject::tr("Hibakeresési üzenetek megjelenítése")});
#endif

	parser.process(*m_application);

#ifdef QT_NO_DEBUG
	if (parser.isSet(QStringLiteral("debug"))) {
		QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
	} else
		QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false"));
#endif

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
	else if (parser.isSet(QStringLiteral("map"))) {
		m_commandLine = Map;
		m_loadMap = parser.value(QStringLiteral("map"));
	} else if (parser.isSet(QStringLiteral("play"))) {
		m_commandLine = Play;
		m_loadMap = parser.value(QStringLiteral("map"));
	}

	m_arguments = parser.positionalArguments();
}


/**
 * @brief DesktopApplication::initialize
 */

void DesktopApplication::initialize()
{
#ifdef Q_OS_WIN32
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		m_streamO = freopen("CONOUT$", "w", stdout);
		m_streamE = freopen("CONOUT$", "w", stderr);
	}
#endif

#ifdef Q_OS_ANDROID
	AndroidAppender *appender = new AndroidAppender;
#else
	ColorConsoleAppender *appender = new ColorConsoleAppender;
#endif

#ifndef QT_NO_DEBUG
	appender->setFormat(QStringLiteral("%{time}{hh:mm:ss} [%{TypeOne}] %{category} <%{function}> %{message}\n"));
#else
	appender->setFormat(QStringLiteral("%{time}{hh:mm:ss} [%{TypeOne}] %{category} %{message}\n"));
#endif

	cuteLogger->registerAppender(appender);

	qRegisterMetaType<Sound::SoundType>("SoundType");
	qmlRegisterUncreatableType<Sound>("CallOfSuli", 1, 0, "Sound", "Sound is uncreatable");

	createStandardPath();

}


/**
 * @brief DesktopApplication::shutdown
 */

void DesktopApplication::shutdown()
{
#ifdef Q_OS_WIN32
	if (m_streamO != NULL)
		fclose(m_streamO);
	if (m_streamE != NULL)
		fclose(m_streamE);
#endif
}


/**
 * @brief DesktopApplication::performCommandLine
 */

bool DesktopApplication::performCommandLine()
{
	if (m_commandLine == License)
	{
		QFile f(QStringLiteral(":/license.txt"));

		if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
			qCWarning(lcApp).noquote() << QObject::tr("A licensz nem található vagy nem olvasható!");
			return false;
		}

		QByteArray b = f.readAll();

		f.close();

		QTextStream out(stdout);
		out << b << Qt::endl;

		return false;
	}

	return true;
}



/**
 * @brief DesktopApplication::createStandardPath
 */

void DesktopApplication::createStandardPath()
{
	QDir d(Utils::standardPath());

	if (!d.exists()) {
		qCInfo(lcApp).noquote() << QObject::tr("Create directory:") + d.absolutePath();
		d.mkpath(d.absolutePath());
	} else {
		qCDebug(lcApp).noquote() << QObject::tr("Standard path:") << d.absolutePath();
	}
}



/**
 * @brief DesktopApplication::createClient
 * @return
 */

Client *DesktopApplication::createClient()
{
	return new DesktopClient(this, m_application);
}



