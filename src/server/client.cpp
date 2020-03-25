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

Client::Client(CosSql *database, QWebSocket *socket, QObject *parent)
	: QObject(parent)
	, m_db(database)
	, m_socket(socket)
{
	Q_ASSERT(socket);

	m_serverMsgId = 0;
	m_clientState = ClientInvalid;

	qInfo().noquote() << tr("Client connected: ") << this << m_socket->peerAddress().toString() << m_socket->peerPort();

	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
	connect(m_socket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(onBinaryMessageReceived(QByteArray)));
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

void Client::sendError(const COS::ServerError &error, const int &clientMsgId)
{
	QString msgType = "error";
	SEND_BEGIN;

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

	ds << QJsonDocument(object).toBinaryData();

	SEND_END;
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

	if (msgType == "json") {
		QByteArray msgData;
		ds >> msgData;
		parseJson(msgData, clientMsgId, serverMsgId);
	} else {
		sendError(COS::InvalidMessageType, clientMsgId);
	}
}


/**
 * @brief Client::clientAuthorize
 * @param data
 */

void Client::clientAuthorize(const QJsonObject &data)
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
		QVariantMap m = m_db->runSimpleQuery("SELECT username FROM session WHERE token=?", l);
		QVariantList r = m["records"].toList();
		if (!m["error"].toBool() && !r.isEmpty()) {
			setClientState(ClientAuthorized);
			setClientUserName(r.value(0).toMap().value("username").toString());
			return;
		}
	}
}


/**
 * @brief Client::parseJson
 * @param data
 * @param clientMsgId
 */

void Client::parseJson(const QByteArray &data, const int &clientMsgId, const int &serverMsgId)
{
	Q_UNUSED (serverMsgId)

	QJsonDocument d = QJsonDocument::fromBinaryData(data);

	if (d.isNull()) {
		sendError(COS::InvalidJson, clientMsgId);
		return;
	}

	QJsonObject o = d.object();

	if (!o.contains("callofsuli")) {
		sendError(COS::InvalidMessage, clientMsgId);
		return;
	}

	QJsonObject cos = o["callofsuli"].toObject();

	clientAuthorize(cos);

	QJsonObject o2;
	o2["szi"] = "válasz";
	o2["második"] = 2;

	sendJson(o2, clientMsgId);
}

