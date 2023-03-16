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
#include "classobject.h"
#include "fontimage.h"
#include "gameenemysniper.h"
#include "gamequestioncomponent.h"
#include "gamescene.h"
#include "studentgroup.h"
#include <QFontDatabase>
#include <QDebug>

#include <Qaterial/Qaterial.hpp>

#ifndef QZXING_QML
#define QZXING_QML
#endif

#include <QZXing.h>

#ifdef WITH_BOX2D
#include <box2dplugin.h>
#endif

#ifdef Q_OS_ANDROID
#include <QtAndroid>
//#include "AndroidAppender.h"
#endif

#include "application.h"
#include "../../version/version.h"
#include "../modules/staticmodules.h"



#include "abstractgame.h"
#include "actiongame.h"
#include "gameentity.h"
#include "gameenemysoldier.h"
#include "gameladder.h"
#include "gameobject.h"
#include "gameplayer.h"
#include "gamepickable.h"
#include "gamequestion.h"
#include "mapplay.h"
#include "websocket.h"



const int Application::m_versionMajor = VERSION_MAJOR;
const int Application::m_versionMinor = VERSION_MINOR;
const int Application::m_versionBuild = VERSION_BUILD;
const char *Application::m_version = VERSION_FULL;
Application *Application::m_instance = nullptr;


#ifdef QT_NO_DEBUG
const bool Application::m_debug = false;
#else
const bool Application::m_debug = true;
#endif

#ifndef Q_OS_WASM
#include <QtWebEngine/QtWebEngine>
#endif

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

	QCoreApplication::setApplicationName(QStringLiteral("callofsuli"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("callofsuli"));
	QCoreApplication::setApplicationVersion(m_version);
	QGuiApplication::setApplicationDisplayName(QStringLiteral("Call of Suli"));

	if (!m_instance) {
		LOG_CTRACE("app") << "Create application instance";
		m_instance = this;
	}

#ifndef Q_OS_WASM
	QtWebEngine::initialize();
#endif

	m_application = new QGuiApplication(argc, argv);

	m_engine = new QQmlApplicationEngine(m_application);
}


/**
 * @brief BaseApplication::~BaseApplication
 */

Application::~Application()
{
	LOG_CTRACE("app") << "Destroy Application";

	delete m_engine;
	m_engine = nullptr;

	if (m_client)
		delete m_client;
	m_client = nullptr;

	delete m_application;
	m_application = nullptr;

	m_instance = nullptr;
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
	loadModules();

	m_engine->addImageProvider("font", new FontImage());

	m_engine->rootContext()->setContextProperty("Client", m_client);

	if (!loadResources()) {
		LOG_CERROR("app") << "Failed to load resources";
		return -1;
	}

	if (!loadMainQml()) {
		LOG_CERROR("app") << "Failed to load main qml";
		return -1;
	}

	if (m_engine->rootObjects().isEmpty())
	{
		LOG_CERROR("app") << "Missing root object";
		return -1;
	}


	LOG_CINFO("app") << "Run Application";

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
		LOG_CDEBUG("app") << "Search resources:" << qPrintable(dir);
		QDirIterator it(dir+"/", {"*.cres"});

		while (it.hasNext()) {
			QString realname = it.next();
			LOG_CINFO("app") << "Register resource:" << qPrintable(realname);
			QResource::registerResource(realname);
		}
	}

	loadFonts();

	return true;
}




/**
 * @brief Application::registerQmlTypes
 */

void Application::registerQmlTypes()
{
	LOG_CTRACE("app") << "Register QML types";

	QZXing::registerQMLTypes();

	qmlRegisterUncreatableType<AbstractGame>("CallOfSuli", 1, 0, "AbstractGame", "AbstractGame is uncreatable");
	qmlRegisterUncreatableType<ActionGame>("CallOfSuli", 1, 0, "ActionGame", "ActionGame is uncreatable");
	qmlRegisterUncreatableType<MapPlay>("CallOfSuli", 1, 0, "MapPlay", "MapPlay is uncreatable");
	qmlRegisterUncreatableType<WebSocket>("CallOfSuli", 1, 0, "WebSocket", "WebSocket is uncreatable");
	qmlRegisterUncreatableType<WebSocketReply>("CallOfSuli", 1, 0, "WebSocketReply", "WebSocketReply is uncreatable");
	qmlRegisterUncreatableType<Credential>("CallOfSuli", 1, 0, "Credential", "Credential is uncreatable");

	qmlRegisterType<GameScene>("CallOfSuli", 1, 0, "GameSceneImpl");
	qmlRegisterType<GameObject>("CallOfSuli", 1, 0, "GameObject");
	qmlRegisterType<GameEntity>("CallOfSuli", 1, 0, "GameEntityImpl");
	qmlRegisterType<GameEnemy>("CallOfSuli", 1, 0, "GameEnemyImpl");
	qmlRegisterType<GameEnemySoldier>("CallOfSuli", 1, 0, "GameEnemySoldierImpl");
	qmlRegisterType<GameEnemySniper>("CallOfSuli", 1, 0, "GameEnemySniperImpl");
	qmlRegisterType<GameLadder>("CallOfSuli", 1, 0, "GameLadderImpl");
	qmlRegisterType<GamePlayer>("CallOfSuli", 1, 0, "GamePlayerImpl");
	qmlRegisterType<GamePickable>("CallOfSuli", 1, 0, "GamePickableImpl");
	qmlRegisterType<GameQuestion>("CallOfSuli", 1, 0, "GameQuestionImpl");
	qmlRegisterType<GameQuestionComponent>("CallOfSuli", 1, 0, "GameQuestionComponentImpl");
	qmlRegisterType<SelectableObject>("CallOfSuli", 1, 0, "SelectableObject");
	qmlRegisterType<User>("CallOfSuli", 1, 0, "User");
	qmlRegisterType<UserList>("CallOfSuli", 1, 0, "UserList");
	qmlRegisterType<ClassObject>("CallOfSuli", 1, 0, "ClassObject");
	qmlRegisterType<ClassList>("CallOfSuli", 1, 0, "ClassList");
	qmlRegisterType<StudentGroup>("CallOfSuli", 1, 0, "StudentGroup");
	qmlRegisterType<StudentGroupList>("CallOfSuli", 1, 0, "StudentGroupList");
}

