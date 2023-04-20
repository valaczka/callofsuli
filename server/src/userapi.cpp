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
#include "qjsonarray.h"
#include "serverservice.h"



/**
 * @brief StudentAPI::StudentAPI
 * @param service
 */

UserAPI::UserAPI(ServerService *service)
	: AbstractAPI(service)
{
	m_validateRole = Credential::Student;

	addMap("^campaign/*$", this, &UserAPI::campaigns);
	addMap("^campaign/(\\d+)/*$", this, &UserAPI::campaignOne);
	addMap("^campaign/(\\d+)/game/create/*$", this, &UserAPI::gameCreate);

	addMap("^game/info/*", this, &UserAPI::gameInfo);
	addMap("^game/(\\d+)/update/*$", this, &UserAPI::gameUpdate);
	addMap("^game/(\\d+)/finish/*$", this, &UserAPI::gameFinish);

	addMap("^group/*$", this, &UserAPI::groups);

	addMap("^map/*$", this, &UserAPI::maps);
	addMap("^map/([^/]+)/*$", this, &UserAPI::mapOne);
	addMap("^map/([^/]+)/solver/*$", this, &UserAPI::mapSolver);

}




/**
 * @brief UserAPI::solverInfo
 * @param api
 * @param username
 * @param map
 * @return
 */

QMap<QString, GameMap::SolverInfo> UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map)
{
	Q_ASSERT(api);

	QDefer ret;
	QMap<QString, GameMap::SolverInfo> solver;

	api->databaseMainWorker()->execInThread([api, username, map, ret, &solver]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

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

	return solver;
}






/**
 * @brief UserAPI::solverInfo
 * @param api
 * @param username
 * @param map
 * @param mission
 * @return
 */

GameMap::SolverInfo UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission)
{
	Q_ASSERT(api);

	QDefer ret;
	GameMap::SolverInfo solver;

	api->databaseMainWorker()->execInThread([api, username, map, mission, ret, &solver]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

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

	return solver;
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

int UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission, const int &level)
{
	Q_ASSERT(api);

	QDefer ret;
	int num = -1;

	api->databaseMainWorker()->execInThread([api, username, map, mission, ret, level, &num]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		bool err = false;
		const int &n = QueryBuilder::q(db)
				.addQuery("SELECT COUNT(*) AS num FROM game WHERE username=").addValue(username)
				.addQuery(" AND success=true")
				.addQuery(" AND mapid=").addValue(map)
				.addQuery(" AND missionid=").addValue(mission)
				.addQuery(" AND level=").addValue(level)
				.execToValue("num", &err).toInt();

		if (err)
			return ret.reject();

		num = n;

		ret.resolve();
	});

	QDefer::await(ret);

	return num;
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

int UserAPI::solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission, const int &level,
						const bool &deathmatch)
{
	Q_ASSERT(api);

	QDefer ret;
	int num = -1;

	api->databaseMainWorker()->execInThread([api, username, map, mission, ret, level, deathmatch, &num]() mutable {
		QSqlDatabase db = QSqlDatabase::database(api->databaseMain()->dbName());

		QMutexLocker(api->databaseMain()->mutex());

		bool err = false;
		const int &n = QueryBuilder::q(db)
				.addQuery("SELECT COUNT(*) AS num FROM game WHERE username=").addValue(username)
				.addQuery(" AND success=true")
				.addQuery(" AND mapid=").addValue(map)
				.addQuery(" AND missionid=").addValue(mission)
				.addQuery(" AND level=").addValue(level)
				.addQuery(" AND deathmatch=").addValue(deathmatch)
				.execToValue("num", &err).toInt();

		if (err)
			return ret.reject();

		num = n;

		ret.resolve();
	});

	QDefer::await(ret);

	return num;
}





/**
 * @brief StudentAPI::groups
 * @param response
 */

void UserAPI::groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT id, name, owner, familyName AS ownerFamilyName, givenName AS ownerGivenName FROM studentGroupInfo "
						  "LEFT JOIN user ON (user.username=studentGroupInfo.owner) "
						  "WHERE studentGroupInfo.active=true AND studentGroupInfo.username=").addValue(username)
				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}




/**
 * @brief StudentAPI::campaigns
 * @param response
 */

void UserAPI::campaigns(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
						  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
						  "description, finished, groupid "
						  "FROM campaign WHERE started=true AND groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")")
				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}



/**
 * @brief UserAPI::campaignOne
 * @param match
 * @param response
 */

