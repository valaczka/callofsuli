/*
 * ---- Call of Suli ----
 *
 * client.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Client
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

#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QMediaPlaylist>
#include <QPluginLoader>
#include <QQmlContext>
#include <QQuickItem>
#include <QResource>
#include <QStandardPaths>
#include <RollingFileAppender.h>

#include "../Bacon2D-static/qml-box2d/box2dplugin.h"
#include "../Bacon2D-static/src/plugins.h"

#include "../../version/buildnumber.h"
#include "abstractactivity.h"
#include "androidshareutils.h"
#include "cosclient.h"
#include "cosdownloader.h"
#include "cosgame.h"
#include "cosmessage.h"
#include "gameactivity.h"
#include "gameenemy.h"
#include "gameenemysoldier.h"
#include "gameenemysniper.h"
#include "gameentity.h"
#include "gameladder.h"
#include "gamemapmodel.h"
#include "gamematch.h"
#include "gamepickable.h"
#include "gameplayer.h"
#include "gamequestion.h"
#include "gamescene.h"
#include "gameterrain.h"
#include "maplistobject.h"
#include "mapeditor.h"
#include "objectlistmodelobject.h"
#include "objectlistmodel.h"
#include "profile.h"
#include "servers.h"
#include "serverobject.h"
#include "serversettings.h"
#include "studentmaps.h"
#include "teachergroups.h"
#include "teachermaps.h"
#include "tiledpaintedlayer.h"
#include "variantmapmodel.h"


Client* Client::m_clientInstance = nullptr;
QList<TerrainData> Client::m_availableTerrains;
QVariantMap Client::m_characterData;
QStringList Client::m_musicList;
QStringList Client::m_medalIconList;
QHash<QString, ModuleInterface *> Client::m_moduleObjectiveList;
QHash<QString, ModuleInterface *> Client::m_moduleStorageList;
QStringList Client::m_positionalArgumentsToProcess = QStringList();
QString Client::m_guiLoad = "";

Client::Client(QObject *parent) : QObject(parent)
{
	m_connectionState = Standby;
	m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest);
	m_timer = new QTimer(this);

	m_userRoles = CosMessage::RoleGuest;
	m_userXP = 0;
	m_userRank = 0;
	m_userRankImage = "";
	m_userRankLevel = -1;
	m_userNickName = "";
	m_userPlayerCharacter = "";
	m_userPicture = "";

	m_cosMessage = nullptr;

	m_serverDataDir = "";

	m_sfxVolume = 1.0;

	m_registrationEnabled = false;

	m_sslErrorSignalHandlerConnected = false;
	m_forcedLandscape = false;
	m_serverUuid = "";

	m_rootContext = nullptr;

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)
	m_singleInstance = nullptr;
#endif


	m_socket->moveToThread(&m_socketThread);
	connect(&m_socketThread, &QThread::finished, m_socket, &QObject::deleteLater);

	m_socketThread.start();

	QStringList dbList = {"objectiveDb"};

	foreach (QString s, dbList) {
		if (QSqlDatabase::contains(s))
			QSqlDatabase::removeDatabase(s);
	}

	m_clientSound = new CosSound();
	m_clientSound->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished, m_clientSound, &QObject::deleteLater);
	connect(m_clientSound, &CosSound::volumeSfxChanged, this, &Client::setSfxVolumeInt);

	m_workerThread.start();

	QMetaObject::invokeMethod(m_clientSound, "init", Qt::QueuedConnection);


	connect(AndroidShareUtils::instance(), &AndroidShareUtils::storagePermissionsDenied, this, &Client::storagePermissionsDenied);
	connect(AndroidShareUtils::instance(), &AndroidShareUtils::storagePermissionsGranted, this, &Client::storagePermissionsGranted);
	connect(AndroidShareUtils::instance(), &AndroidShareUtils::mediaPermissionsDenied, this, &Client::mediaPermissionsDenied);
	connect(AndroidShareUtils::instance(), &AndroidShareUtils::mediaPermissionsGranted, this, &Client::mediaPermissionsGranted);

	qRegisterMetaType<QAbstractSocket::SocketError>();
	qRegisterMetaType<QAbstractSocket::SocketState>();
	qRegisterMetaType<QList<QSslError>>();


	connect(m_socket, &QWebSocket::connected, this, &Client::onSocketConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &Client::onSocketDisconnected);
	connect(m_socket, &QWebSocket::stateChanged, this, &Client::onSocketStateChanged);
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, &Client::onSocketBinaryFrameReceived);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onSocketBinaryMessageReceived);
	//	connect(m_socket, &QWebSocket::bytesWritten, this, &Client::onSocketBytesWritten);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &Client::onSocketError);

	connect(m_timer, &QTimer::timeout, this, &Client::socketPing);
	m_timer->start(5000);
}

/**
 * @brief Client::~Client
 */

Client::~Client()
{
	m_workerThread.quit();
	m_workerThread.wait();

	m_socketThread.quit();
	m_socketThread.wait();

	delete m_timer;

	if (m_cosMessage)
		delete m_cosMessage;

}


/**
 * @brief Client::initialize
 */

void Client::initialize()
{
	QCoreApplication::setApplicationName("callofsuli");
	QCoreApplication::setOrganizationDomain("callofsuli");
	QCoreApplication::setApplicationVersion(_VERSION_FULL);

	srand(time(NULL));

	Box2DPlugin box2dplugin;
	box2dplugin.registerTypes("Box2D");

	Plugins plugin;
	plugin.registerTypes("Bacon2D");

	Q_INIT_RESOURCE(Bacon2D_static);
}


/**
 * @brief Client::loadModules
 */

void Client::loadModules()
{
	m_moduleObjectiveList.clear();
	m_moduleStorageList.clear();

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
			qDebug().noquote() << "Load storage module" << i->name();
			m_moduleStorageList[name] = i;
		} else {
			qDebug().noquote() << "Load objective module" << i->name();
			m_moduleObjectiveList[name] = i;
		}

		i->registerQmlTypes();
	}


	// TODO: load d
}






/**
 * @brief Client::commandLineParse
 * @param app
 * @return
 */

