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
 * @brief AuthHandler::getCredential
 * @param username
 * @return
 */

QDeferred<Credential> AuthHandler::getCredential(const QString &username) const
{
	LOG_CTRACE("client") << "Get credential for" << qPrintable(username);

	QDeferred<Credential> ret;

	databaseMain()->worker()->execInThread([ret, username, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT active, isAdmin, isTeacher, isPanel FROM user WHERE username=")
				.addValue(username);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(username);
			ret.reject(Credential());
			return;
		}

		if (!q.value("active").toBool()) {
			LOG_CDEBUG("client") << "Inactive user:" << qPrintable(username);
			ret.reject(Credential());
			return;
		}


		Credential c;
		c.setUsername(username);

		if (q.value("isPanel").toBool()) {
			c.setRole(Credential::Panel);
		} else {
			c.setRole(Credential::Student);
			c.setRole(Credential::Admin, q.value("isAdmin").toBool());
			c.setRole(Credential::Teacher, q.value("isTeacher").toBool());
		}

		ret.resolve(c);
	});

	return ret;
}



/**
 * @brief AuthHandler::authorizePlain
 * @param username
 * @param passoword
 * @return
 */

QDeferred<Credential> AuthHandler::authorizePlain(const Credential &credential, const QString &password) const
{
	LOG_CTRACE("client") << "Authorize plain" << qPrintable(credential.username());

	QDeferred<Credential> ret;

	databaseMain()->worker()->execInThread([ret, credential, password, this]() mutable {
		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Invalid credential";
			ret.reject(credential);
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT salt, password, oauth FROM auth WHERE username=")
				.addValue(credential.username());

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(credential.username());
			ret.reject(credential);
			return;
		}

		if (!q.value("oauth").isNull()) {
			LOG_CDEBUG("client") << "OAuth2 user:" << qPrintable(credential.username());
			ret.reject(credential);
			return;
		}

		const QString &storedPassword = q.value("password").toString();

		if (storedPassword.isEmpty()) {
			LOG_CDEBUG("client") << "Empty password stored for user:" << qPrintable(credential.username());
			ret.reject(credential);
			return;
		}

		const QString &hashedPassword = Credential::hashString(password, q.value("salt").toString());

		if (QString::compare(storedPassword, hashedPassword, Qt::CaseInsensitive) == 0)
			ret.resolve(credential);
		else {
			LOG_CDEBUG("client") << "Invalid password for user:" << qPrintable(credential.username());
			ret.reject(credential);
		}
	});

	return ret;
}


/**
 * @brief AuthHandler::authorizeOAuth2
 * @param username
 * @param oauthType
 * @return
 */

QDeferred<Credential> AuthHandler::authorizeOAuth2(const Credential &credential, const char *oauthType) const
{
	LOG_CTRACE("client") << "Authorize OAuth2" << oauthType << qPrintable(credential.username());

	QDeferred<Credential> ret;

	databaseMain()->worker()->execInThread([ret, credential, oauthType, this]() mutable {
		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Invalid credential";
			ret.reject(credential);
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT oauth FROM auth WHERE username=")
				.addValue(credential.username());

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(credential.username());
			ret.reject(credential);
			return;
		}

		if (q.value("oauth").toString() != QString::fromUtf8(oauthType)) {
			LOG_CDEBUG("client") << "Non-oauth2 user:" << oauthType << qPrintable(credential.username());
			ret.reject(credential);
			return;
		}

		ret.resolve(credential);
	});

	return ret;
}


/**
 * @brief AuthHandler::userExists
 * @param username
 * @return
 */

