/*
 * ---- Call of Suli ----
 *
 * generalapi.cpp
 *
 * Created on: 2023. 03. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GeneralAPI
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

#include "generalapi.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "rank.h"
#include "Logger.h"
#include "serverservice.h"

/**
 * @brief GeneralAPI::GeneralAPI
 * @param service
 */

GeneralAPI::GeneralAPI(Handler *handler, ServerService *service)
	: AbstractAPI("general", handler, service)
{
	auto server = m_handler->httpServer().lock().get();

	Q_ASSERT(server);

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	server->route(path+"config", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return config();
	});

	server->route(path+"grade", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return grade();
	});

	server->route(path+"rank", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::rank, &*this, -1);
	});

	server->route(path+"rank/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::rank, &*this, id);
	});



	server->route(path+"class", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::class_, &*this, -1);
	});

	server->route(path+"class/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::class_, &*this, id);
	});

	server->route(path+"class/<arg>/users", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::classUsers, &*this, id);
	});



	server->route(path+"user/<arg>/log", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::userLog, &*this, username);
	});

	server->route(path+"user/<arg>/log/xp", QHttpServerRequest::Method::Post, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_GET();
		return QtConcurrent::run(&GeneralAPI::userXpLog, &*this, username, jsonObject.value_or(QJsonObject{}));
	});

	server->route(path+"user/<arg>/log/game", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::userGameLog, &*this, username);
	});

	server->route(path+"user", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::user, &*this, QStringLiteral(""), Credential::None);
	});

	server->route(path+"user/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QString &username, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::user, &*this, username, Credential::None);
	});

	server->route(path+"me", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API_X(Credential::Student|Credential::Admin);
		return QtConcurrent::run(&GeneralAPI::me, &*this, credential);
	});

	server->route(path+"score", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&GeneralAPI::user, &*this, QStringLiteral(""), Credential::Student);
	});

}



/**
 * @brief GeneralAPI::config
 * @param request
 * @return
 */

QHttpServerResponse GeneralAPI::config()
{
	LOG_CTRACE("client") << "Get config";

	QJsonObject c = m_service->config().get();

	QJsonArray pList;

	for (const auto &a : qAsConst(m_service->authenticators()))
		pList.append(QString::fromLatin1(a->type()));

	c.insert(QStringLiteral("oauthProviders"), pList);

	QJsonObject r;
	r.insert(QStringLiteral("server"), QStringLiteral("Call of Suli server"));
	r.insert(QStringLiteral("name"), m_service->serverName());
	r.insert(QStringLiteral("versionMajor"), m_service->versionMajor());
	r.insert(QStringLiteral("versionMinor"), m_service->versionMinor());
	r.insert(QStringLiteral("config"), c);

	return QHttpServerResponse(r, QHttpServerResponse::StatusCode::Ok);
}



/**
 * @brief GeneralAPI::rank
 * @param request
 * @param id
 * @return
 */

QHttpServerResponse GeneralAPI::rank(const int &id)
{
	LOG_CTRACE("client") << "Get rank" << id;

	LAMBDA_THREAD_BEGIN(id);

	QueryBuilder q(db);
	q.addQuery("SELECT id, level, sublevel, xp, name FROM rank");

	if (id != -1)
		q.addQuery(" WHERE id=").addValue(id);

	q.addQuery(" ORDER BY level, sublevel");

	LAMBDA_SQL_ASSERT(q.exec());

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

	if (id == -1)
		response = responseResult("list", list.toJson());
	else if (list.size() != 1)
		response = responseError("not found");
	else {
		response = QHttpServerResponse(list.at(0).toJson());
	}

	LAMBDA_THREAD_END;
}




/**
 * @brief GeneralAPI::grade
 * @return
 */