QString Client::commandLineParse(QCoreApplication &app)
{
	QCommandLineParser parser;
	parser.setApplicationDescription(QString::fromUtf8("Call of Suli – Copyright © 2012-2022 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOption({{"t", "terrain"}, tr("Terepfájl <file> adatainak lekérdezése"), "file"});
	parser.addOption({"license", tr("Licensz")});
	parser.addOption({{"l", "log"}, tr("Naplózás <file> fájlba"), "file"});
	parser.addOption({{"e", "editor"}, tr("Pályszerkesztő indítása")});
	parser.addOption({{"m", "map"}, tr("Pálya szerkesztése"), "file"});
	parser.addOption({{"p", "play"}, tr("Pálya lejátszása"), "file"});


#ifdef QT_NO_DEBUG
	parser.addOption({"debug", tr("Hibakeresési üzenetek megjelenítése")});
#endif

#ifdef SQL_DEBUG
	QLoggingCategory::setFilterRules(QStringLiteral("sql.debug=true"));
#else
	QLoggingCategory::setFilterRules(QStringLiteral("sql.debug=false"));
#endif

#ifdef QT_NO_DEBUG
	if (parser.isSet("debug")) {
		QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
		qInfo() << "DEBUG TRUE";
	} else
		QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false"));
#endif


	parser.process(app);

	if (parser.isSet("license"))
		return "license";


	if (parser.isSet("terrain")) {
		QString tmx = parser.value("terrain");

		QTextStream out(stdout);
		out << terrainDataToJson(tmx) << Qt::endl;

		return "terrain";
	}

	QString logFile;

	if (parser.isSet("log")) logFile = parser.value("log");


	if (!logFile.isEmpty()) {
		RollingFileAppender* appender = new RollingFileAppender(logFile);
		appender->setFormat("%{time}{yyyy-MM-dd hh:mm:ss} [%{TypeOne}] %{message}\n");
		appender->setDatePattern(RollingFileAppender::DailyRollover);
		cuteLogger->registerAppender(appender);
	}



	if (parser.isSet("editor"))
		m_guiLoad = "map:";
	else if (parser.isSet("map"))
		m_guiLoad = "map:"+parser.value("map");
	else if (parser.isSet("play"))
		m_guiLoad = "play:"+parser.value("play");

	m_positionalArgumentsToProcess = parser.positionalArguments();

	return "";
}


/**
 * @brief Client::registerResource
 * @param filename
 * @return
 */

void Client::registerResources()
{

	QStringList searchList;
#ifdef Q_OS_ANDROID
	searchList.append("assets:");
	searchList << QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
	searchList << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
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
#endif
	searchList << QStandardPaths::standardLocations(QStandardPaths::DataLocation);

	searchList.removeDuplicates();

#endif

	foreach (QString dir, searchList)
	{
		qDebug().noquote() << tr("Search resources: ")+dir;
		QDirIterator it(dir+"/", QStringList() << "*.cres");

		while (it.hasNext()) {
			QString realname = it.next();
			qInfo().noquote() << tr("Registered resource: ")+realname;
			QResource::registerResource(realname);
		}
	}
}



/**
 * @brief Client::registerTypes
 */

void Client::registerTypes()
{
	qRegisterMetaType<CosMessage::CosClass>("CosClass");
	qRegisterMetaType<CosMessage::CosMessageError>("CosMessageError");
	qRegisterMetaType<CosMessage::CosMessageServerError>("CosMessageServerError");
	qRegisterMetaType<CosMessage::CosMessageType>("CosMessageType");
	qRegisterMetaType<CosSound::ChannelType>("CosSoundChannelType");
	qRegisterMetaType<CosSound::SoundType>("CosSoundType");
	qRegisterMetaType<GameMatch::GameMode>("GameMode");
	qmlRegisterType<AbstractActivity>("COS.Client", 1, 0, "AbstractActivity");
	qmlRegisterType<Client>("COS.Client", 1, 0, "Client");
	qmlRegisterType<CosDb>("COS.Client", 1, 0, "CosDb");
	qmlRegisterType<CosGame>("COS.Client", 1, 0, "CosGame");
	qmlRegisterType<GameActivity>("COS.Client", 1, 0, "GameActivity");
	qmlRegisterType<GameEnemy>("COS.Client", 1, 0, "GameEnemyPrivate");
	qmlRegisterType<GameEnemyData>("COS.Client", 1, 0, "GameEnemyData");
	qmlRegisterType<GameEnemySoldier>("COS.Client", 1, 0, "GameEnemySoldierPrivate");
	qmlRegisterType<GameEnemySniper>("COS.Client", 1, 0, "GameEnemySniperPrivate");
	qmlRegisterType<GameEntity>("COS.Client", 1, 0, "GameEntityPrivate");
	qmlRegisterType<GameLadder>("COS.Client", 1, 0, "GameLadderPrivate");
	qmlRegisterType<GameMapModel>("COS.Client", 1, 0, "GameMapModel");
	qmlRegisterType<GamePickable>("COS.Client", 1, 0, "GamePickablePrivate");
	qmlRegisterType<GamePlayer>("COS.Client", 1, 0, "GamePlayerPrivate");
	qmlRegisterType<GameScene>("COS.Client", 1, 0, "GameScenePrivate");
	qmlRegisterType<GameTerrain>("COS.Client", 1, 0, "GameTerrain");
	qmlRegisterType<MapEditor>("COS.Client", 1, 0, "MapEditor");
	qmlRegisterType<GameMapEditor>("COS.Client", 1, 0, "GameMapEditor");
	qmlRegisterType<MapListObject>("COS.Client", 1, 0, "MapListObject");
	qmlRegisterType<ObjectListModelObject>("COS.Client", 1, 0, "ObjectListModelObject");
	qmlRegisterType<Profile>("COS.Client", 1, 0, "Profile");
	qmlRegisterType<ServerSettings>("COS.Client", 1, 0, "ServerSettings");
	qmlRegisterType<ServerObject>("COS.Client", 1, 0, "ServerObject");
	qmlRegisterType<Servers>("COS.Client", 1, 0, "Servers");
	qmlRegisterType<StudentMaps>("COS.Client", 1, 0, "StudentMaps");
	qmlRegisterType<TeacherGroups>("COS.Client", 1, 0, "TeacherGroups");
	qmlRegisterType<TeacherMaps>("COS.Client", 1, 0, "TeacherMaps");
	qmlRegisterType<TiledPaintedLayer>("COS.Client", 1, 0, "TiledPaintedLayer");
	qmlRegisterType<VariantMapModel>("COS.Client", 1, 0, "VariantMapModel");
	qmlRegisterType<EditorUndoStack>("COS.Client", 1, 0, "EditorUndoStack");
	qmlRegisterUncreatableType<CosDownloader>("COS.Client", 1, 0, "CosDownloader", "uncreatable");
	qmlRegisterUncreatableType<CosMessage>("COS.Client", 1, 0, "CosMessage", "uncreatable");
	qmlRegisterUncreatableType<CosSound>("COS.Client", 1, 0, "CosSound", "uncreatable");
	qmlRegisterUncreatableType<ObjectListModel>("COS.Client", 1, 0, "ObjectListModel", "uncreatable");
	qmlRegisterUncreatableType<GameMatch>("COS.Client", 1, 0, "GameMatch", "uncreatable");
	qmlRegisterUncreatableType<GameQuestion>("COS.Client", 1, 0, "GameQuestionPrivate", "uncreatable");
	qmlRegisterUncreatableType<MapEditorAction>("COS.Client", 1, 0, "MapEditorAction", "uncreatable");
	qmlRegisterUncreatableType<GameMapEditorChapter>("COS.Client", 1, 0, "GameMapEditorChapter", "uncreatable");
	qmlRegisterUncreatableType<GameMapEditorMissionLevel>("COS.Client", 1, 0, "GameMapEditorMissionLevel", "uncreatable");
	qmlRegisterUncreatableType<GameMapEditorObjective>("COS.Client", 1, 0, "GameMapEditorObjective", "uncreatable");
	qmlRegisterUncreatableType<GameMapEditorStorage>("COS.Client", 1, 0, "GameMapEditorStorage", "uncreatable");
	qmlRegisterUncreatableType<GameMapEditorMission>("COS.Client", 1, 0, "GameMapEditorMission", "uncreatable");
	qRegisterMetaType<MapEditorAction::MapEditorActionType>("MapEditorActionType");
}



/**
 * @brief Client::windowSaveGeometry
 * @param window
 */

void Client::windowSaveGeometry(QQuickWindow *window)
{
	QSettings s(this);
	s.beginGroup("window");

#ifdef Q_OS_LINUX
	s.setValue("size", window->size());
	s.setValue("position", window->position());
#else
	Q_UNUSED(window)
#endif

	s.endGroup();
}



/**
 * @brief Client::restoreWindowGeometry
 * @param window
 * @param forceFullscreen
 */

void Client::windowRestoreGeometry(QQuickWindow *window)
{

	QSettings s(this);
	s.beginGroup("window");

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	window->showFullScreen();
#else

#ifdef Q_OS_LINUX
	window->resize(s.value("size", QSize(600, 600)).toSize());
	window->setPosition(s.value("position", QPoint(0, 0)).toPoint());
#endif

#ifdef QT_DEBUG
	window->showMaximized();
#else
	window->showFullScreen();
#endif

#endif

	s.endGroup();
}


/**
 * @brief Client::windowSetIcon
 * @param window
 */

void Client::windowSetIcon(QQuickWindow *window)
{
	window->setIcon(QIcon(":/internal/img/cos.png"));
}


/**
 * @brief Client::textToClipboard
 * @param text
 */

void Client::textToClipboard(const QString &text) const
{
	QClipboard *clipboard = QGuiApplication::clipboard();
	clipboard->setText(text);
}

/**
 * @brief Client::getWindowSafeMargins
 * @param window
 * @return
 */

QVariantMap Client::getWindowSafeMargins(QQuickWindow *window) const
{
	QVariantMap ret;
	const QMarginsF m = AndroidShareUtils::instance()->getWindowSafeArea(window);

	ret["top"] = m.top();
	ret["left"] = m.left();
	ret["right"] = m.right();
	ret["bottom"] = m.bottom();

	return ret;
}


/**
 * @brief Client::connectionInfoToClipboard
 */

QString Client::connectionInfo(const QString &func, const QVariantMap &queries, const QUrl::FormattingOptions &format) const
{
	if (m_connectionState == Client::Standby)
		return "";

	QUrl url;
	url.setHost(m_socket->requestUrl().host());
	url.setPort(m_socket->requestUrl().port());
	url.setScheme("callofsuli");

	QString path = "";
	if (m_socket->requestUrl().scheme() == "wss")
		path += "/ssl";

	path += "/"+func;

	QUrlQuery query;

	QMapIterator<QString, QVariant> i(queries);
	while (i.hasNext()) {
		i.next();
		query.addQueryItem(i.key(), i.value().toString());
	}

	url.setPath(path);
	url.setQuery(query);

	return url.toString(format);
}


/**
 * @brief Client::connectionInfoMap
 * @return
 */

QVariantMap Client::connectionInfoMap() const
{
	if (m_connectionState == Client::Standby)
		return QVariantMap();

	QVariantMap m;
	m["host"] = m_socket->requestUrl().host();
	m["port"] = m_socket->requestUrl().port();
	m["ssl"] = (m_socket->requestUrl().scheme() == "wss" ? true : false);

	return m;
}






/**
 * @brief Client::standardPathCreate
 */

void Client::standardPathCreate()
{
	QDir d(QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0));
	if (!d.exists()) {
		qInfo().noquote() << tr("Create directory ") + d.absolutePath();
		d.mkpath(d.absolutePath());
	}
}



/**
 * @brief Client::standardPath
 * @param path
 * @return
 */

QString Client::standardPath(const QString &path)
{
	return QStandardPaths::standardLocations(QStandardPaths::DataLocation).at(0)+"/"+path;
}

QString Client::homePath(const QString &path)
{
	return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0)+"/"+path;
}

