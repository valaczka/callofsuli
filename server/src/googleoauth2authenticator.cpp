/*
 * ---- Call of Suli ----
 *
 * googleoauth2authenticator.cpp
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GoogleOAuth2Authenticator
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

#include "googleoauth2authenticator.h"
#include "Logger.h"




/**
 * @brief GoogleOAuth2Authenticator::setCodeFlow
 * @param flow
 */

void GoogleOAuth2Authenticator::setCodeFlow(OAuth2CodeFlow *flow) const
{
	Q_ASSERT(flow);

	flow->setAuthorizationUrl(QUrl(QStringLiteral("https://accounts.google.com/o/oauth2/auth")));
	flow->setAccessTokenUrl(QUrl(QStringLiteral("https://oauth2.googleapis.com/token")));
	flow->setScope(QStringLiteral("email+profile"));

	flow->setClientIdentifier(m_oauth.clientId);
	flow->setClientIdentifierSharedKey(m_oauth.clientKey);
}



/**
 * @brief GoogleOAuth2Authenticator::localAuthData
 * @return
 */

QJsonObject GoogleOAuth2Authenticator::localAuthData() const
{
	QJsonObject d;

	if (m_oauth.localClientId.isEmpty() || m_oauth.localClientKey.isEmpty())
		return d;

	d[QStringLiteral("authorization_url")] = QStringLiteral("https://accounts.google.com/o/oauth2/auth");
	d[QStringLiteral("access_token_url")] = QStringLiteral("https://oauth2.googleapis.com/token");
	d[QStringLiteral("scope")] = QStringLiteral("email+profile");
	d[QStringLiteral("client_id")] = m_oauth.localClientId;
	d[QStringLiteral("client_key")] = m_oauth.localClientKey;

	return d;
}


/**
 * @brief GoogleOAuth2Authenticator::parseResponse
 * @param query
 * @return
 */

bool GoogleOAuth2Authenticator::parseResponse(const QUrlQuery &query)

{
	const QString error = query.queryItemValue(QStringLiteral("error"));
	const QString code = query.queryItemValue(QStringLiteral("code"));
	const QString receivedState = query.queryItemValue(QStringLiteral("state"));
	if (error.size()) {
		const QString uri = query.queryItemValue(QStringLiteral("error_uri"));
		const QString description = query.queryItemValue(QStringLiteral("error_description"));

		LOG_CERROR("oauth2") << "AuthenticationError:" << qPrintable(error) << qPrintable(uri) << qPrintable(description);
		return false;
	}

	if (code.isEmpty()) {
		LOG_CERROR("oauth2") << "AuthenticationError: Code not received";
		return false;
	}

	if (receivedState.isEmpty()) {
		LOG_CERROR("oauth2") << "State not received";
		return false;
	}

	OAuth2CodeFlow *flow = getCodeFlowForState(receivedState);


	if (flow) {
		flow->requestAccesToken(code);
		return true;
	} else {
		LOG_CTRACE("oauth2") << "Flow not found" << flow;
		return false;
	}
}
