/*
 * ---- Call of Suli ----
 *
 * studentapi.cpp
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentAPI
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

#include "userapi.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include "generalapi.h"
#include "qjsonarray.h"
#include "serverservice.h"
#include "teacherapi.h"

#include <QJsonObject>



/**
 * @brief StudentAPI::StudentAPI
 * @param service
 */

UserAPI::UserAPI(Handler *handler, ServerService *service)
	: AbstractAPI("user", handler, service)
{
	auto server = m_handler->httpServer().lock().get();

	Q_ASSERT(server);

	m_validateRole = Credential::Student;

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	server->route(path+"group", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::group, &*this, *credential);
	});

	server->route(path+"group/<arg>/score", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::groupScore, &*this, id);
	});

	server->route(path+"update", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::update, &*this, *credential, *jsonObject);
	});

	server->route(path+"password", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::password, &*this, *credential, *jsonObject);
	});


	server->route(path+"campaign", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::campaigns, &*this, *credential);
	});

	server->route(path+"campaign/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::campaign, &*this, *credential, id);
	});

	server->route(path+"campaign/<arg>/result", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_GET();
		return QtConcurrent::run(&UserAPI::campaignResult, &*this, *credential, id, jsonObject.value_or(QJsonObject{}));
	});


	server->route(path+"map/<arg>/solver", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::mapSolver, &*this, *credential, uuid);
	});

	server->route(path+"map", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::map, &*this, *credential);
	});

	server->route(path+"map/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::mapContent, &*this, *credential, uuid);
	});




	server->route(path+"game/info", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::gameInfo, &*this, *credential, *jsonObject);
	});

	server->route(path+"campaign/<arg>/game/create", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::gameCreate, &*this, *credential, id, *jsonObject);
	});

	server->route(path+"campaign/<arg>/game", QHttpServerRequest::Method::Put,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::gameCreate, &*this, *credential, id, *jsonObject);
	});

	server->route(path+"game/<arg>/update", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::gameUpdate, &*this, *credential, id, *jsonObject);
	});

	server->route(path+"game/<arg>/finish", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		JSON_OBJECT_ASSERT();
		return QtConcurrent::run(&UserAPI::gameFinish, &*this, *credential, id, *jsonObject);
	});


	server->route(path+"inventory", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_FUTURE_API();
		return QtConcurrent::run(&UserAPI::inventory, &*this, *credential);
	});

	/*server->route(path+"group/<arg>/score/live", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return responseFakeEventStream();
	});*/

}



/**
 * @brief UserAPI::group
 * @return
 */

QHttpServerResponse UserAPI::group(const Credential &credential)
{
	LOG_CTRACE("client") << "Get user groups";

	LAMBDA_THREAD_BEGIN(credential);

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT id, name, owner, familyName AS ownerFamilyName, givenName AS ownerGivenName FROM studentGroupInfo "
					  "LEFT JOIN user ON (user.username=studentGroupInfo.owner) "
					  "WHERE studentGroupInfo.active=true AND studentGroupInfo.username=").addValue(credential.username())
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::groupScore
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse UserAPI::groupScore(const int &id)
{
	LOG_CTRACE("client") << "Get user group score" << id;

	if (id < 0)
		return responseError("invalid id");

	const auto &list = getGroupScore(databaseMain(), id);

	if (list)
		return responseResult("list", *list);
	else
		return responseErrorSql();
}



