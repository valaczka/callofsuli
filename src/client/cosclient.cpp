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
#include "cosclient.h"
#include "servers.h"
#include "map.h"
#include "mapeditor.h"
#include "teacher.h"
#include "adminusers.h"
#include "game.h"
#include "intro.h"
#include "student.h"
#include "studentmap.h"



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

	m_serverDataDir = "";

	m_registrationEnabled = false;
	m_passwordResetEnabled = false;
	m_registrationDomains = QVariantList();

	connect(m_socket, &QWebSocket::connected, this, &Client::onSocketConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &Client::onSocketDisconnected);
	connect(m_socket, &QWebSocket::stateChanged, this, &Client::onSocketStateChanged);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onSocketBinaryMessageReceived);
	connect(m_socket, &QWebSocket::sslErrors, this, &Client::onSocketSslErrors);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
			[=](QAbstractSocket::SocketError error){
		qDebug().noquote() << tr("Socket error") << error;
		if (m_connectionState == Standby || m_connectionState == Connecting)
			sendMessageError(m_socket->errorString(), m_socket->requestUrl().toString());
	});

	connect(this, &Client::jsonUserInfoReceived, this, &Client::onJsonUserInfoReceived);

	connect(m_timer, &QTimer::timeout, this, &Client::socketPing);
	//m_timer->start(5000);
}

/**
 * @brief Client::~Client
 */

Client::~Client()
{
	delete m_timer;
	delete m_socket;
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
	//QCoreApplication::setOrganizationName("Call of Suli");
	QCoreApplication::setApplicationVersion(_VERSION_FULL);

}



/**
 * @brief Client::registerResource
 * @param filename
 * @return
 */

bool Client::registerResource(const QString &filename)
{
	// REGISTER TEST MAP RESOURCE
	QResource::registerResource("/home/valaczka/Projektek/callofsuli/src/maps/map1.cosm", "/map1");
	QResource::registerResource("/home/valaczka/Projektek/callofsuli/src/characters/character1.cosc", "/character1");

#ifdef Q_OS_ANDROID
	if (QResource::registerResource("assets:/"+filename)) {
		qDebug().noquote() << tr("Registered resource from assets") + filename;
		return true;
	}
#endif

	QString binDir = QCoreApplication::applicationDirPath();

	QStringList searchList;

	searchList.append(binDir);
	searchList.append(binDir+"/share");
	searchList.append(binDir+"/../share");
	searchList.append(binDir+"/../../share");
	searchList.append(binDir+"/../../../share");
	searchList.append(binDir+"/../../../../share");
	searchList.append(binDir+"/callofsuli/share");
	searchList.append(binDir+"/../callofsuli/share");
	searchList.append(binDir+"/../../callofsuli/share");
	searchList.append(binDir+"/../../../callofsuli/share");
	searchList.append(binDir+"/../../../../callofsuli/share");
	searchList.append(QDir::rootPath()+"usr/local/share/callofsuli");
	searchList.append(QDir::rootPath()+"usr/share/callofsuli");
	searchList.append(QDir::rootPath()+"share/callofsuli");

	searchList.append(QStandardPaths::displayName(QStandardPaths::HomeLocation));

	foreach (QString dir, searchList)
	{
		QFile file(dir+"/"+filename);

		if (file.exists())
		{
			QString realname=QDir(file.fileName()).canonicalPath();
			qInfo().noquote() << tr("Registered resource: ")+realname;
			QResource::registerResource(realname);

			return true;
		}
	}

	return false;
}



/**
 * @brief Client::registerTypes
 */

