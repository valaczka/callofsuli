/*
 * ---- Call of Suli ----
 *
 * oauth2authenticator.cpp
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

#include "oauth2authenticator.h"
#include "Logger.h"
#include "oauth2codeflow.h"
#include "serverservice.h"

OAuth2Authenticator::OAuth2Authenticator(const char *type, ServerService *service)
	: QObject{service}
	, m_handler(new OAuth2ReplyHandler(this))
	, m_type(type)
	, m_service(service)
{
	Q_ASSERT(m_service);

	LOG_CTRACE("oauth2") << "OAuth2Authenticator created" << m_type << this;
}


/**
 * @brief OAuth2Authenticator::~OAuth2Authenticator
 */

OAuth2Authenticator::~OAuth2Authenticator()
{
	qDeleteAll(m_codeFlowList);
	delete m_handler;

	LOG_CTRACE("oauth2") << "OAuth2Authenticator destroyed" << m_type << this;
}



/**
 * @brief OAuth2Authenticator::addCodeFlow
 * @return
 */

OAuth2CodeFlow *OAuth2Authenticator::addCodeFlow()
{
	OAuth2CodeFlow *flow = new OAuth2CodeFlow(this);

	flow->setReplyHandler(m_handler);

	setCodeFlow(flow);

	LOG_CDEBUG("oauth2") << "Add new code flow" << flow->state();

	m_codeFlowList.append(flow);


	return flow;
}


/**
 * @brief OAuth2Authenticator::removeCodeFlow
 * @param flow
 */

void OAuth2Authenticator::removeCodeFlow(OAuth2CodeFlow *flow)
{
	if (m_codeFlowList.contains(flow)) {
		LOG_CTRACE("oauth2") << "Remove code flow:" << flow;
		m_codeFlowList.removeAll(flow);
	} else {
		LOG_CDEBUG("oauth2") << "Code flow not found:" << flow;
	}
}





/**
 * @brief OAuth2Authenticator::getCodeFlowForStatus
 * @param status
 * @return
 */

OAuth2CodeFlow *OAuth2Authenticator::getCodeFlowForState(const QString &state) const
{
	foreach (OAuth2CodeFlow *f, m_codeFlowList)
		if (f->state() == state)
			return f;

	return nullptr;
}



/**
 * @brief OAuth2Authenticator::updateUserInfo
 * @param flow
 */