/**
 * @brief UserAPI::campaigns
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::campaigns(const Credential &credential)
{
	LOG_CTRACE("client") << "Get user groups";

	LAMBDA_THREAD_BEGIN(credential);

	const auto &list = QueryBuilder::q(db)
			.addQuery("WITH studentList(username, campaignid) AS (SELECT username, campaignid FROM campaignStudent) "
					  "SELECT campaign.id AS id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
					  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
					  "description, finished, groupid,"
					  "score.xp AS resultXP, campaignResult.gradeid AS resultGrade "
					  "FROM campaign "
					  "LEFT JOIN campaignResult ON (campaignResult.campaignid=campaign.id AND campaignResult.username=").addValue(credential.username())
			.addQuery(") LEFT JOIN score ON (campaignResult.scoreid=score.id) "
					  "WHERE started=true AND groupid IN "
					  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
			.addQuery(") AND (NOT EXISTS(SELECT * FROM studentList WHERE studentList.campaignid=campaign.id) "
					  "OR EXISTS(SELECT * FROM studentList WHERE studentList.campaignid=campaign.id AND studentList.username=").addValue(credential.username())
			.addQuery("))")
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::campaign
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse UserAPI::campaign(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get user campaign";

	if (id < 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id);

	auto obj = QueryBuilder::q(db)
			.addQuery("WITH studentList(username, campaignid) AS (SELECT username, campaignid FROM campaignStudent) "
					  "SELECT campaign.id AS id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
					  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
					  "description, finished, groupid, defaultGrade, score.xp AS resultXP, campaignResult.gradeid AS resultGrade "
					  "FROM campaign LEFT JOIN campaignResult ON (campaignResult.campaignid=campaign.id	AND campaignResult.username=")
			.addValue(credential.username())
			.addQuery(") LEFT JOIN score ON (campaignResult.scoreid=score.id) "
					  "WHERE started=true AND campaign.id=").addValue(id)
			.addQuery(" AND groupid IN "
					  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
			.addQuery(") AND (NOT EXISTS(SELECT * FROM studentList WHERE studentList.campaignid=campaign.id) "
					  "OR EXISTS(SELECT * FROM studentList WHERE studentList.campaignid=campaign.id AND studentList.username=").addValue(credential.username())
			.addQuery("))")
			.execToJsonObject();

	LAMBDA_SQL_ASSERT(obj);

	LAMBDA_SQL_ERROR("not found", !obj->isEmpty());

	const bool &finished = obj->value(QStringLiteral("finished")).toVariant().toBool();
	const auto &result = TeacherAPI::_campaignUserResult(this, id, finished, credential.username(), true);

	LAMBDA_SQL_ASSERT(result);

	if (!finished) {
		obj->insert(QStringLiteral("resultXP"), result->xp > 0 ? result->xp : QJsonValue::Null);
		obj->insert(QStringLiteral("resultGrade"), result->grade > 0 ? result->grade : QJsonValue::Null);
	}

	obj->insert(QStringLiteral("taskList"), result->tasks);

	response = QHttpServerResponse(*obj);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::campaignResult
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::campaignResult(const Credential &credential, const int &id, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Get user campaign result" << id;

	LAMBDA_THREAD_BEGIN(credential, id, json);

	int offset = json.value(QStringLiteral("offset")).toInt(0);
	int limit = json.value(QStringLiteral("limit")).toInt(DEFAULT_LIMIT);

	const auto &list = TeacherAPI::_campaignUserGameResult(this, id, credential.username(), limit, offset);

	LAMBDA_SQL_ASSERT(list);

	response = QHttpServerResponse(QJsonObject{
									   { QStringLiteral("list"), *list },
									   { QStringLiteral("limit"), limit },
									   { QStringLiteral("offset"), offset },
								   });
	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::map
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::map(const Credential &credential)
{
	LOG_CTRACE("client") << "Get maps" << credential.username();

	LAMBDA_THREAD_BEGIN(credential);

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT mapdb.map.uuid, name, md5, "
					  "mapdb.cache.data AS cache, length(mapdb.map.data) as size "
					  "FROM mapdb.map LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
					  "WHERE mapdb.map.uuid IN "
					  "(SELECT mapuuid FROM task WHERE campaignid IN "
					  "(SELECT id FROM campaign WHERE groupid IN "
					  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
			.addQuery(")))")
			.execToJsonArray({
								 { QStringLiteral("cache"), [](const QVariant &v) {
									   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
								   } }
							 });


	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::mapContent
 * @param credential
 * @param uuid
 * @return
 */

QHttpServerResponse UserAPI::mapContent(const Credential &credential, const QString &uuid)
{
	LOG_CTRACE("client") << "Get map content" << uuid;

	LAMBDA_THREAD_BEGIN(credential, uuid);

	QueryBuilder q(db);

	q.addQuery("SELECT data FROM mapdb.map WHERE uuid=").addValue(uuid);

	LAMBDA_SQL_ASSERT(q.exec());

	LAMBDA_SQL_ERROR("not found", q.sqlQuery().first());

	response = QHttpServerResponse(q.sqlQuery().value(QStringLiteral("data")).toByteArray());

	LAMBDA_THREAD_END;
}