QString Client::genericDataPath(const QString &path)
{
#ifdef Q_OS_ANDROID
	QAndroidJniObject mediaDir = QAndroidJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
	QAndroidJniObject mediaPath = mediaDir.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
	return "file://"+mediaPath.toString();
#endif
	return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/"+path;
}



/**
 * @brief Client::setSetting
 * @param key
 * @param value
 */

void Client::setSetting(const QString &key, const QVariant &value)
{
	QSettings s;
	s.setValue(key, value);
}


/**
 * @brief Client::getSetting
 * @param key
 * @return
 */

QVariant Client::getSetting(const QString &key, const QVariant &defaultValue)
{
	QSettings s;
	return s.value(key, defaultValue);
}

/**
 * @brief Client::getSettingBool
 * @param key
 * @param defaultValue
 * @return
 */

bool Client::getSettingBool(const QString &key, const bool &defaultValue)
{
	QSettings s;
	return s.value(key, defaultValue).toBool();
}


/**
 * @brief Client::setServerSetting
 * @param key
 * @param value
 */

void Client::setServerSetting(const QString &key, const QVariant &value)
{
	if (m_serverDataDir.isEmpty()) {
		setSetting(key, value);
		return;
	}

	const QString fname = m_serverDataDir+"/settings.ini";

	QSettings s(fname, QSettings::IniFormat, this);
	s.setValue(key, value);
}


