/*
 * ---- Call of Suli ----
 *
 * handler.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Handler
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

#include <QJsonDocument>
#include <QRegularExpression>
#include <QDebug>
#include "client.h"
#include "server.h"

#include "userinfo.h"
#include "admin.h"
#include "teacher.h"
#include "student.h"


Client::Client(QWebSocket *socket, Server *server, QObject *parent)
	: QObject(parent)
	, m_server(server)
	, m_socket(socket)
	, m_oauthTokenInitilized(false)
	, m_oauthRequestSent(false)
{
	Q_ASSERT(socket);
	Q_ASSERT(server);

	m_clientState = ClientInvalid;
	m_clientRoles = CosMessage::RoleGuest;
	m_clientSession = "";

	qInfo().noquote() << tr("Client connected: ") << m_socket->peerAddress().toString() << m_socket->peerPort();

	connect(m_socket, &QWebSocket::disconnected, this, &Client::onDisconnected);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onBinaryMessageReceived);

	connect(this, &Client::clientRolesChanged, this, &Client::sendClientRoles);

	connect(this, &Client::oauth2UserinfoReceived, this, &Client::onOAuth2UserinfoReceived);

	connect(server, &Server::serverInfoChanged, this, &Client::sendServerInfo);
	connect(server->db()->worker(), &CosDbWorker::notification, this, [=](QString table) {
		if (table == "ranklog")
			sendUserInfo();
	});
}


/**
 * @brief Handler::~Handler
 */

Client::~Client()
{
}


/**
 * @brief Client::db
 * @return
 */

CosDb *Client::db() const
{
	if (m_server)
		return m_server->db();
	else
		return nullptr;
}


/**
 * @brief Client::mapsDb
 * @return
 */

CosDb *Client::mapsDb() const
{
	if (m_server)
		return m_server->mapsDb();
	else
		return nullptr;
}


/**
 * @brief Client::statDb
 * @return
 */

CosDb *Client::statDb() const
{
	if (m_server)
		return m_server->statDb();
	else
		return nullptr;
}



/**
 * @brief Client::sendClientRoles
 * @param clientRoles
 */

void Client::sendClientRoles()
{
	QJsonObject obj;

	obj["username"] = m_clientUserName;

	CosMessage m(obj, CosMessage::ClassUserInfo, "newRoles");
	m.setClientRole(m_clientRoles);
	m.send(m_socket);
}


/**
 * @brief Client::sendServerInfo
 */

void Client::sendServerInfo()
{
	UserInfo u(this, CosMessage(QJsonObject(), CosMessage::ClassUserInfo, "getServerInfo"));
	u.start();
}


/**
 * @brief Client::sendUserInfo
 */


void Client::sendUserInfo()
{
	UserInfo u(this, CosMessage(QJsonObject(), CosMessage::ClassUserInfo, "getUser"));
	u.start();
}




void Client::setClientState(Client::ClientState clientState)
{
	if (m_clientState == clientState)
		return;

	m_clientState = clientState;
	emit clientStateChanged(m_clientState);
}


void Client::setClientUserName(QString clientUserName)
{
	if (m_clientUserName == clientUserName)
		return;

	m_clientUserName = clientUserName;
	emit clientUserNameChanged(m_clientUserName);

	if (m_clientUserName.isEmpty())
		setClientSession("");
}

/**
 * @brief Client::emailRegistrationDomainList
 * @return
 */

QStringList Client::emailRegistrationDomainList() const
{
	if (!db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='email.registration'")
		.value("v", false).toBool())
		return QStringList();

	QVariantMap m = db()->execSelectQueryOneRow("SELECT value as list FROM settings WHERE key='email.registrationDomains'");

	QString s = m.value("list", "").toString();

	if (s.isEmpty())
		return QStringList();

	QStringList list = s.split(",");
	list.replaceInStrings(QRegExp("\\s"), "");
	list.replaceInStrings(QRegExp("^([^@])"), "@\\1");

	return list;
}

/**
 * @brief Client::setClientRoles
 * @param clientRoles
 */

void Client::setClientRoles(CosMessage::ClientRoles clientRoles)
{
	if (m_clientRoles == clientRoles)
		return;

	m_clientRoles = clientRoles;
	emit clientRolesChanged(m_clientRoles);
}



/**
 * @brief Handler::onDisconnected
 */

void Client::onDisconnected()
{
	qInfo().noquote() << tr("Client disconnected:") << m_socket->peerAddress().toString() << m_socket->peerPort();
	emit disconnected();
}


/**
 * @brief Handler::onBinaryMessageReceived
 * @param message
 */

