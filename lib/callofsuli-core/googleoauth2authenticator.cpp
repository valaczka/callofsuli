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


/**
 * @brief GoogleOAuth2Authenticator::addCodeFlow
 */

OAuth2CodeFlow *GoogleOAuth2Authenticator::addCodeFlow(QObject *referenceObject)
{
	if (m_clientId.isEmpty()) {
		LOG_CERROR("oauth2") << "Client id empty";
		return nullptr;
	}

	if (m_clientKey.isEmpty()) {
		LOG_CERROR("oauth2") << "Client key empty";
		return nullptr;
	}


	OAuth2CodeFlow *flow = new OAuth2CodeFlow(this, referenceObject);

	setCodeFlow(flow);

	OAuth2Authenticator::addCodeFlow(flow);

	return flow;
}


/**
 * @brief GoogleOAuth2Authenticator::addCodeFlow
 * @param flow
 * @return
 */

OAuth2CodeFlow *GoogleOAuth2Authenticator::addCodeFlow(OAuth2CodeFlow *flow)
{
	setCodeFlow(flow);

	OAuth2Authenticator::addCodeFlow(flow);

	return flow;
}


/**
 * @brief GoogleOAuth2Authenticator::setCodeFlow
 * @param flow
 */

void GoogleOAuth2Authenticator::setCodeFlow(OAuth2CodeFlow *flow) const
{
	Q_ASSERT(flow);

	flow->setAuthorizationUrl(QUrl(QStringLiteral("https://accounts.google.com/o/oauth2/auth")));
	flow->setAccessTokenUrl(QUrl(QStringLiteral("https://oauth2.googleapis.com/token")));
	flow->setScope(QStringLiteral("email profile"));

	flow->setClientIdentifier(m_clientId);
	flow->setClientIdentifierSharedKey(m_clientKey);
}






/**
 * @brief GoogleOAuth2AccessCodeFlow::getUserInfoWithAccessToken
 * @param accessToken
 */

void GoogleOAuth2AccessCodeFlow::getUserInfoWithAccessToken(const QString &accessToken) const
{
	LOG_CTRACE("oauth2") << "Get user info with Google access token";

	QUrl url(QStringLiteral("https://www.googleapis.com/oauth2/v1/userinfo"));

	QUrlQuery q;
	q.addQueryItem(QStringLiteral("alt"), QStringLiteral("json"));
	q.addQueryItem(QStringLiteral("access_token"), accessToken);
	url.setQuery(q);

	QNetworkRequest request(url);

	QNetworkReply *reply = m_authenticator->networkManager()->get(request);

	QObject::connect(reply, &QNetworkReply::finished, this, &GoogleOAuth2AccessCodeFlow::onRequestFinished);
}


/**
 * @brief GoogleOAuth2AccessCodeFlow::onRequestFinished
 */

void GoogleOAuth2AccessCodeFlow::onRequestFinished()
{
	LOG_CTRACE("oauth2") << "Request user info with Google access token finished";

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if (!reply) {
		LOG_CERROR("oauth2") << "Invalid QNetworkReply";
		emit getUserInfoFailed();
		return;
	}

	if (reply->error() != QNetworkReply::NoError) {
		LOG_CERROR("oauth2") << "NetworkReply error:" << qPrintable(reply->errorString());
		emit getUserInfoFailed();
		reply->deleteLater();
		return;
	}
	if (reply->header(QNetworkRequest::ContentTypeHeader).isNull()) {
		LOG_CERROR("oauth2") << "Empty Content-type header";
		emit getUserInfoFailed();
		reply->deleteLater();
		return;
	}

	const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).isNull() ?
				QStringLiteral("text/html") :
				reply->header(QNetworkRequest::ContentTypeHeader).toString();

	const QByteArray data = reply->readAll();

	reply->deleteLater();

	if (data.isEmpty()) {
		LOG_CERROR("oauth2") << "No received data";
		emit getUserInfoFailed();
		return;
	}

	QVariantMap ret;

	if (contentType.startsWith(QStringLiteral("text/html")) ||
			contentType.startsWith(QStringLiteral("application/x-www-form-urlencoded"))) {
		QUrlQuery query(QString::fromUtf8(data));
		auto queryItems = query.queryItems(QUrl::FullyDecoded);

		for (auto it = queryItems.begin(), end = queryItems.end(); it != end; ++it)
			ret.insert(it->first, it->second);
	} else if (contentType.startsWith(QStringLiteral("application/json"))
			   || contentType.startsWith(QStringLiteral("text/javascript"))) {
		const QJsonDocument document = QJsonDocument::fromJson(data);
		if (!document.isObject()) {
			LOG_CERROR("oauth2") << "Received data is not a JSON object:" << qPrintable(QString::fromUtf8(data));
			emit getUserInfoFailed();
			return;
		}
		const QJsonObject object = document.object();
		if (object.isEmpty()) {
			LOG_CERROR("oauth2") << "Received empty JSON object:" << qPrintable(QString::fromUtf8(data));
		}
		ret = object.toVariantMap();
	} else {
		LOG_CERROR("oauth2") << "Unknown content-type:" << qPrintable(contentType);
		emit getUserInfoFailed();
		return;
	}

	LOG_CTRACE("oauth2") << "Received data:" << ret;

	emit getUserInfoSuccess(ret);
}
