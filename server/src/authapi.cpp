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
#include "querybuilder.hpp"
#include "serverservice.h"
#include "adminapi.h"



/**
 * @brief AuthAPI::AuthAPI
 * @param service
 */

AuthAPI::AuthAPI(Handler *handler, ServerService *service)
	: AbstractAPI("auth", handler, service)
{
	auto server = m_handler->httpServer();

	Q_ASSERT(server);

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	server->route(path+"login", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return login(*jsonObject);
	});

	server->route(path+"login/", QHttpServerRequest::Method::Post, [this](const QString &provider, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_GET();
		return loginOAuth2(provider, jsonObject.value_or(QJsonObject{}));
	});

	server->route(path+"registration", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return registration(*jsonObject);
	});

	server->route(path+"registration/", QHttpServerRequest::Method::Post, [this](const QString &provider, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return registrationOAuth2(provider,*jsonObject);
	});
}



/**
 * @brief AuthAPI::login
 * @param data
 * @param response
 */

QHttpServerResponse AuthAPI::login(const QJsonObject &data) const
{
	LOG_CTRACE("client") << "Login";

	const QByteArray &token = data.value(QStringLiteral("token")).toString().toUtf8();
	const QString &username = data.value(QStringLiteral("username")).toString();
	const QString &password = data.value(QStringLiteral("password")).toString();

	if (token.isEmpty() && (username.isEmpty() || password.isEmpty()))
		return responseError("missing username/password");

	if (!token.isEmpty()) {
		if (!Credential::verify(token, m_service->settings()->jwtSecret(), m_service->config().get("tokenFirstIat").toInteger(0))) {
			LOG_CDEBUG("client") << "Token verification failed";
			return responseError("invalid token");
		}

		Credential c = Credential::fromJWT(token);

		if (!c.isValid()) {
			LOG_CDEBUG("client") << "Invalid JWT";
			return responseError("invalid token");
		}

		const QString &username = c.username();

		const auto &ret = getCredential(username);

		if (ret)
			return QHttpServerResponse(getToken(*ret));
		else
			return responseError("invalid user");
	}

	// Without token

	const auto &c = getCredential(username);

	if (c) {
		if (authorizePlain(*c, password))
			return QHttpServerResponse(getToken(*c));
		else
			return responseError("authorization failed");
	} else
		return responseError("invalid user");
}



/**
 * @brief AuthAPI::loginOAuth2
 * @param match
 * @param data
 * @param response
 */

QHttpServerResponse AuthAPI::loginOAuth2(const QString &provider, const QJsonObject &data) const
{
	OAuth2Authenticator *authenticator = m_service->oauth2Authenticator(provider.toUtf8())->lock().get();

	if (!authenticator)
		return responseError("invalid provider");

	if (data.contains(QStringLiteral("state"))) {
		const QString &state = data.value(QStringLiteral("state")).toString();
		const auto &ptr = authenticator->getCodeFlowForState(state);

		if (!ptr)
			return responseError("invalid state");

		OAuth2CodeFlow *flow = ptr->lock().get();

		switch (flow->authState()) {
			case OAuth2CodeFlow::Invalid:
				return responseError("invalid state");
				break;
			case OAuth2CodeFlow::Failed:
			case OAuth2CodeFlow::UserExists:
			case OAuth2CodeFlow::InvalidDomain:
			case OAuth2CodeFlow::InvalidCode:
				return responseError("authentication failed");
				break;
			case OAuth2CodeFlow::TokenReceived:
				return QHttpServerResponse(QJsonObject{
											   { QStringLiteral("pending"), true },
											   { QStringLiteral("tokenReceived"), true }
										   });
				break;
			case OAuth2CodeFlow::Authenticated:
				if (flow->credential().isValid())
					return QHttpServerResponse(getToken(flow->credential()));
				else
					return QHttpServerResponse(QJsonObject{
												   { QStringLiteral("pending"), true },
												   { QStringLiteral("tokenReceived"), true }
											   });
				break;
			case OAuth2CodeFlow::Pending:
				return responseResult("pending", true);
				break;
		}
	}

	std::weak_ptr<OAuth2CodeFlow> ptr;

	QMetaObject::invokeMethod(authenticator, "addCodeFlow",
							  (authenticator->thread() == QThread::currentThread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection),
							  Q_RETURN_ARG(std::weak_ptr<OAuth2CodeFlow>, ptr));

	Q_ASSERT(!ptr.expired());

	OAuth2CodeFlow *flow = ptr.lock().get();

	if (data.value(QStringLiteral("wasm")).toBool())
		flow->setIsWasm(true);

	QObject::connect(flow, &OAuth2CodeFlow::authenticated, flow, [this, flow](){
		QString email = flow->getUserInfo().username;

		LOG_CTRACE("oauth2") << "Login OAuth2 authenticated" << flow << email;

		if (!email.isEmpty()) {
			const auto &c = getCredential(email);

			if (c) {
				flow->setCredential(*c);
				flow->setAuthState(OAuth2CodeFlow::Authenticated);
				updateOAuth2TokenInfo(flow);

				if (m_service->config().get("oauth2NameUpdate").toBool())
					updateOAuth2UserData(flow);
			} else
				flow->setAuthState(OAuth2CodeFlow::Failed);
		}
	});

	return QHttpServerResponse(QJsonObject{
								   { QStringLiteral("state"), flow->state() },
								   { QStringLiteral("url"), flow->requestAuthorizationUrl().toString() }
							   });
}