void UserAPI::campaignOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
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
						  "description, finished, groupid "
						  "FROM campaign WHERE started=true "
						  "AND id=").addValue(id)
				.addQuery(" AND groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")")
				.execToJsonObject(&err);

		if (err)
			return responseErrorSql(response);

		if (obj.isEmpty())
			return responseError(response, "not found");


		/// TODO: solved?

		obj[QStringLiteral("taskList")] = QueryBuilder::q(db)
				.addQuery("SELECT id, gradeid, xp, required, mapuuid, criterion, false AS solved FROM task WHERE campaignid=").addValue(id)
				.execToJsonArray({
									 { QStringLiteral("criterion"), [](const QVariant &v) {
										   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
									   } }
								 });


		responseAnswer(response, obj);
	});

}




/**
 * @brief StudentAPI::maps
 * @param response
 */

void UserAPI::maps(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT mapdb.map.uuid, name, md5, "
						  "mapdb.cache.data AS cache, length(mapdb.map.data) as size "
						  "FROM mapdb.map LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
						  "WHERE mapdb.map.uuid IN "
						  "(SELECT mapuuid FROM task WHERE campaignid IN "
						  "(SELECT id FROM campaign WHERE groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")))")
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
 * @brief UserAPI::mapOne
 * @param match
 * @param response
 */

void UserAPI::mapOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);

	databaseMainWorker()->execInThread([this, response, uuid]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);

		q.addQuery("SELECT data FROM mapdb.map WHERE uuid=").addValue(uuid);

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
 * @brief UserAPI::mapSolver
 * @param match
 * @param response
 */

void UserAPI::mapSolver(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);
	const QString &username = m_credential.username();

	const QMap<QString, GameMap::SolverInfo> &solver = solverInfo(this, username, uuid);

	QJsonObject ret;

	for (auto it = solver.constBegin(); it != solver.constEnd(); ++it)
		ret.insert(it.key(), it.value().toJsonObject());

	responseAnswer(response, ret);
}



/**
 * @brief UserAPI::gameInfo
 * @param data
 * @param response
 */

void UserAPI::gameInfo(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response, data]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT game.username, COUNT(*) AS num, MAX(duration) AS dMax, MIN(duration) as dMin, "
						  "ROW_NUMBER() OVER (ORDER BY MIN(duration)) durationPos, "
						  "ROW_NUMBER() OVER (ORDER BY COUNT(*) DESC, MIN(duration)) numPos, "
						  "familyName, givenName, nickname, picture, rankid FROM game "
						  "LEFT JOIN user ON (user.username=game.username) "
						  "LEFT JOIN userRank ON (userRank.username=game.username) "
						  "WHERE success=true AND user.active=true ")
				.addQuery(" AND mapid=").addValue(data.value(QStringLiteral("map")).toString())
				.addQuery(" AND missionid=").addValue(data.value(QStringLiteral("mission")).toString())
				.addQuery(" AND game.level=").addValue(data.value(QStringLiteral("level")).toInt())
				.addQuery(" AND mode=").addValue(data.value(QStringLiteral("mode")).toInt())
				.addQuery(" AND deathmatch=").addValue(data.value(QStringLiteral("deathmatch")).toVariant().toBool())
				.addQuery(" GROUP BY game.username, mapid, missionid, game.level, mode, deathmatch")
				.execToJsonArray(&err);

		if (err)
			responseErrorSql(response);
		else
			responseAnswer(response, "list", list);
	});
}





/**
 * @brief UserAPI::gameCreate
 * @param match
 * @param data
 * @param response
 */

