/*
 * ---- Call of Suli ----
 *
 * authapi.h
 *
 * Created on: 2023. 03. 14.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AuthAPI
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

#ifndef AUTHAPI_H
#define AUTHAPI_H

#include "abstractapi.h"
#include "oauth2authenticator.h"

class AuthAPI : public AbstractAPI
{
public:
	AuthAPI(ServerService *service);

	void login(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void loginOAuth2(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

	QDeferred<Credential> getCredential(const QString &username) const;
	QDeferred<Credential> authorizePlain(const Credential &credential, const QString &password) const;
	QDeferred<Credential> authorizeOAuth2(const Credential &credential, const char *oauthType) const;

	QJsonObject getToken(const Credential &credential) const;
};

#endif // AUTHAPI_H
