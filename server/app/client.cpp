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

#define OLD_OAUTH2_TOKEN_DAYS -3

Client::Client(QWebSocket *socket, Server *server, QObject *parent)
	: QObject(parent)
	, m_server(server)
	, m_socket(socket)
	, m_oauthTokenInitialized(false)
	, m_httpRequestList()
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

	//connect(this, &Client::oauth2UserinfoReceived, this, &Client::onOAuth2UserinfoReceived);

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




void Client::setClientState(ClientState clientState)
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
	qInfo().noquote() << tr("Client disconnected:") << m_socket->peerAddress().toString() << m_socket->peerPort() << m_clientUserName;
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
		case CosMessage::ClassExamEngine: {
				if (m_examEngine) {
					m_examEngine->run(this, m);
				} else {
					CosMessage r(CosMessage::InvalidClass, m);
					r.send(m_socket);
					return;
				}
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
	QString session = a.value("session").toString("");
	QString oauth2token = a.value("oauth2Token").toString();
	QString oauth2refreshToken = a.value("oauth2RefreshToken").toString();
	QDateTime expiration;

	if (a.contains("oauth2Expiration"))
		expiration = QDateTime::fromString(a.value("oauth2Expiration").toString(), "yyyy-MM-dd HH:mm:ss");

	if (!session.isEmpty()) {
		QVariantMap m = db()->execSelectQueryOneRow("SELECT username FROM session WHERE token=?", {session});
		if (!m.isEmpty()) {
			const QString &user = m.value("username").toString();

			// Ha dolgozatot ír és nem ebben a kliensben és nem tanár

			if (!db()->execSelectQuery("SELECT token FROM session "
									   "LEFT JOIN user ON (user.username=session.username) "
									   "WHERE session.username=? AND examEngineId IS NOT NULL AND isTeacher=false", {user}).isEmpty()
				&& !m_examEngine) {
				CosMessage r(CosMessage::ExamLock, message);
				r.send(m_socket);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}


			QVariantMap pwdMap = db()->execSelectQueryOneRow("SELECT password, oauthToken, datetime(expiration, 'localtime') as expiration "
															 "FROM auth WHERE username=?", { user });

			db()->execSimpleQuery("UPDATE session SET lastDate=datetime('now') WHERE token=?", {session});
			setClientState(ClientAuthorized);
			setClientUserName(user);
			setClientSession(session);

			/// SKIP OAUTH TOKEN REFRESH

			/*if (pwdMap.value("password") == "*" && !m_oauthTokenInitialized) {
				const QString &token = pwdMap.value("oauthToken").toString();
				const QDateTime &aexpiration = pwdMap.value("expiration").toDateTime();
				if (token.isEmpty() || !aexpiration.isValid()) {
					CosMessage r(CosMessage::InvalidSession, message);
					r.send(m_socket);
					setClientState(ClientUnauthorized);
					setClientUserName("");
					return;
				} else {
					if (QDateTime::currentDateTime().daysTo(aexpiration) <= OLD_OAUTH2_TOKEN_DAYS)
						getOAuth2Userinfo(token, expiration, oauth2refreshToken);
					return;
				}
			}*/
		} else {
			CosMessage r(CosMessage::InvalidSession, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}
	} else if (!oauth2token.isEmpty()) {
		getOAuth2Userinfo(oauth2token, expiration, oauth2refreshToken);
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


		// Aktív-e?

		if (!db()->execSelectQueryOneRow("SELECT active FROM user WHERE username=?", {username}).value("active", false).toBool()) {
			CosMessage r(CosMessage::InvalidUser, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}


		// Ha dolgozatot ír és nem tanár

		if (!db()->execSelectQuery("SELECT token FROM session "
								   "LEFT JOIN user ON (user.username=session.username) "
								   "WHERE session.username=? AND examEngineId IS NOT NULL AND isTeacher=false", {username}).isEmpty()) {
			CosMessage r(CosMessage::ExamLock, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
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

void Client::onOAuth2UserinfoReceived(QNetworkReply *reply, void *p_data)
{
	QString token, refreshToken;
	QDateTime expiration;
	bool isRefresh = false;

	if (p_data) {
		UserInfo::OAuth2Data *d = static_cast<UserInfo::OAuth2Data*>(p_data);

		if (d) {
			token = d->token;
			refreshToken = d->refreshToken;
			expiration = d->expiration;
			isRefresh = d->isRefreshQuery;

			delete d;
		}
	}

	const QByteArray &content = reply->readAll();
	const QJsonDocument &doc = QJsonDocument::fromJson(content);

	const QJsonObject &data = doc.object();

	if (isRefresh) {
		if (data.contains("error") || !data.contains("access_token")) {
			qWarning().noquote() << tr("Refresh token error") << doc.toJson(QJsonDocument::Indented).data();
			db()->execSimpleQuery("UPDATE auth SET expiration=null, refreshToken=null WHERE oauthToken=? AND refreshToken=?",
								  { token, refreshToken });

			setClientUserName("");
			setClientState(ClientUnauthorized);

			CosMessage r(CosMessage::InvalidSession, CosMessage(CosMessage::InvalidClass));
			r.send(m_socket);

		} else {
			const QString &newToken = data.value("access_token").toString();
			const int &secs = data.value("expires_in").toInt();

			db()->execSimpleQuery("UPDATE auth SET oauthToken=?, expiration=datetime('now', ?) WHERE oauthToken=? AND refreshToken=?",
								  {
									  newToken,
									  QString("+%1 seconds").arg(secs),
									  token,
									  refreshToken
								  });

			getOAuth2Userinfo(newToken, QDateTime::currentDateTime().addSecs(secs), refreshToken);
		}

		return;
	}


	if (data.contains("error") && !m_clientUserName.isEmpty() && m_clientState == ClientAuthorized) {
		qWarning() << "OAUTH2 ERROR" << data;
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

	if (m_clientState == ClientAuthorized && !m_clientUserName.isEmpty() && user != m_clientUserName) {
		qWarning() << tr("OAuth2 user name mismatch");
		return;
	}

	if (user.isEmpty())
		return;

	QVariantMap u = db()->execSelectQueryOneRow("SELECT username, active FROM user WHERE username=?", {user});

	if (u.isEmpty()) {
		CosMessage r(CosMessage::InvalidUser, CosMessage(CosMessage::InvalidClass));
		r.send(m_socket);
		setClientState(ClientUnauthorized);
		setClientUserName("");
		return;
	} else {
		if (!u.value("active").toBool()) {
			db()->execSimpleQuery("DELETE FROM session WHERE username=?", {user});

			CosMessage r(CosMessage::InvalidUser, CosMessage(CosMessage::InvalidClass));
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}

		if (db()->execSelectQueryOneRow("SELECT password FROM auth WHERE username=?", {user}).value("password") != "*") {
			CosMessage r(CosMessage::InvalidUser, CosMessage(CosMessage::InvalidClass));
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}


		if (expiration.isValid() && !refreshToken.isEmpty())
			db()->execSimpleQuery("UPDATE auth SET oauthToken=?, refreshToken=?, expiration=? WHERE username=?", {
									  token, refreshToken, expiration.toUTC().toString("yyyy-MM-dd HH:mm:ss"), user
								  });
		else
			db()->execSimpleQuery("UPDATE auth SET oauthToken=? WHERE username=?", {token, user});

		db()->execSimpleQuery("UPDATE user SET firstname=?, lastname=?, picture=? WHERE username=?", {familyname, givenname, picture, user});
	}

	m_oauthTokenInitialized = true;

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

void Client::getOAuth2Userinfo(const QString &token, const QDateTime &expiration, const QString &refreshToken)
{
	UserInfo::OAuth2Data *d = new UserInfo::OAuth2Data;
	d->token = token;
	d->refreshToken = refreshToken;
	d->expiration = expiration;
	d->isRefreshQuery = false;

	/// SKIP OAUTH2 TOKEN REFRESH

	/*if (!refreshToken.isEmpty() && expiration <= QDateTime::currentDateTime().addSecs(5*60)) {

		const QString &googleId = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.googleID'")
								  .value("v").toString();
		const QString &googleKey = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.googleKey'")
								   .value("v").toString();

		if (!googleId.isEmpty() && !googleKey.isEmpty()) {
			qInfo().noquote() << tr("Refresh token");

			QUrl url("https://oauth2.googleapis.com/token");
			QNetworkRequest request(url);

			request.setOriginatingObject(this);
			request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

			QUrlQuery params;
			params.addQueryItem("client_id", googleId);
			params.addQueryItem("client_secret", googleKey);
			params.addQueryItem("refresh_token", refreshToken);
			params.addQueryItem("grant_type", "refresh_token");

			d->isRefreshQuery = true;

			httpPost(request, CosMessage(QJsonObject(), CosMessage::ClassLogin, ""), params.query().toUtf8(), d);

			return;
		}
	}*/

	QUrl url("https://www.googleapis.com/oauth2/v1/userinfo");
	QUrlQuery q;
	q.addQueryItem("alt", "json");
	q.addQueryItem("access_token", token);
	url.setQuery(q);

	httpGet(QNetworkRequest(url), CosMessage(QJsonObject(), CosMessage::ClassLogin, ""), d);

}


/**
 * @brief Client::examEngine
 * @return
 */

ExamEngine *Client::examEngine() const
{
	return m_examEngine;
}


/**
 * @brief Client::setExamEngine
 * @param newExamEngine
 */

void Client::setExamEngine(ExamEngine *newExamEngine, const bool &isOwnerClient)
{
	if (m_examEngine == newExamEngine)
		return;

	if (m_examEngine)
		m_examEngine->clientRemove(this);

	m_examEngine = newExamEngine;

	if (m_examEngine) {
		if (isOwnerClient)
			m_examEngine->addOwnerClient(this);
		else
			m_examEngine->clientAdd(this);
	}
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
 * @brief Client::httpGet
 * @param request
 * @param message
 */

void Client::httpGet(QNetworkRequest request, const CosMessage &message, void *data)
{
	qDebug() << "HTTP GET" << request.url() << message.cosClass() << message.cosFunc();

	request.setOriginatingObject(this);

	QNetworkReply *reply = m_server->networkRequestGet(request);

	HttpRequestMap m;
	m.reply = reply;
	m.message = message;
	m.data = data;

	m_httpRequestList.append(m);
}





/**
 * @brief Client::httpPost
 * @param request
 * @param message
 * @param data
 */

void Client::httpPost(QNetworkRequest request, const CosMessage &message, const QByteArray &postData, void *data)
{
	qDebug() << "HTTP POST" << request.url() << message.cosClass() << message.cosFunc();

	request.setOriginatingObject(this);

	QNetworkReply *reply = m_server->networkRequestPost(request, postData);

	HttpRequestMap m;
	m.reply = reply;
	m.message = message;
	m.data = data;

	m_httpRequestList.append(m);
}



/**
 * @brief Client::httpReply
 * @param reply
 */

void Client::httpReply(QNetworkReply *reply)
{
	qDebug() << "HTTP REPLY" << reply->error();

	CosMessage m;
	void *data = nullptr;

	for (int i=0; i<m_httpRequestList.size(); ++i) {
		const HttpRequestMap &p = m_httpRequestList.at(i);

		if (p.reply == reply) {
			m = p.message;
			data = p.data;
			m_httpRequestList.removeAt(i);
			break;
		}
	}

	if (!m.valid()) {
		qDebug() << "Original message not found";
		return;
	}

	switch (m.cosClass()) {
		case CosMessage::ClassLogin: {
				onOAuth2UserinfoReceived(reply, data);
				break;
			}
		case CosMessage::ClassAdmin: {
				Admin u(this, m);
				u.startHttpReply(reply, data);
				break;
			}
		case CosMessage::ClassUserInfo: {
				UserInfo u(this, m);
				u.startHttpReply(reply, data);
				break;
			}
		case CosMessage::ClassTeacher: {
				Teacher u(this, m);
				u.startHttpReply(reply, data);
				break;
			}
		case CosMessage::ClassStudent: {
				Student u(this, m);
				u.startHttpReply(reply, data);
				break;
			}
		case CosMessage::ClassInvalid:
		case CosMessage::ClassLogout:
		default: {
				break;
			}

	}
}





