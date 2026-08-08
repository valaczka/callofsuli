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
#include "offlineserverengine.h"



#define	CHARACTER_PRESTIGE			QStringLiteral("--prestige--")


/**
 * @brief StudentAPI::StudentAPI
 * @param service
 */

UserAPI::UserAPI(Handler *handler, ServerService *service)
	: AbstractAPI("user", handler, service)
	, m_rnd(std::random_device{}())
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

	server->route(path+"freeplay/permit", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return permitCreate(*credential, 0, jsonObject.value_or(QJsonObject{}));
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


	server->route(path+"campaign/<arg>/permit", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_GET();
		return permitCreate(*credential, id, jsonObject.value_or(QJsonObject{}));
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



	server->route(path+"offline", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return permitUpload(*credential, *jsonObject);
	});



	server->route(path+"rpg", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		return rpg(*credential);
	});

	server->route(path+"rpg/token", QHttpServerRequest::Method::Post,
				  [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return gameTokenCreate(*credential, *jsonObject);
	});

	server->route(path+"rpg/target/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &target, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return rpgTarget(*credential, target);
	});

	server->route(path+"rpg/buy/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const QString &target, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return rpgBuy(*credential, target);
	});

	server->route(path+"rpg/drop/", QHttpServerRequest::Method::Post|QHttpServerRequest::Method::Get,
				  [this](const int &id, const QHttpServerRequest &request){
		AUTHORIZE_API();
		return rpgDrop(*credential, id);
	});

	server->route(path+"rpg/upgrade", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request){
		AUTHORIZE_API();
		JSON_OBJECT_ASSERT();
		return rpgUpgrade(*credential, *jsonObject);
	});
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

	if (g.mode == GameMap::Rpg)
		return gameCreateRpg(json, credential.username(), campaign, g);
	else
		return gameCreate(credential.username(), campaign, g);

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
										const UserGame &game, int *gameIdPtr)
{
	if (gameIdPtr)
		*gameIdPtr = -1;

	return gameCreate(databaseMain(), username, campaign, game, gameIdPtr);
}





/**
 * @brief UserAPI::gameCreateRpg
 * @param username
 * @param campaign
 * @param game
 * @return
 */

QHttpServerResponse UserAPI::gameCreateRpg(const QJsonObject &json, const QString &username, const int &campaign,
										   const UserGame &game, int *gameIdPtr)
{
	QString character = json.value(QStringLiteral("character")).toString();
	QString terrain = json.value(QStringLiteral("terrain")).toString();

	if (character.isEmpty() || terrain.isEmpty())
		return responseError("missing character/terrain");

	return gameCreateRpg(this->databaseMain(), username, campaign, game, character,
						 RpgStream::HashFnv1A64::hashFnv1a64(terrain.toStdString()),
						 gameIdPtr);
}



/**
 * @brief UserAPI::gameTokenCreate
 * @param credential
 * @param campaign
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::gameTokenCreate(const Credential &credential, const QJsonObject &json)
{
	RpgStream::ConnectionToken token;

	token.fromJson(json);
	token.type = EngineRpg;

	if (token.mapUuid.isEmpty() || token.missionUuid.isEmpty())
		return responseError("missing map/mission");

	if (token.missionLevel < 0)
		return responseError("invalid level");

	if (token.campaign < 0)
		token.campaign = 0;

	UdpServer *udpServer = m_service->udpServer();

	if (!udpServer)
		return responseError("internal error");


	QDateTime exp = QDateTime::currentDateTimeUtc().addSecs(120);

	quint32 id = 0;
	////std::shared_ptr<RpgEngine> engine = RpgEngine::peerFind(udpServer, credential.username(), &id);

	if (id > 0) {
		LOG_CERROR("engine") << "SEAT EXISTS" << credential.username() << id;

		token.peer = 0;

		/*if (engine) {
			LOG_CERROR("engine") << "SEAT ENGINE EXISTS" << credential.username() << id << engine->id();
			return responseResult("error", QStringLiteral("active/%1/%2").arg(id).arg(engine->id()));
		}

		token.peer = udpServer->resetPeer(id, credential.username(), exp); */
	} else {
		token.peer = udpServer->addPeer(credential.username(), exp);
	}

	if (token.peer == 0) {
		return responseError("player create error");
	}

	token.user = credential.username();
	token.exp = exp.toSecsSinceEpoch();
	token.ses = QString::fromLatin1(credential.session().toBase64());
	token.pub = QString::fromLatin1(credential.devicePub().toBase64());

	Token jwt;

	jwt.setSecret(m_service->settings()->jwtSecret());
	jwt.setPayload(token.toJson());

	return responseResult("token", QString::fromUtf8(jwt.getToken()));
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

