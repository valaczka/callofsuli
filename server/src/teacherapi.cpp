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
#include "gamemap.h"
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

	addMap("^group/(\\d+)/campaign/create/*$", this, &TeacherAPI::campaignCreate);

	addMap("^campaign/(\\d+)/*$", this, &TeacherAPI::campaignOne);
	addMap("^campaign/(\\d+)/update/*$", this, &TeacherAPI::campaignUpdate);
	addMap("^campaign/(\\d+)/run/*$", this, &TeacherAPI::campaignRun);
	addMap("^campaign/(\\d+)/finish/*$", this, &TeacherAPI::campaignFinish);
	addMap("^campaign/(\\d+)/delete/*$", this, &TeacherAPI::campaignDeleteOne);
	addMap("^campaign/delete/*$", this, &TeacherAPI::campaignDelete);
	addMap("^campaign/(\\d+)/task/*$", this, &TeacherAPI::task);
	addMap("^campaign/(\\d+)/task/create/*$", this, &TeacherAPI::taskCreate);

	addMap("^task/(\\d+)/*$", this, &TeacherAPI::taskOne);
	addMap("^task/(\\d+)/update/*$", this, &TeacherAPI::taskUpdate);
	addMap("^task/(\\d+)/delete/*$", this, &TeacherAPI::taskDeleteOne);
	addMap("^task/delete/*$", this, &TeacherAPI::taskDelete);

	addMap("^panel/*$", this, &TeacherAPI::panels);
	addMap("^panel/(\\d+)/*$", this, &TeacherAPI::panelOne);
	addMap("^panel/(\\d+)/grab/*$", this, &TeacherAPI::panelGrab);
	addMap("^panel/(\\d+)/release/*$", this, &TeacherAPI::panelRelease);
	addMap("^panel/(\\d+)/update/*$", this, &TeacherAPI::panelUpdate);

	addMap("^map/*$", this, &TeacherAPI::maps);
	addMap("^map/create/*$", this, &TeacherAPI::mapCreate);
	addMap("^map/create/([^/]+)/*$", this, &TeacherAPI::mapCreate);
	addMap("^map/delete/*$", this, &TeacherAPI::mapDelete);
	addMap("^map/([^/]+)/*$", this, &TeacherAPI::mapOne);
	addMap("^map/([^/]+)/content/*$", this, &TeacherAPI::mapOneContent);
	addMap("^map/([^/]+)/draft/(\\d+)/*$", this, &TeacherAPI::mapOneDraft);
	addMap("^map/([^/]+)/delete/*$", this, &TeacherAPI::mapDeleteOne);
	addMap("^map/([^/]+)/deleteDraft/(\\d+)/*$", this, &TeacherAPI::mapDeleteDraft);
	addMap("^map/([^/]+)/update/*$", this, &TeacherAPI::mapUpdate);
	addMap("^map/([^/]+)/upload/(\\d+)/*$", this, &TeacherAPI::mapUpload);
	addMap("^map/([^/]+)/publish/(\\d+)/*$", this, &TeacherAPI::mapPublish);

}


/**
 * @brief TeacherAPI::mapMd5
 * @param map
 * @return
 */

QString TeacherAPI::mapMd5(GameMap *map)
{
	Q_ASSERT(map);
	return mapMd5(map->toBinaryData());
}


/**
 * @brief TeacherAPI::mapMd5
 * @param data
 * @return
 */

QString TeacherAPI::mapMd5(const QByteArray &data)
{
	return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());
}



/**
 * @brief TeacherAPI::mapCache
 * @param map
 * @return
 */

QJsonObject TeacherAPI::mapCache(GameMap *map)
{
	Q_ASSERT(map);
	QJsonObject obj;

	QJsonArray mList;
	foreach (GameMapMission *m, map->missions()) {
		mList.append(QJsonObject{
						 { QStringLiteral("uuid"), m->uuid() },
						 { QStringLiteral("name"), m->name() },
						 { QStringLiteral("medal"), m->medalImage() },
						 { QStringLiteral("levels"), m->levels().size() }
					 });
	}

	obj.insert(QStringLiteral("missions"), mList);

	return obj;
}


/**
 * @brief TeacherAPI::mapCacheString
 * @param map
 * @return
 */

