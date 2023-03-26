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
#include "qsqlrecord.h"
#include "serverservice.h"

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

	addMap("^group/(\\d+)/class/add/(\\d+)/*$", this, &TeacherAPI::groupClassAddOne);
	addMap("^group/(\\d+)/class/add/*$", this, &TeacherAPI::groupClassAdd);
	addMap("^group/(\\d+)/class/remove/(\\d+)/*$", this, &TeacherAPI::groupClassRemoveOne);
	addMap("^group/(\\d+)/class/remove/*$", this, &TeacherAPI::groupClassRemove);
	addMap("^group/(\\d+)/class/exclude/*$", this, &TeacherAPI::groupClassExclude);

	addMap("^group/(\\d+)/user/add/(.+)/*$", this, &TeacherAPI::groupUserAddOne);
	addMap("^group/(\\d+)/user/add/*$", this, &TeacherAPI::groupUserAdd);
	addMap("^group/(\\d+)/user/remove/(.+)/*$", this, &TeacherAPI::groupUserRemoveOne);
	addMap("^group/(\\d+)/user/remove/*$", this, &TeacherAPI::groupUserRemove);
	addMap("^group/(\\d+)/user/exclude/*$", this, &TeacherAPI::groupUserExclude);

	addMap("^panel/*$", this, &TeacherAPI::panels);
	addMap("^panel/(\\d+)/*$", this, &TeacherAPI::panelOne);
	addMap("^panel/(\\d+)/grab/*$", this, &TeacherAPI::panelGrab);
	addMap("^panel/(\\d+)/release/*$", this, &TeacherAPI::panelRelease);
	addMap("^panel/(\\d+)/update/*$", this, &TeacherAPI::panelUpdate);
}


/**
 * @brief TeacherAPI::groupOne
 * @param match
 * @param response
 */

void TeacherAPI::groupOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	LOG_CTRACE("client") << "Get group" << id;

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		LOG_CTRACE("client") << "Get group" << id << username;

		QJsonObject data = QueryBuilder::q(db).addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username)
				.addQuery(" AND id=").addValue(id)
				.execToJsonObject();

		if (data.isEmpty())
			return responseError(response, "invalid id");

		data[QStringLiteral("classList")] =
				QueryBuilder::q(db).addQuery("SELECT class.id, name FROM bindGroupClass "
											 "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
											 "WHERE groupid=").addValue(id)
				.execToJsonArray();

		data[QStringLiteral("userList")] =
				QueryBuilder::q(db).addQuery("SELECT classid, user.username, familyName, givenName, nickname, picture, "
											 "class.name as classname FROM bindGroupStudent "
											 "LEFT JOIN user ON (user.username=bindGroupStudent.username) "
											 "LEFT JOIN class ON (class.id=user.classid) "
											 "WHERE user.active=true AND groupid=").addValue(id)
				.execToJsonArray();

		data[QStringLiteral("memberList")] =
				QueryBuilder::q(db).addQuery("SELECT user.username, familyName, givenName, nickname, picture, "
											 "class.name as classname FROM studentGroupInfo "
											 "LEFT JOIN user ON (user.username=studentGroupInfo.username) "
											 "LEFT JOIN class ON (class.id=user.classid) "
											 "WHERE user.active=true AND studentGroupInfo.id=").addValue(id)
				.execToJsonArray();

		data[QStringLiteral("mapList")] =
				QueryBuilder::q(db).addQuery("SELECT id, mapid, active FROM bindGroupMap "
											 "WHERE groupid=").addValue(id)
				.execToJsonArray();


		responseAnswer(response, data);
	});
}




/**
 * @brief TeacherAPI::groups
 * @param response
 */

