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
#include "QtConcurrent/qtconcurrentrun.h"
#include "gamemap.h"
#include "qjsonarray.h"
#include "qsqlrecord.h"
#include "serverservice.h"




#define CHECK_GROUP(username, id)	\
	LAMBDA_SQL_ERROR("invalid id", \
	QueryBuilder::q(db).addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username) \
	.addQuery(" AND id=").addValue(id) \
	.execCheckExists());


#define CHECK_MAP(username, uuid)	\
	LAMBDA_SQL_ERROR("invalid uuid", \
	QueryBuilder::q(db).addQuery("SELECT mapuuid FROM mapOwner WHERE username=") \
	.addValue(username).addQuery(" AND mapuuid=").addValue(uuid) \
	.execCheckExists());


/**
 * @brief TeacherAPI::TeacherAPI
 * @param handler
 * @param service
 */

TeacherAPI::TeacherAPI(Handler *handler, ServerService *service)
	: AbstractAPI("teacher", handler, service)
{
	auto server = m_handler->httpServer().lock().get();

	Q_ASSERT(server);

	m_validateRole = Credential::Teacher;

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	server->route(path+"group", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groups, &*this, *credential);
	});

	server->route(path+"group/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::group, &*this, *credential, id);
	});

	server->route(path+"group/create", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupCreate, &*this, *credential, *jsonObject);
	});

	server->route(path+"group", QHttpServerRequest::Method::Put, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupCreate, &*this, *credential, *jsonObject);
	});

	server->route(path+"group/<arg>/update", QHttpServerRequest::Method::Post, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupUpdate, &*this, *credential, id, *jsonObject);
	});

	server->route(path+"group/<arg>/delete", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupDelete, &*this, *credential, QJsonArray{id});
	});

	server->route(path+"group/", QHttpServerRequest::Method::Delete, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupDelete, &*this, *credential, QJsonArray{id});
	});

	server->route(path+"group/delete", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupDelete, &*this, *credential, jsonObject->value(QStringLiteral("list")).toArray());
	});


	server->route(path+"group/<arg>/class/add/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const int &classid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupClassAdd, &*this, *credential, id, QJsonArray{classid});
	});

	server->route(path+"group/<arg>/class/", QHttpServerRequest::Method::Put,
				  [this](const int &id, const int &classid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupClassAdd, &*this, *credential, id, QJsonArray{classid});
	});

	server->route(path+"group/<arg>/class/add", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupClassAdd, &*this, *credential, id, jsonObject->value(QStringLiteral("list")).toArray());
	});

	server->route(path+"group/<arg>/class/remove/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const int &classid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupClassRemove, &*this, *credential, id, QJsonArray{classid});
	});

	server->route(path+"group/<arg>/class/", QHttpServerRequest::Method::Delete,
				  [this](const int &id, const int &classid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupClassRemove, &*this, *credential, id, QJsonArray{classid});
	});

	server->route(path+"group/<arg>/class/remove", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupClassRemove, &*this, *credential, id, jsonObject->value(QStringLiteral("list")).toArray());
	});

	server->route(path+"group/<arg>/class/exclude", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupClassExclude, &*this, *credential, id);
	});


	server->route(path+"group/<arg>/user/add/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QString &userid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupUserAdd, &*this, *credential, id, QJsonArray{userid});
	});

	server->route(path+"group/<arg>/user/", QHttpServerRequest::Method::Put,
				  [this](const int &id, const QString &userid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupUserAdd, &*this, *credential, id, QJsonArray{userid});
	});

	server->route(path+"group/<arg>/user/add", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupUserAdd, &*this, *credential, id, jsonObject->value(QStringLiteral("list")).toArray());
	});

	server->route(path+"group/<arg>/user/remove/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QString &userid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupUserRemove, &*this, *credential, id, QJsonArray{userid});
	});

	server->route(path+"group/<arg>/user/", QHttpServerRequest::Method::Delete,
				  [this](const int &id, const QString &userid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupUserRemove, &*this, *credential, id, QJsonArray{userid});
	});

	server->route(path+"group/<arg>/user/remove", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::groupUserRemove, &*this, *credential, id, jsonObject->value(QStringLiteral("list")).toArray());
	});

	server->route(path+"group/<arg>/user/exclude", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupUserExclude, &*this, *credential, id);
	});


	server->route(path+"group/<arg>/result", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::groupResult, &*this, *credential, id);
	});

	server->route(path+"group/<arg>/result/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_GET();
		return QtConcurrent::run(&TeacherAPI::groupUserResult, &*this, *credential, id, username, jsonObject.value_or(QJsonObject{}));
	});

	server->route(path+"group/<arg>/log", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_GET();
		return QtConcurrent::run(&TeacherAPI::groupGameLog, &*this, *credential, id, jsonObject.value_or(QJsonObject{}));
	});





	server->route(path+"map", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::map, &*this, *credential, QStringLiteral(""));
	});

	server->route(path+"map/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::map, &*this, *credential, uuid);
	});

	server->route(path+"map/create", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapCreate, &*this, *credential, request.body(), QStringLiteral(""));
	});

	server->route(path+"map/create/", QHttpServerRequest::Method::Post,
				  [this](const QString &name, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapCreate, &*this, *credential, request.body(), name);
	});

	server->route(path+"map", QHttpServerRequest::Method::Put, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapCreate, &*this, *credential, request.body(), QStringLiteral(""));
	});

	server->route(path+"map/<arg>/update", QHttpServerRequest::Method::Post, [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::mapUpdate, &*this, *credential, uuid, *jsonObject);
	});

	server->route(path+"map/<arg>/publish/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const int &version, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapPublish, &*this, *credential, uuid, version);
	});

	server->route(path+"map/<arg>/deleteDraft/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const int &version, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapDeleteDraft, &*this, *credential, uuid, version);
	});

	server->route(path+"map/<arg>/upload/", QHttpServerRequest::Method::Post,
				  [this](const QString &uuid, const int &version, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapUpload, &*this, *credential, uuid, version, request.body());
	});


	server->route(path+"map/<arg>/delete", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapDelete, &*this, *credential, QJsonArray{uuid});
	});

	server->route(path+"map/", QHttpServerRequest::Method::Delete, [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapDelete, &*this, *credential, QJsonArray{uuid});
	});

	server->route(path+"map/delete", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&TeacherAPI::mapDelete, &*this, *credential, jsonObject->value(QStringLiteral("list")).toArray());
	});

	server->route(path+"map/<arg>/content", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapContent, &*this, *credential, uuid, -1);
	});

	server->route(path+"map/<arg>/draft/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const int &version, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&TeacherAPI::mapContent, &*this, *credential, uuid, version);
	});


