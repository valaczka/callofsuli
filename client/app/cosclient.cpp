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

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QResource>
#include <QQuickItem>
#include <QJsonDocument>
#include <QMediaPlaylist>
#include <QDesktopServices>
#include <QPluginLoader>

#include "../../version/buildnumber.h"
#include "cosmessage.h"
#include "cosclient.h"
#include "servers.h"
#include "cosgame.h"
#include "gameentity.h"
#include "gameplayer.h"
#include "gamescene.h"
#include "gamequestion.h"
#include "gameladder.h"
#include "gameenemy.h"
#include "gameenemysoldier.h"
#include "gamepickable.h"
#include "gameterrain.h"
#include "gamematch.h"
#include "mapeditor.h"
#include "tiledpaintedlayer.h"
#include "variantmapmodel.h"
#include "serversettings.h"
#include "scores.h"
#include "gameactivity.h"
#include "cosdownloader.h"
#include "teachermaps.h"
#include "teachergroups.h"
#include "studentmaps.h"



QList<TerrainData> Client::m_availableTerrains;
QVariantMap Client::m_characterData;
QStringList Client::m_musicList;
QStringList Client::m_medalIconList;
QHash<QString, ModuleInterface *> Client::m_moduleObjectiveList;
QHash<QString, ModuleInterface *> Client::m_moduleStorageList;

Client::Client(QObject *parent) : QObject(parent)
{
	m_connectionState = Standby;
	m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
	m_timer = new QTimer(this);

	m_userRoles = CosMessage::RoleGuest;
	m_userXP = 0;
	m_userRank = 0;
	m_userRankImage = "";
	m_userRankLevel = -1;
	m_userNickName = "";
	m_userPlayerCharacter = "";

	m_cosMessage = nullptr;

	m_serverDataDir = "";

	m_sfxVolume = 1.0;

#ifdef WITH_CGRAPH
	m_gvContext = gvContext();
#endif

	m_registrationEnabled = false;
	m_passwordResetEnabled = false;
	m_registrationDomains = QVariantList();
	m_registrationClasses = QVariantList();

	m_clientSound = new CosSound();
	m_clientSound->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished, m_clientSound, &QObject::deleteLater);
	connect(m_clientSound, &CosSound::volumeSfxChanged, this, &Client::setSfxVolumeInt);

	m_workerThread.start();

	QMetaObject::invokeMethod(m_clientSound, "init", Qt::QueuedConnection);




	connect(m_socket, &QWebSocket::connected, this, &Client::onSocketConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &Client::onSocketDisconnected);
	connect(m_socket, &QWebSocket::stateChanged, this, &Client::onSocketStateChanged);
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, &Client::onSocketBinaryFrameReceived);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onSocketBinaryMessageReceived);
	connect(m_socket, &QWebSocket::sslErrors, this, &Client::onSocketSslErrors);
	connect(m_socket, &QWebSocket::bytesWritten, this, &Client::onSocketBytesWritten);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
			[=](QAbstractSocket::SocketError error){
		qDebug().noquote() << tr("Socket error") << error;
		if (m_connectionState == Standby || m_connectionState == Connecting)
			sendMessageError(m_socket->errorString(), m_socket->requestUrl().toString());
	});


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

	delete m_timer;
	delete m_socket;

	if (m_cosMessage)
		delete m_cosMessage;

#ifdef WITH_CGRAPH
	gvFreeContext(m_gvContext);
	m_gvContext = NULL;
#endif
}


/**
 * @brief Client::initialize
 */

void Client::initialize()
{
	/*
#ifndef QT_NO_DEBUG_OUTPUT
	qSetMessagePattern("%{time hh:mm:ss} [%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{message}");
#endif
*/
	QCoreApplication::setApplicationName("callofsuli");
	QCoreApplication::setOrganizationDomain("callofsuli");
	QCoreApplication::setApplicationVersion(_VERSION_FULL);

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

		ModuleInterface *i = qobject_cast<ModuleInterface *>(o);
		QString name = i->name();

		if (i->isStorageModule()) {
			qDebug().noquote() << "Load storage module" << i->name();
			m_moduleStorageList[name] = i;
		} else {
			qDebug().noquote() << "Load objective module" << i->name();
			m_moduleObjectiveList[name] = i;
		}
	}


	// TODO: load d
}