void TeacherAPI::groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		QueryBuilder q(db);
		q.addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username);

		if (!q.exec())
			return responseErrorSql(response);

		QJsonArray list;

		while (q.sqlQuery().next()) {
			const QSqlRecord &rec = q.sqlQuery().record();
			QJsonObject obj;

			int id = -1;

			for (int i=0; i<rec.count(); ++i) {
				if (rec.fieldName(i) == QStringLiteral("id"))
					id = rec.value(i).toInt();
				obj.insert(rec.fieldName(i), rec.value(i).toJsonValue());
			}

			QJsonArray clist;

			if (id > 0) {
				clist = QueryBuilder::q(db).addQuery("SELECT class.id, name FROM bindGroupClass "
													 "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
													 "WHERE groupid=").addValue(id)
						.execToJsonArray();
			}

			obj.insert(QStringLiteral("classList"), clist);

			list.append(obj);
		}

		responseAnswer(response, "list", list);
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



/**
 * @brief TeacherAPI::groupClassAdd
 * @param id
 * @param list
 * @param response
 */

void TeacherAPI::groupClassAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (id <= 0)
		return responseError(response, "invalid id");

	if (list.isEmpty())
		return responseError(response, "invalid class");

	databaseMainWorker()->execInThread([id, list, response, this]() {
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

		foreach (const QJsonValue &v, list) {
			const int &classid = v.toInt(-1);

			if (classid > 0) {
				if (!QueryBuilder::q(db).addQuery("INSERT OR IGNORE INTO bindGroupClass(")
						.setFieldPlaceholder()
						.addQuery(") VALUES (")
						.setValuePlaceholder()
						.addQuery(")")
						.addField("groupid", id)
						.addField("classid", classid)
						.exec()) {
					db.rollback();
					return responseErrorSql(response);
				}
			} else {
				db.rollback();
				return responseError(response, "invalid class");
			}
		}

		db.commit();

		LOG_CDEBUG("client") << "Class added to group:" << id << list;
		responseAnswerOk(response);
	});
}


/**
 * @brief TeacherAPI::groupClassRemove
 * @param id
 * @param list
 * @param response
 */

void TeacherAPI::groupClassRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (id <= 0)
		return responseError(response, "invalid id");

	if (list.isEmpty())
		return responseError(response, "invalid class");

	databaseMainWorker()->execInThread([list, response, this, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM bindGroupClass WHERE groupid=").addValue(id)
				.addQuery(" AND classid IN (").addList(list.toVariantList())
				.addQuery(") AND (SELECT owner FROM studentgroup WHERE id=").addValue(id)
				.addQuery(")=").addValue(username)
				.exec()) {
			LOG_CWARNING("client") << "Class remove from group error:" << id << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "Class removed from group:" << id << list;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::groupClassExclude
 * @param match
 * @param response
 */

void TeacherAPI::groupClassExclude(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	LOG_CTRACE("client") << "Get excluded classes from group" << id;

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		LOG_CTRACE("client") << "Get group" << id << username;

		if (!QueryBuilder::q(db).addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username)
				.addQuery(" AND id=").addValue(id)
				.execCheckExists())
			return responseError(response, "invalid id");


		bool err = false;
		const QJsonArray &list = QueryBuilder::q(db).addQuery("SELECT c.id, c.name FROM class c WHERE c.id NOT IN "
															  "(SELECT classid FROM bindGroupClass "
															  "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
															  "WHERE groupid=").addValue(id).addQuery(")")
				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}


/**
 * @brief TeacherAPI::groupUserAdd
 * @param id
 * @param list
 * @param response
 */

void TeacherAPI::groupUserAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (id <= 0)
		return responseError(response, "invalid id");

	if (list.isEmpty())
		return responseError(response, "invalid user");

	databaseMainWorker()->execInThread([id, list, response, this]() {
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

		foreach (const QJsonValue &v, list) {
			const QString &user = v.toString();

			if (!user.isEmpty()) {
				if (!QueryBuilder::q(db).addQuery("INSERT OR IGNORE INTO bindGroupStudent(")
						.setFieldPlaceholder()
						.addQuery(") VALUES (")
						.setValuePlaceholder()
						.addQuery(")")
						.addField("groupid", id)
						.addField("username", user)
						.exec()) {
					db.rollback();
					return responseErrorSql(response);
				}
			} else {
				db.rollback();
				return responseError(response, "invalid class");
			}
		}

		db.commit();

		LOG_CDEBUG("client") << "User added to group:" << id << list;
		responseAnswerOk(response);
	});
}