#ifdef _COMPAT

	addMap("^group/(\\d+)/campaign/create/*$", this, &TeacherAPI::campaignCreate);

	addMap("^campaign/(\\d+)/*$", this, &TeacherAPI::campaignOne);
	addMap("^campaign/(\\d+)/update/*$", this, &TeacherAPI::campaignUpdate);
	addMap("^campaign/(\\d+)/run/*$", this, &TeacherAPI::campaignRun);
	addMap("^campaign/(\\d+)/finish/*$", this, &TeacherAPI::campaignFinish);
	addMap("^campaign/(\\d+)/delete/*$", this, &TeacherAPI::campaignDeleteOne);
	addMap("^campaign/(\\d+)/duplicate/*$", this, &TeacherAPI::campaignDuplicate);
	addMap("^campaign/(\\d+)/result/*$", this, &TeacherAPI::campaignOneResult);
	addMap("^campaign/(\\d+)/result/([^/]+)/*$", this, &TeacherAPI::campaignUserResult);
	addMap("^campaign/delete/*$", this, &TeacherAPI::campaignDelete);
	addMap("^campaign/(\\d+)/task/*$", this, &TeacherAPI::task);
	addMap("^campaign/(\\d+)/task/create/*$", this, &TeacherAPI::taskCreate);

	addMap("^campaign/(\\d+)/user/*$", this, &TeacherAPI::campaignUser);
	addMap("^campaign/(\\d+)/user/clear/*$", this, &TeacherAPI::campaignUserClear);
	addMap("^campaign/(\\d+)/user/add/([^/]+)/*$", this, &TeacherAPI::campaignUserAddOne);
	addMap("^campaign/(\\d+)/user/add/*$", this, &TeacherAPI::campaignUserAdd);
	addMap("^campaign/(\\d+)/user/remove/([^/]+)/*$", this, &TeacherAPI::campaignUserRemoveOne);
	addMap("^campaign/(\\d+)/user/remove/*$", this, &TeacherAPI::campaignUserRemove);
	addMap("^campaign/(\\d+)/user/copy/*$", this, &TeacherAPI::campaignUserCopy);

	addMap("^task/(\\d+)/*$", this, &TeacherAPI::taskOne);
	addMap("^task/(\\d+)/update/*$", this, &TeacherAPI::taskUpdate);
	addMap("^task/(\\d+)/delete/*$", this, &TeacherAPI::taskDeleteOne);
	addMap("^task/delete/*$", this, &TeacherAPI::taskDelete);

	addMap("^user/peers/*$", this, &TeacherAPI::userPeers);
	addMap("^user/peers/live/*$", this, &TeacherAPI::userPeersLive);

#endif
}



/**
 * @brief TeacherAPI::groups
 * @return
 */

QHttpServerResponse TeacherAPI::groups(const Credential &credential)
{
	LOG_CTRACE("client") << "Get groups";

	LAMBDA_THREAD_BEGIN(credential);

	const QString &username = credential.username();

	QueryBuilder q(db);
	q.addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username);

	LAMBDA_SQL_ASSERT(q.exec());

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
			const auto &l = QueryBuilder::q(db).addQuery("SELECT class.id, name FROM bindGroupClass "
														 "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
														 "WHERE groupid=").addValue(id)
					.execToJsonArray();

			LAMBDA_SQL_ASSERT(l);

			clist = *l;
		}

		obj.insert(QStringLiteral("classList"), clist);

		list.append(obj);
	}

	response = responseResult("list", list);

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::group
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse TeacherAPI::group(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get group" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id);

	const QString &username = credential.username();


	const auto &d = QueryBuilder::q(db).addQuery("SELECT id, name, active FROM studentgroup WHERE owner=").addValue(username)
			.addQuery(" AND id=").addValue(id)
			.execToJsonObject();

	LAMBDA_SQL_ASSERT(d);
	LAMBDA_SQL_ERROR("invalid id", !d->isEmpty());

	QJsonObject data = *d;

	data[QStringLiteral("classList")] =
			QueryBuilder::q(db).addQuery("SELECT class.id, name FROM bindGroupClass "
										 "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
										 "WHERE groupid=").addValue(id)
			.execToJsonArray().value_or(QJsonArray{});

	data[QStringLiteral("userList")] =
			QueryBuilder::q(db).addQuery("SELECT classid, user.username, familyName, givenName, nickname, picture, "
										 "class.name as classname FROM bindGroupStudent "
										 "LEFT JOIN user ON (user.username=bindGroupStudent.username) "
										 "LEFT JOIN class ON (class.id=user.classid) "
										 "WHERE user.active=true AND groupid=").addValue(id)
			.execToJsonArray().value_or(QJsonArray{});

	data[QStringLiteral("memberList")] =
			QueryBuilder::q(db).addQuery("SELECT user.username, familyName, givenName, nickname, picture, "
										 "class.name as classname FROM studentGroupInfo "
										 "LEFT JOIN user ON (user.username=studentGroupInfo.username) "
										 "LEFT JOIN class ON (class.id=user.classid) "
										 "WHERE user.active=true AND studentGroupInfo.id=").addValue(id)
			.execToJsonArray().value_or(QJsonArray{});


	QueryBuilder q(db);
	q.addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
			   "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
			   "description, started, finished, defaultGrade, groupid "
			   "FROM campaign WHERE groupid=").addValue(id);

	LAMBDA_SQL_ASSERT(q.exec());

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

	response = QHttpServerResponse(data);

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::groupCreate
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse TeacherAPI::groupCreate(const Credential &credential, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Create group" << credential.username();

	const QString &name = json.value(QStringLiteral("name")).toString();

	if (name.isEmpty())
		return responseError("missing name");

	LAMBDA_THREAD_BEGIN(credential, json, name);

	const QString &username = credential.username();

	const auto &id = QueryBuilder::q(db)
			.addQuery("INSERT INTO studentgroup(")
			.setFieldPlaceholder()
			.addQuery(") VALUES (")
			.setValuePlaceholder()
			.addQuery(")")
			.addField("name", name)
			.addField("owner", username)
			.addField("active", json.value(QStringLiteral("active")).toBool(true))
			.execInsertAsInt()
			;

	LAMBDA_SQL_ASSERT(id);

	LOG_CDEBUG("client") << "Group created:" << qPrintable(name) << *id;
	response = responseResult("id", *id);

	LAMBDA_THREAD_END;

}



