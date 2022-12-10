/*
 * ---- Call of Suli ----
 *
 * baseapplication.cpp
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * BaseApplication
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

#include "Logger.h"
#include <QFontDatabase>
#include <QDebug>

#include <Qaterial/Qaterial.hpp>

#ifdef WITH_BOX2D
#include <box2dplugin.h>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
//#include "AndroidAppender.h"
#endif

#include "application.h"
#include "../../version/version.h"

const int Application::m_versionMajor = VERSION_MAJOR;
const int Application::m_versionMinor = VERSION_MINOR;
const int Application::m_versionBuild = VERSION_BUILD;
const char *Application::m_version = VERSION_FULL;

Q_LOGGING_CATEGORY(lcApp, "app.application")

/**
 * @brief Application::Application
 * @param argc
 * @param argv
 */

Application::Application(int &argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	QCoreApplication::setApplicationName("callofsuli");
	QCoreApplication::setOrganizationDomain("callofsuli");
	QCoreApplication::setApplicationVersion(m_version);
	QGuiApplication::setApplicationDisplayName("Call of Suli");

	m_application = new QGuiApplication(argc, argv);
	m_engine = new QQmlApplicationEngine;

	cuteLoggerInstance()->logToGlobalInstance("app.application", true);
	cuteLoggerInstance()->logToGlobalInstance("qaterial.utils", true);
	cuteLoggerInstance()->logToGlobalInstance("app.client", true);
	cuteLoggerInstance()->logToGlobalInstance("qml", true);
}


/**
 * @brief BaseApplication::~BaseApplication
 */

Application::~Application()
{
	qCDebug(lcApp).noquote() << QObject::tr("Destroy Application");

	delete m_engine;

	if (m_client)
		delete m_client;

	delete m_application;
}


/**
 * @brief BaseApplication::run
 * @return
 */

int Application::run()
{
	m_client = createClient();

	Q_ASSERT(m_client);

	registerQmlTypes();
	loadQaterial();
	loadBox2D();

	qCDebug(lcApp).noquote() << QObject::tr("Load main qml");

#ifdef QT_NO_DEBUG
	m_engine->rootContext()->setContextProperty("DEBUG_MODE", QVariant(false));
#else
	m_engine->rootContext()->setContextProperty("DEBUG_MODE", QVariant(true));
#endif

	m_engine->rootContext()->setContextProperty("Client", m_client);

	if (!loadResources()) {
		qCCritical(lcApp).noquote() << QObject::tr("Failed to load resources");
		return -1;
	}

	if (!loadMainQml()) {
		qCCritical(lcApp).noquote() << QObject::tr("Failed to load main qml");
		return -1;
	}

	if (m_engine->rootObjects().isEmpty())
	{
		qCCritical(lcApp).noquote() << QObject::tr("Missing root object");
		return -1;
	}



	qCDebug(lcApp).noquote() << QObject::tr("Run Application");

	return m_application->exec();
}



int Application::versionMajor()
{
	return m_versionMajor;
}

int Application::versionMinor()
{
	return m_versionMinor;
}

int Application::versionBuild()
{
	return m_versionBuild;
}

const char *Application::version() const
{
	return m_version;
}


/**
 * @brief Application::application
 * @return
 */

QGuiApplication *Application::application() const
{
	return m_application;
}


/**
 * @brief Application::engine
 * @return
 */

QQmlApplicationEngine *Application::engine() const
{
	return m_engine;
}


/**
 * @brief Application::loadMainQml
 * @return
 */

bool Application::loadMainQml()
{
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(m_engine, &QQmlApplicationEngine::objectCreated,
					 m_application, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);

#ifdef Q_OS_ANDROID
		QtAndroid::hideSplashScreen();
#endif
	}, Qt::QueuedConnection);

	m_engine->load(url);

	return true;
}


/**
 * @brief Application::loadResources
 */

bool Application::loadResources()
{
	QStringList searchList;

#ifdef Q_OS_ANDROID
	searchList.append("assets:");
	searchList.append(QStandardPaths::standardLocations(QStandardPaths::HomeLocation));
	searchList.append(QStandardPaths::standardLocations(QStandardPaths::DataLocation));
#else
	QString binDir = QCoreApplication::applicationDirPath();

	searchList.append(binDir);
	searchList.append(binDir+"/share");

#ifndef Q_OS_IOS
	searchList.append(binDir+"/../share");
	searchList.append(binDir+"/../../share");
	searchList.append(binDir+"/../../../share");
#endif

#ifndef QT_NO_DEBUG
	searchList.append(binDir+"/../callofsuli/share");
	searchList.append(binDir+"/../../callofsuli/share");
	searchList.append(binDir+"/../../../callofsuli/share");
#endif
	searchList.append(QStandardPaths::standardLocations(QStandardPaths::DataLocation));

	searchList.removeDuplicates();
#endif

	foreach (QString dir, searchList)
	{
		qCDebug(lcApp).noquote() << QObject::tr("Search resources: ")+dir;
		QDirIterator it(dir+"/", {"*.cres"});

		while (it.hasNext()) {
			QString realname = it.next();
			qCInfo(lcApp).noquote() << QObject::tr("Register resource: %1").arg(realname);
			QResource::registerResource(realname);
		}
	}

	loadFonts();

	return true;
}


/**
 * @brief Application::createClient
 * @return
 */

Client *Application::createClient()
{
	return new Client(m_application);
}



/**
 * @brief Application::registerQmlTypes
 */

void Application::registerQmlTypes()
{
	qCDebug(lcApp).noquote() << QObject::tr("Register QML types");

}

/**
 * @brief Application::loadFonts
 */

void Application::loadFonts()
{
	qCDebug(lcApp).noquote() << QObject::tr("Load fonts");

	const QVector<QString> fontsToLoad = {
		":/internal/font/ariblk.ttf",
		":/internal/font/Books.ttf",
		":/internal/font/Material.ttf",
		":/internal/font/School.ttf",
		":/internal/font/Academic.ttf",
		":/internal/font/AcademicI.ttf",

		":/internal/font/rajdhani-bold.ttf",
		":/internal/font/rajdhani-light.ttf",
		":/internal/font/rajdhani-regular.ttf",
		":/internal/font/rajdhani-medium.ttf",
		":/internal/font/rajdhani-semibold.ttf",

		":/internal/font/SpecialElite.ttf",
		":/internal/font/HVD_Peace.ttf",
		":/internal/font/RenegadeMaster.ttf",
	};

	for (const QString &fontPath : fontsToLoad) {
		if (QFontDatabase::addApplicationFont(fontPath) == -1) {
			qCWarning(lcApp).noquote() << QObject::tr("Failed to load font: %1").arg(fontPath);
		} else {
			qCInfo(lcApp).noquote() << QObject::tr("Font loaded: %1").arg(fontPath);
		}
	}
}


/**
 * @brief Application::loadQaterial
 */

void Application::loadQaterial()
{
	m_engine->addImportPath("qrc:/");

	qaterial::loadQmlResources();
	qaterial::registerQmlTypes();
}


/**
 * @brief Application::loadBox2D
 */

void Application::loadBox2D()
{
#ifdef WITH_BOX2D
	qCDebug(lcApp).noquote() << QObject::tr("Load Box2D");

	Box2DPlugin plugin;
	plugin.registerTypes("Box2D");
	qmlProtectModule("Box2D", 2);
#else
	qCDebug(lcApp).noquote() << QObject::tr("Skip Box2D loading");
#endif
}

Client *Application::client() const
{
	return m_client;
}