QHttpServerResponse UserAPI::gameFinish(const Credential &credential, const int &id, const QJsonObject &json,
										QJsonObject *dst)
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

		///LAMBDA_THREAD_END;				/// Nem lehet!!!

		ret.resolve();
	});
	QDefer::await(ret);

	const QJsonArray &statistics = json.value(QStringLiteral("statistics")).toArray();
	const int &duration = json.value(QStringLiteral("duration")).toInt();

	if (ret.state() == QDeferredState::RESOLVED) {
		//const QJsonObject &inventory = json.value(QStringLiteral("extended")).toObject();
		const bool &success = json.value(QStringLiteral("success")).toVariant().toBool();
		const int &xp = json.value(QStringLiteral("xp")).toInt();

		return gameFinish(username, id, g, statistics, success, xp, duration, nullptr, dst, GameFinishFull, json);
	} else {
		return gameFinish(username, id, g, statistics, false, 0, duration, nullptr, dst, GameFinishFull, json);
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
										const QJsonArray &statistics,
										const bool &success, const int &xp, const int &duration,
										bool *okPtr, QJsonObject *dst, const GameFinishMode &mode, const QJsonObject &src)
{
	if (okPtr)
		*okPtr = false;

	LOG_CDEBUG("client") << "Finish game" << id << "for user:" << qPrintable(username) << "success:" << success;

	LAMBDA_THREAD_BEGIN(username, statistics, id, xp, duration, success, game, okPtr, mode, dst, src);

	QJsonObject retObj;

	if (mode & GameFinishGameOnly) {
		// Statistics

		if (!statistics.isEmpty())
			_addStatistics(username, statistics);

		int sumXP = xp;

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


		db.commit();




		// Rpg

		if (game.mode == GameMap::Rpg)
			retObj[QStringLiteral("rpg")] = _finishRpgGame(username, id, src);
	}



	if (mode & GameFinishCampaignOnly) {
		if (success) {
			LAMBDA_SQL_ASSERT(TeacherAPI::_evaluateCampaign(this, game.campaign, username));
		}
	}

	response = responseOk(retObj);

	if (okPtr)
		*okPtr = true;


	if (dst)
		*dst = retObj;

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::permitCreate
 * @param credential
 * @param campaign
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::permitCreate(const Credential &credential, const int &campaign, const QJsonObject &json)
{
	LOG_CDEBUG("client") << "Request permit" << credential.username() << "for campaign:" << campaign << "device:" << credential.devicePub().toBase64();

	const qint64 clientClock = json.value(QStringLiteral("clock")).toInteger();

	if (clientClock <= 0)
		return responseError("missing clock");


	OfflineServerEngine engine(m_service);

	const auto &permit = engine.createPermit(credential.username(), campaign, credential.devicePub(), clientClock);

	if (!permit)
		return responseError("permit create error");

	return responseOk(permit->toJson());
}




/**
 * @brief UserAPI::permitUpload
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::permitUpload(const Credential &credential, const QJsonObject &json)
{
	QByteArray content = QByteArray::fromBase64(json.value(QStringLiteral("data")).toString().toLatin1());


	PublicKeySigner signer;
	signer.setPublicKey(credential.devicePub());

	const std::optional<ReceiptList> data = signer.verifyTo<ReceiptList>(content);

	if (!data)
		return responseError("invalid content");

	OfflineServerEngine engine(m_service);

	const std::optional<PermitContent> permit = engine.verifyPermit(data->permit);

	if (!permit)
		return responseError("invalid permit");

	if (permit->deviceid != credential.devicePub())
		return responseError("invalid device");

	LOG_CINFO("client") << "Upload permit receipts" << credential.username() << "for campaign:" << permit->campaign << "device:" << credential.devicePub().toBase64();

	std::vector<Receipt> list;
	list.reserve(data->receipts.size());


	// check receipt hashes

	QByteArray prev;

	for (const QByteArray &d : data->receipts) {
		Receipt receipt;

		receipt.fromCbor(QCborValue::fromCbor(d));

		if (!prev.isEmpty() && receipt.prevHash != prev) {
			LOG_CERROR("client") << "Receipt hash chain error";
			return responseError("receipt list error");
		}

		prev = OfflineEngine::computeMapHash(d);

		list.push_back(std::move(receipt));
	}

	if (list.empty())
		return responseError("empty receipt list");

	const auto &ptr = engine.uploadReceipts(this, *permit, list);

	if (!ptr)
		return responseError("internal error");

	return responseOk(ptr->toJson());

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
 * @brief UserAPI::rpg
 * @param credential
 * @return
 */

QHttpServerResponse UserAPI::rpg(const Credential &credential)
{
	LOG_CTRACE("client") << "Get user RPG info";

	LAMBDA_THREAD_BEGIN(credential);

	// Open old drops


	const auto &dList = QueryBuilder::q(db)
						.addQuery("SELECT id FROM rpgDrop WHERE finished=FALSE AND timestamp<datetime('now', '-1 day') "
								  "AND username=").addValue(credential.username())
						.execToJsonArray();

	LAMBDA_SQL_ASSERT(dList);

	for (const QJsonValue &v : dList.value()) {
		const int id = v.toObject().value(QStringLiteral("id")).toInt();

		LOG_CDEBUG("client") << "Open old drop" << id << "automatically";

		_openRpgDrop(databaseMain(), id, credential.username());
	}


	const auto &chList = QueryBuilder::q(db)
						 .addQuery("SELECT character, level, coin FROM rpgCharacter WHERE username=")
						 .addValue(credential.username())
						 .execToJsonArray();

	LAMBDA_SQL_ASSERT(chList);

	const auto &target = QueryBuilder::q(db)
						 .addQuery("SELECT character, coin FROM rpgTarget WHERE username=")
						 .addValue(credential.username())
						 .execToJsonObject();

	LAMBDA_SQL_ASSERT(target);


	RpgUserData udata;


	// DEPRECATED

	const auto &curr = QueryBuilder::q(db)
					   .addQuery("SELECT SUM(amount) AS amount FROM currency WHERE username=")
					   .addValue(credential.username())
					   .execToValue("amount", 0);

	LAMBDA_SQL_ASSERT(curr);

	udata.oldCurrency = curr->toInt();




	// Load

	QSet<QString> terrains;

	{
		QueryBuilder q(db);
		q.addQuery("SELECT terrain, character FROM rpgGame "
				   "LEFT JOIN game ON (game.id=rpggame.gameid) "
				   "WHERE username=").addValue(credential.username())
				.addQuery(" ORDER BY timestamp DESC")
				.exec();

		LAMBDA_SQL_ASSERT(q.exec());

		while (q.sqlQuery().next()) {
			terrains.insert(q.value("terrain").toString());

			if (udata.lastCharacter.isEmpty())
				udata.lastCharacter = q.value("character").toString();

			if (udata.lastTerrain.isEmpty())
				udata.lastTerrain = q.value("terrain").toString();
		}
	}

	udata.terrains.assign(terrains.cbegin(), terrains.cend());

	QMap<int, QString> avaliableTargets;
	QSet<QString> unlockedCharacters;

	if (m_service->rpgConfig()->characters().empty()) {
		LOG_CERROR("client") << "Missing RPG character data";

	} else {
		for (const auto &[ch, data] : m_service->rpgConfig()->characters().asKeyValueRange()) {
			RpgUserCharacter character(data);

			character.character = ch;

			const auto it = std::find_if(chList->cbegin(),
										 chList->cend(),
										 [&ch](const QJsonValue &v) {
				return v.toObject().value(QStringLiteral("character")).toString() == ch;
			});

			if (it != chList->cend()) {
				character.level = it->toObject().value(QStringLiteral("level")).toInt();
				character.point = it->toObject().value(QStringLiteral("coin")).toInt();

				unlockedCharacters.insert(ch);
			} else {
				// Auto add free characters

				if (data.unlock == 0) {
					character.level = 1;
					character.point = 0;

					unlockedCharacters.insert(ch);

					LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
									  .addQuery("INSERT INTO rpgCharacter(").setFieldPlaceholder()
									  .addQuery(") VALUES (").setValuePlaceholder()
									  .addQuery(")")
									  .addField("username", credential.username())
									  .addField("character", ch)
									  .addField("level", character.level)
									  .addField("coin", character.point)
									  .exec());

				} else {
					avaliableTargets.insert(data.unlock, ch);
				}
			}

			udata.characters.append(character);
		}
	}


	// Check if target is invalid

	udata.target = target->value(QStringLiteral("character")).toString();
	udata.token = target->value(QStringLiteral("coin")).toInt();

	if (!udata.target.isEmpty() &&
			(m_service->rpgConfig()->characters().value(udata.target).unlock == 0 ||
			 unlockedCharacters.contains(udata.target))) {
		LOG_CWARNING("client") << "Invalid target" << udata.target << "for user" << qPrintable(credential.username());

		udata.target.clear();
	}


	// Auto select the lowest target

	if (udata.target.isEmpty()) {
		if (avaliableTargets.isEmpty()) {
			LOG_CWARNING("client") << "No available targets for user" << qPrintable(credential.username());
		} else {
			udata.target = avaliableTargets.first();


			LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
							  .addQuery("INSERT OR REPLACE INTO rpgTarget(").setFieldPlaceholder()
							  .addQuery(") VALUES (").setValuePlaceholder()
							  .addQuery(")")
							  .addField("username", credential.username())
							  .addField("character", udata.target)
							  .addField("coin", udata.token)
							  .exec());

			LOG_CINFO("client") << "Auto target" << udata.target << "for user" << qPrintable(credential.username());
		}
	}



	// Get Drops

	const auto &dropList = QueryBuilder::q(db)
						   .addQuery("SELECT id, type, tier, xp, coinCharacter AS point, coinTarget AS token "
									 "FROM rpgDrop WHERE username=").addValue(credential.username())
						   .addQuery(" AND finished=false")
						   .execToJsonArray();

	LAMBDA_SQL_ASSERT(dropList);

	udata.drops.reserve(dropList->size());

	for (const QJsonValue &v : dropList.value()) {
		RpgUserDrop drop;
		drop.fromJson(v.toObject());
		udata.drops.append(drop);
	}

	response = responseOk(udata.toJson());

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::rpgTarget
 * @param credential
 * @param target
 * @return
 */