QString TeacherAPI::mapCacheString(GameMap *map)
{
	return QString::fromUtf8(QJsonDocument(mapCache(map)).toJson(QJsonDocument::Compact));
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


		QueryBuilder q(db);
		q.addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
				   "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
				   "description, started, finished, defaultGrade "
				   "FROM campaign WHERE groupid=").addValue(id);

		if (!q.exec())
			return responseErrorSql(response);


		QJsonArray list;

		while (q.sqlQuery().next()) {
			const QSqlRecord &rec = q.sqlQuery().record();
			QJsonObject obj;

			int id = -1;

			for (int i=0; i<rec.count(); ++i) {
				const QString &field = rec.fieldName(i);
				obj.insert(field, rec.value(i).toJsonValue());
				if (field == QLatin1String("id"))
					id = rec.value(i).toInt();
			}

			QJsonArray tlist;

			if (id != -1)
				tlist = _taskList(id);

			obj.insert(QStringLiteral("taskList"), tlist);

			list.append(obj);
		}

		data[QStringLiteral("campaignList")] = list;


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



/**
 * @brief TeacherAPI::mapOne
 * @param match
 * @param response
 */

void TeacherAPI::mapOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);

	databaseMainWorker()->execInThread([this, response, uuid]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonObject &obj = QueryBuilder::q(db)
				.addQuery("SELECT mapdb.map.uuid, name, version, md5, CAST(strftime('%s', lastModified) AS INTEGER) AS lastModified, "
						  "COALESCE((SELECT version FROM mapdb.draft WHERE mapdb.draft.uuid=mapdb.map.uuid),-1) AS draftVersion, "
						  "mapdb.cache.data AS cache, length(mapdb.map.data) as size, lastEditor "
						  "FROM mapdb.map LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
						  "WHERE mapdb.map.uuid IN "
						  "(SELECT mapuuid FROM mapOwner WHERE username=").addValue(username)
				.addQuery(") AND mapdb.map.uuid=").addValue(uuid)
				.execToJsonObject({
									  { QStringLiteral("cache"), [](const QVariant &v) {
											return QJsonDocument::fromJson(v.toString().toUtf8()).object();
										} }
								  },
								  &err);

		if (err)
			return responseErrorSql(response);
		else if (obj.isEmpty())
			return responseError(response, "not found");

		responseAnswer(response, obj);
	});
}




/**
 * @brief TeacherAPI::mapContent
 * @param uuid
 * @param response
 * @param isDraft
 */

void TeacherAPI::mapContent(const QString &uuid, const QPointer<HttpResponse> &response, const int &draftVersion) const
{
	databaseMainWorker()->execInThread([this, response, uuid, draftVersion]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		QueryBuilder q(db);

		q.addQuery("SELECT data FROM ")
				.addQuery(draftVersion > 0 ? "mapdb.draft" : "mapdb.map")
				.addQuery(" WHERE uuid IN "
						  "(SELECT mapuuid FROM mapOwner WHERE username=").addValue(username)
				.addQuery(") AND uuid=").addValue(uuid);

		if (draftVersion > 0)
			q.addQuery(" AND version=").addValue(draftVersion);

		if (!q.exec())
			return responseErrorSql(response);

		if (q.sqlQuery().first()) {
			const QByteArray &b = q.sqlQuery().value(QStringLiteral("data")).toByteArray();
			if (response)
				response->setStatus(HttpStatus::Ok, b);
			return;
		} else
			return responseError(response, "not found");

	});
}




/**
 * @brief TeacherAPI::maps
 * @param response
 */

void TeacherAPI::maps(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT mapdb.map.uuid, name, version, md5, CAST(strftime('%s', lastModified) AS INTEGER) AS lastModified, "
						  "COALESCE((SELECT version FROM mapdb.draft WHERE mapdb.draft.uuid=mapdb.map.uuid),-1) AS draftVersion, "
						  "mapdb.cache.data AS cache, length(mapdb.map.data) as size, lastEditor "
						  "FROM mapdb.map LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
						  "WHERE mapdb.map.uuid IN "
						  "(SELECT mapuuid FROM mapOwner WHERE username=").addValue(username).addQuery(")")
				.execToJsonArray({
									 { QStringLiteral("cache"), [](const QVariant &v) {
										   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
									   } }
								 },
								 &err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}