/**
 * @brief TeacherAPI::groupUpdate
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse TeacherAPI::groupUpdate(const Credential &credential, const int &id, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Update group" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id, json);

	const QString &username = credential.username();

	db.transaction();

	LAMBDA_SQL_ERROR_ROLLBACK("invalid group",
							  QueryBuilder::q(db)
							  .addQuery("SELECT * FROM studentgroup WHERE id=").addValue(id)
							  .addQuery(" AND owner=").addValue(username)
							  .execCheckExists());


	QueryBuilder q(db);
	q.addQuery("UPDATE studentgroup SET ").setCombinedPlaceholder();

	if (json.contains(QStringLiteral("name")))
		q.addField("name", json.value(QStringLiteral("name")).toString());

	if (json.contains(QStringLiteral("active")))
		q.addField("active", json.value(QStringLiteral("active")).toBool());

	q.addQuery(" WHERE id=").addValue(id);

	LAMBDA_SQL_ASSERT_ROLLBACK(q.fieldCount() && q.exec());

	db.commit();

	LOG_CDEBUG("client") << "Group modified:" << id;

	response = responseOk();

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::groupDelete
 * @param credential
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::groupDelete(const Credential &credential, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Delete group" << list;

	if (list.isEmpty())
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, list);

	const QString &username = credential.username();

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db).
					  addQuery("DELETE FROM studentgroup WHERE id IN (").addList(list.toVariantList())
					  .addQuery(") AND owner=").addValue(username)
					  .exec());

	LOG_CDEBUG("client") << "Groups deleted:" << list;
	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::groupClassAdd
 * @param credential
 * @param id
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::groupClassAdd(const Credential &credential, const int &id, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Group add class" << id << list;

	if (id <= 0)
		return responseError("invalid id");

	if (list.isEmpty())
		return responseError("invalid classid");

	LAMBDA_THREAD_BEGIN(credential, list, id);

	const QString &username = credential.username();

	db.transaction();

	LAMBDA_SQL_ERROR_ROLLBACK("invalid id",
							  QueryBuilder::q(db)
							  .addQuery("SELECT * FROM studentgroup WHERE id=").addValue(id)
							  .addQuery(" AND owner=").addValue(username)
							  .execCheckExists());

	for (const QJsonValue &v : qAsConst(list)) {
		const int &classid = v.toInt(-1);

		LAMBDA_SQL_ERROR("invalid class", classid > 0) {
			LAMBDA_SQL_ASSERT(QueryBuilder::q(db).addQuery("INSERT OR IGNORE INTO bindGroupClass(")
							  .setFieldPlaceholder()
							  .addQuery(") VALUES (")
							  .setValuePlaceholder()
							  .addQuery(")")
							  .addField("groupid", id)
							  .addField("classid", classid)
							  .exec());
		}
	}

	db.commit();

	LOG_CDEBUG("client") << "Class added to group:" << id << list;
	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::groupClassRemove
 * @param credential
 * @param id
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::groupClassRemove(const Credential &credential, const int &id, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Group remove class" << id << list;

	if (id <= 0)
		return responseError("invalid id");

	if (list.isEmpty())
		return responseError("invalid classid");

	LAMBDA_THREAD_BEGIN(credential, list, id);

	const QString &username = credential.username();

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db).
					  addQuery("DELETE FROM bindGroupClass WHERE groupid=").addValue(id)
					  .addQuery(" AND classid IN (").addList(list.toVariantList())
					  .addQuery(") AND (SELECT owner FROM studentgroup WHERE id=").addValue(id)
					  .addQuery(")=").addValue(username)
					  .exec());

	LOG_CDEBUG("client") << "Class removed from group:" << id << list;
	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::groupClassExclude
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse TeacherAPI::groupClassExclude(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get excluded classes from group" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id);

	CHECK_GROUP(credential.username(), id);

	const auto &list = QueryBuilder::q(db).addQuery("SELECT c.id, c.name FROM class c WHERE c.id NOT IN "
													"(SELECT classid FROM bindGroupClass "
													"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
													"WHERE groupid=").addValue(id).addQuery(")")
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}





/**
 * @brief TeacherAPI::groupUserAdd
 * @param credential
 * @param id
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::groupUserAdd(const Credential &credential, const int &id, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Group add user" << id << list;

	if (id <= 0)
		return responseError("invalid id");

	if (list.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(credential, list, id);

	CHECK_GROUP(credential.username(), id);

	for (const QJsonValue &v : qAsConst(list)) {
		const QString &user = v.toString();

		LAMBDA_SQL_ERROR("invalid user", !user.isEmpty()) {
			LAMBDA_SQL_ASSERT(QueryBuilder::q(db).addQuery("INSERT OR IGNORE INTO bindGroupStudent(")
							  .setFieldPlaceholder()
							  .addQuery(") VALUES (")
							  .setValuePlaceholder()
							  .addQuery(")")
							  .addField("groupid", id)
							  .addField("username", user)
							  .exec());
		}
	}

	LOG_CDEBUG("client") << "Users added to group:" << id << list;
	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::groupUserRemove
 * @param credential
 * @param id
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::groupUserRemove(const Credential &credential, const int &id, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Group remove user" << id << list;

	if (id <= 0)
		return responseError("invalid id");

	if (list.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(credential, list, id);

	const QString &username = credential.username();

	CHECK_GROUP(username, id);

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db).
					  addQuery("DELETE FROM bindGroupStudent WHERE groupid=").addValue(id)
					  .addQuery(" AND username IN (").addList(list.toVariantList())
					  .addQuery(") AND (SELECT owner FROM studentgroup WHERE id=").addValue(id)
					  .addQuery(")=").addValue(username)
					  .exec());


	LOG_CDEBUG("client") << "User removed from group:" << id << list;
	response = responseOk();

	LAMBDA_THREAD_END;
}


/**
 * @brief TeacherAPI::groupUserExclude
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse TeacherAPI::groupUserExclude(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get excluded users from group" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id);

	CHECK_GROUP(credential.username(), id);

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT classid, user.username, familyName, givenName, nickname, picture, "
					  "class.name as classname FROM user "
					  "LEFT JOIN class ON (class.id=user.classid) WHERE user.active=TRUE "
					  "AND user.isTeacher=false AND user.username NOT IN "
					  "(SELECT username FROM bindGroupStudent WHERE groupid=").addValue(id).addQuery(")")
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::groupResult
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse TeacherAPI::groupResult(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get excluded users from group" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id);

	CHECK_GROUP(credential.username(), id);

	QueryBuilder q(db);
	q.addQuery("SELECT id FROM campaign WHERE groupid=").addValue(id);

	LAMBDA_SQL_ASSERT(q.exec());

	QJsonArray list;

	while (q.sqlQuery().next()) {
		const int &id = q.value("id").toInt();

		const auto &resultList = QueryBuilder::q(db)
				.addQuery("SELECT studentGroupInfo.username AS username, score.xp AS resultXP, campaignResult.gradeid AS resultGrade "
						  "FROM studentGroupInfo "
						  "LEFT JOIN campaignResult ON (campaignResult.campaignid=").addValue(id)
				.addQuery(" AND campaignResult.username=studentGroupInfo.username) "
						  "LEFT JOIN score ON (campaignResult.scoreid=score.id) "
						  "WHERE active=true AND studentGroupInfo.id=(SELECT groupid FROM campaign WHERE campaign.id=").addValue(id)
				.addQuery(")")
				.execToJsonArray();

		LAMBDA_SQL_ASSERT(resultList);

		QJsonObject obj;
		obj[QStringLiteral("campaignid")] = id;
		obj[QStringLiteral("resultList")] = *resultList;
		list.append(obj);
	}

	response = QHttpServerResponse(list);

	LAMBDA_THREAD_END;
}





/**
 * @brief TeacherAPI::groupUserResult
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse TeacherAPI::groupUserResult(const Credential &credential, const int &id, const QString &username, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Get user result" << id << username;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id, json);

	CHECK_GROUP(credential.username(), id);

	int offset = json.value(QStringLiteral("offset")).toInt(0);
	int limit = json.value(QStringLiteral("limit")).toInt(DEFAULT_LIMIT);

	const auto &list = TeacherAPI::_groupUserGameResult(this, id, credential.username(), limit, offset);

	LAMBDA_SQL_ASSERT(list);

	response = QHttpServerResponse(QJsonObject{
									   { QStringLiteral("list"), *list },
									   { QStringLiteral("limit"), limit },
									   { QStringLiteral("offset"), offset },
								   });

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::groupGameLog
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse TeacherAPI::groupGameLog(const Credential &credential, const int &id, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Get group game log" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id, json);

	CHECK_GROUP(credential.username(), id);

	int offset = json.value(QStringLiteral("offset")).toInt(0);
	int limit = json.value(QStringLiteral("limit")).toInt(DEFAULT_LIMIT);

	const auto &list = TeacherAPI::_groupGameResult(this, id, limit, offset);

	LAMBDA_SQL_ASSERT(list);

	response = QHttpServerResponse(QJsonObject{
									   { QStringLiteral("list"), *list },
									   { QStringLiteral("limit"), limit },
									   { QStringLiteral("offset"), offset },
								   });

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::map
 * @param credential
 * @param uuid
 * @return
 */

