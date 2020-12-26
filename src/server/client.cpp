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
#include "teachermap.h"


Client::Client(QWebSocket *socket, Server *server, QObject *parent)
	: QObject(parent)
	, m_server(server)
	, m_socket(socket)
{
	Q_ASSERT(socket);
	Q_ASSERT(server);

	m_clientState = ClientInvalid;
	m_clientRoles = CosMessage::RoleGuest;

	qInfo().noquote() << tr("Client connected: ") << m_socket->peerAddress().toString() << m_socket->peerPort();

	connect(m_socket, &QWebSocket::disconnected, this, &Client::onDisconnected);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Client::onBinaryMessageReceived);
#ifdef QT_DEBUG
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, [=](const QByteArray &frame, bool) {
		qDebug() << "RECEIVED FRAME" << frame.size();
	});
#endif

	connect(this, &Client::clientRolesChanged, this, &Client::sendClientRoles);

	connect(server, &Server::serverInfoChanged, this, &Client::sendServerInfo);
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
}

/**
 * @brief Client::emailRegistrationDomainList
 * @return
 */

QStringList Client::emailRegistrationDomainList() const
{
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
		case CosMessage::ClassTeacherMap: {
				TeacherMap u(this, m);
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

	if (a.value("passwordRequest").toBool()) {
		setClientState(ClientUnauthorized);
		setClientUserName("");
		clientPasswordRequest(message);
		return;
	}


	QString session = a.value("session").toString("");

	if (!session.isEmpty()) {
		QVariantList l;
		l << session;
		QVariantMap m = db()->execSelectQueryOneRow("SELECT username FROM session WHERE token=?", l);
		if (!m.isEmpty()) {
			db()->execSimpleQuery("UPDATE session SET lastDate=datetime('now') WHERE token=?", l);
			setClientState(ClientAuthorized);
			setClientUserName(m.value("username").toString());
		} else {
			CosMessage r(CosMessage::InvalidSession, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}
	} else {
		QString username = a.value("username").toString();
		QString password = a.value("password").toString();

		if (username.isEmpty()) {
			CosMessage r(CosMessage::InvalidUser, message);
			r.send(m_socket);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}

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


		QVariantList l;
		l << username;
		QVariantMap m = db()->execSelectQueryOneRow("SELECT password, salt FROM auth WHERE auth.username IN "
													"(SELECT username FROM user WHERE active=1) AND auth.username=?", l);

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
				QVariantList lToken;
				lToken << rowId;
				QVariantMap mToken = db()->execSelectQueryOneRow("SELECT token FROM session WHERE rowid=?", lToken);
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

		QVariantList l;
		l << a.value("session").toString();
		l << m_clientUserName;

		db()->execSimpleQuery("DELETE FROM session WHERE token=? AND username=?", l);

		setClientState(ClientUnauthorized);
		setClientUserName("");
	}
}


/**
 * @brief Client::clientPasswordRequest
 * @param data
 */

bool Client::clientPasswordRequest(const CosMessage &message)
{
	if (message.jsonAuth().isEmpty())
		return false;

	QJsonObject a = message.jsonAuth();

	QString email = a.value("username").toString("");
	QString code = a.value("code").toString("");

	QRegularExpression regex("^[0-9a-zA-Z]+([0-9a-zA-Z][-._+])[0-9a-zA-Z]+@[0-9a-zA-Z]+([-.][0-9a-zA-Z]+)([0-9a-zA-Z][.])[a-zA-Z]{2,6}$");

	if (email.isEmpty() || !regex.match(email).hasMatch()) {
		CosMessage r(CosMessage::PasswordRequestMissingEmail, message);
		r.send(m_socket);
		return false;
	}

	QVariantList l;
	l << email;
	QVariantMap m = db()->execSelectQueryOneRow("SELECT firstname, lastname FROM user WHERE active=1 AND username=?", l);
	if (!m.isEmpty()) {
		QString firstname = m.value("firstname").toString();
		QString lastname = m.value("lastname").toString();

		QVariantList ll;
		ll << email;

		if (code.isEmpty()) {
			if (!db()->execSimpleQuery("INSERT OR REPLACE INTO passwordReset (username) VALUES (?)", ll)) {
				CosMessage r(CosMessage::ServerInternalError, "passwordReset", message);
				r.send(m_socket);
				return false;
			}

			QVariantMap mm = db()->execSelectQueryOneRow("SELECT code FROM passwordReset WHERE username=?", ll);
			if (!mm.isEmpty()) {
				if (emailPasswordReset(email, firstname, lastname, mm.value("code").toString())) {
					CosMessage r(CosMessage::PasswordRequestCodeSent, message);
					r.send(m_socket);
					return true;
				} else {
					CosMessage r(CosMessage::ServerSmtpError, "sendEmail", message);
					r.send(m_socket);
					return false;
				}
			} else {
				CosMessage r(CosMessage::ServerInternalError, "passwordResetCode", message);
				r.send(m_socket);
				return false;
			}
		} else {
			QVariantList cc;
			cc << code;
			QVariantMap mm = db()->execSelectQueryOneRow("SELECT username FROM passwordReset WHERE code=?", cc);
			if (!mm.isEmpty()) {
				if (mm.value("username").toString() == email) {
					if (db()->execSimpleQuery("UPDATE auth SET password=null, salt=null WHERE username=?", ll) &&
						db()->execSimpleQuery("DELETE FROM passwordReset WHERE username=?", ll)) {
						CosMessage r(CosMessage::PasswordRequestSuccess, message);
						r.send(m_socket);
						qDebug().noquote() << "Password reset:" << email;
						return true;
					} else {
						CosMessage r(CosMessage::ServerInternalError, "passwordUpdate", message);
						r.send(m_socket);
						return false;
					}
				} else {
					CosMessage r(CosMessage::PasswordRequestInvalidCode, message);
					r.send(m_socket);
					return false;
				}
			}
		}
	} else {
		CosMessage r(CosMessage::PasswordRequestInvalidEmail, message);
		r.send(m_socket);
		return false;
	}

	return false;
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

	QVariantList l;
	l << m_clientUserName;
	QVariantMap m = db()->execSelectQueryOneRow("SELECT isTeacher, isAdmin FROM user WHERE active=1 AND username=?", l);

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
 * @brief Client::onSmtpError
 * @param e
 */

void Client::onSmtpError(SmtpClient::SmtpError e)
{
	qWarning().noquote() << "SMTP error" << e;
}





/**
 * @brief Client::emailSmptClient
 * @param type
 * @return
 */

bool Client::emailSmptClient(const QString &emailType, SmtpClient *smtpClient, QString *serverName, QString *serverEmail)
{
	if (!smtpClient) {
		qWarning().noquote() << "Missing smtpClient!";
		return false;
	}

	bool enabled = false;

	if (emailType == "passwordReset")
		enabled = db()->execSelectQueryOneRow("SELECT value as enabled FROM settings WHERE key='email.passwordReset'").value("enabled", false).toBool();
	else if (emailType == "registration")
		enabled = db()->execSelectQueryOneRow("SELECT value as enabled FROM settings WHERE key='email.registration'").value("enabled", false).toBool();

	QString server =  db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='smtp.server'").value("v").toString();
	int port = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='smtp.port'").value("v", 0).toInt();
	int type = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='smtp.type'").value("v", 0).toInt();
	QString user = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='smtp.user'").value("v").toString();
	QString password = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='smtp.password'").value("v").toString();

	qDebug() << enabled << server << port << user << password;

	if (!enabled ||
		server.isEmpty() ||
		port <= 0 ||
		user.isEmpty() ||
		password.isEmpty()) {

		qWarning().noquote() << "Email password reset disabled!";
		return false;
	}

	SmtpClient::ConnectionType c = SmtpClient::TcpConnection;

	if (type==1)
		c = SmtpClient::SslConnection;
	else if (type==2)
		c = SmtpClient::TlsConnection;

	if (serverName)
		*serverName = db()->execSelectQueryOneRow("SELECT serverName from system").value("serverName", tr("Call of Suli szerver")).toString();

	if (serverEmail)
		*serverEmail = db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='smtp.email'").value("v").toString();

	smtpClient->setHost(server);
	smtpClient->setPort(port);
	smtpClient->setConnectionType(c);
	smtpClient->setUser(user);
	smtpClient->setPassword(password);

	connect(smtpClient, &SmtpClient::smtpError, this, &Client::onSmtpError);

	if (!smtpClient->connectToHost()) {
		qWarning().noquote() << "Couldn't connect to SMTP host" << server << port;
		return false;
	}

	if (!smtpClient->login()) {
		qWarning().noquote() << "Couldn't login to SMTP host" << user;
		return false;
	}

	return true;
}


/**
 * @brief Client::emailPasswordReset
 * @param email
 * @param firstname
 * @param lastname
 * @param code
 */

bool Client::emailPasswordReset(const QString &email, const QString &firstname, const QString &lastname, const QString &code)
{
	SmtpClient smtp;
	QString serverName;
	QString serverEmail;

	if (!emailSmptClient("passwordReset", &smtp, &serverName, &serverEmail))
		return false;


	MimeMessage message;

	message.setSender(new EmailAddress(serverEmail, serverName));
	message.addRecipient(new EmailAddress(email, firstname+" "+lastname));
	message.setSubject(tr("Call of Suli aktivációs kód"));

	MimeText text;

	text.setText(QString("Kedves %1!\n\n"
						 "A(z) %2 email címhez tartozó fiók jelszavának alaphelyzetbe állítását kérted.\n"
						 "Ehhez add meg az alábbi aktivációs kódot:\n\n"
						 "%3\n\n"
						 "%4\n"
						 "Call of Suli")
				 .arg(lastname)
				 .arg(email)
				 .arg(code)
				 .arg(serverName)
				 );

	message.addPart(&text);

	smtp.sendMail(message);
	smtp.quit();

	qInfo().noquote() << tr("Aktivációs kód elküldve: ") << email;

	return true;
}




