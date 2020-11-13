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
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QResource>
#include <QQuickItem>
#include <QJsonDocument>

#include "../version/buildnumber.h"
#include "../common/cosmessage.h"
#include "cosclient.h"
#include "servers.h"
#include "mapdata.h"
#include "mapeditor.h"
#include "teacher.h"
#include "adminusers.h"
#include "cosgame.h"
#include "gameentity.h"
#include "gameplayer.h"
#include "gamescene.h"
#include "gameladder.h"
#include "gameenemy.h"
#include "gameenemysoldier.h"
#include "intro.h"
#include "student.h"
#include "studentmap.h"
#include "tiledpaintedlayer.h"



Client::Client(QObject *parent) : QObject(parent)
{
	m_connectionState = Standby;
	m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
	m_timer = new QTimer(this);

	m_clientMsgId = 0;
	m_userRoles = RoleGuest;
	m_userXP = 0;
	m_userRank = 0;

	m_signalList["userInfo"] = "UserInfo";
	m_signalList["teacherMaps"] = "TeacherMaps";
	m_signalList["teacherGroups"] = "TeacherGroups";
	m_signalList["user"] = "User";
	m_signalList["student"] = "Student";

	m_cosMessage = nullptr;

	m_serverDataDir = "";

	m_registrationEnabled = false;
	m_passwordResetEnabled = false;
	m_registrationDomains = QVariantList();

	m_waitForResources = QStringList();

	connect(m_socket, &QWebSocket::connected, this, &Client::onSocketConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &Client::onSocketDisconnected);
	connect(m_socket, &QWebSocket::stateChanged, this, &Client::onSocketStateChanged);
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, &Client::onSocketBinaryFrameReceived);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onSocketBinaryMessageReceived);
	connect(m_socket, &QWebSocket::sslErrors, this, &Client::onSocketSslErrors);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
			[=](QAbstractSocket::SocketError error){
		qDebug().noquote() << tr("Socket error") << error;
		if (m_connectionState == Standby || m_connectionState == Connecting)
			sendMessageError(m_socket->errorString(), m_socket->requestUrl().toString());
	});

	connect(this, &Client::messageReceived, this, &Client::onMessageReceived);

	connect(m_timer, &QTimer::timeout, this, &Client::socketPing);
	m_timer->start(5000);
}

/**
 * @brief Client::~Client
 */

Client::~Client()
{
	delete m_timer;
	delete m_socket;

	if (m_cosMessage)
		delete m_cosMessage;
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
	QCoreApplication::setOrganizationDomain("client.callofsuli.vjp.piarista.hu");
	QCoreApplication::setApplicationVersion(_VERSION_FULL);

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
	searchList.append(binDir+"/callofsuli/share");
	searchList.append(binDir+"/../callofsuli/share");
	searchList.append(binDir+"/../../callofsuli/share");
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
	qmlRegisterType<AdminUsers>("COS.Client", 1, 0, "AdminUsers");
	qmlRegisterType<Client>("COS.Client", 1, 0, "Client");
	qmlRegisterType<CosGame>("COS.Client", 1, 0, "CosGame");
	qRegisterMetaType<CosMessage::CosMessageError>("CosMessageError");
	qRegisterMetaType<CosMessage::CosMessageServerError>("CosMessageServerError");
	qRegisterMetaType<CosMessage::CosClass>("CosClass");
	qRegisterMetaType<CosMessage::CosMessageType>("CosMessageType");
	qmlRegisterUncreatableType<CosMessage>("COS.Client", 1, 0, "CosMessage", "uncreatable");
	qmlRegisterType<GameEnemy>("COS.Client", 1, 0, "GameEnemyPrivate");
	qmlRegisterType<GameEnemyData>("COS.Client", 1, 0, "GameEnemyData");
	qmlRegisterType<GameEnemySoldier>("COS.Client", 1, 0, "GameEnemySoldierPrivate");
	qmlRegisterType<GameEntity>("COS.Client", 1, 0, "GameEntityPrivate");
	qmlRegisterType<GameLadder>("COS.Client", 1, 0, "GameLadderPrivate");
	qmlRegisterType<GamePlayer>("COS.Client", 1, 0, "GamePlayerPrivate");
	qmlRegisterType<GameScene>("COS.Client", 1, 0, "GameScenePrivate");
	qmlRegisterType<Intro>("COS.Client", 1, 0, "Intro");
	qmlRegisterType<MapData>("COS.Client", 1, 0, "Map");
	qmlRegisterType<MapEditor>("COS.Client", 1, 0, "MapEditor");
	qmlRegisterType<Servers>("COS.Client", 1, 0, "Servers");
	qmlRegisterType<Student>("COS.Client", 1, 0, "Student");
	qmlRegisterType<StudentMap>("COS.Client", 1, 0, "StudentMap");
	qmlRegisterType<Teacher>("COS.Client", 1, 0, "Teacher");
	qmlRegisterType<TiledPaintedLayer>("COS.Client", 1, 0, "TiledPaintedLayer");
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
	QDir d(QStandardPaths::standardLocations(QStandardPaths::DataLocation).first());
	if (!d.exists()) {
		qInfo().noquote() << tr("Create directory ") + d.absolutePath();
		d.mkpath(d.absolutePath());
	}
}


