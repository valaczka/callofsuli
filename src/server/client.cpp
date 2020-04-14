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
#include "teachermaps.h"
#include "user.h"

Client::Client(CosSql *database, MapRepository *mapDb, QWebSocket *socket, QObject *parent)
	: QObject(parent)
	, m_db(database)
	, m_mapDb(mapDb)
	, m_socket(socket)
{
	Q_ASSERT(socket);

	m_serverMsgId = 0;
	m_clientState = ClientInvalid;
	m_clientRoles = RoleGuest;

	qInfo().noquote() << tr("Client connected: ") << this << m_socket->peerAddress().toString() << m_socket->peerPort();

	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));

	connect(this, &Client::clientRolesChanged, this, &Client::sendClientRoles);
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
	QDataStream ds(message);
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

	QJsonObject a = data["auth"].toObject();

	if (a.contains("session")) {
		QVariantList l;
		l << a["session"].toString();
		//		l << a["username"].toString();
		//		QVariantMap m = m_db->runSimpleQuery("SELECT username FROM session WHERE token=? AND username=?", l);
		QVariantMap m = m_db->runSimpleQuery("SELECT username FROM session WHERE token=?", l);
		QVariantList r = m["records"].toList();
		if (!m["error"].toBool() && !r.isEmpty()) {
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
		QString username = a["username"].toString();
		QString password = a["password"].toString();

		if (username.isEmpty() || password.isEmpty()) {
			setClientState(ClientUnauthorized);
			setClientUserName("");
			return;
		}

		QVariantList l;
		l << username;
		QVariantMap m = m_db->runSimpleQuery("SELECT password, salt FROM auth WHERE auth.username IN "
											 "(SELECT username FROM user WHERE active=1) AND auth.username=?", l);
		QVariantList r = m["records"].toList();
		if (!m["error"].toBool() && !r.isEmpty()) {
			QString storedPassword = r.value(0).toMap().value("password").toString();
			QString salt = r.value(0).toMap().value("salt").toString();
			QString hashedPassword = CosSql::hashPassword(password, &salt);

			if (QString::compare(storedPassword, hashedPassword, Qt::CaseInsensitive) == 0) {
				QVariantMap s = m_db->runSimpleQuery("INSERT INTO session (username) VALUES (?)", l);
				QVariant vId = s["lastInsertId"];
				if (!m["error"].toBool() && !vId.isNull()) {
					QVariantList rl;
					rl << vId.toInt();
					QVariantMap mToken = m_db->runSimpleQuery("SELECT token FROM session WHERE rowid=?", rl);
					QVariantList rToken = mToken["records"].toList();
					if (!mToken["error"].toBool() && !rToken.isEmpty()) {
						QJsonObject json;
						json["token"] = rToken.value(0).toMap().value("token").toString();
						QJsonObject doc;
						doc["session"] = json;
						sendJson(doc);
					} else {
						qWarning().noquote() << tr("Internal error ")+mToken["errorString"].toString();
						sendError("internalError", clientMsgId);
						setClientState(ClientUnauthorized);
						setClientUserName("");
						return;
					}
				} else {
					qWarning().noquote() << tr("Internal error ")+m["errorString"].toString();
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

	if(data["logout"].toBool()) {
		QJsonObject a = data["auth"].toObject();

		QVariantList l;
		l << a["session"].toString();
		l << m_clientUserName;

		QVariantMap m = m_db->runSimpleQuery("DELETE FROM session WHERE token=? AND username=?", l);

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
		setClientRoles(RoleGuest);
		return;
	}

	qDebug().noquote() << tr("Update roles");

	ClientRoles newRoles;

	QVariantList l;
	l << m_clientUserName;
	QVariantMap m = m_db->runSimpleQuery("SELECT isTeacher, isAdmin FROM user WHERE active=1 AND username=?", l);
	QVariantList r = m["records"].toList();
	if (!m["error"].toBool() && !m.isEmpty()) {
		bool isTeacher = r.value(0).toMap().value("isTeacher").toBool();
		bool isAdmin = r.value(0).toMap().value("isAdmin").toBool();
		newRoles.setFlag(RoleStudent, !isTeacher);
		newRoles.setFlag(RoleTeacher, isTeacher);
		newRoles.setFlag(RoleAdmin, isAdmin);
	}

	setClientRoles(newRoles);
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

	QJsonObject cos = o["callofsuli"].toObject();

	clientAuthorize(cos, clientMsgId);
	clientLogout(cos);
	updateRoles();

	QString cl = cos["class"].toString();
	QString func = cos["func"].toString();

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
	} else if (cl == "teacherMaps") {
		TeacherMaps q(this, cos, binaryData);
		q.start(func, &fdata, &bdata);
	} else if (cl == "user") {
		User q(this, cos, binaryData);
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