/**
 * @brief AuthAPI::registration
 * @param data
 * @param response
 */

QHttpServerResponse AuthAPI::registration(const QJsonObject &data) const
{
	if (!m_service->config().registrationEnabled() || m_service->config().oAuth2RegistrationForced()) {
		LOG_CWARNING("client") << "Registration disabled";
		return responseError("registration disabled");
	}

	const QString &username = data.value(QStringLiteral("username")).toString();
	const QString &code = data.value(QStringLiteral("code")).toString();

	LOG_CDEBUG("client") << "Registration:" << qPrintable(username);

	if (!AdminAPI::userNotExists(this, username))
		return responseError("user exists");

	const auto &classid = AdminAPI::getClassIdFromCode(this, code);

	if (!classid)
		return responseError("invalid code");

	AdminAPI::User user;
	user.username = username;
	user.familyName = data.value(QStringLiteral("familyName")).toString();
	user.givenName = data.value(QStringLiteral("givenName")).toString();
	user.picture = data.value(QStringLiteral("picture")).toString();
	user.active = true;
	user.classid = *classid;

	if (!AdminAPI::userAdd(this, user))
		return responseError("registration failed");

	if (!AdminAPI::authAddPlain(this, username, data.value(QStringLiteral("password")).toString()))
		return responseError("registration failed");

	const auto &c = getCredential(username);

	if (!c)
		return responseError("invalid user");
	else
		return QHttpServerResponse(getToken(*c));
}



/**
 * @brief AuthAPI::registrationOAuth2
 * @param match
 * @param data
 * @param response
 */

