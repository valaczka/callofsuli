/*
 * ---- Call of Suli ----
 *
 * microsoftoauth2authenticator.h
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

#ifndef MICROSOFTOAUTH2AUTHENTICATOR_H
#define MICROSOFTOAUTH2AUTHENTICATOR_H

#include "oauth2authenticator.h"

class MicrosoftOAuth2Authenticator : public OAuth2Authenticator
{
	Q_OBJECT

public:
	explicit MicrosoftOAuth2Authenticator(ServerService *service)
		: OAuth2Authenticator("microsoft", service) { }

	void setCodeFlow(const std::weak_ptr<OAuth2CodeFlow> &flow) const override;
	OAuth2CodeFlow *parseResponse(const QUrlQuery &query) override;
	bool profileUpdateSupport() const override { return false; }
	bool profileUpdate(const QString &/*username*/, const QJsonObject &/*data*/) const override { return false; }
	void profileUpdateWithAccessToken(const QString &/*username*/, const QString &/*token*/) const override {};

	AdminAPI::User getUserInfoFromIdToken(const QJsonObject &data) const override;
};

#endif // MICROSOFTOAUTH2AUTHENTICATOR_H