/**
 * @brief Client::getServerSetting
 * @param key
 * @param defaultValue
 * @return
 */

QVariant Client::getServerSetting(const QString &key, const QVariant &defaultValue)
{
	if (m_serverDataDir.isEmpty())
		return getSetting(key, defaultValue);

	const QString fname = m_serverDataDir+"/settings.ini";

	QSettings s(fname, QSettings::IniFormat, this);

	return s.value(key, defaultValue);
}


/**
 * @brief Client::readJsonFile
 * @param filename
 * @return
 */

QVariant Client::readJsonFile(QString filename)
{
	QJsonDocument doc = Client::readJsonDocument(filename);
	return doc.toVariant();
}


/**
 * @brief Client::readJsonDocument
 * @param filename
 * @return
 */

QJsonDocument Client::readJsonDocument(QString filename)
{
	QJsonDocument doc;

	if (filename.startsWith("qrc:/"))
		filename.replace("qrc:/", ":/");


	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		qWarning().noquote() << tr("A fájl nem található vagy nem olvasható!") << filename;
		return doc;
	}

	QByteArray b = f.readAll();

	f.close();

	QJsonParseError error;
	doc = QJsonDocument::fromJson(b, &error);
	if (error.error != QJsonParseError::NoError)
		qWarning().noquote() << tr("invalid JSON file '%1' at offset %2").arg(error.errorString()).arg(error.offset);

	return doc;
}


/**
 * @brief Client::saveJsonDocument
 * @param doc
 * @param filename
 * @return
 */

bool Client::saveJsonDocument(QJsonDocument doc, const QString &filename)
{
	QByteArray b = doc.toJson(QJsonDocument::Indented);

	QFile f(filename);
	if (f.open(QIODevice::WriteOnly)) {
		f.write(b);
		f.close();
		return true;
	} else {
		qWarning() << "Write error" << f.errorString();
	}

	return false;
}




/**
 * @brief Client::licenseText
 * @return
 */

QByteArray Client::fileContent(const QString &filename)
{
	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		qWarning().noquote() << tr("A fájl nem található vagy nem olvasható!") << filename;
		return QByteArray();
	}

	QByteArray b = f.readAll();

	f.close();

	return b;
}




/**
 * @brief Client::byteArrayToJsonMap
 * @param data
 * @return
 */

QVariantMap Client::byteArrayToJsonMap(const QByteArray &data)
{
	QJsonDocument doc = QJsonDocument::fromJson(data);
	return doc.toVariant().toMap();
}


/**
 * @brief Client::jsonMapToByteArray
 * @param map
 * @return
 */

QByteArray Client::jsonMapToByteArray(const QVariantMap &map)
{
	QJsonDocument doc = QJsonDocument::fromVariant(map);
	return doc.toJson(QJsonDocument::Compact);
}


/**
 * @brief Client::formattedDataSize
 * @param size
 * @return
 */

QString Client::formattedDataSize(const qint64 &size)
{
	QLocale l = QLocale::system();
	return l.formattedDataSize(size);
}



/**
 * @brief Client::rotatePolygon
 * @param points
 * @param angle
 * @return
 */

QList<QPointF> Client::rotatePolygon(const QList<QPointF> &points, const qreal &angle, const QRectF &boundRect, Qt::Axis axis)
{
	QPolygonF polygon;
	foreach (QPointF p, points) {
		polygon << p;
	}


	//QRect rect = polygon.boundingRect();

	polygon = QTransform()
			  .translate(boundRect.width()/2, boundRect.height()/2)
			  .rotate(angle, axis)
			  .translate(-boundRect.width()/2, -boundRect.height()/2)
			  .map(polygon);

	QList<QPointF> list;

	for (int i=0; i<polygon.count(); ++i) {
		list.append(polygon.at(i));
	}

	return list;
}


/**
 * @brief Client::rotatePolygon
 * @param points
 * @param angle
 * @param axis
 * @return
 */

QList<QPointF> Client::rotatePolygon(const QVariantList &points, const qreal &angle, const QRectF &boundRect, Qt::Axis axis)
{
	QList<QPointF> pointList;

	foreach (QVariant v, points)
		pointList.append(v.toPointF());


	return rotatePolygon(pointList, angle, boundRect, axis);
}



/**
 * @brief Client::rankImageSource
 * @param rankImage
 * @param rankLevel
 * @return
 */

QUrl Client::rankImageSource(int rank, int rankLevel, QString rankImage)
{
	if (!m_rootContext || !m_rootContext->engine()) {
		return QUrl("");
	}

	QQmlEngine *engine = m_rootContext->engine();

	if (!engine->imageProvider("sql")) {
		return QUrl("");
	}

	if (rank == -1) {
		rank = m_userRank;
		rankLevel = m_userRankLevel;
		rankImage = m_userRankImage;
	}

	if (rankImage.isEmpty()) {
		if (rankLevel > 0)
			return QUrl(QString("image://sql/rank/%1.svg/%2").arg(rank).arg(rankLevel));
		else
			return QUrl(QString("image://sql/rank/%1.svg").arg(rank));
	} else {
		if (rankLevel > 0)
			return QUrl(QString("image://sql/%1/%2").arg(rankImage).arg(rankLevel));
		else
			return QUrl(QString("image://sql/%1").arg(rankImage));
	}
}


/**
 * @brief Client::openUrl
 * @param url
 */

void Client::openUrl(const QUrl &url)
{
	QDesktopServices::openUrl(url);
}


/**
 * @brief Client::nextRank
 * @param rankId
 * @return
 */

QVariantMap Client::nextRank(const int &rankId) const
{
	int nextIdx = -1;

	for (int i=0; i<m_rankList.size(); ++i) {
		QVariantMap m = m_rankList.value(i).toMap();
		if (m.value("rankid").toInt() == rankId && i < m_rankList.size()-1) {
			nextIdx = i+1;
			break;
		}
	}

	if (nextIdx != -1) {
		QVariantMap m;
		m["current"] = m_rankList.value(nextIdx-1).toMap();
		m["next"] = m_rankList.value(nextIdx).toMap();
		return m;
	}

	return QVariantMap();
}



/**
 * @brief Client::mapToList
 * @param map
 * @return
 */

QVariantList Client::mapToList(const QVariantMap &map, const QString &keyName)
{
	QVariantList list;

	QMapIterator<QString, QVariant> it(map);

	while (it.hasNext()) {
		it.next();
		QVariantMap mm = it.value().toMap();
		mm[keyName] = it.key();
		list.append(mm);
	}

	return list;
}