void Client::onBinaryMessageReceived(const QByteArray &message)
{
	CosMessage m(message);

	qDebug() << "RECEIVED" << m;

	if (!m.valid()) {
		CosMessage r(CosMessage::InvalidMessageType, m);
		r.send(m_socket);
		return;
	}

	clientAuthorize(m);
	clientLogout(m);
	updateRoles();

	switch (m.cosClass()) {
		case CosMessage::ClassLogin:
		case CosMessage::ClassLogout: {
				break;
			}
		case CosMessage::ClassAdmin: {
				Admin u(this, m);
				u.start();
				break;
			}
		case CosMessage::ClassUserInfo: {
				UserInfo u(this, m);
				u.start();
				break;
			}
		case CosMessage::ClassTeacher: {
				Teacher u(this, m);
				u.start();
				break;
			}
		case CosMessage::ClassStudent: {
				Student u(this, m);
				u.start();
				break;
			}
		case CosMessage::ClassInvalid:
		default: {
				CosMessage r(CosMessage::InvalidClass, m);
				r.send(m_socket);
				return;
			}
	}

}




/**
 * @brief Client::clientAuthorize
 * @param data
 */

void Client::clientAuthorize(const CosMessage &message)
{
	if (message.jsonAuth().isEmpty()) {
		setClientState(ClientUnauthorized);
		setClientUserName("");
		return;
	}

	QJsonObject a = message.jsonAuth();

	/*if (a.value("passwordRequest").toBool()) {
		setClientState(ClientUnauthorized);
		setClientUserName("");
		clientPasswordRequest(message);
		return;
	}*/


	QString session = a.value("session").toString("");
	QString oauth2token = a.value("oauth2Token").toString();

	if (!session.isEmpty()) {
		QVariantMap m = db()->execSelectQueryOneRow("SELECT username FROM session WHERE token=?", {session});
		if (!m.isEmpty()) {
			QString user = m.value("username").toString();
			QVariantMap pwdMap = db()->execSelectQueryOneRow("SELECT password, oauthToken FROM auth WHERE username=?", { user });

			db()->execSimpleQuery("UPDATE session SET lastDate=datetime('now') WHERE token=?", {session});
			setClientState(ClientAuthorized);
			setClientUserName(m.value("username").toString());
			setClientSession(session);


			if (pwdMap.value("password") == "*" && !m_oauthTokenInitilized) {
				QString token = pwdMap.value("oauthToken").toString();
				if (token.isEmpty()) {
					CosMessage r(CosMessage::InvalidSession, message);
					r.send(m_socket);
					setClientState(ClientUnauthorized);
					setClientUserName("");
					return;
				} else {
					getOAuth2Userinfo(token);
				}
			}
		} else {
			CosMessage r(CosMessage::InvalidSession, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}
	} else if (!oauth2token.isEmpty()) {
		getOAuth2Userinfo(oauth2token);
		setClientState(ClientUnauthorized);
		setClientUserName("");
		return;
	} else {
		QString username = a.value("username").toString().trimmed();
		QString password = a.value("password").toString();

		if (username.isEmpty()) {
			CosMessage r(CosMessage::InvalidUser, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}


		/*
		if (a.value("reset").toBool(false) && !password.isEmpty()) {
			QVariantList l;
			QString salt;
			QString hashedPassword = CosDb::hashPassword(password, &salt);
			l << username << hashedPassword << salt;
			if (!db()->execSimpleQuery("INSERT OR REPLACE INTO auth (username, password, salt) VALUES (?, ?, ?)", l)) {
				CosMessage r(CosMessage::ServerInternalError, "passwordReset", message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}
		}
*/



		// Regisztráció

		QVariantMap rm = db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM user WHERE username=?) as hasUser, code FROM registration WHERE email=?",
													 {username, username});
		if (rm.value("hasUser", -1).toInt() == 0 && !rm.value("code").toString().isEmpty()) {
			if (!registerUser(username, password)) {
				qDebug() << "REGISTRATION FAILED";
				CosMessage r(CosMessage::ServerInternalError, "registration", message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}
		}


		// Aktív-e?

		if (!db()->execSelectQueryOneRow("SELECT active FROM user WHERE username=?", {username}).value("active", false).toBool()) {
			CosMessage r(CosMessage::InvalidUser, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}


		// Reset password

		if (a.value("reset").toBool() && !password.isEmpty()) {
			if (!db()->execSelectQueryOneRow("SELECT password FROM auth WHERE username=?", {username}).value("password").toString().isEmpty()) {
				CosMessage r(CosMessage::PasswordRequestInvalidEmail, message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			} else {
				QJsonObject o;
				o["username"] = username;
				o["password"] = password;
				CosMessage m2(o, CosMessage::ClassInvalid, "");
				Admin u(this, m2);
				QJsonObject ret;
				if (!u.userPasswordChange(&ret, nullptr)) {
					CosMessage r(CosMessage::ServerInternalError, "password", message);
					r.send(m_socket);
					setClientState(ClientUnauthorized);
					setClientUserName("");
					return;
				}
			}
		}


		// Auth

		QVariantMap m = db()->execSelectQueryOneRow("SELECT password, salt FROM auth WHERE auth.username IN "
													"(SELECT username FROM user WHERE active=1) AND auth.username=?", {username});

		if (!m.isEmpty()) {
			QString storedPassword = m.value("password").toString();
			QString salt = m.value("salt").toString();
			QString hashedPassword = CosDb::hashPassword(password, &salt);

			if (storedPassword.isEmpty()) {
				CosMessage r(CosMessage::PasswordResetRequired, message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName(username);
				return;
			}

			if (password.isEmpty()) {
				CosMessage r(CosMessage::InvalidUser, message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}

			if (QString::compare(storedPassword, hashedPassword, Qt::CaseInsensitive) == 0) {
				QVariantMap r;
				r["username"] = username;
				int rowId = db()->execInsertQuery("INSERT INTO session (?k?) VALUES (?)", r);
				QVariantMap mToken = db()->execSelectQueryOneRow("SELECT token FROM session WHERE rowid=?", {rowId});
				if (!mToken.isEmpty()) {
					QJsonObject json;
					json["token"] = mToken.value("token").toString();
					CosMessage r(json, CosMessage::ClassUserInfo, "newSessionToken");
					r.send(m_socket);
				} else {
					CosMessage r(CosMessage::ServerInternalError, "token", message);
					r.send(m_socket);
					setClientState(ClientUnauthorized);
					setClientUserName("");
					return;
				}
			} else {
				CosMessage r(CosMessage::InvalidUser, message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}

			setClientState(ClientAuthorized);
			setClientUserName(username);
			return;
		} else {
			CosMessage r(CosMessage::InvalidUser, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}
	}

}


/**
 * @brief Client::clientLogout
 * @param data
 * @param clientMsgId
 */

void Client::clientLogout(const CosMessage &message)
{
	if (m_clientState != ClientAuthorized)
		return;

	if(message.cosClass() == CosMessage::ClassLogout) {
		QJsonObject a = message.jsonAuth();

		db()->execSimpleQuery("DELETE FROM session WHERE token=? AND username=?", { a.value("session").toString(),
																					m_clientUserName });

		setClientState(ClientUnauthorized);
		setClientUserName("");
	}
}





/**
 * @brief Client::getRoles
 */

void Client::updateRoles()
{
	if (m_clientState != ClientAuthorized || m_clientUserName.isEmpty()) {
		setClientRoles(CosMessage::RoleGuest);
		return;
	}

	qDebug().noquote() << tr("Update roles");

	CosMessage::ClientRoles newRoles;

	QVariantMap m = db()->execSelectQueryOneRow("SELECT isTeacher, isAdmin FROM user WHERE active=1 AND username=?", {m_clientUserName});

	if (!m.isEmpty()) {
		bool isTeacher = m.value("isTeacher").toBool();
		bool isAdmin = m.value("isAdmin").toBool();
		newRoles.setFlag(CosMessage::RoleStudent, !isTeacher);
		newRoles.setFlag(CosMessage::RoleTeacher, isTeacher);
		newRoles.setFlag(CosMessage::RoleAdmin, isAdmin);
	}

	setClientRoles(newRoles);
}






/**
 * @brief Client::onOAuth2UserinfoReceived
 * @param data
 */

void Client::onOAuth2UserinfoReceived(const QJsonObject &data)
{
	if (data.contains("error") && !m_clientUserName.isEmpty() && m_clientState == ClientAuthorized) {
		qDebug() << "OAUTH2" << data;
		db()->execSimpleQuery("DELETE FROM session WHERE username=?", {m_clientUserName});
		setClientUserName("");
		setClientState(ClientUnauthorized);

		CosMessage r(CosMessage::InvalidSession, CosMessage(CosMessage::InvalidClass));
		r.send(m_socket);
		return;
	}

	QString user = data.value("email").toString();
	QString familyname = data.value("family_name").toString();
	QString givenname = data.value("given_name").toString();
	QString picture = data.value("picture").toString();
	QString token = data.value("origToken").toString();

	if (m_clientState == ClientAuthorized && !m_clientUserName.isEmpty() && user != m_clientUserName) {
		qWarning() << tr("OAuth2 user name mismatch");
		return;
	}

	if (user.isEmpty())
		return;

	QVariantMap u = db()->execSelectQueryOneRow("SELECT username, active FROM user WHERE username=?", {user});

	if (u.isEmpty()) {
		// Registration
		if (db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.registration'")
			.value("v", false).toBool()) {

			QJsonObject obj;
			obj["username"] = user;
			obj["firstname"] = familyname;
			obj["lastname"] = givenname;
			obj["picture"] = picture;
			obj["active"] = true;
			obj["oauthToken"] = token;

			CosMessage m2(obj, CosMessage::ClassInvalid, "");

			QJsonObject ret;
			Admin u(this, m2);
			bool isSuccess = u.userCreate(&ret, nullptr);

			if (isSuccess) {
				qInfo().noquote() << tr("User registered (automatic)") << user;
			} else {
				CosMessage r(CosMessage::ServerInternalError, "oauth2 registration");
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}

		} else {
			CosMessage r(CosMessage::InvalidUser, CosMessage(CosMessage::InvalidClass));
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}
	} else {
		if (!u.value("active").toBool()) {
			db()->execSimpleQuery("DELETE FROM session WHERE username=?", {user});

			CosMessage r(CosMessage::InvalidUser, CosMessage(CosMessage::InvalidClass));
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}

		QVariantMap pwdMap = db()->execSelectQueryOneRow("SELECT password FROM auth WHERE username=?", { user });

		if (pwdMap.value("password") != "*") {
			CosMessage r(CosMessage::InvalidUser, CosMessage(CosMessage::InvalidClass));
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}

		db()->execSimpleQuery("UPDATE auth SET oauthToken=? WHERE username=?", {token, user});
		db()->execSimpleQuery("UPDATE user SET firstname=?, lastname=?, picture=? WHERE username=?", {familyname, givenname, picture, user});
	}

	m_oauthTokenInitilized = true;

	if (m_clientState != ClientAuthorized) {
		setClientState(ClientAuthorized);
		setClientUserName(user);

		QVariantMap r;
		r["username"] = user;
		int rowId = db()->execInsertQuery("INSERT INTO session (?k?) VALUES (?)", r);
		QVariantMap mToken = db()->execSelectQueryOneRow("SELECT token FROM session WHERE rowid=?", {rowId});
		if (!mToken.isEmpty()) {
			QJsonObject json;
			json["token"] = mToken.value("token").toString();
			CosMessage r(json, CosMessage::ClassUserInfo, "newSessionToken");
			r.send(m_socket);
		}

		updateRoles();

	}
}



/**
 * @brief Client::getOAuth2Userinfo
 * @param token
 */

void Client::getOAuth2Userinfo(const QString &token)
{
	if (m_oauthRequestSent) {
		return;
	}

	QUrl url("https://www.googleapis.com/oauth2/v1/userinfo");
	QUrlQuery q;
	q.addQueryItem("alt", "json");
	q.addQueryItem("access_token", token);
	url.setQuery(q);

	m_oauthRequestSent = true;

	QNetworkReply *reply = m_networkAccessManager.get(QNetworkRequest(url));

	qDebug() << "OAUTH2 REQUEST" << url;

	connect(reply, &QNetworkReply::finished, [=]() {
		QByteArray content = reply->readAll();
		QJsonDocument doc = QJsonDocument::fromJson(content);

		qDebug() << "OAUTH2 RESPONSE" << doc;

		if (doc.isObject()) {
			QJsonObject o = doc.object();
			o["origToken"] = token;
			emit oauth2UserinfoReceived(o);
		}

		m_oauthRequestSent = false;

		reply->deleteLater();
	});
}



/**
 * @brief Client::setClientSession
 * @param clientSession
 */


void Client::setClientSession(const QString &clientSession)
{
	m_clientSession = clientSession;
}



/**
 * @brief Client::registerUser
 * @param email
 * @param code
 */

bool Client::registerUser(const QString &email, const QString &code)
{
	QVariantMap m;

	m = db()->execSelectQueryOneRow("SELECT firstname, lastname, classid FROM registration WHERE email=? and code=?", {email, code});
	if (m.isEmpty()) {
		return false;
	}

	QJsonObject obj;
	obj["username"] = email;
	obj["firstname"] = m.value("firstname").toString();
	obj["lastname"] = m.value("lastname").toString();
	obj["active"] = true;

	if (!m.value("classid").isNull())
		obj["classid"] = m.value("classid").toInt();


	CosMessage m2(obj, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Admin u(this, m2);
	bool isSuccess = u.userCreate(&ret, nullptr);

	if (isSuccess) {
		qInfo().noquote() << tr("User registered") << email;
		db()->execSimpleQuery("DELETE FROM registration WHERE email=? AND code=?", {email, code});
		return true;
	} else {
		return false;
	}
}




