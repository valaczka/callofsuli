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
#include "adminapi.h"



/**
 * @brief AuthAPI::AuthAPI
 * @param service
 */

AuthAPI::AuthAPI(ServerService *service)
	: AbstractAPI(service)
{
	addMap("^login/*$", this, &AuthAPI::login);
	addMap("^login/(\\w+)/*$", this, &AuthAPI::loginOAuth2);

	addMap("^registration/*$", this, &AuthAPI::registration);
	addMap("^registration/(\\w+)/*$", this, &AuthAPI::registrationOAuth2);
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
		if (!Credential::verify(token, m_service->settings()->jwtSecret(), m_service->config().get("tokenFirstIat").toInt(0))) {
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
		case OAuth2CodeFlow::UserExists:
		case OAuth2CodeFlow::InvalidDomain:
		case OAuth2CodeFlow::InvalidCode:
			return responseError(response, "authentication failed");
			break;
		case OAuth2CodeFlow::TokenReceived:
			return responseAnswer(response, {
									  { QStringLiteral("pending"), true },
									  { QStringLiteral("tokenReceived"), true }
								  });
			break;
		case OAuth2CodeFlow::Authenticated:
			if (flow->credential().isValid())
				return responseAnswer(response, getToken(flow->credential()));
			else
				return responseAnswer(response, {
										  { QStringLiteral("pending"), true },
										  { QStringLiteral("tokenReceived"), true }
									  });
			break;
		case OAuth2CodeFlow::Pending:
			return responseAnswer(response, {
									  { QStringLiteral("pending"), true }
								  });
			break;
		}
	}

	OAuth2CodeFlow *flow = authenticator->addCodeFlow();


	QObject::connect(flow, &OAuth2CodeFlow::authenticated, flow, [this, flow](){
		QString email = flow->getUserInfo().username;

		if (!email.isEmpty()) {
			QPointer<OAuth2CodeFlow> fp(flow);
			getCredential(email).fail([fp](Credential){
				if (fp) fp->setAuthState(OAuth2CodeFlow::Failed);
			})
			.done([fp, this](Credential c){
				if (fp) {
					fp->setCredential(c);
					fp->setAuthState(OAuth2CodeFlow::Authenticated);
				}
				updateOAuth2TokenInfo(fp);

				if (m_service->config().get("oauth2NameUpdate").toBool())
					updateOAuth2UserData(fp);
			});
		}
	});

	responseAnswer(response, {
					   { QStringLiteral("state"), flow->state() },
					   { QStringLiteral("url"), flow->requestAuthorizationUrl().toString() }
				   });
}



/**
 * @brief AuthAPI::registration
 * @param data
 * @param response
 */

void AuthAPI::registration(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	if (!m_service->config().registrationEnabled() || m_service->config().oAuth2RegistrationForced()) {
		LOG_CWARNING("client") << "Registration disabled";
		return responseError(response, "registration disabled");
	}

	const QString &username = data.value(QStringLiteral("username")).toString();
	const QString &code = data.value(QStringLiteral("code")).toString();

	LOG_CDEBUG("client") << "Registration:" << qPrintable(username);

	AdminAPI::userNotExists(this, username)
			.fail([this, response](){
		responseError(response, "user exists");
	})
			.then<bool, int>([this, code, response](){
		return AdminAPI::getClassIdFromCode(this, code);
	})
			.done([this, username, response, data](bool exists, int classid) {
		if (!exists) {
			return responseError(response, "invalid code");
		}

		AdminAPI::User user;
		user.username = username;
		user.familyName = data.value(QStringLiteral("familyName")).toString();
		user.givenName = data.value(QStringLiteral("givenName")).toString();
		user.picture = data.value(QStringLiteral("picture")).toString();
		user.active = true;
		user.classid = classid;

		AdminAPI::userAdd(this, user)
				.fail([this, response]{
			return responseError(response, "registration failed");
		})
				.then([username, this, data]{
			return AdminAPI::authAddPlain(this, username, data.value(QStringLiteral("password")).toString());
		})
				.fail([this, response]{
			return responseError(response, "registration failed");
		})
				.done([username, response, this]{
			getCredential(username)
					.fail([this, response](Credential){
				responseError(response, "invalid user");
			})
					.done([this, response](Credential c){
				responseAnswer(response, getToken(c));
			});

		});
	});
}



/**
 * @brief AuthAPI::registrationOAuth2
 * @param match
 * @param data
 * @param response
 */