QHttpServerResponse UserAPI::rpgTarget(const Credential &credential, const QString &target)
{
	LOG_CTRACE("client") << "New RPG target for user" << qPrintable(credential.username()) << target;

	LAMBDA_THREAD_BEGIN(credential, target);


	db.transaction();

	const auto &chList = QueryBuilder::q(db)
						 .addQuery("SELECT character, level, coin FROM rpgCharacter WHERE username=")
						 .addValue(credential.username())
						 .execToJsonArray();

	LAMBDA_SQL_ASSERT_ROLLBACK(chList);


	QSet<QString> avaliableTargets;

	if (m_service->rpgConfig()->characters().empty()) {
		LOG_CERROR("client") << "Missing RPG character data";

	} else {
		for (const auto &[ch, data] : m_service->rpgConfig()->characters().asKeyValueRange()) {
			const auto it = std::find_if(chList->cbegin(),
										 chList->cend(),
										 [&ch](const QJsonValue &v) {
				return v.toObject().value(QStringLiteral("character")).toString() == ch;
			});

			if (it == chList->cend() && data.unlock > 0)
				avaliableTargets.insert(ch);
		}
	}


	if (avaliableTargets.isEmpty()) {
		LOG_CWARNING("client") << "No available targets for user" << qPrintable(credential.username());
		response = responseError("no available target");
	} else if (!avaliableTargets.contains(target)) {
		LOG_CWARNING("client") << "Targets can't be selected for user" << qPrintable(credential.username()) << target;
		response = responseError("invalid target");
	} else {
		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("UPDATE rpgTarget SET ")
								   .setCombinedPlaceholder()
								   .addField("character", target)
								   .addQuery(" WHERE username =").addValue(credential.username())
								   .exec());

		LOG_CINFO("client") << "New target" << target << "for user" << qPrintable(credential.username());

		response = responseOk(QJsonObject{
								  { QStringLiteral("target"), target }
							  });
	}

	db.commit();

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::rpgBuy
 * @param credential
 * @param target
 * @return
 */

