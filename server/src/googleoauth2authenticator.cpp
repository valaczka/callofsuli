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
#include "serverservice.h"
#include "utils_.h"
#include "querybuilder.hpp"




/**
 * @brief GoogleOAuth2Authenticator::setCodeFlow
 * @param flow
 */

void GoogleOAuth2Authenticator::setCodeFlow(const std::weak_ptr<OAuth2CodeFlow> &flow) const
{
	OAuth2CodeFlow *f = flow.lock().get();
	Q_ASSERT(f);

	f->setAuthorizationUrl(QUrl(QStringLiteral("https://accounts.google.com/o/oauth2/auth")));
	f->setAccessTokenUrl(QUrl(QStringLiteral("https://oauth2.googleapis.com/token")));
	f->setScope(QStringLiteral("email+profile"));

	f->setClientIdentifier(m_oauth.clientId);
	f->setClientIdentifierSharedKey(m_oauth.clientKey);
}










/**
 * @brief GoogleOAuth2Authenticator::profileUpdate
 * @return
 */

bool GoogleOAuth2Authenticator::profileUpdate(const QString &username, const QJsonObject &data) const
{
	LOG_CDEBUG("oauth2") << "Update user profile:" << qPrintable(username);

	const QDateTime &exp = QDateTime::fromSecsSinceEpoch(data.value(QStringLiteral("exp")).toInteger());

	if (exp <= QDateTime::currentDateTime()) {
		LOG_CTRACE("oauth2") << "Token expired, get access token for user:" << qPrintable(username);

		const QString &refreshToken = data.value(QStringLiteral("refresh")).toString();

		if (refreshToken.isEmpty()) {
			LOG_CDEBUG("oauth2") << "Missing refresh token for user:" << qPrintable(username);
			return false;
		}


		QUrlQuery q;
		q.addQueryItem(QStringLiteral("client_id"), m_oauth.clientId);
		q.addQueryItem(QStringLiteral("client_secret"), m_oauth.clientKey);
		q.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
		q.addQueryItem(QStringLiteral("refresh_token"), refreshToken);

		QNetworkRequest r{QUrl(QStringLiteral("https://accounts.google.com/o/oauth2/token"))};
		r.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

		QNetworkReply *reply = service()->networkManager()->post(r, q.toString(QUrl::FullyEncoded).toUtf8());

		connect(reply, &QNetworkReply::errorOccurred, this, [](const QNetworkReply::NetworkError &err){
			LOG_CERROR("oauth2") << "Profile update error:" << err;
		});


		connect(reply, &QNetworkReply::finished, this, [reply, this, username, data]{
			LOG_CTRACE("oauth2") << "Refresh token received for user:" << qPrintable(username);

			const QByteArray &tdata = reply->readAll();
			reply->deleteLater();

			const auto &json = Utils::byteArrayToJsonObject(tdata);

			if (!service() || !service()->databaseMain() || !json)
				return;

			const QDateTime &tokenExp = QDateTime::currentDateTime().addSecs(json->value(QStringLiteral("expires_in")).toInteger());
			const QString &accessToken = json->value(QStringLiteral("access_token")).toString();


			QJsonObject newData = data;
			newData[QStringLiteral("exp")] = tokenExp.toSecsSinceEpoch();
			newData[QStringLiteral("token")] = accessToken;


			service()->databaseMain()->worker()->execInThread([this, newData, username]{
				QSqlDatabase db = QSqlDatabase::database(service()->databaseMain()->dbName());

				QMutexLocker _locker(service()->databaseMain()->mutex());

				const QString &d = QString::fromUtf8(QJsonDocument(newData).toJson(QJsonDocument::Compact));

				if (!QueryBuilder::q(db).addQuery("UPDATE auth SET ")
						.setCombinedPlaceholder()
						.addField("oauthData", d)
						.addQuery(" WHERE username=").addValue(username)
						.exec()) {
					LOG_CERROR("oauth2") << "User update error:" << qPrintable(username);
					return;
				}

				LOG_CINFO("oauth2") << "User token updated:" << qPrintable(username);
			});

			profileUpdateWithAccessToken(username, accessToken);
		});
	} else {
		profileUpdateWithAccessToken(username, data.value(QStringLiteral("token")).toString());
	}

	return true;
}