/**
 * @brief TeacherAPI::mapUpdate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::mapUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);

	LOG_CTRACE("client") << "Update map:" << uuid;

	if (uuid.isEmpty())
		return responseError(response, "invalid uuid");

	databaseMainWorker()->execInThread([uuid, data, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT mapuuid FROM mapOwner WHERE username=")
				.addValue(username)
				.addQuery(" AND mapuuid=")
				.addValue(uuid)
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "invalid uuid");
		}

		QueryBuilder q(db);
		q.addQuery("UPDATE mapdb.map SET ").setCombinedPlaceholder();

		if (data.contains(QStringLiteral("name")))
			q.addField("name", data.value(QStringLiteral("name")).toString());

		q.addQuery(" WHERE uuid=").addValue(uuid);

		if (!q.fieldCount() || !q.exec()) {
			LOG_CWARNING("client") << "Map modify error:" << uuid;
			db.rollback();
			return responseError(response, "update error");
		}

		db.commit();

		LOG_CDEBUG("client") << "Map modified:" << uuid;
		responseAnswerOk(response);
	});
}




/**
 * @brief TeacherAPI::mapPublish
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::mapPublish(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);
	int version = match.captured(2).toInt();

	LOG_CTRACE("client") << "Publis map draft:" << uuid << version;

	databaseMainWorker()->execInThread([this, response, uuid, version]() {
		const QString &username = m_credential.username();

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT mapuuid FROM mapOwner WHERE username=")
				.addValue(username)
				.addQuery(" AND mapuuid=")
				.addValue(uuid)
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "map doesn't exists");
		}


		QueryBuilder q(db);

		q.addQuery("SELECT data FROM mapdb.draft WHERE uuid=").addValue(uuid).addQuery(" AND version=").addValue(version);

		if (!q.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		if (!q.sqlQuery().first())
		{
			db.rollback();
			return responseError(response, "invalid uuid/version");
		}

		const QByteArray &b = q.sqlQuery().value(QStringLiteral("data")).toByteArray();

		GameMap *map = GameMap::fromBinaryData(b);

		if (!map) {
			db.rollback();
			return responseError(response, "invalid map");
		}

		QString mapuuid = map->uuid();
		const QString &md5 = mapMd5(b);
		const QString &cache = mapCacheString(map);
		delete map;
		map = nullptr;

		if (mapuuid != uuid) {
			db.rollback();
			return responseError(response, "map uuid mismatch");
		}


		if (!QueryBuilder::q(db)
				.addQuery("UPDATE mapdb.map SET version=version+1, lastModified=datetime('now'), ").setCombinedPlaceholder()
				.addField("md5", md5)
				.addField("data", b)
				.addField("lastEditor", username)
				.addQuery(" WHERE uuid=").addValue(uuid)
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		if (!QueryBuilder::q(db)
				.addQuery("INSERT OR REPLACE INTO mapdb.cache(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
				.addField("uuid", uuid)
				.addField("data", cache)
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		if (!QueryBuilder::q(db)
				.addQuery("DELETE FROM mapdb.draft WHERE uuid=").addValue(uuid).exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::mapDeleteDraft
 * @param match
 * @param response
 */

void TeacherAPI::mapDeleteDraft(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);
	int version = match.captured(2).toInt();

	LOG_CTRACE("client") << "Delete map draft:" << uuid << version;

	databaseMainWorker()->execInThread([this, response, uuid, version]() {
		const QString &username = m_credential.username();

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT mapuuid FROM mapOwner WHERE username=")
				.addValue(username)
				.addQuery(" AND mapuuid=")
				.addValue(uuid)
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "map doesn't exists");
		}

		if (!QueryBuilder::q(db).addQuery("DELETE FROM mapdb.draft WHERE uuid=").addValue(uuid).addQuery(" AND version=").addValue(version).exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		LOG_CTRACE("client") << "Draft deleted" << uuid << version;

		db.commit();
		responseAnswerOk(response);
	});
}




/**
 * @brief TeacherAPI::mapDelete
 * @param list
 * @param response
 */

