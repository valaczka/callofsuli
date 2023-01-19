/*
 * ---- Call of Suli ----
 *
 * authhandler.h
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

#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include "abstracthandler.h"

class OAuth2Authenticator;
class OAuth2CodeFlow;
class GoogleOAuth2Authenticator;

class AuthHandler : public AbstractHandler
{
	Q_OBJECT

public:
	AuthHandler(Client *client);

	QDeferred<Credential> getCredential(const QString &username) const;
	QDeferred<Credential> getCredential(const QString &username, const qint64 &iat) const;
	QDeferred<Credential> authorizePlain(const Credential &credential, const QString &password) const;
	QDeferred<Credential> authorizeOAuth2(const Credential &credential, const char *oauthType) const;

	QDeferred<bool> userExists(const QString &username) const;

	bool isRegistrationEnabled() const;
	void setRegistrationEnabled(const bool &on = true);
	bool isOAuth2RegistrationForced() const;
	void setOAuth2RegistrationForced(const bool &on = true);


protected:
	void handleRequestResponse();
	void handleEvent();

private slots:
	void loginGoogle();
	void loginPlain();
	void loginToken();

	void logout();

	void registrationGoogle();
	void registrationPlain();

	void getGoogleLocalClientId();

private:
	void _loginUser(const Credential &credential, const bool &createToken = true);
	void _loginWithAccessToken(const QVariantMap &data, const char *provider);
	void _registrationWithAccessToken(const QVariantMap &data, const char *provider);

	void _OAuthFailed(OAuth2Authenticator *authenticator, OAuth2CodeFlow *flow);
	void _OAuthSuccess(OAuth2Authenticator *authenticator, OAuth2CodeFlow *flow);

	void _loginGoogleOAuthSuccess(const QVariantMap &data, GoogleOAuth2Authenticator *authenticator, OAuth2CodeFlow *flow);
	void _loginGoogleWithAccessToken(const QString &accessToken, GoogleOAuth2Authenticator *authenticator);
	void _registrationGoogleOAuthSuccess(const QVariantMap &data, GoogleOAuth2Authenticator *authenticator, OAuth2CodeFlow *flow);
	void _registrationGoogleWithAccessToken(const QString &accessToken, GoogleOAuth2Authenticator *authenticator);


};

#endif // AUTHHANDLER_H