/**
 * @brief Client::commandLineParse
 * @param app
 * @return
 */

bool Client::commandLineParse(QCoreApplication &app)
{
	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::instance()->applicationName()+
									 QString::fromUtf8(" – Copyright © 2012-2021 Valaczka János Pál"));
	parser.addHelpOption();
	parser.addVersionOption();

	parser.addOption({{"t", "terrain"}, QString::fromUtf8("Terepfájl <file> adatainak lekérdezése"), "file"});
	parser.addOption({"license", QString::fromUtf8("Licensz")});

	parser.process(app);

	if (parser.isSet("license")) {
		QByteArray b = Client::fileContent(":/common/license.txt");

		QTextStream out(stdout);
		out << b << Qt::endl;

		return false;
	}

	if (parser.isSet("terrain")) {
		QString tmx = parser.value("terrain");

		QTextStream out(stdout);
		out << terrainDataToJson(tmx) << Qt::endl;

		return false;
	}

	return true;
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
	searchList.append(binDir+"/../share");
	searchList.append(binDir+"/../../share");
	searchList.append(binDir+"/../../../share");
	searchList.append(binDir+"/callofsuli/share");
	searchList.append(binDir+"/../callofsuli/share");
	searchList.append(binDir+"/../../callofsuli/share");
	searchList.append(binDir+"/../../../callofsuli/share");
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
	qRegisterMetaType<CosSound::SoundType>("CosSoundType");
	qRegisterMetaType<CosSound::ChannelType>("CosSoundChannelType");
	qmlRegisterType<AbstractActivity>("COS.Client", 1, 0, "AbstractActivity");
	qmlRegisterType<Client>("COS.Client", 1, 0, "Client");
	qmlRegisterType<CosGame>("COS.Client", 1, 0, "CosGame");
	qmlRegisterType<GameEnemy>("COS.Client", 1, 0, "GameEnemyPrivate");
	qmlRegisterType<GameEnemyData>("COS.Client", 1, 0, "GameEnemyData");
	qmlRegisterType<GameEnemySoldier>("COS.Client", 1, 0, "GameEnemySoldierPrivate");
	qmlRegisterType<GameTerrain>("COS.Client", 1, 0, "GameTerrain");
	qmlRegisterType<GameEntity>("COS.Client", 1, 0, "GameEntityPrivate");
	qmlRegisterType<GameLadder>("COS.Client", 1, 0, "GameLadderPrivate");
	qmlRegisterType<GamePickable>("COS.Client", 1, 0, "GamePickablePrivate");
	qmlRegisterType<GamePlayer>("COS.Client", 1, 0, "GamePlayerPrivate");
	qmlRegisterType<GameScene>("COS.Client", 1, 0, "GameScenePrivate");
	qmlRegisterType<MapEditor>("COS.Client", 1, 0, "MapEditor");
	qmlRegisterType<Servers>("COS.Client", 1, 0, "Servers");
	qmlRegisterType<StudentMaps>("COS.Client", 1, 0, "StudentMaps");
	qmlRegisterType<TeacherMaps>("COS.Client", 1, 0, "TeacherMaps");
	qmlRegisterType<TeacherGroups>("COS.Client", 1, 0, "TeacherGroups");
	qmlRegisterType<TiledPaintedLayer>("COS.Client", 1, 0, "TiledPaintedLayer");
	qmlRegisterType<VariantMapModel>("COS.Client", 1, 0, "VariantMapModel");
	qmlRegisterType<ServerSettings>("COS.Client", 1, 0, "ServerSettings");
	qmlRegisterType<Scores>("COS.Client", 1, 0, "Scores");
	qmlRegisterType<CosDb>("COS.Client", 1, 0, "CosDb");
	qmlRegisterType<GameActivity>("COS.Client", 1, 0, "GameActivity");
	qmlRegisterUncreatableType<GameMatch>("COS.Client", 1, 0, "GameMatch", "uncreatable");
	qmlRegisterUncreatableType<GameQuestion>("COS.Client", 1, 0, "GameQuestionPrivate", "uncreatable");
	qmlRegisterUncreatableType<CosMessage>("COS.Client", 1, 0, "CosMessage", "uncreatable");
	qmlRegisterUncreatableType<CosDownloader>("COS.Client", 1, 0, "CosDownloader", "uncreatable");
	qmlRegisterUncreatableType<CosSound>("COS.Client", 1, 0, "CosSound", "uncreatable");
}



