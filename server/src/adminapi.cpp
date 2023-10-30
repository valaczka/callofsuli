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
//#include "oauth2authenticator.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "serverservice.h"
//#include "teacherapi.h"

#define _SQL_QUERY_USERS "SELECT user.username, familyName, givenName, active, user.classid, class.name as className, " \
	"isTeacher, isAdmin, isPanel, nickname, character, picture," \
	"oauth, oauthData " \
	"FROM user LEFT JOIN auth ON (auth.username=user.username) " \
	"LEFT JOIN class ON (class.id=user.classid)"


/**
 * @brief AdminAPI::AdminAPI
 * @param service
 */

AdminAPI::AdminAPI(Handler *handler, ServerService *service)
	: AbstractAPI("admin", handler, service)
{
	auto server = m_handler->httpServer();

	Q_ASSERT(server);

	m_validateRole = Credential::Admin;

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	(*server)->route(path+"user", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classUsers, &*this, 0);
	});

	(*server)->route(path+"user/noclass", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classUsers, &*this, -1);
	});

	(*server)->route(path+"user/create", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::userCreate, &*this, *jsonObject);
	});

	(*server)->route(path+"user", QHttpServerRequest::Method::Put, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::userCreate, &*this, *jsonObject);
	});




	(*server)->route(path+"class", QHttpServerRequest::Method::Put, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::classCreate, &*this, *jsonObject);
	});

	(*server)->route(path+"class/", QHttpServerRequest::Method::Delete, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classDelete, &*this, QJsonArray{id});
	});

	(*server)->route(path+"class/create", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::classCreate, &*this, *jsonObject);
	});

	(*server)->route(path+"class/<arg>/users", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classUsers, &*this, id);
	});

	(*server)->route(path+"class/<arg>/update", QHttpServerRequest::Method::Post, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::classUpdate, &*this, id, *jsonObject);
	});

	(*server)->route(path+"class/code", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classCode, &*this, -2);
	});

	(*server)->route(path+"class/nocode", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classCode, &*this, -1);
	});

	(*server)->route(path+"class/<arg>/code", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classCode, &*this, id);
	});

	(*server)->route(path+"class/<arg>/updateCode", QHttpServerRequest::Method::Post, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::classUpdateCode, &*this, id, *jsonObject);
	});

	(*server)->route(path+"class/<arg>/delete", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&AdminAPI::classDelete, &*this, QJsonArray{id});
	});

	(*server)->route(path+"class/delete", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&AdminAPI::classDelete, &*this, jsonObject->value(QStringLiteral("list")).toArray());
	});


}



/**
 * @brief AdminAPI::classUsers
 * @param id
 * @return
 */

QHttpServerResponse AdminAPI::classUsers(const int &id)
{
	LOG_CTRACE("client") << "Get class users" << id;

	LAMBDA_THREAD_BEGIN(id);

	QueryBuilder q(db);

	q.addQuery(_SQL_QUERY_USERS);

	if (id > 0)
		q.addQuery("WHERE user.classid=").addValue(id);
	else if (id < 0)
		q.addQuery("WHERE user.classid=NULL");

	const auto &list = q.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}





/**
 * @brief AdminAPI::classCreate
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::classCreate(const QJsonObject &json)
{
	const QString &name = json.value(QStringLiteral("name")).toString();

	LOG_CTRACE("client") << "Class create" << name;

	if (name.isEmpty())
		return responseError("missing name");


	LAMBDA_THREAD_BEGIN(name);

	const auto &id = _classCreate(this, std::move(Class(name)));

	LAMBDA_SQL_ASSERT(id);

	LOG_CINFO("client") << "Class created:" << qPrintable(name) << *id;

	response = QHttpServerResponse(QJsonObject {
									   { QStringLiteral("id"), *id }
								   });

	LAMBDA_THREAD_END;
}






/**
 * @brief AdminAPI::classUpdate
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::classUpdate(const int &id, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Class update" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(id, json);

	db.transaction();

	QueryBuilder q(db);
	q.addQuery("UPDATE class SET ").setCombinedPlaceholder();

	if (json.contains(QStringLiteral("name")))
		q.addField("name", json.value(QStringLiteral("name")).toString());

	q.addQuery(" WHERE id=").addValue(id);

	LAMBDA_SQL_ASSERT_ROLLBACK(q.fieldCount() && q.exec());

	db.commit();

	LOG_CINFO("client") << "Class modified:" << id;

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief AdminAPI::classCode
 * @param id
 * @return
 */