QHttpServerResponse GeneralAPI::grade()
{
	LOG_CTRACE("client") << "Get grade";

	LAMBDA_THREAD_BEGIN_NOVAR();

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT id, shortname, longname, value FROM grade")
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief GeneralAPI::class_
 * @param id
 * @return
 */

QHttpServerResponse GeneralAPI::class_(const int &id)
{
	LOG_CTRACE("client") << "Get class" << id;

	LAMBDA_THREAD_BEGIN(id);

	QueryBuilder q(db);
	q.addQuery("SELECT id, name FROM class");

	if (id != -1)
		q.addQuery(" WHERE id=").addValue(id);

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
 * @brief GeneralAPI::classUser
 * @param id
 * @return
 */

QHttpServerResponse GeneralAPI::classUsers(const int &id)
{
	LOG_CTRACE("client") << "Get class users" << id;

	LAMBDA_THREAD_BEGIN(id);

	QueryBuilder q(db);
	q.addQuery(_SQL_get_user)
			.addQuery("WHERE isPanel=false AND active=true AND user.classid=")
			.addValue(id);

	const std::optional<QJsonArray> &list = q.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}




/**
 * @brief GeneralAPI::user
 * @param username
 * @return
 */

QHttpServerResponse GeneralAPI::user(const QString &username, const Credential::Roles &roles)
{
	LOG_CTRACE("client") << "Get user" << username;

	LAMBDA_THREAD_BEGIN(username, roles);

	QueryBuilder q(db);
	q.addQuery(_SQL_get_user)
			.addQuery("WHERE active=true");

	if (!roles.testFlag(Credential::None)) {
		QStringList list;
		if (roles.testFlag(Credential::Teacher) && !roles.testFlag(Credential::Student))
			list.append(QStringLiteral("isTeacher=true"));
		if (!roles.testFlag(Credential::Teacher) && roles.testFlag(Credential::Student))
			list.append(QStringLiteral("isTeacher=false"));
		if (roles.testFlag(Credential::Admin))
			list.append(QStringLiteral("isAdmin=true"));
		if (roles.testFlag(Credential::Panel))
			list.append(QStringLiteral("isPanel=true"));

		if (!roles.testFlag(Credential::Panel))
			q.addQuery(" AND isPanel=false");

		if (!list.isEmpty()) {
			QString str = QStringLiteral(" AND (")+list.join(QStringLiteral(" OR "))+QStringLiteral(")");
			q.addQuery(str.toUtf8());
		}
	}

	if (!username.isEmpty()) {
		q.addQuery(" AND user.username=")
				.addValue(username);
	}

	const std::optional<QJsonArray> &list = q.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	if (username.isEmpty())
		response = responseResult("list", *list);
	else if (list->size() != 1)
		response = responseError("not found");
	else {
		response = QHttpServerResponse(list->at(0).toObject());
	}

	LAMBDA_THREAD_END;
}




/**
 * @brief GeneralAPI::userLog
 * @param username
 * @return
 */

QHttpServerResponse GeneralAPI::userLog(const QString &username)
{
	LOG_CTRACE("client") << "Get user log" << username;

	if (username.isEmpty())
		return responseError("missing username");


	LAMBDA_THREAD_BEGIN(username);

	QJsonObject obj;

	const auto &rankList = QueryBuilder::q(db)
			.addQuery("SELECT rankid, CAST(strftime('%s', timestamp) AS INTEGER) AS timestamp, xp FROM ranklog WHERE username=").addValue(username)
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(rankList);

	obj[QStringLiteral("ranklog")] = *rankList;


	const auto &streakList = QueryBuilder::q(db)
			.addQuery("SELECT streak, CAST(strftime('%s', started_on) AS INTEGER) AS started_on, "
					  "CAST(strftime('%s', ended_on) AS INTEGER) AS ended_on FROM streak "
					  "WHERE streak > 1 AND username=").addValue(username)
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(streakList);

	obj[QStringLiteral("streaklog")] = *streakList;

	const auto &durationList = QueryBuilder::q(db)
			.addQuery("WITH modes(mode) AS (SELECT DISTINCT mode FROM game), "
					  "usermodes(username, mode) AS (SELECT DISTINCT username, modes.mode FROM game LEFT JOIN modes) "
					  "SELECT usermodes.mode AS mode, SUM(duration) AS duration FROM usermodes "
					  "LEFT JOIN game ON (game.username = usermodes.username AND game.mode = usermodes.mode) "
					  "WHERE usermodes.username=").addValue(username)
			.addQuery(" GROUP BY usermodes.username, usermodes.mode")
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(durationList);

	obj[QStringLiteral("durations")] = *durationList;


	const auto &trophyList = QueryBuilder::q(db)
			.addQuery("WITH modes(mode) AS (SELECT DISTINCT mode FROM game), "
					  "usermodes(username, mode) AS (SELECT DISTINCT username, modes.mode FROM game LEFT JOIN modes) "
					  "SELECT usermodes.mode AS mode, COUNT(success) AS trophy FROM usermodes "
					  "LEFT JOIN game ON (game.username = usermodes.username AND game.mode = usermodes.mode AND success=true) "
					  "WHERE usermodes.username=").addValue(username)
			.addQuery(" GROUP BY usermodes.username, usermodes.mode")
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(trophyList);

	obj[QStringLiteral("trophies")] = *trophyList;

	response = QHttpServerResponse(obj);

	LAMBDA_THREAD_END;
}



/**
 * @brief GeneralAPI::userXpLog
 * @param username
 * @return
 */

QHttpServerResponse GeneralAPI::userXpLog(const QString &username, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Get user XP log" << username;

	if (username.isEmpty())
		return responseError("missing username");

	LAMBDA_THREAD_BEGIN(username, json);

	std::optional<QJsonArray> list;

	if (json.value(QStringLiteral("cummulate")).toBool()) {
		list = QueryBuilder::q(db)
				.addQuery("WITH cte AS (SELECT username, date(timestamp) AS day FROM score GROUP BY username, date(timestamp)) "
						  "SELECT day, CAST(JULIANDAY(date('now'))-JULIANDAY(day) AS INTEGER) AS diff, "
						  "SUM(xp) AS xp FROM cte LEFT JOIN score ON (score.username=cte.username AND score.timestamp<=cte.day) "
						  "WHERE cte.username=").addValue(username)
				.addQuery(" GROUP BY day").execToJsonArray();
	} else {
		list = QueryBuilder::q(db)
				.addQuery("WITH cte AS (SELECT username, date(timestamp) AS day FROM score GROUP BY username, date(timestamp)) "
						  "SELECT day, CAST(JULIANDAY(date('now'))-JULIANDAY(day) AS INTEGER) AS diff, "
						  "SUM(xp) AS xp FROM cte LEFT JOIN score ON (score.username=cte.username AND date(score.timestamp)=cte.day) "
						  "WHERE cte.username=").addValue(username)
				.addQuery(" GROUP BY day").execToJsonArray();
	}

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief GeneralAPI::userGameLog
 * @param username
 * @return
 */

QHttpServerResponse GeneralAPI::userGameLog(const QString &username)
{
	LOG_CTRACE("client") << "Get user game log" << username;

	if (username.isEmpty())
		return responseError("missing username");

	LAMBDA_THREAD_BEGIN(username);

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT date(timestamp) AS day, CAST(JULIANDAY(date('now'))-JULIANDAY(date(timestamp)) AS INTEGER) AS diff, "
					  "SUM(CASE WHEN success THEN 1 ELSE 0 END) AS success, COUNT(*) AS full FROM game "
					  "WHERE username=").addValue(username)
			.addQuery(" GROUP BY username, date(timestamp)").execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;

}



/**
 * @brief GeneralAPI::me
 * @return
 */

QHttpServerResponse GeneralAPI::me(const std::optional<Credential> &credential)
{
	return user(credential->username());
}