/**
 * @brief Client::windowSaveGeometry
 * @param window
 */

void Client::windowSaveGeometry(QQuickWindow *window, const int &fontSize)
{
#ifndef Q_OS_ANDROID
	QSettings s;
	s.beginGroup("window");

	s.setValue("size", window->size());
	s.setValue("position", window->position());
	s.setValue("visibility", window->visibility());

	if (fontSize > 0)
		s.setValue("fontSize", fontSize);

	s.endGroup();
#else
	Q_UNUSED(window)
	Q_UNUSED(fontSize)
#endif
}



/**
 * @brief Client::restoreWindowGeometry
 * @param window
 * @param forceFullscreen
 */

int Client::windowRestoreGeometry(QQuickWindow *window, const bool &forceFullscreen)
{
	int fontSize = -1;

	QSettings s;
	s.beginGroup("window");

#ifndef Q_OS_ANDROID
	window->resize(s.value("size", QSize(600, 600)).toSize());
	window->setPosition(s.value("position", QPoint(0, 0)).toPoint());

	int v = s.value("visibility", QWindow::AutomaticVisibility).toInt();

	if (forceFullscreen)
		v = QWindow::FullScreen;

	switch (v) {
		case QWindow::Windowed:
		case QWindow::Maximized:
		case QWindow::FullScreen:
		case QWindow::AutomaticVisibility:
			window->setVisibility(static_cast<QWindow::Visibility>(v));
			break;
	}


#else
	Q_UNUSED(window);
	Q_UNUSED(forceFullscreen);
#endif

	fontSize = s.value("fontSize", -1).toInt();

	s.endGroup();

	return fontSize;
}


/**
 * @brief Client::windowSetIcon
 * @param window
 */