QHttpServerResponse AdminAPI::classCode(const int &id)
{
	LOG_CTRACE("client") << "Get class code" << id;

	LAMBDA_THREAD_BEGIN(id);

	QueryBuilder q(db);
	q.addQuery("SELECT classid, code FROM classCode ");

	if (id == -1)
		q.addQuery("WHERE classid IS NULL");
	else if (id > 0)
		q.addQuery("WHERE classid=").addValue(id);

	const std::optional<QJsonArray> &list = q.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	if (id == -1)
		response = responseResult("list", *list);
	else if (list->size() != 1)
		response = responseError("not found");
	else {
		response = QHttpServerResponse(list->at(0).toObject());
	}

	LAMBDA_THREAD_END;
}


/**
 * @brief AdminAPI::classUpdateCode
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::classUpdateCode(const int &id, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Class update code" << id;

	LAMBDA_THREAD_BEGIN(id, json);

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
					  .addQuery("UPDATE classCode SET code=").addValue(json.value(QStringLiteral("code")).toString())
					  .addQuery(" WHERE classid=").addValue(id <= 0 ? QVariant(QMetaType::fromType<int>()) : id)
					  .exec());

	LOG_CDEBUG("client") << "Class code modified:" << id;

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief AdminAPI::classDelete
 * @param idList
 * @return
 */

QHttpServerResponse AdminAPI::classDelete(const QJsonArray &idList)
{
	LOG_CTRACE("client") << "Class delete" << idList;

	if (idList.isEmpty())
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(idList);

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
					  .addQuery("DELETE FROM class WHERE id IN (")
					  .addList(idList.toVariantList()).addQuery(")")
					  .exec());

	LOG_CDEBUG("client") << "Class removed:" << idList;

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief AdminAPI::user
 * @param username
 * @return
 */