QHttpServerResponse TeacherAPI::map(const Credential &credential, const QString &uuid)
{
	LOG_CTRACE("client") << "Get map" << uuid;

	LAMBDA_THREAD_BEGIN(credential, uuid);

	const QString &username = credential.username();

	QueryBuilder q(db);
	q.addQuery("SELECT mapdb.map.uuid, name, version, md5, CAST(strftime('%s', lastModified) AS INTEGER) AS lastModified, "
			   "COALESCE((SELECT version FROM mapdb.draft WHERE mapdb.draft.uuid=mapdb.map.uuid),-1) AS draftVersion, "
			   "mapdb.cache.data AS cache, length(mapdb.map.data) as size, lastEditor "
			   "FROM mapdb.map LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
			   "WHERE mapdb.map.uuid IN "
			   "(SELECT mapuuid FROM mapOwner WHERE username=").addValue(username).addQuery(")");

	if (!uuid.isEmpty())
		q.addQuery(" AND mapdb.map.uuid=").addValue(uuid);

	const auto &list = q.execToJsonArray({
											 { QStringLiteral("cache"), [](const QVariant &v) {
												   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
											   } }
										 });

	LAMBDA_SQL_ASSERT(list);

	if (uuid.isEmpty())
		response = responseResult("list", *list);
	else if (list->isEmpty())
		response = responseError("not found");
	else
		response = QHttpServerResponse(list->at(0).toObject());

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::mapCreate
 * @param credential
 * @param json
 * @param name
 * @return
 */

QHttpServerResponse TeacherAPI::mapCreate(const Credential &credential, const QByteArray &body, const QString &name)
{
	LOG_CTRACE("client") << "Map create" << name;

	QScopedPointer<GameMap> map(GameMap::fromBinaryData(qUncompress(body)));

	if (!map)
		return responseError("invalid map");

	const QString &uuid = map->uuid();
	const QString &cache = mapCacheString(map.get());

	LAMBDA_THREAD_BEGIN(credential, name, uuid, cache, body);

	const QString &md5 = mapMd5(body);

	db.transaction();

	LAMBDA_SQL_ERROR_ROLLBACK("map already exists", !QueryBuilder::q(db).addQuery("SELECT uuid FROM mapdb.map WHERE uuid=").addValue(uuid).execCheckExists());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("INSERT INTO mapdb.map(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
							   .addField("uuid", uuid)
							   .addField("name", name.isEmpty() ? uuid : name)
							   .addField("md5", md5)
							   .addField("data", body)
							   .addField("lastEditor", credential.username())
							   .exec());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("INSERT OR REPLACE INTO mapdb.cache(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
							   .addField("uuid", uuid)
							   .addField("data", cache)
							   .exec());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("INSERT INTO mapOwner(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
							   .addField("mapuuid", uuid)
							   .addField("username", credential.username())
							   .exec());

	db.commit();

	response = responseOk(QJsonObject{{QStringLiteral("uuid"), uuid}});

	LAMBDA_THREAD_END;
}





/**
 * @brief TeacherAPI::mapUpdate
 * @param credential
 * @param uuid
 * @param json
 * @return
 */

QHttpServerResponse TeacherAPI::mapUpdate(const Credential &credential, const QString &uuid, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Update map" << uuid;

	LAMBDA_THREAD_BEGIN(credential, uuid, json);

	CHECK_MAP(credential.username(), uuid);

	QueryBuilder q(db);
	q.addQuery("UPDATE mapdb.map SET ").setCombinedPlaceholder();

	if (json.contains(QStringLiteral("name")))
		q.addField("name", json.value(QStringLiteral("name")).toString());

	q.addQuery(" WHERE uuid=").addValue(uuid);

	LAMBDA_SQL_ASSERT(q.fieldCount() && q.exec());

	LOG_CDEBUG("client") << "Map modified:" << uuid;

	response = responseOk();

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::mapPublish
 * @param credential
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::mapPublish(const Credential &credential, const QString &uuid, const int &version)
{
	LOG_CTRACE("client") << "Publish map" << uuid << "version" << version;

	LAMBDA_THREAD_BEGIN(credential, uuid, version);

	CHECK_MAP(credential.username(), uuid);

	db.transaction();

	QueryBuilder q(db);

	q.addQuery("SELECT data FROM mapdb.draft WHERE uuid=").addValue(uuid).addQuery(" AND version=").addValue(version);

	LAMBDA_SQL_ASSERT_ROLLBACK(q.exec());

	LAMBDA_SQL_ERROR_ROLLBACK("invalid uuid/version", q.sqlQuery().first());

	const QByteArray &b = q.sqlQuery().value(QStringLiteral("data")).toByteArray();

	QScopedPointer<GameMap> map(GameMap::fromBinaryData(qUncompress(b)));

	LAMBDA_SQL_ERROR_ROLLBACK("invalid map", map);

	QString mapuuid = map->uuid();

	LAMBDA_SQL_ERROR_ROLLBACK("map uuid mismatch", mapuuid == uuid);

	const QString &md5 = mapMd5(b);
	const QString &cache = mapCacheString(map.get());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("UPDATE mapdb.map SET version=version+1, lastModified=datetime('now'), ").setCombinedPlaceholder()
							   .addField("md5", md5)
							   .addField("data", b)
							   .addField("lastEditor", credential.username())
							   .addQuery(" WHERE uuid=").addValue(uuid)
							   .exec());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("INSERT OR REPLACE INTO mapdb.cache(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
							   .addField("uuid", uuid)
							   .addField("data", cache)
							   .exec());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("DELETE FROM mapdb.draft WHERE uuid=").addValue(uuid)
							   .exec());

	db.commit();

	LOG_CDEBUG("client") << "Map published:" << uuid;

	response = responseOk();

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::mapDelete
 * @param credential
 * @param list
 * @return
 */

QHttpServerResponse TeacherAPI::mapDelete(const Credential &credential, const QJsonArray &list)
{
	LOG_CTRACE("client") << "Delete map:" << list;

	if (list.isEmpty())
		return responseError("invalid uuid");

	LAMBDA_THREAD_BEGIN(credential, list);

	db.transaction();

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db).
							   addQuery("DELETE FROM mapdb.map WHERE uuid IN (SELECT mapuuid FROM mapOwner WHERE mapuuid IN (").addList(list.toVariantList())
							   .addQuery(") AND username=").addValue(credential.username()).addQuery(")")
							   .exec());

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("DELETE FROM mapOwner WHERE mapuuid IN (").addList(list.toVariantList())
							   .addQuery(") AND mapuuid IN (SELECT mapuuid FROM mapOwner WHERE username=").addValue(credential.username())
							   .addQuery(")")
							   .exec());

	db.commit();

	LOG_CDEBUG("client") << "Map deleted:" << list;

	response = responseOk();

	LAMBDA_THREAD_END;
}





