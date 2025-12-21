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
#include "commonsettings.h"
#include "generalapi.h"
#include "qjsonarray.h"
#include "serverservice.h"
#include "teacherapi.h"
#include "rpgengine.h"

#include <QJsonObject>
#include "querybuilder.hpp"



/**
 * @brief StudentAPI::StudentAPI
 * @param service
 */

UserAPI::UserAPI(Handler *handler, ServerService *service)
	: AbstractAPI("user", handler, service)
{
	auto server = m_handler->httpServer();

	Q_ASSERT(server);

	m_validateRole = Credential::Student;

	const QByteArray path = QByteArray(m_apiPath).append(m_path).append(QByteArrayLiteral("/"));

	server->route(path+"group", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return group(*credential);
	});

	server->route(path+"exam", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return exam(*credential, -1);
	});

	server->route(path+"freeplay", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return freePlay(*credential);
	});

	server->route(path+"group/<arg>/score", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return groupScore(id);
	});

	server->route(path+"group/<arg>/exam", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return exam(*credential, id);
	});

	server->route(path+"update", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return update(*credential, *jsonObject);
	});

	server->route(path+"password", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return password(*credential, *jsonObject);
	});

	server->route(path+"notification", QHttpServerRequest::Method::Post | QHttpServerRequest::Method::Get,
				  [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return notification(*credential);
	});

	server->route(path+"notification/update", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return notificationUpdate(*credential, *jsonObject);
	});


	server->route(path+"campaign", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return campaigns(*credential);
	});

	server->route(path+"campaign/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return campaign(*credential, id);
	});

	server->route(path+"campaign/<arg>/result", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_GET();
		return campaignResult(*credential, id, jsonObject.value_or(QJsonObject{}));
	});



	server->route(path+"pass", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return passes(*credential);
	});

	server->route(path+"pass/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return pass(*credential, id);
	});



	server->route(path+"map/<arg>/solver", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return mapSolver(*credential, uuid);
	});

	server->route(path+"map", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return map(*credential);
	});

	server->route(path+"map/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &uuid, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return mapContent(*credential, uuid);
	});




	server->route(path+"game/info", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameInfo(*credential, *jsonObject);
	});

	server->route(path+"campaign/<arg>/game/create", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameCreate(*credential, id, *jsonObject);
	});

	server->route(path+"campaign/<arg>/game", QHttpServerRequest::Method::Put,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameCreate(*credential, id, *jsonObject);
	});

	server->route(path+"campaign/<arg>/game/token", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameTokenCreate(*credential, id, *jsonObject);
	});

	server->route(path+"campaign/<arg>/game/close", QHttpServerRequest::Method::Post,
				  [this](const int &/*id*/, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameClose(*credential, *jsonObject);
	});

	server->route(path+"game/<arg>/update", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameUpdate(*credential, id, *jsonObject);
	});

	server->route(path+"game/<arg>/finish", QHttpServerRequest::Method::Post,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameFinish(*credential, id, *jsonObject);
	});


	server->route(path+"inventory", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return inventory(*credential);
	});


	server->route(path+"wallet", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return wallet(*credential);
	});

	server->route(path+"buy", QHttpServerRequest::Method::Post,
				  [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return buy(*credential, *jsonObject);
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
 * @brief UserAPI::passes
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::passes(const Credential &credential)
{
	LOG_CTRACE("client") << "Get user passes";

	LAMBDA_THREAD_BEGIN(credential);

	const auto &list = QueryBuilder::q(db)
					   .addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, groupid, "
								 "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, title, grading, childless, pts, maxPts "
								 "FROM pass LEFT JOIN passSumResult ON (passSumResult.passid=pass.id AND passSumResult.username=")
					   .addValue(credential.username())
					   .addQuery(") WHERE starttime IS NOT NULL AND strftime('%s', starttime)<=strftime('%s', datetime('now')) AND groupid IN "
								 "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
					   .addQuery(")")
					   .execToJsonArray({
											{ QStringLiteral("grading"), [](const QVariant &v) {
												  return QJsonDocument::fromJson(v.toString().toUtf8()).object();
											  } }
										});

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::pass
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse UserAPI::pass(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get user pass" << id;

	LAMBDA_THREAD_BEGIN(credential, id);

	auto data = QueryBuilder::q(db)
					   .addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, groupid, "
								 "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, title, grading, childless, pts, maxPts "
								 "FROM pass LEFT JOIN passSumResult ON (passSumResult.passid=pass.id AND passSumResult.username=")
					   .addValue(credential.username())
					   .addQuery(") WHERE starttime IS NOT NULL AND strftime('%s', starttime)<=strftime('%s', datetime('now')) AND groupid IN "
								 "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
					   .addQuery(") AND id=").addValue(id)
					   .execToJsonObject({
											{ QStringLiteral("grading"), [](const QVariant &v) {
												  return QJsonDocument::fromJson(v.toString().toUtf8()).object();
											  } }
										});

	LAMBDA_SQL_ASSERT(data);

	const auto &list = QueryBuilder::q(db)
					   .addQuery("SELECT passHierarchy.passitemid AS id, result, description, pts, maxPts, extra, category, categoryid "
								 "FROM passHierarchy "
								 "JOIN passResultUser ON (passResultUser.passitemid=passHierarchy.passitemid "
								 "AND passResultUser.username=").addValue(credential.username())
					   .addQuery(") WHERE passid=").addValue(id)
					   .execToJsonArray();

	LAMBDA_SQL_ASSERT(list);

	data->insert(QStringLiteral("items"), *list);

	response = responseOk(*data);

	LAMBDA_THREAD_END;
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
								 "score.xp AS resultXP, campaignResult.gradeid AS resultGrade, maxPts, progress "
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
						 "description, finished, groupid, defaultGrade, score.xp AS resultXP, campaignResult.gradeid AS resultGrade,"
						 "maxPts, progress "
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
	const auto &result = TeacherAPI::_campaignUserResult(this, id, finished, credential.username());

	LAMBDA_SQL_ASSERT(result);

	if (!finished) {
		obj->insert(QStringLiteral("resultXP"), result->xp > 0 ? result->xp : QJsonValue::Null);
		obj->insert(QStringLiteral("resultGrade"), result->grade > 0 ? result->grade : QJsonValue::Null);
		if (result->maxPts > 0) {
			obj->insert(QStringLiteral("maxPts"), result->maxPts);
			obj->insert(QStringLiteral("progress"), result->progress);
		} else {
			obj->insert(QStringLiteral("maxPts"), QJsonValue::Null);
			obj->insert(QStringLiteral("progress"), QJsonValue::Null);
		}
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
 * @brief UserAPI::freePlay
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::freePlay(const Credential &credential)
{
	LOG_CTRACE("client") << "Get user freeplays";

	LAMBDA_THREAD_BEGIN(credential);

	const auto &ptr = QueryBuilder::q(db)
					  .addQuery("SELECT DISTINCT mapuuid, mission FROM freeplay WHERE groupid IN ("
								"SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
					  .addQuery(")")
					  .execToJsonArray();

	LAMBDA_SQL_ASSERT(ptr);

	response = responseResult("list", *ptr);

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
					   .addQuery("))) OR mapdb.map.uuid IN "
								 "(SELECT mapuuid FROM examContent LEFT JOIN exam ON (examContent.examid=exam.id) "
								 "WHERE examContent.username=").addValue(credential.username())
					   .addQuery(") OR mapdb.map.uuid IN "
								 "(SELECT mapuuid FROM freeplay WHERE groupid IN ("
								 "SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(credential.username())
					   .addQuery("))")
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
			ret.insert(it.key(), it.value().toJsonArray());

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
	UserGame g;

	g.map = json.value(QStringLiteral("map")).toString();
	g.mission = json.value(QStringLiteral("mission")).toString();
	g.level = json.value(QStringLiteral("level")).toInt(-1);
	g.mode = json.value(QStringLiteral("mode")).toVariant().value<GameMap::GameMode>();

	if (g.map.isEmpty() || g.mission.isEmpty())
		return responseError("missing map/mission");

	if (g.level < 0)
		return responseError("invalid level");

	if (g.mode == GameMap::Invalid)
		return responseError("invalid mode");


	const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();

	return gameCreate(credential.username(), campaign, g, inventory);
}



/**
 * @brief UserAPI::gameCreate
 * @param credential
 * @param campaign
 * @param game
 * @param inventory
 * @param okPtr
 * @return
 */

QHttpServerResponse UserAPI::gameCreate(const QString &username, const int &campaign,
										const UserGame &game, const QJsonObject &inventory, int *gameIdPtr)
{
	if (gameIdPtr)
		*gameIdPtr = -1;

	LAMBDA_THREAD_BEGIN(campaign, game, inventory, username, gameIdPtr);

	LOG_CDEBUG("client") << "Create game for user:" << qPrintable(username) << "in campaign:" << campaign;

	if (campaign > 0) {
		LAMBDA_SQL_ERROR("invalid campaign",
						 QueryBuilder::q(db)
						 .addQuery("SELECT id FROM campaign WHERE started=true AND finished=false AND groupid IN "
								   "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
						 .addQuery(")")
						 .execCheckExists());
	}



	db.transaction();

	// Close running games

	const auto &list = QueryBuilder::q(db)
					   .addQuery("SELECT gameid, xp FROM runningGame LEFT JOIN game ON (game.id=runningGame.gameid) WHERE username=")
					   .addValue(username)
					   .execToJsonArray();

	LAMBDA_SQL_ASSERT_ROLLBACK(list);


	for (const QJsonValue &v : std::as_const(*list)) {
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
						 .addField("mapid", game.map)
						 .addField("missionid", game.mission)
						 .addField("campaignid", campaign > 0 ? campaign : QVariant(QMetaType::fromType<int>()))
						 .addField("level", game.level)
						 .addField("success", false)
						 .addField("mode", game.mode)
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

	/*
	if (game.mode == GameMap::Action && game.deathmatch) {
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
	}*/

	db.commit();

	response = QHttpServerResponse(obj);

	if (gameIdPtr)
		*gameIdPtr = *gameId;

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::gameTokenCreate
 * @param credential
 * @param campaign
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameTokenCreate(const Credential &credential, const int &campaign, const QJsonObject &json)
{
	RpgConfigBase g;

	g.fromJson(json);
	g.campaign = campaign;

	if (g.mapUuid.isEmpty() || g.missionUuid.isEmpty())
		return responseError("missing map/mission");

	if (g.missionLevel < 0)
		return responseError("invalid level");

	if (g.campaign < 0)
		return responseError("invalid campaign");

	UdpServer *udpServer = m_service->udpServer();

	if (!udpServer)
		return responseError("internal error");


	RpgGameData::ConnectionToken token;
	QDateTime exp = QDateTime::currentDateTimeUtc();
	exp = exp.addSecs(120);

	quint32 id = 0;
	std::shared_ptr<RpgEngine> engine = RpgEngine::peerFind(udpServer, credential.username(), &id);

	if (id > 0) {
		LOG_CINFO("engine") << "SEAT EXISTS" << credential.username() << id << engine.get();

		if (engine) {
			LOG_CERROR("engine") << "SEAT ENGINE EXISTS" << credential.username() << id << engine->id();
			return responseResult("error", QStringLiteral("active/%1/%2").arg(id).arg(engine->id()));
		}

		token.peer = udpServer->resetPeer(id, credential.username(), exp);
	} else {
		token.peer = udpServer->addPeer(credential.username(), exp);
	}

	if (token.peer == 0) {
		return responseError("player create error");
	}

	token.user = credential.username();
	token.config = g;
	token.exp = exp.toSecsSinceEpoch();


	Token jwt;

	jwt.setSecret(m_service->settings()->jwtSecret());
	jwt.setPayload(token.toJson());

	return responseResult("token", QString::fromUtf8(jwt.getToken()));
}



/**
 * @brief UserAPI::gameClose
 * @param credential
 * @param campaign
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameClose(const Credential &credential, const QJsonObject &json)
{
	UdpServer *udpServer = m_service->udpServer();

	if (!udpServer)
		return responseError("internal error");

	const quint32 id = (quint32) json.value(QStringLiteral("seat")).toInteger();
	const int engineId = json.value(QStringLiteral("engine")).toInteger();

	quint32 idPtr = 0;

	std::shared_ptr<RpgEngine> engine = RpgEngine::peerFind(udpServer, credential.username(), &idPtr);

	LOG_CINFO("engine") << "???" << id << idPtr << engineId << engine->id();

	if (idPtr == 0 || !engine || idPtr != id || engine->id() != engineId) {
		return responseError("invalid id");
	}

	QMetaObject::invokeMethod(engine.get(), std::bind(&RpgEngine::peerAbort, engine.get(), id), Qt::QueuedConnection);

	return responseOk();
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

	LAMBDA_THREAD_BEGIN(username, json, id);

	QueryBuilder qq(db);

	qq.addQuery("SELECT mapid, missionid, level, deathmatch, mode, campaignid FROM game "
				"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
				"WHERE runningGame.gameid=game.id AND game.id=").addValue(id)
			.addQuery(" AND username=").addValue(username);

	LAMBDA_SQL_ASSERT(qq.exec());

	LAMBDA_SQL_ERROR("invalid game", qq.sqlQuery().first());


	// Statistics

	if (json.contains(QStringLiteral("statistics")))
		_addStatistics(username, json.value(QStringLiteral("statistics")).toArray());

	// Wallet, currency

	_addWallet(username, id, json.value(QStringLiteral("wallet")).toArray());
	if (json.contains(QStringLiteral("currency")))
		_setCurrency(username, id, json.value(QStringLiteral("currency")).toInt());

	// XP

	LAMBDA_SQL_ASSERT(QueryBuilder::q(db).addQuery("UPDATE runningGame SET xp=").addValue(json.value(QStringLiteral("xp")).toInt())
					  .addQuery(" WHERE gameid=").addValue(id).exec());

	response = responseOk();

	LAMBDA_THREAD_END;
}


/**
 * @brief UserAPI::gameUpdateStatistics
 * @param credential
 * @param id
 * @param statistics
 * @return
 */

QHttpServerResponse UserAPI::gameUpdateStatistics(const QString &username, const QJsonArray &statistics)
{
	LOG_CTRACE("client") << "Update game statistics for user:" << qPrintable(username);

	LAMBDA_THREAD_BEGIN(username, statistics);

	_addStatistics(username, statistics);

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
	UserGame g;

	////LAMBDA_THREAD_BEGIN(username, id, &g);

	QDefer ret;
	QHttpServerResponse response(QHttpServerResponse::StatusCode::InternalServerError);

	databaseMainWorker()->execInThread([&response, ret, this, username, id, &g, json]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());
		QMutexLocker _locker(databaseMain()->mutex());

		QueryBuilder qq(db);

		qq.addQuery("SELECT mapid, missionid, level, mode, campaignid, passitemid FROM game "
					"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
					"LEFT JOIN campaign ON (campaign.id=game.campaignid) "
					"WHERE runningGame.gameid=game.id AND game.id=").addValue(id)
				.addQuery(" AND username=").addValue(username);

		LAMBDA_SQL_ASSERT(qq.exec());

		LAMBDA_SQL_ERROR("invalid game", qq.sqlQuery().first());

		g.map = qq.value("mapid").toString();
		g.mission = qq.value("missionid").toString();
		g.level = qq.value("level").toInt();
		g.mode = qq.value("mode").value<GameMap::GameMode>();
		g.campaign = qq.value("campaignid", -1).toInt();
		g.passitemid = qq.value("passitemid", -1).toInt();


		// Wallet, currency

		_addWallet(username, id, json.value(QStringLiteral("wallet")).toArray());
		if (json.contains(QStringLiteral("currency")))
			_setCurrency(username, id, json.value(QStringLiteral("currency")).toInt());

		///LAMBDA_THREAD_END;				/// Nem lehet!!!

		ret.resolve();
	});
	QDefer::await(ret);

	const QJsonArray &statistics = json.value(QStringLiteral("statistics")).toArray();
	const int &duration = json.value(QStringLiteral("duration")).toInt();

	if (ret.state() == QDeferredState::RESOLVED) {
		const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();
		const bool &success = json.value(QStringLiteral("success")).toVariant().toBool();
		const int &xp = json.value(QStringLiteral("xp")).toInt();

		return gameFinish(username, id, g, inventory, statistics, success, xp, duration);
	} else {
		return gameFinish(username, id, g, {}, statistics, false, 0, duration);
	}
}




/**
 * @brief UserAPI::gameFinish
 * @param credential
 * @param game
 * @param inventory
 * @param okPtr
 * @return
 */

QHttpServerResponse UserAPI::gameFinish(const QString &username, const int &id, const UserGame &game,
										const QJsonObject &inventory, const QJsonArray &statistics,
										const bool &success, const int &xp, const int &duration,
										bool *okPtr, QPointer<RpgEngine> engine)
{
	if (okPtr)
		*okPtr = false;

	LOG_CDEBUG("client") << "Finish game" << id << "for user:" << qPrintable(username) << "success:" << success;

	LAMBDA_THREAD_BEGIN(username, statistics, id, inventory, xp, duration, success, game, okPtr, engine);

	// Statistics

	if (!statistics.isEmpty())
		_addStatistics(username, statistics);

	int sumXP = xp;

	QJsonObject retObj;

	const int &baseXP = m_service->config().get("gameBaseXP").toInt(100);
	const int &oldSolved = _solverInfo(this, username, game.map, game.mission, game.level).value_or(0);

	if (success) {
		// Solved XP

		const int &xpSolved = GameMap::computeSolvedXpFactor(game.level, oldSolved, game.mode) * baseXP;

		sumXP += xpSolved;
		retObj[QStringLiteral("xpSolved")] = xpSolved;

		// Duration XP

		const auto &s = QueryBuilder::q(db)
						.addQuery("SELECT COALESCE(MIN(duration),0) AS duration FROM game "
								  "WHERE success=true AND username=").addValue(username)
						.addQuery(" AND mapid=").addValue(game.map)
						.addQuery(" AND missionid=").addValue(game.mission)
						.addQuery(" AND level=").addValue(game.level)
						.addQuery(" AND mode=").addValue(game.mode)
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

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db).addQuery("DELETE FROM runningGame WHERE gameid=").addValue(id).exec());

	if (sumXP <= 0 && duration < 5 && !success) {
		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("DELETE FROM game WHERE id=")
								   .addValue(id)
								   .exec());
	} else {
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
	}




	// Inventory

	/*if (success && game.mode == GameMap::Action) {
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
									   .addQuery(" AS value) INSERT OR REPLACE INTO inventory(username, key, value) "
												 "SELECT username, key, MIN(value, COALESCE((SELECT value FROM inventoryLimit WHERE key=t.key), 999)) "
												 "FROM t")
									   .exec());

			iList.append(QJsonObject{
							 { QStringLiteral("key"), it.key() },
							 { QStringLiteral("value"), it.value() }
						 });
		}

		retObj[QStringLiteral("inventory")] = iList;
	}*/

	if (success && game.mode == GameMap::Rpg) {
		QJsonArray iList;

		for (auto it = inventory.constBegin(); it != inventory.constEnd(); ++it) {
			if (it.key() == QStringLiteral("map")) {
				LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
										   .addQuery("INSERT OR IGNORE INTO wallet(").setFieldPlaceholder()
										   .addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
										   .addField("username", username)
										   .addField("type", (int) RpgMarket::Map)
										   .addField("name", it.value().toString())
										   .addField("amount", 1)
										   .exec()
										   );


				LOG_CDEBUG("client") << "Achieved map:" << it.value().toString() << qPrintable(username);
			}
		}
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
		LAMBDA_SQL_ASSERT(TeacherAPI::_evaluateCampaign(this, game.campaign, username, game.passitemid));
	}


	// Clear wallet

	TeacherAPI::_clearWallet(databaseMain(), m_service);

	response = responseOk(retObj);

	if (okPtr)
		*okPtr = true;


	if (engine)
		QMetaObject::invokeMethod(engine, std::bind(&RpgEngine::playerSetFinal, engine, id, retObj), Qt::QueuedConnection);

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
 * @brief UserAPI::exam
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse UserAPI::exam(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Get exams for" << credential.username() << "in group:" << id;

	LAMBDA_THREAD_BEGIN(credential, id);

	QueryBuilder q(db);
	q.addQuery("SELECT exam.id, mode, state, mapuuid, description, CAST(strftime('%s', timestamp) AS INTEGER) AS timestamp, data, result, gradeid, answer, correction "
			   "FROM exam LEFT JOIN examContent ON (examContent.examid=exam.id AND username=").addValue(credential.username())
			.addQuery(") LEFT JOIN examAnswer ON (examAnswer.contentid=examContent.id) "
					  "WHERE state>=4 AND (username IS NOT NULL OR mode=2)");

	if (id > 0) {
		q.addQuery(" AND exam.groupid=").addValue(id);
	}

	const auto &list = q.execToJsonArray({
											 { QStringLiteral("data"), [](const QVariant &v) {
												   return QJsonDocument::fromJson(v.toString().toUtf8()).array();
											   } },
											 { QStringLiteral("answer"), [](const QVariant &v) {
												   return QJsonDocument::fromJson(v.toString().toUtf8()).array();
											   } },
											 { QStringLiteral("correction"), [](const QVariant &v) {
												   return QJsonDocument::fromJson(v.toString().toUtf8()).array();
											   } }
										 });

	LAMBDA_SQL_ASSERT(list);

	response = responseResult("list", *list);

	LAMBDA_THREAD_END;
}




/**
 * @brief UserAPI::wallet
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::wallet(const Credential &credential)
{
	LOG_CTRACE("client") << "Get wallet for" << credential.username();

	LAMBDA_THREAD_BEGIN(credential);

	const auto &ptr = TeacherAPI::_wallet(this, credential.username());

	LAMBDA_SQL_ASSERT(ptr);


	const auto &ptrC = TeacherAPI::_currency(this, credential.username());

	LAMBDA_SQL_ASSERT(ptrC);

	QJsonArray list;

	for (const RpgWallet &w : *ptr) {
		list.append(w.toJson());
	}

	QJsonObject r {
		{ QStringLiteral("list"), list },
		{ QStringLiteral("currency"), ptrC.value() }
	};

	response = responseOk(r);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::buy
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::buy(const Credential &credential, const QJsonObject &json)
{
	LOG_CTRACE("client") << "Buy item for" << credential.username();

	LAMBDA_THREAD_BEGIN(credential, json);

	RpgWallet w;
	w.fromJson(json);

	LAMBDA_SQL_ERROR("invalid type", w.type != RpgMarket::Invalid);
	LAMBDA_SQL_ERROR("invalid amount", w.amount > 0);

	db.transaction();

	const auto &currency = TeacherAPI::_currency(this, credential.username());

	LAMBDA_SQL_ASSERT_ROLLBACK(currency);

	const auto &list = m_service->market().list;

	const auto it = std::find_if(list.constBegin(), list.constEnd(), [&w](const RpgMarket &m){
		return w.isEqual(m);
	});

	LAMBDA_SQL_ERROR_ROLLBACK("invalid type", it != list.constEnd());


	// Check currency, rank, rollover

	const RpgMarket &market = *it;

	const int cost = market.cost * w.amount;

	LAMBDA_SQL_ERROR_ROLLBACK("insufficient currency", cost <= currency.value());

	const auto &rankPtr = QueryBuilder::q(db).addQuery("SELECT rankid FROM userRank WHERE username=").addValue(credential.username())
						  .execToValue("rankid", 0);

	LAMBDA_SQL_ASSERT_ROLLBACK(rankPtr);

	LAMBDA_SQL_ERROR_ROLLBACK("insufficient rank", rankPtr->toInt() >= market.rank);

	if (market.rollover == RpgMarket::Game) {
		const int gameid = json.value(QStringLiteral("gameid")).toInt(0);

		LAMBDA_SQL_ERROR_ROLLBACK("missing gameid", gameid > 0);

		const auto &ptr = QueryBuilder::q(db).addQuery("SELECT amount FROM wallet WHERE username=").addValue(credential.username())
						  .addQuery(" AND type=").addValue(w.type)
						  .addQuery(" AND name=").addValue(w.name)
						  .addQuery(" AND gameid=").addValue(gameid)
						  .execToValue("amount", 0);

		LAMBDA_SQL_ASSERT_ROLLBACK(ptr);

		LAMBDA_SQL_ERROR_ROLLBACK("game limit reached", ptr->toInt() < market.num);

		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("INSERT OR REPLACE INTO wallet(").setFieldPlaceholder()
								   .addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
								   .addField("username", credential.username())
								   .addField("type", (int) w.type)
								   .addField("name", w.name)
								   .addField("amount", ptr->toInt() + w.amount * market.amount)
								   .addField("gameid", gameid)
								   .exec()
								   );


		LOG_CDEBUG("client") << "Buy" << qPrintable(credential.username()) << w.type << qPrintable(w.name)
							 << "gameid:" << gameid
							 << "amount:" << w.amount << "cost:" << cost;

	} else if (market.rollover == RpgMarket::Day) {
		const auto &ptr = QueryBuilder::q(db).addQuery("SELECT SUM(amount) AS amount FROM wallet WHERE username=").addValue(credential.username())
						  .addQuery(" AND type=").addValue(w.type)
						  .addQuery(" AND name=").addValue(w.name)
						  .addQuery(" AND date(timestamp)=date('now')")
						  .execToValue("amount", 0);

		LAMBDA_SQL_ASSERT_ROLLBACK(ptr);

		LAMBDA_SQL_ERROR_ROLLBACK("daily limit reached", ptr->toInt() < market.num);
	}


	if (market.rollover != RpgMarket::Game) {
		QueryBuilder q(db);

		// Itt kell, különben az exec() már nem éri el a memóriában

		QByteArray b = QByteArrayLiteral("datetime('now', '+")+
					   QByteArray::number(market.exp)+
					   QByteArrayLiteral(" minutes')");

		if (market.exp > 0) {
			q.addQuery("INSERT INTO wallet(expiry,").setFieldPlaceholder()
					.addQuery(") VALUES (")
					.addQuery(b)
					.addQuery(",")
					.setValuePlaceholder().addQuery(")");
		} else {
			q.addQuery("INSERT INTO wallet(").setFieldPlaceholder()
					.addQuery(") VALUES (").setValuePlaceholder().addQuery(")");
		}

		q.addField("username", credential.username())
				.addField("type", (int) w.type)
				.addField("name", w.name)
				.addField("amount", w.amount * market.amount);

		LAMBDA_SQL_ASSERT_ROLLBACK(q.exec());

		LOG_CDEBUG("client") << "Buy" << qPrintable(credential.username()) << w.type << qPrintable(w.name)
							 << "amount:" << w.amount << "cost:" << cost;
	}


	if (market.type == RpgMarket::Xp) {
		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("INSERT INTO score(").setFieldPlaceholder()
								   .addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
								   .addField("username", credential.username())
								   .addField("xp", market.amount * w.amount)
								   .exec()
								   );
	}

	if (cost > 0) {
		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("INSERT INTO currency(").setFieldPlaceholder()
								   .addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
								   .addField("username", credential.username())
								   .addField("amount", -cost)
								   .exec()
								   );
	}


	db.commit();

	response = responseOk();

	LAMBDA_THREAD_END;
}






/**
 * @brief UserAPI::setCurrency
 * @param username
 * @param gameid
 * @param amount
 */

void UserAPI::setCurrency(const QString &username, const int &gameid, const int &amount) const
{
	databaseMainWorker()->execInThread([this, username, gameid, amount]() {
		_setCurrency(username, gameid, amount);
	});
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

	LAMBDA_SQL_ASSERT(q.fieldCount() && q.exec());

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
 * @brief UserAPI::notification
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::notification(const Credential &credential)
{
	LOG_CTRACE("client") << "Get user notifications";

	LAMBDA_THREAD_BEGIN(credential);

	QueryBuilder q(db);
	q.addQuery("SELECT type FROM notification WHERE username=").addValue(credential.username());

	LAMBDA_SQL_ASSERT(q.exec());

	QJsonArray list;

	while (q.sqlQuery().next()) {
		const CallOfSuli::NotificationType notification = q.value("type").value<CallOfSuli::NotificationType>();
		if (notification != CallOfSuli::NotificationInvalid)
			list.append(notification);
	}

	response = responseResult("list", list);

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::notificationUpdate
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::notificationUpdate(const Credential &credential, const QJsonObject &json)
{
	const QString &username = credential.username();

	LOG_CTRACE("client") << "Modify notification settings:" << qPrintable(username);

	LAMBDA_THREAD_BEGIN(username, json);

	const QJsonArray enable = json.value(QStringLiteral("enable")).toArray();
	const QJsonArray disable = json.value(QStringLiteral("disable")).toArray();

	db.transaction();

	for (const QJsonValue &v : enable) {
		const CallOfSuli::NotificationType notification = v.toVariant().value<CallOfSuli::NotificationType>();
		LAMBDA_SQL_ERROR_ROLLBACK("invalid notification type", notification != CallOfSuli::NotificationInvalid);

		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("INSERT OR IGNORE INTO notification(").setFieldPlaceholder()
								   .addQuery(") VALUES (").setValuePlaceholder()
								   .addQuery(")")
								   .addField("username", username)
								   .addField("type", notification)
								   .exec());
	}

	for (const QJsonValue &v : disable) {
		const CallOfSuli::NotificationType notification = v.toVariant().value<CallOfSuli::NotificationType>();
		LAMBDA_SQL_ERROR_ROLLBACK("invalid notification type", notification != CallOfSuli::NotificationInvalid);

		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("DELETE FROM notification WHERE username=").addValue(username)
								   .addQuery(" AND type=").addValue(notification)
								   .exec());
	}

	db.commit();

	response = responseOk();

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::_addStatistics
 * @param username
 * @param list
 */

void UserAPI::_addStatistics(const QString &username, const QJsonArray &list) const
{
	if (list.isEmpty())
		return;

	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker _locker(databaseMain()->mutex());

	for (const QJsonValue &v : list) {
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
			q.addField("username", username);
		else
			continue;

		q.exec();
	}

}



/**
 * @brief UserAPI::_addWallet
 * @param username
 * @param list
 */

void UserAPI::_addWallet(const QString &username, const int &gameid, const QJsonArray &list) const
{
	if (list.isEmpty())
		return;

	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker _locker(databaseMain()->mutex());

	for (const QJsonValue &v : list) {
		RpgWallet wallet;
		wallet.fromJson(v);

		if (wallet.type == RpgMarket::Invalid) {
			LOG_CDEBUG("client") << "Invalid wallet type" << v << "user:" << username;
			continue;
		}

		if (!QueryBuilder::q(db)
				.addQuery("INSERT OR REPLACE INTO wallet(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
				.addField("username", username)
				.addField("type", (int) wallet.type)
				.addField("name", wallet.name)
				.addField("amount", -wallet.amount)
				.addField("gameid", gameid)
				.exec()) {
			LOG_CERROR("client") << "Game wallet update error" << gameid << qPrintable(username);
		}
	}

}


/**
 * @brief UserAPI::_setCurrency
 * @param username
 * @param gameid
 * @param amount
 */

void UserAPI::_setCurrency(const QString &username, const int &gameid, const int &amount) const
{
	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker _locker(databaseMain()->mutex());

	if (!QueryBuilder::q(db)
			.addQuery("INSERT OR REPLACE INTO currency(").setFieldPlaceholder().addQuery(") VALUES (").setValuePlaceholder().addQuery(")")
			.addField("username", username)
			.addField("amount", amount)
			.addField("gameid", gameid)
			.exec()) {
		LOG_CERROR("client") << "Game currency update error" << gameid << qPrintable(username);
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

			s.setSolved(q.value("level").toInt(), q.value("num").toInt());

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
			solver.setSolved(q.value("level").toInt(), q.value("num").toInt());

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

std::optional<int> UserAPI::_solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission, const int &level)
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
					.execToValue("num");

	if (n)
		return n->toInt();

	return std::nullopt;

}