/**
 * @brief Client::registerServerResource
 * @param filename
 */

void Client::registerServerResource(const QString &filename)
{
	if (!filename.endsWith(".cres"))
		return;

	qInfo() << tr("Register server resource:") << filename;
	QResource::registerResource(filename);
	m_registeredServerResources.append(filename);
}


/**
 * @brief Client::unregisterServerResources
 */

void Client::unregisterServerResources()
{
	foreach (QString f, m_registeredServerResources) {
		qInfo() << tr("Unregister server resource:") << f;
		QResource::unregisterResource(f);
	}
	m_registeredServerResources.clear();
}


/**
 * @brief Client::standardPath
 * @param path
 * @return
 */

QString Client::standardPath(const QString &path)
{
	return QStandardPaths::standardLocations(QStandardPaths::DataLocation).first()+"/"+path;
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

QVariant Client::getSetting(const QString &key)
{
	QSettings s;
	return s.value(key);
}


/**
 * @brief Client::readJsonFile
 * @param filename
 * @return
 */

QVariant Client::readJsonFile(const QUrl &file)
{
	QString filename;
	if (file.url().startsWith("qrc:/"))
		filename = file.url().replace("qrc:/", ":/");
	else
		filename = file.toLocalFile();

	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		qWarning().noquote() << tr("A fájl nem található vagy nem olvasható!") << filename;
		return QVariant();
	}

	QByteArray b = f.readAll();

	f.close();

	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(b, &error);
	if (error.error != QJsonParseError::NoError) {
		qWarning().noquote() << tr("invalid JSON file '%1' at offset %2").arg(error.errorString()).arg(error.offset);
		return QVariant();
	}
	return doc.toVariant();
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
		setUserRank(0);
		setUserXP(0);
		setUserRoles(RoleGuest);
	}

	unregisterServerResources();
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

	QJsonObject	d2 {
		{"auth", d}
	};

	//socketSend(d2);
}



/**
 * @brief Client::logout
 */