/**
 * @brief TeacherAPI::mapDeleteDraft
 * @param credential
 * @param uuid
 * @param version
 * @return
 */

QHttpServerResponse TeacherAPI::mapDeleteDraft(const Credential &credential, const QString &uuid, const int &version)
{
	LOG_CTRACE("client") << "Delete map draft:" << uuid << "version" << version;

	LAMBDA_THREAD_BEGIN(credential, uuid, version);

	CHECK_MAP(credential.username(), uuid);

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db).addQuery("DELETE FROM mapdb.draft WHERE uuid=").addValue(uuid).addQuery(" AND version=").addValue(version).exec());

	LOG_CTRACE("client") << "Map draft deleted:" << uuid << version;

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief TeacherAPI::mapUpload
 * @param credential
 * @param uuid
 * @param version
 * @param body
 * @return
 */

QHttpServerResponse TeacherAPI::mapUpload(const Credential &credential, const QString &uuid, const int &version, const QByteArray &body)
{
	LOG_CTRACE("client") << "Update map" << uuid;

	QScopedPointer<GameMap> map(GameMap::fromBinaryData(qUncompress(body)));

	if (!map)
		return responseError("invalid map");

	if (map->uuid() != uuid)
		return responseError("map uuid mismatch");

	LAMBDA_THREAD_BEGIN(credential, uuid, version, body);

	CHECK_MAP(credential.username(), uuid);

	db.transaction();

	if (version == 0) {
		LAMBDA_SQL_ERROR_ROLLBACK("version mismatch",
								  !QueryBuilder::q(db).addQuery("SELECT version FROM mapdb.draft WHERE uuid=").addValue(uuid)
								  .execCheckExists());
	} else {
		LAMBDA_SQL_ERROR_ROLLBACK("version mismatch",
								  QueryBuilder::q(db).addQuery("SELECT version FROM mapdb.draft WHERE uuid=").addValue(uuid)
								  .addQuery(" AND version=").addValue(version)
								  .execCheckExists());
	}


	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("INSERT OR REPLACE INTO mapdb.draft(uuid, version, lastModified, data) VALUES (")
							   .addValue(uuid)
							   .addValue(version+1)
							   .addQuery(", datetime('now'), ")
							   .addValue(body)
							   .addQuery(")")
							   .exec());

	db.commit();

	LOG_CDEBUG("client") << "Map modified:" << uuid;

	response = responseOk(QJsonObject{{QStringLiteral("version"), version}});

	LAMBDA_THREAD_END;
}




/**
 * @brief TeacherAPI::mapContent
 * @param credential
 * @param uuid
 * @param draftVersion
 * @return
 */

