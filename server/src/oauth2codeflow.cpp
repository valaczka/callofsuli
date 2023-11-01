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
#include "serverservice.h"

OAuth2CodeFlow::OAuth2CodeFlow(OAuth2Authenticator *authenticator)
	: QOAuth2AuthorizationCodeFlow{authenticator}
	, m_authenticator(authenticator)
{
	Q_ASSERT(m_authenticator);

	LOG_CTRACE("oauth2") << "OAuth2CodeFlow created" << state() << this;

	setModifyParametersFunction([](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
		if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
			parameters->insert(QStringLiteral("access_type"), QStringLiteral("offline"));
			parameters->insert(QStringLiteral("prompt"), QStringLiteral("consent"));
		}
	});

	connect(&m_removeTimer, &QTimer::timeout, this, &OAuth2CodeFlow::onRemoveTimerTimeout);
	m_removeTimer.start(15*60*1000);
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
	m_authState = Pending;
	return buildAuthenticateUrl();
}





/**
 * @brief OAuth2CodeFlow::requestAccesToken
 * @param code
 */

void OAuth2CodeFlow::requestAccesToken(const QString &code)
{
	if (m_authState != Invalid && m_authState != Pending) {
		LOG_CTRACE("oauth2") << "Flow already finished";
		return;
	}

	LOG_CTRACE("oauth2") << "Start request token";

	m_authState = Pending;

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

	QNetworkReply *reply = m_authenticator->service()->networkManager()->post(std::move(request), query.query(QUrl::FullyEncoded).toUtf8());

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
		m_authState = Failed;
		return;
	}

	if (reply->error() != QNetworkReply::NoError) {
		LOG_CERROR("oauth2") << "NetworkReply error:" << reply->error() << qPrintable(reply->errorString());
		m_authState = Failed;
		return reply->deleteLater();
	}

	if (reply->header(QNetworkRequest::ContentTypeHeader).isNull()) {
		LOG_CERROR("oauth2") << "Empty Content-type header";
		m_authState = Failed;
		return reply->deleteLater();
	}

	const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).isNull() ?
				QStringLiteral("text/html") :
				reply->header(QNetworkRequest::ContentTypeHeader).toString();

	const QByteArray data = reply->readAll();

	reply->deleteLater();

	if (data.isEmpty()) {
		LOG_CERROR("oauth2") << "No received data";
		m_authState = Failed;
		return;
	}

	QJsonObject object;

	if (contentType.startsWith(QStringLiteral("text/html")) ||
			contentType.startsWith(QStringLiteral("application/x-www-form-urlencoded"))) {
		QUrlQuery query(QString::fromUtf8(data));
		auto queryItems = query.queryItems(QUrl::FullyDecoded);

		for (auto it = queryItems.begin(), end = queryItems.end(); it != end; ++it)
			object.insert(it->first, it->second);
	} else if (contentType.startsWith(QStringLiteral("application/json"))
			   || contentType.startsWith(QStringLiteral("text/javascript"))) {
		const QJsonDocument &document = QJsonDocument::fromJson(data);
		if (!document.isObject()) {
			LOG_CERROR("oauth2") << "Received data is not a JSON object:" << qPrintable(QString::fromUtf8(data));
			m_authState = Failed;
			return;
		}
		object = document.object();
		if (object.isEmpty()) {
			LOG_CERROR("oauth2") << "Received empty JSON object:" << qPrintable(QString::fromUtf8(data));
			m_authState = Failed;
			return;
		}
	} else {
		LOG_CERROR("oauth2") << "Unknown content-type:" << qPrintable(contentType);
		m_authState = Failed;
		return;
	}



	const QString &accessToken = object.value(QStringLiteral("access_token")).toString();
	if (accessToken.isEmpty()) {
		LOG_CERROR("oauth2") << "Access token not received";
		m_authState = Failed;
		return;
	}


	// Check

	m_token.token = accessToken;
	m_token.refreshToken = object.value(QStringLiteral("refresh_token")).toString();
	m_token.idToken = object.value(QStringLiteral("id_token")).toString();

	const int &expiresIn = object.value(QStringLiteral("expires_in")).toInt(-1);
	if (expiresIn > 0)
		m_token.expiration = QDateTime::currentDateTime().addSecs(expiresIn);
	else
		m_token.expiration = QDateTime();

	m_authState = TokenReceived;

	if (m_token.idToken.isEmpty()) {
		LOG_CTRACE("oauth2") << "Token received";
	} else {
		LOG_CTRACE("oauth2") << "Authenticated";
		emit authenticated();
	}

}