/**
 * @brief TeacherAPI::groupUserRemove
 * @param id
 * @param list
 * @param response
 */

void TeacherAPI::groupUserRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (id <= 0)
		return responseError(response, "invalid id");

	if (list.isEmpty())
		return responseError(response, "invalid user");

	databaseMainWorker()->execInThread([list, response, this, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM bindGroupStudent WHERE groupid=").addValue(id)
				.addQuery(" AND username IN (").addList(list.toVariantList())
				.addQuery(") AND (SELECT owner FROM studentgroup WHERE id=").addValue(id)
				.addQuery(")=").addValue(username)
				.exec()) {
			LOG_CWARNING("client") << "Class remove from group error:" << id << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "User removed from group:" << id << list;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::groupUserExclude
 * @param match
 * @param response
 */

void TeacherAPI::groupUserExclude(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	LOG_CTRACE("client") << "Get excluded classes from group" << id;

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		LOG_CTRACE("client") << "Get group" << id << username;

		if (!QueryBuilder::q(db).addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username)
				.addQuery(" AND id=").addValue(id)
				.execCheckExists())
			return responseError(response, "invalid id");



		bool err = false;
		const QJsonArray &list = QueryBuilder::q(db).addQuery("SELECT classid, user.username, familyName, givenName, nickname, picture, "
															  "class.name as classname FROM user "
															  "LEFT JOIN class ON (class.id=user.classid) WHERE user.active=TRUE "
															  "AND user.isTeacher=false AND user.username NOT IN "
															  "(SELECT username FROM bindGroupStudent WHERE groupid=").addValue(id).addQuery(")")

				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});

}



/**
 * @brief TeacherAPI::panelOne
 * @param match
 * @param response
 */

void TeacherAPI::panelOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	Panel *p = m_service->panel(id);

	if (!p)
		return responseError(response, "invalid id");

	QJsonObject o;
	o.insert(QStringLiteral("id"), p->id());
	o.insert(QStringLiteral("owner"), p->owner());

	if (p->owner() == m_credential.username())
		o.insert(QStringLiteral("config"), p->config());

	responseAnswer(response, o);

}


/**
 * @brief TeacherAPI::panels
 * @param response
 */

void TeacherAPI::panels(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	QJsonArray list;

	foreach (Panel *p, m_service->panels()) {
		if (!p) continue;
		QJsonObject o;
		o.insert(QStringLiteral("id"), p->id());
		o.insert(QStringLiteral("owner"), p->owner());

		if (p->owner() == m_credential.username())
			o.insert(QStringLiteral("config"), p->config());

		list.append(o);
	}

	responseAnswer(response, "list", list);
}


/**
 * @brief TeacherAPI::panelGrab
 * @param match
 * @param response
 */

void TeacherAPI::panelGrab(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	Panel *p = m_service->panel(id);

	if (!p || !p->owner().isEmpty())
		return responseError(response, "invalid id");

	p->setOwner(m_credential.username());
	responseAnswerOk(response);
}


/**
 * @brief TeacherAPI::panelRelease
 * @param match
 * @param response
 */

void TeacherAPI::panelRelease(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	Panel *p = m_service->panel(id);

	if (!p || p->owner() != m_credential.username())
		return responseError(response, "invalid id");

	p->setOwner(QLatin1String(""));
	responseAnswerOk(response);
}



/**
 * @brief TeacherAPI::panelUpdate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::panelUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	Panel *p = m_service->panel(id);

	if (!p)
		return responseError(response, "invalid id");

	if (p->owner() != m_credential.username())
		return responseError(response, "invalid panel");

	p->setConfig(data);
	responseAnswerOk(response);
}