QHttpServerResponse TeacherAPI::mapContent(const Credential &credential, const QString &uuid, const int &draftVersion)
{
	LOG_CTRACE("client") << "Get map content:" << uuid << "version" << draftVersion;

	LAMBDA_THREAD_BEGIN(credential, uuid, draftVersion);

	QueryBuilder q(db);

	q.addQuery("SELECT data FROM ")
			.addQuery(draftVersion > 0 ? "mapdb.draft" : "mapdb.map")
			.addQuery(" WHERE uuid IN "
					  "(SELECT mapuuid FROM mapOwner WHERE username=").addValue(credential.username())
			.addQuery(") AND uuid=").addValue(uuid);

	if (draftVersion > 0)
		q.addQuery(" AND version=").addValue(draftVersion);

	LAMBDA_SQL_ASSERT(q.exec());

	if (q.sqlQuery().first()) {
		const QByteArray &b = q.sqlQuery().value(QStringLiteral("data")).toByteArray();
		response = QHttpServerResponse(b);
	} else
		response = responseError("not found");

	LAMBDA_THREAD_END;
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
		QJsonArray levels;

		for (GameMapMissionLevel *ml : m->levels()) {
			levels.append(QJsonObject {
							  { QStringLiteral("l"), ml->level() },
							  { QStringLiteral("dm"), ml->canDeathmatch() }
						  });
		}

		mList.append(QJsonObject{
						 { QStringLiteral("uuid"), m->uuid() },
						 { QStringLiteral("name"), m->name() },
						 { QStringLiteral("medal"), m->medalImage() },
						 { QStringLiteral("levels"), levels }
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



#ifdef _COMPAT





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
						  "description, started, finished, defaultGrade, groupid "
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
 * @brief TeacherAPI::campaignOneResult
 * @param match
 * @param response
 */

void TeacherAPI::campaignOneResult(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		QJsonObject obj = QueryBuilder::q(db)
				.addQuery("SELECT id, finished FROM campaign WHERE id=").addValue(id)
				.addQuery(" AND groupid IN (SELECT id FROM studentgroup WHERE owner=").addValue(username).addQuery(")")
				.execToJsonObject(&err);

		if (err)
			return responseErrorSql(response);

		if (obj.isEmpty())
			return responseError(response, "not found");


		const bool &finished = obj.value(QStringLiteral("finished")).toVariant().toBool();

		QJsonArray resultList;

		QueryBuilder q(db);
		q.addQuery("WITH studentList(username, campaignid) AS (SELECT username, campaignid FROM campaignStudent) "
				   "SELECT studentGroupInfo.username AS username, score.xp AS resultXP, campaignResult.gradeid AS resultGrade, "
				   "(SELECT NOT EXISTS(SELECT * FROM studentList WHERE studentList.campaignid=").addValue(id)
				.addQuery(") OR studentGroupInfo.username IN (SELECT username FROM studentList WHERE studentList.campaignid=").addValue(id)
				.addQuery(")) AS included "
						  "FROM studentGroupInfo "
						  "LEFT JOIN campaignResult ON (campaignResult.campaignid=").addValue(id)
				.addQuery(" AND campaignResult.username=studentGroupInfo.username) "
						  "LEFT JOIN score ON (campaignResult.scoreid=score.id) "
						  "WHERE active=true AND studentGroupInfo.id=(SELECT groupid FROM campaign WHERE campaign.id=").addValue(id)
				.addQuery(")");

		if (!q.exec())
			return responseErrorSql(response);


		while (q.sqlQuery().next()) {
			const QString &username = q.value("username").toString();

			const TeacherAPI::UserCampaignResult &result = TeacherAPI::_campaignUserResult(this, id, finished, username, false, &err);

			if (err)
				return responseErrorSql(response);

			int xp = 0;
			int grade = 0;

			if (finished) {
				xp = q.value("resultXP").toInt();
				grade = q.value("resultGrade").toInt();
			} else {
				xp = result.xp;
				grade = result.grade;
			}

			QJsonObject obj;

			obj[QStringLiteral("username")] = username;
			obj[QStringLiteral("included")] = q.value("included").toString();
			obj[QStringLiteral("resultXP")] = xp > 0 ? xp : QJsonValue::Null;
			obj[QStringLiteral("resultGrade")] = grade > 0 ? grade : QJsonValue::Null;
			obj[QStringLiteral("taskList")] = result.tasks;

			resultList.append(obj);
		}

		responseAnswer(response, "list", resultList);
	});
}



/**
 * @brief TeacherAPI::campaignUserResult
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignUserResult(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();
	const QString &username = match.captured(2);

	if (username.isEmpty())
		return responseError(response, "empty username");

	databaseMainWorker()->execInThread([this, response, id, data, username]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		bool err = false;

		if (!QueryBuilder::q(db)
				.addQuery("SELECT id FROM campaign WHERE id=").addValue(id)
				.addQuery(" AND groupid IN (SELECT id FROM studentgroup WHERE owner=").addValue(m_credential.username()).addQuery(")")
				.execCheckExists(&err))
			return responseError(response, "invalid campaign");

		if (err)
			return responseErrorSql(response);

		int offset = data.value(QStringLiteral("offset")).toInt(0);
		int limit = data.value(QStringLiteral("limit")).toInt(DEFAULT_LIMIT);

		const QJsonArray &list = TeacherAPI::_campaignUserGameResult(this, id, username, limit, offset, &err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, QJsonObject{
						   { QStringLiteral("list"), list },
						   { QStringLiteral("limit"), limit },
						   { QStringLiteral("offset"), offset },
					   });
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
 * @brief TeacherAPI::campaignDuplicate
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignDuplicate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
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

		db.transaction();

		QVector<int> myGroupIds;

		QueryBuilder q2(db);
		q2.addQuery("SELECT id FROM studentgroup WHERE owner=").addValue(username);

		if (!q2.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		while (q2.sqlQuery().next())
			myGroupIds.append(q2.value("id", -1).toInt());

		QVector<int> targetIds;

		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();

		foreach (const QJsonValue &v, list) {
			int id = v.toInt();
			if (!targetIds.contains(id))
				targetIds.append(id);
		}

		if (data.contains(QStringLiteral("id"))) {
			int id = data.value(QStringLiteral("id")).toInt();
			if (!targetIds.contains(id))
				targetIds.append(id);
		}

		if (targetIds.isEmpty()) {
			db.rollback();
			return responseError(response, "missing target id");
		}


		QJsonArray newIdList;

		foreach (const int target, targetIds) {
			if (!myGroupIds.contains(target)) {
				LOG_CWARNING("client") << "Duplicate campaign, skip id (owner error)" << id << "->" << target;
				continue;
			}

			bool err = false;

			const int newId = QueryBuilder::q(db)
					.addQuery("INSERT INTO campaign (groupid, endtime, description, defaultGrade) "
							  "SELECT ").addValue(target)
					.addQuery(", endtime, description, defaultGrade FROM campaign WHERE id=").addValue(id)
					.execInsertAsInt(&err);

			if (err) {
				db.rollback();
				return responseErrorSql(response);
			}

			if (!QueryBuilder::q(db)
					.addQuery("INSERT INTO task (campaignid, gradeid, xp, required, mapuuid, criterion) "
							  "SELECT ").addValue(newId)
					.addQuery(", gradeid, xp,required, mapuuid, criterion FROM task WHERE campaignid=").addValue(id)
					.exec()) {
				db.rollback();
				return responseErrorSql(response);
			}

			newIdList.append(newId);
		}

		db.commit();

		LOG_CDEBUG("client") << "Campaign duplicated:" << id << "->" << newIdList;
		responseAnswer(response, "list", newIdList);
	});
}



/**
 * @brief TeacherAPI::campaignStudent
 * @param match
 * @param response
 */

void TeacherAPI::campaignUser(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([id, response, this]() {
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

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT username FROM campaignStudent WHERE campaignid=").addValue(id)
				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}



/**
 * @brief TeacherAPI::campaignUserClear
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignUserClear(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	databaseMainWorker()->execInThread([id, response, this]() {
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

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM campaignStudent WHERE campaignid=").addValue(id)
				.exec()) {
			LOG_CWARNING("client") << "All user remove from campaign error:" << id;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "All user removed from campaign:" << id;
		responseAnswerOk(response);
	});
}


/**
 * @brief TeacherAPI::campaignUserAdd
 * @param id
 * @param list
 * @param response
 */

void TeacherAPI::campaignUserAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (id <= 0)
		return responseError(response, "invalid id");

	if (list.isEmpty())
		return responseError(response, "invalid user");

	databaseMainWorker()->execInThread([id, list, response, this]() {
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

		db.transaction();

		foreach (const QJsonValue &v, list) {
			const QString &user = v.toString();

			if (!user.isEmpty()) {
				if (!QueryBuilder::q(db).addQuery("INSERT OR IGNORE INTO campaignStudent(")
						.setFieldPlaceholder()
						.addQuery(") VALUES (")
						.setValuePlaceholder()
						.addQuery(")")
						.addField("campaignid", id)
						.addField("username", user)
						.exec()) {
					db.rollback();
					return responseErrorSql(response);
				}
			} else {
				db.rollback();
				return responseError(response, "invalid user");
			}
		}

		db.commit();

		LOG_CDEBUG("client") << "User added to campaign:" << id << list;
		responseAnswerOk(response);
	});
}


/**
 * @brief TeacherAPI::campaignUserRemove
 * @param id
 * @param list
 * @param response
 */

void TeacherAPI::campaignUserRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (id <= 0)
		return responseError(response, "invalid id");

	if (list.isEmpty())
		return responseError(response, "invalid user");

	databaseMainWorker()->execInThread([id, list, response, this]() {
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

		db.transaction();

		if (!QueryBuilder::q(db).
				addQuery("DELETE FROM campaignStudent WHERE campaignid=").addValue(id)
				.addQuery(" AND username IN (").addList(list.toVariantList())
				.addQuery(")")
				.exec()) {
			LOG_CWARNING("client") << "User remove from campaign error:" << id << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "User removed from campaign:" << id << list;
		responseAnswerOk(response);
	});
}



/**
 * @brief TeacherAPI::campaignUserCopy
 * @param match
 * @param data
 * @param response
 */

void TeacherAPI::campaignUserCopy(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	if (id <= 0)
		return responseError(response, "invalid id");

	const int &fromId = data.value(QStringLiteral("from")).toInt(-1);

	if (fromId <= 0)
		return responseError(response, "invalid from-campaign id");

	QVariantList grades;

	if (data.contains(QStringLiteral("gradeList"))) {
		const QJsonArray &list = data.value(QStringLiteral("gradeList")).toArray();

		foreach (const QJsonValue &v, list) {
			int g = v.toInt(-1);

			if (g >= 0 && !grades.contains(g))
				grades.append(g);
		}
	}

	const int g = data.value(QStringLiteral("grade")).toInt(-1);

	if (g >= 0 && !grades.contains(g))
		grades.append(g);


	databaseMainWorker()->execInThread([id, fromId, grades, response, this]() {
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

		db.transaction();

		if (grades.contains(0)) {
			if (!QueryBuilder::q(db).
					addQuery("INSERT OR IGNORE INTO campaignStudent(campaignid, username) "
							 "SELECT ").addValue(id)
					.addQuery(", username FROM campaignResult WHERE campaignid=").addValue(fromId)
					.addQuery(" AND gradeid IS NOT NULL")
					.exec()) {
				LOG_CWARNING("client") << "User copy from campaign error:" << id << grades;
				db.rollback();
				return responseErrorSql(response);
			}
		}

		if (!QueryBuilder::q(db).
				addQuery("INSERT OR IGNORE INTO campaignStudent(campaignid, username) "
						 "SELECT ").addValue(id)
				.addQuery(", username FROM campaignResult WHERE campaignid=").addValue(fromId)
				.addQuery(" AND gradeid IN (").addList(grades)
				.addQuery(")")
				.exec()) {
			LOG_CWARNING("client") << "User copy from campaign error:" << id << grades;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CDEBUG("client") << "User copied to campaign:" << id << grades;
		responseAnswerOk(response);
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
 * @brief TeacherAPI::userPeers
 * @param response
 */

void TeacherAPI::userPeers(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	responseAnswer(response, "list", PeerUser::toJson(&(m_service->peerUser())));
}


/**
 * @brief TeacherAPI::userPeersLive
 * @param response
 */

void TeacherAPI::userPeersLive(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	HttpConnection *conn = qobject_cast<HttpConnection*>(response->parent());
	EventStream *stream = new EventStream(EventStream::EventStreamPeerUsers, conn);
	conn->setEventStream(stream);
	m_service->addEventStream(stream);

	response->setStatus(HttpStatus::Ok);

	stream->trigger();
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
		else if (module == QLatin1String("mapmission"))
			success = _evaluateCriterionMapMission(api, campaign, criterion, map, username, &e);

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

	if (xp >= criterion.value(QStringLiteral("num")).toInt())
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



/**
 * @brief TeacherAPI::_evaluateCriterionMapMission
 * @param api
 * @param campaign
 * @param criterion
 * @param map
 * @param username
 * @param err
 * @return
 */

bool TeacherAPI::_evaluateCriterionMapMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion,
											  const QString &map, const QString &username, bool *err)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	bool e = false;

	int num = QueryBuilder::q(db)
			.addQuery("WITH s AS (SELECT DISTINCT missionid FROM game WHERE success=true AND username=").addValue(username)
			.addQuery(" AND campaignid=").addValue(campaign)
			.addQuery(" AND mapid=").addValue(map)
			.addQuery(") SELECT COUNT(*) AS num FROM s")
			.execToValue("num", &e).toInt()
			;

	if (e) {
		if (err) *err = true;
		return false;
	}

	if (num >= criterion.value(QStringLiteral("num")).toInt())
		return true;

	return false;
}


#endif




/**
 * @brief TeacherAPI::_campaignUserResutl
 * @param api
 * @param campaign
 * @param username
 * @return
 */

std::optional<TeacherAPI::UserCampaignResult> TeacherAPI::_campaignUserResult(const AbstractAPI *api, const int &campaign, const bool &finished,
																			  const QString &username, const bool &withCriterion)
{
	Q_ASSERT(api);

	return _campaignUserResult(api->databaseMain(), campaign, finished, username, withCriterion);
}



/**
 * @brief TeacherAPI::_campaignUserResult
 * @param dbMain
 * @param campaign
 * @param finished
 * @param username
 * @param withCriterion
 * @param err
 * @return
 */

std::optional<TeacherAPI::UserCampaignResult> TeacherAPI::_campaignUserResult(const DatabaseMain *dbMain, const int &campaign, const bool &finished,
																			  const QString &username, const bool &withCriterion)
{
	Q_ASSERT(dbMain);

	UserCampaignResult result;

	QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

	QMutexLocker(dbMain->mutex());

	std::optional<QJsonArray> list;

	if (withCriterion) {
		list = QueryBuilder::q(db)
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
								 });
	} else {
		list = QueryBuilder::q(db)
				.addQuery("SELECT task.id, gradeid, grade.value AS gradeValue, xp, required, "
						  "(taskSuccess.username IS NOT NULL) AS success FROM task "
						  "LEFT JOIN grade ON (grade.id=task.gradeid) "
						  "LEFT JOIN taskSuccess ON (taskSuccess.taskid=task.id AND taskSuccess.username=")
				.addValue(username)
				.addQuery(") WHERE campaignid=").addValue(campaign)
				.execToJsonArray();
	}

	if (!list)
		return std::nullopt;


	result.tasks = std::move(*list);



	// Finished campaign

	if (finished)
		return result;




	// Default grade

	QueryBuilder q(db);
	q.addQuery("SELECT defaultGrade, grade.value AS value FROM campaign LEFT JOIN grade ON (grade.id=campaign.defaultGrade) WHERE campaign.id=").addValue(campaign);

	if (!q.exec())
		return std::nullopt;

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

	for (const auto &it : qAsConst(*list)) {
		const QJsonObject &o = it.toObject();

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

	return result;
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
							  })
			.value_or(QJsonObject{});
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
							 })
			.value_or(QJsonArray{});
}









