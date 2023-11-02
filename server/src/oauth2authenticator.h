/*
 * ---- Call of Suli ----
 *
 * oauth2authenticator.h
 *
 * Created on: 2023. 01. 03.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2Authenticator
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

#ifndef OAUTH2AUTHENTICATOR_H
#define OAUTH2AUTHENTICATOR_H

#include <QObject>
#include <QtNetworkAuth>
#include "oauth2codeflow.h"
#include "adminapi.h"
#include "serversettings.h"

class ServerService;
class OAuth2ReplyHandler;

/**
 * @brief The OAuth2Authenticator class
 */

class OAuth2Authenticator : public QObject
{
	Q_OBJECT

public:
	explicit OAuth2Authenticator(const char *type, ServerService *service);
	virtual ~OAuth2Authenticator();

	virtual void setCodeFlow(const std::weak_ptr<OAuth2CodeFlow> &flow) const = 0;
	virtual QJsonObject localAuthData() const = 0;
	virtual bool parseResponse(const QUrlQuery &query) = 0;
	virtual bool profileUpdateSupport() const = 0;
	Q_INVOKABLE virtual bool profileUpdate(const QString &username, const QJsonObject &data) const = 0;
	virtual void profileUpdateWithAccessToken(const QString &username, const QString &token) const = 0;

	Q_INVOKABLE std::weak_ptr<OAuth2CodeFlow> addCodeFlow();

	void removeCodeFlow(OAuth2CodeFlow *flow);
	std::optional<std::weak_ptr<OAuth2CodeFlow> > getCodeFlowForState(const QString &status) const;

	const ServerSettings::OAuth &oauth() const;
	void setOAuth(const ServerSettings::OAuth &newOauth);

	ServerService *service() const;

	const char *type() const;

	virtual AdminAPI::User getUserInfoFromIdToken(const QJsonObject &data) const;

	static const char *callbackPath();

protected:
	std::shared_ptr<QVector<std::shared_ptr<OAuth2CodeFlow> > > m_codeFlowList;
	std::unique_ptr<OAuth2ReplyHandler> m_handler = nullptr;
	ServerSettings::OAuth m_oauth;

private:
	const char* m_type = "";
	ServerService *const m_service = nullptr;
	static const char* m_callbackPath;
};




/**
 * @brief The OAuth2ReplyHandler class
 */

class OAuth2ReplyHandler : public QAbstractOAuthReplyHandler
{
	Q_OBJECT

public:
	explicit OAuth2ReplyHandler(OAuth2Authenticator *authenticator);
	virtual ~OAuth2ReplyHandler();

	QString callback() const override;

public slots:
	void networkReplyFinished(QNetworkReply *) override {}

private:
	OAuth2Authenticator *const m_authenticator = nullptr;
};

#endif // OAUTH2AUTHENTICATOR_H
