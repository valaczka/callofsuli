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

OAuth2CodeFlow::OAuth2CodeFlow(OAuth2Authenticator *authenticator, QObject *referenceObject)
	: QOAuth2AuthorizationCodeFlow{authenticator}
	, m_authenticator(authenticator)
	, m_referenceObject(referenceObject)
{
	Q_ASSERT(m_authenticator);

	if (m_referenceObject)
		connect(m_referenceObject, &QObject::destroyed, this, &OAuth2CodeFlow::onReferenceObjectDestroyed);

	LOG_CTRACE("oauth2") << "OAuth2CodeFlow created" << state() << this;
}


/**
 * @brief OAuth2CodeFlow::~OAuth2CodeFlow
 */

OAuth2CodeFlow::~OAuth2CodeFlow()
{
	LOG_CTRACE("oauth2") << "OAuth2CodeFlow destroyed" << this;
}


/**
 * @brief OAuth2CodeFlow::requestAuthorizationUrl
 * @return
 */

QUrl OAuth2CodeFlow::requestAuthorizationUrl()
{
	return buildAuthenticateUrl();
}





/**
 * @brief OAuth2CodeFlow::requestAccesToken
 * @param code
 */

void OAuth2CodeFlow::requestAccesToken(const QString &code)
{
	LOG_CTRACE("oauth2") << "Start request token";

	QNetworkRequest request(accessTokenUrl());

	QUrlQuery query;
	query.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
	query.addQueryItem(QStringLiteral("code"), QUrl::fromPercentEncoding(code.toLatin1()));
	query.addQueryItem(QStringLiteral("redirect_uri"), callback());
	query.addQueryItem(QStringLiteral("client_id"), clientIdentifier());

	if (!clientIdentifierSharedKey().isEmpty())
		query.addQueryItem("client_secret", clientIdentifierSharedKey());

	request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

	LOG_CTRACE("oauth2") << "Post request token" << request.url() << query.query();

	QNetworkReply *reply = m_authenticator->networkManager()->post(request, query.query(QUrl::FullyEncoded).toUtf8());

	LOG_CTRACE("oauth2") << "Posted request token" << reply;

	QObject::connect(reply, &QNetworkReply::finished, this, &OAuth2CodeFlow::onRequestAccessFinished);

}



/**
 * @brief OAuth2CodeFlow::onRequestAccessFinished
 */

void OAuth2CodeFlow::onRequestAccessFinished()
{
	LOG_CTRACE("oauth2") << "Request finished";

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if (!reply) {
		LOG_CERROR("oauth2") << "Invalid QNetworkReply";
		emit authenticationFailed();
		return;
	}

	if (reply->error() != QNetworkReply::NoError) {
		LOG_CERROR("oauth2") << "NetworkReply error:" << qPrintable(reply->errorString());
		emit authenticationFailed();
		reply->deleteLater();
		return;
	}
	if (reply->header(QNetworkRequest::ContentTypeHeader).isNull()) {
		LOG_CERROR("oauth2") << "Empty Content-type header";
		emit authenticationFailed();
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
		emit authenticationFailed();
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
			emit authenticationFailed();
			return;
		}
		const QJsonObject object = document.object();
		if (object.isEmpty()) {
			LOG_CERROR("oauth2") << "Received empty JSON object:" << qPrintable(QString::fromUtf8(data));
		}
		ret = object.toVariantMap();
	} else {
		LOG_CERROR("oauth2") << "Unknown content-type:" << qPrintable(contentType);
		emit authenticationFailed();
		return;
	}

	LOG_CTRACE("oauth2") << "Received data:" << ret;

	emit authenticationSuccess(ret);
}


/**
 * @brief OAuth2CodeFlow::onClientDestroyed
 */

void OAuth2CodeFlow::onReferenceObjectDestroyed()
{
	if (m_authenticator)
		m_authenticator->removeCodeFlow(this);
}



/**
 * @brief OAuth2CodeFlow::referenceObject
 * @return
 */

QObject *OAuth2CodeFlow::referenceObject() const
{
	return m_referenceObject;
}
