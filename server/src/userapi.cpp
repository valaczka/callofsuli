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
#include "teacherapi.h"



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
				.addQuery("SELECT campaign.id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
						  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
						  "description, finished, groupid, defaultGrade, score.xp AS resultXP, campaignResult.gradeid AS resultGrade "
						  "FROM campaign LEFT JOIN campaignResult ON (campaignResult.campaignid=campaign.id	AND campaignResult.username=")
				.addValue(username)
				.addQuery(") LEFT JOIN score ON (campaignResult.scoreid=score.id) "
						  "WHERE started=true AND campaign.id=").addValue(id)
				.addQuery(" AND groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")")
				.execToJsonObject(&err);

		if (err)
			return responseErrorSql(response);

		if (obj.isEmpty())
			return responseError(response, "not found");

		const bool &finished = obj.value(QStringLiteral("finished")).toVariant().toBool();


		const TeacherAPI::UserCampaignResult &result = TeacherAPI::_campaignUserResult(this, id, finished, username, true, &err);

		if (err)
			return responseErrorSql(response);

		if (!finished) {
			obj[QStringLiteral("resultXP")] = result.xp > 0 ? result.xp : QJsonValue::Null;
			obj[QStringLiteral("resultGrade")] = result.grade > 0 ? result.grade : QJsonValue::Null;
		}

		obj[QStringLiteral("taskList")] = result.tasks;

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
			if (response.data())
				response.data()->setStatus(HttpStatus::Ok, b);
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

	TeacherAPI::UserGame g;

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
	const int &gameid = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, gameid, data]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		LOG_CTRACE("client") << "Update game" << gameid << "for user:" << qPrintable(username);

		QueryBuilder qq(db);

		qq.addQuery("SELECT mapid, missionid, level, deathmatch, mode, campaignid FROM game "
					"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
					"WHERE runningGame.gameid=game.id AND game.id=").addValue(gameid)
				.addQuery(" AND username=").addValue(username);

		if (!qq.exec())
			return responseErrorSql(response);

		if (!qq.sqlQuery().first())
			return responseError(response, "invalid game");


		// Statistics

		if (data.contains(QStringLiteral("statistics")))
			_addStatistics(data.value(QStringLiteral("statistics")).toArray());

		// XP

		if (QueryBuilder::q(db).addQuery("UPDATE runningGame SET xp=").addValue(data.value(QStringLiteral("xp")).toInt())
				.addQuery(" WHERE gameid=").addValue(gameid).exec())
			return responseAnswerOk(response);
		else
			return responseErrorSql(response);
	});
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

		qq.addQuery("SELECT mapid, missionid, level, deathmatch, mode, campaignid FROM game "
					"LEFT JOIN runningGame ON (runningGame.gameid=game.id) "
					"WHERE runningGame.gameid=game.id AND game.id=").addValue(gameid)
				.addQuery(" AND username=").addValue(username);

		if (!qq.exec())
			return responseErrorSql(response);

		if (!qq.sqlQuery().first())
			return responseError(response, "invalid game");


		// Statistics

		if (data.contains(QStringLiteral("statistics")))
			_addStatistics(data.value(QStringLiteral("statistics")).toArray());



		TeacherAPI::UserGame g;

		g.map = qq.value("mapid").toString();
		g.mission = qq.value("missionid").toString();
		g.level = qq.value("level").toInt();
		g.deathmatch = qq.value("deathmatch").toBool();
		g.mode = qq.value("mode").value<GameMap::GameMode>();
		g.campaign = qq.value("campaignid", -1).toInt();



		const bool &success = data.value(QStringLiteral("success")).toVariant().toBool();
		const int &xp = data.value(QStringLiteral("xp")).toInt();
		const int &duration = data.value(QStringLiteral("duration")).toInt();
		int sumXP = xp;



		QJsonObject ret;

		const int &baseXP = m_service->config().get("gameBaseXP").toInt(100);
		const int &oldSolved = solverInfo(this, username, g.map, g.mission, g.level, g.deathmatch);

		if (success) {
			// Solved XP

			const int &xpSolved = GameMap::computeSolvedXpFactor(g.level, g.deathmatch, oldSolved, g.mode) * baseXP;

			sumXP += xpSolved;
			ret[QStringLiteral("xpSolved")] = xpSolved;

			// Duration XP

			bool err = false;

			const int &shortestDuration = QueryBuilder::q(db)
					.addQuery("SELECT COALESCE(MIN(duration),0) AS duration FROM game "
							  "WHERE success=true AND username=").addValue(username)
					.addQuery(" AND mapid=").addValue(g.map)
					.addQuery(" AND missionid=").addValue(g.mission)
					.addQuery(" AND level=").addValue(g.level)
					.addQuery(" AND mode=").addValue(g.mode)
					.execToValue("duration", &err).toInt();

			if (err)
				return responseErrorSql(response);

			if (shortestDuration > 0 && duration < shortestDuration) {
				const int &durationXP = (shortestDuration-duration)/1000 * baseXP * XP_FACTOR_DURATION_SEC;
				sumXP += durationXP;
				ret[QStringLiteral("xpDuration")] = durationXP;
			}



			// Streak XP

			const int &longestStreak = QueryBuilder::q(db)
					.addQuery("SELECT COALESCE(MAX(streak),0) AS streak FROM streak WHERE username=").addValue(username)
					.execToValue("streak", &err).toInt();

			if (err)
				return responseErrorSql(response);

			QueryBuilder q(db);
			q.addQuery("SELECT COALESCE(streak, 0) AS streak, COALESCE((ended_on = date('now')), false) AS streakToday "
						"FROM streak WHERE ended_on >= date('now', '-1 day') AND username=").addValue(username);

			if (!q.exec())
				return responseErrorSql(response);

			q.sqlQuery().first();

			const bool &sToday = q.value("streakToday", false).toBool();
			const int &streak = q.value("streak", 0).toInt();

			if (!sToday && streak > 0) {
				if (streak+1 > longestStreak) {
					const int &streakXP = (streak+1) * baseXP * XP_FACTOR_NEW_STREAK;
					sumXP += streakXP;
					ret[QStringLiteral("longestStreak")] = true;
					ret[QStringLiteral("xpStreak")] = streakXP;
				} else {
					const int &streakXP = (streak+1) * baseXP * XP_FACTOR_STREAK;
					sumXP += streakXP;
					ret[QStringLiteral("longestStreak")] = false;
					ret[QStringLiteral("xpStreak")] = streakXP;
				}
				ret[QStringLiteral("streak")] = streak+1;
			}

		}

		ret[QStringLiteral("sumXP")] = sumXP;
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

		if (success) {
			if (!TeacherAPI::_evaluateCampaign(this, g.campaign, username))
				return responseErrorSql(response);
		}

		responseAnswerOk(response, ret);
	});

}




/**
 * @brief UserAPI::_addStatistics
 * @param username
 * @param list
 */

void UserAPI::_addStatistics(const QJsonArray &list) const
{
	if (list.isEmpty())
		return;

	QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

	QMutexLocker(databaseMain()->mutex());

	const QString &username = m_credential.username();

	foreach (const QJsonValue &v, list) {
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