/**
 * @brief Application::loadFonts
 */

void Application::loadFonts()
{
	LOG_CTRACE("app") << "Load fonts";

	const QVector<QString> fontsToLoad = {
		QStringLiteral(":/internal/font/ariblk.ttf"),
		QStringLiteral(":/internal/font/Books.ttf"),
		QStringLiteral(":/internal/font/Material.ttf"),
		QStringLiteral(":/internal/font/School.ttf"),
		QStringLiteral(":/internal/font/Academic.ttf"),
		QStringLiteral(":/internal/font/AcademicI.ttf"),

		QStringLiteral(":/internal/font/rajdhani-bold.ttf"),
		QStringLiteral(":/internal/font/rajdhani-light.ttf"),
		QStringLiteral(":/internal/font/rajdhani-regular.ttf"),
		QStringLiteral(":/internal/font/rajdhani-medium.ttf"),
		QStringLiteral(":/internal/font/rajdhani-semibold.ttf"),

		QStringLiteral(":/internal/font/SpecialElite.ttf"),
		QStringLiteral(":/internal/font/HVD_Peace.ttf"),
		QStringLiteral(":/internal/font/RenegadeMaster.ttf"),
	};

	for (const QString &fontPath : fontsToLoad) {
		if (QFontDatabase::addApplicationFont(fontPath) == -1) {
			LOG_CWARNING("app") << "Failed to load font:" << qPrintable(fontPath);
		} else {
			LOG_CINFO("app") << "Font loaded:" << qPrintable(fontPath);
		}
	}
}


/**
 * @brief Application::loadQaterial
 */

void Application::loadQaterial()
{
	m_engine->addImportPath(QStringLiteral("qrc:/"));

	qaterial::loadQmlResources();
	qaterial::registerQmlTypes();
}


/**
 * @brief Application::loadBox2D
 */

void Application::loadBox2D()
{
#ifdef WITH_BOX2D
	LOG_CTRACE("app") << "Load Box2D";

	Box2DPlugin plugin;
	plugin.registerTypes("Box2D");
	qmlProtectModule("Box2D", 2);
#else
	LOG_CTRACE("app") << "Skip Box2D loading");
#endif
}


/**
 * @brief Application::loadModules
 */

void Application::loadModules()
{
	LOG_CTRACE("app") << "Load modules";

	m_objectiveModules.clear();
	m_storageModules.clear();

	QVector<QStaticPlugin> l = QPluginLoader::staticPlugins();

	foreach (QStaticPlugin ll, l) {
		QObject *o = ll.instance();

		if (!o)
			continue;

		ModuleInterface *i = qobject_cast<ModuleInterface *>(o);

		if (!i)
			continue;

		QString name = i->name();

		if (i->isStorageModule()) {
			LOG_CTRACE("app") << "Load storage module:" << qPrintable(i->name());
			m_storageModules.insert(name, i);
		} else {
			LOG_CTRACE("app") << "Load objective module:" << qPrintable(i->name());
			m_objectiveModules.insert(name, i);
		}

		i->registerQmlTypes();
	}

}



/**
 * @brief Application::storageModules
 * @return
 */

const QHash<QString, ModuleInterface *> &Application::storageModules() const
{
	return m_storageModules;
}




/**
 * @brief Application::objectiveModules
 * @return
 */

const QHash<QString, ModuleInterface *> &Application::objectiveModules() const
{
	return m_objectiveModules;
}


/**
 * @brief Application::debug
 * @return
 */

bool Application::debug()
{
	return m_debug;
}



/**
 * @brief Application::instance
 * @return
 */

Application *Application::instance()
{
	return m_instance;
}



/**
 * @brief Application::messageInfo
 * @param text
 * @param title
 */

void Application::messageInfo(const QString &text, const QString &title) const
{
	if (m_client)
		m_client->messageInfo(text, title);
	else
		LOG_CINFO("app") << qPrintable(QStringLiteral("%1 (%2)").arg(text).arg(title));
}


/**
 * @brief Application::messageWarning
 * @param text
 * @param title
 */

void Application::messageWarning(const QString &text, const QString &title) const
{
	if (m_client)
		m_client->messageWarning(text, title);
	else
		LOG_CWARNING("app") << qPrintable(QStringLiteral("%1 (%2)").arg(text).arg(title));
}


/**
 * @brief Application::messageError
 * @param text
 * @param title
 */

void Application::messageError(const QString &text, const QString &title) const
{
	if (m_client)
		m_client->messageError(text, title);
	else
		LOG_CERROR("app") << qPrintable(QStringLiteral("%1 (%2)").arg(text).arg(title));
}


/**
 * @brief Application::client
 * @return
 */

Client *Application::client() const
{
	return m_client;
}

