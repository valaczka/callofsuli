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
#include <QDebug>
#include "client.h"


#include "userinfo.h"
#include "teachergroups.h"
#include "user.h"
#include "student.h"

Client::Client(CosSql *database, MapRepository *mapDb, QWebSocket *socket, QObject *parent)
	: QObject(parent)
	, m_db(database)
	, m_mapDb(mapDb)
	, m_socket(socket)
	, m_msgSize(0)
	, m_cosMessage()
{
	Q_ASSERT(socket);

	m_serverMsgId = 0;
	m_clientState = ClientInvalid;
	m_clientRoles = RoleGuest;

	qInfo().noquote() << tr("Client connected: ") << this << m_socket->peerAddress().toString() << m_socket->peerPort();

	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, &Client::onBinaryFrameReceived);

	connect(this, &Client::clientRolesChanged, this, &Client::sendClientRoles);

	qDebug() << m_cosMessage;
}


/**
 * @brief Handler::~Handler
 */

Client::~Client()
{

}


/**
 * @brief Client::nextServerMsgId
 * @return
 */

int Client::nextServerMsgId()
{
	if (m_serverMsgId == INT_MAX)
		m_serverMsgId=0;

	return ++m_serverMsgId;
}




#define SEND_BEGIN QByteArray d; \
	QDataStream ds(&d, QIODevice::WriteOnly); \
	ds.setVersion(QDataStream::Qt_5_14); \
	int serverMsgId = (clientMsgId == -1 ? nextServerMsgId() : -1); \
	ds << serverMsgId; \
	ds << clientMsgId; \
	ds << msgType

#define SEND_END qDebug().noquote() << tr("SEND to client ") << m_socket << serverMsgId << clientMsgId << msgType; \
	m_socket->sendBinaryMessage(d)




/**
 * @brief Client::sendError
 * @param errorText
 * @param clientMsgId
 */

void Client::sendError(const QString &error, const int &clientMsgId)
{
	QString msgType = "error";
	SEND_BEGIN;

	qDebug() << error;

	ds << error;

	SEND_END;
}


/**
 * @brief Client::sendJson
 * @param object
 * @param clientMsgId
 */

void Client::sendJson(const QJsonObject &object, const int &clientMsgId)
{
	QString msgType = "json";
	SEND_BEGIN;

	qDebug() << object;

	ds << QJsonDocument(object).toBinaryData();

	SEND_END;
}




/**
 * @brief Client::sendFile
 * @param filename
 * @param clientMsgId
 */

void Client::sendBinary(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId)
{
	QString msgType = "binary";
	SEND_BEGIN;

	qDebug() << object;

	ds << QJsonDocument(object).toBinaryData();
	ds << binaryData;

	SEND_END;
}



/**
 * @brief Client::sendClientRoles
 * @param clientRoles
 */