QHttpServerResponse AdminAPI::user(const QString &username)
{
	LOG_CTRACE("client") << "Get user" << username;

	if (username.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(username);

	const auto &obj = QueryBuilder::q(db)
			.addQuery(_SQL_QUERY_USERS)
			.addQuery("WHERE user.username=").addValue(username)
			.execToJsonObject();

	LAMBDA_SQL_ASSERT(obj);

	if (obj->isEmpty())
		response = responseError("not found");
	else {
		response = QHttpServerResponse(*obj);
	}

	LAMBDA_THREAD_END;
}



/**
 * @brief AdminAPI::userCreate
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::userCreate(const QJsonObject &json)
{
	const QString &username = json.value(QStringLiteral("username")).toString();
	const QString &password = json.value(QStringLiteral("password")).toString();
	const QString &oauth = json.value(QStringLiteral("oauth")).toString();

	if (username.isEmpty())
		return responseError("missing username");

	if (password.isEmpty() && oauth.isEmpty())
		return responseError("missing password/oauth");

#ifdef _COMPAT
	if (!oauth.isEmpty() && !m_service->oauth2Authenticator(oauth.toUtf8()))
		return responseError("invalid provider");
#endif

	User user;
	user.username = username;

	LOG_CTRACE("client") << "Add user" << qPrintable(user.username);

	user.familyName = json.value(QStringLiteral("familyName")).toString();
	user.givenName = json.value(QStringLiteral("givenName")).toString();
	user.nickname = json.value(QStringLiteral("nickName")).toString();
	user.character = json.value(QStringLiteral("character")).toString();
	user.picture = json.value(QStringLiteral("picture")).toString();
	user.active = json.value(QStringLiteral("active")).toBool(true);
	user.classid = json.value(QStringLiteral("classid")).toInt(-1);
	user.isAdmin = json.value(QStringLiteral("isAdmin")).toBool(false);
	user.isTeacher = json.value(QStringLiteral("isTeacher")).toBool(false);
	user.isPanel = json.value(QStringLiteral("isPanel")).toBool(false);

	if (!userAdd(this, user))
		return responseError("user creation failed");

	if (!oauth.isEmpty()) {
		if (!authAddOAuth2(this, user.username, oauth))
			return responseError("user creation failed");
	} else {
		if (!authAddPlain(this, user.username, password))
			return responseError("user creation failed");
	}

	return responseOk({{QStringLiteral("username"), user.username}});
}






#ifdef _COMPAT



addMap("^user/import/*$", this, &AdminAPI::userImport);
addMap("^user/update/*$", this, &AdminAPI::usersProfileUpdate);
addMap("^user/([^/]+)/update/*$", this, &AdminAPI::userUpdate);
addMap("^user/([^/]+)/delete/*$", this, &AdminAPI::userDeleteOne);
addMap("^user/([^/]+)/password/*$", this, &AdminAPI::userPassword);
addMap("^user/([^/]+)/activate/*$", this, &AdminAPI::userActivateOne);
addMap("^user/([^/]+)/inactivate/*$", this, &AdminAPI::userInactivateOne);
addMap("^user/([^/]+)/move/(\\d+)/*$", this, &AdminAPI::userMoveOne);
addMap("^user/([^/]+)/move/none/*$", this, &AdminAPI::userMoveOneNoClass);
addMap("^user/delete/*$", this, &AdminAPI::userDelete);
addMap("^user/move/(\\d+)/*$", this, &AdminAPI::userMove);
addMap("^user/move/none/*$", this, &AdminAPI::userMoveNoClass);
addMap("^user/activate/*$", this, &AdminAPI::userActivate);
addMap("^user/inactivate/*$", this, &AdminAPI::userInactivate);
addMap("^user/([^/]+)/*$", this, &AdminAPI::user);
addMap("^class/(\\d+)/users/create/*$", this, &AdminAPI::userCreateClass);

addMap("^user/peers/*$", this, &AdminAPI::userPeers);
addMap("^user/peers/live/*$", this, &AdminAPI::userPeersLive);

addMap("^config/*$", this, &AdminAPI::configUpdate);
}








/**
 * @brief AdminAPI::userUpdate
 * @param match
 * @param data
 * @param response
 */


void AdminAPI::userUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const QString &username = match.captured(1);

	if (username.isEmpty())
		return responseError(response, "missing username");

	LOG_CTRACE("client") << "Modify user" << qPrintable(username);

	databaseMainWorker()->execInThread([username, response, data, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT username FROM user WHERE username=").addValue(username).execCheckExists()) {
			LOG_CWARNING("client") << "User doesn't exists:" << qPrintable(username);
			db.rollback();
			return responseError(response, "invalid username");
		}

		QueryBuilder q(db);

		q.addQuery("UPDATE user SET ").setCombinedPlaceholder();

		if (data.contains(QStringLiteral("familyName")))	q.addField("familyName", data.value(QStringLiteral("familyName")).toString());
		if (data.contains(QStringLiteral("givenName")))		q.addField("givenName", data.value(QStringLiteral("givenName")).toString());
		if (data.contains(QStringLiteral("active")))		q.addField("active", data.value(QStringLiteral("active")).toBool());
		if (data.contains(QStringLiteral("classid"))) {
			const int &id = data.value(QStringLiteral("classid")).toInt();
			if (id > 0)
				q.addField("classid", id);
			else
				q.addField("classid", QVariant::Invalid);
		}

		if (data.contains(QStringLiteral("isTeacher")))		q.addField("isTeacher", data.value(QStringLiteral("isTeacher")).toBool());
		if (data.contains(QStringLiteral("isAdmin")))		q.addField("isAdmin", data.value(QStringLiteral("isAdmin")).toBool());
		if (data.contains(QStringLiteral("isPanel")))		q.addField("isPanel", data.value(QStringLiteral("isPanel")).toBool());
		if (data.contains(QStringLiteral("nickname")))		q.addField("nickname", data.value(QStringLiteral("nickname")).toString());
		if (data.contains(QStringLiteral("character")))		q.addField("character", data.value(QStringLiteral("character")).toString());
		if (data.contains(QStringLiteral("picture")))		q.addField("picture", data.value(QStringLiteral("picture")).toString());

		q.addQuery(" WHERE username=").addValue(username);

		if (!q.fieldCount() || !q.exec()) {
			LOG_CWARNING("client") << "User update failed:" << username;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		responseAnswerOk(response, {{QStringLiteral("username"), username}});
	});
}




/**
 * @brief AdminAPI::userDelete
 * @param list
 * @param response
 */