QHttpServerResponse UserAPI::rpgBuy(const Credential &credential, const QString &target)
{
	LOG_CTRACE("client") << "Buy RPG target for user" << qPrintable(credential.username()) << target;

	LAMBDA_THREAD_BEGIN(credential, target);


	db.transaction();

	const auto &chList = QueryBuilder::q(db)
						 .addQuery("SELECT character, level, coin FROM rpgCharacter WHERE username=")
						 .addValue(credential.username())
						 .execToJsonArray();

	LAMBDA_SQL_ASSERT_ROLLBACK(chList);

	const auto &coin = QueryBuilder::q(db)
					   .addQuery("SELECT coin FROM rpgTarget WHERE username=")
					   .addValue(credential.username())
					   .execToValue("coin", 0);

	LAMBDA_SQL_ASSERT_ROLLBACK(coin);


	int value = 0;

	if (m_service->rpgConfig()->characters().empty()) {
		LOG_CERROR("client") << "Missing RPG character data";

	} else {
		for (const auto &[ch, data] : m_service->rpgConfig()->characters().asKeyValueRange()) {
			const auto it = std::find_if(chList->cbegin(),
										 chList->cend(),
										 [&ch](const QJsonValue &v) {
				return v.toObject().value(QStringLiteral("character")).toString() == ch;
			});

			if (it == chList->cend() && data.unlock > 0 && data.unlock <= coin->toInt() && ch == target) {
				value = data.unlock;
				break;
			}
		}
	}


	if (value == 0) {
		LOG_CWARNING("client") << "Targets can't be sell to user" << qPrintable(credential.username()) << target;
		response = responseError("invalid target");
	} else {
		const int token = coin->toInt() - value;

		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("UPDATE rpgTarget SET ")
								   .setCombinedPlaceholder()
								   .addField("coin", token)
								   .addQuery(" WHERE username =").addValue(credential.username())
								   .exec());

		LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
								   .addQuery("INSERT INTO rpgCharacter(").setFieldPlaceholder()
								   .addQuery(") VALUES (").setValuePlaceholder()
								   .addQuery(")")
								   .addField("username", credential.username())
								   .addField("character", target)
								   .addField("level", 1)
								   .addField("coin", 0)
								   .exec());

		LOG_CINFO("client") << "New target" << target << "for user" << qPrintable(credential.username());


		response = responseOk(QJsonObject{
								  { QStringLiteral("target"), target },
								  { QStringLiteral("token"), token },
							  });
	}

	db.commit();

	LAMBDA_THREAD_END;
}



/**
 * @brief UserAPI::rpgDrop
 * @param credential
 * @param id
 * @return
 */

QHttpServerResponse UserAPI::rpgDrop(const Credential &credential, const int &id)
{
	LOG_CTRACE("client") << "Player" << credential.username() << "opens drop" << id;

	if (id <= 0)
		return responseError("invalid id");

	LAMBDA_THREAD_BEGIN(credential, id);

	const auto &r = _openRpgDrop(databaseMain(), id, credential.username());

	LAMBDA_SQL_ASSERT(r);

	LOG_CDEBUG("client") << "User" << qPrintable(credential.username()) << "opened RPG drop" << id;

	response = responseOk(r.value());

	LAMBDA_THREAD_END;
}






/**
 * @brief UserAPI::rpgUpgrade
 * @param credential
 * @param json
 * @return
 */

