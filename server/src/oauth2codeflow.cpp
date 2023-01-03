/*
 * ---- Call of Suli ----
 *
 * oauth2codeflow.cpp
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2CodeFlow
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

#include "oauth2codeflow.h"
#include "Logger.h"
#include "oauth2authenticator.h"

OAuth2CodeFlow::OAuth2CodeFlow(OAuth2Authenticator *authenticator)
	: QOAuth2AuthorizationCodeFlow{authenticator}
{
	LOG_CTRACE("oauth2") << "OAuth2CodeFlow created" << this;

	connect(this, &QOAuth2AuthorizationCodeFlow::statusChanged, this, &OAuth2CodeFlow::onStatusChanged);

//			connect(&m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &GoogleOAuth2::browserRequest);

}


/**
 * @brief OAuth2CodeFlow::~OAuth2CodeFlow
 */

OAuth2CodeFlow::~OAuth2CodeFlow()
{
	LOG_CTRACE("oauth2") << "OAuth2CodeFlow destroyed" << this;
}


/**
 * @brief OAuth2CodeFlow::onStatusChanged
 * @param status
 */

void OAuth2CodeFlow::onStatusChanged(const Status &status)
{
	LOG_CTRACE("oauth2") << "OAuth2CodeFlow status changed:" << (int) status << this;

	/*if (status == QAbstractOAuth::Status::Granted) {
			emit authenticated(m_oauth.token(), m_oauth.expirationAt().toString("yyyy-MM-dd HH:mm:ss"), m_oauth.refreshToken());
	}*/
}
