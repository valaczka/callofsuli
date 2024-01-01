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
#include "peerengine.h"
#include "querybuilder.hpp"
#include "serverservice.h"
#include "teacherapi.h"


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
	auto server = m_handler->httpServer().lock().get();

	Q_ASSERT(server);

	m_validateRole = Credential::Admin;

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	server->route(path+"user", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classUsers(0);
	});


	server->route(path+"user/noclass", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classUsers(-1);
	});

	server->route(path+"user/create", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userCreate(*jsonObject);
	});

	server->route(path+"user/delete", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userDelete(jsonObject->value(QStringLiteral("list")).toArray());
	});

	server->route(path+"user/activate", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userActivate(jsonObject->value(QStringLiteral("list")).toArray(), true);
	});

	server->route(path+"user/inactivate", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userActivate(jsonObject->value(QStringLiteral("list")).toArray(), false);
	});

	server->route(path+"user/move/", QHttpServerRequest::Method::Post, [this](const int &classid, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userMove(jsonObject->value(QStringLiteral("list")).toArray(), classid);
	});

	server->route(path+"user/move/none", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userMove(jsonObject->value(QStringLiteral("list")).toArray(), -1);
	});

	server->route(path+"user/import", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userImport(*jsonObject);
	});

	server->route(path+"user/update", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return usersProfileUpdate();
	});




	server->route(path+"user", QHttpServerRequest::Method::Put, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userCreate(*jsonObject);
	});




	server->route(path+"user/<arg>/update", QHttpServerRequest::Method::Post, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userUpdate(username, *jsonObject);
	});

	server->route(path+"user/<arg>/delete", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userDelete(QJsonArray{username});
	});

	server->route(path+"user/<arg>/password", QHttpServerRequest::Method::Post, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userPassword(username, *jsonObject);
	});

	server->route(path+"user/<arg>/activate", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userActivate(QJsonArray{username}, true);
	});

	server->route(path+"user/<arg>/inactivate", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userActivate(QJsonArray{username}, false);
	});

	server->route(path+"user/<arg>/move/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &username, const int &classid, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userMove(QJsonArray{username}, classid);
	});

	server->route(path+"user/<arg>/move/none", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userMove(QJsonArray{username}, -1);
	});

	server->route(path+"user/", QHttpServerRequest::Method::Delete, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userDelete(QJsonArray{username});
	});

	server->route(path+"user/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return user(username);
	});




	server->route(path+"class/<arg>/users", QHttpServerRequest::Method::Put, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userCreateClass(*jsonObject, id);
	});

	server->route(path+"class/<arg>/users/create", QHttpServerRequest::Method::Post, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return userCreateClass(*jsonObject, id);
	});

	server->route(path+"class", QHttpServerRequest::Method::Put, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return classCreate(*jsonObject);
	});

	server->route(path+"class/", QHttpServerRequest::Method::Delete, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classDelete(QJsonArray{id});
	});

	server->route(path+"class/create", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return classCreate(*jsonObject);
	});

	server->route(path+"class/<arg>/users", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classUsers(id);
	});

	server->route(path+"class/<arg>/update", QHttpServerRequest::Method::Post, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return classUpdate(id, *jsonObject);
	});

	server->route(path+"class/code", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classCode(-2);
	});

	server->route(path+"class/nocode", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classCode(-1);
	});

	server->route(path+"class/<arg>/code", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classCode(id);
	});

	server->route(path+"class/<arg>/updateCode", QHttpServerRequest::Method::Post, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return classUpdateCode(id, *jsonObject);
	});

	server->route(path+"class/<arg>/delete", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return classDelete(QJsonArray{id});
	});

	server->route(path+"class/delete", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return classDelete(jsonObject->value(QStringLiteral("list")).toArray());
	});





	server->route(path+"user/peers", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return userPeers();
	});

	server->route(path+"config", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return configUpdate(*jsonObject);
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

	response = responseResult("list", std::move(*list));

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
		response = responseResult("list", std::move(*list));
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

	if (!oauth.isEmpty() && !m_service->oauth2Authenticator(oauth.toUtf8()))
		return responseError("invalid provider");

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




/**
 * @brief AdminAPI::userCreateClass
 * @param json
 * @param classid
 * @return
 */

QHttpServerResponse AdminAPI::userCreateClass(QJsonObject json, const int &classid)
{
	json[QStringLiteral("classid")] = classid;
	return userCreate(json);
}




/**
 * @brief AdminAPI::userUpdate
 * @param username
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::userUpdate(const QString &username, const QJsonObject &json)
{
	LOG_CTRACE("client") << "User update" << username;

	if (username.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(username, json);

	db.transaction();

	LAMBDA_SQL_ERROR_ROLLBACK("not found", QueryBuilder::q(db).addQuery("SELECT username FROM user WHERE username=").addValue(username).execCheckExists());

	QueryBuilder q(db);

	q.addQuery("UPDATE user SET ").setCombinedPlaceholder();

	if (json.contains(QStringLiteral("familyName")))	q.addField("familyName", json.value(QStringLiteral("familyName")).toString());
	if (json.contains(QStringLiteral("givenName")))		q.addField("givenName", json.value(QStringLiteral("givenName")).toString());
	if (json.contains(QStringLiteral("active")))		q.addField("active", json.value(QStringLiteral("active")).toBool());
	if (json.contains(QStringLiteral("classid"))) {
		const int &id = json.value(QStringLiteral("classid")).toInt();
		if (id > 0)
			q.addField("classid", id);
		else
			q.addField("classid", QVariant(QMetaType::fromType<int>()));
	}

	if (json.contains(QStringLiteral("isTeacher")))		q.addField("isTeacher", json.value(QStringLiteral("isTeacher")).toBool());
	if (json.contains(QStringLiteral("isAdmin")))		q.addField("isAdmin", json.value(QStringLiteral("isAdmin")).toBool());
	if (json.contains(QStringLiteral("isPanel")))		q.addField("isPanel", json.value(QStringLiteral("isPanel")).toBool());
	if (json.contains(QStringLiteral("nickname")))		q.addField("nickname", json.value(QStringLiteral("nickname")).toString());
	if (json.contains(QStringLiteral("character")))		q.addField("character", json.value(QStringLiteral("character")).toString());
	if (json.contains(QStringLiteral("picture")))		q.addField("picture", json.value(QStringLiteral("picture")).toString());

	q.addQuery(" WHERE username=").addValue(username);

	LAMBDA_SQL_ASSERT_ROLLBACK(q.fieldCount() && q.exec());

	db.commit();

	response = responseOk({{QStringLiteral("username"), username}});

	LAMBDA_THREAD_END;
}



/**
 * @brief AdminAPI::userDelete
 * @param userList
 * @return
 */

QHttpServerResponse AdminAPI::userDelete(const QJsonArray &userList)
{
	LOG_CTRACE("client") << "User delete" << userList;

	if (userList.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(userList);

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
					  .addQuery("DELETE FROM user WHERE username IN (")
					  .addList(userList.toVariantList())
					  .addQuery(")")
					  .exec());

	LOG_CDEBUG("client") << "User deleted:" << userList;

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief AdminAPI::userPassword
 * @param username
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::userPassword(const QString &username, const QJsonObject &json)
{
	LOG_CTRACE("client") << "User update password" << username;

	if (username.isEmpty())
		return responseError("invalid username");

	const QString &password = json.value(QStringLiteral("password")).toString();

	if (password.isEmpty())
		return responseError("missing password");

	LOG_CDEBUG("client") << "Change password for user:" << qPrintable(username);

	if (!authPlainPasswordChange(this, username, QString(), password, false))
		return responseError("failed");
	else
		return responseOk({{QStringLiteral("username"), username}});
}





/**
 * @brief AdminAPI::userActivate
 * @param userList
 * @param active
 * @return
 */

QHttpServerResponse AdminAPI::userActivate(const QJsonArray &userList, const bool &active)
{
	LOG_CTRACE("client") << "User activate" << active << userList;

	if (userList.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(userList, active);

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
					  .addQuery("UPDATE user SET active=")
					  .addValue(active)
					  .addQuery(" WHERE username IN (").addList(userList.toVariantList()).addQuery(")")
					  .exec());

	LOG_CDEBUG("client") << "User activated:" << active << userList;

	response = responseOk();

	LAMBDA_THREAD_END;
}




/**
 * @brief AdminAPI::userMove
 * @param userList
 * @param classid
 * @return
 */

QHttpServerResponse AdminAPI::userMove(const QJsonArray &userList, const int &classid)
{
	LOG_CTRACE("client") << "Move users to class" << classid << userList;

	if (userList.isEmpty())
		return responseError("invalid username");

	LAMBDA_THREAD_BEGIN(userList, classid);

	db.transaction();

	LAMBDA_SQL_ERROR_ROLLBACK("invalid class",
							  (classid < 0 || QueryBuilder::q(db).addQuery("SELECT id FROM class WHERE id=").addValue(classid).execCheckExists()));

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("UPDATE user SET classid=")
							   .addValue(classid > 0 ? classid : QVariant(QMetaType::fromType<int>()))
							   .addQuery(" WHERE username IN (").addList(userList.toVariantList()).addQuery(")")
							   .exec());

	LOG_CDEBUG("client") << "Users moved to class:" << classid << userList;

	db.commit();

	response = responseOk();

	LAMBDA_THREAD_END;
}




/**
 * @brief AdminAPI::userImport
 * @return
 */

QHttpServerResponse AdminAPI::userImport(const QJsonObject &json)
{
	LOG_CTRACE("client") << "User batch import";

	const QJsonArray &list = json.value(QStringLiteral("list")).toArray();
	const int classid = json.value(QStringLiteral("classid")).toInt(-1);

	if (list.isEmpty())
		return responseError("missing list");

	QVariantList usernames;

	for (const QJsonValue &v : std::as_const(list)) {
		const QString &u = v.toObject().value(QStringLiteral("username")).toString();
		if (!u.isEmpty())
			usernames << u;
	}

	if (usernames.isEmpty())
		return responseError("missing usernames");

	LAMBDA_THREAD_BEGIN(list, classid, usernames);


	// Check existing usernames

	QueryBuilder q(db);
	q.addQuery("SELECT username FROM user WHERE username IN (").addList(usernames).addQuery(")");

	LAMBDA_SQL_ASSERT(q.exec());

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

	for (const QJsonValue &v : std::as_const(list)) {
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

		if (userAdd(this, user)) {
			if (!oauth.isEmpty()) {
				if (authAddOAuth2(this, user.username, oauth))
					ret.insert(QStringLiteral("status"), QStringLiteral("ok"));
				else
					ret.insert(QStringLiteral("error"), QStringLiteral("oauth failed"));
			} else {
				if (!authAddPlain(this, user.username, password))
					ret.insert(QStringLiteral("status"), QStringLiteral("ok"));
				else
					ret.insert(QStringLiteral("error"), QStringLiteral("plain auth failed"));
			}
		} else {
			ret.insert(QStringLiteral("error"), QStringLiteral("failed"));
		}

		retList.append(ret);
	}

	response = responseResult("list", retList);

	LAMBDA_THREAD_END;
}




/**
 * @brief AdminAPI::userPeers
 * @return
 */

QHttpServerResponse AdminAPI::userPeers()
{
	const auto &list = m_service->engineHandler()->engineGet<PeerEngine>(AbstractEngine::EnginePeer);

	QJsonArray r;

	if (!list.isEmpty()) {
		r = PeerUser::toJson(list.at(0)->peerUser());
	}

	return responseResult("list", r);
}



/**
 * @brief AdminAPI::configUpdate
 * @param json
 * @return
 */

QHttpServerResponse AdminAPI::configUpdate(const QJsonObject &json)
{
	LOG_CINFO("client") << "Update configuration" << json;

	LAMBDA_THREAD_BEGIN(json);

	QJsonObject realData = json;

	if (json.contains(QStringLiteral("serverName"))) {
		const QString &name = json.value(QStringLiteral("serverName")).toString();

		LAMBDA_SQL_ASSERT(QueryBuilder::q(db).addQuery("UPDATE system SET serverName=").addValue(name).exec());

		realData.remove(QStringLiteral("serverName"));
		m_service->setServerName(json.value(QStringLiteral("serverName")).toString());
	}

	if (!realData.isEmpty())
		m_service->config().set(realData);

	response = responseOk();

	LAMBDA_THREAD_END;
}






/**
 * @brief AdminAPI::usersProfileUpdate
 * @return
 */

QHttpServerResponse AdminAPI::usersProfileUpdate()
{
	LOG_CINFO("client") << "Update users profile";

	QDefer ret;
	std::optional<QJsonArray> userList;

	databaseMainWorker()->execInThread([ret, this, &userList]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker _locker(databaseMain()->mutex());

		userList = QueryBuilder::q(db).addQuery("SELECT username, oauth, oauthData FROM auth WHERE oauth IS NOT NULL AND oauthData IS NOT NULL")
				.execToJsonArray();

		ret.resolve();
	});

	QDefer::await(ret);

	if (!userList)
		return responseErrorSql();

	for (const QJsonValue &v : std::as_const(*userList)) {
		const QJsonObject &o = v.toObject();

		const QString &username = o.value(QStringLiteral("username")).toString();
		const QString &oauth = o.value(QStringLiteral("oauth")).toString();
		const QJsonObject &oauthData = Utils::byteArrayToJsonObject(o.value(QStringLiteral("oauthData")).toString().toUtf8()).value_or(QJsonObject{});

		if (username.isEmpty() || oauth.isEmpty())
			continue;

		OAuth2Authenticator *authenticator = m_service->oauth2Authenticator(oauth.toLatin1())->lock().get();

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

	return responseOk();
}









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

		QMutexLocker _locker(api->databaseMain()->mutex());

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

	dbMain->worker()->execInThread([ret, user, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

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
		ret.resolve();
	});

	QDefer::await(ret);

	return (ret.state() == RESOLVED);
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

	dbMain->worker()->execInThread([ret, username, password, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

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

	QDefer::await(ret);

	return (ret.state() == RESOLVED);
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

	api->databaseMainWorker()->execInThread([ret, username, type, api]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

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

	QDefer::await(ret);

	return (ret.state() == RESOLVED);
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

bool AdminAPI::authPlainPasswordChange(const AbstractAPI *api, const QString &username,
									   const QString &oldPassword, const QString &password,
									   const bool &check)
{
	Q_ASSERT(api);

	if (username.isEmpty()) {
		LOG_CWARNING("client") << "Empty username";
		return false;
	}

	QDefer ret;

	LOG_CDEBUG("client") << "Password change:" << qPrintable(username);

	api->databaseMainWorker()->execInThread([ret, username, oldPassword, password, check, api]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

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

	QDefer::await(ret);

	return (ret.state() == RESOLVED);
}







/**
 * @brief AdminAPI::campaignStart
 * @param api
 * @param campaign
 * @return
 */

bool AdminAPI::campaignStart(const AbstractAPI *api, const int &campaign)
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

bool AdminAPI::campaignStart(const DatabaseMain *dbMain, const int &campaign)
{
	Q_ASSERT (dbMain);

	LOG_CDEBUG("client") << "Campaign start:" << campaign;

	QDefer ret;

	dbMain->worker()->execInThread([ret, campaign, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

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

	QDefer::await(ret);
	return (ret.state() == RESOLVED);
}


/**
 * @brief AdminAPI::campaignFinish
 * @param api
 * @param campaign
 * @return
 */

bool AdminAPI::campaignFinish(const AbstractAPI *api, const int &campaign)
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

bool AdminAPI::campaignFinish(const DatabaseMain *dbMain, const int &campaign)
{
	Q_ASSERT(dbMain);

	LOG_CDEBUG("client") << "Campaign finish:" << campaign;

	QDefer ret;

	dbMain->worker()->execInThread([ret, campaign, dbMain]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

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

		while (q.sqlQuery().next()) {
			const QString &username = q.value("username").toString();

			const auto &result = TeacherAPI::_campaignUserResult(dbMain, campaign, false, username, false);

			if (!result) {
				LOG_CERROR("client") << "Campaign finish error:" << campaign;
				db.rollback();
				return ret.reject();
			}

			if (result->grade <= 0 && result->xp <= 0)
				continue;

			//QVariant scoreId = QVariant(QMetaType::fromType<int>());
			int scoreId = -1;

			if (result->xp > 0) {
				const auto &sId = QueryBuilder::q(db)
						.addQuery("INSERT OR REPLACE INTO score (")
						.setFieldPlaceholder()
						.addQuery(") VALUES (")
						.setValuePlaceholder()
						.addQuery(")")
						.addField("username", username)
						.addField("xp", result->xp)
						.execInsertAsInt();

				if (!sId) {
					LOG_CERROR("client") << "Campaign finish error:" << campaign;
					db.rollback();
					return ret.reject();
				}

				scoreId = *sId;
			}

			if (!QueryBuilder::q(db)
					.addQuery("INSERT OR REPLACE INTO campaignResult (")
					.setFieldPlaceholder()
					.addQuery(") VALUES (")
					.setValuePlaceholder()
					.addQuery(")")
					.addField("campaignid", campaign)
					.addField("username", username)
					.addField("gradeid", result->grade > 0 ? result->grade : QVariant(QMetaType::fromType<int>()))
					.addField("scoreid", scoreId > 0 ? scoreId : QVariant(QMetaType::fromType<int>()))
					.execInsert())
			{
				LOG_CERROR("client") << "Campaign finish error:" << campaign;
				db.rollback();
				return ret.reject();
			}
		}

		db.commit();

		LOG_CINFO("client") << "Campaign finished:" << campaign;
		ret.resolve();
	});

	QDefer::await(ret);
	return (ret.state() == RESOLVED);
}









/**
 * @brief AdminAPI::userExists
 * @param username
 * @return
 */

bool AdminAPI::userExists(const AbstractAPI *api, const QString &username, const bool &inverse)
{
	Q_ASSERT (api);

	if (inverse)
		LOG_CTRACE("client") << "Check user not exists:" << username;
	else
		LOG_CTRACE("client") << "Check user exists:" << username;

	QDefer ret;

	api->databaseMainWorker()->execInThread([ret, username, api, inverse]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

		const auto &r = QueryBuilder::q(db)
				.addQuery("SELECT username FROM user WHERE username=")
				.addValue(username)
				.execCheckExists();

		if (!r != !inverse)
			ret.resolve();
		else
			ret.reject();
	});

	QDefer::await(ret);
	return (ret.state() == RESOLVED);
}





/**
 * @brief AdminAPI::getClassIdFromCode
 * @param api
 * @param code
 * @return
 */

std::optional<int> AdminAPI::getClassIdFromCode(const AbstractAPI *api, const QString &code)
{
	Q_ASSERT (api);

	LOG_CTRACE("client") << "Get class id for code:" << code;

	QDefer ret;
	int retInt = 0;

	api->databaseMainWorker()->execInThread([ret, code, api, &retInt]() mutable {
		if (code.isEmpty()) {
			LOG_CDEBUG("client") << "Empty code";
			return ret.reject();
		}

		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT code, classid FROM classCode WHERE code=")
				.addValue(code);

		if (!q.exec() || !q.sqlQuery().first()) {
			LOG_CDEBUG("client") << "Class code doesn't exists:" << qPrintable(code);
			return ret.reject();
		}

		retInt = q.value("classid", -1).toInt();
		ret.resolve();
	});

	QDefer::await(ret);

	if (ret.state() == RESOLVED)
		return retInt;
	else
		return std::nullopt;
}
