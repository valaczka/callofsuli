/*
 * ---- Call of Suli ----
 *
 * oauth2authenticator.cpp
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2Authenticator
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

#include "oauth2authenticator.h"
#include "Logger.h"
#include "oauth2codeflow.h"
#include "serverservice.h"

OAuth2Authenticator::OAuth2Authenticator(const char *type, ServerService *service)
	: QObject{service}
	, m_handler(new OAuth2ReplyHandler(this))
	, m_type(type)
	, m_service(service)
{
	Q_ASSERT(m_service);

	LOG_CTRACE("oauth2") << "OAuth2Authenticator created" << m_type << this;
}


/**
 * @brief OAuth2Authenticator::~OAuth2Authenticator
 */

OAuth2Authenticator::~OAuth2Authenticator()
{
	qDeleteAll(m_codeFlowList);
	delete m_handler;

	LOG_CTRACE("oauth2") << "OAuth2Authenticator destroyed" << m_type << this;
}



/**
 * @brief OAuth2Authenticator::addCodeFlow
 * @return
 */

OAuth2CodeFlow *OAuth2Authenticator::addCodeFlow()
{
	OAuth2CodeFlow *flow = new OAuth2CodeFlow(this);

	flow->setReplyHandler(m_handler);

	setCodeFlow(flow);

	LOG_CDEBUG("oauth2") << "Add new code flow" << flow->state();

	m_codeFlowList.append(flow);


	return flow;
}


/**
 * @brief OAuth2Authenticator::removeCodeFlow
 * @param flow
 */

void OAuth2Authenticator::removeCodeFlow(OAuth2CodeFlow *flow)
{
	if (m_codeFlowList.contains(flow)) {
		LOG_CTRACE("oauth2") << "Remove code flow:" << flow;
		m_codeFlowList.removeAll(flow);
	} else {
		LOG_CDEBUG("oauth2") << "Code flow not found:" << flow;
	}
}





/**
 * @brief OAuth2Authenticator::getCodeFlowForStatus
 * @param status
 * @return
 */

OAuth2CodeFlow *OAuth2Authenticator::getCodeFlowForState(const QString &state) const
{
	foreach (OAuth2CodeFlow *f, m_codeFlowList)
		if (f->state() == state)
			return f;

	return nullptr;
}




ServerService *OAuth2Authenticator::service() const
{
	return m_service;
}



/**
 * @brief OAuth2Authenticator::updateUserInfoFromIdToken
 * @param data
 */

AdminAPI::User OAuth2Authenticator::getUserInfoFromIdToken(const QJsonObject &data) const
{
	AdminAPI::User user;

	bool emailVerified = data.value(QStringLiteral("email_verified")).toBool();

	if (!emailVerified) {
		LOG_CDEBUG("oauth2") << "Email not verified";
		return user;
	}

	const QString &email = data.value(QStringLiteral("email")).toString();

	if (email.isEmpty()) {
		LOG_CDEBUG("oauth2") << "Missing email";
		return user;
	}

	user.username = email;
	user.familyName = data.value(QStringLiteral("family_name")).toString();
	user.givenName = data.value(QStringLiteral("given_name")).toString();
	user.picture = data.value(QStringLiteral("picture")).toString();

	return user;
}

const ServerSettings::OAuth &OAuth2Authenticator::oauth() const
{
	return m_oauth;
}

void OAuth2Authenticator::setOAuth(const ServerSettings::OAuth &newOauth)
{
	m_oauth = newOauth;
}





/**
 * @brief OAuth2Authenticator::type
 * @return
 */

const char *OAuth2Authenticator::type() const
{
	return m_type;
}





/**
 * @brief OAuth2ReplyHandler::OAuth2ReplyHandler
 * @param service
 */

OAuth2ReplyHandler::OAuth2ReplyHandler(OAuth2Authenticator *authenticator)
	: QAbstractOAuthReplyHandler(authenticator)
	, m_authenticator(authenticator)
{
	Q_ASSERT(m_authenticator);

	LOG_CTRACE("oauth2") << "Reply handler created" << this;
}


/**
 * @brief OAuth2ReplyHandler::~OAuth2ReplyHandler
 */

OAuth2ReplyHandler::~OAuth2ReplyHandler()
{
	LOG_CTRACE("oauth2") << "Reply handler destroyed" << this;
}



/**
 * @brief OAuth2ReplyHandler::callback
 * @return
 */

QString OAuth2ReplyHandler::callback() const
{
	Q_ASSERT(m_authenticator);

	const QUrl url(QString::fromLatin1("%1://%2:%3/cb/%4")
				   .arg(m_authenticator->service()->settings()->ssl() ? QStringLiteral("https") : QStringLiteral("http"))
				   .arg(m_authenticator->service()->webServer()->redirectHost())
				   .arg(m_authenticator->service()->settings()->listenPort())
				   .arg(m_authenticator->oauth().path)
				   );
	return url.toString(QUrl::EncodeDelimiters);
}