void AdminAPI::userDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "missing username");

	LOG_CTRACE("client") << "Delete users" << list;

	databaseMainWorker()->execInThread([list, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("DELETE FROM user WHERE username IN (").addList(list.toVariantList()).addQuery(")")
				.exec()) {
			LOG_CWARNING("client") << "Users remove error:" << list;
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		LOG_CINFO("client") << "Users removed:" << list;
		responseAnswerOk(response);
	});
}





/**
 * @brief AdminAPI::userPassword
 * @param data
 * @param response
 */

void AdminAPI::userPassword(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const QString &username = match.captured(1);

	if (username.isEmpty())
		return responseError(response, "missing username");

	const QString &password = data.value(QStringLiteral("password")).toString();

	if (password.isEmpty())
		return responseError(response, "missing password");

	LOG_CTRACE("client") << "Change password for user:" << qPrintable(username);

	authPlainPasswordChange(this, username, QString(), password, false)
			.fail([this, response](){
		responseError(response, "failed");
	})			.done([this, username, response](){
		responseAnswerOk(response, {{QStringLiteral("username"), username}});
	});


}



/**
 * @brief AdminAPI::userActivate
 * @param list
 * @param active
 * @param response
 */

void AdminAPI::userActivate(const QJsonArray &list, const bool &active, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "missing username");

	if (active)
		LOG_CTRACE("client") << "Activate users" << list;
	else
		LOG_CTRACE("client") << "Inactivate users" << list;

	databaseMainWorker()->execInThread([list, active, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE user SET active=")
				.addValue(active)
				.addQuery(" WHERE username IN (").addList(list.toVariantList()).addQuery(")")
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		responseAnswerOk(response);
	});
}



/**
 * @brief AdminAPI::userMove
 * @param list
 * @param classid
 * @param response
 */

void AdminAPI::userMove(const QJsonArray &list, const int &classid, const QPointer<HttpResponse> &response) const
{
	if (list.isEmpty())
		return responseError(response, "missing username");

	LOG_CTRACE("client") << "Move users to class" << classid << list;

	databaseMainWorker()->execInThread([list, classid, response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		db.transaction();

		if (classid > 0 && !QueryBuilder::q(db).addQuery("SELECT id FROM class WHERE id=").addValue(classid).execCheckExists()) {
			LOG_CWARNING("client") << "Class doesn't exists:" << classid;
			db.rollback();
			return responseError(response, "invalid class");
		}

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE user SET classid=")
				.addValue(classid > 0 ? classid : QVariant(QVariant::Invalid))
				.addQuery(" WHERE username IN (").addList(list.toVariantList()).addQuery(")")
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		responseAnswerOk(response);
	});
}





/**
 * @brief AdminAPI::userImport
 * @param match
 * @param data
 * @param response
 */

