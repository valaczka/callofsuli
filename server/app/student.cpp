/*
 * ---- Call of Suli ----
 *
 * student.cpp
 *
 * Created on: 2020. 12. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Student
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "student.h"
#include "admin.h"
#include "userinfo.h"
#include "teacher.h"


Student::Student(Client *client, const CosMessage &message)
	: AbstractHandler(client, message, CosMessage::ClassStudent)
{

}


/**
 * @brief Student::classInit
 * @return
 */

bool Student::classInit()
{
	if (!m_client->clientRoles().testFlag(CosMessage::RoleStudent))
		return false;

	return true;
}



/**
 * @brief Student::groupListGet
 * @param jsonResponse
 * @return
 */

bool Student::groupListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList list = m_client->db()->execSelectQuery("SELECT studentGroupInfo.id as id, name, "
														"user.firstname as teacherfirstname, user.lastname as teacherlastname, "
														"(SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
														"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
														"WHERE bindGroupClass.groupid=studentGroupInfo.id) as readableClassList "
														"FROM studentGroupInfo "
														"LEFT JOIN user ON (user.username=studentGroupInfo.owner) "
														"WHERE studentGroupInfo.username=?", {m_client->clientUserName()});

	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);

	return true;
}


/**
 * @brief Student::mapListGet
 * @param jsonResponse
 * @return
 */

bool Student::mapListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int groupid = params.value("groupid", -1).toInt();

	if (groupid == -1) {
		(*jsonResponse)["list"] = QJsonArray();
		(*jsonResponse)["error"] = "missing group";
		return true;
	}

	QVariantList l;

	l.append(m_client->clientUserName());
	l.append(groupid);

	QVariantList uuidList = m_client->db()->execSelectQuery("SELECT mapid, active FROM studentGroupInfo "
															"INNER JOIN bindGroupMap ON (bindGroupMap.groupid=studentGroupInfo.id) WHERE username=? AND studentGroupInfo.id=?"
															, l);

	QJsonArray list;

	foreach (QVariant v, uuidList) {
		QVariantMap m = v.toMap();
		QVariantList l;
		l.append(m.value("mapid").toString());

		QVariantMap map = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, name, version, datetime(lastModified, 'localtime') as lastModified, md5, "
																	"COALESCE(LENGTH(data),0) as dataSize FROM maps WHERE uuid=?", l);

		map["active"] = m.value("active").toBool();

		list.append(QJsonObject::fromVariantMap(map));
	}

	(*jsonResponse)["list"] = list;

	return true;
}


/**
 * @brief Student::userListGet
 * @param jsonResponse
 * @return
 */

bool Student::userListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int groupid = params.value("groupid", -1).toInt();

	(*jsonResponse)["groupid"] = groupid;
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT userInfo.username, firstname, lastname, "
																						  "rankid, rankname, COALESCE(ranklevel, -1) as ranklevel, rankimage, nickname,"
																						  "t1, t2, t3, d1, d2, d3, sumxp "
																						  "FROM studentGroupInfo LEFT JOIN userInfo ON (studentGroupInfo.username=userInfo.username) "
																						  "LEFT JOIN groupTrophy ON (groupTrophy.username=studentGroupInfo.username AND groupTrophy.id=studentGroupInfo.id) "
																						  "WHERE active=true AND studentGroupInfo.id=?", {
																							  groupid
																						  }));

	return true;
}



/**
 * @brief Student::missionListGet
 * @param jsonResponse
 * @return
 */

