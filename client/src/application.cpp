
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
#include "actionrpggame.h"
#include "actionrpgmultiplayergame.h"
#include "classobject.h"
#include "commonsettings.h"
#include "exam.h"
#include "examgame.h"
#include "examresultmodel.h"
#include "fetchmodel.h"
#include "fontimage.h"
#include "gamequestioncomponent.h"
#include "isometricobject.h"
#include "litegame.h"
#include "mapeditor.h"
#include "mapgame.h"
#include "mapplaycampaign.h"
#include "maskedmousearea.h"
#include "offsetmodel.h"
#include "qapplication.h"
#include "qsjsonlistmodel.h"
#include "rpguserwallet.h"
#include "rpgworldlanddata.h"
#include "scorelist.h"
#include "studentgroup.h"
#include "studentmap.h"
#include "studentmaphandler.h"
#include "teacherexam.h"
#include "teachergroup.h"
#include "teachermap.h"
#include "teachermaphandler.h"
#include "testgame.h"
#include "rpggame.h"
#include "tileddebugdraw.h"
#include "tiledobject.h"
#include "tiledscene.h"
#include "tiledspritehandler.h"
#include "tiledvisualitem.h"
#include "userimporter.h"
#include "userloglist.h"
#include "utils_.h"
#include "updater.h"
#include "server.h"
#include <QFontDatabase>
#include <QDebug>

#include <Qaterial/Qaterial.hpp>

#include "application.h"
#include "../modules/staticmodules.h"
#include "utils_.h"


#include "abstractgame.h"
#include "gamequestion.h"
#include "mapplay.h"
#include "httpconnection.h"

// IOS and WASM bug

#if QT_VERSION >= 0x060800 && defined(Q_OS_IOS)
#include <QtQuickEffects/6.8.0/QtQuickEffects/private/qgfxsourceproxy_p.h>
#endif


Application *Application::m_instance = nullptr;

const QString Application::m_userAgent = QStringLiteral("CallOfSuli/%1 (%2; %3)")
										 .arg(Utils::versionNumber().toString())
										 .arg(QSysInfo::prettyProductName())
										 .arg(QSysInfo::currentCpuArchitecture())
										 ;



#ifdef QT_NO_DEBUG
const bool Application::m_debug = false;
#else
const bool Application::m_debug = true;
#endif



/**
 * @brief Application::initialize
 */