void Client::registerTypes()
{
	qmlRegisterType<Client>("COS.Client", 1, 0, "Client");
	qmlRegisterType<CosSql>("COS.Client", 1, 0, "CosSql");
	qmlRegisterType<COSdb>("COS.Client", 1, 0, "COSdb");
	qmlRegisterType<Servers>("COS.Client", 1, 0, "Servers");
	qmlRegisterType<Map>("COS.Client", 1, 0, "Map");
	qmlRegisterType<MapEditor>("COS.Client", 1, 0, "MapEditor");
	qmlRegisterType<Teacher>("COS.Client", 1, 0, "Teacher");
	qmlRegisterType<AdminUsers>("COS.Client", 1, 0, "AdminUsers");
	qmlRegisterType<AbstractDbActivity>("COS.Client", 1, 0, "AbstractDbActivity");
	//qmlRegisterType<Game>("COS.Client", 1, 0, "GameEngine");
	qmlRegisterType<Intro>("COS.Client", 1, 0, "Intro");
	qmlRegisterType<Student>("COS.Client", 1, 0, "Student");
	qmlRegisterType<StudentMap>("COS.Client", 1, 0, "StudentMap");
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
	window->setIcon(QIcon(":/img/cos96.png"));
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

int Client::clientVersionMajor() { return _VERSION_MAJOR; }

int Client::clientVersionMinor() { return _VERSION_MINOR; }

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

	socketSend(d2);
}



/**
 * @brief Client::logout
 */

void Client::logout()
{
	socketSend({
				   {"logout", true}
			   });
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

	socketSend(d2);
}






int Client::socketNextClientMsgId()
{
	return ++m_clientMsgId;
}



/**
 * @brief Client::socketSend
 * @param msgType
 * @param data
 * @param serverMsgId
 */

int Client::socketSend(const QJsonObject &jsonObject, const QByteArray &binaryData, const int &serverMsgId)
{
	if (m_connectionState != Connected && m_connectionState != Reconnected) {
		sendMessageWarning(tr("Nincs kapcsolat"), tr("A szerver jelenleg nem elérhető!"));
		return -1;
	}

	QString msgType = binaryData.isNull() ? "json" : "binary";

	QByteArray d;
	QDataStream ds(&d, QIODevice::WriteOnly);
	ds.setVersion(QDataStream::Qt_5_14);

	int clientMsgId = (serverMsgId == -1 ? socketNextClientMsgId() : -1);
	ds << clientMsgId;
	ds << serverMsgId;
	ds << msgType;
	ds << prepareJson(jsonObject);

	if (!binaryData.isNull())
		ds << binaryData;

	qDebug().noquote() << tr("SEND to server ") << m_socket << clientMsgId << serverMsgId << msgType << jsonObject;

	m_socket->sendBinaryMessage(d);

	return clientMsgId;
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
		socketSend({
					   {"class", "userInfo"},
					   {"func", "getUser"}
				   });
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

		socketSend({
					   {"class", "userInfo"},
					   {"func", "getUser"}
				   });
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
		socketSend({{"class", "userInfo"}, {"func", "getServerInfo"}});
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
 * @brief Client::onSocketBinaryMessageReceived
 * @param message
 */

void Client::onSocketBinaryMessageReceived(const QByteArray &message)
{
	QDataStream ds(message);
	ds.setVersion(QDataStream::Qt_5_14);

	int serverMsgId = -1;
	int clientMsgId = -1;
	QString msgType;

	ds >> serverMsgId >> clientMsgId >> msgType;

	qDebug().noquote() << tr("RECEIVED from server ") << serverMsgId << clientMsgId << msgType;

	if (msgType == "json" || msgType == "binary") {
		QByteArray data;
		QByteArray binaryData;
		ds >> data;

		if (msgType == "binary")
			ds >> binaryData;

		QJsonDocument doc = QJsonDocument::fromBinaryData(data);
		if (doc.isNull()) {
			sendMessageError(tr("Internal server error"), tr("Hibás válasz"));
			return;
		}

		QJsonObject obj = doc.object();

		qDebug() << obj;

		parseJson(obj, binaryData, clientMsgId);
	} else {
		QString error;
		ds >> error;
		onSocketServerError(error);
	}
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

void Client::onJsonUserInfoReceived(const QJsonObject &object, const QByteArray &, const int &)
{
	QString func = object.value("func").toString();
	QJsonObject d = object.value("data").toObject();

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
	}
}


