/*
 * ---- Call of Suli ----
 *
 * teacherhandler.cpp
 *
 * Created on: 2023. 01. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherHandler
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

#include "teacherhandler.h"
#include "qjsonarray.h"

TeacherHandler::TeacherHandler(Client *client)
	: AbstractHandler(client)
{
	m_defaultRoleToValidate = Credential::Teacher;
}



/**
 * @brief TeacherHandler::groupList
 */

void TeacherHandler::groupList()
{
	databaseMain()->worker()->execInThread([this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_client->credential().username();

		QueryBuilder q(db);
		q.addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username);

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
 * @brief TeacherHandler::groupAdd
 */

void TeacherHandler::groupAdd()
{
	const QString &name = json().value(QStringLiteral("name")).toString();

	if (name.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing name")));
		return;
	}

	databaseMain()->worker()->execInThread([this, name]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_client->credential().username();

		QueryBuilder q(db);
		q.addQuery("INSERT INTO studentgroup(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("name", name)
				.addField("owner", username)
				.addField("active", json().value(QStringLiteral("active")).toBool(true))
				;

		if (!q.exec()) {
			HANDLER_LOG_WARNING() << "Group create error:" << qPrintable(name);
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		const int &id = q.sqlQuery().lastInsertId().toInt();

		HANDLER_LOG_TRACE() << "Group created:" << qPrintable(name) << id;
		send(m_message.createResponse("id", id));
	});
}



/**
 * @brief TeacherHandler::groupModify
 */

void TeacherHandler::groupModify()
{
	const int &id = json().value(QStringLiteral("id")).toInt(-1);

	if (id == -1) {
		send(m_message.createErrorResponse(QStringLiteral("missing id")));
		return;
	}

	databaseMain()->worker()->execInThread([id, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_client->credential().username();

		db.transaction();

		if (!QueryBuilder::q(db)
				.addQuery("SELECT * FROM studentgroup WHERE id=").addValue(id)
				.addQuery(" AND owner=").addValue(username)
				.execCheckExists()) {
			HANDLER_LOG_WARNING() << "Invalid group:" << id << username;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("invalid group")));
			return;
		}

		QueryBuilder q(db);
		q.addQuery("UPDATE studentgroup SET ").setCombinedPlaceholder();

		if (json().contains(QStringLiteral("name")))
			q.addField("name", json().value(QStringLiteral("name")).toString());

		if (json().contains(QStringLiteral("active")))
			q.addField("active", json().value(QStringLiteral("active")).toBool());

		q.addQuery(" WHERE id=").addValue(id);

		if (!q.fieldCount() || !q.exec()) {
			HANDLER_LOG_WARNING() << "Group modify error:" << id;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_TRACE() << "Group modified:" << id;
		send(m_message.createStatusResponse());
	});
}


/**
 * @brief TeacherHandler::groupRemove
 */

void TeacherHandler::groupRemove()
{
	QVariantList list = json().value(QStringLiteral("list")).toArray().toVariantList();

	const int &id = json().value(QStringLiteral("id")).toInt(-1);

	if (id != -1)
		list.append(id);

	if (list.isEmpty()) {
		send(m_message.createErrorResponse(QStringLiteral("missing id")));
		return;
	}

	databaseMain()->worker()->execInThread([list, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_client->credential().username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM studentgroup WHERE id IN (").addList(list)
				.addQuery(") AND owner=").addValue(username)
				.exec()) {
			HANDLER_LOG_WARNING() << "Group remove error:" << list;
			db.rollback();
			send(m_message.createErrorResponse(QStringLiteral("sql error")));
			return;
		}

		db.commit();

		HANDLER_LOG_TRACE() << "Groups removed:" << list;
		send(m_message.createStatusResponse());
	});
}