void TeacherAPI::mapDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "invalid uuid");

	databaseMainWorker()->execInThread([list, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM mapdb.map WHERE uuid IN (SELECT mapuuid FROM mapOwner WHERE mapuuid IN (").addList(list.toVariantList())
				.addQuery(") AND username=").addValue(username).addQuery(")")
				.exec()) {
			LOG_CWARNING("client") << "Map remove error:" << list;
			db.rollback();
			return responseErrorSql(response);
		}

		if (!QueryBuilder::q(db)
				.addQuery("DELETE FROM mapOwner WHERE mapuuid IN (").addList(list.toVariantList())
				.addQuery(") AND mapuuid IN (SELECT mapuuid FROM mapOwner WHERE username=").addValue(username)
				.addQuery(")")
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "Maps removed:" << list;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::mapCreate
 * @param request
 * @param response
 */

void TeacherAPI::mapCreate(const QRegularExpressionMatch &match, HttpRequest *request, QPointer<HttpResponse> response) const
{
	const QString &name = match.captured(1);

	LOG_CTRACE("client") << "Create map:" << qPrintable(name);

	databaseMainWorker()->execInThread([this, response, request, name]() {
		const QByteArray &b = request->body();

		GameMap *map = GameMap::fromBinaryData(b);

		if (!map)
			return responseError(response, "invalid map");

		QString uuid = map->uuid();
		const QString &username = m_credential.username();
		const QString &cache = mapCacheString(map);
		delete map;
		map = nullptr;


		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (QueryBuilder::q(db).addQuery("SELECT uuid FROM mapdb.map WHERE uuid=").addValue(uuid).execCheckExists()) {
			db.rollback();
			return responseError(response, "map already exists");
		}

		const QString &md5 = mapMd5(b);

		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO mapdb.map(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
				.addField("uuid", uuid)
				.addField("name", name.isEmpty() ? uuid : name)
				.addField("md5", md5)
				.addField("data", b)
				.addField("lastEditor", username)
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		if (!QueryBuilder::q(db)
				.addQuery("INSERT OR REPLACE INTO mapdb.cache(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
				.addField("uuid", uuid)
				.addField("data", cache)
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO mapOwner(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
				.addField("mapuuid", uuid)
				.addField("username", username)
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();
		responseAnswerOk(response, {{QStringLiteral("uuid"), uuid}});
	});

}



/**
 * @brief TeacherAPI::mapUpload
 * @param match
 * @param request
 * @param response
 */

void TeacherAPI::mapUpload(const QRegularExpressionMatch &match, HttpRequest *request, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);
	int version = match.captured(2).toInt();

	LOG_CTRACE("client") << "Upload map draft:" << uuid << version;

	databaseMainWorker()->execInThread([this, response, request, uuid, version]() mutable {
		const QByteArray &b = request->body();

		GameMap *map = GameMap::fromBinaryData(b);

		if (!map)
			return responseError(response, "invalid map");

		QString mapuuid = map->uuid();
		delete map;
		map = nullptr;

		if (mapuuid != uuid)
			return responseError(response, "map uuid mismatch");

		const QString &username = m_credential.username();


		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT uuid FROM mapdb.map WHERE uuid=").addValue(uuid).execCheckExists()) {
			db.rollback();
			return responseError(response, "map doesn't exists");
		}

		if (!QueryBuilder::q(db).addQuery("SELECT mapuuid FROM mapOwner WHERE username=")
				.addValue(username)
				.addQuery(" AND mapuuid=")
				.addValue(uuid)
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "map doesn't exists");
		}


		if (version == 0 && QueryBuilder::q(db).addQuery("SELECT version FROM mapdb.draft WHERE uuid=").addValue(uuid).execCheckExists()) {
			db.rollback();
			return responseError(response, "version mismatch");
		}

		if (version > 0 && !QueryBuilder::q(db).addQuery("SELECT version FROM mapdb.draft WHERE uuid=").addValue(uuid)
				.addQuery(" AND version=").addValue(version)
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "version mismatch");
		}

		++version;


		if (!QueryBuilder::q(db)
				.addQuery("INSERT OR REPLACE INTO mapdb.draft(uuid, version, lastModified, data) VALUES (")
				.addValue(uuid)
				.addValue(version)
				.addQuery(", datetime('now'), ")
				.addValue(b)
				.addQuery(")")
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();
		responseAnswerOk(response, {{QStringLiteral("version"), version}});
	});
}



/**
 * @brief TeacherAPI::campaignOne
 * @param match
 * @param response
 */

void TeacherAPI::campaignOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		QJsonObject obj = QueryBuilder::q(db)
				.addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
						  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
						  "description, started, finished, defaultGrade "
						  "FROM campaign WHERE id=").addValue(id)
				.addQuery(" AND groupid IN (SELECT id FROM studentgroup WHERE owner=").addValue(username).addQuery(")")
				.execToJsonObject(&err);

		if (err)
			return responseErrorSql(response);

		if (obj.isEmpty())
			return responseError(response, "not found");

		obj.insert(QStringLiteral("taskList"), _taskList(id));

		responseAnswer(response, obj);
	});
}






/**
 * @brief TeacherAPI::campaignCreate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignCreate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &groupid = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, groupid, data]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=")
				.addValue(groupid)
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "invalid group");
		}

		QueryBuilder q(db);
		q.addQuery("INSERT INTO campaign(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("groupid", groupid)
				;

		if (data.contains(QStringLiteral("starttime"))) {
			const int &g = data.value(QStringLiteral("starttime")).toInt();
			q.addField("starttime", g>0 ? QDateTime::fromSecsSinceEpoch(g).toUTC() : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("endtime"))) {
			const int &g = data.value(QStringLiteral("endtime")).toInt();
			q.addField("endtime", g>0 ? QDateTime::fromSecsSinceEpoch(g).toUTC() : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("defaultGrade"))) {
			const int &g = data.value(QStringLiteral("defaultGrade")).toInt();
			q.addField("defaultGrade", g>0 ? g : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("description")))
			q.addField("description", data.value(QStringLiteral("description")).toString());

		if (!q.exec()) {
			LOG_CWARNING("client") << "Campaign create error in group:" << groupid;
			db.rollback();
			return responseErrorSql(response);
		}

		const int &id = q.sqlQuery().lastInsertId().toInt();

		db.commit();

		LOG_CDEBUG("client") << "Campaign created:" << id;
		responseAnswerOk(response, {
							 { QStringLiteral("id"), id }
						 });
	});
}



/**
 * @brief TeacherAPI::campaignUpdate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([id, data, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=")
				.addValue(id)
				.addQuery(")")
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "invalid campaign");
		}

		QueryBuilder q(db);
		q.addQuery("UPDATE campaign SET ").setCombinedPlaceholder();

		if (data.contains(QStringLiteral("starttime"))) {
			const int &g = data.value(QStringLiteral("starttime")).toInt();
			q.addField("starttime", g>0 ? QDateTime::fromSecsSinceEpoch(g).toUTC() : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("endtime"))) {
			const int &g = data.value(QStringLiteral("endtime")).toInt();
			q.addField("endtime", g>0 ? QDateTime::fromSecsSinceEpoch(g).toUTC() : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("defaultGrade"))) {
			const int &g = data.value(QStringLiteral("defaultGrade")).toInt();
			q.addField("defaultGrade", g>0 ? g : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("description")))
			q.addField("description", data.value(QStringLiteral("description")).toString());

		q.addQuery(" WHERE id=").addValue(id);

		if (!q.fieldCount() || !q.exec()) {
			LOG_CWARNING("client") << "Campaign modify error:" << id;
			db.rollback();
			return responseError(response, "invalid campaign");
		}

		db.commit();

		LOG_CDEBUG("client") << "Campaign modified:" << id;
		responseAnswerOk(response);
	});
}




/**
 * @brief TeacherAPI::campaignDelete
 * @param list
 * @param response
 */

void TeacherAPI::campaignDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([list, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM campaign WHERE id IN "
						 "(SELECT campaign.id FROM campaign LEFT JOIN studentgroup ON (studentgroup.id=campaign.groupid) WHERE campaign.id IN(")
				.addList(list.toVariantList())
				.addQuery(") AND studentgroup.owner=").addValue(username)
				.addQuery(")")
				.exec()) {
			LOG_CWARNING("client") << "Campaign delete error:" << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "Campaigns deleted:" << list;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::campaignRun
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignRun(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([id, data, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=")
				.addValue(id)
				.addQuery(")")
				.execCheckExists()) {
			return responseError(response, "invalid campaign");
		}

		AdminAPI::campaignStart(this, id)
				.fail([response, this](){responseError(response, "campaign run error");})
				.done([response, this](){responseAnswerOk(response);});
	});
}




/**
 * @brief TeacherAPI::campaignFinish
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignFinish(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([id, data, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=")
				.addValue(id)
				.addQuery(")")
				.execCheckExists()) {
			return responseError(response, "invalid campaign");
		}

		AdminAPI::campaignFinish(this, id)
				.fail([response, this](){responseError(response, "campaign finish error");})
				.done([response, this](){responseAnswerOk(response);});
	});
}



/**
 * @brief TeacherAPI::taskOne
 * @param match
 * @param response
 */

void TeacherAPI::taskOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=(SELECT campaignid FROM task WHERE id=")
				.addValue(id)
				.addQuery("))")
				.execCheckExists()) {
			return responseError(response, "invalid id");
		}

		responseAnswer(response, _task(id));
	});
}