void Client::logout()
{
	/*socketSend({
				   {"logout", true}
			   });*/
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

int Client::socketSend(CosMessage::CosClass cosClass, const QString &cosFunc, const QJsonObject &jsonData, const QByteArray &binaryData)
{
	if (m_connectionState != Connected && m_connectionState != Reconnected) {
		sendMessageWarning(tr("Nincs kapcsolat"), tr("A szerver jelenleg nem elérhető!"));
		return -1;
	}

	CosMessage m(jsonData, cosClass, cosFunc);
	if (!binaryData.isEmpty())
		m.setBinaryData(binaryData);
	m.send(m_socket);

	return m.msgId();
}


/**
 * @brief Client::socketSendJson
 * @param jsonObject
 * @return
 */

QByteArray Client::prepareJson(const QJsonObject &jsonObject)
{
	QJsonObject d;
	QJsonObject d2 = jsonObject;

	if (!m_sessionToken.isEmpty() && !d2.contains("auth")) {
		QJsonObject auth {
			{"session", m_sessionToken}
		};
		d2["auth"] = auth;
	}

	d["callofsuli"] = d2;

	QJsonDocument data(d);
	return data.toBinaryData();
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
 * @brief Client::reloadServerResources
 * @param resources
 */

void Client::reloadServerResources(QVariantMap resources)
{
	qDebug() << "Reload server resources" << resources;

	unregisterServerResources();

	m_waitForResources.clear();
	emit waitForResourcesChanged(m_waitForResources);

	QMapIterator<QString, QVariant> i(resources);

	while (i.hasNext()) {
		i.next();

		QString filename = i.key();
		QString md5 = i.value().toString();
		QString localFile = m_serverDataDir+"/"+filename;


		QFile f(localFile);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray filecontent = f.readAll();
			f.close();

			QString localmd5 = QCryptographicHash::hash(filecontent, QCryptographicHash::Md5).toHex();

			if (localmd5 == md5) {
				qDebug() << localFile << "success";
				registerServerResource(localFile);
				continue;
			}
		}

		qDebug() << "Download" << localFile;

		m_waitForResources.append(filename);
		emit waitForResourcesChanged(m_waitForResources);

		QJsonObject o;
		o["filename"] = filename;
		socketSend(CosMessage::ClassUserInfo, "downloadFile", o);
	}

	if (!m_waitForResources.count())
		emit serverResourcesReady();
}


/**
 * @brief Client::getDownloadedResource
 * @param message
 */

void Client::getDownloadedResource(const CosMessage &message)
{
	QString filename = message.jsonData().value("filename").toString();
	QString md5 = message.jsonData().value("md5").toString();

	if (message.messageType() != CosMessage::MessageBinaryData) {
		qWarning() << tr("Érvénytelen fájl érkezett") << message;
	}

	if (filename.isEmpty()) {
		qWarning() << tr("Érvénytelen fájl érkezett") << message;
		return;
	}

	m_waitForResources.removeAll(filename);
	emit waitForResourcesChanged(m_waitForResources);

	if (!m_waitForResources.count())
		emit serverResourcesReady();

	QByteArray data = message.binaryData();

	QString localmd5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

	if (localmd5 != md5) {
		qWarning() << tr("MD5 hash hiba") << message;
		return;
	}

	QString newFile = m_serverDataDir+"/"+filename;

	QFile f(newFile);

	if (!f.open(QIODevice::WriteOnly)) {
		sendMessageError(tr("Fájl létrehozási hiba"), newFile, f.errorString());
		return;
	}

	if (f.write(data) == -1) {
		sendMessageError(tr("Fájl írási hiba"), newFile, f.errorString());
		f.close();
		return;
	}

	f.close();

	qDebug() << tr("Fájl létrehozva") << newFile;

	registerServerResource(newFile);
}



void Client::setWaitForResources(QStringList waitForResources)
{
	if (m_waitForResources == waitForResources)
		return;

	m_waitForResources = waitForResources;
	emit waitForResourcesChanged(m_waitForResources);
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

void Client::setUserRoles(Roles userRoles)
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
		qDebug() << "reconnect";
		emit reconnecting();
		m_socket->open(m_connectedUrl);
	}
}


/**
 * @brief Client::parseJson
 * @param object
 * @return
 */

void Client::parseJson(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId)
{
	if (object.value("session").isObject()) {
		QJsonObject o = object.value("session").toObject();
		if (o.contains("token")) {
			QString token = o.value("token").toString();
			setSessionToken(token);
			qDebug() << "new session token" <<token;
		}
	}

	if (object.value("roles").isObject()) {
		QJsonObject o = object.value("roles").toObject();
		setUserName(o.value("username").toString());
		Roles newRole;
		newRole.setFlag(RoleGuest, o.value("guest").toBool());
		newRole.setFlag(RoleStudent, o.value("student").toBool());
		newRole.setFlag(RoleTeacher, o.value("teacher").toBool());
		newRole.setFlag(RoleAdmin, o.value("admin").toBool());
		setUserRoles(newRole);
		qDebug() << "set user roles from server" <<newRole;

		socketSend(CosMessage::ClassUserInfo, "getUser");
	}

	QString cl = object.value("class").toString();

	if (cl.isEmpty())
		return;

	QString s = m_signalList.value(cl, "");

	if (s.isEmpty()) {
		qWarning() << tr("Invalid JSON class ")+cl;
		return;
	}

	if (object.value("data").toObject()["error"] == "permission denied") {
		sendMessageWarning(tr("Hozzáférés megtagadva"), tr("Nincs elég jogosultságod a funkció eléréshez!"));
		return;
	}

	QString f = "json"+s+"Received";

	QMetaObject::invokeMethod(this, f.toStdString().data(), Qt::DirectConnection,
							  Q_ARG(QJsonObject, object),
							  Q_ARG(QByteArray, binaryData),
							  Q_ARG(int, clientMsgId)
							  );
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

	qDebug() << m_cosMessage->receivedDataRatio();

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

	if (m.valid()) {
		emit messageReceived(message);
		return;
	}

	///onSocketServerError(error);
}