QHttpServerResponse AuthAPI::registrationOAuth2(const QString &provider, const QJsonObject &data) const
{
	OAuth2Authenticator *authenticator = m_service->oauth2Authenticator(provider.toUtf8())->lock().get();

	if (!authenticator)
		return responseError("invalid provider");

	if (data.contains(QStringLiteral("state"))) {
		const QString &state = data.value(QStringLiteral("state")).toString();
		const auto &ptr = authenticator->getCodeFlowForState(state);

		if (!ptr)
			return responseError("invalid state");

		OAuth2CodeFlow *flow = ptr->lock().get();

		switch (flow->authState()) {
			case OAuth2CodeFlow::Invalid:
				return responseError("invalid state");
				break;
			case OAuth2CodeFlow::Failed:
				return responseError("authentication failed");
				break;
			case OAuth2CodeFlow::UserExists:
				return responseError("user exists");
				break;
			case OAuth2CodeFlow::InvalidCode:
				return responseError("invalid code");
				break;
			case OAuth2CodeFlow::InvalidDomain:
				return responseError("invalid domain");
				break;
			case OAuth2CodeFlow::TokenReceived:
				return QHttpServerResponse(QJsonObject{
											   { QStringLiteral("pending"), true },
											   { QStringLiteral("tokenReceived"), true }
										   });
				break;
			case OAuth2CodeFlow::Authenticated:
				if (flow->credential().isValid())
					return QHttpServerResponse(getToken(flow->credential()));
				else
					return QHttpServerResponse(QJsonObject{
												   { QStringLiteral("pending"), true },
												   { QStringLiteral("tokenReceived"), true }
											   });
				break;
			case OAuth2CodeFlow::Pending:
				return QHttpServerResponse(QJsonObject{
											   { QStringLiteral("pending"), true }
										   });
				break;
		}
	}

	std::weak_ptr<OAuth2CodeFlow> ptr;

	QMetaObject::invokeMethod(authenticator, "addCodeFlow",
							  (authenticator->thread() == QThread::currentThread() ? Qt::DirectConnection : Qt::BlockingQueuedConnection),
							  Q_RETURN_ARG(std::weak_ptr<OAuth2CodeFlow>, ptr));

	Q_ASSERT(!ptr.expired());

	OAuth2CodeFlow *flow = ptr.lock().get();


	QObject::connect(flow, &OAuth2CodeFlow::authenticated, flow, [this, flow, data](){
		AdminAPI::User user = flow->getUserInfo();
		QString code = data.value(QStringLiteral("code")).toString();

		LOG_CTRACE("oauth2") << "Registration OAuth2 authenticated" << flow << user.username << code;

		if (!user.username.isEmpty()) {

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

			if (!domainEnabled)
				return flow->setAuthState(OAuth2CodeFlow::InvalidDomain);

			if (!AdminAPI::userNotExists(this, user.username))
				return flow->setAuthState(OAuth2CodeFlow::UserExists);

			const auto &classid = AdminAPI::getClassIdFromCode(this, code);

			//LOG_CTRACE("client") << "CODE:" << code << *classid;

			if (!classid)
				return flow->setAuthState(OAuth2CodeFlow::InvalidCode);


			user.active = true;
			user.classid = *classid;

			if (!AdminAPI::userAdd(this, user))
				return flow->setAuthState(OAuth2CodeFlow::Failed);

			if (!AdminAPI::authAddOAuth2(this, user.username, QString::fromUtf8(flow->authenticator()->type())))
				return flow->setAuthState(OAuth2CodeFlow::Failed);

			const auto &c = getCredential(user.username);

			if (!c)
				return flow->setAuthState(OAuth2CodeFlow::Failed);

			flow->setCredential(*c);
			flow->setAuthState(OAuth2CodeFlow::Authenticated);

			updateOAuth2TokenInfo(flow);
		}
	});

	return QHttpServerResponse(QJsonObject{
								   { QStringLiteral("state"), flow->state() },
								   { QStringLiteral("url"), flow->requestAuthorizationUrl().toString() }
							   });
}








/**
 * @brief AuthAPI::getCredential
 * @param username
 * @return
 */

std::optional<Credential> AuthAPI::getCredential(const QString &username) const
{
	return getCredential(databaseMain(), username);
}



/**
 * @brief AuthAPI::getCredential
 * @param dbMain
 * @param username
 * @return
 */