/**
 * @brief TeacherAPI::task
 * @param match
 * @param response
 */

void TeacherAPI::task(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &campaign = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, campaign]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=")
				.addValue(campaign)
				.addQuery(")")
				.execCheckExists()) {
			return responseError(response, "invalid campaign");
		}

		responseAnswer(response, "list", _taskList(campaign));
	});
}



/**
 * @brief TeacherAPI::taskCreate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::taskCreate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &campaign = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, campaign, data]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=")
				.addValue(campaign)
				.addQuery(")")
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "invalid campaign");
		}

		QueryBuilder q(db);
		q.addQuery("INSERT INTO task(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("campaignid", campaign)
				;

		if (data.contains(QStringLiteral("gradeid"))) {
			const int &g = data.value(QStringLiteral("gradeid")).toInt();
			q.addField("gradeid", g>0 ? g : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("xp"))) {
			const int &g = data.value(QStringLiteral("xp")).toInt();
			q.addField("xp", g>0 ? g : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("required")))
			q.addField("required", data.value(QStringLiteral("required")).toBool());

		if (data.contains(QStringLiteral("mapuuid")))
			q.addField("mapuuid", data.value(QStringLiteral("mapuuid")).toString());

		if (data.contains(QStringLiteral("criterion")))
			q.addField("criterion", QString::fromUtf8(QJsonDocument(data.value(QStringLiteral("criterion")).toObject()).toJson(QJsonDocument::Compact)));


		if (!q.exec()) {
			LOG_CWARNING("client") << "Task create error in campaign:" << campaign;
			db.rollback();
			return responseErrorSql(response);
		}

		const int &id = q.sqlQuery().lastInsertId().toInt();

		db.commit();

		LOG_CDEBUG("client") << "Task created:" << id;
		responseAnswerOk(response, {
							 { QStringLiteral("id"), id }
						 });
	});
}




/**
 * @brief TeacherAPI::taskUpdate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::taskUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, id, data]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		const QString &username = m_credential.username();

		if (!QueryBuilder::q(db).addQuery("SELECT id FROM studentgroup WHERE owner=")
				.addValue(username)
				.addQuery(" AND id=(SELECT groupid FROM campaign WHERE id=(SELECT campaignid FROM task WHERE id=")
				.addValue(id)
				.addQuery("))")
				.execCheckExists()) {
			db.rollback();
			return responseError(response, "invalid task");
		}

		QueryBuilder q(db);
		q.addQuery("UPDATE task SET ").setCombinedPlaceholder();

		if (data.contains(QStringLiteral("gradeid"))) {
			const int &g = data.value(QStringLiteral("gradeid")).toInt();
			q.addField("gradeid", g>0 ? g : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("xp"))) {
			const int &g = data.value(QStringLiteral("xp")).toInt();
			q.addField("xp", g>0 ? g : QVariant(QVariant::Invalid));
		}

		if (data.contains(QStringLiteral("required")))
			q.addField("required", data.value(QStringLiteral("required")).toBool());

		if (data.contains(QStringLiteral("mapuuid")))
			q.addField("mapuuid", data.value(QStringLiteral("mapuuid")).toString());

		if (data.contains(QStringLiteral("criterion")))
			q.addField("criterion", QString::fromUtf8(QJsonDocument(data.value(QStringLiteral("criterion")).toObject()).toJson(QJsonDocument::Compact)));

		q.addQuery(" WHERE id=").addValue(id);

		if (!q.fieldCount() || !q.exec()) {
			LOG_CWARNING("client") << "Task modify error:" << id;
			db.rollback();
			return responseError(response, "invalid task");
		}

		db.commit();

		LOG_CDEBUG("client") << "Task updated:" << id;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::taskDelete
 * @param list
 * @param response
 */

