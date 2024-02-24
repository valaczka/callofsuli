/*
 * ---- Call of Suli ----
 *
 * oauth2codeflow.h
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

#ifndef OAUTH2CODEFLOW_H
#define OAUTH2CODEFLOW_H

#include "credential.h"
#include "qtimer.h"
#include <QOAuth2AuthorizationCodeFlow>
#include <QPointer>
#include "adminapi.h"

class OAuth2Authenticator;


/**
 * @brief The OAuth2CodeFlow class
 */

class OAuth2CodeFlow : public QOAuth2AuthorizationCodeFlow
{
	Q_OBJECT

public:
	explicit OAuth2CodeFlow(OAuth2Authenticator *authenticator);
	virtual ~OAuth2CodeFlow();

	enum AuthState {
		Invalid = 0,
		Pending,
		Failed,
		TokenReceived,
		Authenticated,
		UserExists,
		InvalidCode,
		InvalidDomain
	};

	struct Token {
		QString token;
		QString refreshToken;
		QDateTime expiration;
		QString idToken;

		QJsonObject toJson() const;
	};

	QUrl requestAuthorizationUrl();
	virtual void requestAccesToken(const QString &code);

	AuthState authState() const;
	void setAuthState(AuthState newAuthState);

	const Token &token() const;

	const Credential &credential() const;
	void setCredential(const Credential &newCredential);

	OAuth2Authenticator *authenticator() const;

	AdminAPI::User getUserInfo() const;
	QJsonObject getJWT() const;

	bool isWasm() const;
	void setIsWasm(bool newIsWasm);

signals:
	void authenticated();

private slots:
	void onRequestAccessFinished();
	void onRemoveTimerTimeout();

protected:
	OAuth2Authenticator *const m_authenticator;
	AuthState m_authState = Invalid;
	Token m_token;
	Credential m_credential;
	bool m_isWasm = false;

private:
	QTimer m_removeTimer;

};



#endif // OAUTH2CODEFLOW_H
