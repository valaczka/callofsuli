/*
 * ---- Call of Suli ----
 *
 * authhandler.cpp
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AuthHandler
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

#include "authhandler.h"
#include "client.h"
#include "oauth2codeflow.h"
#include "serverservice.h"

AuthHandler::AuthHandler(Client *client)
	: AbstractHandler(client)
{

}


/**
 * @brief AuthHandler::handleRequest
 */

void AuthHandler::handleRequest()
{
	const QString &func = json().value(QStringLiteral("func")).toString();

	if (func.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		LOG_CTRACE("client") << m_client << "Missing function";
		return;
	}

	if (func.startsWith(QStringLiteral("on"))) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		LOG_CWARNING("client") << m_client << "Disabled function:" << qPrintable(func);
		return;
	}

	if (!QMetaObject::invokeMethod(this, func.toStdString().data(), Qt::DirectConnection)) {
		send(m_message.createErrorResponse(QStringLiteral("missing func key")));
		LOG_CDEBUG("client") << m_client << "Invalid function:" << qPrintable(func);
		return;
	}
}

void AuthHandler::handleRequestResponse()
{

}

void AuthHandler::handleEvent()
{

}



/**
 * @brief AuthHandler::loginGoogle
 */

void AuthHandler::loginGoogle()
{
	if (m_client->oauth2CodeFlow()) {
		send(m_message.createErrorResponse(QStringLiteral("login already in progress")));
		LOG_CWARNING("client") << m_client << "OAuth2CodeFlow already exists";
		return;
	}

	OAuth2CodeFlow *flow = service()->googleOAuth2Authenticator()->addCodeFlow(m_client);
	m_client->setOauth2CodeFlow(flow);

	if (flow) {
		send(m_message.createStatusResponse());

		connect(flow, &OAuth2CodeFlow::authenticationFailed, this, &AuthHandler::onOAuthFailed);
		connect(flow, &OAuth2CodeFlow::authenticationSuccess, this, &AuthHandler::onOAuthSuccess);

		flow->startAuthenticate();
	} else {
		send(m_message.createErrorResponse(QStringLiteral("unable to create oauth2 code flow")));
	}

}



void AuthHandler::test()
{
	service()->databaseMain()->test();
}




/**
 * @brief AuthHandler::onOAuthFailed
 */

void AuthHandler::onOAuthFailed()
{
	send(WebSocketMessage::createErrorEvent(QStringLiteral("authentication failed"), WebSocketMessage::ClassAuth));
	service()->googleOAuth2Authenticator()->removeCodeFlow(qobject_cast<OAuth2CodeFlow*>(sender()));
	if (m_client)
		m_client->setOauth2CodeFlow(nullptr);
}


/**
 * @brief AuthHandler::onOAuthSucceed
 * @param data
 */

void AuthHandler::onOAuthSuccess(const QVariantMap &data)
{
	service()->googleOAuth2Authenticator()->removeCodeFlow(qobject_cast<OAuth2CodeFlow*>(sender()));

	if (m_client)
		m_client->setOauth2CodeFlow(nullptr);


	auto map = GoogleOAuth2Authenticator::getInfoFromRequestAccess(data);

	QString email = QString::fromStdString(map.value("email"));

	/**
	{
	  "iss": "https://accounts.google.com",
	  "azp": "822493401756-d8ooae3ja5s9g5a1u7ld5b38es1no61f.apps.googleusercontent.com",
	  "aud": "822493401756-d8ooae3ja5s9g5a1u7ld5b38es1no61f.apps.googleusercontent.com",
	  "sub": "117872218068592040950",
	  "email": "valaczka@gmail.com",
	  "email_verified": true,
	  "at_hash": "ZP9S8fNZSxKf1MNF1VhXnA",
	  "name": "János Pál Valaczka",
	  "picture": "https://lh3.googleusercontent.com/a/AEdFTp43_bD1XcEzYyBDId_CikF17qC-vXsYRjh9NV3hrw=s96-c",
	  "given_name": "János Pál",
	  "family_name": "Valaczka",
	  "locale": "hu",
	  "iat": 1672832265,
	  "exp": 1672835865
	}

	*/

	if (QString::fromStdString(map.value("email_verified")) == QLatin1String("true")) {
		LOG_CDEBUG("client") << "Authenticated with Google:" << qPrintable(email);

		loginUser(email);
	} else {
		send(WebSocketMessage::createErrorEvent(QStringLiteral("Authenticated email not verified"), WebSocketMessage::ClassAuth));
	}

}



/**
 * @brief AuthHandler::loginUser
 * @param username
 */

void AuthHandler::loginUser(const QString &username)
{
	LOG_CINFO("client") << "Login:" << qPrintable(username);
}