/**
 * @brief GoogleOAuth2Authenticator::profileUpdateWithAccessToken
 * @param username
 * @param token
 */

void GoogleOAuth2Authenticator::profileUpdateWithAccessToken(const QString &username, const QString &token) const
{
	if (username.isEmpty() || token.isEmpty())
		return;

	//LOG_CINFO("oauth2") << "Update user profile:" << qPrintable(username);

	QUrlQuery q;
	q.addQueryItem(QStringLiteral("alt"), QStringLiteral("json"));
	q.addQueryItem(QStringLiteral("access_token"), token);

	QUrl url(QStringLiteral("https://www.googleapis.com/oauth2/v1/userinfo"));
	url.setQuery(q);

	QNetworkRequest r{url};
	r.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

	QNetworkReply *reply = service()->networkManager()->get(r);

	connect(reply, &QNetworkReply::errorOccurred, this, [](const QNetworkReply::NetworkError &err){
		LOG_CERROR("oauth2") << "Profile update error:" << err;
	});


	connect(reply, &QNetworkReply::finished, this, [reply, this, username]{
		LOG_CTRACE("oauth2") << "Profile received for user:" << qPrintable(username);

		const QByteArray &tdata = reply->readAll();
		reply->deleteLater();

		const auto &json = Utils::byteArrayToJsonObject(tdata);

		if (!service() || !service()->databaseMain() || !json)
			return;

		service()->databaseMain()->worker()->execInThread([this, json, username]{
			QSqlDatabase db = QSqlDatabase::database(service()->databaseMain()->dbName());

			QMutexLocker _locker(service()->databaseMain()->mutex());

			if (QueryBuilder::q(db)
					.addQuery("UPDATE user SET ").setCombinedPlaceholder()
					.addField("familyName", json->value(QStringLiteral("family_name")).toString())
					.addField("givenName", json->value(QStringLiteral("given_name")).toString())
					.addField("picture", json->value(QStringLiteral("picture")).toString())
					.addQuery(" WHERE username=").addValue(username).exec()) {
				LOG_CINFO("oauth2") << "User profile updated:" << qPrintable(username);
			} else {
				LOG_CERROR("oauth2") << "User profile update error:" << qPrintable(username);
			}
		});

	});


}







/**
 * @brief GoogleOAuth2Authenticator::parseResponse
 * @param query
 * @return
 */

OAuth2CodeFlow* GoogleOAuth2Authenticator::parseResponse(const QUrlQuery &query)

{
	const QString error = query.queryItemValue(QStringLiteral("error"));
	const QString code = query.queryItemValue(QStringLiteral("code"));
	const QString receivedState = query.queryItemValue(QStringLiteral("state"));
	if (error.size()) {
		const QString uri = query.queryItemValue(QStringLiteral("error_uri"));
		const QString description = query.queryItemValue(QStringLiteral("error_description"));

		LOG_CERROR("oauth2") << "AuthenticationError:" << qPrintable(error) << qPrintable(uri) << qPrintable(description);
		return nullptr;
	}

	if (code.isEmpty()) {
		LOG_CERROR("oauth2") << "AuthenticationError: Code not received";
		return nullptr;
	}

	if (receivedState.isEmpty()) {
		LOG_CERROR("oauth2") << "State not received";
		return nullptr;
	}

	const auto &ptr = getCodeFlowForState(receivedState);

	if (!ptr) {
		LOG_CDEBUG("oauth2") << "Flow not found for state:" << receivedState;
		return nullptr;
	}

	OAuth2CodeFlow *flow = ptr->lock().get();

	flow->requestAccesToken(code);
	return flow;
}
