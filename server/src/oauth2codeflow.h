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

#include <QOAuth2AuthorizationCodeFlow>
#include <QPointer>

class OAuth2Authenticator;
class Client;

class OAuth2CodeFlow : public QOAuth2AuthorizationCodeFlow
{
	Q_OBJECT

public:
	explicit OAuth2CodeFlow(OAuth2Authenticator *authenticator, Client *client);
	virtual ~OAuth2CodeFlow();

	void startAuthenticate();
	void requestAccesToken(const QString &code);

	Client *client() const;

signals:
	void browserRequired(const QUrl &url);
	void authenticationSuccess(const QVariantMap &data);
	void authenticationFailed();


private slots:
	void onRequestAccessFinished();
	void onClientDestroyed();

private:
	OAuth2Authenticator *const m_authenticator;
	Client *m_client = nullptr;

};

#endif // OAUTH2CODEFLOW_H