/**
 * @brief UserAPI::mapSolver
 * @param credential
 * @param uuid
 * @return
 */

QHttpServerResponse UserAPI::mapSolver(const Credential &credential, const QString &uuid)
{
	LOG_CTRACE("client") << "Get map solver" << uuid << credential.username();

	const auto &solver = solverInfo(this, credential.username(), uuid);

	if (!solver)
		return responseErrorSql();
	else {
		QJsonObject ret;

		for (auto it = solver->constBegin(); it != solver->constEnd(); ++it)
			ret.insert(it.key(), it.value().toJsonObject());

		return QHttpServerResponse(ret);
	}
}



/**
 * @brief UserAPI::gameInfo
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameInfo(const Credential &credential, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Get game info";

	LAMBDA_THREAD_BEGIN(credential, json);

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT game.username, COUNT(*) AS num, MAX(duration) AS dMax, MIN(duration) as dMin, "
					  "ROW_NUMBER() OVER (ORDER BY MIN(duration)) durationPos, "
					  "ROW_NUMBER() OVER (ORDER BY COUNT(*) DESC, MIN(duration)) numPos, "
					  "familyName, givenName, nickname, picture, rankid FROM game "
					  "LEFT JOIN user ON (user.username=game.username) "
					  "LEFT JOIN userRank ON (userRank.username=game.username) "
					  "WHERE success=true AND user.active=true ")
			.addQuery(" AND mapid=").addValue(json.value(QStringLiteral("map")).toString())
			.addQuery(" AND missionid=").addValue(json.value(QStringLiteral("mission")).toString())
			.addQuery(" AND game.level=").addValue(json.value(QStringLiteral("level")).toInt())
			.addQuery(" AND mode=").addValue(json.value(QStringLiteral("mode")).toInt())
			.addQuery(" AND deathmatch=").addValue(json.value(QStringLiteral("deathmatch")).toVariant().toBool())
			.addQuery(" GROUP BY game.username, mapid, missionid, game.level, mode, deathmatch")
			.execToJsonArray();


	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}




/**
 * @brief UserAPI::gameCreate
 * @param credential
 * @param campaignId
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameCreate(const Credential &credential, const int &campaign, const QJsonObject &json)
{
	TeacherAPI::UserGame g;

	g.map = json.value(QStringLiteral("map")).toString();
	g.mission = json.value(QStringLiteral("mission")).toString();
	g.level = json.value(QStringLiteral("level")).toInt(-1);
	g.deathmatch = json.value(QStringLiteral("deathmatch")).toVariant().toBool();
	g.mode = json.value(QStringLiteral("mode")).toVariant().value<GameMap::GameMode>();

	if (g.map.isEmpty() || g.mission.isEmpty())
		return responseError("missing map/mission");

	if (g.level < 0)
		return responseError("invalid level");

	if (g.mode == GameMap::Invalid)
		return responseError("invalid mode");



	LAMBDA_THREAD_BEGIN(campaign, g, json, credential);


	const QString &username = credential.username();

	LOG_CDEBUG("client") << "Create game for user:" << qPrintable(username) << "in campaign:" << campaign;

	LAMBDA_SQL_ERROR("invalid campaign",
					 QueryBuilder::q(db)
					 .addQuery("SELECT id FROM campaign WHERE started=true AND finished=false AND groupid IN "
							   "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
					 .addQuery(")")
					 .execCheckExists());



	db.transaction();

	// Close running games

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT gameid, xp FROM runningGame LEFT JOIN game ON (game.id=runningGame.gameid) WHERE username=")
			.addValue(username)
			.execToJsonArray();

	LAMBDA_SQL_ASSERT_ROLLBACK(list);


	for (const QJsonValue &v : qAsConst(*list)) {
		const QJsonObject &o = v.toObject();
		const int &gid = o.value(QStringLiteral("gameid")).toInt();
		const int &xp = o.value(QStringLiteral("xp")).toInt();

		LOG_CDEBUG("client") << "Close running game " << gid << "for user:" << qPrintable(username);

		int scoreId = -1;

		if (xp > 0) {
			const auto &s = QueryBuilder::q(db)
					.addQuery("INSERT INTO score (").setFieldPlaceholder()
					.addQuery(") VALUES (").setValuePlaceholder()
					.addQuery(")")
					.addField("username", username)
					.addField("xp", xp)
					.execInsertAsInt();

			LAMBDA_SQL_ASSERT_ROLLBACK(s);

			scoreId = *s;
		}

		//.addQuery("UPDATE game SET duration=((julianday(CURRENT_TIMESTAMP)-julianday(timestamp))*86400.0), success=false, "

		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("UPDATE game SET duration=NULL, success=false, "
											 "scoreid=")
								   .addValue(scoreId > 0 ? scoreId : QVariant(QMetaType::fromType<int>()))
								   .addQuery(" WHERE id=")
								   .addValue(gid)
								   .exec());
	}


	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("DELETE FROM runningGame WHERE gameid IN "
										 "(SELECT gameid FROM runningGame LEFT JOIN game ON (game.id=runningGame.gameid) WHERE username=")
							   .addValue(username)
							   .addQuery(")")
							   .exec());

	// Create game

	const auto &gameId = QueryBuilder::q(db)
			.addQuery("INSERT INTO game (").setFieldPlaceholder()
			.addQuery(") VALUES (").setValuePlaceholder()
			.addQuery(")")
			.addField("username", username)
			.addField("mapid", g.map)
			.addField("missionid", g.mission)
			.addField("campaignid", campaign)
			.addField("level", g.level)
			.addField("deathmatch", g.deathmatch)
			.addField("success", false)
			.addField("mode", g.mode)
			.execInsertAsInt();

	LAMBDA_SQL_ASSERT_ROLLBACK(gameId);

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("INSERT INTO runningGame (").setFieldPlaceholder()
							   .addQuery(") VALUES (").setValuePlaceholder()
							   .addQuery(")")
							   .addField("gameid", *gameId)
							   .addField("xp", 0)
							   .execInsert());



	QJsonObject obj;
	obj.insert(QStringLiteral("id"), *gameId);
	obj.insert(QStringLiteral("closedGames"), *list);


	if (g.mode == GameMap::Action && g.deathmatch) {
		const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();
		QJsonObject iList;

		for (auto it = inventory.constBegin(); it != inventory.constEnd(); ++it) {
			const int num = it.value().toInt(0);

			if (num <= 0)
				continue;

			int realNum = qMin(num, QueryBuilder::q(db).addQuery("SELECT value FROM inventory WHERE username=").addValue(username)
							   .addQuery(" AND key=").addValue(it.key())
							   .execToValue("value").value_or(0).toInt());

			const auto &e = QueryBuilder::q(db)
					.addQuery("WITH t AS (SELECT ").addValue(username)
					.addQuery(" AS username, ").addValue(it.key())
					.addQuery(" AS key, COALESCE((SELECT value FROM inventory WHERE username=").addValue(username)
					.addQuery(" AND key=").addValue(it.key())
					.addQuery("),0)-").addValue(realNum)
					.addQuery(" AS value) INSERT OR REPLACE INTO inventory(username, key, value) SELECT * FROM t")
					.exec();

			LAMBDA_SQL_ASSERT_ROLLBACK(e);

			iList.insert(it.key(), realNum);
		}

		obj.insert(QStringLiteral("extended"), iList);
	}

	db.commit();

	response = QHttpServerResponse(obj);

	LAMBDA_THREAD_END;
}




/**
 * @brief UserAPI::gameUpdate
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameUpdate(const Credential &credential, const int &id, const QJsonObject &json)
{
	const QString &username = credential.username();

	LOG_CTRACE("client") << "Update game" << id << "for user:" << qPrintable(username);

	LAMBDA_THREAD_BEGIN(username, json, id, credential);

	QueryBuilder qq(db);

	qq.addQuery("SELECT mapid, missionid, level, deathmatch, mode, campaignid FROM game "
				"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
				"WHERE runningGame.gameid=game.id AND game.id=").addValue(id)
			.addQuery(" AND username=").addValue(username);

	LAMBDA_SQL_ASSERT(qq.exec());

	LAMBDA_SQL_ERROR("invalid game", qq.sqlQuery().first());


	// Statistics

	if (json.contains(QStringLiteral("statistics")))
		_addStatistics(credential, json.value(QStringLiteral("statistics")).toArray());

	// XP

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db).addQuery("UPDATE runningGame SET xp=").addValue(json.value(QStringLiteral("xp")).toInt())
					  .addQuery(" WHERE gameid=").addValue(id).exec());

	response = responseOk();

	LAMBDA_THREAD_END;
}







/**
 * @brief UserAPI::gameFinish
 * @param credential
 * @param id
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameFinish(const Credential &credential, const int &id, const QJsonObject &json)
{
	const QString &username = credential.username();

	LOG_CDEBUG("client") << "Finish game" << id << "for user:" << qPrintable(username);

	LAMBDA_THREAD_BEGIN(username, json, id, credential);

	QueryBuilder qq(db);

	qq.addQuery("SELECT mapid, missionid, level, deathmatch, mode, campaignid FROM game "
				"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
				"LEFT JOIN campaign ON (campaign.id=game.campaignid) "
				"WHERE runningGame.gameid=game.id AND game.id=").addValue(id)
			.addQuery(" AND username=").addValue(username);

	LAMBDA_SQL_ASSERT(qq.exec());

	LAMBDA_SQL_ERROR("invalid game", qq.sqlQuery().first());


	// Statistics

	if (json.contains(QStringLiteral("statistics")))
		_addStatistics(credential, json.value(QStringLiteral("statistics")).toArray());


	TeacherAPI::UserGame g;

	g.map = qq.value("mapid").toString();
	g.mission = qq.value("missionid").toString();
	g.level = qq.value("level").toInt();
	g.deathmatch = qq.value("deathmatch").toBool();
	g.mode = qq.value("mode").value<GameMap::GameMode>();
	g.campaign = qq.value("campaignid", -1).toInt();



	const bool &success = json.value(QStringLiteral("success")).toVariant().toBool();
	const int &xp = json.value(QStringLiteral("xp")).toInt();
	const int &duration = json.value(QStringLiteral("duration")).toInt();
	int sumXP = xp;



	QJsonObject retObj;

	const int &baseXP = m_service->config().get("gameBaseXP").toInt(100);
	const int &oldSolved = _solverInfo(this, username, g.map, g.mission, g.level, g.deathmatch).value_or(0);

	if (success) {
		// Solved XP

		const int &xpSolved = GameMap::computeSolvedXpFactor(g.level, g.deathmatch, oldSolved, g.mode) * baseXP;

		sumXP += xpSolved;
		retObj[QStringLiteral("xpSolved")] = xpSolved;

		// Duration XP

		const auto &s = QueryBuilder::q(db)
				.addQuery("SELECT COALESCE(MIN(duration),0) AS duration FROM game "
						  "WHERE success=true AND username=").addValue(username)
				.addQuery(" AND mapid=").addValue(g.map)
				.addQuery(" AND missionid=").addValue(g.mission)
				.addQuery(" AND level=").addValue(g.level)
				.addQuery(" AND mode=").addValue(g.mode)
				.execToValue("duration");

		LAMBDA_SQL_ASSERT(s);

		const int &shortestDuration = s->toInt();

		if (shortestDuration > 0 && duration < shortestDuration) {
			const int &durationXP = (shortestDuration-duration)/1000 * baseXP * XP_FACTOR_DURATION_SEC;
			sumXP += durationXP;
			retObj[QStringLiteral("xpDuration")] = durationXP;
		}



		// Streak XP


		const auto &ss = QueryBuilder::q(db)
				.addQuery("SELECT COALESCE(MAX(streak),0) AS streak FROM streak WHERE username=").addValue(username)
				.execToValue("streak");

		LAMBDA_SQL_ASSERT(ss);

		const int &longestStreak = ss->toInt();

		QueryBuilder q(db);
		q.addQuery("SELECT COALESCE(streak, 0) AS streak, COALESCE((ended_on = date('now')), false) AS streakToday "
				   "FROM streak WHERE ended_on >= date('now', '-1 day') AND username=").addValue(username);

		LAMBDA_SQL_ASSERT(q.exec());

		const bool &hasFirst = q.sqlQuery().first();

		const bool &sToday = hasFirst ? q.value("streakToday", false).toBool() : false;
		const int &streak = hasFirst ? q.value("streak", 0).toInt() : 0;

		if (!sToday && streak > 0) {
			if (streak+1 > longestStreak) {
				const int &streakXP = (streak+1) * baseXP * XP_FACTOR_NEW_STREAK;
				sumXP += streakXP;
				retObj[QStringLiteral("longestStreak")] = true;
				retObj[QStringLiteral("xpStreak")] = streakXP;
			} else {
				const int &streakXP = (streak+1) * baseXP * XP_FACTOR_STREAK;
				sumXP += streakXP;
				retObj[QStringLiteral("longestStreak")] = false;
				retObj[QStringLiteral("xpStreak")] = streakXP;
			}
			retObj[QStringLiteral("streak")] = streak+1;
		}
	}

	retObj[QStringLiteral("sumXP")] = sumXP;
	retObj[QStringLiteral("xpGame")] = xp;

	retObj[QStringLiteral("success")] = success;
	retObj[QStringLiteral("id")] = id;


	db.transaction();

	const auto &scoreId = QueryBuilder::q(db)
			.addQuery("INSERT INTO score (").setFieldPlaceholder()
			.addQuery(") VALUES (").setValuePlaceholder()
			.addQuery(")")
			.addField("username", username)
			.addField("xp", sumXP)
			.execInsertAsInt();

	LAMBDA_SQL_ASSERT_ROLLBACK(scoreId);


	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("UPDATE game SET ").setCombinedPlaceholder()
							   .addField("duration", duration)
							   .addField("success", success)
							   .addField("scoreid", *scoreId)
							   .addQuery(" WHERE id=")
							   .addValue(id)
							   .exec());


	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db).addQuery("DELETE FROM runningGame WHERE gameid=").addValue(id).exec());


	// Inventory

	if (success && g.mode == GameMap::Action) {
		const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();
		QJsonArray iList;

		for (auto it = inventory.constBegin(); it != inventory.constEnd(); ++it) {
			const int num = it.value().toInt(0);

			if (num <= 0)
				continue;

			LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
									   .addQuery("WITH t AS (SELECT ").addValue(username)
									   .addQuery(" AS username, ").addValue(it.key())
									   .addQuery(" AS key, COALESCE((SELECT value FROM inventory WHERE username=").addValue(username)
									   .addQuery(" AND key=").addValue(it.key())
									   .addQuery("),0)+").addValue(num)
									   .addQuery(" AS value) INSERT OR REPLACE INTO inventory(username, key, value) SELECT * FROM t")
									   .exec());

			iList.append(QJsonObject{
							 { QStringLiteral("key"), it.key() },
							 { QStringLiteral("value"), it.value() }
						 });
		}

		retObj[QStringLiteral("inventory")] = iList;
	}

	db.commit();

	/*
	QueryBuilder qg(db);
	qg.addQuery("SELECT DISTINCT id FROM studentGroupInfo WHERE username=").addValue(username);

	if (qg.exec()) {
		while (qg.sqlQuery().next()) {
			const int groupId = qg.value("id", -1).toInt();

			if (groupId > -1)
				m_service->triggerEventStreams(EventStream::EventStreamGroupScore, groupId);
		}
	} else {
		LOG_CERROR("client") << "SQL error" << qg.sqlQuery().lastError();
	}
*/

	if (success) {
		LAMBDA_SQL_ASSERT(TeacherAPI::_evaluateCampaign(this, g.campaign, username));
	}

	response = responseOk(retObj);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::inventory
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::inventory(const Credential &credential)
{
	LOG_CTRACE("client") << "Get inventory" << credential.username();

	LAMBDA_THREAD_BEGIN(credential);

	const auto &list = QueryBuilder::q(db)
			.addQuery("SELECT key, value FROM inventory WHERE username=").addValue(credential.username())
			.execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}





