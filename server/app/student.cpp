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
	QVariantList params;

	params.append(m_client->clientUserName());

	QVariantList list = m_client->db()->execSelectQuery("SELECT studentGroupInfo.id as id, name, "
"user.firstname as teacherfirstname, user.lastname as teacherlastname, "
"(SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
"WHERE bindGroupClass.groupid=studentGroupInfo.id) as readableClassList "
"FROM studentGroupInfo "
"LEFT JOIN user ON (user.username=studentGroupInfo.owner) "
"WHERE studentGroupInfo.username=?", params);

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
"LEFT JOIN bindGroupMap ON (bindGroupMap.groupid=studentGroupInfo.id) WHERE username=? AND studentGroupInfo.id=?"
, l);

	QJsonArray list;

	foreach (QVariant v, uuidList) {
		QVariantMap m = v.toMap();
		QVariantList l;
		l.append(m.value("mapid").toString());

		QVariantMap map = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, name, version, lastModified, md5, "
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

	(*jsonResponse)["list"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT userInfo.username, firstname, lastname, "
									"rankid, rankname, ranklevel, rankimage, nickname,"
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

	QVariantList l;

	l.append(m_client->clientUserName());
	l.append(gameid);

	QVariantMap r = m_client->db()->execSelectQueryOneRow("SELECT id FROM game WHERE username=? AND id=? AND tmpScore IS NOT NULL", l);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid game";
		return false;
	}

	QVariantList ll;
	ll.append(xp);
	ll.append(gameid);
	m_client->db()->execSimpleQuery("UPDATE game SET tmpScore=? WHERE id=?", ll);

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


	QVariantMap r = m_client->db()->execSelectQueryOneRow("SELECT id, mapid, missionid, level, deathmatch FROM game "
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


	QJsonObject xpObject;

	// Eredeti állapot

	GameMap::SolverInfo oldSolver = missionSolverInfo(mapid, missionid);

	QVariantMap oldDurations = m_client->db()->execSelectQueryOneRow("SELECT "
														"(SELECT MAX(duration) FROM game WHERE username=? and missionid=? "
														"AND level=? AND success=true) as maxDuration,"
														"(SELECT MIN(duration) FROM game WHERE username=? and missionid=? "
														"AND level=? AND success=true) as minDuration"
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

	int solvedXP = GameMap::computeSolvedXpFactor(oldSolver, level, deathmatch) * baseXP;
	int durationXP = 0;

	if (success) {
		int shortestDuration = oldDurations.value("minDuration", -1).toInt();

		if (shortestDuration > -1 && duration < shortestDuration) {
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
		(*jsonResponse)["maxDuration"] = oldDurations.value("maxDuration", -1).toInt();
		(*jsonResponse)["minDuration"] = oldDurations.value("minDuration", -1).toInt();
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
	(*jsonResponse)["solvedCount"] = oldSolver.solve(level, deathmatch).solved(level, deathmatch);


	return true;
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

	qInfo() << "m" << m;
	return GameMap::SolverInfo(m);
}
