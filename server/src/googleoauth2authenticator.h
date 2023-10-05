/*
 * ---- Call of Suli ----
 *
 * googleoauth2authenticator.h
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

#ifndef GOOGLEOAUTH2AUTHENTICATOR_H
#define GOOGLEOAUTH2AUTHENTICATOR_H

#include "oauth2authenticator.h"


/**
 * @brief The GoogleOAuth2Authenticator class
 */

class GoogleOAuth2Authenticator : public OAuth2Authenticator
{
	Q_OBJECT

public:
	explicit GoogleOAuth2Authenticator(ServerService *service)
		: OAuth2Authenticator("google", service) { }

	void setCodeFlow(OAuth2CodeFlow *flow) const override;
	bool parseResponse(const QUrlQuery &query) override;
	QJsonObject localAuthData() const override;

	bool profileUpdateSupport() const override { return true; };
	Q_INVOKABLE bool profileUpdate(const QString &username, const QJsonObject &data) const override;
	void profileUpdateWithAccessToken(const QString &username, const QString &token) const override;

};



#endif // GOOGLEOAUTH2AUTHENTICATOR_H