void Client::windowSetIcon(QQuickWindow *window)
{
	window->setIcon(QIcon(":/internal/img/cos96.png"));
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



#ifdef WITH_CGRAPH

/**
 * @brief Client::graphvizImage
 * @param dotData
 * @return
 */

QByteArray Client::graphvizImage(const QString &dotData, const char *format)
{
	Agraph_t *G = agmemread(dotData.toUtf8());
	gvLayout(m_gvContext, G, "dot");

	char *result = NULL;
	unsigned int length = 0;

	gvRenderData(m_gvContext, G, format, &result, &length);

	QByteArray imageData(result, length);

	gvFreeLayout(m_gvContext, G);

	free(result);

	return imageData;
}

#endif


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

QUrl Client::rankImageSource(const int &rank, const int &rankLevel, const QString &rankImage)
{
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

	QVariantMap::const_iterator it;

	for (it = map.constBegin(); it != map.constEnd(); ++it) {
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
		m["details"] = QString(tr("%1 csatatér, %2 célpont")).arg(d.blocks.count()).arg(d.enemies);
		m["data"] = d.data;
		m["readableName"] = d.data.contains("name") ? d.data.value("name").toString() : d.name;
		m["thumbnail"] = "qrc:/terrain/"+d.name+"/thumbnail.png";
		map[d.name] = m;
	}

	return map;
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

void Client::setRegistrationClasses(QVariantList registrationClasses)
{
	if (m_registrationClasses == registrationClasses)
		return;

	m_registrationClasses = registrationClasses;
	emit registrationClassesChanged(m_registrationClasses);
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

TerrainData Client::terrain(const QString &name)
{
	foreach (TerrainData d, m_availableTerrains) {
		if (d.name == name)
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
 * @brief Client::availableTerrains
 * @return
 */

void Client::loadTerrains()
{
	m_availableTerrains.clear();

	QDirIterator it(":/terrain", {"terrain.tmx"}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		QString realname = it.next();

		QString terrainName = realname.section('/',-2,-2);

		QString blockfile = ":/terrain/"+terrainName+"/blocks.json";

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

				QString datafile = ":/terrain/"+terrainName+"/data.json";
				QVariantMap dataMap;
				if (QFile::exists(datafile))
					dataMap = Client::readJsonFile(datafile).toMap();

				TerrainData d(terrainName,
							  blist,
							  enemies,
							  dataMap);

				m_availableTerrains.append(d);

				continue;
			}
		}



		TerrainData d = terrainDataFromFile(realname);

		if (d.enemies != -1)
			m_availableTerrains.append(d);
		else {
			qWarning().noquote() << "Invalid terrain map" << realname;
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

TerrainData Client::terrainDataFromFile(const QString &filename)
{
	GameTerrain t;

	if (t.loadTmxFile(filename)) {
		QMap<int, int> blockData;

		QMap<int, GameBlock*> b = t.blocks();
		QMap<int, GameBlock*>::const_iterator it;
		for (it = b.constBegin(); it != b.constEnd(); ++it) {
			blockData[it.key()] = it.value()->enemies().count();
		}


		QString terrainName = filename.section('/',-2,-2);

		QString datafile = ":/terrain/"+terrainName+"/data.json";
		QVariantMap dataMap;
		if (QFile::exists(datafile))
			dataMap = Client::readJsonFile(datafile).toMap();

		return TerrainData(terrainName,
						   blockData,
						   t.enemies().count(),
						   dataMap);
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
		QMap<int, int>::const_iterator it;
		for (it = m.constBegin(); it != m.constEnd(); ++it) {
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




void Client::setConnectionState(Client::ConnectionState connectionState)
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
		m_socket->close();
	} else {
		setConnectionState(Standby);
		m_socket->close();
		setUserName("");
		setUserNickName("");
		setUserPlayerCharacter("");
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
 * @brief Client::passwordReset
 * @param email
 * @param code
 */

void Client::passwordRequest(const QString &email, const QString &code)
{
	if (email.isEmpty())
		return;

	QJsonObject d;
	d["username"] = email;
	if (!code.isEmpty()) {
		d["code"] = code;
	}

	d["passwordRequest"] = true;

	QJsonObject	d2 {
		{"auth", d}
	};

	//socketSend(d2);
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
		sendMessageWarning(tr("Nincs kapcsolat"), tr("A szerver jelenleg nem elérhető!"));
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
				setUserRankImage(d.value("rankimage").toString());
				setUserNickName(d.value("nickname").toString());
				setUserPlayerCharacter(d.value("character").toString());
			}
		} else if (func == "getServerInfo") {
			setServerName(d.value("serverName").toString());
			setRegistrationEnabled(d.value("registrationEnabled").toBool());
			setPasswordResetEnabled(d.value("passwordResetEnabled").toBool(false));
			setRegistrationDomains(d.value("registrationDomains").toArray().toVariantList());
			setRankList(d.value("ranklist").toArray().toVariantList());

			QVariantList classList = d.value("classlist").toArray().toVariantList();
			if (!classList.isEmpty()) {
				QVariantMap m;
				m["classid"] = -1;
				m["classname"] = tr("Osztály nélkül");
				classList.prepend(m);
			}
			setRegistrationClasses(classList);

		} else if (func == "newSessionToken") {
			QString token = d.value("token").toString();
			setSessionToken(token);
			qDebug() << "new session token" <<token;
		} else if (func == "newRoles") {
			setUserName(d.value("username").toString());
			setUserRoles(message.clientRole());
			qDebug() << "set user roles from server" << message.clientRole();

			socketSend(CosMessage::ClassUserInfo, "getUser");
		} else if (func == "registrationRequest") {
			if (d.contains("error")) {
				emit registrationRequestFailed(d.value("error").toString());
			} else if (d.contains("createdUserName")) {
				login(d.value("createdUserName").toString(), "");
			} else if (d.value("success").toBool(false)) {
				emit registrationRequestSuccess();
			}
		} else if (func == "getMyGroups") {
			emit myGroupListReady(d.value("list").toArray());
		}
	}
}


/**
 * @brief Client::performError
 * @param message
 */

void Client::performError(const CosMessage &message)
{
	if (!message.hasError())
		return;

	CosMessage::CosMessageServerError serverError = message.serverError();
	CosMessage::CosMessageError error = message.messageError();

	switch (serverError) {
		case CosMessage::ServerInternalError:
			sendMessageError(tr("Belső szerver hiba"), message.serverErrorDetails());
			return;
			break;
		case CosMessage::ServerSmtpError:
			sendMessageError(tr("SMTP hiba"), message.serverErrorDetails());
			return;
			break;
		case CosMessage::ServerNoError:
			break;
	}

	switch (error) {
		case CosMessage::BadMessageFormat:
			sendMessageError(tr("Belső hiba"), tr("Hibás üzenetformátum"));
			return;
			break;
		case CosMessage::MessageTooOld:
			sendMessageError(tr("Belső hiba"), tr("Elavult kliens"));
			return;
			break;
		case CosMessage::MessageTooNew:
			sendMessageError(tr("Belső hiba"), tr("Elavult szerver"));
			return;
			break;
		case CosMessage::InvalidMessageType:
			sendMessageError(tr("Belső hiba"), tr("Érvénytelen üzenetformátum"));
			return;
			break;
		case CosMessage::PasswordRequestMissingEmail:
			sendMessageError(tr("Elfelejtett jelszó"), tr("Nincs megadva email cím!"));
			return;
			break;
		case CosMessage::PasswordRequestInvalidEmail:
			sendMessageError(tr("Elfelejtett jelszó"), tr("Érvénytelen email cím!"));
			return;
			break;
		case CosMessage::PasswordRequestInvalidCode:
			sendMessageError(tr("Elfelejtett jelszó"), tr("Érvénytelen aktivációs kód!"));
			return;
			break;
		case CosMessage::PasswordRequestCodeSent:
			sendMessageInfo(tr("Elfelejtett jelszó"), tr("Az aktivációs kód a megadot email címre elküldve"));
			return;
			break;
		case CosMessage::PasswordRequestSuccess:
			emit authPasswordResetSuccess();
			return;
			break;
		case CosMessage::InvalidSession:
			sendMessageError(tr("Bejelentkezés"), tr("A munkamenetazonosító lejárt. Jelentkezz be ismét!"));
			setSessionToken("");
			setUserName("");
			emit authInvalid();
			return;
			break;
		case CosMessage::InvalidUser:
			sendMessageError(tr("Bejelentkezés"), tr("Hibás felhasználónév vagy jelszó!"));
			setSessionToken("");
			setUserName("");
			emit authInvalid();
			return;
			break;
		case CosMessage::PasswordResetRequired:
			sendMessageWarning(tr("Bejelentkezés"), tr("A jelszó alaphelyzetben van. Adj meg egy új jelszót!"));
			setSessionToken("");
			emit authRequirePasswordReset();
			return;
			break;
		case CosMessage::InvalidClass:
		case CosMessage::InvalidFunction:
			sendMessageError(tr("Belső hiba"), tr("Érvénytelen kérés"));
			return;
			break;
		case CosMessage::ClassPermissionDenied:
			sendMessageError(tr("Belső hiba"), tr("Hozzáférés megtagadva"));
			return;
			break;
		case CosMessage::NoBinaryData:
		case CosMessage::OtherError:
		case CosMessage::NoError:
			break;
	}
}







void Client::setRegistrationDomains(QVariantList registrationDomains)
{
	if (m_registrationDomains == registrationDomains)
		return;

	m_registrationDomains = registrationDomains;
	emit registrationDomainsChanged(m_registrationDomains);
}

void Client::setRegistrationEnabled(bool registrationEnabled)
{
	if (m_registrationEnabled == registrationEnabled)
		return;

	m_registrationEnabled = registrationEnabled;
	emit registrationEnabledChanged(m_registrationEnabled);
}

void Client::setPasswordResetEnabled(bool passwordResetEnabled)
{
	if (m_passwordResetEnabled == passwordResetEnabled)
		return;

	m_passwordResetEnabled = passwordResetEnabled;
	emit passwordResetEnabledChanged(m_passwordResetEnabled);
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
		m_socket->open(m_connectedUrl);
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

	qDebug() << m;

	performUserInfo(m);

	if (m.valid()) {
		emit messageReceived(message);
		return;
	}

	performError(m);
}



void Client::onSocketSslErrors(const QList<QSslError> &errors)
{
	sendMessageError("SSL hiba", "");
	qDebug() << "ssl error" << errors;
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
 * @brief Client::onSocketBytesWritten
 * @param bytes
 */

void Client::onSocketBytesWritten(const qint64)
{
	emit socketBytesToWrite(m_socket->bytesToWrite());
}