/**
 * @brief Client::terrainMap
 * @return
 */

QVariantMap Client::terrainMap()
{
	QVariantMap map;

	foreach (TerrainData d, m_availableTerrains) {
		QVariantMap m;
		m["blocks"] = d.blocks.count();
		m["enemies"] = d.enemies;
		m["details"] = QString(tr("Level %1: %2 csatatér, %3 célpont"))
					   .arg(d.level)
					   .arg(d.blocks.count())
					   .arg(d.enemies);

		m["data"] = d.data;
		m["level"] = d.level;
		m["readableName"] = d.data.contains("name") ? d.data.value("name").toString() : d.name;
		m["thumbnail"] = "qrc:/terrain/"+d.name+"/thumbnail.png";
		map[QString("%1/%2").arg(d.name).arg(d.level)] = m;
	}

	return map;
}




/**
 * @brief Client::takePositionalArgumentsToProcess
 * @return
 */

QStringList Client::takePositionalArgumentsToProcess()
{
	QStringList l = m_positionalArgumentsToProcess;
	m_positionalArgumentsToProcess.clear();
	return l;
}



/**
 * @brief Client::checkPermissions
 */

void Client::checkStoragePermissions() const
{
	AndroidShareUtils::instance()->checkStoragePermissions();
}



/**
 * @brief Client::checkMediaPermissions
 */

void Client::checkMediaPermissions() const
{
	AndroidShareUtils::instance()->checkMediaPermissions();
}






/**
 * @brief Client::playSound
 * @param source
 * @param soundType
 */

void Client::playSound(const QString &source, const CosSound::SoundType &soundType)
{
	QMetaObject::invokeMethod(m_clientSound, "playSound", Qt::QueuedConnection,
							  Q_ARG(QString, source),
							  Q_ARG(CosSound::SoundType, soundType)
							  );
}


/**
 * @brief Client::stopSound
 * @param source
 * @param soundType
 */

void Client::stopSound(const QString &source, const CosSound::SoundType &soundType)
{
	QMetaObject::invokeMethod(m_clientSound, "stopSound", Qt::QueuedConnection,
							  Q_ARG(QString, source),
							  Q_ARG(CosSound::SoundType, soundType)
							  );
}


/**
 * @brief Client::volume
 * @param channel
 * @return
 */


int Client::volume(const CosSound::ChannelType &channel) const
{
	QByteArray func;

	switch (channel) {
		case CosSound::MusicChannel:
			func = "volumeMusic";
			break;
		case CosSound::SfxChannel:
			func = "volumeSfx";
			break;
		case CosSound::VoiceoverChannel:
			func = "volumeVoiceOver";
			break;
	}

	int volume = 0;

	QMetaObject::invokeMethod(m_clientSound, func, Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(int, volume)
							  );

	return volume;
}


/**
 * @brief Client::setVolume
 * @param channel
 * @param volume
 */

void Client::setVolume(const CosSound::ChannelType &channel, const int &volume) const
{
	QByteArray func;

	switch (channel) {
		case CosSound::MusicChannel:
			func = "setVolumeMusic";
			break;
		case CosSound::SfxChannel:
			func = "setVolumeSfx";
			break;
		case CosSound::VoiceoverChannel:
			func = "setVolumeVoiceOver";
			break;
	}

	QMetaObject::invokeMethod(m_clientSound, func, Qt::DirectConnection,
							  Q_ARG(int, volume)
							  );
}

void Client::setSfxVolume(qreal sfxVolume)
{
	if (qFuzzyCompare(m_sfxVolume, sfxVolume))
		return;

	m_sfxVolume = sfxVolume;
	emit sfxVolumeChanged(m_sfxVolume);
}




void Client::setSfxVolumeInt(int sfxVolume)
{
	qreal r = qreal(sfxVolume)/100;

	setSfxVolume(r);
}

void Client::setForcedLandscape(bool forcedLandscape)
{
	if (m_forcedLandscape == forcedLandscape)
		return;

	m_forcedLandscape = forcedLandscape;
	emit forcedLandscapeChanged(m_forcedLandscape);
}

void Client::setServerUuid(QString serverUuid)
{
	if (m_serverUuid == serverUuid)
		return;

	m_serverUuid = serverUuid;
	emit serverUuidChanged(m_serverUuid);
}


/**
 * @brief Client::forceLandscape
 */

void Client::forceLandscape()
{
	if (AndroidShareUtils::instance()->forceLandscape())
		setForcedLandscape(true);
}


/**
 * @brief Client::resetLandscape
 */

void Client::resetLandscape()
{
	if (AndroidShareUtils::instance()->resetLandscape())
		setForcedLandscape(false);
}


void Client::setUserPlayerCharacter(QString userPlayerCharacter)
{
	if (m_userPlayerCharacter == userPlayerCharacter)
		return;

	m_userPlayerCharacter = userPlayerCharacter;
	emit userPlayerCharacterChanged(m_userPlayerCharacter);
}




void Client::setRankList(QVariantList rankList)
{
	if (m_rankList == rankList)
		return;

	m_rankList = rankList;
	emit rankListChanged(m_rankList);
}




void Client::setUserNickName(QString userNickName)
{
	if (m_userNickName == userNickName)
		return;

	m_userNickName = userNickName;
	emit userNickNameChanged(m_userNickName);
}

void Client::setUserRankLevel(int userRankLevel)
{
	if (m_userRankLevel == userRankLevel)
		return;

	m_userRankLevel = userRankLevel;
	emit userRankLevelChanged(m_userRankLevel);
}



/**
 * @brief Client::terrain
 * @param name
 * @return
 */

TerrainData Client::terrain(const QString &name, const int &level)
{
	foreach (TerrainData d, m_availableTerrains) {
		if (d.name == name && d.level == level)
			return d;
	}

	return TerrainData();
}


/**
 * @brief Client::medalIconPath
 * @param name
 * @param qrcPrepend
 * @return
 */

QString Client::medalIconPath(const QString &name, const bool &qrcPrepend)
{
	if (!m_medalIconList.contains(name))
		return "";

	if (qrcPrepend)
		return "qrc:/internal/medals/"+name;
	else
		return ":/internal/medals/"+name;
}



/**
 * @brief Client::connectSslErrorSignalHandler
 * @param handler
 */