/**
 * @brief UserAPI::update
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::update(const Credential &credential, const QJsonObject &json)
{
	const QString &username = credential.username();

	LOG_CTRACE("client") << "Modify user data:" << qPrintable(username);

	LAMBDA_THREAD_BEGIN(username, json);

	QueryBuilder q(db);

	q.addQuery("UPDATE user SET ").setCombinedPlaceholder();

	if (m_service->config().nameUpdateEnabled()) {
		if (json.contains(QStringLiteral("familyName")))	q.addField("familyName", json.value(QStringLiteral("familyName")).toString());
		if (json.contains(QStringLiteral("givenName")))		q.addField("givenName", json.value(QStringLiteral("givenName")).toString());
		if (json.contains(QStringLiteral("picture")))		q.addField("picture", json.value(QStringLiteral("picture")).toString());
	}

	if (json.contains(QStringLiteral("nickname")))	q.addField("nickname", json.value(QStringLiteral("nickname")).toString());
	if (json.contains(QStringLiteral("character")))		q.addField("character", json.value(QStringLiteral("character")).toString());

	q.addQuery(" WHERE username=").addValue(username);

	LAMBDA_SQL_ASSERT(q.fieldCount() && !q.exec());

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::password
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::password(const Credential &credential, const QJsonObject &json)
{
	const QString &username = credential.username();
	const QString &password = json.value(QStringLiteral("password")).toString();

	LOG_CTRACE("client") << "Change password for user:" << qPrintable(username);

	if (password.isEmpty())
		return responseError("missing password");

	if (AdminAPI::authPlainPasswordChange(this, username, json.value(QStringLiteral("oldPassword")).toString(), password, true))
		return responseOk();
	else
		return responseError("failed");
}



/**
 * @brief UserAPI::_addStatistics
 * @param username
 * @param list
 */