/**
 * @brief OAuth2CodeFlow::onRemoveTimerTimeout
 */

void OAuth2CodeFlow::onRemoveTimerTimeout()
{
	LOG_CTRACE("oauth2") << "Auto remove code flow:" << state() << this;

	if (m_authenticator)
		m_authenticator->removeCodeFlow(this);
}




OAuth2Authenticator *OAuth2CodeFlow::authenticator() const
{
	return m_authenticator;
}


/**
 * @brief OAuth2CodeFlow::getUserInfo
 * @return
 */

AdminAPI::User OAuth2CodeFlow::getUserInfo() const
{
	Q_ASSERT(m_authenticator);

	LOG_CTRACE("oauth2") << "Get user info" << this;

	const QJsonObject &obj = getJWT();

	if (obj.isEmpty()) {
		LOG_CTRACE("oauth2") << "Invalid id_token";
		return AdminAPI::User();
	}

	AdminAPI::User user = m_authenticator->getUserInfoFromIdToken(obj);

	if (user.username.isEmpty()) {
		LOG_CDEBUG("oauth2") << "Invalid id_token";
		return AdminAPI::User();
	}

	return user;
}



QJsonObject OAuth2CodeFlow::getJWT() const
{
	QStringList listJwtParts = m_token.idToken.split(".");
	if (listJwtParts.count() != 3) {
		LOG_CWARNING("oauth2") << "id_token format error";
		return QJsonObject();
	}


	QJsonParseError error, errorP;
	QJsonDocument::fromJson(QByteArray::fromBase64(listJwtParts.at(0).toUtf8(),QByteArray::Base64UrlEncoding), &error);
	QJsonDocument tmpPayload = QJsonDocument::fromJson(QByteArray::fromBase64(listJwtParts.at(1).toUtf8(),QByteArray::Base64UrlEncoding), &errorP);

	if (error.error != QJsonParseError::NoError) {
		LOG_CWARNING("oauth2") << "id_token header error:" << error.errorString();
		return QJsonObject();
	}

	if (errorP.error != QJsonParseError::NoError) {
		LOG_CWARNING("oauth2") << "id_token payload error:" << errorP.errorString();
		return QJsonObject();
	}

	return tmpPayload.object();
}


/**
 * @brief OAuth2CodeFlow::credential
 * @return
 */

const Credential &OAuth2CodeFlow::credential() const
{
	return m_credential;
}

void OAuth2CodeFlow::setCredential(const Credential &newCredential)
{
	m_credential = newCredential;
}

void OAuth2CodeFlow::setAuthState(AuthState newAuthState)
{
	m_authState = newAuthState;
}


/**
 * @brief OAuth2CodeFlow::token
 * @return
 */

const OAuth2CodeFlow::Token &OAuth2CodeFlow::token() const
{
	return m_token;
}


/**
 * @brief OAuth2CodeFlow::authState
 * @return
 */

OAuth2CodeFlow::AuthState OAuth2CodeFlow::authState() const
{
	return m_authState;
}





/**
 * @brief OAuth2CodeFlow::Token::toJson
 * @return
 */

QJsonObject OAuth2CodeFlow::Token::toJson() const
{
	QJsonObject o;

	o[QStringLiteral("token")] = token;
	o[QStringLiteral("refresh")] = refreshToken;
	o[QStringLiteral("exp")] = expiration.toSecsSinceEpoch();

	return o;
}