void Client::connectSslErrorSignalHandler(QObject *handler)
{
	if (handler)
		m_sslErrorSignalHandlerConnected = true;
	else
		m_sslErrorSignalHandlerConnected = false;
}




/**
 * @brief Client::availableTerrains
 * @return
 */

void Client::loadTerrains()
{
	m_availableTerrains.clear();

	QDirIterator it(":/terrain", {"level1.tmx"}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		QString realname = it.next();

		QString terrainName = realname.section('/',-2,-2);

		QString terrainDir = ":/terrain/"+terrainName;
		QString datafile = terrainDir+"/data.json";

		QVariantMap dataMap;
		if (QFile::exists(datafile))
			dataMap = Client::readJsonFile(datafile).toMap();

		for (int level=1; level<=3; level++) {
			QString tmxFile = QString("%1/level%2.tmx").arg(terrainDir).arg(level);

			if (!QFile::exists(tmxFile))
				continue;

			qInfo() << "Load terrain" << terrainName << "level" << level;

			QString blockfile = QString("%1/blocks%2.json").arg(terrainDir).arg(level);

			if (QFile::exists(blockfile)) {
				QJsonDocument doc = readJsonDocument(blockfile);
				QJsonObject o = doc.object();

				int enemies = o.value("enemies").toInt(-1);

				if (enemies != -1) {
					QMap<int, int> blist;

					qDebug().noquote() << "Load block data from" << blockfile;

					QJsonArray l = o.value("blocks").toArray();
					foreach (QJsonValue v, l) {
						QJsonObject b = v.toObject();
						blist[b.value("num").toInt()] = b.value("enemies").toInt();
					}

					TerrainData d(terrainName,
								  blist,
								  enemies,
								  dataMap,
								  level);

					m_availableTerrains.append(d);
					continue;
				}
			}

			TerrainData d = terrainDataFromFile(tmxFile, terrainName, dataMap, level);

			if (d.enemies != -1)
				m_availableTerrains.append(d);
			else {
				qWarning().noquote() << "Invalid terrain map" << tmxFile;
			}

		}
	}
}



/**
 * @brief Client::loadCharacters
 */

void Client::loadCharacters()
{
	m_characterData.clear();

	QDirIterator it(":/character", {"data.json"}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		QString realname = it.next();

		QString characterName = realname.section('/',-2,-2);

		QVariantMap dataMap;
		dataMap = Client::readJsonFile(realname).toMap();

		QVariantMap d;
		d["name"] = dataMap.value("name").toString();

		m_characterData[characterName] = d;
	}
}


/**
 * @brief Client::loadMusics
 */

void Client::loadMusics()
{
	m_musicList.clear();

	QDirIterator it(":/sound/music", QDir::Files);

	while (it.hasNext()) {
		m_musicList.append(it.next());
	}
}


/**
 * @brief Client::loadMedalIcons
 */

void Client::loadMedalIcons()
{
	m_medalIconList.clear();

	QDirIterator it(":/internal/medals", QDir::Files);

	while (it.hasNext()) {
		it.next();
		m_medalIconList.append(it.fileName());
	}
}


/**
 * @brief Client::reloadGameResources
 */

void Client::reloadGameResources()
{
	Client::loadTerrains();
	Client::loadCharacters();
	Client::loadMusics();
	Client::loadMedalIcons();
}


/**
 * @brief Client::terrainDataFromFile
 * @param filename
 * @return
 */

TerrainData Client::terrainDataFromFile(const QString &filename,
										const QString &terrainName,
										const QVariantMap &dataMap,
										const int &level)
{
	GameTerrain t;

	if (t.loadTmxFile(filename)) {
		QMap<int, int> blockData;

		QMap<int, GameBlock*> b = t.blocks();
		QMapIterator<int, GameBlock*> it(b);
		while (it.hasNext()) {
			it.next();
			blockData[it.key()] = it.value()->enemies().count();
		}

		return TerrainData(terrainName,
						   blockData,
						   t.enemies().count(),
						   dataMap,
						   level);
	}

	return TerrainData("",
					   QMap<int,int>(),
					   -1);
}


/**
 * @brief Client::terrainDataToJson
 * @param filename
 * @return
 */

QByteArray Client::terrainDataToJson(const QString &filename)
{
	TerrainData t = terrainDataFromFile(filename);

	QJsonObject o;

	if (t.enemies != -1) {
		QJsonArray blocks;
		QMap<int, int> m = t.blocks;
		QMapIterator<int, int> it (m);
		while (it.hasNext()) {
			it.next();
			QJsonObject b;
			b["num"] = it.key();
			b["enemies"] = it.value();
			blocks.append(b);
		}

		o["blocks"] = blocks;
		o["enemies"] = t.enemies;
	}

	QJsonDocument doc(o);

	return doc.toJson(QJsonDocument::Indented);
}




void Client::setConnectionState(ConnectionState connectionState)
{
	if (m_connectionState == connectionState)
		return;

	m_connectionState = connectionState;
	emit connectionStateChanged(m_connectionState);
}


/**
 * @brief Client::closeConnection
 */

void Client::closeConnection()
{
	if (m_socket->state() == QAbstractSocket::ConnectedState) {
		setConnectionState(Closing);
		QMetaObject::invokeMethod(m_socket, "close", Qt::QueuedConnection);
	} else {
		setConnectionState(Standby);
		QMetaObject::invokeMethod(m_socket, "close", Qt::QueuedConnection);
		setUserName("");
		setUserNickName("");
		setUserPlayerCharacter("");
		setUserPicture(QUrl());
		setUserRank(0);
		setUserRankLevel(-1);
		setUserXP(0);
		setUserRoles(CosMessage::RoleGuest);
		setRankList(QVariantList());
	}
}


/**
 * @brief Client::login
 * @param username
 * @param session
 * @param password
 */

void Client::login(const QString &username, const QString &session, const QString &password, const bool &isPasswordReset)
{
	if (username.isEmpty())
		return;

	CosMessage m(QJsonObject(), CosMessage::ClassLogin, "");

	QJsonObject d;
	d["username"] = username;
	setUserName(username);
	if (!session.isEmpty()) {
		setSessionToken(session);
		d["session"] = session;
	} else {
		d["password"] = password;
	}

	if (isPasswordReset)
		d["reset"] = true;

	m.setJsonAuth(d);
	m.send(m_socket);
}



/**
 * @brief Client::logout
 */

void Client::logout()
{
	socketSend(CosMessage::ClassLogout, "");
	setSessionToken("");
	setUserName("");
}


/**
 * @brief Client::oauth2Login
 * @param accessToken
 */