bool Student::missionListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString mapuuid = params.value("map").toString();

	if (mapuuid.isEmpty()) {
		(*jsonResponse)["error"] = "missing map";
		return false;
	}


	QVariantList list = m_client->db()->execSelectQuery("SELECT DISTINCT missionid, "
														"(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=1 AND deathmatch=false "
														"AND success=true AND username=game.username AND mapid=game.mapid AND missionid=game.missionid) as t1, "
														"(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=2 AND deathmatch=false "
														"AND success=true AND username=game.username AND mapid=game.mapid AND missionid=game.missionid) as t2, "
														"(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=3 AND deathmatch=false "
														"AND success=true AND username=game.username AND mapid=game.mapid AND missionid=game.missionid) as t3, "
														"(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=1 AND deathmatch=true "
														"AND success=true AND username=game.username AND mapid=game.mapid AND missionid=game.missionid) as d1, "
														"(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=2 AND deathmatch=true "
														"AND success=true AND username=game.username AND mapid=game.mapid AND missionid=game.missionid) as d2, "
														"(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=3 AND deathmatch=true "
														"AND success=true AND username=game.username AND mapid=game.mapid AND missionid=game.missionid) as d3 "
														"FROM game WHERE username=? AND mapid=?"
														, {
															m_client->clientUserName(),
															mapuuid
														});


	(*jsonResponse)["uuid"] = mapuuid;
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);


	// Base XP

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT value FROM settings WHERE key='xp.base'");

	int baseXP = m.value("value", 100).toInt();

	(*jsonResponse)["baseXP"] = baseXP;

	return true;
}



/**
 * @brief Student::campaignGet
 * @param jsonResponse
 * @return
 */

bool Student::campaignGet(QJsonObject *jsonResponse, QByteArray *)
{
	const QVariantMap params = m_message.jsonData().toVariantMap();
	const int groupid = params.value("groupid", -1).toInt();

	if (groupid == -1) {
		(*jsonResponse)["error"] = "missing group";
		return false;
	}

	QList<int> campignIds;

	if (params.contains("id")) {
		campignIds.append(params.value("id").toInt());
	}

	if (params.contains("list")) {
		foreach (QVariant v, params.value("list").toList())
			campignIds.append(v.toInt());
	}

	if (campignIds.isEmpty()) {
		QVariantList l = m_client->db()->execSelectQuery("SELECT id FROM campaign WHERE groupid=? AND started=true AND finished=false", {groupid});
		foreach (QVariant v, l)
			campignIds.append(v.toMap().value("id").toInt());
	}


	// Grades

	(*jsonResponse)["gradeList"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id, shortname, longname, value FROM grade"));


	// Campaigns

	QJsonArray list;

	foreach (const int &id, campignIds) {
		const QVariantMap jm = m_client->db()->execSelectQueryOneRow("SELECT id, datetime(starttime, 'localtime') as starttime, "
																	 "datetime(endtime, 'localtime') as endtime, "
																	 "description, started, finished FROM campaign "
																	 "WHERE id=?", {id});

		QJsonObject jo = QJsonObject::fromVariantMap(jm);

		const bool isFinished = jm.value("finished", false).toBool();

		QVariantList assList = m_client->db()->execSelectQuery("SELECT id, name FROM assignment WHERE campaignid=?", {id});

		QJsonArray jlist;

		foreach (const QVariant &v, assList) {
			const int aid = v.toMap().value("id").toInt();

			QJsonArray grades = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT datetime(timestamp, 'localtime') as timestamp, "
																							"gradeid, -1 as xp, false as forecast "
																							"FROM gradebook "
																							"WHERE assignmentid=? AND username=? "
																							"UNION "
																							"SELECT datetime(timestamp, 'localtime') as timestamp, "
																							"-1 as gradeid, xp, false as forecast "
																							"FROM score WHERE assignmentid=? AND username=?"
																							, {aid, m_client->clientUserName(),
																							   aid, m_client->clientUserName()}));


			Teacher t(m_client, CosMessage());
			QVector<Teacher::Grading> grading = t.gradingGet(aid, id, m_client->clientUserName(), isFinished);


			if (grades.isEmpty() && !isFinished) {
				Teacher::Grading g = Teacher::gradingResult(grading, Teacher::Grading::TypeGrade);
				if (g.isValid()) {
					grades.append(QJsonObject({
												  { "timestamp", "" },
												  { "gradeid", g.ref },
												  { "xp", -1 },
												  { "forecast", true }
											  }));
				}


				Teacher::Grading g2 = Teacher::gradingResult(grading, Teacher::Grading::TypeXP);
				if (g2.isValid()) {
					grades.append(QJsonObject({
												  { "timestamp", "" },
												  { "gradeid", -1 },
												  { "xp", g2.value },
												  { "forecast", true }
											  }));
				}
			}


			QJsonObject ja;

			ja["id"] = aid;
			ja["name"] = v.toMap().value("name").toString();
			ja["grades"] = grades;
			ja["grading"] = Teacher::Grading::toNestedArray(grading);

			jlist.append(ja);
		}

		jo["assignment"] = jlist;
		list.append(jo);
	}


	(*jsonResponse)["list"] = list;

	return true;
}



