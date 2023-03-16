/*
 * ---- Call of Suli ----
 *
 * authhandler.cpp
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

#include "adminhandler.h"
#include "authhandler.h"
#include "client.h"
#include "oauth2codeflow.h"
#include "serverservice.h"

AuthHandler::AuthHandler(Client *client)
	: AbstractHandler(client)
{

}




/**
 * @brief AuthHandler::getClassIdFromCode
 * @param code
 * @return
 */

QDeferred<bool, int> AuthHandler::getClassIdFromCode(const QString &code) const
{
	HANDLER_LOG_TRACE() << "Get class id from code:" << qPrintable(code);

	QDeferred<bool, int> ret;

	databaseMain()->worker()->execInThread([ret, code, this]() mutable {
		if (code.isEmpty()) {
			HANDLER_LOG_WARNING() << "Empty code";
			ret.resolve(false, -1);
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT code, classid FROM classCode WHERE code=")
				.addValue(code);

		if (!q.exec() || !q.sqlQuery().first()) {
			HANDLER_LOG_TRACE() << "Class code doesn't exists:" << qPrintable(code);
			ret.resolve(false, -1);
			return;
		}

		ret.resolve(true, q.value("classid", -1).toInt());
	});

	return ret;
}



/**
 * @brief AuthHandler::isRegistrationEnabled
 * @return
 */

bool AuthHandler::isRegistrationEnabled() const
{
	return service()->config().get("registrationEnabled").toBool(false);
}


/**
 * @brief AuthHandler::setRegistrationEnabled
 * @param on
 * @return
 */

void AuthHandler::setRegistrationEnabled(const bool &on)
{
	service()->config().set("registrationEnabled", on);
}


/**
 * @brief AuthHandler::isOAuth2RegistrationForced
 * @return
 */

bool AuthHandler::isOAuth2RegistrationForced() const
{
	return service()->config().get("oauth2RegistrationForced").toBool(false);
}


/**
 * @brief AuthHandler::setOAuth2RegistrationForced
 * @param on
 */

void AuthHandler::setOAuth2RegistrationForced(const bool &on)
{
	service()->config().set("oauth2RegistrationForced", on);
}







void AuthHandler::handleRequestResponse()
{

}

void AuthHandler::handleEvent()
{

}




/**
 * @brief AuthHandler::loginToken
 */

void AuthHandler::loginToken()
{
	HANDLER_LOG_TRACE() << "Login token";

	if (!service()) {
		HANDLER_LOG_ERROR() << "Missing service";
		send(m_message.createErrorResponse(QStringLiteral("internal error")));
		return;
	}

	const QString &token = json().value(QStringLiteral("token")).toString();

	if (token.isEmpty()) {
		HANDLER_LOG_TRACE() << "Missing token";
		send(m_message.createErrorResponse(QStringLiteral("authentication required")));
		return;
	}

	if (!Credential::verify(token, service()->settings()->jwtSecret())) {
		LOG_CDEBUG("client") << "Token verification failed";
		send(m_message.createErrorResponse(QStringLiteral("invalid token")));
		return;
	}

	Credential c = Credential::fromJWT(token);

	if (!c.isValid()) {
		LOG_CDEBUG("client") << "Invalid token";
		send(m_message.createErrorResponse(QStringLiteral("invalid token")));
		return;
	}

	getCredential(c.username(), c.iat())
			.fail([this](Credential){
		send(m_message.createErrorResponse(QStringLiteral("invalid user")));
	})
	.done([c, this](Credential c2){
		if (c2.roles() != c.roles()) {
			send(m_message.createErrorResponse(QStringLiteral("corrupted token")));
			return;
		}
		_loginUser(c2, false);
	});
}










/**
 * @brief AuthHandler::registrationGoogle
 */

void AuthHandler::registrationGoogle()
{
	if (!isRegistrationEnabled()) {
		send(m_message.createErrorResponse(QStringLiteral("registration disabled")));
		HANDLER_LOG_WARNING() << "Registration disabled";
		return;
	}

	GoogleOAuth2Authenticator *authenticator = qobject_cast<GoogleOAuth2Authenticator*>(service()->oauth2Authenticator(OAuth2Authenticator::Google));

	if (!authenticator) {
		send(m_message.createErrorResponse(QStringLiteral("invalid provider")));
		HANDLER_LOG_WARNING() << "Invalid provider" << OAuth2Authenticator::Google;
		return;
	}

	if (m_client->oauth2CodeFlow()) {
		send(m_message.createErrorResponse(QStringLiteral("registration already in progress")));
		HANDLER_LOG_WARNING() << "OAuth2CodeFlow already exists";
		return;
	}

	const QString &code = json().value(QStringLiteral("code")).toString();
	m_internalData.insert(QStringLiteral("registrationClassCode"), code);


	if (json().contains(QStringLiteral("access_token"))) {
		_registrationGoogleWithAccessToken(json().value(QStringLiteral("access_token")).toString(), authenticator);
		return;
	}


	OAuth2CodeFlow *flow = authenticator->addCodeFlow(m_client);
	m_client->setOauth2CodeFlow(flow);

	if (flow) {
		connect(flow, &OAuth2CodeFlow::authenticationFailed, this,
				[this, authenticator, flow]() { _OAuthFailed(authenticator, flow); });
		connect(flow, &OAuth2CodeFlow::authenticationSuccess, this,
				[this, authenticator, flow](const QVariantMap &data) { _registrationGoogleOAuthSuccess(data, authenticator, flow); });

		send(m_message.createResponse(QJsonObject({
													  { QStringLiteral("url"), flow->requestAuthorizationUrl().toString() }
												  })), false); // Don't delete handler!
	} else {
		send(m_message.createErrorResponse(QStringLiteral("unable to create oauth2 code flow")));
	}

}





/**
 * @brief AuthHandler::registrationPlain
 */

void AuthHandler::registrationPlain()
{
	if (!isRegistrationEnabled() || isOAuth2RegistrationForced()) {
		send(m_message.createErrorResponse(QStringLiteral("registration disabled")));
		HANDLER_LOG_WARNING() << "Registration disabled";
		return;
	}

	const QString &username = json().value(QStringLiteral("username")).toString();
	const QString &code = json().value(QStringLiteral("code")).toString();

	HANDLER_LOG_DEBUG() << "Registration:" << qPrintable(username);

	userExists(username)
			.fail([this](bool){
		send(m_message.createErrorResponse(QStringLiteral("invalid user")));
	})
			.then<bool, int>([this, code](bool exists){
		QDeferred<bool, int> ret;
		if (exists) {
			send(m_message.createErrorResponse(QStringLiteral("user already exists")));
			ret.reject(false, -1);
			return ret;
		}
		return getClassIdFromCode(code);
	})
			.done([this, username](bool exists, int classid) {
		if (!exists) {
			send(m_message.createErrorResponse(QStringLiteral("invalid code")));
			return;
		}

		AdminHandler::User user;
		user.username = username;
		user.familyName = json().value(QStringLiteral("familyName")).toString();
		user.givenName = json().value(QStringLiteral("givenName")).toString();
		user.picture = json().value(QStringLiteral("picture")).toString();
		user.active = true;
		user.classid = classid;

		AdminHandler::userAdd(this, user)
				.fail([this]{
			send(m_message.createErrorResponse(QStringLiteral("registration failed")));
		})
				.then([username, this]{
			return AdminHandler::authAddPlain(this, username, json().value(QStringLiteral("password")).toString());
		})
				.fail([this]{
			send(m_message.createErrorResponse(QStringLiteral("registration failed")));
		})
				.done([user, this]{
			_loginUser(user.toCredential());
		});
	});
}











/**
 * @brief AuthHandler::registrationWithIdToken
 * @param data
 */

void AuthHandler::_registrationWithAccessToken(const QVariantMap &data, const char *provider)
{
	if (data.value(QStringLiteral("email_verified")).toString() == QLatin1String("true") ||
			data.value(QStringLiteral("verified_email")).toString() == QLatin1String("true")) {
		const QString &email = data.value(QStringLiteral("email")).toString();
		const QString &code = m_internalData.value(QStringLiteral("registrationClassCode")).toString();

		HANDLER_LOG_DEBUG() << "Registration with" << provider << "user:" << qPrintable(email);


		userExists(email)
				.fail([this](bool){
			send(m_message.createErrorResponse(QStringLiteral("invalid user")));
		})
				.then<bool, int>([this, code](bool exists){
			QDeferred<bool, int> ret;
			if (exists) {
				send(m_message.createErrorResponse(QStringLiteral("user already exists")));
				ret.reject(false, -1);
				return ret;
			}
			return getClassIdFromCode(code);
		})
				.done([this, email, data, provider](bool exists, int classid) {
			if (!exists) {
				send(m_message.createErrorResponse(QStringLiteral("invalid code")));
				return;
			}

			AdminHandler::User user;
			user.username = email;
			user.familyName = data.value(QStringLiteral("family_name")).toString();
			user.givenName = data.value(QStringLiteral("given_name")).toString();
			user.picture = data.value(QStringLiteral("picture")).toString();
			user.active = true;
			user.classid = classid;

			AdminHandler::userAdd(this, user)
					.fail([this]{
				send(m_message.createErrorResponse(QStringLiteral("registration failed")));
			})
					.then([this, email, provider](){
				return AdminHandler::authAddOAuth2(this, email, QString::fromLatin1(provider));
			})
					.fail([this]{
				send(m_message.createErrorResponse(QStringLiteral("registration failed")));
			})
					.done([user, this]{
				_loginUser(user.toCredential());
			});

		});

	} else {
		send(WebSocketMessage::createErrorEvent(QStringLiteral("Authenticated email not verified"), WebSocketMessage::ClassAuth));
	}
}







/**
 * @brief AuthHandler::_loginGoogleWithAccessToken
 * @param accessToken
 */

void AuthHandler::_loginGoogleWithAccessToken(const QString &accessToken, GoogleOAuth2Authenticator *authenticator)
{
	GoogleOAuth2AccessCodeFlow *flow = new GoogleOAuth2AccessCodeFlow(authenticator, this);

	authenticator->addCodeFlow(flow);
	m_client->setOauth2CodeFlow(flow);

	connect(flow, &GoogleOAuth2AccessCodeFlow::getUserInfoFailed, this,
			[this, authenticator, flow]() { _OAuthFailed(authenticator, flow); });
	connect(flow, &GoogleOAuth2AccessCodeFlow::getUserInfoSuccess, this,
			[this, authenticator, flow](const QVariantMap &data) {
		_OAuthSuccess(authenticator, flow);
		_loginWithAccessToken(data, "google");
	});

	flow->getUserInfoWithAccessToken(accessToken);

	send(m_message.createStatusResponse(QStringLiteral("pending")), false);
}



/**
 * @brief AuthHandler::_registrationGoogleOAuthSuccess
 * @param data
 * @param authenticator
 * @param flow
 */

void AuthHandler::_registrationGoogleOAuthSuccess(const QVariantMap &data, GoogleOAuth2Authenticator *authenticator, OAuth2CodeFlow *flow)
{
	_OAuthSuccess(authenticator, flow);

	if (!data.contains(QStringLiteral("access_token"))) {
		send(m_message.createErrorResponse(QStringLiteral("missing access token")));
		HANDLER_LOG_WARNING() << "Missing access token";
		return;
	}

	_registrationGoogleWithAccessToken(data.value(QStringLiteral("access_token")).toString(), authenticator);
}



/**
 * @brief AuthHandler::_registrationGoogleWithAccessToken
 * @param accessToken
 * @param authenticator
 */

void AuthHandler::_registrationGoogleWithAccessToken(const QString &accessToken, GoogleOAuth2Authenticator *authenticator)
{
	GoogleOAuth2AccessCodeFlow *flow = new GoogleOAuth2AccessCodeFlow(authenticator, this);

	authenticator->addCodeFlow(flow);
	m_client->setOauth2CodeFlow(flow);

	connect(flow, &GoogleOAuth2AccessCodeFlow::getUserInfoFailed, this,
			[this, authenticator, flow]() { _OAuthFailed(authenticator, flow); });
	connect(flow, &GoogleOAuth2AccessCodeFlow::getUserInfoSuccess, this,
			[this, authenticator, flow](const QVariantMap &data) {
		_OAuthSuccess(authenticator, flow);
		_registrationWithAccessToken(data, "google");
	});

	flow->getUserInfoWithAccessToken(accessToken);

	send(m_message.createStatusResponse(QStringLiteral("pending")), false);
}




/**
 * @brief AuthHandler::_googleOAuthSuccess
 * @param data
 * @param authenticator
 * @param flow
 */

void AuthHandler::_loginGoogleOAuthSuccess(const QVariantMap &data, GoogleOAuth2Authenticator *authenticator, OAuth2CodeFlow *flow)
{
	_OAuthSuccess(authenticator, flow);

	if (!data.contains(QStringLiteral("access_token"))) {
		send(m_message.createErrorResponse(QStringLiteral("missing access token")));
		HANDLER_LOG_WARNING() << "Missing access token";
		return;
	}

	_loginGoogleWithAccessToken(data.value(QStringLiteral("access_token")).toString(), authenticator);
}





