/*
 * ---- Call of Suli ----
 *
 * microsoftoauth2authenticator.cpp
 *
 * Created on: 2023. 07. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MicrosoftOAuth2Authenticator
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

#include "microsoftoauth2authenticator.h"

/**
 * @brief MicrosoftOAuth2Authenticator::setCodeFlow
 * @param flow
 */

void MicrosoftOAuth2Authenticator::setCodeFlow(const std::weak_ptr<OAuth2CodeFlow> &flow) const
{
	OAuth2CodeFlow *f = flow.lock().get();
	Q_ASSERT(f);

	f->setAuthorizationUrl(QUrl(QStringLiteral("https://login.microsoftonline.com/%1/oauth2/v2.0/authorize").arg(m_oauth.tenant)));
	f->setAccessTokenUrl(QUrl(QStringLiteral("https://login.microsoftonline.com/%1/oauth2/v2.0/token").arg(m_oauth.tenant)));
	f->setScope(QStringLiteral("openid+email+profile"));

	f->setClientIdentifier(m_oauth.clientId);
	f->setClientIdentifierSharedKey(m_oauth.clientKey);
}



/**
 * @brief MicrosoftOAuth2Authenticator::parseResponse
 * @param query
 * @return
 */

OAuth2CodeFlow* MicrosoftOAuth2Authenticator::parseResponse(const QUrlQuery &query)
{
	const QString error = query.queryItemValue(QStringLiteral("error"));
	const QString code = query.queryItemValue(QStringLiteral("code"));
	const QString receivedState = query.queryItemValue(QStringLiteral("state"));

	if (error.size()) {
		const QString uri = query.queryItemValue(QStringLiteral("error_uri"));
		const QString description = query.queryItemValue(QStringLiteral("error_description"));

		LOG_CERROR("oauth2") << "AuthenticationError:" << qPrintable(error) << qPrintable(uri) << qPrintable(description);
		return nullptr;
	}

	if (code.isEmpty()) {
		LOG_CERROR("oauth2") << "AuthenticationError: Code not received";
		return nullptr;
	}

	if (receivedState.isEmpty()) {
		LOG_CERROR("oauth2") << "State not received";
		return nullptr;
	}


	const auto &ptr = getCodeFlowForState(receivedState);

	if (!ptr) {
		LOG_CDEBUG("oauth2") << "Flow not found for state:" << receivedState;
		return nullptr;
	}

	OAuth2CodeFlow *flow = ptr->lock().get();
	flow->requestAccesToken(code);
	return flow;
}


/**
 * @brief MicrosoftOAuth2Authenticator::getUserInfoFromIdToken
 * @param data
 * @return
 */

AdminAPI::User MicrosoftOAuth2Authenticator::getUserInfoFromIdToken(const QJsonObject &data) const
{
	AdminAPI::User user;

	const QString &name = data.value(QStringLiteral("name")).toString();

	user.username = data.value(QStringLiteral("email")).toString();
	user.familyName = name.section(' ', 0, 0);
	user.givenName = name.section(' ', 1);

	return user;
}