/**
 * @brief Student::campaignListGet
 * @param jsonResponse
 * @return
 */

bool Student::campaignListGet(QJsonObject *jsonResponse, QByteArray *)
{
	const QVariantMap params = m_message.jsonData().toVariantMap();
	const int groupid = params.value("groupid", -1).toInt();

	if (groupid == -1) {
		(*jsonResponse)["error"] = "missing group";
		return false;
	}

	QJsonArray list = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id, datetime(starttime, 'localtime') as starttime, "
																				  "datetime(endtime, 'localtime') as endtime, "
																				  "finished, description FROM campaign "
																				  "WHERE groupid=? AND started=true", {groupid}));

	(*jsonResponse)["list"] = list;

	return true;
}






/**
 * @brief Student::gameCreate
 * @param jsonResponse
 * @return
 */


bool Student::gameCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString mapuuid = params.value("map").toString();
	QString missionuuid = params.value("mission").toString();
	int level = params.value("level", -1).toInt();
	bool deathmatch = params.value("deathmatch", false).toBool();
	bool lite = params.value("lite", false).toBool();

	if (mapuuid.isEmpty() || missionuuid.isEmpty() || level <= 0) {
		(*jsonResponse)["error"] = "missing map or mission";
		return false;
	}

	QVariantList l;

	l.append(m_client->clientUserName());


	// Folyamatban lévő játékok lezárása

	QVariantList r = m_client->db()->execSelectQuery("SELECT id FROM game WHERE username=? AND tmpScore IS NOT NULL", l);

	if (!r.isEmpty()) {
		m_client->db()->execSimpleQuery("INSERT INTO score (username, xp, gameid) "
										"SELECT username, tmpScore, id FROM game WHERE username=? AND tmpScore IS NOT NULL", l);

		m_client->db()->execSimpleQuery("UPDATE game SET tmpScore=null WHERE username=?", l);
	}


	// Új játék készítése

	QVariantMap m;
	m["username"] = m_client->clientUserName();
	m["mapid"] = mapuuid;
	m["missionid"] = missionuuid;
	m["level"] = level;
	m["success"] = false;
	m["deathmatch"] = deathmatch;
	m["lite"] = lite;
	m["tmpScore"] = 0;

	int rowid = m_client->db()->execInsertQuery("INSERT INTO game (?k?) VALUES (?)", m);

	if (rowid == -1) {
		(*jsonResponse)["error"] = "sql error";
		return false;
	}

	(*jsonResponse)["created"] = true;
	(*jsonResponse)["gameid"] = rowid;
	(*jsonResponse)["missionid"] = missionuuid;
	(*jsonResponse)["level"] = level;
	(*jsonResponse)["deathmatch"] = deathmatch;
	(*jsonResponse)["lite"] = lite;
	return true;
}



/**
 * @brief Student::gameUpdate
 * @param jsonResponse
 * @return
 */

bool Student::gameUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int gameid = params.value("id", -1).toInt();
	int xp = params.value("xp", -1).toInt();

	if (gameid < 0 || xp < 0) {
		(*jsonResponse)["error"] = "missing id or xp";
		return false;
	}

	updateStatistics(params.value("stat").toList());


	QVariantMap r = m_client->db()->execSelectQueryOneRow("SELECT id FROM game WHERE username=? AND id=? AND tmpScore IS NOT NULL",
														  {m_client->clientUserName(), gameid});

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid game";
		return false;
	}

	m_client->db()->execSimpleQuery("UPDATE game SET tmpScore=? WHERE id=?", {xp, gameid});

	(*jsonResponse)["gameid"] = gameid;
	(*jsonResponse)["updated"] = true;
	return true;
}




