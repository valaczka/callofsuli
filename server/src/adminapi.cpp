/*
 * ---- Call of Suli ----
 *
 * adminapi.cpp
 *
 * Created on: 2023. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AdminAPI
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

#include "adminapi.h"


AdminAPI::AdminAPI(ServerService *service)
	: AbstractAPI(service)
{
	m_validateRole = Credential::Admin;
}


/**
 * @brief AdminAPI::userExists
 * @param username
 * @return
 */

QDefer AdminAPI::userExists(const AbstractAPI *api, const QString &username, const bool &inverse)
{
	Q_ASSERT (api);

	if (inverse)
		LOG_CTRACE("client") << "Check user not exists:" << username;
	else
		LOG_CTRACE("client") << "Check user exists:" << username;

	QDefer ret;

	api->databaseMainWorker()->execInThread([ret, username, api, inverse]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		const bool &r = QueryBuilder::q(db)
				.addQuery("SELECT username FROM user WHERE username=")
				.addValue(username)
				.execCheckExists();

		if (!r != !inverse)
			ret.resolve();
		else
			ret.reject();
	});

	return ret;
}





/**
 * @brief AdminAPI::getClassIdFromCode
 * @param api
 * @param code
 * @return
 */

QDeferred<bool, int> AdminAPI::getClassIdFromCode(const AbstractAPI *api, const QString &code)
{
	Q_ASSERT (api);

	LOG_CTRACE("client") << "Get class id for code:" << code;

	QDeferred<bool, int> ret;

	api->databaseMainWorker()->execInThread([ret, code, api]() mutable {
		if (code.isEmpty()) {
			LOG_CDEBUG("client") << "Empty code";
			ret.resolve(false, -1);
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT code, classid FROM classCode WHERE code=")
				.addValue(code);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Class code doesn't exists:" << qPrintable(code);
			ret.resolve(false, -1);
			return;
		}

		ret.resolve(true, q.value("classid", -1).toInt());
	});

	return ret;
}




/**
 * @brief AdminAPI::userAdd
 * @param api
 * @param user
 * @return
 */

QDefer AdminAPI::userAdd(const AbstractAPI *api, const User &user)
{
	Q_ASSERT(api);

	LOG_CDEBUG("client") << "Add new user:" << qPrintable(user.username);

	QDefer ret;

	if (user.username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		ret.reject();
		return ret;
	}

	api->databaseMainWorker()->execInThread([ret, user, api]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);

		q.addQuery("SELECT username FROM user WHERE username=")
				.addValue(user.username);

		if (q.exec() && q.sqlQuery().first()) {
			LOG_CWARNING("client") << "User already exists:" << qPrintable(user.username);
			db.rollback();
			return ret.reject();
		}

		q.clear();

		q.addQuery("INSERT INTO user(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("username", user.username)
				.addField("familyName", user.familyName)
				.addField("givenName", user.givenName)
				.addField("active", user.active)
				.addField("classid", user.classid > 0 ? user.classid : QVariant(QVariant::Invalid))
				.addField("isTeacher", user.isTeacher)
				.addField("isAdmin", user.isAdmin)
				.addField("isPanel", user.isPanel)
				.addField("nickname", user.nickname)
				.addField("character", user.character)
				.addField("picture", user.picture)
				;


		if (!q.exec()) {
			LOG_CERROR("client") << "User create error:" << qPrintable(user.username);
			db.rollback();
			return ret.reject();
		}

		db.commit();

		LOG_CDEBUG("client") << "User created:" << qPrintable(user.username);
		ret.resolve();
	});

	return ret;
}




/**
 * @brief AdminAPI::authAddPlain
 * @param handler
 * @param username
 * @param password
 * @return
 */

QDefer AdminAPI::authAddPlain(const AbstractAPI *api, const QString &username, const QString &password)
{
	Q_ASSERT(api);

	LOG_CDEBUG("client") << "Add plain auth:" << qPrintable(username);

	QDefer ret;

	if (username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		ret.reject();
		return ret;
	}

	api->databaseMainWorker()->execInThread([ret, username, password, api]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		QString salt;
		QString pwd = Credential::hashString(password, &salt);

		QueryBuilder q(db);
		q.addQuery("INSERT OR REPLACE INTO auth(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("password", pwd)
				.addField("salt", salt);


		if (!q.exec()) {
			LOG_CERROR("client") << "User auth create error:" << qPrintable(username);
			return ret.reject();
		}

		LOG_CTRACE("client") << "User auth created:" << qPrintable(username);
		ret.resolve();
	});

	return ret;
}





/**
 * @brief AdminAPI::authAddOAuth2
 * @param handler
 * @param username
 * @param type
 * @return
 */

QDefer AdminAPI::authAddOAuth2(const AbstractAPI *api, const QString &username, const QString &type)
{
	Q_ASSERT(api);

	LOG_CDEBUG("client") << "Add OAuth2 auth:" << qPrintable(username) << qPrintable(type);

	QDefer ret;

	if (username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		ret.reject();
		return ret;
	}

	api->databaseMainWorker()->execInThread([ret, username, type, api]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("INSERT OR REPLACE INTO auth(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("oauth", type);


		if (!q.exec()) {
			LOG_CERROR("client") << "User auth create error:" << qPrintable(username);
			return ret.reject();
		}

		LOG_CTRACE("client") << "User auth created:" << qPrintable(username);
		ret.resolve();
	});

	return ret;
}