void Client::onSocketSslErrors(const QList<QSslError> &errors)
{
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
 * @brief Client::onSocketServerError
 * @param error
 */

void Client::onSocketServerError(const QString &error)
{
	if (error == "invalidJSON") {
		sendMessageError(tr("Internal server error"), tr("Hibás JSON"));
	} else if (error == "invalidMessage") {
		sendMessageError(tr("Internal server error"), tr("Hibás kérés"));
	} else if (error == "invalidMessageType") {
		sendMessageError(tr("Internal server error"), tr("Hibás kérés"));
	} else if (error == "invalidUser") {
		sendMessageError(tr("Bejelentkezés"), tr("Hibás felhasználónév vagy jelszó!"));
		setSessionToken("");
		setUserName("");
		emit authInvalid();
	} else if (error == "invalidSession") {
		sendMessageError(tr("Bejelentkezés"), tr("A munkamenetazonosító lejárt. Jelentkezz be ismét!"));
		setSessionToken("");
		setUserName("");
		emit authInvalid();
	} else if (error == "requirePasswordReset") {
		sendMessageError(tr("Bejelentkezés"), tr("A jelszó alaphelyzetben van. Adj meg egy új jelszót!"));
		setSessionToken("");
		emit authRequirePasswordReset();
	} else if (error == "invalid class" || error == "invalid func") {
		sendMessageError(tr("Internal error"), tr("Érvénytelen kérés"));
	} else if (error == "permission denied") {
		sendMessageError(tr("Internal error"), tr("Hozzáférés megtagadva"));
	} else if (error == "passwordRequestNoEmail") {
		sendMessageError(tr("Elfelejtett jelszó"), tr("Nincs megadva email cím!"));
	} else if (error == "passwordRequestInvalidEmail") {
		sendMessageError(tr("Elfelejtett jelszó"), tr("Érvénytelen email cím!"));
	} else if (error == "passwordRequestInvalidCode") {
		sendMessageError(tr("Elfelejtett jelszó"), tr("Érvénytelen aktivációs kód!"));
	} else if (error == "passwordRequestCodeSent") {
		sendMessageInfo(tr("Elfelejtett jelszó"), tr("Az aktivációs kód a megadott email címre elküldve."));
	} else if (error == "passwordRequestSuccess") {
		emit authPasswordResetSuccess();
	} else {
		sendMessageError(tr("Internal server error"), tr("Internal error"), error);
	}
}


/**
 * @brief Client::onJsonUserInfoReceived
 * @param object
 */

void Client::onMessageReceived(const CosMessage &message)
{
	QString func = message.cosFunc();
	QJsonObject d = message.jsonData();

	if (message.cosClass() == CosMessage::ClassUserInfo) {
		if (func == "getUser") {
			if (d.value("username").toString() == m_userName) {
				setUserXP(d.value("xp").toInt(0));
				setUserRank(d.value("rankid").toInt(0));
				setUserRankName(d.value("rankname").toString());
				setUserLastName(d.value("lastname").toString());
				setUserFirstName(d.value("firstname").toString());
			}
		} else if (func == "getServerInfo") {
			setServerName(d.value("serverName").toString());
			setRegistrationEnabled(d.value("registrationEnabled").toString("0").toInt());
			setPasswordResetEnabled(d.value("passwordResetEnabled").toString("0").toInt());
			setRegistrationDomains(d.value("registrationDomains").toArray().toVariantList());
		} else if (func == "registrationRequest") {
			bool error = d.value("error").toBool(false);

			if (error) {
				emit registrationRequestFailed();
				QString errorString = d.value("errorString").toString();

				if (errorString == "email empty")
					sendMessageWarning(tr("Regisztráció"), tr("Nincs megadva email cím!"));
				else if (errorString == "email exists")
					sendMessageWarning(tr("Regisztráció"), tr("A megadott email cím már regisztrálva van!"));
				else
					sendMessageWarning(tr("Regisztráció"), tr("Internal error"), errorString);
			} else {
				emit registrationRequestSuccess();
			}
		} else if (func == "getSettings") {
			if (d.contains("serverName"))
				setServerName(d.value("serverName").toString());

			emit settingsLoaded(d);
		} else if (func == "setSettings") {
			bool error = d.value("error").toBool(true);

			if (error)
				emit settingsError();
			else
				emit settingsSuccess();
		} else if (func == "getResources") {
			reloadServerResources(d.toVariantMap());
		} else if (func == "downloadFile") {
			getDownloadedResource(message);
		}
	}
}