void AdminAPI::userImport(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	LOG_CTRACE("client") << "Batch user import";


	databaseMainWorker()->execInThread([response, data, this]() {
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		const int classid = data.value(QStringLiteral("classid")).toInt(-1);

		if (list.isEmpty())
			return responseError(response, "missing list");

		QVariantList usernames;

		foreach (const QJsonValue &v, list) {
			const QString &u = v.toObject().value(QStringLiteral("username")).toString();
			if (!u.isEmpty())
				usernames << u;
		}

		if (usernames.isEmpty())
			return responseError(response, "missing usernames");

		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());


		// Check existing usernames

		QueryBuilder q(db);
		q.addQuery("SELECT username FROM user WHERE username IN (").addList(usernames).addQuery(")");

		if (!q.exec())
			return responseErrorSql(response);


		QStringList exists;

		while (q.sqlQuery().next())
			exists.append(q.value("username").toString());

		if (!exists.isEmpty())
			LOG_CWARNING("client") << "Users already exists:" << exists;
		/*return responseAnswer(response, {
									  { QStringLiteral("error"), QStringLiteral("user exists") },
									  { QStringLiteral("list"), QJsonArray::fromStringList(exists) }
								  });*/



		// Add users

		QJsonArray retList;

		foreach (const QJsonValue &v, list) {
			const QJsonObject &o = v.toObject();

			QJsonObject ret;

			const QString &username = o.value(QStringLiteral("username")).toString();
			const QString &password = o.value(QStringLiteral("password")).toString();
			const QString &oauth = o.value(QStringLiteral("oauth2")).toString();


			if (username.isEmpty()) {
				ret.insert(QStringLiteral("error"), QStringLiteral("missing username"));
				retList.append(ret);
				continue;
			}

			ret.insert(QStringLiteral("username"), username);

			if (exists.contains(username)) {
				ret.insert(QStringLiteral("error"), QStringLiteral("already exists"));
				retList.append(ret);
				continue;
			}

			if (password.isEmpty() && oauth.isEmpty()) {
				ret.insert(QStringLiteral("error"), QStringLiteral("missing password/oauth"));
				retList.append(ret);
				continue;
			}

			if (!oauth.isEmpty() && !m_service->oauth2Authenticator(oauth.toUtf8())) {
				ret.insert(QStringLiteral("error"), QStringLiteral("invalid provider"));
				retList.append(ret);
				continue;
			}

			User user;

			user.username = username;

			LOG_CTRACE("client") << "Add user" << qPrintable(user.username);

			user.familyName = o.value(QStringLiteral("familyName")).toString();
			user.givenName = o.value(QStringLiteral("givenName")).toString();
			user.nickname = o.value(QStringLiteral("nickName")).toString();
			user.character = o.value(QStringLiteral("character")).toString();
			user.picture = o.value(QStringLiteral("picture")).toString();
			user.active = true;
			user.classid = classid;
			user.isAdmin = false;
			user.isTeacher = false;
			user.isPanel = false;

			QDefer dret;

			userAdd(this, user)
					.fail([&ret, dret]() mutable {
				ret.insert(QStringLiteral("error"), QStringLiteral("failed"));
				dret.reject();
			})
					.then([this, user, password, oauth](){
				if (!oauth.isEmpty())
					return authAddOAuth2(this, user.username, oauth);
				else
					return authAddPlain(this, user.username, password);
			})
					.fail([&ret, dret]() mutable {
				ret.insert(QStringLiteral("error"), QStringLiteral("failed"));
				dret.reject();
			})
					.done([&ret, dret]() mutable {
				ret.insert(QStringLiteral("status"), QStringLiteral("ok"));
				dret.resolve();
			});

			QDefer::await(dret);

			retList.append(ret);
		}

		responseAnswerOk(response, {
							 { QStringLiteral("list"), retList }
						 });

	});
}



/**
 * @brief AdminAPI::userPeers
 * @param match
 * @param response
 */

void AdminAPI::userPeers(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	responseAnswer(response, "list", PeerUser::toJson(&(m_service->peerUser())));
}



/**
 * @brief AdminAPI::userPeersLive
 * @param match
 * @param response
 */

void AdminAPI::userPeersLive(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	HttpConnection *conn = qobject_cast<HttpConnection*>(response->parent());
	EventStream *stream = new EventStream(EventStream::EventStreamPeerUsers, conn);
	conn->setEventStream(stream);
	m_service->addEventStream(stream);

	response->setStatus(HttpStatus::Ok);

	stream->trigger();
}





/**
 * @brief AdminAPI::configUpdate
 * @param data
 * @param response
 */

void AdminAPI::configUpdate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	LOG_CINFO("client") << "Update configuration" << data;

	databaseMainWorker()->execInThread([response, data, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QJsonObject realData = data;

		if (data.contains(QStringLiteral("serverName"))) {
			const QString &name = data.value(QStringLiteral("serverName")).toString();
			if (!QueryBuilder::q(db).addQuery("UPDATE system SET serverName=").addValue(name).exec()) {
				LOG_CWARNING("client") << "Server name change error:" << qPrintable(name);
				return responseErrorSql(response);
			}

			realData.remove(QStringLiteral("serverName"));
			m_service->setServerName(data.value(QStringLiteral("serverName")).toString());
		}

		if (!realData.isEmpty())
			m_service->config().set(realData);

		responseAnswerOk(response);
	});
}






/**
 * @brief AdminAPI::usersProfileUpdate
 * @param response
 */

