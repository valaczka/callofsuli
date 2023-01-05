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
		Credential::Roles r;
		c.setUsername(username);

		if (q.value("isPanel").toBool()) {
			r.setFlag(Credential::Panel);
		} else {
			r.setFlag(Credential::Student);
			r.setFlag(Credential::Admin, q.value("isAdmin").toBool());
			r.setFlag(Credential::Teacher, q.value("isTeacher").toBool());
		}

		c.setRoles(r);

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
			LOG_CDEBUG("client") << "Non-oauth2 user:" << qPrintable(credential.username());
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
		send(m_message.createStatusResponse());

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
	LOG_CTRACE("client") << "Login plain";

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

void AuthHandler::testToken()
{
	LOG_CTRACE("client") << "Test token";

	if (!validateJwtToken())
		return;

	LOG_CINFO("client") << "Validated" << credential().username() << credential().roles();

	send(m_message.createStatusResponse());
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

	QString email = QString::fromStdString(map.value("email"));

	/**
	{
	  "iss": "https://accounts.google.com",
	  "azp": "822493401756-d8ooae3ja5s9g5a1u7ld5b38es1no61f.apps.googleusercontent.com",
	  "aud": "822493401756-d8ooae3ja5s9g5a1u7ld5b38es1no61f.apps.googleusercontent.com",
	  "sub": "117872218068592040950",
	  "email": "valaczka@gmail.com",
	  "email_verified": true,
	  "at_hash": "ZP9S8fNZSxKf1MNF1VhXnA",
	  "name": "János Pál Valaczka",
	  "picture": "https://lh3.googleusercontent.com/a/AEdFTp43_bD1XcEzYyBDId_CikF17qC-vXsYRjh9NV3hrw=s96-c",
	  "given_name": "János Pál",
	  "family_name": "Valaczka",
	  "locale": "hu",
	  "iat": 1672832265,
	  "exp": 1672835865
	}

	*/

	if (QString::fromStdString(map.value("email_verified")) == QLatin1String("true")) {
		LOG_CDEBUG("client") << "Authenticated with Google:" << qPrintable(email);

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