void TeacherAPI::taskDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([list, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM task WHERE id IN "
						 "(SELECT task.id FROM task LEFT JOIN campaign ON (campaign.id=task.campaignid) "
						 "LEFT JOIN studentgroup ON (studentgroup.id=campaign.groupid) WHERE task.id IN(")
				.addList(list.toVariantList())
				.addQuery(") AND studentgroup.owner=").addValue(username)
				.addQuery(")")
				.exec()) {
			LOG_CWARNING("client") << "Task delete error:" << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "Tasks deleted:" << list;
		responseAnswerOk(response);
	});
}




/**
 * @brief TeacherAPI::_task
 * @param id
 * @return
 */


QJsonObject TeacherAPI::_task(const int &id) const
{
	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker(databaseMain()->mutex());

	return QueryBuilder::q(db)
			.addQuery("SELECT id, gradeid, xp, required, mapuuid, criterion, map.name as mapname FROM task "
					  "LEFT JOIN mapdb.map ON (mapdb.map.uuid=task.mapuuid) WHERE id=").addValue(id)
			.execToJsonObject({
								  { QStringLiteral("criterion"), [](const QVariant &v) {
										return QJsonDocument::fromJson(v.toString().toUtf8()).object();
									} }
							  });
}




/**
 * @brief TeacherAPI::_taskList
 * @param campaign
 * @return
 */

QJsonArray TeacherAPI::_taskList(const int &campaign) const
{
	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker(databaseMain()->mutex());

	return QueryBuilder::q(db)
			.addQuery("SELECT id, gradeid, xp, required, mapuuid, criterion, map.name as mapname FROM task "
					  "LEFT JOIN mapdb.map ON (mapdb.map.uuid=task.mapuuid) WHERE campaignid=").addValue(campaign)
			.execToJsonArray({
								 { QStringLiteral("criterion"), [](const QVariant &v) {
									   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
								   } }
							 });
}