void AuthAPI::registrationOAuth2(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
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
		case OAuth2CodeFlow::UserExists:
			return responseError(response, "user exists");
			break;
		case OAuth2CodeFlow::InvalidCode:
			return responseError(response, "invalid code");
			break;
		case OAuth2CodeFlow::InvalidDomain:
			return responseError(response, "invalid domain");
			break;
		case OAuth2CodeFlow::TokenReceived:
			return responseAnswer(response, {
									  { QStringLiteral("pending"), true },
									  { QStringLiteral("tokenReceived"), true }
								  });
			break;
		case OAuth2CodeFlow::Authenticated:
			if (flow->credential().isValid())
				return responseAnswer(response, getToken(flow->credential()));
			else
				return responseAnswer(response, {
										  { QStringLiteral("pending"), true },
										  { QStringLiteral("tokenReceived"), true }
									  });
			break;
		case OAuth2CodeFlow::Pending:
			return responseAnswer(response, {
									  { QStringLiteral("pending"), true }
								  });
			break;
		}
	}

	OAuth2CodeFlow *flow = authenticator->addCodeFlow();


	QObject::connect(flow, &OAuth2CodeFlow::authenticated, flow, [this, flow, response, data](){
		AdminAPI::User user = flow->getUserInfo();
		QString code = data.value(QStringLiteral("code")).toString();

		if (!user.username.isEmpty()) {
			QPointer<OAuth2CodeFlow> fp(flow);

			const QStringList &array = m_service->config().get("oauth2DomainList").toString().simplified()
					.split(QStringLiteral(","), Qt::SkipEmptyParts);
			bool domainEnabled = array.isEmpty();


			for (const QString &s : array) {
				if (domainEnabled)
					break;

				if (s.isEmpty())
					continue;

				if (user.username.endsWith(s.simplified()))
					domainEnabled = true;
			}

			LOG_CTRACE("client") << "Check domains:" << array << domainEnabled;

			if (!domainEnabled) {
				if (fp) fp->setAuthState(OAuth2CodeFlow::InvalidDomain);
				return;
			}

			AdminAPI::userNotExists(this, user.username)
					.fail([fp](){
				if (fp)
					fp->setAuthState(OAuth2CodeFlow::UserExists);
			})
					.then<bool, int>([this, code, response](){
				return AdminAPI::getClassIdFromCode(this, code);
			})
					.done([this, user, data, fp](bool exists, int classid) mutable {
				if (!exists) {
					if (fp) fp->setAuthState(OAuth2CodeFlow::InvalidCode);
					return;
				}

				user.active = true;
				user.classid = classid;

				AdminAPI::userAdd(this, user)
						.fail([fp]{
					if (fp) fp->setAuthState(OAuth2CodeFlow::Failed);
				})
						.then([user, this, data, fp]{
					return AdminAPI::authAddOAuth2(this, user.username, fp ? QString::fromUtf8(fp->authenticator()->type()) : QLatin1String(""));
				})
						.fail([fp]{
					if (fp) fp->setAuthState(OAuth2CodeFlow::Failed);
				})
						.done([user, fp, this]{
					getCredential(user.username)
							.fail([fp](Credential){
						if (fp) fp->setAuthState(OAuth2CodeFlow::Failed);
					})
					.done([fp, this](Credential c){
						if (fp) {
							fp->setCredential(c);
							fp->setAuthState(OAuth2CodeFlow::Authenticated);
						}
						updateOAuth2TokenInfo(fp);
					});
				});
			});
		}
	});

	responseAnswer(response, {
					   { QStringLiteral("state"), flow->state() },
					   { QStringLiteral("url"), flow->requestAuthorizationUrl().toString() }
				   });
}








/**
 * @brief AuthAPI::getCredential
 * @param username
 * @return
 */

QDeferred<Credential> AuthAPI::getCredential(const QString &username) const
{
	return getCredential(databaseMain(), username);
}



/**
 * @brief AuthAPI::getCredential
 * @param dbMain
 * @param username
 * @return
 */

QDeferred<Credential> AuthAPI::getCredential(DatabaseMain *dbMain, const QString &username)
{
	Q_ASSERT(dbMain);

	LOG_CTRACE("client") << "Get credential for" << qPrintable(username);

	QDeferred<Credential> ret;

	dbMain->worker()->execInThread([ret, username, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker(dbMain->mutex());

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


/**
 * @brief AuthAPI::updateOAuth2TokenInfo
 * @param flow
 */

void AuthAPI::updateOAuth2TokenInfo(OAuth2CodeFlow *flow) const
{
	Q_ASSERT(flow);
	Q_ASSERT(flow->authenticator());

	m_service->databaseMain()->worker()->execInThread([flow, this]() mutable {
		const QJsonObject &tokenData = flow->token().toJson();
		const QString &username = flow->getUserInfo().username;
		const QString type = QString::fromUtf8(flow->authenticator()->type());

		QSqlDatabase db = QSqlDatabase::database(m_service->databaseMain()->dbName());

		QMutexLocker(m_service->databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("UPDATE auth SET oauthData=")
				.addValue(QString::fromUtf8(QJsonDocument(tokenData).toJson(QJsonDocument::Compact)))
				.addQuery(" WHERE username=")
				.addValue(username)
				.addQuery(" AND oauth=")
				.addValue(type);

		if (q.exec())
			LOG_CDEBUG("oauth2") << "OAuth tokenData updated:" << username;
	});
}



/**
 * @brief AuthAPI::updateOAuth2UserData
 * @param flow
 */

void AuthAPI::updateOAuth2UserData(OAuth2CodeFlow *flow) const
{
	Q_ASSERT(flow);
	Q_ASSERT(flow->authenticator());

	const AdminAPI::User &user = flow->getUserInfo();

	if (user.username.isEmpty()) {
		LOG_CWARNING("oauth2") << "Invalid user in code flow:" << flow->state();
		return;
	}

	if (flow->authenticator()->profileUpdateSupport()) {
		const QJsonObject &oauthData = flow->token().toJson();

		QMetaObject::invokeMethod(flow->authenticator(), "profileUpdate", Qt::QueuedConnection,
								  Q_ARG(QString, user.username),
								  Q_ARG(QJsonObject, oauthData)
								  );
	}
}
