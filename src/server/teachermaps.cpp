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

TeacherMaps::TeacherMaps(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: AbstractHandler(client, object, binaryData)
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

void TeacherMaps::getAllMap(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;
	params << m_client->clientUserName();

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT id, name, timeCreated, timeModified, version, objectives, "
									"EXISTS(SELECT * FROM bindGroupMap WHERE mapid=map.id) AS hasGroup, "
									"EXISTS(SELECT * FROM score WHERE mapid=map.id) AS hasScore "
									"FROM map WHERE owner=?",
									params,
									&l);
	(*jsonResponse)["list"] = l;
}


/**
 * @brief TeacherMaps::getMapData
 * @return
 */

void TeacherMaps::getMap(QJsonObject *jsonResponse, QByteArray *binaryResponse)
{
	int refid = m_jsonData["id"].toInt();

	QVariantList params;
	params << refid;
	params << m_client->clientUserName();

	if (!m_client->db()->execSelectQueryOneRow("SELECT id, name, version, timeCreated, timeModified, objectives, "
											   "EXISTS(SELECT * FROM bindGroupMap WHERE mapid=map.id) AS hasGroup, "
											   "EXISTS(SELECT * FROM score WHERE mapid=map.id) AS hasScore "
											   " FROM map WHERE id=? AND owner=?", params, jsonResponse)) {
		(*jsonResponse)["error"] = "internal db error";
		return;
	}

	if (jsonResponse->isEmpty()) {
		(*jsonResponse)["error"] = "map not found";
		return;
	}


	QVariantList l;
	l << refid;

	m_client->mapDb()->db()->execSelectQueryOneRow("SELECT uuid, md5 FROM mapdata WHERE refid=?", l, jsonResponse);



	QVariantMap mdata = m_client->mapDb()->db()->runSimpleQuery("SELECT data FROM mapdata WHERE refid=?", l);

	*binaryResponse = mdata["records"].toList().value(0).toMap().value("data").toByteArray();
	if (binaryResponse->isEmpty()) {
		qWarning() << tr("Map data not found") << refid;
		(*jsonResponse)["error"] = "map not found";
		return;
	}
}


/**
 * @brief TeacherMaps::createMap
 * @return
 */

void TeacherMaps::createMap(QJsonObject *jsonResponse, QByteArray *)
{
	QString name = m_jsonData["name"].toString();

	QVariantMap params;
	params["name"] = name;
	params["owner"] = m_client->clientUserName();
	int id = m_client->db()->execInsertQuery("INSERT INTO map (?k?) VALUES (?)", params);

	if (id == -1) {
		(*jsonResponse)["error"] = "map create error";
		return;
	}

	QVariantMap info = m_client->mapDb()->create(id);

	(*jsonResponse)["created"] = id;
}


/**
 * @brief TeacherMaps::updateMap
 * @return
 */

void TeacherMaps::updateMap(QJsonObject *jsonResponse, QByteArray *)
{
	int mapid = m_jsonData["id"].toInt();

	QJsonObject r = m_client->mapDb()->updateData(mapid, m_binaryData, m_jsonData["uuidOverwrite"].toBool(false));
	if (r.contains("error")) {
		(*jsonResponse)["error"] = r["error"].toString();
		return;
	}

	QVariantMap params;
	if (r.contains("timeCreated"))  params["timeCreated"] = r["timeCreated"].toString();
	if (r.contains("timeModified"))  params["timeModified"] = r["timeModified"].toString();

	params["objectives"] = r["objectives"].toInt();

	updateMapInfo(mapid, params, true);

	(*jsonResponse)["updated"] = mapid;
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