void UserAPI::gameCreate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &campaign = match.captured(1).toInt();


	Game g;

	g.map = data.value(QStringLiteral("map")).toString();
	g.mission = data.value(QStringLiteral("mission")).toString();
	g.level = data.value(QStringLiteral("level")).toInt(-1);
	g.deathmatch = data.value(QStringLiteral("deathmatch")).toVariant().toBool();
	g.mode = data.value(QStringLiteral("mode")).toVariant().value<GameMap::GameMode>();

	if (g.map.isEmpty() || g.mission.isEmpty())
		return responseError(response, "missing map/mission");

	if (g.level < 0)
		return responseError(response, "invalid level");

	if (g.mode == GameMap::Invalid)
		return responseError(response, "invalid mode");


	databaseMainWorker()->execInThread([this, response, campaign, g]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		LOG_CDEBUG("client") << "Create game for user:" << qPrintable(username) << "in campaign:" << campaign;

		if (!QueryBuilder::q(db)
				.addQuery("SELECT id FROM campaign WHERE started=true AND finished=false AND groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")")
				.execCheckExists()) {
			return responseError(response, "invalid campaign");
		}



		db.transaction();

		bool err = false;

		// Close running games

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT gameid, xp FROM runningGame LEFT JOIN game ON (game.id=runningGame.gameid) WHERE username=")
				.addValue(username)
				.execToJsonArray(&err);

		if (err) {
			db.rollback();
			return responseErrorSql(response);
		}

		foreach (const QJsonValue &v, list) {
			const QJsonObject &o = v.toObject();
			const int &gid = o.value(QStringLiteral("gameid")).toInt();
			const int &xp = o.value(QStringLiteral("xp")).toInt();

			LOG_CDEBUG("client") << "Close running game " << gid << "for user:" << qPrintable(username);

			int scoreId = -1;

			if (xp > 0) {
				scoreId = QueryBuilder::q(db)
						.addQuery("INSERT INTO score (").setFieldPlaceholder()
						.addQuery(") VALUES (").setValuePlaceholder()
						.addQuery(")")
						.addField("username", username)
						.addField("xp", xp)
						.execInsertAsInt(&err);

				if (err) {
					db.rollback();
					return responseErrorSql(response);
				}
			}

			//.addQuery("UPDATE game SET duration=((julianday(CURRENT_TIMESTAMP)-julianday(timestamp))*86400.0), success=false, "

			if (!QueryBuilder::q(db)
					.addQuery("UPDATE game SET duration=NULL, success=false, "
							  "scoreid=")
					.addValue(scoreId > 0 ? scoreId : QVariant(QVariant::Invalid))
					.addQuery(" WHERE id=")
					.addValue(gid)
					.exec()) {
				db.rollback();
				return responseErrorSql(response);
			}
		}


		if (!QueryBuilder::q(db)
				.addQuery("DELETE FROM runningGame WHERE gameid IN "
						  "(SELECT gameid FROM runningGame LEFT JOIN game ON (game.id=runningGame.gameid) WHERE username=")
				.addValue(username)
				.addQuery(")")
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		// Create game

		const int &gameId = QueryBuilder::q(db)
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
				.execInsertAsInt(&err);

		if (err) {
			db.rollback();
			return responseErrorSql(response);
		}

		QueryBuilder::q(db)
				.addQuery("INSERT INTO runningGame (").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("gameid", gameId)
				.addField("xp", 0)
				.execInsert(&err);

		if (err) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		QJsonObject ret;
		ret.insert(QStringLiteral("id"), gameId);
		ret.insert(QStringLiteral("closedGames"), list);

		responseAnswerOk(response, ret);
	});

}



/**
 * @brief UserAPI::gameUpdate
 * @param match
 * @param data
 * @param response
 */

void UserAPI::gameUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{

}



/**
 * @brief UserAPI::gameFinish
 * @param match
 * @param data
 * @param response
 */

void UserAPI::gameFinish(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const
{
	const int &gameid = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, gameid, data]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		LOG_CDEBUG("client") << "Finish game" << gameid << "for user:" << qPrintable(username);

		QueryBuilder qq(db);

		qq.addQuery("SELECT mapid, missionid, level, deathmatch, mode FROM game "
					"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
					"WHERE runningGame.gameid=game.id AND game.id=").addValue(gameid)
				.addQuery(" AND username=").addValue(username);

		if (!qq.exec())
			return responseErrorSql(response);

		if (!qq.sqlQuery().first())
			return responseError(response, "invalid game");

		Game g;

		g.map = qq.value("mapid").toString();
		g.mission = qq.value("missionid").toString();
		g.level = qq.value("level").toInt();
		g.deathmatch = qq.value("deathmatch").toBool();
		g.mode = qq.value("mode").value<GameMap::GameMode>();



		const bool &success = data.value(QStringLiteral("success")).toVariant().toBool();
		const bool &flawless = data.value(QStringLiteral("flawless")).toVariant().toBool();
		const int &xp = data.value(QStringLiteral("xp")).toInt();
		const int &duration = data.value(QStringLiteral("duration")).toInt();
		int sumXP = xp;


		// duration.....

		// streak....


		QJsonObject ret;

		const int &baseXP = m_service->config().get("gameBaseXP").toInt(100);
		const int &oldSolved = solverInfo(this, username, g.map, g.mission, g.level, g.deathmatch);

		if (success) {
			const int &xpSolved = GameMap::computeSolvedXpFactor(g.level, g.deathmatch, oldSolved, g.mode) * baseXP;
			/*int shortestDuration = oldDurations.value("minDuration", -1).toInt();

			if (shortestDuration > -1 && duration < shortestDuration && !lite) {
				durationXP = (shortestDuration-duration) * baseXP * XP_FACTOR_DURATION_SEC;
			}*/

			sumXP += xpSolved;
			ret[QStringLiteral("xpSolved")] = xpSolved;
		}

		ret[QStringLiteral("xpGame")] = xp;


		ret[QStringLiteral("success")] = success;
		ret[QStringLiteral("id")] = gameid;


		db.transaction();

		bool err = false;

		const int &scoreId = QueryBuilder::q(db)
				.addQuery("INSERT INTO score (").setFieldPlaceholder()
				.addQuery(") VALUES (").setValuePlaceholder()
				.addQuery(")")
				.addField("username", username)
				.addField("xp", sumXP)
				.execInsertAsInt(&err);

		if (err) {
			db.rollback();
			return responseErrorSql(response);
		}

		if (!QueryBuilder::q(db)
				.addQuery("UPDATE game SET ").setCombinedPlaceholder()
				.addField("duration", duration)
				.addField("success", success)
				.addField("scoreid", scoreId)
				.addQuery(" WHERE id=")
				.addValue(gameid)
				.exec()) {
			db.rollback();
			return responseErrorSql(response);
		}


		if (!QueryBuilder::q(db).addQuery("DELETE FROM runningGame WHERE gameid=").addValue(gameid).exec()) {
			db.rollback();
			return responseErrorSql(response);
		}

		db.commit();

		responseAnswerOk(response, ret);
	});

}



