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

		QueryBuilder q(db);
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
			ret.reject();
			return;
		}

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
		q.addQuery("INSERT INTO auth(")
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
		q.addQuery("INSERT INTO auth(")
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
 * @brief AdminHandler::generateClassCode
 * @return
 */

QString AdminHandler::generateClassCode()
{
	return QString::fromLatin1(Utils::generateRandomString(6, "1234567890"));
}


/**
 * @brief AdminHandler::classAdd
 * @param handler
 * @param _class
 * @return
 */

QDefer AdminHandler::classAdd(AbstractHandler *handler, const Class &_class)
{
	Q_ASSERT(handler);

	LOG_CDEBUG("client") << handler->m_client << "Add Class:" << qPrintable(_class.name);

	QDefer ret;

	handler->databaseMain()->worker()->execInThread([ret, _class, handler]() mutable {
		QSqlDatabase db = QSqlDatabase::database(handler->databaseMain()->dbName());

		QMutexLocker(handler->databaseMain()->mutex());

		db.transaction();

		QueryBuilder q(db);
		q.addQuery("INSERT INTO class(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("name", _class.name);

		if (!q.exec()) {
			LOG_CWARNING("client") << handler->m_client << "Class create error:" << qPrintable(_class.name);
			db.rollback();
			ret.reject();
			return;
		}

		const int &id = q.sqlQuery().lastInsertId().toInt();

		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO classCode(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("id", id)
				.addField("code", generateClassCode())
				.exec()) {
			LOG_CWARNING("client") << handler->m_client << "Class create error:" << qPrintable(_class.name);
			db.rollback();
			ret.reject();
			return;
		}

		db.commit();

		LOG_CTRACE("client") << handler->m_client << "Class created:" << qPrintable(_class.name) << id;
		ret.resolve();
	});

	return ret;
}