/**
 * @brief TeacherAPI::_campaignUserResutl
 * @param api
 * @param campaign
 * @param username
 * @return
 */

TeacherAPI::UserCampaignResult TeacherAPI::_campaignUserResult(const AbstractAPI *api, const int &campaign, const bool &finished,
															   const QString &username, bool *err)
{
	Q_ASSERT(api);

	UserCampaignResult result;

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	bool e = false;

	const QJsonArray &list = QueryBuilder::q(db)
			.addQuery("SELECT task.id, gradeid, grade.value AS gradeValue, xp, required, mapuuid, criterion, map.name as mapname, "
					  "(taskSuccess.username IS NOT NULL) AS success FROM task "
					  "LEFT JOIN mapdb.map ON (mapdb.map.uuid=task.mapuuid) "
					  "LEFT JOIN grade ON (grade.id=task.gradeid) "
					  "LEFT JOIN taskSuccess ON (taskSuccess.taskid=task.id AND taskSuccess.username=")
			.addValue(username)
			.addQuery(") WHERE campaignid=").addValue(campaign)
			.execToJsonArray({
								 { QStringLiteral("criterion"), [](const QVariant &v) {
									   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
								   } }
							 }, &e);

	if (e) {
		if (err) *err = true;
		return result;
	}


	result.tasks = list;



	// Finished campaign

	if (finished) {
		if (err) *err = false;
		return result;
	}




	// Default grade

	QueryBuilder q(db);
	q.addQuery("SELECT defaultGrade, grade.value AS value FROM campaign LEFT JOIN grade ON (grade.id=campaign.defaultGrade) WHERE campaign.id=").addValue(campaign);

	if (!q.exec()) {
		if (err) *err = true;
		return result;
	}

	if (q.sqlQuery().first()) {
		result.grade = q.value("defaultGrade", -1).toInt();
		result.gradeValue = q.value("value", -1).toInt();
	}




	/// Calculate result

	struct ResultTask {
		bool required = false;
		bool success = false;

		ResultTask(const bool &r, const bool &s) : required(r), success(s) {}

		static bool allCompleted(const QVector<ResultTask> &list) {
			bool completed = true;

			foreach (const ResultTask &r, list) {
				if (!r.success) {
					completed = false;
					break;
				}
			}

			return completed;
		}

		static bool hasRequired(const QVector<ResultTask> &list) {
			bool has = false;

			foreach (const ResultTask &r, list) {
				if (r.required) {
					has = true;
					break;
				}
			}

			return has;
		}
	};

	struct ResultGrade {
		int gradevalue = -1;
		bool success = false;
		QVector<ResultTask> tasks;

		ResultGrade(const int &v, const QVector<ResultTask> &t) : gradevalue(v), tasks(t) {}
	};

	struct ResultXP {
		bool success = false;
		QVector<ResultTask> tasks;

		ResultXP(const QVector<ResultTask> &t) : tasks(t) {}
	};


	QMap<int, ResultGrade> gradeList;
	QMap<int, ResultXP> xpList;


	// Task list

	foreach (const QJsonValue &v, list) {
		const QJsonObject &o = v.toObject();

		const int &gradeid = o.value(QStringLiteral("gradeid")).toInt(-1);
		const int &gradeValue = o.value(QStringLiteral("gradeValue")).toInt(-1);
		const int &xp = o.value(QStringLiteral("xp")).toInt(-1);
		const bool &required = o.value(QStringLiteral("required")).toVariant().toBool();
		const bool &success = o.value(QStringLiteral("success")).toVariant().toBool();

		if (gradeid > 0) {
			auto it = gradeList.find(gradeid);
			if (it != gradeList.end())
				it.value().tasks.append({required, success});
			else
				gradeList.insert(gradeid, {gradeValue, {{required, success}}});
		}

		if (xp > 0) {
			auto it = xpList.find(xp);
			if (it != xpList.end())
				it.value().tasks.append({required, success});
			else
				xpList.insert(xp, {{{required, success}}});
		}
	}

	int maxRequiredValue = -1;

	for (auto it = gradeList.begin(); it != gradeList.end(); ++it) {
		it->success = ResultTask::allCompleted(it->tasks);
		const bool &r = ResultTask::hasRequired(it->tasks);
		if (r && !it->success)
			maxRequiredValue = it->gradevalue;
	}

	for (auto it = gradeList.constBegin(); it != gradeList.constEnd(); ++it) {
		if (!it->success)
			continue;

		if (it->gradevalue > result.gradeValue && (maxRequiredValue == -1 || it->gradevalue <= maxRequiredValue)) {
			result.gradeValue = it->gradevalue;
			result.grade = it.key();
		}
	}


	int maxRequiredXP = -1;

	for (auto it = xpList.begin(); it != xpList.end(); ++it) {
		it->success = ResultTask::allCompleted(it->tasks);
		const bool &r = ResultTask::hasRequired(it->tasks);
		if (r && !it->success)
			maxRequiredXP = it.key();
	}

	for (auto it = xpList.constBegin(); it != xpList.constEnd(); ++it) {
		if (!it->success)
			continue;

		if (it.key() > result.xp && (maxRequiredXP == -1 || it.key() <= maxRequiredXP)) {
			result.xp = it.key();
		}
	}

	if (err) *err = false;
	return result;
}



