/*
 * ---- Call of Suli ----
 *
 * adminhandler.cpp
 *
 * Created on: 2023. 01. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AdminHandler
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
#include "utils.h"

AdminHandler::AdminHandler(Client *client)
	: AbstractHandler(client)
{
	m_defaultRoleToValidate = Credential::Admin;
}


/**
 * @brief AdminHandler::userAdd
 * @param user
 * @return
 */

QDefer AdminHandler::userAdd(AbstractHandler *handler, const User &user)
{
	Q_ASSERT(handler);

	LOG_CDEBUG("client") << handler->m_client << "Add new user:" << qPrintable(user.username);

	QDefer ret;

	handler->databaseMain()->worker()->execInThread([ret, user, handler]() mutable {
		if (user.username.isEmpty()) {
			LOG_CWARNING("client") << handler->m_client << "Empty username";
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(handler->databaseMain()->dbName());

		QMutexLocker(handler->databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);

		q.addQuery("SELECT username FROM user WHERE username=")
				.addValue(user.username);

		if (q.exec() && q.sqlQuery().first()) {
			LOG_CWARNING("client") << handler->m_client << "User already exists:" << qPrintable(user.username);
			db.rollback();
			ret.reject();
			return;
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
			LOG_CWARNING("client") << handler->m_client << "User create error:" << qPrintable(user.username);
			db.rollback();
			ret.reject();
			return;
		}

		db.commit();

		LOG_CTRACE("client") << handler->m_client << "User created:" << qPrintable(user.username);
		ret.resolve();
	});

	return ret;
}


/**
 * @brief AdminHandler::authAddPlain
 * @param handler
 * @param username
 * @param password
 * @return
 */

QDefer AdminHandler::authAddPlain(AbstractHandler *handler, const QString &username, const QString &password)
{
	Q_ASSERT(handler);

	LOG_CDEBUG("client") << handler->m_client << "Add plain auth:" << qPrintable(username);

	QDefer ret;

	handler->databaseMain()->worker()->execInThread([ret, username, password, handler]() mutable {
		if (username.isEmpty()) {
			LOG_CWARNING("client") << handler->m_client << "Empty username";
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(handler->databaseMain()->dbName());

		QMutexLocker(handler->databaseMain()->mutex());


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
			LOG_CWARNING("client") << handler->m_client << "User auth create error:" << qPrintable(username);
			ret.reject();
			return;
		}

		LOG_CTRACE("client") << handler->m_client << "User auth created:" << qPrintable(username);
		ret.resolve();
	});

	return ret;
}



/**
 * @brief AdminHandler::authAddOAuth2
 * @param handler
 * @param username
 * @param oauthType
 * @return
 */

QDefer AdminHandler::authAddOAuth2(AbstractHandler *handler, const QString &username, const QString &oauthType)
{
	Q_ASSERT(handler);

	LOG_CDEBUG("client") << handler->m_client << "Add OAuth2 auth:" << qPrintable(username);

	QDefer ret;

	handler->databaseMain()->worker()->execInThread([ret, username, oauthType, handler]() mutable {
		if (username.isEmpty()) {
			LOG_CWARNING("client") << handler->m_client << "Empty username";
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(handler->databaseMain()->dbName());

		QMutexLocker(handler->databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("INSERT OR REPLACE INTO auth(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("oauth", oauthType);


		if (!q.exec()) {
			LOG_CWARNING("client") << handler->m_client << "User auth create error:" << qPrintable(username);
			ret.reject();
			return;
		}

		LOG_CTRACE("client") << handler->m_client << "User auth created:" << qPrintable(username);
		ret.resolve();
	});

	return ret;
}


/**
 * @brief AdminHandler::authPlainPasswordChange
 * @param handler
 * @param username
 * @param oldPassowrd
 * @param password
 * @param check
 * @return
 */

QDefer AdminHandler::authPlainPasswordChange(AbstractHandler *handler, const QString &username,
											 const QString &oldPassword, const QString &password,
											 const bool &check)
{
	Q_ASSERT(handler);

	LOG_CDEBUG("client") << handler->m_client << "Password change:" << qPrintable(username);

	QDefer ret;

	handler->databaseMain()->worker()->execInThread([ret, username, oldPassword, password, check, handler]() mutable {
		if (username.isEmpty()) {
			LOG_CWARNING("client") << handler->m_client << "Empty username";
			ret.reject();
			return;
		}

		QSqlDatabase db = QSqlDatabase::database(handler->databaseMain()->dbName());

		QMutexLocker(handler->databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);
		q.addQuery("SELECT username FROM user WHERE username=")
				.addValue(username);

		if (!q.exec() && !q.sqlQuery().first()) {
			LOG_CWARNING("client") << handler->m_client << "Invalid user:" << qPrintable(username);
			db.rollback();
			ret.reject();
			return;
		}

		q.clear();

		q.addQuery("SELECT password, salt, oauth FROM auth WHERE username=").addValue(username);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CWARNING("client") << handler->m_client << "Sql error";
			db.rollback();
			ret.reject();
			return;
		}

		if (!q.value("oauth").isNull()) {
			LOG_CWARNING("client") << handler->m_client << "Unable to change password for OAuth2 user:" << qPrintable(username);
			db.rollback();
			ret.reject();
			return;
		}

		if (check) {
			const QString &pwd = q.value("password").toString();
			const QString &salt = q.value("salt").toString();

			if (pwd != Credential::hashString(oldPassword, salt)) {
				LOG_CWARNING("client") << handler->m_client << "Invalid password for user:" << qPrintable(username);
				db.rollback();
				ret.reject();
				return;
			}
		}


		QString salt;
		const QString &pwd = Credential::hashString(password, &salt);


		if (!QueryBuilder::q(db).addQuery("INSERT OR REPLACE INTO auth(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("password", pwd)
				.addField("salt", salt)
				.exec()) {
			LOG_CWARNING("client") << handler->m_client << "User password change error:" << qPrintable(username);
			db.rollback();
			ret.reject();
			return;
		}

		db.commit();

		LOG_CTRACE("client") << handler->m_client << "User password changed:" << qPrintable(username);
		ret.resolve();
	});

	return ret;
}



/**
 * @brief AdminHandler::generateClassCode
 * @return
 */

QString AdminHandler::generateClassCode()
{
	return QString::fromLatin1(Utils::generateRandomString(6, "1234567890"));
}




/**
 * @brief AdminHandler::userListByClass
 */

void AdminHandler::userListByClass()
{
	const int &id = json().value(QStringLiteral("classid")).toInt(-1);

	HANDLER_LOG_TRACE() << "User list by class" << id;

	databaseMain()->worker()->execInThread([id, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT user.username, familyName, givenName, active, user.classid, class.name as className, "
				   "isTeacher, isAdmin, isPanel, nickname, character, picture,"
				   "oauth, tokenIat "
				   "FROM user LEFT JOIN auth ON (auth.username=user.username) "
				   "LEFT JOIN class ON (class.id=user.classid)");


		if (id > 0) {
			q.addQuery("WHERE user.classid=").addValue(id);
		} else if (id < -1) {
			q.addQuery("WHERE user.classid=NULL");
		}

		bool err = false;

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err) {
			HANDLER_LOG_DEBUG() << "SQL error";
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		send(m_message.createResponse("list", list));
	});
}



/**
 * @brief AdminHandler::userAdd
 */

void AdminHandler::userAdd()
{
	const QString &username = json().value(QStringLiteral("username")).toString();

	if (username.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username")));
		return;
	}

	const QString &password = json().value(QStringLiteral("password")).toString();

	if (password.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing password")));
		return;
	}

	User user;
	user.username = username;

	HANDLER_LOG_TRACE() << "Add user" << qPrintable(user.username);

	user.familyName = json().value(QStringLiteral("familyName")).toString();
	user.givenName = json().value(QStringLiteral("givenName")).toString();
	user.nickname = json().value(QStringLiteral("nickName")).toString();
	user.character = json().value(QStringLiteral("character")).toString();
	user.picture = json().value(QStringLiteral("picture")).toString();
	user.active = json().value(QStringLiteral("active")).toBool(true);
	user.classid = json().value(QStringLiteral("classid")).toInt(-1);
	user.isAdmin = json().value(QStringLiteral("isAdmin")).toBool(false);
	user.isTeacher = json().value(QStringLiteral("isTeacher")).toBool(false);
	user.isPanel = json().value(QStringLiteral("isPanel")).toBool(false);

	userAdd(this, user)
			.fail([this]{
		send(m_message.createErrorResponse(QStringLiteral("user creation failed")));
	})
			.then([this, user, password](){
		return authAddPlain(this, user.username, password);
	})
			.fail([this]{
		send(m_message.createErrorResponse(QStringLiteral("user creation failed")));
	})
			.done([user, this]{
		send(m_message.createResponse("username", user.username));
	});

}




/**
 * @brief AdminHandler::userModify
 */

void AdminHandler::userModify()
{
	const QString &username = json().value(QStringLiteral("username")).toString();

	if (username.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username")));
		return;
	}

	User user;
	user.username = username;

	HANDLER_LOG_TRACE() << "Modify user" << qPrintable(user.username);

	user.familyName = json().value(QStringLiteral("familyName")).toString();
	user.givenName = json().value(QStringLiteral("givenName")).toString();
	user.nickname = json().value(QStringLiteral("nickName")).toString();
	user.character = json().value(QStringLiteral("character")).toString();
	user.picture = json().value(QStringLiteral("picture")).toString();
	user.active = json().value(QStringLiteral("active")).toBool(true);
	user.classid = json().value(QStringLiteral("classid")).toInt(-1);
	user.isAdmin = json().value(QStringLiteral("isAdmin")).toBool(false);
	user.isTeacher = json().value(QStringLiteral("isTeacher")).toBool(false);
	user.isPanel = json().value(QStringLiteral("isPanel")).toBool(false);

	databaseMain()->worker()->execInThread([user, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);

		q.addQuery("SELECT username FROM user WHERE username=")
				.addValue(user.username);

		if (!q.exec() && !q.sqlQuery().first()) {
			HANDLER_LOG_TRACE() << "User doesn't exists:" << qPrintable(user.username);
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("invalid user")));
			return;
		}

		q.clear();

		q.addQuery("UPDATE user SET ")
				.setCombinedPlaceholder()
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
				.addQuery(" WHERE username=").addValue(user.username);

		if (!q.exec()) {
			HANDLER_LOG_WARNING() << "User modify error:" << user.username;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_TRACE() << "User modified:" << qPrintable(user.username);
		send(m_message.createResponse("username", user.username));
	});

}


/**
 * @brief AdminHandler::userRemove
 */

void AdminHandler::userRemove()
{
	QVariantList users;

	foreach (const QJsonValue &v, json().value(QStringLiteral("list")).toArray()) {
		const QString &u = v.toString();
		if (!u.isEmpty())
			users.append(u);
	}

	const QString &username = json().value(QStringLiteral("username")).toString();

	if (!username.isEmpty())
		users.append(username);

	if (users.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username")));
		return;
	}

	HANDLER_LOG_TRACE() << "Remove users" << users;

	databaseMain()->worker()->execInThread([users, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("DELETE FROM user WHERE username IN (").addList(users).addQuery(")")
				.exec()) {
			HANDLER_LOG_WARNING() << "Users remove error:" << users;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_INFO() << "Users removed:" << users;
		send(m_message.createStatusResponse());
	});
}


/**
 * @brief AdminHandler::userActivate
 */

void AdminHandler::userActivate()
{
	QVariantList users;

	const bool &active = json().value(QStringLiteral("active")).toBool(true);

	foreach (const QJsonValue &v, json().value(QStringLiteral("list")).toArray()) {
		const QString &u = v.toString();
		if (!u.isEmpty())
			users.append(u);
	}

	const QString &username = json().value(QStringLiteral("username")).toString();

	if (!username.isEmpty())
		users.append(username);

	if (users.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username")));
		return;
	}

	HANDLER_LOG_TRACE() << "Activate users" << active << users;

	databaseMain()->worker()->execInThread([users, active, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE user SET active=")
				.addValue(active)
				.addQuery(" WHERE username IN (").addList(users).addQuery(")")
				.exec()) {
			HANDLER_LOG_WARNING() << "Users activate error:" << users;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		if (active)
			HANDLER_LOG_INFO() << "Users activated:" << users;
		else
			HANDLER_LOG_INFO() << "Users deactivated:" << users;
		send(m_message.createStatusResponse());
	});
}


/**
 * @brief AdminHandler::userMoveToClass
 */

void AdminHandler::userMoveToClass()
{
	const int &classid = json().value(QStringLiteral("classid")).toInt(-1);

	QVariantList users;

	foreach (const QJsonValue &v, json().value(QStringLiteral("list")).toArray()) {
		const QString &u = v.toString();
		if (!u.isEmpty())
			users.append(u);
	}

	const QString &username = json().value(QStringLiteral("username")).toString();

	if (!username.isEmpty())
		users.append(username);

	if (users.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username")));
		return;
	}

	HANDLER_LOG_TRACE() << "Users move to class:" << classid << users;

	databaseMain()->worker()->execInThread([users, classid, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE user SET classid=")
				.addValue(classid > 0 ? classid : QVariant(QVariant::Invalid))
				.addQuery(" WHERE username IN (").addList(users).addQuery(")")
				.exec()) {
			HANDLER_LOG_WARNING() << "Users move to class error:" << users;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_INFO() << "Users moved to class:" << classid << users;
		send(m_message.createStatusResponse());
	});
}



/**
 * @brief AdminHandler::userPasswordChange
 */

void AdminHandler::userPasswordChange()
{
	const QString &username = json().value(QStringLiteral("username")).toString();

	if (username.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing username")));
		return;
	}


	const QString &password = json().value(QStringLiteral("password")).toString();

	if (password.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing password")));
		return;
	}

	HANDLER_LOG_TRACE() << "Change password for user:" << qPrintable(username);

	authPlainPasswordChange(this, username, QString(), password, false)
			.fail([this](){
		send(m_message.createErrorResponse(QStringLiteral("error")));
	})			.done([this, username](){
		send(m_message.createResponse("username", username));
	});

}



/**
 * @brief AdminHandler::classAdd
 */

void AdminHandler::classAdd()
{
	Class c;
	c.name = json().value(QStringLiteral("name")).toString();

	HANDLER_LOG_TRACE() << "Add class" << qPrintable(c.name);

	databaseMain()->worker()->execInThread([c, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);
		q.addQuery("INSERT INTO class(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("name", c.name);

		if (!q.exec()) {
			HANDLER_LOG_WARNING() << "Class create error:" << qPrintable(c.name);
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		const int &id = q.sqlQuery().lastInsertId().toInt();

		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO classCode(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("classid", id)
				.addField("code", generateClassCode())
				.exec()) {
			HANDLER_LOG_WARNING() << "Class create error:" << qPrintable(c.name);
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_TRACE() << "Class created:" << qPrintable(c.name) << id;
		send(m_message.createResponse("id", id));
	});
}




/**
 * @brief AdminHandler::classModify
 */

void AdminHandler::classModify()
{
	const int &id = json().value(QStringLiteral("classid")).toInt(-1);

	if (id == -1) {
		send(m_message.createErrorResponse(QStringLiteral("missing classid")));
		return;
	}

	databaseMain()->worker()->execInThread([id, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);
		q.addQuery("UPDATE class SET ").setCombinedPlaceholder();

		if (json().contains(QStringLiteral("name")))
			q.addField("name", json().value(QStringLiteral("name")).toString());

		q.addQuery(" WHERE id=").addValue(id);

		if (!q.fieldCount() || !q.exec()) {
			HANDLER_LOG_WARNING() << "Class modify error:" << id;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_TRACE() << "Class modified:" << id;
		send(m_message.createStatusResponse());
	});
}



/**
 * @brief AdminHandler::classRemove
 */


void AdminHandler::classRemove()
{
	QVariantList list = json().value(QStringLiteral("list")).toArray().toVariantList();

	const int &id = json().value(QStringLiteral("classid")).toInt(-1);

	if (id != -1)
		list.append(id);

	if (list.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing classid")));
		return;
	}

	databaseMain()->worker()->execInThread([list, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();


		if (!QueryBuilder::q(db).addQuery("DELETE FROM class WHERE id IN (")
				.addList(list).addQuery(")").exec()) {
			HANDLER_LOG_WARNING() << "Class remove error:" << list;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_TRACE() << "Class removed:" << list;
		send(m_message.createStatusResponse());
	});

}