void Client::oauth2Login(const QString &accessToken, const QString &expiration, const QString &refreshToken)
{
	CosMessage m(QJsonObject(), CosMessage::ClassLogin, "");

	QJsonObject d;
	d["oauth2Token"] = accessToken;

	if (!expiration.isEmpty() && !refreshToken.isEmpty()) {
		d["oauth2Expiration"] = expiration;
		d["oauth2RefreshToken"] = refreshToken;
	}

	m.setJsonAuth(d);
	m.send(m_socket);
}


/**
 * @brief Client::passwordReset
 * @param email
 * @param code
 */

void Client::passwordRequest(const QString &email, const QString &code)
{
	if (email.isEmpty())
		return;

	QJsonObject d;
	d["user"] = email;
	if (!code.isEmpty()) {
		d["code"] = code;
	}

	d["passwordRequest"] = true;

	CosMessage m(QJsonObject(), CosMessage::ClassLogin, "");

	m.setJsonAuth(d);
	m.send(m_socket);
}







/**
 * @brief Client::socketSend
 * @param cosClass
 * @param cosFunc
 * @param jsonData
 * @param binaryData
 * @return
 */

int Client::socketSend(const CosMessage::CosClass &cosClass, const QString &cosFunc, const QJsonObject &jsonData, const QByteArray &binaryData)
{
	if (m_connectionState != Connected && m_connectionState != Reconnected) {
		sendMessageWarningImage("qrc:/internal/icon/lan-disconnect.svg", tr("Nincs kapcsolat"), tr("A szerver jelenleg nem elérhető!"));
		return -1;
	}

	CosMessage m(jsonData, cosClass, cosFunc);

	if (!m_sessionToken.isEmpty()) {
		QJsonObject auth;
		auth["session"] = m_sessionToken;
		m.setJsonAuth(auth);
	}

	if (!binaryData.isEmpty())
		m.setBinaryData(binaryData);

	m.send(m_socket);

	return m.msgId();
}




void Client::setServerName(QString serverName)
{
	if (m_serverName == serverName)
		return;

	m_serverName = serverName;
	emit serverNameChanged(m_serverName);
}

void Client::setServerDataDir(QString resourceDbName)
{
	if (m_serverDataDir == resourceDbName)
		return;

	m_serverDataDir = resourceDbName;
	emit serverDataDirChanged(m_serverDataDir);
}


/**
 * @brief Client::clearSession
 */

void Client::clearSession()
{
	setUserName("");
	setSessionToken("");
	setUserXP(0);
	setUserRank(0);
	setUserRankLevel(-1);
	setUserRankName("");
	setUserLastName("");
	setUserFirstName("");
	setUserNickName("");
	setUserPlayerCharacter("");
	setUserPicture(QUrl());
	setUserRankImage("");
	setUserRoles(CosMessage::RoleGuest);
	setRankList(QVariantList());
}





void Client::setUserRankImage(QString userRankImage)
{
	if (m_userRankImage == userRankImage)
		return;

	m_userRankImage = userRankImage;
	emit userRankImageChanged(m_userRankImage);
}





/**
 * @brief Client::performUserInfo
 * @param message
 */

void Client::performUserInfo(const CosMessage &message)
{
	QString func = message.cosFunc();
	QJsonObject d = message.jsonData();

	if (message.cosClass() == CosMessage::ClassUserInfo) {
		if (func == "getUser") {
			if (d.value("username").toString() == m_userName) {
				setUserXP(d.value("xp").toInt(0));
				setUserRank(d.value("rankid").toInt(0));
				setUserRankName(d.value("rankname").toString());
				setUserRankLevel(d.value("ranklevel").toInt(-1));
				setUserLastName(d.value("lastname").toString());
				setUserFirstName(d.value("firstname").toString());
				setUserPicture(d.value("picture").toString());
				setUserRankImage(d.value("rankimage").toString());
				setUserNickName(d.value("nickname").toString());
				setUserPlayerCharacter(d.value("character").toString());
			}
		} else if (func == "getServerInfo") {
			setServerName(d.value("serverName").toString());
			setServerUuid(d.value("serverUuid").toString());
			setRegistrationEnabled(d.value("registrationEnabled").toBool());
			setRankList(d.value("ranklist").toArray().toVariantList());
		} else if (func == "newSessionToken") {
			QString token = d.value("token").toString();
			setSessionToken(token);
			qDebug() << "new session token" <<token;
		} else if (func == "newRoles") {
			setUserName(d.value("username").toString());
			setUserRoles(message.clientRole());
			qDebug() << "set user roles from server" << message.clientRole();

			socketSend(CosMessage::ClassUserInfo, "getUser");
		} else if (func == "getMyGroups") {
			emit myGroupListReady(d.value("list").toArray());
		}
	}
}


/**
 * @brief Client::performError
 * @param message
 */

bool Client::checkError(const CosMessage &message)
{
	if (!message.hasError())
		return false;

	CosMessage::CosMessageServerError serverError = message.serverError();
	CosMessage::CosMessageError error = message.messageError();

	switch (serverError) {
		case CosMessage::ServerInternalError:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső szerver hiba"), message.serverErrorDetails());
			break;
		case CosMessage::ServerSmtpError:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("SMTP hiba"), message.serverErrorDetails());
			break;
		case CosMessage::ServerNoError:
			break;
	}

	if (serverError != CosMessage::ServerNoError)
		return true;

	switch (error) {
		case CosMessage::BadMessageFormat:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hibás üzenetformátum"));
			break;
		case CosMessage::InvalidMessageType:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen üzenetformátum"));
			break;
		case CosMessage::InvalidSession:
			sendMessageErrorImage("qrc:/internal/icon/account-clock.svg",tr("Bejelentkezés"), tr("A munkamenetazonosító lejárt. Jelentkezz be ismét!"));
			setSessionToken("");
			setUserName("");
			emit authInvalid();
			break;
		case CosMessage::InvalidUser:
			sendMessageErrorImage("qrc:/internal/icon/account-alert.svg",tr("Bejelentkezés"), tr("Hibás felhasználónév vagy jelszó!"));
			setSessionToken("");
			setUserName("");
			emit authInvalid();
			break;
		case CosMessage::PasswordResetRequired:
			setSessionToken("");
			emit authRequirePasswordReset();
			break;
		case CosMessage::InvalidClass:
		case CosMessage::InvalidFunction:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen kérés"));
			break;
		case CosMessage::ClassPermissionDenied:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Hozzáférés megtagadva"));
			break;
		case CosMessage::NoBinaryData:
		case CosMessage::OtherError:
			sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), message.serverErrorDetails());
			break;
		case CosMessage::NoError:
			break;
	}

	if (error != CosMessage::NoError)
		return true;

	return false;
}


