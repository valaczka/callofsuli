/*
 * ---- Call of Suli ----
 *
 * teacherapi.cpp
 *
 * Created on: 2023. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherAPI
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

#include "teacherapi.h"
#include "qjsonarray.h"

TeacherAPI::TeacherAPI(ServerService *service)
	: AbstractAPI(service)
{
	m_validateRole = Credential::Teacher;

	addMap("^group/*$", this, &TeacherAPI::groups);
	addMap("^group/(\\d+)/*$", this, &TeacherAPI::groupOne);
	addMap("^group/create/*$", this, &TeacherAPI::groupCreate);
	addMap("^group/(\\d+)/update/*$", this, &TeacherAPI::groupUpdate);
	addMap("^group/(\\d+)/delete/*$", this, &TeacherAPI::groupDeleteOne);
	addMap("^group/delete/*$", this, &TeacherAPI::groupDelete);
	//addMap("^class/(\\d+)/users/*$", this, &GeneralAPI::classUsers);
}


/**
 * @brief TeacherAPI::groups
 * @param id
 * @param response
 */

void TeacherAPI::groups(const int &id, const QPointer<HttpResponse> &response) const
{
	databaseMainWorker()->execInThread([this, id, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		QueryBuilder q(db);
		q.addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username);

		if (id > 0)
			q.addQuery(" AND id=").addValue(id);

		bool err = false;

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		if (id == -1)
			responseAnswer(response, "list", list);
		else if (list.size() != 1)
			responseError(response, "not found");
		else
			responseAnswer(response, list.at(0).toObject());
	});
}



/**
 * @brief TeacherAPI::groupAdd
 * @param data
 * @param response
 */

void TeacherAPI::groupCreate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const QString &name = data.value(QStringLiteral("name")).toString();

	if (name.isEmpty())
		return responseError(response, "missing name");

	databaseMainWorker()->execInThread([this, name, data, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		QueryBuilder q(db);
		q.addQuery("INSERT INTO studentgroup(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("name", name)
				.addField("owner", username)
				.addField("active", data.value(QStringLiteral("active")).toBool(true))
				;

		if (!q.exec()) {
			LOG_CWARNING("client") << "Group create error:" << qPrintable(name);
			return responseErrorSql(response);
		}

		const int &id = q.sqlQuery().lastInsertId().toInt();

		LOG_CDEBUG("client") << "Group created:" << qPrintable(name) << id;
		responseAnswerOk(response, {
							 { QStringLiteral("id"), id }
						 });
	});
}



/**
 * @brief TeacherAPI::groupModify
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::groupUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([id, data, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db)
				.addQuery("SELECT * FROM studentgroup WHERE id=").addValue(id)
				.addQuery(" AND owner=").addValue(username)
				.execCheckExists()) {
			LOG_CWARNING("client") << "Invalid group:" << id << username;
			db.rollback();
			return responseError(response, "invalid id");
		}

		QueryBuilder q(db);
		q.addQuery("UPDATE studentgroup SET ").setCombinedPlaceholder();

		if (data.contains(QStringLiteral("name")))
			q.addField("name", data.value(QStringLiteral("name")).toString());

		if (data.contains(QStringLiteral("active")))
			q.addField("active", data.value(QStringLiteral("active")).toBool());

		q.addQuery(" WHERE id=").addValue(id);

		if (!q.fieldCount() || !q.exec()) {
			LOG_CWARNING("client") << "Group modify error:" << id;
			db.rollback();
			return responseError(response, "invalid id");
		}

		db.commit();

		LOG_CDEBUG("client") << "Group modified:" << id;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::groupDelete
 * @param list
 * @param response
 */

void TeacherAPI::groupDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([list, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM studentgroup WHERE id IN (").addList(list.toVariantList())
				.addQuery(") AND owner=").addValue(username)
				.exec()) {
			LOG_CWARNING("client") << "Group remove error:" << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "Groups removed:" << list;
		responseAnswerOk(response);
	});
}
