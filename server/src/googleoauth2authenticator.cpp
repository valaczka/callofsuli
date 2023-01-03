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
#include "client.h"


/**
 * @brief GoogleOAuth2Authenticator::addCodeFlow
 */

OAuth2CodeFlow *GoogleOAuth2Authenticator::addCodeFlow(Client *client)
{
	if (m_clientId.isEmpty()) {
		LOG_CERROR("oauth2") << "Client id empty";
		return nullptr;
	}

	if (m_clientKey.isEmpty()) {
		LOG_CERROR("oauth2") << "Client key empty";
		return nullptr;
	}


	OAuth2CodeFlow *flow = new OAuth2CodeFlow(this);

	flow->setAuthorizationUrl(QUrl(QStringLiteral("https://accounts.google.com/o/oauth2/auth")));
	flow->setAccessTokenUrl(QUrl(QStringLiteral("https://oauth2.googleapis.com/token")));
	flow->setScope(QStringLiteral("email profile"));

	flow->setClientIdentifier(m_clientId);
	flow->setClientIdentifierSharedKey(m_clientKey);

	flow->setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QVariantMap *parameters) {
		if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
			const QByteArray &code = parameters->value("code").toByteArray();
			(*parameters)["code"] = QUrl::fromPercentEncoding(code);
		}
	});


	OAuth2Authenticator::addCodeFlow(flow, client);

	return flow;
}
