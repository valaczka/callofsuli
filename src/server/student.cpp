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
 * @brief Student::mapListGet
 * @param jsonResponse
 * @return
 */

bool Student::mapListGet(QJsonObject *jsonResponse, QByteArray *)
{
	/*QVariantList params;

	params.append(m_client->clientUserName()); */

	QVariantList mapList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name, version, lastModified, md5, COALESCE(LENGTH(data),0) as dataSize "
"FROM maps");

	(*jsonResponse)["list"] = QJsonArray::fromVariantList(mapList);

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
	bool success = params.value("success", false).toBool();

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
	ll.append(success);
	ll.append(gameid);
	m_client->db()->execSimpleQuery("UPDATE game SET tmpScore=NULL, success=? WHERE id=?", ll);

	QVariantMap m;
	m["username"] = m_client->clientUserName();
	m["xp"] = xp;
	m["gameid"] = gameid;
	int ret = m_client->db()->execInsertQuery("INSERT INTO score (?k?) VALUES (?)", m);

	if (ret == -1) {
		(*jsonResponse)["error"] = "sql error";
		return false;
	}

	(*jsonResponse)["gameid"] = gameid;
	(*jsonResponse)["finished"] = true;
	return true;
}