void UserAPI::_addStatistics(const Credential &credential, const QJsonArray &list) const
{
	if (list.isEmpty())
		return;

	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker _locker(databaseMain()->mutex());

	for (const QJsonValue &v : qAsConst(list)) {
		const QJsonObject &o = v.toObject();

		QueryBuilder q(db);

		q.addQuery("INSERT INTO statdb.statistics (").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")");

		if (o.contains(QStringLiteral("map")))
			q.addField("map", o.value(QStringLiteral("map")).toString());

		if (o.contains(QStringLiteral("mode")))
			q.addField("mode", o.value(QStringLiteral("mode")).toInt());

		if (o.contains(QStringLiteral("objective")))
			q.addField("objective", o.value(QStringLiteral("objective")).toString());

		if (o.contains(QStringLiteral("success")))
			q.addField("success", o.value(QStringLiteral("success")).toVariant().toBool());

		if (o.contains(QStringLiteral("elapsed")))
			q.addField("elapsed", o.value(QStringLiteral("elapsed")).toInt());

		if (o.contains(QStringLiteral("module")))
			q.addField("module", o.value(QStringLiteral("module")).toString());

		if (q.fieldCount())
			q.addField("username", credential.username());
		else
			continue;

		q.exec();
	}

}








/**
 * @brief UserAPI::getGroupScore
 * @param database
 * @param id
 * @return
 */

