/*
 * ---- Call of Suli ----
 *
 * oauth2replyhandler.cpp
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2ReplyHandler
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

#include "oauth2replyhandler.h"
#include "Logger.h"
#include "oauth2authenticator.h"

/**
 * @brief OAuth2ReplyHandler::~OAuth2ReplyHandler
 */

OAuth2ReplyHandler::OAuth2ReplyHandler(OAuth2Authenticator *authenticator)
	: QObject{authenticator}
	, m_authenticator(authenticator)
{
	Q_ASSERT(m_authenticator);

	LOG_CTRACE("oauth2") << "Reply handler created" << this;
}


OAuth2ReplyHandler::~OAuth2ReplyHandler()
{
	if (m_handler)
		delete m_handler;

	LOG_CTRACE("oauth2") << "Reply handler destroyed" << this;
}




/**
 * @brief OAuth2ReplyHandler::listen
 * @param address
 * @param port
 */

bool OAuth2ReplyHandler::listen(const QHostAddress &address, quint16 port)
{
	if (m_handler)
		delete m_handler;

	m_handler = new _Handler(m_authenticator, address, port, this);

	connect(m_handler, &QOAuthHttpServerReplyHandler::callbackReceived, this, &OAuth2ReplyHandler::onCallbackReceived);

	LOG_CINFO("oauth2") << "OAuth2Handler listening on" << address << port;

	return m_handler->isListening();
}



OAuth2Authenticator *OAuth2ReplyHandler::authenticator() const
{
	return m_authenticator;
}



/**
 * @brief OAuth2ReplyHandler::onCallbackReceived
 * @param data
 */

void OAuth2ReplyHandler::onCallbackReceived(const QVariantMap &data)
{
	LOG_CTRACE("oauth2") << "Callback received" << data;

	const QString error = data.value(QStringLiteral("error")).toString();
	const QString code = QUrl::fromPercentEncoding(data.value(QStringLiteral("code")).toByteArray());
	const QString receivedState = data.value(QStringLiteral("state")).toString();
	if (error.size()) {
		const QString uri = data.value(QStringLiteral("error_uri")).toString();
		const QString description = data.value(QStringLiteral("error_description")).toString();

		LOG_CERROR("oauth2") << "AuthenticationError:" << qPrintable(error) << qPrintable(uri) << qPrintable(description);
		return;
	}

	if (code.isEmpty()) {
		LOG_CERROR("oauth2") << "AuthenticationError: Code not received";
		return;
	}

	if (receivedState.isEmpty()) {
		LOG_CERROR("oauth2") << "State not received";
		return;
	}

	OAuth2CodeFlow *flow = m_authenticator->getCodeFlowForState(receivedState);


	if (flow) {
		flow->requestAccesToken(code);
	} else {
		LOG_CTRACE("oauth2") << "Flow not found" << flow;
	}
}


/**
 * @brief OAuth2ReplyHandler::handler
 * @return
 */

QOAuthHttpServerReplyHandler *OAuth2ReplyHandler::handler() const
{
	return m_handler;
}


/**
 * @brief _Handler::callback
 * @return
 */

QString _Handler::callback() const
{
	return m_authenticator->listenCallback();
}
