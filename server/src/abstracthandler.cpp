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

	LOG_CTRACE("client") << "Abstract handler created:" << this;
}


/**
 * @brief AbstractHandler::~AbstractHandler
 */

AbstractHandler::~AbstractHandler()
{
	LOG_CTRACE("client") << "Abstract handler destroyed:" << this;
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
		LOG_CWARNING("client") << "Invalid message:" << message;
		break;
	}
}


/**
 * @brief AbstractHandler::validateJwtToken
 * @return
 */

bool AbstractHandler::validateJwtToken(const bool &autoResponseOnFail)
{
	if (m_jwtTokenValidated)
		return true;

	if (!service()) {
		LOG_CERROR("client") << "Missing ServerService";
		send(m_message.createErrorResponse(QStringLiteral("internal error")));
		return false;
	}

	const QString &token = json().value(QStringLiteral("token")).toString();

	if (token.isEmpty()) {
		LOG_CTRACE("client") << "Missing token";
		if (autoResponseOnFail)
			send(m_message.createErrorResponse(QStringLiteral("authentication required")));
		return false;
	}

	if (!Credential::verify(token, &service()->verifier())) {
		LOG_CDEBUG("client") << "Token verification failed";
		if (autoResponseOnFail)
			send(m_message.createErrorResponse(QStringLiteral("invalid token")));
		return false;
	}

	Credential c = Credential::fromJWT(token);

	if (!c.isValid()) {
		LOG_CTRACE("client") << "Invalid token";
		if (autoResponseOnFail)
			send(m_message.createErrorResponse(QStringLiteral("corrupt token")));
		return false;
	}

	setCredential(c);
	m_jwtTokenValidated = true;

	return true;
}




/**
 * @brief AbstractHandler::validateJwtToken
 * @param role
 * @param autoResponseOnFail
 * @return
 */

bool AbstractHandler::validateJwtToken(const Credential::Role &role, const bool &autoResponseOnFail)
{
	if (!validateJwtToken(autoResponseOnFail))
		return false;

	if (credential().roles().testFlag(role))
		return true;

	setCredential(Credential());
	m_jwtTokenValidated = false;

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
		LOG_CTRACE("client") << m_client << "Missing function";
		return;
	}

	if (func.startsWith(QStringLiteral("on")) || func.startsWith(QStringLiteral("_"))) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		LOG_CWARNING("client") << m_client << "Disabled function:" << qPrintable(func);
		return;
	}

	if (!QMetaObject::invokeMethod(this, func.toStdString().data(), Qt::DirectConnection)) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		LOG_CDEBUG("client") << m_client << "Invalid function:" << qPrintable(func);
		return;
	}

	LOG_CDEBUG("client") << m_client << "Request:" << qPrintable(func);
}


/**
 * @brief AbstractHandler::clear
 */

void AbstractHandler::clear()
{
	LOG_CTRACE("client") << "Clear handler" << this;
	m_message = WebSocketMessage();
	m_jwtTokenValidated = false;
	m_credential = Credential();
}


/**
 * @brief AbstractHandler::send
 */

void AbstractHandler::send(const WebSocketMessage &message, const bool &deleteHandler)
{
	if (m_client)
		m_client->send(message);
	else
		LOG_CERROR("client") << "Missing client" << this;

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
	LOG_CWARNING("client") << "Destroy handler timeout";
	this->deleteLater();
}


/**
 * @brief AbstractHandler::setCredential
 * @param newCredential
 */

void AbstractHandler::setCredential(const Credential &newCredential)
{
	m_credential = newCredential;

	if (m_client)
		m_client->setCredential(newCredential);
}


/**
 * @brief AbstractHandler::credential
 * @return
 */

const Credential &AbstractHandler::credential() const
{
	return m_credential;
}