/**
 * @brief Client::guiLoad
 * @return
 */

QString Client::guiLoad() const
{
	return m_guiLoad;
}


/**
 * @brief Client::clientInstance
 * @return
 */

Client *Client::clientInstance()
{
	if (!m_clientInstance) {
		m_clientInstance = new Client();
	}

	return m_clientInstance;
}



#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN32)

/**
 * @brief Client::setSingleInstance
 * @param singleInstance
 */

void Client::setSingleInstance(QSingleInstance *singleInstance)
{
	m_singleInstance = singleInstance;

	if (m_singleInstance)
		connect(m_singleInstance, &QSingleInstance::instanceMessage, this, &Client::urlsToProcessReady);
}

#endif



void Client::setRegistrationEnabled(bool registrationEnabled)
{
	if (m_registrationEnabled == registrationEnabled)
		return;

	m_registrationEnabled = registrationEnabled;
	emit registrationEnabledChanged(m_registrationEnabled);
}

void Client::setUserRankName(QString userRankName)
{
	if (m_userRankName == userRankName)
		return;

	m_userRankName = userRankName;
	emit userRankNameChanged(m_userRankName);
}

void Client::setUserFirstName(QString userFirstName)
{
	if (m_userFirstName == userFirstName)
		return;

	m_userFirstName = userFirstName;
	emit userFirstNameChanged(m_userFirstName);
}

void Client::setUserLastName(QString userLastName)
{
	if (m_userLastName == userLastName)
		return;

	m_userLastName = userLastName;
	emit userLastNameChanged(m_userLastName);
}

void Client::setUserName(QString userName)
{
	if (m_userName == userName)
		return;

	m_userName = userName;
	emit userNameChanged(m_userName);
}

void Client::setUserRoles(CosMessage::ClientRoles userRoles)
{
	if (m_userRoles == userRoles)
		return;

	m_userRoles = userRoles;
	emit userRolesChanged(m_userRoles);
}

void Client::setSessionToken(QString sessionToken)
{
	if (m_sessionToken == sessionToken)
		return;

	m_sessionToken = sessionToken;
	emit sessionTokenChanged(m_sessionToken);
}

void Client::setUserXP(int userXP)
{
	if (m_userXP == userXP)
		return;

	m_userXP = userXP;
	emit userXPChanged(m_userXP);
}

void Client::setUserRank(int userRank)
{
	if (m_userRank == userRank)
		return;

	m_userRank = userRank;
	emit userRankChanged(m_userRank);
}





/**
 * @brief Client::setSocket
 * @param socket
 */

void Client::setSocket(QWebSocket *socket)
{
	if (m_socket == socket)
		return;

	m_socket = socket;
	emit socketChanged(m_socket);
}


/**
 * @brief Client::socketPing
 */

void Client::socketPing()
{
	if (m_connectionState == Connected || m_connectionState == Reconnected) {
		socketSend(CosMessage::ClassUserInfo, "getUser");
	} else if (m_connectionState == Disconnected) {
		qInfo() << tr("Újracsatlakozás");
		emit reconnecting();

		QMetaObject::invokeMethod(m_socket, "open", Qt::QueuedConnection, Q_ARG(QUrl, m_connectedUrl));
	}
}



/**
 * @brief Client::onSocketConnected
 */

void Client::onSocketConnected()
{
	m_connectedUrl = m_socket->requestUrl();

	if (m_connectionState == Connecting || m_connectionState == Standby) {
		setConnectionState(Connected);
	} else if (m_connectionState == Reconnecting || m_connectionState == Disconnected)
		setConnectionState(Reconnected);
}


void Client::onSocketDisconnected()
{
	if (m_connectionState == Connected ||
		m_connectionState == Reconnecting ||
		m_connectionState == Reconnected)
		setConnectionState(Disconnected);
	else
		setConnectionState(Standby);

}


/**
 * @brief Client::onSocketBinaryFrameReceived
 * @param message
 * @param isLastFrame
 */

void Client::onSocketBinaryFrameReceived(const QByteArray &frame, const bool &isLastFrame)
{
	if (!m_cosMessage) {
		if (isLastFrame)
			return;

		m_cosMessage = new CosMessage(frame);
	} else
		m_cosMessage->appendFrame(frame);

	emit messageFrameReceived(*m_cosMessage);

	if (isLastFrame) {
		delete m_cosMessage;
		m_cosMessage = nullptr;
	}

}

/**
 * @brief Client::onSocketBinaryMessageReceived
 * @param message
 */

void Client::onSocketBinaryMessageReceived(const QByteArray &message)
{
	CosMessage m(message);

	if (checkError(m)) {
		emit messageReceivedError(message);
		return;
	}

	performUserInfo(m);

	emit messageReceived(message);
}





/**
 * @brief Client::onSocketStateChanged
 * @param state
 */

void Client::onSocketStateChanged(QAbstractSocket::SocketState state)
{
	if (m_connectionState == Standby && state == QAbstractSocket::ConnectingState)
		setConnectionState(Connecting);
	else if (m_connectionState == Disconnected && state == QAbstractSocket::ConnectingState)
		setConnectionState(Reconnecting);

}




/**
 * @brief Client::onSocketError
 * @param error
 */

void Client::onSocketError(const QAbstractSocket::SocketError &error)
{
	qDebug().noquote() << tr("Socket error") << error;

	if (m_sslErrorSignalHandlerConnected && error == QAbstractSocket::SslHandshakeFailedError)
		return;

	if (m_connectionState == Standby || m_connectionState == Connecting)
		sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Hiba"), QString("%1\n%2").arg(m_socket->errorString()).arg(m_socket->requestUrl().toString()));
}





QQmlContext *Client::rootContext() const
{
	return m_rootContext;
}

void Client::setRootContext(QQmlContext *newRootContext)
{
	if (m_rootContext == newRootContext)
		return;
	m_rootContext = newRootContext;
	emit rootContextChanged();
}

QQmlEngine *Client::rootEngine() const
{
	return m_rootContext->engine();
}

const QUrl &Client::userPicture() const
{
	return m_userPicture;
}

void Client::setUserPicture(const QUrl &newUserPicture)
{
	if (m_userPicture == newUserPicture)
		return;
	m_userPicture = newUserPicture;
	emit userPictureChanged();
}
