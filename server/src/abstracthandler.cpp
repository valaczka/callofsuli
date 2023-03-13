/*
 * ---- Call of Suli ----
 *
 * abstracthandler.cpp
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractHandler
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

#include "abstracthandler.h"
#include "Logger.h"
#include "client.h"
#include "serverservice.h"

AbstractHandler::AbstractHandler(Client *client)
	: QObject(client)
	, m_client(client)
{
	Q_ASSERT(m_client);

	QTimer::singleShot(HANDLER_DESTROY_TIMEOUT_MSEC, this, &AbstractHandler::destroyTimeout);

	HANDLER_LOG_TRACE() << "Abstract handler created:" << this;
}


/**
 * @brief AbstractHandler::~AbstractHandler
 */

AbstractHandler::~AbstractHandler()
{
	if (m_client)
		m_client->removeRunningHandler(this);

	HANDLER_LOG_TRACE() << "Abstract handler destroyed:" << this;
}


/**
 * @brief AbstractHandler::handleMessage
 * @param message
 */

void AbstractHandler::handleMessage(const WebSocketMessage &message)
{
	clear();

	m_message = message;

	switch (message.opCode()) {
	case WebSocketMessage::Request:
		handleRequest();
		break;

	case WebSocketMessage::RequestResponse:
		handleRequestResponse();
		break;

	case WebSocketMessage::Event:
		handleEvent();
		break;

	default:
		HANDLER_LOG_WARNING() << "Invalid message:" << message;
		break;
	}
}


/**
 * @brief AbstractHandler::validateJwtToken
 * @return
 */

bool AbstractHandler::validateCredential(const bool &autoResponseOnFail)
{
	if (!m_client) {
		HANDLER_LOG_ERROR() << "Missing Client";
		send(m_message.createErrorResponse(QStringLiteral("internal error")));
		return false;
	}

	if (!m_client->credential().isValid()) {
		HANDLER_LOG_TRACE() << "Invalid credential";
		if (autoResponseOnFail)
			send(m_message.createErrorResponse(QStringLiteral("invalid credential")));
		return false;
	}

	return true;
}




/**
 * @brief AbstractHandler::validateJwtToken
 * @param role
 * @param autoResponseOnFail
 * @return
 */

bool AbstractHandler::validateCredential(const Credential::Role &role, const bool &autoResponseOnFail)
{
	if (!validateCredential(autoResponseOnFail))
		return false;

	if (m_client->credential().roles().testFlag(role))
		return true;

	if (autoResponseOnFail)
		send(m_message.createErrorResponse(QStringLiteral("permission denied")));

	return false;
}




/**
 * @brief AbstractHandler::handleRequest
 */

void AbstractHandler::handleRequest()
{
	const QString &func = json().value(QStringLiteral("func")).toString();

	if (func.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		HANDLER_LOG_TRACE() << m_client << "Missing function";
		return;
	}

	if (func.startsWith(QStringLiteral("on")) || func.startsWith(QStringLiteral("_"))) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		HANDLER_LOG_WARNING() << m_client << "Disabled function:" << qPrintable(func);
		return;
	}

	if (m_defaultRoleToValidate != Credential::None && !validateCredential(m_defaultRoleToValidate))
		return;

	if (!QMetaObject::invokeMethod(this, func.toStdString().data(), Qt::DirectConnection)) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		HANDLER_LOG_DEBUG() << m_client << "Invalid function:" << qPrintable(func);
		return;
	}

	HANDLER_LOG_DEBUG() << m_client << "Request:" << qPrintable(func);
}


/**
 * @brief AbstractHandler::clear
 */

void AbstractHandler::clear()
{
	HANDLER_LOG_TRACE() << "Clear handler" << this;
	m_message = WebSocketMessage();
}


/**
 * @brief AbstractHandler::send
 */

void AbstractHandler::send(const WebSocketMessage &message, const bool &deleteHandler)
{
	if (m_client)
		m_client->send(message);
	else
		HANDLER_LOG_ERROR() << "Missing client" << this;

	if (deleteHandler)
		this->deleteLater();
}




ServerService *AbstractHandler::service() const
{
	if (m_client)
		return m_client->service();
	else
		return nullptr;
}


/**
 * @brief AbstractHandler::databaseMain
 * @return
 */

DatabaseMain *AbstractHandler::databaseMain() const
{
	if (service())
		return service()->databaseMain();
	else
		return nullptr;
}


/**
 * @brief AbstractHandler::destroyAfterTimeout
 */

void AbstractHandler::destroyTimeout()
{
	HANDLER_LOG_WARNING() << "Destroy handler timeout";
	this->deleteLater();
}