/*
QVariantMap params = m_message.jsonData().toVariantMap();
int gameid = params.value("id", -1).toInt();
int xp = params.value("xp", -1).toInt();
int duration = params.value("duration", -1).toInt();
bool success = params.value("success", false).toBool();
bool flawless = params.value("flawless", false).toBool();

if (gameid < 0 || xp < 0) {
	(*jsonResponse)["error"] = "missing id or xp";
	return false;
}


QVariantMap r = m_client->db()->execSelectQueryOneRow("SELECT id, mapid, missionid, level, deathmatch, lite FROM game "
													  "WHERE username=? AND id=? AND tmpScore IS NOT NULL", {
														  m_client->clientUserName(),
														  gameid
													  });

if (r.isEmpty()) {
	(*jsonResponse)["error"] = "invalid game";
	return false;
}


QString mapid = r.value("mapid").toString();
QString missionid = r.value("missionid").toString();
int level = r.value("level").toInt();
bool deathmatch = r.value("deathmatch").toBool();
bool lite = r.value("lite").toBool();

QJsonObject xpObject;

// Eredeti állapot

GameMap::SolverInfo oldSolver = missionSolverInfo(mapid, missionid);

QVariantMap oldDurations = m_client->db()->execSelectQueryOneRow("SELECT "
																 "(SELECT MAX(duration) FROM game WHERE username=? and missionid=? "
																 "AND level=? AND success=true AND lite=false) as maxDuration,"
																 "(SELECT MIN(duration) FROM game WHERE username=? and missionid=? "
																 "AND level=? AND success=true AND lite=false) as minDuration"
																 , {
																	 m_client->clientUserName(),
																	 missionid,
																	 level,
																	 m_client->clientUserName(),
																	 missionid,
																	 level
																 });


// lastStreak

int lastStreak = m_client->db()->execSelectQueryOneRow("SELECT streak FROM "
													   "(SELECT MAX(dt) as dt, COUNT(*) as streak FROM "
													   "(SELECT t1.dt as dt, date(t1.dt,-(select count(*) FROM "
													   "(SELECT DISTINCT date(timestamp) AS dt "
													   "FROM game WHERE username=? AND success=true) t2 "
													   "WHERE t2.dt<=t1.dt)||' day', 'localtime') as grp FROM "
													   "(SELECT DISTINCT date(timestamp, 'localtime') AS dt FROM game "
													   "WHERE username=? AND success=true) t1) t GROUP BY grp) "
													   "WHERE dt=date('now', 'localtime')", {
														   m_client->clientUserName(),
														   m_client->clientUserName()
													   })
				 .value("streak", 0).toInt();


// maxStreak

int maxStreak = m_client->db()->execSelectQueryOneRow("SELECT MAX(maxStreak) as maxStreak FROM score WHERE username=?", {
														  m_client->clientUserName()
													  })
				.value("maxStreak", 0).toInt();

// XP alap

QVariantMap mxp = m_client->db()->execSelectQueryOneRow("SELECT value FROM settings WHERE key='xp.base'");

int baseXP = mxp.value("value", 100).toInt();


// Rögzítés

QVariantList ll;
ll.append(success);
if (duration > 0)
	ll.append(duration);
else
	ll.append(QVariant::Invalid);
ll.append(gameid);

m_client->db()->execSimpleQuery("UPDATE game SET tmpScore=NULL, success=?, duration=? WHERE id=?", ll);


// Új állapot

int solvedXP = 0;
int durationXP = 0;

if (success) {
	solvedXP = GameMap::computeSolvedXpFactor(oldSolver, level, deathmatch, lite) * baseXP;
	int shortestDuration = oldDurations.value("minDuration", -1).toInt();

	if (shortestDuration > -1 && duration < shortestDuration && !lite) {
		durationXP = (shortestDuration-duration) * baseXP * XP_FACTOR_DURATION_SEC;
	}
}

xpObject["game"] = xp;
xpObject["solved"] = solvedXP;
xpObject["duration"] = durationXP;

QVariantMap m;
m["username"] = m_client->clientUserName();
m["xp"] = xp+solvedXP+durationXP;
m["gameid"] = gameid;
int ret = m_client->db()->execInsertQuery("INSERT INTO score (?k?) VALUES (?)", m);

if (ret == -1) {
	(*jsonResponse)["error"] = "sql error";
	return false;
}






// currentStreak

int  currentStreak = m_client->db()->execSelectQueryOneRow("SELECT streak FROM "
														   "(SELECT MAX(dt) as dt, COUNT(*) as streak FROM "
														   "(SELECT t1.dt as dt, date(t1.dt,-(select count(*) FROM "
														   "(SELECT DISTINCT date(timestamp) AS dt "
														   "FROM game WHERE username=? AND success=true) t2 "
														   "WHERE t2.dt<=t1.dt)||' day', 'localtime') as grp FROM "
														   "(SELECT DISTINCT date(timestamp, 'localtime') AS dt FROM game "
														   "WHERE username=? AND success=true) t1) t GROUP BY grp) "
														   "WHERE dt=date('now', 'localtime')", {
															   m_client->clientUserName(),
															   m_client->clientUserName()
														   })
					 .value("streak", 0).toInt();



// Streak lépés

int streakXP = 0;
int maxStreakXP = 0;


if (currentStreak > 1 && currentStreak > lastStreak) {
	streakXP = baseXP * XP_FACTOR_STREAK * currentStreak;

	if (currentStreak > maxStreak) {
		qInfo().noquote() << tr("New longest streak '%1': %2").arg(m_client->clientUserName()).arg(currentStreak);

		maxStreakXP = baseXP * XP_FACTOR_NEW_STREAK * currentStreak;
	} else {
		qInfo().noquote() << tr("New streak '%1': %2").arg(m_client->clientUserName()).arg(currentStreak);
	}


	QVariantMap p;
	p["maxStreak"] = currentStreak;
	p["xp"] = streakXP+maxStreakXP;
	p["username"] = m_client->clientUserName();

	if (m_client->db()->execInsertQuery("INSERT INTO score (?k?) VALUES (?)", p) == -1) {
		(*jsonResponse)["error"] = "sql error";
		return false;
	}

	xpObject["streak"] = streakXP;
	xpObject["maxStreak"] = maxStreakXP;
}

QJsonObject streakObject;
streakObject["current"] = currentStreak;
streakObject["last"] = lastStreak;
streakObject["max"] = maxStreak;





(*jsonResponse)["streak"] = streakObject;
(*jsonResponse)["xp"] = xpObject;

if (success) {
	(*jsonResponse)["maxDuration"] = lite ? -1 : oldDurations.value("maxDuration", -1).toInt();
	(*jsonResponse)["minDuration"] = lite ? -1 : oldDurations.value("minDuration", -1).toInt();
	(*jsonResponse)["duration"] = duration;
}


(*jsonResponse)["mapid"] = mapid;
(*jsonResponse)["missionid"] = missionid;
(*jsonResponse)["gameid"] = gameid;
(*jsonResponse)["level"] = level;
(*jsonResponse)["finished"] = true;
(*jsonResponse)["success"] = success;
(*jsonResponse)["level"] = level;
(*jsonResponse)["deathmatch"] = deathmatch;
(*jsonResponse)["lite"] = lite;
(*jsonResponse)["flawless"] = (success && flawless);
(*jsonResponse)["solvedCount"] = oldSolver.solve(level, deathmatch).solved(level, deathmatch);


return true;

*/