/**
 * @brief TeacherAPI::_campaignUserGameResult
 * @param api
 * @param campaign
 * @param username
 * @param limit
 * @param offset
 * @return
 */

std::optional<QJsonArray> TeacherAPI::_campaignUserGameResult(const AbstractAPI *api, const int &campaign, const QString &username,
															  const int &limit, const int &offset)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	return QueryBuilder::q(db)
			.addQuery("WITH t AS (SELECT game.id as id, CAST(strftime('%s', game.timestamp) AS INTEGER) AS timestamp, mapid, missionid, "
					  "level, mode, deathmatch, success, duration, xp "
					  "FROM game LEFT JOIN score ON (score.id=game.scoreid) "
					  "WHERE campaignid=").addValue(campaign)
			.addQuery(" AND game.username=").addValue(username)
			.addQuery(") SELECT * FROM t WHERE id NOT IN (SELECT id FROM t ORDER BY timestamp DESC, id DESC LIMIT ").addValue(offset)
			.addQuery(") ORDER BY timestamp DESC, id DESC LIMIT ").addValue(limit)
			.execToJsonArray();
}




/**
 * @brief TeacherAPI::_groupUserGameResult
 * @param api
 * @param group
 * @param username
 * @param limit
 * @param offset
 * @return
 */

std::optional<QJsonArray> TeacherAPI::_groupUserGameResult(const AbstractAPI *api, const int &group, const QString &username,
														   const int &limit, const int &offset)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	return QueryBuilder::q(db)
			.addQuery("WITH t AS (SELECT game.id as id, CAST(strftime('%s', game.timestamp) AS INTEGER) AS timestamp, mapid, missionid, "
					  "level, mode, deathmatch, success, duration, xp "
					  "FROM game LEFT JOIN score ON (score.id=game.scoreid) "
					  "WHERE (campaignid IS NULL OR campaignid IN (SELECT id FROM campaign WHERE groupid=").addValue(group)
			.addQuery(")) AND game.username=").addValue(username)
			.addQuery(") SELECT * FROM t WHERE id NOT IN (SELECT id FROM t ORDER BY timestamp DESC, id DESC LIMIT ").addValue(offset)
			.addQuery(") ORDER BY timestamp DESC, id DESC LIMIT ").addValue(limit)
			.execToJsonArray();
}





/**
 * @brief TeacherAPI::_groupGameResult
 * @param api
 * @param group
 * @param limit
 * @param offset
 * @return
 */

std::optional<QJsonArray> TeacherAPI::_groupGameResult(const AbstractAPI *api, const int &group, const int &limit, const int &offset)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker(api->databaseMain()->mutex());

	return QueryBuilder::q(db)
			.addQuery("WITH t AS (SELECT game.id as id, CAST(strftime('%s', game.timestamp) AS INTEGER) AS timestamp, mapid, missionid, "
					  "level, mode, deathmatch, success, duration, xp, game.username as username, familyName, givenName "
					  "FROM game LEFT JOIN score ON (score.id=game.scoreid) "
					  "LEFT JOIN user ON (user.username=game.username) "
					  "WHERE (campaignid IS NULL OR campaignid IN (SELECT id FROM campaign WHERE groupid=").addValue(group)
			.addQuery("))) SELECT * FROM t WHERE id NOT IN (SELECT id FROM t ORDER BY timestamp DESC, id DESC LIMIT ").addValue(offset)
			.addQuery(") ORDER BY timestamp DESC, id DESC LIMIT ").addValue(limit)
			.execToJsonArray();
}