void Client::sendClientRoles(const ClientRoles &clientRoles)
{
	QJsonObject roles, obj;

	roles["username"] = m_clientUserName;
	roles["guest"] = clientRoles.testFlag(RoleGuest);
	roles["student"] = clientRoles.testFlag(RoleStudent);
	roles["teacher"] = clientRoles.testFlag(RoleTeacher);
	roles["admin"] = clientRoles.testFlag(RoleAdmin);

	obj["roles"] = roles;

	sendJson(obj, -1);
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

void Client::setClientRoles(ClientRoles clientRoles)
{
	if (m_clientRoles == clientRoles)
		return;

	m_clientRoles = clientRoles;
	emit clientRolesChanged(m_clientRoles);
}


/**
 * @brief Client::emailRegistration
 * @param email
 * @param firstname
 * @param lastname
 * @param code
 * @return
 */

bool Client::emailRegistration(const QString &email, const QString &firstname, const QString &lastname, const QString &code)
{
	SmtpClient smtp;
	QString serverName;
	QString serverEmail;

	if (!emailSmptClient("registration", &smtp, &serverName, &serverEmail))
		return false;


	MimeMessage message;

	message.setSender(new EmailAddress(serverEmail, serverName));
	message.addRecipient(new EmailAddress(email, firstname+" "+lastname));
	message.setSubject(tr("Call of Suli regisztráció"));

	MimeText text;

	text.setText(QString("Kedves %1!\n\n"
						 "A(z) %2 szerverre a(z) %3 címmel regisztráltál.\n"
						 "A regisztráció aktiválásához jelentkezz be a következő ideiglenes jelszóval:\n\n"
						 "%4\n\n"
						 "Call of Suli")
				 .arg(lastname)
				 .arg(serverName)
				 .arg(email)
				 .arg(code)
				 );

	message.addPart(&text);

	smtp.sendMail(message);
	smtp.quit();

	qInfo().noquote() << tr("Regisztrációs kód elküldve: ") << email;

	return true;
}


/**
 * @brief Client::emailRegistrationDomainList
 * @return
 */

QStringList Client::emailRegistrationDomainList() const
{
	QVariantMap m;

	m_db->execSelectQueryOneRow("SELECT value as list FROM settings WHERE key='email.registrationDomains'", QVariantList(), &m);

	QString s = m.value("list", "").toString();

	if (s.isEmpty())
		return QStringList();

	QStringList list = s.split(",");
	list.replaceInStrings(QRegExp("\\s"), "");
	list.replaceInStrings(QRegExp("^([^@])"), "@\\1");

	return list;
}



/**
 * @brief Handler::onDisconnected
 */

void Client::onDisconnected()
{
	qInfo().noquote() << tr("Client disconnected:") << this << m_socket->peerAddress().toString() << m_socket->peerPort();
	emit disconnected();
}


/**
 * @brief Handler::onBinaryMessageReceived
 * @param message
 */

void Client::onBinaryMessageReceived(const QByteArray &message)
{
	CosMessage m;

	QDataStream ds(message);

	ds >> m;

	qDebug() << "*********" << m;

	/*QDataStream ds(message);
	ds.setVersion(QDataStream::Qt_5_14);

	int clientMsgId = -1;
	int serverMsgId = -1;
	QString msgType;

	ds >> clientMsgId >> serverMsgId >> msgType;

	qDebug().noquote() << tr("RECEIVED from client ") << m_socket << clientMsgId << serverMsgId << msgType;

	if (msgType == "json" || msgType == "binary") {
		QByteArray msgData;
		QByteArray binaryData;
		ds >> msgData;

		if (msgType == "binary")
			ds >> binaryData;

		parseJson(msgData, clientMsgId, serverMsgId, binaryData);
	} else {
		sendError("invalidMessageType", clientMsgId);
	} */
}


/**
 * @brief Client::onBinaryFrameReceived
 * @param frame
 * @param isLastFrame
 */

void Client::onBinaryFrameReceived(const QByteArray &frame, bool isLastFrame)
{
	qDebug() << "RECEIVED" << frame.size() << isLastFrame;

	QDataStream ds(frame);
	ds >> m_cosMessage;

	qDebug() << m_cosMessage;

	if (isLastFrame) {
		m_cosMessage = CosMessage();
		qDebug() << m_cosMessage;
	}

}


/**
 * @brief Client::clientAuthorize
 * @param data
 */

void Client::clientAuthorize(const QJsonObject &data, const int &clientMsgId)
{
	if (!data.contains("auth")) {
		setClientState(ClientUnauthorized);
		setClientUserName("");
		return;
	}

	QJsonObject a = data.value("auth").toObject();

	if (a.value("passwordRequest").toBool()) {
		setClientState(ClientUnauthorized);
		setClientUserName("");
		clientPasswordRequest(a, clientMsgId);
		return;
	}


	QString session = a.value("session").toString("");

	if (!session.isEmpty()) {
		QVariantList l;
		l << session;
		QVariantMap m = m_db->runSimpleQuery("SELECT username FROM session WHERE token=?", l);
		QVariantList r = m.value("records").toList();
		if (!m.value("error").toBool() && !r.isEmpty()) {
			m_db->runSimpleQuery("UPDATE session SET lastDate=datetime('now') WHERE token=?", l);
			setClientState(ClientAuthorized);
			setClientUserName(r.value(0).toMap().value("username").toString());
		} else {
			sendError("invalidSession", clientMsgId);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}
	} else {
		QString username = a.value("username").toString();
		QString password = a.value("password").toString();

		if (username.contains("@")) {
			QVariantList l;
			l << username;
			QVariantMap m = m_db->runSimpleQuery("SELECT username FROM user WHERE email=?", l);
			QVariantList r = m.value("records").toList();
			if (!m.value("error").toBool() && !r.isEmpty()) {
				username = r.value(0).toMap().value("username").toString();
			} else {
				username = tryRegisterUser(username, password);
			}
		}


		if (username.isEmpty()) {
			sendError("invalidUser", clientMsgId);
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}

		if (a.value("reset").toBool(false) && !password.isEmpty()) {
			QVariantList l;
			QString salt;
			QString hashedPassword = CosSql::hashPassword(password, &salt);
			l << username << hashedPassword << salt;
			if (!m_db->execSimpleQuery("INSERT OR REPLACE INTO auth (username, password, salt) VALUES (?, ?, ?)", l)) {
				sendError("password reset error", clientMsgId);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}
		}


		QVariantList l;
		l << username;
		QVariantMap m = m_db->runSimpleQuery("SELECT password, salt FROM auth WHERE auth.username IN "
											 "(SELECT username FROM user WHERE active=1) AND auth.username=?", l);
		QVariantList r = m.value("records").toList();
		if (!m.value("error").toBool() && !r.isEmpty()) {
			QString storedPassword = r.value(0).toMap().value("password").toString();
			QString salt = r.value(0).toMap().value("salt").toString();
			QString hashedPassword = CosSql::hashPassword(password, &salt);

			if (storedPassword.isEmpty()) {
				sendError("requirePasswordReset", clientMsgId);
				setClientState(ClientUnauthorized);
				setClientUserName(username);
				return;
			}

			if (password.isEmpty()) {
				sendError("invalidUser", clientMsgId);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}

			if (QString::compare(storedPassword, hashedPassword, Qt::CaseInsensitive) == 0) {
				QVariantMap s = m_db->runSimpleQuery("INSERT INTO session (username) VALUES (?)", l);
				QVariant vId = s.value("lastInsertId");
				if (!m.value("error").toBool() && !vId.isNull()) {
					QVariantList rl;
					rl << vId.toInt();
					QVariantMap mToken = m_db->runSimpleQuery("SELECT token FROM session WHERE rowid=?", rl);
					QVariantList rToken = mToken.value("records").toList();
					if (!mToken.value("error").toBool() && !rToken.isEmpty()) {
						QJsonObject json;
						json["token"] = rToken.value(0).toMap().value("token").toString();
						QJsonObject doc;
						doc["session"] = json;
						sendJson(doc);
					} else {
						qWarning().noquote() << tr("Internal error ")+mToken.value("errorString").toString();
						sendError("internalError", clientMsgId);
						setClientState(ClientUnauthorized);
						setClientUserName("");
						return;
					}
				} else {
					qWarning().noquote() << tr("Internal error ")+m.value("errorString").toString();
					sendError("internalError", clientMsgId);
					setClientState(ClientUnauthorized);
					setClientUserName("");
					return;
				}

				setClientState(ClientAuthorized);
				setClientUserName(username);
				return;
			} else {
				sendError("invalidUser", clientMsgId);
				setClientState(ClientUnauthorized);
				setClientUserName("");
				return;
			}
		} else {
			sendError("invalidUser", clientMsgId);
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

void Client::clientLogout(const QJsonObject &data)
{
	if (m_clientState != ClientAuthorized)
		return;

	if(data.value("logout").toBool()) {
		QJsonObject a = data.value("auth").toObject();

		QVariantList l;
		l << a.value("session").toString();
		l << m_clientUserName;

		QVariantMap m = m_db->runSimpleQuery("DELETE FROM session WHERE token=? AND username=?", l);

		setClientState(ClientUnauthorized);
		setClientUserName("");
	}
}


/**
 * @brief Client::clientPasswordRequest
 * @param data
 */

bool Client::clientPasswordRequest(const QJsonObject &data, const int &clientMsgId)
{
	QString email = data.value("username").toString("");
	QString code = data.value("code").toString("");

	if (email.isEmpty()) {
		sendError("passwordRequestNoEmail", clientMsgId);
		return false;
	}

	QVariantList l;
	l << email;
	QVariantMap m = m_db->runSimpleQuery("SELECT username, email, firstname, lastname FROM user WHERE active=1 AND email=?", l);
	QVariantList r = m.value("records").toList();

	if (!m.value("error").toBool() && !r.isEmpty()) {
		QString username = r.value(0).toMap().value("username").toString();
		QString storedEmail = r.value(0).toMap().value("email").toString();
		QString firstname = r.value(0).toMap().value("firstname").toString();
		QString lastname = r.value(0).toMap().value("lastname").toString();

		QVariantList ll;
		ll << username;

		if (code.isEmpty()) {
			if (!m_db->execSimpleQuery("INSERT OR REPLACE INTO passwordReset (username) VALUES (?)", ll)) {
				sendError("internal error", clientMsgId);
				return false;
			}

			QVariantMap mm;
			if (m_db->execSelectQueryOneRow("SELECT code FROM passwordReset WHERE username=?", ll, &mm)) {
				if (emailPasswordReset(storedEmail, firstname, lastname, mm.value("code").toString())) {
					sendError("passwordRequestCodeSent", clientMsgId);
					return true;
				} else {
					sendError("passwordRequestEmailError", clientMsgId);
					return false;
				}
			} else {
				sendError("internal error", clientMsgId);
				return false;
			}
		} else {
			QVariantMap mm;
			QVariantList cc;
			cc << code;
			if (m_db->execSelectQueryOneRow("SELECT username FROM passwordReset WHERE code=?", cc, &mm)) {
				if (mm.value("username").toString() == username) {
					if (m_db->execSimpleQuery("UPDATE auth SET password=null, salt=null WHERE username=?", ll) &&
						m_db->execSimpleQuery("DELETE FROM passwordReset WHERE username=?", ll)) {
						sendError("passwordRequestSuccess", clientMsgId);
						qDebug().noquote() << "Password reset:" << username;
						return true;
					} else {
						sendError("internal error", clientMsgId);
						return false;
					}
				} else {
					sendError("passwordRequestInvalidCode", clientMsgId);
					return false;
				}
			}
		}
	} else {
		sendError("passwordRequestInvalidEmail", clientMsgId);
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
		setClientRoles(RoleGuest);
		return;
	}

	qDebug().noquote() << tr("Update roles");

	ClientRoles newRoles;

	QVariantList l;
	l << m_clientUserName;
	QVariantMap m = m_db->runSimpleQuery("SELECT isTeacher, isAdmin FROM user WHERE active=1 AND username=?", l);
	QVariantList r = m.value("records").toList();
	if (!m.value("error").toBool() && !m.isEmpty()) {
		bool isTeacher = r.value(0).toMap().value("isTeacher").toBool();
		bool isAdmin = r.value(0).toMap().value("isAdmin").toBool();
		newRoles.setFlag(RoleStudent, !isTeacher);
		newRoles.setFlag(RoleTeacher, isTeacher);
		newRoles.setFlag(RoleAdmin, isAdmin);
	}

	setClientRoles(newRoles);
}


/**
 * @brief Client::tryRegisterUser
 * @param email
 * @return
 */

QString Client::tryRegisterUser(const QString &email, const QString &password)
{
	QVariantMap m;

	QVariantList l;
	l << email;
	l << password;
	if (!m_db->execSelectQueryOneRow("SELECT firstname, lastname FROM registration WHERE email=? and code=?", l, &m))
		return "";

	QString username = email;
	username.replace("@", "_");
	username.append(".");

	m_db->db().transaction();


	QVariantList ll;
	ll << username;
	if (!m_db->execSelectQueryOneRow("SELECT newname FROM "
									 "(WITH RECURSIVE cnt(x) AS (SELECT 1 UNION ALL SELECT x+1 FROM cnt LIMIT 10000) SELECT ?||x as newname FROM cnt) "
									 "WHERE EXISTS(SELECT * from user WHERE username=newname) is false LIMIT 1", ll, &m)) {
		qWarning().noquote() << "Nem sikerült megfelelő felhasználónevet találni!";
		m_db->db().rollback();
		return "";
	}

	QJsonObject obj;
	obj["username"] = m.value("newname").toString();
	obj["firstname"] = m.value("firstname").toString();
	obj["lastname"] = m.value("lastname").toString();
	obj["active"] = true;
	obj["email"] = email;

	QJsonObject ret;
	User u(this, obj, QByteArray());
	u.userCreate(&ret, nullptr);

	if (ret.value("created").toInt(-1) > 0) {
		m_db->execSimpleQuery("DELETE FROM registration WHERE email=? AND code=?", l);
		m_db->db().commit();
		return ret.value("createdUserName").toString();
	} else {
		m_db->db().rollback();
		return "";
	}

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
 * @brief Client::parseJson
 * @param data
 * @param clientMsgId
 */

void Client::parseJson(const QByteArray &jsonData, const int &clientMsgId, const int &serverMsgId, const QByteArray &binaryData)
{
	Q_UNUSED (serverMsgId)

	QJsonDocument d = QJsonDocument::fromBinaryData(jsonData);

	if (d.isNull()) {
		sendError("invalidJSON", clientMsgId);
		return;
	}

	QJsonObject o = d.object();

	qDebug() << o;

	if (!o.contains("callofsuli")) {
		sendError("invalidMessage", clientMsgId);
		return;
	}

	QJsonObject cos = o.value("callofsuli").toObject();

	clientAuthorize(cos, clientMsgId);
	clientLogout(cos);
	updateRoles();

	QString cl = cos.value("class").toString();
	QString func = cos.value("func").toString();

	if (cl.isEmpty() || func.isEmpty())
		return;

	QJsonObject ret;
	ret["class"] = cl;
	ret["func"] = func;
	QJsonObject fdata;
	QByteArray bdata;

	if (cl == "userInfo") {
		UserInfo q(this, cos, binaryData);
		q.start(func, &fdata, &bdata);
	} else if (cl == "teacherGroups") {
		TeacherGroups q(this, cos, binaryData);
		q.start(func, &fdata, &bdata);
	} else if (cl == "user") {
		User q(this, cos, binaryData);
		q.start(func, &fdata, &bdata);
	} else if (cl == "student") {
		Student q(this, cos, binaryData);
		q.start(func, &fdata, &bdata);
	} else {
		sendError("invalidClass", clientMsgId);
		return;
	}

	ret["data"] = fdata;

	if (bdata.isNull())
		sendJson(ret, clientMsgId);
	else
		sendBinary(ret, bdata, clientMsgId);
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

	QVariantMap m;

	m_db->execSelectQueryOneRow("SELECT value as smtpServer FROM settings WHERE key='smtp.server'", QVariantList(), &m);
	m_db->execSelectQueryOneRow("SELECT value as smtpPort FROM settings WHERE key='smtp.port'", QVariantList(), &m);
	m_db->execSelectQueryOneRow("SELECT value as smtpType FROM settings WHERE key='smtp.type'", QVariantList(), &m);
	m_db->execSelectQueryOneRow("SELECT value as smtpEmail FROM settings WHERE key='smtp.email'", QVariantList(), &m);
	m_db->execSelectQueryOneRow("SELECT value as smtpUser FROM settings WHERE key='smtp.user'", QVariantList(), &m);
	m_db->execSelectQueryOneRow("SELECT value as smtpPassword FROM settings WHERE key='smtp.password'", QVariantList(), &m);

	if (emailType == "passwordReset")
		m_db->execSelectQueryOneRow("SELECT value as enabled FROM settings WHERE key='email.passwordReset'", QVariantList(), &m);
	else if (emailType == "registration")
		m_db->execSelectQueryOneRow("SELECT value as enabled FROM settings WHERE key='email.registration'", QVariantList(), &m);

	m_db->execSelectQueryOneRow("SELECT serverName from system", QVariantList(), &m);

	bool enabled = m.value("enabled", "").toString().toInt();

	QString server = m.value("smtpServer", "").toString();
	int port = m.value("smtpPort", "-1").toString().toInt();
	int type = m.value("smtpType", "0").toString().toInt();
	QString user = m.value("smtpUser", "").toString();
	QString password = m.value("smtpPassword", "").toString();

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
		*serverName = m.value("serverName", tr("Call of Suli szerver")).toString();

	if (serverEmail)
		*serverEmail = m.value("smtpEmail").toString();

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