QString OAuth2Authenticator::updateUserInfo(OAuth2CodeFlow *flow) const
{
	Q_ASSERT(flow);

	LOG_CTRACE("oauth2") << "Update user info";


	const QJsonObject &obj = getJWT(flow->token().idToken);

	if (obj.isEmpty()) {
		LOG_CTRACE("oauth2") << "Invalid id_token";
		return "";
	}

	const QString &email = updateUserInfoFromIdToken(obj);

	if (email.isEmpty()) {
		LOG_CDEBUG("oauth2") << "Invalid id_token";
		return "";
	}

	const QJsonObject &tokenData = flow->token().toJson();

	m_service->databaseMain()->worker()->execInThread([email, tokenData, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(m_service->databaseMain()->dbName());

		QMutexLocker(m_service->databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("UPDATE auth SET oauthData=")
				.addValue(QString::fromUtf8(QJsonDocument(tokenData).toJson(QJsonDocument::Compact)))
				.addQuery(" WHERE username=")
				.addValue(email)
				.addQuery(" AND oauth=")
				.addValue(QString::fromUtf8(m_type));

		if (q.exec())
			LOG_CDEBUG("oauth2") << "OAuth tokenData updated:" << email;
	});

	return email;
}



/**
 * @brief OAuth2Authenticator::clientId
 * @return
 */

const QString &OAuth2Authenticator::clientId() const
{
	return m_clientId;
}

void OAuth2Authenticator::setClientId(const QString &newClientId)
{
	if (m_clientId == newClientId)
		return;
	m_clientId = newClientId;
	emit clientIdChanged();
}

const QString &OAuth2Authenticator::clientKey() const
{
	return m_clientKey;
}

void OAuth2Authenticator::setClientKey(const QString &newClientKey)
{
	if (m_clientKey == newClientKey)
		return;
	m_clientKey = newClientKey;
	emit clientKeyChanged();
}


ServerService *OAuth2Authenticator::service() const
{
	return m_service;
}



/**
 * @brief OAuth2Authenticator::updateUserInfoFromIdToken
 * @param data
 */

QString OAuth2Authenticator::updateUserInfoFromIdToken(const QJsonObject &data) const
{
	bool emailVerified = data.value(QStringLiteral("email_verified")).toBool();

	if (!emailVerified) {
		LOG_CDEBUG("oauth2") << "Email not verified";
		return "";
	}

	const QString &email = data.value(QStringLiteral("email")).toString();

	if (email.isEmpty()) {
		LOG_CDEBUG("oauth2") << "Missing email";
		return "";
	}

	m_service->databaseMain()->worker()->execInThread([data, email, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(m_service->databaseMain()->dbName());

		QMutexLocker(m_service->databaseMain()->mutex());

		if (!QueryBuilder::q(db).addQuery("SELECT username FROM user WHERE active=TRUE AND username=")
				.addValue(email).execCheckExists()) {
			LOG_CDEBUG("oauth2") << "User doesn't exists:" << qPrintable(email);
			return;
		}

		QueryBuilder q(db);
		q.addQuery("UPDATE user SET ")
				.setCombinedPlaceholder()
				.addQuery(" WHERE username=")
				.addValue(email);

		if (data.contains(QStringLiteral("family_name")))
			q.addField("familyName", data.value(QStringLiteral("family_name")).toString());

		if (data.contains(QStringLiteral("given_name")))
			q.addField("givenName", data.value(QStringLiteral("given_name")).toString());

		if (data.contains(QStringLiteral("picture")))
			q.addField("picture", data.value(QStringLiteral("picture")).toString());

		if (!q.fieldCount()) {
			LOG_CDEBUG("oauth2") << "No filed to update";
			return;
		}

		if (q.exec())
			LOG_CDEBUG("oauth2") << "User info updated:" << email;
	});

	return email;
}



/**
 * @brief OAuth2Authenticator::getJWT
 * @param idToken
 * @return
 */

QJsonObject OAuth2Authenticator::getJWT(const QString &idToken) const
{
	QStringList listJwtParts = idToken.split(".");
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
 * @brief OAuth2Authenticator::type
 * @return
 */

const char *OAuth2Authenticator::type() const
{
	return m_type;
}


const QString &OAuth2Authenticator::path() const
{
	return m_path;
}

void OAuth2Authenticator::setPath(const QString &newPath)
{
	if (m_path == newPath)
		return;
	m_path = newPath;
	emit pathChanged();
}





/**
 * @brief OAuth2ReplyHandler::OAuth2ReplyHandler
 * @param service
 */

OAuth2ReplyHandler::OAuth2ReplyHandler(OAuth2Authenticator *authenticator)
	: QAbstractOAuthReplyHandler(authenticator)
	, m_authenticator(authenticator)
{
	Q_ASSERT(m_authenticator);

	LOG_CTRACE("oauth2") << "Reply handler created" << this;
}


/**
 * @brief OAuth2ReplyHandler::~OAuth2ReplyHandler
 */

OAuth2ReplyHandler::~OAuth2ReplyHandler()
{
	LOG_CTRACE("oauth2") << "Reply handler destroyed" << this;
}



/**
 * @brief OAuth2ReplyHandler::callback
 * @return
 */

QString OAuth2ReplyHandler::callback() const
{
	Q_ASSERT(m_authenticator);

	const QUrl url(QString::fromLatin1("%1://%2:%3/cb/%4")
				   .arg(m_authenticator->service()->settings()->ssl() ? QStringLiteral("https") : QStringLiteral("http"))
				   .arg(m_authenticator->service()->webServer()->redirectHost())
				   .arg(m_authenticator->service()->settings()->listenPort())
				   .arg(m_authenticator->path())
				   );
	return url.toString(QUrl::EncodeDelimiters);
}


