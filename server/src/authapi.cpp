/*
 * ---- Call of Suli ----
 *
 * authapi.cpp
 *
 * Created on: 2023. 03. 14.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AuthAPI
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

#include "authapi.h"
#include "serverservice.h"



/**
 * @brief AuthAPI::AuthAPI
 * @param service
 */

AuthAPI::AuthAPI(ServerService *service)
	: AbstractAPI(service)
{
	addMap("^login/*$", this, &AuthAPI::login);
	addMap("^login/(\\w+)/*$", this, &AuthAPI::loginOAuth2);
}



/**
 * @brief AuthAPI::login
 * @param data
 * @param response
 */

void AuthAPI::login(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	LOG_CTRACE("client") << "Login";

	const QString &token = data.value(QStringLiteral("token")).toString();
	const QString &username = data.value(QStringLiteral("username")).toString();
	const QString &password = data.value(QStringLiteral("password")).toString();

	if (token.isEmpty() && (username.isEmpty() || password.isEmpty()))
		return responseError(response, "missing username/password");

	if (!token.isEmpty()) {
		if (!Credential::verify(token, m_service->settings()->jwtSecret())) {
			LOG_CDEBUG("client") << "Token verification failed";
			return responseError(response, "invalid token");
		}

		Credential c = Credential::fromJWT(token);

		if (!c.isValid()) {
			LOG_CDEBUG("client") << "Invalid JWT";
			return responseError(response, "invalid token");
		}

		const QString &username = c.username();

		getCredential(username)
				.fail([this, response](Credential){
			responseError(response, "invalid user");
		})
				.done([this, response](Credential c){
			responseAnswer(response, getToken(c));
		});

		return;
	}

	// Without token

	getCredential(username)
			.fail([this, response](Credential){
		responseError(response, "invalid user");
	})
			.then<Credential>([this, username, password](Credential c){
		return authorizePlain(c, password);
	})
			.fail([this, response](Credential){
		responseError(response, "authorization failed");
	})
			.done([this, response](Credential c){
		responseAnswer(response, getToken(c));
	});
}



/**
 * @brief AuthAPI::loginOAuth2
 * @param match
 * @param data
 * @param response
 */

void AuthAPI::loginOAuth2(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const QString &provider = match.captured(1);

	OAuth2Authenticator *authenticator = m_service->oauth2Authenticator(provider.toUtf8());

	if (!authenticator)
		return responseError(response, "invalid provider");

	if (data.contains(QStringLiteral("state"))) {
		const QString &state = data.value(QStringLiteral("state")).toString();
		OAuth2CodeFlow *flow = authenticator->getCodeFlowForState(state);

		if (!flow)
			return responseError(response, "invalid state");

		switch (flow->authState()) {
		case OAuth2CodeFlow::Invalid:
			return responseError(response, "invalid state");
			break;
		case OAuth2CodeFlow::Failed:
			return responseError(response, "authentication failed");
			break;
		case OAuth2CodeFlow::TokenReceived:
			return responseAnswer(response, {
									  { "pending", true },
									  { "tokenReceived", true }
								  });
			break;
		case OAuth2CodeFlow::Authenticated:
			if (flow->credential().isValid())
				return responseAnswer(response, getToken(flow->credential()));
			else
				return responseAnswer(response, {
										  { "pending", true },
										  { "tokenReceived", true }
									  });
			break;
		case OAuth2CodeFlow::Pending:
			return responseAnswer(response, {
									  { "pending", true }
								  });
			break;
		}
	}

	OAuth2CodeFlow *flow = authenticator->addCodeFlow();

	QObject::connect(flow, &OAuth2CodeFlow::authenticated, flow, [this, authenticator, flow](){
		QString email = authenticator->updateUserInfo(flow);

		if (!email.isEmpty()) {
			QPointer<OAuth2CodeFlow> fp(flow);
			getCredential(email).fail([fp](Credential){
				if (fp)
					fp->setAuthState(OAuth2CodeFlow::Failed);
			})
			.done([fp](Credential c){
				if (fp)
					fp->setCredential(c);
			});
		}
	});

	responseAnswer(response, {
					   { "state", flow->state() },
					   { "url", flow->requestAuthorizationUrl().toString() }
				   });
}






/**
 * @brief AuthAPI::getCredential
 * @param username
 * @return
 */

QDeferred<Credential> AuthAPI::getCredential(const QString &username) const
{
	LOG_CTRACE("client") << "Get credential for" << qPrintable(username);

	QDeferred<Credential> ret;

	databaseMainWorker()->execInThread([ret, username, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT active, isAdmin, isTeacher, isPanel FROM user WHERE username=")
				.addValue(username);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(username);
			return ret.reject(Credential());
		}

		if (!q.value("active").toBool()) {
			LOG_CDEBUG("client") << "Inactive user:" << qPrintable(username);
			return ret.reject(Credential());
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
 * @brief AuthAPI::authorizePlain
 * @param credential
 * @param password
 * @return
 */


QDeferred<Credential> AuthAPI::authorizePlain(const Credential &credential, const QString &password) const
{
	LOG_CTRACE("client") << "Authorize plain" << qPrintable(credential.username());

	QDeferred<Credential> ret;

	databaseMainWorker()->execInThread([ret, credential, password, this]() mutable {
		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Invalid credential";
			return ret.reject(credential);
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT salt, password, oauth FROM auth WHERE username=")
				.addValue(credential.username());

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(credential.username());
			return ret.reject(credential);
		}

		if (!q.value("oauth").isNull()) {
			LOG_CDEBUG("client") << "OAuth2 user:" << qPrintable(credential.username());
			return ret.reject(credential);
		}

		const QString &storedPassword = q.value("password").toString();

		if (storedPassword.isEmpty()) {
			LOG_CDEBUG("client") << "Empty password stored for user:" << qPrintable(credential.username());
			return ret.reject(credential);
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

QDeferred<Credential> AuthAPI::authorizeOAuth2(const Credential &credential, const char *oauthType) const
{
	LOG_CTRACE("client") << "Authorize OAuth2" << oauthType << qPrintable(credential.username());

	QDeferred<Credential> ret;

	databaseMain()->worker()->execInThread([ret, credential, oauthType, this]() mutable {
		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Invalid credential";
			return ret.reject(credential);
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT oauth FROM auth WHERE username=")
				.addValue(credential.username());

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(credential.username());
			return ret.reject(credential);
		}

		if (q.value("oauth").toString() != QString::fromUtf8(oauthType)) {
			LOG_CDEBUG("client") << "Plain (non oauth2) user:" << qPrintable(credential.username());
			return ret.reject(credential);
		}

		ret.resolve(credential);
	});

	return ret;
}



/**
 * @brief AuthAPI::getToken
 * @param credential
 * @return
 */

QJsonObject AuthAPI::getToken(const Credential &credential) const
{
	LOG_CINFO("client") << "Create token:" << qPrintable(credential.username()) << credential.roles();

	return QJsonObject({
						   { QStringLiteral("status"), QStringLiteral("ok") },
						   { QStringLiteral("auth_token"), credential.createJWT(m_service->settings()->jwtSecret()) }
					   });
}