void AdminAPI::usersProfileUpdate(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	LOG_CINFO("client") << "Update users profile";

	databaseMainWorker()->execInThread([response, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QJsonArray userList;

		{
			QMutexLocker(databaseMain()->mutex());

			bool err = false;

			userList = QueryBuilder::q(db).addQuery("SELECT username, oauth, oauthData FROM auth WHERE oauth IS NOT NULL AND oauthData IS NOT NULL")
					.execToJsonArray(&err);

			if (err)
				responseErrorSql(response);
			else
				responseAnswerOk(response);
		}

		foreach (const QJsonValue &v, userList) {
			const QJsonObject &o = v.toObject();

			const QString &username = o.value(QStringLiteral("username")).toString();
			const QString &oauth = o.value(QStringLiteral("oauth")).toString();
			const QJsonObject &oauthData = Utils::byteArrayToJsonObject(o.value(QStringLiteral("oauthData")).toString().toUtf8());

			if (username.isEmpty() || oauth.isEmpty())
				continue;

			OAuth2Authenticator *authenticator = m_service->oauth2Authenticator(oauth.toLatin1());

			if (!authenticator) {
				LOG_CWARNING("client") << "Invalid authenticator:" << oauth;
				continue;
			}

			if (!authenticator->profileUpdateSupport()) {
				LOG_CTRACE("client") << "Authenticator not supports profile update:" << oauth;
				continue;
			}

			QMetaObject::invokeMethod(authenticator, "profileUpdate", Qt::QueuedConnection,
									  Q_ARG(QString, username),
									  Q_ARG(QJsonObject, oauthData)
									  );
		}

	});
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
 * @brief AdminAPI::campaignStart
 * @param api
 * @param campaign
 * @return
 */

QDefer AdminAPI::campaignStart(const AbstractAPI *api, const int &campaign)
{
	Q_ASSERT(api);
	return campaignStart(api->databaseMain(), campaign);
}


/**
 * @brief AdminAPI::campaignStart
 * @param dbMain
 * @param campaign
 * @return
 */

QDefer AdminAPI::campaignStart(const DatabaseMain *dbMain, const int &campaign)
{
	Q_ASSERT (dbMain);

	LOG_CDEBUG("client") << "Campaign start:" << campaign;

	QDefer ret;

	dbMain->worker()->execInThread([ret, campaign, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker(dbMain->mutex());

		db.transaction();

		QueryBuilder q(db);
		q.addQuery("SELECT starttime FROM campaign WHERE started=false AND finished=false AND id=").addValue(campaign);

		if (!q.exec() || !q.sqlQuery().first()) {
			db.rollback();
			return ret.reject();
		}

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE campaign SET ")
				.setCombinedPlaceholder()
				.addField("started", true)
				.addField("starttime", QDateTime::currentDateTimeUtc())
				.addQuery(" WHERE id=").addValue(campaign)
				.exec()) {
			LOG_CERROR("client") << "Campaign start error:" << campaign;
			db.rollback();
			return ret.reject();
		}

		db.commit();

		LOG_CINFO("client") << "Campaign started:" << campaign;
		ret.resolve();
	});

	return ret;
}


/**
 * @brief AdminAPI::campaignFinish
 * @param api
 * @param campaign
 * @return
 */

QDefer AdminAPI::campaignFinish(const AbstractAPI *api, const int &campaign)
{
	Q_ASSERT(api);

	return campaignFinish(api->databaseMain(), campaign);
}





/**
 * @brief AdminAPI::campaignFinish
 * @param dbMain
 * @param campaign
 * @return
 */