QDeferred<bool> AuthHandler::userExists(const QString &username) const
{
	LOG_CTRACE("client") << "Check user exists:" << qPrintable(username);

	QDeferred<bool> ret;

	databaseMain()->worker()->execInThread([ret, username, this]() mutable {
		if (username.isEmpty()) {
			LOG_CWARNING("client") << "Empty username";
			ret.reject(false);
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT username FROM user WHERE username=")
				.addValue(username);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CTRACE("client") << "User doesn't exists:" << qPrintable(username);
			ret.resolve(false);
			return;
		}

		LOG_CTRACE("client") << "User exists:" << qPrintable(username);
		ret.resolve(true);
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
 * @brief AuthHandler::loginGoogle
 */

void AuthHandler::loginGoogle()
{
	if (m_client->oauth2CodeFlow()) {
		send(m_message.createErrorResponse(QStringLiteral("login already in progress")));
		LOG_CWARNING("client") << m_client << "OAuth2CodeFlow already exists";
		return;
	}

	OAuth2CodeFlow *flow = service()->googleOAuth2Authenticator()->addCodeFlow(m_client);
	m_client->setOauth2CodeFlow(flow);

	if (flow) {
		send(m_message.createStatusResponse(), false);				// Don't delete handler!

		connect(flow, &OAuth2CodeFlow::authenticationFailed, this, &AuthHandler::onOAuthFailed);
		connect(flow, &OAuth2CodeFlow::authenticationSuccess, this, &AuthHandler::onOAuthSuccess);

		flow->startAuthenticate();
	} else {
		send(m_message.createErrorResponse(QStringLiteral("unable to create oauth2 code flow")));
	}

}




/**
 * @brief AuthHandler::loginPlain
 */

void AuthHandler::loginPlain()
{
	LOG_CTRACE("client") << m_client << "Login plain";

	const QString &username = json().value(QStringLiteral("username")).toString();
	const QString &password = json().value(QStringLiteral("password")).toString();

	if (username.isEmpty() || password.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username and/or password")));
		return;
	}

	getCredential(username)
			.fail([this](Credential){
		send(m_message.createErrorResponse(QStringLiteral("invalid user")));
	})
	.then<Credential>([this, username, password](Credential c){
		return authorizePlain(c, password);
	})
			.fail([this](Credential){
		send(m_message.createErrorResponse(QStringLiteral("authentication failed")));
	})
	.done([this](Credential c){
		loginUser(c);
	});
}






/**
 * @brief AuthHandler::registrationGoogle
 */

void AuthHandler::registrationGoogle()
{
	if (!isRegistrationEnabled()) {
		send(m_message.createErrorResponse(QStringLiteral("registration disabled")));
		LOG_CWARNING("client") << m_client << "Registration disabled";
		return;
	}

	if (m_client->oauth2CodeFlow()) {
		send(m_message.createErrorResponse(QStringLiteral("registration already in progress")));
		LOG_CWARNING("client") << m_client << "OAuth2CodeFlow already exists";
		return;
	}

	OAuth2CodeFlow *flow = service()->googleOAuth2Authenticator()->addCodeFlow(m_client);
	m_client->setOauth2CodeFlow(flow);

	if (flow) {
		send(m_message.createStatusResponse(), false);			// Don't delete handler!

		connect(flow, &OAuth2CodeFlow::authenticationFailed, this, &AuthHandler::onOAuthFailed);
		connect(flow, &OAuth2CodeFlow::authenticationSuccess, this, &AuthHandler::onOAuthRegistrationSuccess);

		flow->startAuthenticate();
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
		LOG_CWARNING("client") << m_client << "Registration disabled";
		return;
	}

	const QString &username = json().value(QStringLiteral("username")).toString();

	LOG_CDEBUG("client") << m_client << "Registration:" << qPrintable(username);

	userExists(username)
			.fail([this](bool){
		send(m_message.createErrorResponse(QStringLiteral("invalid user")));
	})
			.done([this, username](bool exists){
		if (exists) {
			send(m_message.createErrorResponse(QStringLiteral("user already exists")));
		} else {
			AdminHandler::User user;
			user.username = username;
			user.familyName = json().value(QStringLiteral("familyName")).toString();
			user.givenName = json().value(QStringLiteral("givenName")).toString();
			user.picture = json().value(QStringLiteral("picture")).toString();
			user.active = true;

			AdminHandler::userAdd(this, user)
					.done([user, this]{
				loginUser(user.toCredential());
			})
					.fail([this]{
				send(m_message.createErrorResponse(QStringLiteral("registration failed")));
			});

		}
	});
}






/**
 * @brief AuthHandler::onOAuthFailed
 */

void AuthHandler::onOAuthFailed()
{
	send(WebSocketMessage::createErrorEvent(QStringLiteral("authentication failed"), WebSocketMessage::ClassAuth));
	service()->googleOAuth2Authenticator()->removeCodeFlow(qobject_cast<OAuth2CodeFlow*>(sender()));
	if (m_client)
		m_client->setOauth2CodeFlow(nullptr);
}


/**
 * @brief AuthHandler::onOAuthSucceed
 * @param data
 */

void AuthHandler::onOAuthSuccess(const QVariantMap &data)
{
	service()->googleOAuth2Authenticator()->removeCodeFlow(qobject_cast<OAuth2CodeFlow*>(sender()));

	if (m_client)
		m_client->setOauth2CodeFlow(nullptr);

	auto map = GoogleOAuth2Authenticator::getInfoFromRequestAccess(data);

	if (QString::fromStdString(map.value("email_verified")) == QLatin1String("true")) {
		QString email = QString::fromStdString(map.value("email"));

		LOG_CDEBUG("client") << m_client << "Authenticated with Google:" << qPrintable(email);

		getCredential(email)
				.fail([this](Credential){
			send(m_message.createErrorResponse(QStringLiteral("invalid user")));
		})
		.then<Credential>([this](Credential c){
			return authorizeOAuth2(c, "google");
		})
		.fail([this](Credential){
			send(m_message.createErrorResponse(QStringLiteral("authentication failed")));
		})
		.done([this](Credential c){
			loginUser(c);
		});
	} else {
		send(WebSocketMessage::createErrorEvent(QStringLiteral("Authenticated email not verified"), WebSocketMessage::ClassAuth));
	}

}



/**
 * @brief AuthHandler::onOAuthRegistrationSuccess
 * @param data
 */


void AuthHandler::onOAuthRegistrationSuccess(const QVariantMap &data)
{
	LOG_CTRACE("client") << m_client << "Registration success";

	service()->googleOAuth2Authenticator()->removeCodeFlow(qobject_cast<OAuth2CodeFlow*>(sender()));

	if (m_client)
		m_client->setOauth2CodeFlow(nullptr);


	auto map = GoogleOAuth2Authenticator::getInfoFromRequestAccess(data);


	if (QString::fromStdString(map.value("email_verified")) == QLatin1String("true")) {
		QString email = QString::fromStdString(map.value("email"));

		LOG_CDEBUG("client") << m_client << "Registration with Google:" << qPrintable(email);

		userExists(email)
				.fail([this](bool){
			send(m_message.createErrorResponse(QStringLiteral("invalid user")));
		})
				.done([this, email, map](bool exists){
			if (exists) {
				send(m_message.createErrorResponse(QStringLiteral("user already exists")));
			} else {
				AdminHandler::User user;
				user.username = email;
				user.familyName = QString::fromStdString(map.value("family_name"));
				user.givenName = QString::fromStdString(map.value("given_name"));
				user.picture = QString::fromStdString(map.value("picture"));
				user.active = true;

				AdminHandler::userAdd(this, user)
						.done([user, this]{
					loginUser(user.toCredential());
				})
						.fail([this]{
					send(m_message.createErrorResponse(QStringLiteral("registration failed")));
				});

			}
		});

	} else {
		send(WebSocketMessage::createErrorEvent(QStringLiteral("Authenticated email not verified"), WebSocketMessage::ClassAuth));
	}
}



/**
 * @brief AuthHandler::loginUser
 * @param username
 */

void AuthHandler::loginUser(const Credential &credential)
{
	LOG_CINFO("client") << m_client << "Login:" << qPrintable(credential.username()) << credential.roles();

	setCredential(credential);

	const QString &token = credential.createJWT(service()->settings()->jwtSecret());

	send(m_message.createResponse(QJsonObject({
												  { QStringLiteral("status"), QStringLiteral("ok") },
												  { QStringLiteral("auth_token"), token }
											  })));
}