std::optional<QJsonArray> UserAPI::getGroupScore(const DatabaseMain *database, const int &id)
{
	Q_ASSERT (database);

	LOG_CTRACE("client") << "Get group score:" << id;

	QDefer ret;

	std::optional<QJsonArray> list;

	database->worker()->execInThread([ret, id, database, &list]() mutable {
		QSqlDatabase db = QSqlDatabase::database(database->dbName());

		QMutexLocker _locker(database->mutex());

		list = QueryBuilder::q(db)
				.addQuery(_SQL_get_user)
				.addQuery("WHERE active=true AND user.username IN (SELECT username FROM studentGroupInfo WHERE active=true AND id=")
				.addValue(id)
				.addQuery(")")
				.execToJsonArray();

		ret.resolve();
	});

	QDefer::await(ret);

	return list;
}










/*
* @brief UserAPI::solverInfo
* @param api
* @param username
* @param map
* @return
*/

std::optional<QMap<QString, GameMap::SolverInfo> > UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map)
{
	Q_ASSERT(api);

	QDefer ret;

	QMap<QString, GameMap::SolverInfo> solver;

	api->databaseMainWorker()->execInThread([api, username, map, ret, &solver]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

		QueryBuilder q(db);

		q.addQuery("SELECT missionid, level, deathmatch, COUNT(*) AS num FROM game WHERE username=").addValue(username)
				.addQuery(" AND success=true")
				.addQuery(" AND mapid=").addValue(map)
				.addQuery(" GROUP BY missionid, level, deathmatch");

		if (!q.exec())
			return ret.reject();

		while (q.sqlQuery().next()) {
			const QString &mission = q.value("missionid").toString();

			GameMap::SolverInfo s;

			if (solver.contains(mission))
				s = solver.value(mission);

			s.setSolved(q.value("level").toInt(), q.value("deathmatch").toBool(), q.value("num").toInt());

			solver.insert(mission, s);
		}

		ret.resolve();
	});

	QDefer::await(ret);

	if (ret.state() == RESOLVED)
		return solver;
	else
		return std::nullopt;
}