QDefer AdminAPI::campaignFinish(const DatabaseMain *dbMain, const int &campaign)
{
	Q_ASSERT(dbMain);

	LOG_CDEBUG("client") << "Campaign finish:" << campaign;

	QDefer ret;

	dbMain->worker()->execInThread([ret, campaign, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker(dbMain->mutex());

		db.transaction();

		if (!QueryBuilder::q(db)
				.addQuery("SELECT starttime FROM campaign WHERE started=true AND finished=false AND id=").addValue(campaign)
				.execCheckExists()) {
			db.rollback();
			return ret.reject();
		}

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE campaign SET ")
				.setCombinedPlaceholder()
				.addField("finished", true)
				.addField("endtime", QDateTime::currentDateTimeUtc())
				.addQuery(" WHERE id=").addValue(campaign)
				.exec()) {
			LOG_CERROR("client") << "Campaign finish error:" << campaign;
			db.rollback();
			return ret.reject();
		}

		QueryBuilder q(db);

		q.addQuery("WITH studentList(username, campaignid) AS (SELECT username, campaignid FROM campaignStudent) "
				   "SELECT username FROM studentGroupInfo WHERE active=true "
				   "AND id=(SELECT groupid FROM campaign WHERE campaign.id=").addValue(campaign)
				.addQuery(") AND (NOT EXISTS(SELECT * FROM studentList WHERE studentList.campaignid=").addValue(campaign)
				.addQuery(") OR studentGroupInfo.username IN (SELECT username FROM studentList WHERE studentList.campaignid=").addValue(campaign)
				.addQuery("))");

		if (!q.exec()) {
			LOG_CERROR("client") << "Campaign finish error:" << campaign;
			db.rollback();
			return ret.reject();
		}

		bool err = false;

		while (q.sqlQuery().next()) {
			const QString &username = q.value("username").toString();

			const TeacherAPI::UserCampaignResult &result = TeacherAPI::_campaignUserResult(dbMain, campaign, false, username, false, &err);

			if (err) {
				LOG_CERROR("client") << "Campaign finish error:" << campaign;
				db.rollback();
				return ret.reject();
			}

			if (result.grade <= 0 && result.xp <= 0)
				continue;

			QVariant scoreId = QVariant::Invalid;

			if (result.xp > 0) {
				scoreId = QueryBuilder::q(db)
						.addQuery("INSERT OR REPLACE INTO score (")
						.setFieldPlaceholder()
						.addQuery(") VALUES (")
						.setValuePlaceholder()
						.addQuery(")")
						.addField("username", username)
						.addField("xp", result.xp)
						.execInsertAsInt(&err);

				if (err) {
					LOG_CERROR("client") << "Campaign finish error:" << campaign;
					db.rollback();
					return ret.reject();
				}
			}

			QueryBuilder::q(db)
					.addQuery("INSERT OR REPLACE INTO campaignResult (")
					.setFieldPlaceholder()
					.addQuery(") VALUES (")
					.setValuePlaceholder()
					.addQuery(")")
					.addField("campaignid", campaign)
					.addField("username", username)
					.addField("gradeid", result.grade > 0 ? result.grade : QVariant(QVariant::Invalid))
					.addField("scoreid", scoreId)
					.execInsert(&err);

			if (err) {
				LOG_CERROR("client") << "Campaign finish error:" << campaign;
				db.rollback();
				return ret.reject();
			}
		}

		db.commit();

		LOG_CINFO("client") << "Campaign finished:" << campaign;
		ret.resolve();
	});

	return ret;
}





/**
 * @brief AdminAPI::authPlainPasswordChange
 * @param api
 * @param username
 * @param oldPassword
 * @param password
 * @param check
 * @return
 */

QDefer AdminAPI::authPlainPasswordChange(const AbstractAPI *api, const QString &username,
										 const QString &oldPassword, const QString &password,
										 const bool &check)
{
	Q_ASSERT(api);

	QDefer ret;

	if (username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		ret.reject();
		return ret;
	}

	LOG_CDEBUG("client") << "Password change:" << qPrintable(username);

	api->databaseMainWorker()->execInThread([ret, username, oldPassword, password, check, api]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		db.transaction();

		if (!QueryBuilder::q(db).addQuery("SELECT username FROM user WHERE username=").addValue(username).execCheckExists()) {
			LOG_CWARNING("client") << "User doesn't exists:" << qPrintable(username);
			db.rollback();
			return ret.reject();
		}

		QueryBuilder q(db);

		q.addQuery("SELECT password, salt, oauth FROM auth WHERE username=").addValue(username);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CWARNING("client") << "Sql error";
			db.rollback();
			return ret.reject();
		}

		if (!q.value("oauth").isNull()) {
			LOG_CWARNING("client") << "Unable to change password for OAuth2 user:" << qPrintable(username);
			db.rollback();
			return ret.reject();
		}

		if (check) {
			const QString &pwd = q.value("password").toString();
			const QString &salt = q.value("salt").toString();

			if (pwd != Credential::hashString(oldPassword, salt)) {
				LOG_CWARNING("client") << "Invalid password for user:" << qPrintable(username);
				db.rollback();
				return ret.reject();
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
			LOG_CWARNING("client") << "User password change error:" << qPrintable(username);
			db.rollback();
			return ret.reject();
		}

		db.commit();

		LOG_CINFO("client") << "User password changed:" << qPrintable(username);
		ret.resolve();
	});

	return ret;
}







#endif







/**
 * @brief AdminAPI::_classCreate
 * @param api
 * @param _class
 * @return
 */

