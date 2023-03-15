/*
 * ---- Call of Suli ----
 *
 * asyncmessagehandler.cpp
 *
 * Created on: 2023. 01. 14.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AsyncMessageHandler
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

#include "asyncmessagehandler.h"
#include "Logger.h"
#include "client.h"
#include "application.h"
#include "websocket.h"

AsyncMessageHandler::AsyncMessageHandler(QObject *parent)
	: QObject{parent}
{
	LOG_CTRACE("client") << "AsyncMessageHandler created" << this;
	setClient(Application::instance()->client());
}


/**
 * @brief AsyncMessageHandler::AsyncMessageHandler
 * @param client
 */

AsyncMessageHandler::AsyncMessageHandler(Client *client)
	: QObject{client}
	, m_client(client)
{
	LOG_CTRACE("client") << "AsyncMessageHandler created with client" << this << client;

}



/**
 * @brief AsyncMessageHandler::~AsyncMessageHandler
 */

AsyncMessageHandler::~AsyncMessageHandler()
{
	if (m_client)
		m_client->removeMessageHandler(this);

	LOG_CTRACE("client") << "AsyncMessageHandler destroyed" << this;
}

/**
 * @brief AsyncMessageHandler::client
 * @return
 */

Client *AsyncMessageHandler::client() const
{
	return m_client;
}




/**
 * @brief AsyncMessageHandler::setClient
 * @param newClient
 */

void AsyncMessageHandler::setClient(Client *newClient)
{
	if (m_client && newClient != m_client)
		m_client->removeMessageHandler(this);

	if (m_client == newClient)
		return;

	m_client = newClient;
	emit clientChanged();

	if (m_client)
		m_client->addMessageHandler(this);
}



/**
 * @brief AsyncMessageHandler::sendRequest
 * @param classHandler
 * @param json
 */

void AsyncMessageHandler::sendRequest(const WebSocketMessage::ClassHandler &classHandler, const QJsonObject &json)
{
	if (!m_client) {
		Application::instance()->messageError(tr("Kliens nincsen beállítva!"), tr("Belső hiba"));
		return;
	}

	//WebSocketReply *reply = m_client->webSocket()->send()


	WebSocketReply *send(const WebSocket::Method &method, const WebSocket::API &api, const QString &path, const QJsonObject &data = {});
	WebSocketReply *send(const WebSocket::API &api, const QString &path, const QJsonObject &data = {}) {
		return send(Get, api, path, data);
	}

	m_messages.insert(message.msgNumber(), message);

	///m_client->webSocket()->send(message);

	setPending(!m_messages.isEmpty());
}


/**
 * @brief AsyncMessageHandler::sendRequestFunc
 * @param classHandler
 * @param func
 * @param json
 */

void AsyncMessageHandler::sendRequestFunc(const WebSocketMessage::ClassHandler &classHandler, const QString &func, const QJsonObject &json)
{
	sendRequest(classHandler, func.toUtf8(), json);
}


/**
 * @brief AsyncMessageHandler::sendRequest
 * @param classHandler
 * @param func
 * @param json
 */

void AsyncMessageHandler::sendRequest(const WebSocketMessage::ClassHandler &classHandler, const char *func, QJsonObject json)
{
	json.insert(QStringLiteral("func"), QString::fromUtf8(func));
	sendRequest(classHandler, json);
}




/**
 * @brief AsyncMessageHandler::handleMessage
 * @param message
 */

void AsyncMessageHandler::handleMessage(const WebSocketMessage &message)
{
	if (!prehandleMessage(message))
		return;

	if (message.opCode() != WebSocketMessage::RequestResponse) {
		LOG_CWARNING("client") << "Can't handle message:" << message;
		return;
	}

	WebSocketMessage orig = m_messages.take(message.requestMsgNumber());

	setPending(!m_messages.isEmpty());

	m_repliedMessages.append(orig);

	if (m_repliedMessages.size() > 100)
		m_repliedMessages.removeFirst();

	if (!orig.isValid()) {
		foreach (const WebSocketMessage &m, m_repliedMessages)
			if (m.msgNumber() == message.requestMsgNumber()) {
				orig = m;
				break;
			}
	}

	if (!orig.isValid())
		return;


	const QString &func = message.data().value(QStringLiteral("func")).toString();

	if (func.isEmpty()) {
		LOG_CWARNING("client") << "Missing function" << message;
		emit invalidMessageReceived(message, orig);
		return;
	}



	QByteArray normalizedSignature = QMetaObject::normalizedSignature(func.toLatin1()+"(QJsonObject,QJsonObject)");

	int methodIndex = this->metaObject()->indexOfMethod(normalizedSignature);

	if (methodIndex != -1) {
		QMetaObject::invokeMethod(this, func.toLatin1(), Qt::DirectConnection,
								  Q_ARG(QJsonObject, message.data()),
								  Q_ARG(QJsonObject, orig.data())
								  );
		return;
	}



	normalizedSignature = QMetaObject::normalizedSignature(func.toLatin1()+"(QJsonObject)");

	methodIndex = this->metaObject()->indexOfMethod(normalizedSignature);

	if (methodIndex != -1) {
		QMetaObject::invokeMethod(this, func.toLatin1(), Qt::DirectConnection,
								  Q_ARG(QJsonObject, message.data())
								  );
		return;
	}



	normalizedSignature = QMetaObject::normalizedSignature(func.toLatin1()+"(QVariant,QVariant)");

	methodIndex = this->metaObject()->indexOfMethod(normalizedSignature);

	if (methodIndex != -1) {
		QMetaObject::invokeMethod(this, func.toLatin1(), Qt::DirectConnection,
								  Q_ARG(QVariant, message.data()),
								  Q_ARG(QVariant, orig.data())
								  );
		return;
	}



	normalizedSignature = QMetaObject::normalizedSignature(func.toLatin1()+"(QVariant)");

	methodIndex = this->metaObject()->indexOfMethod(normalizedSignature);

	if (methodIndex != -1) {
		QMetaObject::invokeMethod(this, func.toLatin1(), Qt::DirectConnection,
								  Q_ARG(QVariant, message.data())
								  );
		return;
	}



	normalizedSignature = QMetaObject::normalizedSignature(func.toLatin1()+"()");

	methodIndex = this->metaObject()->indexOfMethod(normalizedSignature);

	if (methodIndex != -1) {
		QMetaObject::invokeMethod(this, func.toLatin1(), Qt::DirectConnection);
		return;
	}

	LOG_CWARNING("client") << "Invalid function:" << qPrintable(func);

}


/**
 * @brief AsyncMessageHandler::pending
 * @return
 */

bool AsyncMessageHandler::pending() const
{
	return m_pending;
}

void AsyncMessageHandler::setPending(bool newPending)
{
	if (m_pending == newPending)
		return;
	m_pending = newPending;
	emit pendingChanged();
}


/**
 * @brief AsyncMessageHandler::checkStatus
 * @param status
 * @return
 */

bool AsyncMessageHandler::checkStatus(const QJsonObject &json, const QString &status)
{
	return (json.value(QStringLiteral("status")).toString() == status);
}