/**
 * @brief TeacherAPI::_evaluateCampaign
 * @param api
 * @param campaign
 * @param username
 * @param err
 * @return
 */

bool TeacherAPI::_evaluateCampaign(const AbstractAPI *api, const int &campaign, const QString &username, bool *err)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	QueryBuilder q(db);

	q.addQuery("SELECT id, mapuuid, criterion FROM task WHERE campaignid=").addValue(campaign);

	if (!q.exec()) {
		if (err) *err = true;
		return false;
	}


	db.transaction();

	while (q.sqlQuery().next()) {
		const int &task = q.value("id").toInt();
		const QString &map = q.value("mapuuid").toString();
		const QJsonObject &criterion = QJsonDocument::fromJson(q.value("criterion").toString().toUtf8()).object();
		const QString &module = criterion.value(QStringLiteral("module")).toString();

		bool success = false;
		bool e = false;

		if (module == QLatin1String("xp"))
			success = _evaluateCriterionXP(api, campaign, criterion, username, &e);
		else if (module == QLatin1String("mission"))
			success = _evaluateCriterionMission(api, campaign, criterion, map, username, &e);

		if (e) {
			db.rollback();
			if (err) *err = true;
			return false;
		}

		if (success) {
			if (!QueryBuilder::q(db)
					.addQuery("INSERT OR IGNORE INTO taskSuccess(")
					.setFieldPlaceholder()
					.addQuery(") VALUES (")
					.setValuePlaceholder()
					.addQuery(")")
					.addField("taskid", task)
					.addField("username", username)
					.exec()) {
				db.rollback();
				if (err) *err = true;
				return false;
			}
		} else  {
			if (!QueryBuilder::q(db)
					.addQuery("DELETE FROM taskSuccess WHERE taskid=").addValue(task)
					.addQuery(" AND username=").addValue(username)
					.exec()) {
				db.rollback();
				if (err) *err = true;
				return false;
			}
		}
	}


	db.commit();

	if (err) *err = false;
	return true;
}



/**
 * @brief TeacherAPI::_evaluateCriterionXP
 * @param api
 * @param criterion
 * @param username
 * @param err
 * @return
 */

bool TeacherAPI::_evaluateCriterionXP(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &username, bool *err)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	bool e = false;

	const int &xp = QueryBuilder::q(db)
			.addQuery("SELECT SUM(xp) AS xp FROM game LEFT JOIN score ON (game.scoreid=score.id) WHERE game.username=").addValue(username)
			.addQuery(" AND campaignid=").addValue(campaign)
			.execToValue("xp", &e).toInt();

	if (e) {
		if (err) *err = true;
		return false;
	}

	if (xp >= criterion.value(QStringLiteral("xp")).toInt())
		return true;

	return false;
}



/**
 * @brief TeacherAPI::_evaluateCriterionMission
 * @param api
 * @param campaign
 * @param criterion
 * @param map
 * @param username
 * @param err
 * @return
 */

bool TeacherAPI::_evaluateCriterionMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion,
										   const QString &map, const QString &username, bool *err)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	bool e = false;

	QueryBuilder q(db);

	q.addQuery("SELECT * FROM game WHERE success=true AND username=").addValue(username)
			.addQuery(" AND campaignid=").addValue(campaign)
			.addQuery(" AND mapid=").addValue(map)
			.addQuery(" AND missionid=").addValue(criterion.value(QStringLiteral("mission")).toString())
			;

	if (criterion.contains(QStringLiteral("level")))
		q.addQuery(" AND level=").addValue(criterion.value(QStringLiteral("level")).toInt());

	if (criterion.contains(QStringLiteral("deathmatch")))
		q.addQuery(" AND deathmatch=").addValue(criterion.value(QStringLiteral("deathmatch")).toVariant().toBool());

	const bool &success = q.execCheckExists(&e);

	if (e) {
		if (err) *err = true;
		return false;
	}

	return success;
}