QHttpServerResponse UserAPI::rpgUpgrade(const Credential &credential, const QJsonObject &json)
{
	QJsonArray list = json.value(QStringLiteral("list")).toArray();

	LOG_CTRACE("client") << "Player" << credential.username() << "upgrade" << list;

	if (list.empty())
		return responseError("missing list");

	LAMBDA_THREAD_BEGIN(credential, list);

	const auto &curr = QueryBuilder::q(db)
					   .addQuery("SELECT SUM(amount) AS amount FROM currency WHERE username=")
					   .addValue(credential.username())
					   .execToValue("amount", 0);

	LAMBDA_SQL_ASSERT(curr);

	int amount = curr->toInt();

	LAMBDA_SQL_ERROR("no currency", amount > 0);

	QStringList ch;

	if (m_service->rpgConfig()->characters().empty()) {
		LOG_CERROR("client") << "Missing RPG character data";

		for (const QJsonValue &v : list)
			ch.append(v.toString());

	} else {
		for (const QJsonValue &v : list) {
			const QString character = v.toString();

			LAMBDA_SQL_ERROR("invalid character", m_service->rpgConfig()->characters().contains(character));

			ch.append(character);
		}
	}

	LAMBDA_SQL_ERROR("sql error", !ch.isEmpty());

	LAMBDA_SQL_ERROR("no currency", amount > ch.size());

	amount /= ch.size();

	LOG_CTRACE("client") << "Player" << qPrintable(credential.username()) << "upgrade to RPG characters" << ch << "with amount" << amount;

	db.transaction();

	for (const QString &character : ch) {
		int t = 0;
		LAMBDA_SQL_ASSERT_ROLLBACK(_addRpgCoin(databaseMain(), credential.username(), character, amount, nullptr, &t));
		LAMBDA_SQL_ASSERT_ROLLBACK(_addRpgToken(databaseMain(), credential.username(), t, nullptr));
	}

	LAMBDA_SQL_ASSERT_ROLLBACK(QueryBuilder::q(db)
							   .addQuery("DELETE FROM currency WHERE username=")
							   .addValue(credential.username())
							   .exec());

	db.commit();

	response = responseOk();

	LAMBDA_THREAD_END;
}






/**
 * @brief UserAPI::gameCreate
 * @param dbMain
 * @param username
 * @param campaign
 * @param game
 * @param gameIdPtr
 * @return
 */

QHttpServerResponse UserAPI::gameCreate(const DatabaseMain *dbMain, const QString &username, const int &campaign,
										const UserGame &game, int *gameIdPtr)
{
	Q_ASSERT (dbMain);

	QDefer ret;
	QHttpServerResponse response(QHttpServerResponse::StatusCode::InternalServerError);

	dbMain->worker()->execInThread([dbMain, campaign, game, username, gameIdPtr, ret, &response]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

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

		QueryBuilder q(db);

		q.addQuery("INSERT INTO game (").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("mapid", game.map)
				.addField("missionid", game.mission)
				.addField("campaignid", campaign > 0 ? campaign : QVariant(QMetaType::fromType<int>()))
				.addField("level", game.level)
				.addField("success", false)
				.addField("mode", game.mode)
				;

		if (game.timestamp > 0) {
			QDateTime dt = QDateTime::fromMSecsSinceEpoch(game.timestamp).toUTC();
			q.addField("timestamp", dt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")));
		}

		const auto &gameId = q.execInsertAsInt();

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


		db.commit();

		response = QHttpServerResponse(obj);

		if (gameIdPtr)
			*gameIdPtr = *gameId;

		ret.resolve();
	});

	QDefer::await(ret);

	return response;

}



/**
 * @brief UserAPI::gameCreateRpg
 * @param dbMain
 * @param username
 * @param campaign
 * @param game
 * @param character
 * @param terrain
 * @param gameIdPtr
 * @return
 */

QHttpServerResponse UserAPI::gameCreateRpg(const DatabaseMain *dbMain, const QString &username, const int &campaign,
										   const UserGame &game, const QString &character, const quint64 &terrainHash,
										   int *gameIdPtr)
{
	Q_ASSERT (dbMain);

	QDefer ret;
	QHttpServerResponse response(QHttpServerResponse::StatusCode::InternalServerError);

	dbMain->worker()->execInThread([dbMain, campaign, game, username, character, terrainHash, gameIdPtr, ret, &response]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());


		QMutexLocker _locker(dbMain->mutex());

		LOG_CDEBUG("client") << "Create RPG game for user:" << qPrintable(username) << "in campaign:" << campaign;

		int level = -1;

		const auto ptr = QueryBuilder::q(db)
						 .addQuery("SELECT level FROM rpgCharacter WHERE username=").addValue(username)
						 .addQuery(" AND character=").addValue(character)
						 .execToValue("level", 0);

		if (ptr)
			level = ptr->toInt();

		if (level <= 0) {
			if (QueryBuilder::q(db)
					.addQuery("SELECT character FROM rpgWeekly WHERE character=")
					.addValue(character)
					.execCheckExists())
				level = 1;
		}

		LAMBDA_SQL_ERROR("invalid character", level > 0);

		int id = -1;

		response = gameCreate(dbMain, username, campaign, game, &id);

		if (id == -1)
			return ret.reject();

		if (gameIdPtr)
			*gameIdPtr = id;

		LAMBDA_SQL_ASSERT(QueryBuilder::q(db)
						  .addQuery("INSERT INTO rpgGame (").setFieldPlaceholder()
						  .addQuery(") VALUES (").setValuePlaceholder()
						  .addQuery(")")
						  .addField("gameid", id)
						  .addField("terrain", QString::number(terrainHash))
						  .addField("character", character)
						  .addField("coinCharacter", 0)
						  .addField("coinTarget", 0)
						  .execInsert());

		ret.resolve();
	});

	QDefer::await(ret);

	return response;
}