/**
* @brief UserAPI::solverInfo
* @param api
* @param username
* @param map
* @param mission
* @return
*/

std::optional<GameMap::SolverInfo> UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission)
{
	Q_ASSERT(api);

	QDefer ret;

	GameMap::SolverInfo solver;

	api->databaseMainWorker()->execInThread([api, username, map, mission, ret, &solver]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

		QueryBuilder q(db);

		q.addQuery("SELECT level, deathmatch, COUNT(*) AS num FROM game WHERE username=").addValue(username)
				.addQuery(" AND success=true")
				.addQuery(" AND mapid=").addValue(map)
				.addQuery(" AND missionid=").addValue(mission)
				.addQuery(" GROUP BY level, deathmatch");

		if (!q.exec())
			return ret.reject();

		while (q.sqlQuery().next())
			solver.setSolved(q.value("level").toInt(), q.value("deathmatch").toBool(), q.value("num").toInt());

		ret.resolve();
	});

	QDefer::await(ret);

	if (ret.state() == RESOLVED)
		return solver;
	else
		return std::nullopt;
}




/**
* @brief UserAPI::solverInfo
* @param api
* @param username
* @param map
* @param mission
* @param level
* @return
*/

std::optional<int> UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission, const int &level)
{
	Q_ASSERT(api);

	QDefer ret;

	int retValue = -1;

	api->databaseMainWorker()->execInThread([api, username, map, mission, ret, level, &retValue]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker _locker(api->databaseMain()->mutex());

		const auto &n = QueryBuilder::q(db)
				.addQuery("SELECT COUNT(*) AS num FROM game WHERE username=").addValue(username)
				.addQuery(" AND success=true")
				.addQuery(" AND mapid=").addValue(map)
				.addQuery(" AND missionid=").addValue(mission)
				.addQuery(" AND level=").addValue(level)
				.execToValue("num");

		if (n) {
			retValue = n->toInt();
			ret.resolve();
		}

		ret.reject();
	});

	QDefer::await(ret);

	if (ret.state() == RESOLVED)
		return retValue;
	else
		return std::nullopt;
}



/**
* @brief UserAPI::solverInfo
* @param api
* @param username
* @param map
* @param mission
* @param level
* @param deathmatch
* @return
*/

std::optional<int> UserAPI::_solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission, const int &level,
										const bool &deathmatch)
{
	Q_ASSERT(api);

	QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

	QMutexLocker _locker(api->databaseMain()->mutex());

	const auto &n = QueryBuilder::q(db)
			.addQuery("SELECT COUNT(*) AS num FROM game WHERE username=").addValue(username)
			.addQuery(" AND success=true")
			.addQuery(" AND mapid=").addValue(map)
			.addQuery(" AND missionid=").addValue(mission)
			.addQuery(" AND level=").addValue(level)
			.addQuery(" AND deathmatch=").addValue(deathmatch)
			.execToValue("num");

	if (n)
		return n->toInt();

	return std::nullopt;

}
