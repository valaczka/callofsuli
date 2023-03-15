/*
 * ---- Call of Suli ----
 *
 * generalhandler.cpp
 *
 * Created on: 2023. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GeneralHandler
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

#include "generalhandler.h"
#include "client.h"

GeneralHandler::GeneralHandler(Client *client)
	: AbstractHandler(client)
{

}


/**
 * @brief GeneralHandler::getRankList
 * @return
 */

QDeferred<RankList> GeneralHandler::getRankList() const
{
	HANDLER_LOG_TRACE() << "Get rank list";

	QDeferred<RankList> ret;

	databaseMain()->worker()->execInThread([ret, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT id, level, sublevel, xp, name FROM rank ORDER BY level, sublevel");

		if (!q.exec()) {
			HANDLER_LOG_DEBUG() << "SQL error";
			ret.reject(RankList());
			return;
		}

		RankList list;

		list.reserve(q.sqlQuery().size());

		while (q.sqlQuery().next()) {
			Rank r(
						q.value("id").toInt(),
						q.value("level").toInt(),
						q.value("sublevel", -1).toInt(),
						q.value("xp", -1).toInt(),
						q.value("name").toString()
						);
			list.append(r);

		}

		ret.resolve(list);
	});

	return ret;
}



/**
 * @brief GeneralHandler::getUserListGroup
 * @param id
 * @return
 */

QDeferred<QJsonArray> GeneralHandler::getUserListGroup(const int &id) const
{
	HANDLER_LOG_TRACE() << "Get user list group:" << id;

	QDeferred<QJsonArray> ret;

	databaseMain()->worker()->execInThread([ret, id, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery(_SQL_get_user)
				.addQuery("WHERE isPanel=false AND active=true");

		if (id > 0) {
			q.addQuery(" AND user.username IN (SELECT username FROM studentGroupInfo WHERE id=")
					.addValue(id)
					.addQuery(")");
		}

		bool err = false;

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err) {
			HANDLER_LOG_DEBUG() << "SQL error";
			ret.reject(QJsonArray());
			return;
		}

		ret.resolve(list);
	});

	return ret;
}




/**
 * @brief GeneralHandler::getUserListClass
 * @param id
 * @return
 */

QDeferred<QJsonArray> GeneralHandler::getUserListClass(const int &id) const
{
	HANDLER_LOG_TRACE() << "Get user list class:" << id;

	QDeferred<QJsonArray> ret;

	databaseMain()->worker()->execInThread([ret, id, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery(_SQL_get_user)
				.addQuery("WHERE isPanel=false AND active=true");

		if (id > 0) {
			q.addQuery(" AND user.classid=")
					.addValue(id);
		}

		bool err = false;

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err) {
			HANDLER_LOG_DEBUG() << "SQL error";
			ret.reject(QJsonArray());
			return;
		}

		ret.resolve(list);
	});

	return ret;
}


/**
 * @brief GeneralHandler::rankList
 */

void GeneralHandler::rankList()
{
	getRankList().fail([this](RankList){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](RankList list) {
		send(m_message.createResponse("list", list.toJson()));
	});
}


/**
 * @brief GeneralHandler::userList
 */

void GeneralHandler::userList()
{
	getUserListClass().fail([this](QJsonArray){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](QJsonArray list) {
		send(m_message.createResponse("list", list));
	});
}


/**
 * @brief GeneralHandler::userListGroup
 */

void GeneralHandler::userListGroup()
{
	getUserListGroup(json().value(QStringLiteral("id")).toInt(-1)).fail([this](QJsonArray){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](QJsonArray list) {
		send(m_message.createResponse("list", list));
	});
}


/**
 * @brief GeneralHandler::userListClass
 */

void GeneralHandler::userListClass()
{
	getUserListClass(json().value(QStringLiteral("id")).toInt(-1)).fail([this](QJsonArray){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](QJsonArray list) {
		send(m_message.createResponse("list", list));
	});
}



/**
 * @brief GeneralHandler::me
 */

void GeneralHandler::me()
{
	HANDLER_LOG_TRACE() << "Get me";

	if (!validateCredential(Credential::Student, false) && !validateCredential(Credential::Teacher, false)
			&& !validateCredential(Credential::Admin, false)) {
		send(m_message.createErrorResponse(QStringLiteral("permission denied")));
		return;
	}

	const QString &username = m_client->credential().username();

	QDeferred<QJsonObject> ret;

	databaseMain()->worker()->execInThread([ret, username, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery(_SQL_get_user)
				.addQuery("WHERE user.username=")
				.addValue(username)
				.addQuery(" AND active=true");


		bool err = false;

		const QJsonObject &obj = q.execToJsonObject(&err);

		if (err) {
			HANDLER_LOG_DEBUG() << "SQL error";
			ret.reject(QJsonObject());
			return;
		}

		ret.resolve(obj);
	});

	ret.fail([this](QJsonObject){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](QJsonObject obj) {
		send(m_message.createResponse(obj));
	});
}


/**
 * @brief GeneralHandler::classList
 */

void GeneralHandler::classList()
{
	HANDLER_LOG_TRACE() << "Get class list";

	QDeferred<QJsonArray> ret;

	databaseMain()->worker()->execInThread([ret, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT id, name FROM class")
				.execToJsonArray(&err);

		if (err) {
			HANDLER_LOG_DEBUG() << "SQL error";
			ret.reject(QJsonArray());
			return;
		}

		ret.resolve(list);
	});

	ret.fail([this](QJsonArray){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](QJsonArray list) {
		send(m_message.createResponse("list", list));
	});
}