/**
 * @brief UserAPI::_openRpgDrop
 * @param database
 * @param id
 * @return
 */

std::optional<QJsonObject> UserAPI::_openRpgDrop(DatabaseMain *database, const int &id, const QString &username)
{
	Q_ASSERT(database);

	QSqlDatabase db = QSqlDatabase::database(database->dbName());

	QMutexLocker _locker(database->mutex());

	std::optional<QJsonObject> r;

	{
		QueryBuilder q(db);
		q.addQuery("SELECT tier, xp, rpgDrop.coinCharacter AS point, rpgDrop.coinTarget AS token, character "
				   "FROM rpgDrop LEFT JOIN rpgGame ON (rpgGame.id=rpgDrop.gameid) "
				   "WHERE finished=false AND rpgDrop.id=").addValue(id);

		if (!username.isEmpty())
			q.addQuery(" AND username=").addValue(username);

		r = q.execToJsonObject();

		if (!r || r->isEmpty())
			return std::nullopt;
	}

	int point = r->value(QStringLiteral("point")).toInt();
	int token = r->value(QStringLiteral("token")).toInt();
	int xp = r->value(QStringLiteral("xp")).toInt();
	int t = 0;

	if (point > 0)
		_addRpgCoin(database, username, r->value(QStringLiteral("character")).toString(), point, &r.value(), &t);

	token += t;

	if (token > 0)
		_addRpgToken(database, username, token, &r.value());

	if (xp > 0) {
		if (!QueryBuilder::q(db)
				.addQuery("INSERT INTO score (").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("xp", xp)
				.exec())
			return std::nullopt;
	}

	QueryBuilder::q(db)
			.addQuery("UPDATE rpgDrop SET finished=TRUE WHERE id=").addValue(id)
			.exec();


	return r;
}








/**
 * @brief UserAPI::solverInfo
 * @param dbMain
 * @param username
 * @param map
 * @return
 */