/**
 * @brief StudeQJsonObjectish
 * @param jsonResponse
 * @return
 */

bool Student::gameFinish(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int gameid = params.value("id", -1).toInt();
	int xp = params.value("xp", -1).toInt();
	int duration = params.value("duration", -1).toInt();
	bool success = params.value("success", false).toBool();

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
	(*jsonResponse)["solvedCount"] = oldSolver.solve(level, deathmatch).solved(level, deathmatch);


	return true;
}


/**
 * @brief Student::gameListUserGet
 * @param jsonResponse
 * @return
 */

bool Student::gameListUserGet(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();

	QString username = params.value("username").toString();
	if (username.isEmpty())
		username = m_client->clientUserName();

	QJsonObject o;

	o["username"] = username;
	o["groupid"] = params.value("groupid").toInt(-1);
	o["limit"] = params.value("limit").toInt(50);
	o["offset"] = params.value("offset").toInt();

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Teacher u(m_client, m2);
	return u.gameListUserGet(jsonResponse, nullptr);
}



/**
 * @brief Student::gameListUserMissionGet
 * @param jsonResponse
 * @return
 */

bool Student::gameListUserMissionGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString missionid = params.value("missionid").toString();
	int level = params.value("level", -1).toInt();
	bool deathmatch = params.value("deathmatch", false).toBool();
	bool lite = false; //params.value("lite", false).toBool();			// Nem vesszük figyelembe a feladatmegoldást

	if (missionid.isEmpty() || level < 0) {
		(*jsonResponse)["error"] = "missing missionid or level";
		return false;
	}


	QVariantList list = m_client->db()->execSelectQuery("SELECT ROW_NUMBER() OVER (ORDER BY MIN(duration)) durationNum, "
														"ROW_NUMBER() OVER (ORDER BY COUNT(*) DESC, MIN(duration)) successNum, "
														"game.username, firstname, lastname, nickname, rankid, COALESCE(ranklevel, -1) as ranklevel, rankimage, rankname, "
														"MIN(duration) as duration, COUNT(*) as success "
														"FROM game LEFT JOIN userInfo ON (userInfo.username=game.username) "
														"WHERE missionid=? AND level=? AND deathmatch=? "
														"AND success=true AND lite=? AND tmpScore IS NULL "
														"GROUP BY game.username",
														{missionid, level, deathmatch, lite}
														);

	(*jsonResponse)["minDuration"] = m_client->db()->execSelectQueryOneRow("SELECT COALESCE(MIN(duration),0) as v "
																		   "FROM game "
																		   "WHERE missionid=? AND level=? AND deathmatch=? "
																		   "AND success=true AND lite=? AND tmpScore IS NULL",
																		   {missionid, level, deathmatch, lite}
																		   ).value("v").toInt();

	(*jsonResponse)["maxDuration"] = m_client->db()->execSelectQueryOneRow("SELECT COALESCE(MAX(duration),0) as v "
																		   "FROM game "
																		   "WHERE missionid=? AND level=? AND deathmatch=? "
																		   "AND success=true AND lite=? AND tmpScore IS NULL",
																		   {missionid, level, deathmatch, lite}
																		   ).value("v").toInt();

	(*jsonResponse)["maxSuccess"] = m_client->db()->execSelectQueryOneRow("SELECT COALESCE(MAX(success),0) as v FROM "
																		  "(SELECT COUNT(*) as success "
																		  "FROM game LEFT JOIN userInfo ON (userInfo.username=game.username) "
																		  "WHERE missionid=? AND level=? AND deathmatch=? "
																		  "AND success=true AND lite=? AND tmpScore IS NULL "
																		  "GROUP BY game.username) t",
																		  {missionid, level, deathmatch, lite}
																		  ).value("v").toInt();

	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);
	(*jsonResponse)["missionid"] = missionid;
	(*jsonResponse)["level"] = level;
	(*jsonResponse)["deathmatch"] = deathmatch;
	//(*jsonResponse)["lite"] = lite;

	return true;
}


