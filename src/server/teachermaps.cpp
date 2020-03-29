/*
 * ---- Call of Suli ----
 *
 * teachermaps.cpp
 *
 * Created on: 2020. 03. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMaps
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

#include "teachermaps.h"

TeacherMaps::TeacherMaps(Client *client, const QJsonObject &object)
	: AbstractHandler(client, object)
{

}


/**
 * @brief TeacherMaps::classInit
 * @return
 */

bool TeacherMaps::classInit()
{
	if (!m_client->clientRoles().testFlag(Client::RoleTeacher))
		return false;

	return true;
}


/**
 * @brief TeacherMaps::getAllMap
 * @return
 */

QJsonObject TeacherMaps::getAllMap()
{
	QJsonObject ret;

	QVariantList params;
	params << m_client->clientUserName();

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT id, name, timeCreated, timeModified, version, objectives, "
									"EXISTS(SELECT * FROM bindGroupMap WHERE mapid=map.id) AS hasGroup, "
									"EXISTS(SELECT * FROM score WHERE mapid=map.id) AS hasScore "
									"FROM map WHERE owner=?",
									params,
									&l);
	ret["list"] = l;

	return ret;
}


/**
 * @brief TeacherMaps::getMapData
 * @return
 */

QJsonObject TeacherMaps::getMap()
{
	QJsonObject ret;

	int refid = m_object["id"].toInt();

	QVariantList params;
	params << refid;
	params << m_client->clientUserName();

	if (!m_client->db()->execSelectQueryOneRow("SELECT id, name, version, timeCreated, timeModified, objectives, "
											   "EXISTS(SELECT * FROM bindGroupMap WHERE mapid=map.id) AS hasGroup, "
											   "EXISTS(SELECT * FROM score WHERE mapid=map.id) AS hasScore "
											   " FROM map WHERE id=? AND owner=?", params, &ret))
		return ret;

	if (ret.isEmpty())
		return ret;

	QVariantList l;
	l << refid;

	m_client->mapDb()->db()->execSelectQueryOneRow("SELECT uuid, md5 FROM mapdata WHERE refid=?", l, &ret);

	m_client->sendMap("teacherMaps", refid, ret);

	return ret;
}


/**
 * @brief TeacherMaps::createMap
 * @return
 */

QJsonObject TeacherMaps::createMap()
{
	QJsonObject ret;

	QString name = m_object["name"].toString();

	QVariantMap params;
	params["name"] = name;
	params["owner"] = m_client->clientUserName();
	int id = m_client->db()->execInsertQuery("INSERT INTO map (?k?) VALUES (?)", params);

	if (id == -1) {
		ret["error"] = "map create error";
		return ret;
	}

	QVariantMap info = m_client->mapDb()->create(id);

	ret["created"] = id;
	return ret;
}


/**
 * @brief TeacherMaps::updateMapInfo
 */

void TeacherMaps::updateMapInfo(const int &id, const QVariantMap &params, const bool &increaseVersion)
{
	QVariantMap binds;
	binds[":id"] = id;
	binds[":owner"] = m_client->clientUserName();

	m_client->db()->execUpdateQuery("UPDATE map SET ? WHERE id=:id AND owner=:owner", params, binds);

	if (increaseVersion)
		m_client->db()->execUpdateQuery("UPDATE map SET version=version+1 WHERE id=:id AND owner=:owner", params, binds);

}