std::optional<QMap<QString, GameMap::SolverInfo> > UserAPI::solverInfo(const DatabaseMain *dbMain, const QString &username, const QString &map)
{
	Q_ASSERT(dbMain);

	QDefer ret;

	QMap<QString, GameMap::SolverInfo> solver;

	dbMain->worker()->execInThread([dbMain, username, map, ret, &solver]() mutable {
		QSqlDatabase db = QSqlDatabase::database(dbMain->dbName());

		QMutexLocker _locker(dbMain->mutex());

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
 * @brief UserAPI::_finishRpgGame
 * @param username
 * @param id
 * @param json
 */

QJsonObject UserAPI::_finishRpgGame(const QString &username, const int &id, const QJsonObject &json)
{
	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker _locker(databaseMain()->mutex());

	QJsonObject ret;

	QString terrain;
	QString character;
	int rpgId = -1;

	{
		QueryBuilder qq(db);

		qq.addQuery("SELECT id, terrain, character FROM rpgGame WHERE gameid=").addValue(id);

		if (!qq.exec() || !qq.sqlQuery().first())
			return {};

		terrain = qq.value("terrain").toString();
		character = qq.value("character").toString();
		rpgId = qq.value("id").toInt();
	}

	int point = json.value(QStringLiteral("point")).toInt();
	int token = json.value(QStringLiteral("token")).toInt();


	ret[QStringLiteral("point")] = point;
	ret[QStringLiteral("token")] = token;


	QueryBuilder::q(db)
			.addQuery("UPDATE rpgGame SET ")
			.setCombinedPlaceholder()
			.addField("coinCharacter", point)
			.addField("coinTarget", token)
			.addQuery(" WHERE gameid=")
			.addValue(id)
			.exec();


	int t = 0;

	if (point > 0)
		_addRpgCoin(databaseMain(), username, character, point, &ret, &t);

	token += t;

	if (token > 0)
		_addRpgToken(databaseMain(), username, token, &ret);

	_createRpgDrops(username, terrain, rpgId, &ret);

	return ret;
}



/**
 * @brief UserAPI::_addRpgCoin
 * @param username
 * @param character
 * @param point
 * @param dst
 * @param tokenPtr
 * @return
 */

bool UserAPI::_addRpgCoin(DatabaseMain *database, const QString &username, const QString &character,
						  const int &point, QJsonObject *dst, int *tokenPtr)
{
	Q_ASSERT(database);

	if (point <= 0)
		return true;

	QSqlDatabase db = QSqlDatabase::database(database->dbName());

	QMutexLocker _locker(database->mutex());

	const auto &ch = QueryBuilder::q(db)
					 .addQuery("SELECT level, coin FROM rpgCharacter WHERE username=").addValue(username)
					 .addQuery(" AND character=").addValue(character)
					 .execToJsonObject();

	if (!ch)
		return false;

	int token = 0;

	if (ch->isEmpty()) {
		/// TODO: userWeekly
		LOG_CERROR("client") << "Missing implementation";

		const int plus = (float) point / (float) CFG_POWER_POINT_TOKEN;

		LOG_CINFO("client") << "Convert" << point << "to" << plus << "tokens";

		token += plus;

	} else {


		int level = ch->value(QStringLiteral("level")).toInt();
		int rpoint = point + ch->value(QStringLiteral("coin")).toInt();


		///int token = target.value_or({}).value(QStringLiteral("coin")).toInt();

		if (level < 1) {
			LOG_CERROR("client") << "Invalid RPG character level:" << qPrintable(username) << character << level;
			return false;
		}


		if (database->service()->rpgConfig()->characters().empty()) {
			LOG_CERROR("client") << "Missing RPG character data";
		} else {
			const RpgServerCharacter data = database->service()->rpgConfig()->characters().value(character);

			if (data.pwrUnlock.size() != CFG_POWER_LEVEL_COUNT) {
				LOG_CERROR("client") << "Invalid RPG character:" << qPrintable(username) << character;
				return false;
			}

			// Push up levels

			while (rpoint > 0 && level < CFG_POWER_LEVEL_COUNT) {
				const int next = data.pwrUnlock.at(level);

				if (rpoint < next)
					break;

				++level;
				rpoint -= next;
			}

			// Convert to token

			if (level >= CFG_POWER_LEVEL_COUNT) {
				const int plus = (float) rpoint / (float) CFG_POWER_POINT_TOKEN;

				LOG_CINFO("client") << "Convert" << rpoint << "to" << plus << "tokens";

				token += plus;
				rpoint = 0;
			}
		}

		if (dst) {
			dst->insert(QStringLiteral("character"), character);
			dst->insert(QStringLiteral("newLevel"), level);
			dst->insert(QStringLiteral("newPoint"), rpoint);
		}

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE rpgCharacter SET ").setCombinedPlaceholder()
				.addField("level", level)
				.addField("coin", rpoint)
				.addQuery(" WHERE username=")
				.addValue(username)
				.addQuery(" AND character=")
				.addValue(character)
				.exec())
			return false;
	}


	if (tokenPtr)
		*tokenPtr = token;

	return true;
}



/**
 * @brief UserAPI::_addRpgToken
 * @param username
 * @param token
 * @param dst
 * @return
 */

bool UserAPI::_addRpgToken(DatabaseMain *database, const QString &username, const int &token, QJsonObject *dst)
{
	Q_ASSERT(database);

	if (token <= 0)
		return true;

	QSqlDatabase db = QSqlDatabase::database(database->dbName());

	QMutexLocker _locker(database->mutex());

	const auto &target = QueryBuilder::q(db)
						 .addQuery("SELECT character, coin FROM rpgTarget WHERE username=")
						 .addValue(username)
						 .execToJsonObject();

	if (!target)
		return false;

	QString character = target->value(QStringLiteral("character")).toString();

	int real = token + target->value(QStringLiteral("coin")).toInt();


	// Unlock character

	if (!character.isEmpty() && character != CHARACTER_PRESTIGE) {
		const RpgServerCharacter data = database->service()->rpgConfig()->characters().value(character);

		if (data.pwrUnlock.size() != CFG_POWER_LEVEL_COUNT) {
			LOG_CERROR("client") << "Invalid RPG character:" << qPrintable(username) << character;
		} else if (real >= data.unlock) {
			LOG_CINFO("client") << "Unlock character" << character << "for user" << qPrintable(username);

			real -= data.unlock;

			if (!QueryBuilder::q(db)
					.addQuery("INSERT INTO rpgCharacter(").setFieldPlaceholder()
					.addQuery(") VALUES (").setValuePlaceholder()
					.addQuery(")")
					.addField("username", username)
					.addField("character", character)
					.addField("level", 1)
					.addField("coin", 0)
					.exec())
				return false;

			if (dst) {
				dst->insert(QStringLiteral("unlocked"), character);
			}
		}
	}


	// Store token value

	if (!character.isEmpty() && real > 0) {
		if (!QueryBuilder::q(db)
				.addQuery("UPDATE rpgTarget SET ").setCombinedPlaceholder()
				.addField("coin", real)
				.addQuery(" WHERE username=")
				.addValue(username)
				.exec())
			return {};
	}



	if (database->service()->rpgConfig()->characters().empty()) {
		LOG_CERROR("client") << "Missing RPG character data";
		return true;
	}


	if (real <= 0)
		character.clear();

	// Auto select target

	if (character.isEmpty()) {
		QSet<QString> used;
		QMap<int, QString> availableTargets;

		QueryBuilder q(db);
		q.addQuery("SELECT character FROM rpgCharacter WHERE username=").addValue(username);

		if (!q.exec())
			return false;

		while (q.sqlQuery().next())
			used.insert(q.value("character").toString());

		for (const auto &[ch, data] : database->service()->rpgConfig()->characters().asKeyValueRange()) {
			if (data.unlock == 0 || used.contains(ch))
				continue;

			availableTargets.insert(data.unlock, ch);
		}

		if (availableTargets.isEmpty()) {
			LOG_CWARNING("client") << "No available targets for user" << qPrintable(username);
			character = CHARACTER_PRESTIGE;
		} else {
			character = availableTargets.first();
		}

		LOG_CDEBUG("client") << "Auto select target" << character << "for user" << qPrintable(username);

		if (!QueryBuilder::q(db)
				.addQuery("INSERT OR REPLACE INTO rpgTarget(").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("character", character)
				.addField("coin", real)
				.exec())
			return false;


		if (dst) {
			dst->insert(QStringLiteral("target"), character);
			dst->insert(QStringLiteral("targetToken"), real);
		}
	}

	return true;
}



/**
 * @brief UserAPI::_createRpgDrops
 * @param username
 * @param terrain
 * @param dst
 * @return
 */

bool UserAPI::_createRpgDrops(const QString &username, const QString &terrain, const int &gameid, QJsonObject *dst)
{
	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker _locker(databaseMain()->mutex());

	const auto &num = QueryBuilder::q(db)
					  .addQuery("SELECT COUNT(*) AS num FROM rpgGame "
								"LEFT JOIN game ON (rpgGame.gameid=game.id) "
								"WHERE game.success=true AND date(game.timestamp)=date('now') AND username=").addValue(username)
					  .execToValue("num", 0);

	if (!num)
		return false;

	const auto &dcount = QueryBuilder::q(db)
						 .addQuery("SELECT COUNT(*) AS num FROM rpgDrop "
								   "WHERE type=").addValue(CfgDrop::DropGame)
						 .addQuery(" AND date(timestamp)=date('now') AND username=").addValue(username)
						 .execToValue("num", 0);

	if (!dcount)
		return false;

	QJsonArray dropList;

	for (int i=0; i<(int) cfgDropDay.size(); ++i) {
		const int n = cfgDropDay.at(i);

		if (n > num->toInt())
			break;

		if (i < dcount->toInt())
			continue;


		LOG_CINFO("client") << "Create RPG daily drop for user" << qPrintable(username);

		const CfgDrop drop = CfgDropGenerator::generate(m_rnd);

		if (auto v = QueryBuilder::q(db)
				.addQuery("INSERT INTO rpgDrop(").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("type", CfgDrop::DropGame)
				.addField("tier", drop.tier)
				.addField("username", username)
				.addField("gameid", gameid > 0 ? gameid : QVariant(QMetaType::fromType<int>()))
				.addField("xp", drop.xp)
				.addField("coinCharacter", drop.point)
				.addField("coinTarget", drop.token)
				.execInsertAsInt(); v) {
			dropList << v.value();
		} else {
			return false;
		}
	}


	// Terrain drop

	const auto &tnum = QueryBuilder::q(db)
					   .addQuery("SELECT COUNT(*) AS num FROM rpgGame "
								 "LEFT JOIN game ON (rpgGame.gameid=game.id) "
								 "WHERE game.success=true AND username=").addValue(username)
					   .addQuery(" AND terrain=").addValue(terrain)
					   .execToValue("num", 0);

	if (!tnum)
		return false;

	const auto &tcount = QueryBuilder::q(db)
						 .addQuery("SELECT COUNT(*) AS num FROM rpgDrop "
								   "WHERE type=").addValue(CfgDrop::DropTerrain)
						 .addQuery(" AND username=").addValue(username)
						 .addQuery(" AND terrain=").addValue(terrain)
						 .execToValue("num", 0);


	for (int i=0; i<(int) cfgDropTerrain.size(); ++i) {
		const int n = cfgDropTerrain.at(i);

		if (n > tnum->toInt())
			break;

		if (i < tcount->toInt())
			continue;


		LOG_CINFO("client") << "Create RPG terrain drop for user" << qPrintable(username);

		const CfgDrop drop = CfgDropGenerator::generate(m_rnd, cfgDropDistributionMedium);

		if (auto v = QueryBuilder::q(db)
				.addQuery("INSERT INTO rpgDrop(").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("type", CfgDrop::DropTerrain)
				.addField("tier", drop.tier)
				.addField("username", username)
				.addField("gameid", gameid > 0 ? gameid : QVariant(QMetaType::fromType<int>()))
				.addField("terrain", terrain)
				.addField("xp", drop.xp)
				.addField("coinCharacter", drop.point)
				.addField("coinTarget", drop.token)
				.execInsertAsInt(); v) {
			dropList << v.value();
		} else {
			return false;
		}
	}

	if (dst)
		dst->insert(QStringLiteral("dropList"), dropList);

	return true;
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
	return solverInfo(api->databaseMain(), username, map);
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