void Application::initialize()
{
	QCoreApplication::setApplicationName(QStringLiteral("callofsuli"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("callofsuli"));
	QCoreApplication::setApplicationVersion(Utils::versionNumber().toString());
	QApplication::setApplicationDisplayName(QStringLiteral("Call of Suli"));

	QLocale::setDefault(QLocale(QLocale::Hungarian, QLocale::Hungary));

	// Font display bug
#ifdef Q_OS_WIN
	qputenv("QT_QPA_PLATFORM", "windows:fontengine=gdi");
#endif
}




/**
 * @brief Application::Application
 * @param argc
 * @param argv
 */

Application::Application(QApplication *app)
	: m_application(app)
{
	Q_ASSERT(!m_instance);

	m_instance = this;

	QObject::connect(m_application, &QCoreApplication::aboutToQuit, [this](){
		m_engine.reset();
		m_client.reset();
	});

	m_engine = std::make_unique<QQmlApplicationEngine>();

	/*
#ifndef QT_NO_DEBUG
	QTimer *timer = new QTimer(m_application);
	timer->setInterval(100);
	QObject::connect(timer, &QTimer::timeout, m_application, [](){
		unsigned long currRealMem = 0;
		unsigned long currVirtMem = 0;

		//Utils::getMemory(&currRealMem, nullptr, &currVirtMem);

		//LOG_CDEBUG("app") << "USED MEMORY:" << currRealMem << currVirtMem;

		Q_ASSERT(currRealMem < (7*1024*1024));		// 7GB
		Q_ASSERT(currVirtMem < (7*1024*1024));		// 7GB

		//LOG_CTRACE("app") << "USED MEMORY:" << Utils::getCurrentRSS();

});
	timer->start();
#endif
*/
}


/**
 * @brief BaseApplication::~BaseApplication
 */

Application::~Application()
{
	LOG_CTRACE("app") << "Destroy Application" << this;
}


/**
 * @brief BaseApplication::run
 * @return
 */

int Application::run()
{
	registerQmlTypes();
	loadQaterial();
	loadModules();

	m_client.reset(createClient());
	m_engine->addImageProvider(QStringLiteral("font"), std::move(new FontImage()));

	m_engine->rootContext()->setContextProperty("Client", m_client.get());

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

	const int r = m_application->exec();

	LOG_CINFO("app") << "Application finished with code" << r;

	return r;
}




/**
 * @brief Application::application
 * @return
 */

QApplication *Application::application() const
{
	return m_application;
}


/**
 * @brief Application::engine
 * @return
 */

QQmlApplicationEngine *Application::engine() const
{
	return m_engine.get();
}


const QVersionNumber Application::version()
{
	return Utils::versionNumber();
}

/**
 * @brief Application::loadMainQml
 * @return
 */

bool Application::loadMainQml()
{
	const QUrl url(QStringLiteral("qrc:/main.qml"));
	QObject::connect(m_engine.get(), &QQmlApplicationEngine::objectCreated,
					 m_application, [url](QObject *obj, const QUrl &objUrl) {
		if (!obj && url == objUrl)
			QCoreApplication::exit(-1);

#if defined(Q_OS_ANDROID)
		QNativeInterface::QAndroidApplication::hideSplashScreen();
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
	searchList.append(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation));
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
	searchList.append(QStandardPaths::standardLocations(QStandardPaths::AppDataLocation));

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

	qmlRegisterUncreatableType<AbstractGame>("CallOfSuli", 1, 0, "AbstractGame", "AbstractGame is uncreatable");
	qmlRegisterUncreatableType<ActionRpgGame>("CallOfSuli", 1, 0, "ActionRpgGame", "ActionRpgGame is uncreatable");
	qmlRegisterUncreatableType<ActionRpgMultiplayerGame>("CallOfSuli", 1, 0, "ActionRpgMultiplayerGame", "ActionRpgMultiplayerGame is uncreatable");
	qmlRegisterUncreatableType<Downloader>("CallOfSuli", 1, 0, "Downloader", "Downloader is uncreatable");
	qmlRegisterUncreatableType<ExamGame>("CallOfSuli", 1, 0, "ExamGame", "ExamGame is uncreatable");
	qmlRegisterUncreatableType<EditorUndoStack>("CallOfSuli", 1, 0, "EditorUndoStack", "EditorUndoStack is uncreatable");
	qmlRegisterUncreatableType<HttpConnection>("CallOfSuli", 1, 0, "HttpConnection", "HttpConnection is uncreatable");
	qmlRegisterUncreatableType<HttpReply>("CallOfSuli", 1, 0, "HttpConnectionReply", "HttpConnectionReply is uncreatable");
	qmlRegisterUncreatableType<IsometricPlayer>("CallOfSuli", 1, 0, "IsometricPlayer", "IsometricPlayer is uncreatable");
	qmlRegisterUncreatableType<IsometricEnemy>("CallOfSuli", 1, 0, "IsometricEnemy", "IsometricEnemy is uncreatable");
	qmlRegisterUncreatableType<LiteGame>("CallOfSuli", 1, 0, "LiteGame", "LiteGame is uncreatable");
	qmlRegisterUncreatableType<MapEditorChapter>("CallOfSuli", 1, 0, "MapEditorChapter", "MapEditorChapter is uncreatable");
	qmlRegisterUncreatableType<MapEditorImage>("CallOfSuli", 1, 0, "MapEditorImage", "MapEditorImage is uncreatable");
	qmlRegisterUncreatableType<MapEditorInventory>("CallOfSuli", 1, 0, "MapEditorInventory", "MapEditorInventory is uncreatable");
	qmlRegisterUncreatableType<MapEditorMap>("CallOfSuli", 1, 0, "MapEditorMap", "MapEditorMap is uncreatable");
	qmlRegisterUncreatableType<MapEditorMission>("CallOfSuli", 1, 0, "MapEditorMission", "MapEditorMission is uncreatable");
	qmlRegisterUncreatableType<MapEditorMissionLevel>("CallOfSuli", 1, 0, "MapEditorMissionLevel", "MapEditorMissionLevel is uncreatable");
	qmlRegisterUncreatableType<MapEditorObjective>("CallOfSuli", 1, 0, "MapEditorObjective", "MapEditorObjective is uncreatable");
	qmlRegisterUncreatableType<MapEditorStorage>("CallOfSuli", 1, 0, "MapEditorStorage", "MapEditorStorage is uncreatable");
	qmlRegisterUncreatableType<MapPlay>("CallOfSuli", 1, 0, "MapPlay", "MapPlay is uncreatable");
	qmlRegisterUncreatableType<MapPlayCampaign>("CallOfSuli", 1, 0, "MapPlayCampaign", "MapPlayCampaign is uncreatable");
	qmlRegisterUncreatableType<MapPlayMission>("CallOfSuli", 1, 0, "MapPlayMission", "MapPlayMission is uncreatable");
	qmlRegisterUncreatableType<MapPlayMissionLevel>("CallOfSuli", 1, 0, "MapPlayMissionLevel", "MapPlayMissionLevel is uncreatable");
	qmlRegisterUncreatableType<RpgActiveControlObject>("CallOfSuli", 1, 0, "RpgActiveControl", "RpgActiveControl is uncreatable");
	qmlRegisterUncreatableType<RpgArmory>("CallOfSuli", 1, 0, "RpgArmory", "RpgArmory is uncreatable");
	qmlRegisterUncreatableType<RpgEnemy>("CallOfSuli", 1, 0, "RpgEnemy", "RpgEnemy is uncreatable");
	qmlRegisterUncreatableType<RpgPlayer>("CallOfSuli", 1, 0, "RpgPlayer", "RpgPlayer is uncreatable");
	qmlRegisterUncreatableType<RpgMarket>("CallOfSuli", 1, 0, "RpgMarket", "RpgMarket is uncreatable");
	qmlRegisterUncreatableType<RpgMarketExtendedInfo>("CallOfSuli", 1, 0, "RpgMarketExtendedInfo", "RpgWalletExtendedInfo is uncreatable");
	qmlRegisterUncreatableType<RpgQuest>("CallOfSuli", 1, 0, "RpgQuest", "RpgQuest is uncreatable");
	qmlRegisterUncreatableType<RpgUserWallet>("CallOfSuli", 1, 0, "RpgUserWallet", "RpgUserWallet is uncreatable");
	qmlRegisterUncreatableType<RpgWeapon>("CallOfSuli", 1, 0, "RpgWeapon", "RpgWeapon is uncreatable");
	qmlRegisterUncreatableType<Server>("CallOfSuli", 1, 1, "Server", "Server is uncreatable");
	qmlRegisterUncreatableType<Sound>("CallOfSuli", 1, 1, "Sound", "Server is uncreatable");
	qmlRegisterUncreatableType<TestGame>("CallOfSuli", 1, 0, "TestGame", "TestGame is uncreatable");
	qmlRegisterUncreatableType<TiledObjectBody>("CallOfSuli", 1, 0, "TiledObjectBody", "TiledObjectBody is uncreatable");
	qmlRegisterUncreatableType<TiledGame>("CallOfSuli", 1, 0, "TiledGame", "TiledGame is uncreatable");
	qmlRegisterUncreatableType<TiledGameDefinition>("CallOfSuli", 1, 0, "TiledGameDefinition", "TiledGameDefinition is uncreatable");
	qmlRegisterUncreatableType<TiledSceneDefinition>("CallOfSuli", 1, 0, "TiledSceneDefinition", "TiledGameDefinition is uncreatable");
	qmlRegisterUncreatableType<TiledWeapon>("CallOfSuli", 1, 0, "TiledWeapon", "TiledWeapon is uncreatable");
	qmlRegisterUncreatableType<Updater>("CallOfSuli", 1, 0, "Updater", "Updater is uncreatable");
	qmlRegisterUncreatableType<Utils>("CallOfSuli", 1, 1, "Utils", "Utils is uncreatable");
	qmlRegisterUncreatableType<WebSocket>("CallOfSuli", 1, 0, "WebSocket", "WebSocket is uncreatable");


	qmlRegisterUncreatableType<ContextHelperData>("CallOfSuli", 1, 0, "ContextHelperData", "ContextHelperData is uncreatable");
	qmlRegisterUncreatableType<Credential>("CallOfSuli", 1, 0, "Credential", "Credential is uncreatable");
	qmlRegisterUncreatableType<GameMap>("CallOfSuli", 1, 0, "GameMap", "GameMap is uncreatable");
	qmlRegisterUncreatableType<GameMapMission>("CallOfSuli", 1, 0, "GameMapMission", "GameMapMission is uncreatable");
	qmlRegisterUncreatableType<GameMapMissionLevel>("CallOfSuli", 1, 0, "GameMapMissionLevel", "GameMapMissionLevel is uncreatable");
	qmlRegisterUncreatableType<Rank>("CallOfSuli", 1, 0, "Rank", "Rank is uncreatable");
	qmlRegisterUncreatableType<RpgConfig>("CallOfSuli", 1, 0, "RpgConfig", "RpgConfig is uncreatable");
	qmlRegisterUncreatableType<RpgPlayerConfig>("CallOfSuli", 1, 0, "RpgPlayerConfig", "RpgPlayerConfig is uncreatable");
	qmlRegisterUncreatableType<RpgGameData::Weapon>("CallOfSuli", 1, 0, "RpgWeaponType", "RpgWeapon is uncreatable");


	qmlRegisterUncreatableMetaObject(CallOfSuli::staticMetaObject, "CallOfSuli", 1, 0, "NotificationType", "NotificationType is uncreatable");

	qmlRegisterType<QSJsonListModel>("QSyncable", 1, 0, "QSJsonListModel");
	qmlRegisterType<QSListModel>("QSyncable", 1, 0, "QSListModel");

	qmlRegisterType<BaseMap>("CallOfSuli", 1, 0, "BaseMap");
	qmlRegisterType<Campaign>("CallOfSuli", 1, 0, "Campaign");
	qmlRegisterType<CampaignList>("CallOfSuli", 1, 0, "CampaignList");
	qmlRegisterType<ClassList>("CallOfSuli", 1, 0, "ClassList");
	qmlRegisterType<ClassObject>("CallOfSuli", 1, 0, "ClassObject");
	qmlRegisterType<Exam>("CallOfSuli", 1, 0, "Exam");
	qmlRegisterType<ExamList>("CallOfSuli", 1, 0, "ExamList");
	qmlRegisterType<ExamResultModel>("CallOfSuli", 1, 0, "ExamResultModel");
	qmlRegisterType<ExamScanData>("CallOfSuli", 1, 0, "ExamScanData");
	qmlRegisterType<ExamScanDataList>("CallOfSuli", 1, 0, "ExamScanDataList");
	qmlRegisterType<ExamUser>("CallOfSuli", 1, 0, "ExamUser");
	qmlRegisterType<FetchModel>("CallOfSuli", 1, 0, "FetchModelImpl");
	qmlRegisterType<GameQuestion>("CallOfSuli", 1, 0, "GameQuestionImpl");
	qmlRegisterType<GameQuestionComponent>("CallOfSuli", 1, 0, "GameQuestionComponentImpl");
	qmlRegisterType<IsometricObject>("CallOfSuli", 1, 0, "IsometricObjectImpl");
	qmlRegisterType<Grade>("CallOfSuli", 1, 0, "Grade");
	qmlRegisterType<GradeList>("CallOfSuli", 1, 0, "GradeList");
	qmlRegisterType<GradingConfig>("CallOfSuli", 1, 0, "GradingConfig");
	qmlRegisterType<MapEditor>("CallOfSuli", 1, 0, "MapEditor");
	qmlRegisterType<MapGame>("CallOfSuli", 1, 0, "MapGame");
	qmlRegisterType<MapGameList>("CallOfSuli", 1, 0, "MapGameList");
	qmlRegisterType<MaskedMouseArea>("CallOfSuli", 1, 0, "MaskedMouseArea");
	qmlRegisterType<OffsetModel>("CallOfSuli", 1, 0, "OffsetModelImpl");
	qmlRegisterType<RpgUserWalletList>("CallOfSuli", 1, 0, "RpgUserWalletList");
	qmlRegisterType<ScoreList>("CallOfSuli", 1, 0, "ScoreListImpl");
	qmlRegisterType<SelectableObject>("CallOfSuli", 1, 0, "SelectableObject");
	qmlRegisterType<StudentCampaignOffsetModel>("CallOfSuli", 1, 0, "StudentCampaignOffsetModelImpl");
	qmlRegisterType<StudentGroup>("CallOfSuli", 1, 0, "StudentGroup");
	qmlRegisterType<StudentGroupList>("CallOfSuli", 1, 0, "StudentGroupList");
	qmlRegisterType<StudentMap>("CallOfSuli", 1, 0, "StudentMap");
	qmlRegisterType<StudentMapHandler>("CallOfSuli", 1, 0, "StudentMapHandler");
	qmlRegisterType<StudentMapList>("CallOfSuli", 1, 0, "StudentMapList");
	qmlRegisterType<Task>("CallOfSuli", 1, 0, "Task");
	qmlRegisterType<TaskList>("CallOfSuli", 1, 0, "TaskList");
	qmlRegisterType<TeacherExam>("CallOfSuli", 1, 0, "TeacherExam");
	qmlRegisterType<TeacherGroup>("CallOfSuli", 1, 0, "TeacherGroup");
	qmlRegisterType<TeacherGroupCampaignResultModel>("CallOfSuli", 1, 0, "TeacherGroupCampaignResultModel");
	qmlRegisterType<TeacherGroupFreeMap>("CallOfSuli", 1, 0, "TeacherGroupFreeMap");
	qmlRegisterType<TeacherGroupFreeMapList>("CallOfSuli", 1, 0, "TeacherGroupFreeMapList");
	qmlRegisterType<TeacherGroupList>("CallOfSuli", 1, 0, "TeacherGroupList");
	qmlRegisterType<TeacherGroupResultModel>("CallOfSuli", 1, 0, "TeacherGroupResultModel");
	qmlRegisterType<TeacherMap>("CallOfSuli", 1, 0, "TeacherMap");
	qmlRegisterType<TeacherMapHandler>("CallOfSuli", 1, 0, "TeacherMapHandler");
	qmlRegisterType<TeacherMapList>("CallOfSuli", 1, 0, "TeacherMapList");
	qmlRegisterType<RpgGame>("CallOfSuli", 1, 0, "RpgGameImpl");
	qmlRegisterType<RpgUserWorld>("CallOfSuli", 1, 0, "RpgUserWorld");
	qmlRegisterType<RpgWorldLandData>("CallOfSuli", 1, 0, "RpgWorldLandData");
	qmlRegisterType<RpgWorldLandDataList>("CallOfSuli", 1, 0, "RpgWorldLandDataList");
	qmlRegisterType<TiledDebugDraw>("CallOfSuli", 1, 0, "TiledDebugDrawImpl");
	qmlRegisterType<TiledObject>("CallOfSuli", 1, 0, "TiledObjectImpl");
	qmlRegisterType<TiledScene>("CallOfSuli", 1, 0, "TiledSceneImpl");
	qmlRegisterType<TiledVisualItem>("CallOfSuli", 1, 0, "TiledVisualItemImpl");
	qmlRegisterType<TiledSpriteHandler>("CallOfSuli", 1, 0, "TiledSpriteHandlerImpl");
	qmlRegisterType<User>("CallOfSuli", 1, 0, "User");
	qmlRegisterType<UserImporter>("CallOfSuli", 1, 0, "UserImporter");
	qmlRegisterType<UserList>("CallOfSuli", 1, 0, "UserList");
	qmlRegisterType<UserLogList>("CallOfSuli", 1, 0, "UserLogListImpl");

	// IOS and WASM bug

#if QT_VERSION >= 0x060800 && defined(Q_OS_IOS)
	qmlRegisterType<QGfxSourceProxy>(QByteArrayLiteral("Qt5Compat.GraphicalEffects.private"), 1, 0, "SourceProxy");
#endif
}

/**
 * @brief Application::loadFonts
 */

void Application::loadFonts()
{
	LOG_CTRACE("app") << "Load fonts";

	static const QVector<QString> fontsToLoad = {
		QStringLiteral(":/internal/font/Books.ttf"),
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

		QStringLiteral(":/internal/font/UbuntuMono-Regular.ttf"),
		QStringLiteral(":/internal/font/UbuntuMono-Italic.ttf"),
	};

	for (const QString &fontPath : std::as_const(fontsToLoad)) {
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

	qaterial::setDefaultFontFamily(QStringLiteral("Rajdhani"));

	qaterial::loadQmlResources();
	qaterial::registerQmlTypes();
}




/**
 * @brief Application::loadModules
 */

void Application::loadModules()
{
	LOG_CTRACE("app") << "Load modules";

	m_objectiveModules.clear();
	m_storageModules.clear();

	const QVector<QStaticPlugin> &l = QPluginLoader::staticPlugins();

	for (const QStaticPlugin &ll : l) {
		ModuleInterface *i = qobject_cast<ModuleInterface *>(ll.instance());

		if (!i)
			continue;

		if (i->types().testFlag(ModuleInterface::Storage)) {
			LOG_CTRACE("app") << "Load storage module:" << qPrintable(i->name());
			m_storageModules.insert(i->name(), i);
		} else {
			LOG_CTRACE("app") << "Load objective module:" << qPrintable(i->name());
			m_objectiveModules.insert(i->name(), i);
		}

		i->registerQmlTypes();
	}

}





/**
 * @brief Application::userAgent
 * @return
 */

const QString &Application::userAgent()
{
	return m_userAgent;
}




/**
 * @brief Application::userAgentSign
 * @param content
 * @return
 */

const QByteArray Application::userAgentSign(const QByteArray &content)
{
	static const QString fname(":/sign/agent_sign.dat");
	static QByteArray key;
	static bool isFirst = true;

	if (isFirst) {
		QFile f(fname);

		if (f.open(QIODevice::ReadOnly)) {
			key = f.readAll();
			f.close();
		}

		isFirst = false;
	}

	if (key.isEmpty())
		return {};

	return QMessageAuthenticationCode::hash(content, key, QCryptographicHash::Sha3_256).toBase64();
}




/**
 * @brief Application::commandLineData
 * @return
 */

const QString &Application::commandLineData() const
{
	return m_commandLineData;
}


/**
 * @brief Application::selectUrl
 * @param url
 */

void Application::selectUrl(const QUrl &url)
{
	LOG_CDEBUG("app") << "Select APP url:" << url;

	if (!m_client) {
		LOG_CERROR("app") << "Missing client";
		return;
	}

	m_client->notifyWindow();

	if (url.host().isEmpty())
		return;

	m_client->setParseUrl(url);

	QMetaObject::invokeMethod(m_client.get(), &Client::parseUrl, Qt::QueuedConnection);
}



/**
 * @brief Application::commandLine
 * @return
 */

const Application::CommandLine &Application::commandLine() const
{
	return m_commandLine;
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
		LOG_CINFO("app") << qPrintable(QStringLiteral("%1 (%2)").arg(text, title));
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
		LOG_CWARNING("app") << qPrintable(QStringLiteral("%1 (%2)").arg(text, title));
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
		LOG_CERROR("app") << qPrintable(QStringLiteral("%1 (%2)").arg(text, title));
}


/**
 * @brief Application::client
 * @return
 */

Client *Application::client() const
{
	return m_client.get();
}