std::optional<int> AdminAPI::_classCreate(const AbstractAPI *api, const Class &_class)
{
	if (!api) {
		LOG_CERROR("client") << "Missing API";
		return std::nullopt;
	}

	QDefer ret;
	std::optional<int> newId;

	api->databaseMainWorker()->execInThread([ret, api, _class, &newId]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		db.transaction();

		const auto &id = QueryBuilder::q(db)
				.addQuery("INSERT INTO class(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("name", _class.name)
				.execInsertAsInt();

		if (!id) {
			LOG_CWARNING("client") << "Class create error:" << qPrintable(_class.name);
			db.rollback();
			newId = std::nullopt;
			return ret.reject();
		}

		const QString &code = generateClassCode();

		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO classCode(")
				.setFieldPlaceholder()
				.addQuery(") VALUES (")
				.setValuePlaceholder()
				.addQuery(")")
				.addField("classid", *id)
				.addField("code", code)
				.exec())
		{
			LOG_CWARNING("client") << "Class create error:" << qPrintable(_class.name) << *id << code;
			db.rollback();
			newId = std::nullopt;
			return ret.reject();
		}

		db.commit();

		newId = *id;
	});

	QDefer::await(ret);

	return newId;
}



/**
 * @brief AdminAPI::generateClassCode
 * @return
 */

QString AdminAPI::generateClassCode()
{
	return QString::fromLatin1(Utils::generateRandomString(6, "1234567890"));
}







/**
 * @brief AdminAPI::userAdd
 * @param api
 * @param user
 * @return
 */

bool AdminAPI::userAdd(const AbstractAPI *api, const User &user)
{
	Q_ASSERT(api);
	return userAdd(api->databaseMain(), user);
}



/**
 * @brief AdminAPI::userAdd
 * @param db
 * @param user
 * @return
 */

bool AdminAPI::userAdd(const DatabaseMain *dbMain, const User &user)
{
	Q_ASSERT (dbMain);

	LOG_CDEBUG("client") << "Add new user:" << qPrintable(user.username);

	if (user.username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		return false;
	}

	QDefer ret;
	bool success = false;

	dbMain->worker()->execInThread([ret, user, dbMain, &success]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker(dbMain->mutex());

		db.transaction();

		if (QueryBuilder::q(db).addQuery("SELECT username FROM user WHERE username=").addValue(user.username).execCheckExists()) {
			LOG_CWARNING("client") << "User already exists:" << qPrintable(user.username);
			db.rollback();
			return ret.reject();
		}

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
				.addField("classid", user.classid > 0 ? user.classid : QVariant(QMetaType::fromType<int>()))
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

		LOG_CINFO("client") << "New user created:" << qPrintable(user.username);
		success = true;
		ret.resolve();
	});

	QDefer::await(ret);

	return success;
}






/**
 * @brief AdminAPI::authAddPlain
 * @param handler
 * @param username
 * @param password
 * @return
 */

bool AdminAPI::authAddPlain(const AbstractAPI *api, const QString &username, const QString &password)
{
	Q_ASSERT(api);
	return authAddPlain(api->databaseMain(), username, password);
}


/**
 * @brief AdminAPI::authAddPlain
 * @param dbMain
 * @param username
 * @param password
 * @return
 */

bool AdminAPI::authAddPlain(const DatabaseMain *dbMain, const QString &username, const QString &password)
{
	Q_ASSERT(dbMain);

	LOG_CDEBUG("client") << "Add plain auth:" << qPrintable(username);

	if (username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		return false;
	}

	QDefer ret;
	bool success = false;

	dbMain->worker()->execInThread([ret, username, password, dbMain, &success]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker(dbMain->mutex());

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

		success = true;
		ret.resolve();

	});

	QDefer::await(ret);

	return success;
}





/**
 * @brief AdminAPI::authAddOAuth2
 * @param handler
 * @param username
 * @param type
 * @return
 */

bool AdminAPI::authAddOAuth2(const AbstractAPI *api, const QString &username, const QString &type)
{
	Q_ASSERT(api);

	LOG_CDEBUG("client") << "Add OAuth2 auth:" << qPrintable(username) << qPrintable(type);

	if (username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		return false;
	}

	QDefer ret;
	bool success = false;

	api->databaseMainWorker()->execInThread([ret, username, type, api, &success]() mutable {
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
		success = true;
		ret.resolve();
	});

	QDefer::await(ret);

	return success;
}