std::optional<Credential> AuthAPI::getCredential(DatabaseMain *dbMain, const QString &username)
{
	Q_ASSERT(dbMain);

	LOG_CTRACE("client") << "Get credential for" << qPrintable(username);

	QDefer ret;
	Credential returnCredential;

	dbMain->worker()->execInThread([ret, username, dbMain, &returnCredential]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT active, isAdmin, isTeacher, isPanel FROM user WHERE username=")
				.addValue(username);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(username);
			return ret.reject();
		}

		if (!q.value("active").toBool()) {
			LOG_CDEBUG("client") << "Inactive user:" << qPrintable(username);
			return ret.reject();
		}


		returnCredential.setUsername(username);

		if (q.value("isPanel").toBool()) {
			returnCredential.setRole(Credential::Panel);
		} else {
			returnCredential.setRole(Credential::Student);
			returnCredential.setRole(Credential::Admin, q.value("isAdmin").toBool());
			returnCredential.setRole(Credential::Teacher, q.value("isTeacher").toBool());


			// Load extra roles

			QueryBuilder q(db);
			q.addQuery("SELECT role FROM extraRole WHERE username=").addValue(username);
			if (q.exec()) {
				while (q.sqlQuery().next()) {
					const QString &role = q.value("role").toString();
					if (role == QStringLiteral("sni"))
						returnCredential.setRole(Credential::SNI);
				}
			}
		}

		ret.resolve();
	});

	QDefer::await(ret);

	if (ret.state() == RESOLVED)
		return returnCredential;
	else
		return std::nullopt;
}




/**
 * @brief AuthAPI::authorizePlain
 * @param credential
 * @param password
 * @return
 */


bool AuthAPI::authorizePlain(const Credential &credential, const QString &password) const
{
	LOG_CTRACE("client") << "Authorize plain" << qPrintable(credential.username());

	QDefer ret;

	databaseMainWorker()->execInThread([ret, credential, password, this]() mutable {
		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Invalid credential";
			return ret.reject();
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker _locker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT salt, password, oauth FROM auth WHERE username=")
				.addValue(credential.username());

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(credential.username());
			return ret.reject();
		}

		if (!q.value("oauth").isNull()) {
			LOG_CDEBUG("client") << "OAuth2 user:" << qPrintable(credential.username());
			return ret.reject();
		}

		const QString &storedPassword = q.value("password").toString();

		if (storedPassword.isEmpty()) {
			LOG_CDEBUG("client") << "Empty password stored for user:" << qPrintable(credential.username());
			return ret.reject();
		}

		const QString &hashedPassword = Credential::hashString(password, q.value("salt").toString());

		if (QString::compare(storedPassword, hashedPassword, Qt::CaseInsensitive) == 0)
			ret.resolve();
		else {
			LOG_CDEBUG("client") << "Invalid password for user:" << qPrintable(credential.username());
			ret.reject();
		}
	});

	QDefer::await(ret);

	return (ret.state() == RESOLVED);
}





/**
 * @brief AuthHandler::authorizeOAuth2
 * @param username
 * @param oauthType
 * @return
 */

bool AuthAPI::authorizeOAuth2(const Credential &credential, const char *oauthType) const
{
	LOG_CTRACE("client") << "Authorize OAuth2" << oauthType << qPrintable(credential.username());

	QDefer ret;

	databaseMain()->worker()->execInThread([ret, credential, oauthType, this]() mutable {
		if (!credential.isValid()) {
			LOG_CWARNING("client") << "Invalid credential";
			return ret.reject();
		}

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker _locker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT oauth FROM auth WHERE username=")
				.addValue(credential.username());

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Invalid username:" << qPrintable(credential.username());
			return ret.reject();
		}

		if (q.value("oauth").toString() != QString::fromUtf8(oauthType)) {
			LOG_CDEBUG("client") << "Plain (non oauth2) user:" << qPrintable(credential.username());
			return ret.reject();
		}

		ret.resolve();
	});

	QDefer::await(ret);
	return (ret.state() == RESOLVED);
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
						   { QStringLiteral("auth_token"), QString::fromUtf8(credential.createJWT(m_service->settings()->jwtSecret())) }
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

		QMutexLocker _locker(m_service->databaseMain()->mutex());

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

		LOG_CTRACE("oauth2") << "Profile update" << flow;

		QMetaObject::invokeMethod(flow->authenticator(),
								  std::bind(&OAuth2Authenticator::profileUpdate, flow->authenticator(), user.username, oauthData),
								  Qt::QueuedConnection
								  );
	}
}