/**
 * @brief Student::userGet
 * @param jsonResponse
 * @return
 */

bool Student::userGet(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject o = m_message.jsonData();

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	UserInfo u(m_client, m2);
	return u.getUser(jsonResponse, nullptr);
}



/**
 * @brief Student::userModify
 * @param jsonResponse
 * @return
 */

bool Student::userModify(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();

	bool disabledNameModification = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='user.disableNameModification'")
									.value("v", false).toBool();

	bool oauth2Account = m_client->db()->execSelectQueryOneRow("SELECT (password='*') as v FROM auth WHERE username=?", {m_client->clientUserName()})
						 .value("v", false).toBool();

	QJsonObject o;

	QStringList p;
	p.append("nickname");
	p.append("character");

	if (!oauth2Account)
		p.append("picture");

	if (!disabledNameModification && !oauth2Account) {
		p.append("firstname");
		p.append("lastname");
	}

	foreach (QString s, p) {
		if (params.contains(s))
			o[s] = params.value(s);
	}

	if (o.isEmpty()) {
		(*jsonResponse)["error"] = "missing parameter";
		return false;
	}

	o["username"] = m_client->clientUserName();

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Admin u(m_client, m2);
	return u.userModify(jsonResponse, nullptr);
}



/**
 * @brief Student::userPasswordChange
 * @param jsonResponse
 * @return
 */

bool Student::userPasswordChange(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();
	QString password = params.value("password").toString();

	if (password.isEmpty()) {
		(*jsonResponse)["error"] = "missing password";
		return false;
	}

	if (m_client->db()->execSelectQueryOneRow("SELECT (password='*') as v FROM auth WHERE username=?", {m_client->clientUserName()})
		.value("v", false).toBool()) {
		(*jsonResponse)["error"] = "oauth2 account";
		return false;
	}

	QJsonObject o;

	o["username"] = m_client->clientUserName();
	o["password"] = password;
	o["oldPassword"] = params.value("oldPassword").toString();

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Admin u(m_client, m2);
	return u.userPasswordChange(jsonResponse, nullptr);
}




/**
 * @brief Student::missionStatGet
 * @param mapid
 * @param missionid
 * @return
 */

GameMap::SolverInfo Student::missionSolverInfo(const QString &mapid, const QString &missionid) const
{
	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT "
														  "(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=1 AND deathmatch=false "
														  "AND success=true AND username=g.username AND mapid=g.mapid AND missionid=g.missionid) as t1, "
														  "(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=2 AND deathmatch=false "
														  "AND success=true AND username=g.username AND mapid=g.mapid AND missionid=g.missionid) as t2, "
														  "(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=3 AND deathmatch=false "
														  "AND success=true AND username=g.username AND mapid=g.mapid AND missionid=g.missionid) as t3, "
														  "(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=1 AND deathmatch=true "
														  "AND success=true AND username=g.username AND mapid=g.mapid AND missionid=g.missionid) as d1, "
														  "(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=2 AND deathmatch=true "
														  "AND success=true AND username=g.username AND mapid=g.mapid AND missionid=g.missionid) as d2, "
														  "(SELECT COALESCE(num, 0) FROM missionTrophy WHERE level=3 AND deathmatch=true "
														  "AND success=true AND username=g.username AND mapid=g.mapid AND missionid=g.missionid) as d3 "
														  "FROM (SELECT ? as username, ? as mapid, ? as missionid) AS g"
														  , {
															  m_client->clientUserName(),
															  mapid,
															  missionid
														  });


	return GameMap::SolverInfo(m);
}



/**
 * @brief Student::updateStatistics
 * @param list
 * @return
 */

void Student::updateStatistics(const QVariantList &list)
{
	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();

		m_client->statDb()->execInsertQuery("INSERT INTO stat(?k?) VALUES (?)", {
												{ "username", m_client->clientUserName()},
												{ "map", m.value("map", "").toString() },
												{ "objective", m.value("objective", "").toString() },
												{ "success", m.value("success", false).toBool() },
												{ "elapsed", m.value("elapsed", 0).toInt() }
											});
	}
}
