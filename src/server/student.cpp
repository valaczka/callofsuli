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

	QVariantList uuidList = m_client->db()->execSelectQuery("SELECT mapid FROM studentGroupInfo "
"LEFT JOIN bindGroupMap ON (bindGroupMap.groupid=studentGroupInfo.id) WHERE username=? AND id=?"
, l);

	QJsonArray list;

	foreach (QVariant v, uuidList) {
		QVariantList l;
		l.append(v.toMap().value("mapid").toString());

		QVariantMap map = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, name, version, lastModified, md5, "
																	"COALESCE(LENGTH(data),0) as dataSize FROM maps WHERE uuid=?", l);

		list.append(QJsonObject::fromVariantMap(map));
	}

	(*jsonResponse)["list"] = list;

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

	QVariantList l;

	l.append(mapuuid);
	l.append(m_client->clientUserName());

	QVariantList r = m_client->db()->execSelectQuery("SELECT DISTINCT missionid, "
													"(SELECT max(level) FROM game g WHERE g.missionid=game.missionid "
													"AND g.success=true AND g.username=game.username) as maxLevel "
													"FROM game WHERE mapid=? AND username=?", l);

	(*jsonResponse)["list"] = QJsonArray::fromVariantList(r);


	// Base XP

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT value FROM settings WHERE key='xp.base'");

	int baseXP = m.value("value", 100).toInt();

	(*jsonResponse)["baseXP"] = baseXP;

	return true;
}




/**
 * @brief Student::gameNew
 * @param jsonResponse
 * @return
 */

bool Student::gameCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString mapuuid = params.value("map").toString();
	QString missionuuid = params.value("mission").toString();
	int level = params.value("level", -1).toInt();

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
	(*jsonResponse)["hasSolved"] = params.value("hasSolved", false).toBool();
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
 * @brief Student::gameFinish
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

	QVariantList l;

	l.append(m_client->clientUserName());
	l.append(gameid);

	QVariantMap r = m_client->db()->execSelectQueryOneRow("SELECT id, missionid, level FROM game WHERE username=? AND id=? AND tmpScore IS NOT NULL", l);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid game";
		return false;
	}

	QVariantList ll;
	ll.append(success);
	if (duration > 0)
		ll.append(duration);
	else
		ll.append(QVariant::Invalid);
	ll.append(gameid);

	m_client->db()->execSimpleQuery("UPDATE game SET tmpScore=NULL, success=?, duration=? WHERE id=?", ll);

	QVariantMap m;
	m["username"] = m_client->clientUserName();
	m["xp"] = xp;
	m["gameid"] = gameid;
	int ret = m_client->db()->execInsertQuery("INSERT INTO score (?k?) VALUES (?)", m);

	if (ret == -1) {
		(*jsonResponse)["error"] = "sql error";
		return false;
	}



	l.clear();
	l.append(m_client->clientUserName());
	QVariantMap streak = m_client->db()->execSelectQueryOneRow("SELECT MAX(maxStreak) as streak FROM score WHERE username=?", l);
	int maxStreak = streak.value("streak", 0).toInt();


	l.append(m_client->clientUserName());
	QVariantMap cStreak = m_client->db()->execSelectQueryOneRow("SELECT streak FROM "
"(SELECT MAX(dt) as dt, COUNT(*) as streak FROM "
"(SELECT t1.dt as dt, date(t1.dt,-(select count(*) FROM "
"(SELECT DISTINCT date(timestamp) AS dt "
"FROM game WHERE username=? AND success=true) t2 "
"WHERE t2.dt<=t1.dt)||' day', 'localtime') as grp FROM "
"(SELECT DISTINCT date(timestamp, 'localtime') AS dt FROM game "
"WHERE username=? AND success=true) t1) t GROUP BY grp) "
"WHERE dt=date('now', 'localtime')", l);

	int currentStreak = cStreak.value("streak", 0).toInt();

	int streakXP = 0;


	if (currentStreak > maxStreak && currentStreak > 1) {
		qInfo().noquote() << tr("New streak '%1': %2").arg(m_client->clientUserName()).arg(currentStreak);

		QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT value FROM settings WHERE key='xp.base'");

		streakXP = m.value("value", 100).toInt() * currentStreak;

		QVariantMap p;
		p["maxStreak"] = currentStreak;
		p["xp"] = streakXP;
		p["username"] = m_client->clientUserName();

		if (m_client->db()->execInsertQuery("INSERT INTO score (?k?) VALUES (?)", p) == -1) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}

		maxStreak = currentStreak;
	}

	QVariantList lll;
	lll.append(m_client->clientUserName());
	lll.append(r.value("missionid").toString());
	lll.append(r.value("level", 0).toInt());

	lll.append(m_client->clientUserName());
	lll.append(r.value("missionid").toString());
	lll.append(r.value("level", 0).toInt());

	lll.append(m_client->clientUserName());
	lll.append(r.value("missionid").toString());
	lll.append(r.value("level", 0).toInt());

	lll.append(m_client->clientUserName());
	lll.append(r.value("missionid").toString());
	lll.append(r.value("level", 0).toInt());

	QVariantMap stat = m_client->db()->execSelectQueryOneRow("SELECT (SELECT count(*) FROM game WHERE username=? "
														"AND missionid=? AND level=? AND success=true) as solved, "
														"(SELECT count(*) FROM game WHERE username=? and missionid=? AND level=?) as tried,"
														"(SELECT MAX(duration) FROM game WHERE username=? and missionid=? "
														"AND level=? AND success=true) as maxDuration,"
														"(SELECT MIN(duration) FROM game WHERE username=? and missionid=? "
														"AND level=? AND success=true) as minDuration"
														, lll);

	(*jsonResponse)["gameid"] = gameid;
	(*jsonResponse)["finished"] = true;
	(*jsonResponse)["success"] = success;
	(*jsonResponse)["xp"] = xp;
	(*jsonResponse)["tried"] = stat.value("tried", 0).toInt();
	(*jsonResponse)["solved"] = stat.value("solved", 0).toInt();
	(*jsonResponse)["currentStreak"] = currentStreak;
	(*jsonResponse)["maxStreak"] = maxStreak;
	(*jsonResponse)["streakXP"] = streakXP;
	(*jsonResponse)["maxDuration"] = stat.value("maxDuration", -1).toInt();
	(*jsonResponse)["minDuration"] = stat.value("minDuration", -1).toInt();
	(*jsonResponse)["duration"] = duration;

	return true;
}
