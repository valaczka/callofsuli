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
	m_client->db()->execSelectQuery("SELECT id, name, uuid, timeCreated, timeModified, version, objectives, "
									"EXISTS(SELECT * FROM bindGroupMap WHERE mapid=map.id) AS hasGroup "
									"FROM map WHERE owner=?",
									params,
									&l);
	(*jsonResponse)["list"] = l;
}


/**
 * @brief TeacherMaps::getMapData
 * @return
 */

void TeacherMaps::getMap(QJsonObject *jsonResponse, QByteArray *)
{
	int refid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << refid;
	params << m_client->clientUserName();

	if (!m_client->db()->execSelectQueryOneRow("SELECT id, name, uuid, version, timeCreated, timeModified, objectives, "
											   "EXISTS(SELECT * FROM bindGroupMap WHERE mapid=map.id) AS hasGroup "
											   "FROM map WHERE id=? AND owner=?", params, jsonResponse)) {
		(*jsonResponse)["error"] = "internal db error";
		return;
	}

	if (jsonResponse->isEmpty()) {
		(*jsonResponse)["error"] = "map not found";
		return;
	}


	QVariantList l;
	l << (*jsonResponse).value("uuid").toString();

	m_client->mapDb()->db()->execSelectQueryOneRow("SELECT md5 FROM mapdata WHERE uuid=?", l, jsonResponse);

}


/**
 * @brief TeacherMaps::getMapData
 * @param jsonResponse
 * @param binaryResponse
 */

void TeacherMaps::getMapData(QJsonObject *jsonResponse, QByteArray *binaryResponse)
{
	getMap(jsonResponse, binaryResponse);

	QString uuid = (*jsonResponse).value("uuid").toString();
	QVariantList l;
	l << uuid;

	QVariantMap mdata = m_client->mapDb()->db()->runSimpleQuery("SELECT data FROM mapdata WHERE uuid=?", l);

	*binaryResponse = mdata.value("records").toList().value(0).toMap().value("data").toByteArray();
	if (binaryResponse->isEmpty()) {
		qWarning() << tr("Map data not found") << uuid;
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
	m_client->db()->db().transaction();
	m_client->mapDb()->db()->db().transaction();

	QString name = m_jsonData.value("name").toString();

	QVariantMap params;
	params["name"] = name;
	params["owner"] = m_client->clientUserName();
	params["uuid"] = QUuid::createUuid().toString();
	int id = m_client->db()->execInsertQuery("INSERT INTO map (?k?) VALUES (?)", params);

	if (id == -1) {
		(*jsonResponse)["error"] = "map create error";
		m_client->db()->db().rollback();
		m_client->mapDb()->db()->db().rollback();
		return;
	}

	QVariantMap info = m_client->mapDb()->create(params.value("uuid").toString());

	if (info.value("id", -1).toInt() == -1) {
		(*jsonResponse)["error"] = "mapdb create error";
		m_client->db()->db().rollback();
		m_client->mapDb()->db()->db().rollback();
		return;
	}

	(*jsonResponse)["created"] = id;

	m_client->db()->db().commit();
	m_client->mapDb()->db()->db().commit();
}


/**
 * @brief TeacherMaps::updateMap
 * @param jsonResponse
 */

void TeacherMaps::updateMap(QJsonObject *jsonResponse, QByteArray *)
{
	int mapid = m_jsonData.value("id").toInt();

	m_client->db()->db().transaction();

	QVariantMap params;

	if (m_jsonData.contains("name"))
		params["name"] = m_jsonData.value("name").toString();

	if (!params.isEmpty()) {
		QVariantMap binds;
		binds[":id"] = mapid;
		binds[":owner"] = m_client->clientUserName();

		if (!m_client->db()->execUpdateQuery("UPDATE map set ? where id=:id AND owner=:owner", params, binds)) {
			(*jsonResponse)["error"] = "update error";
			m_client->db()->db().rollback();
			return;
		}
	}


	(*jsonResponse)["updated"] = mapid;

	m_client->db()->db().commit();
}


/**
 * @brief TeacherMaps::updateMap
 * @return
 */

void TeacherMaps::updateMapData(QJsonObject *jsonResponse, QByteArray *)
{
	int mapid = m_jsonData.value("id").toInt();

	m_client->db()->db().transaction();
	m_client->mapDb()->db()->db().transaction();

	QVariantList p;
	p << mapid;
	p << m_client->clientUserName();

	QVariantMap ret;
	if (!m_client->db()->execSelectQueryOneRow("SELECT uuid FROM map WHERE id=? AND owner=?", p, &ret)) {
		(*jsonResponse)["error"] = "update error";
		m_client->db()->db().rollback();
		m_client->mapDb()->db()->db().rollback();
		return;
	}

	QString uuid = ret.value("uuid").toString();

	if (uuid.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		m_client->db()->db().rollback();
		m_client->mapDb()->db()->db().rollback();
		return;
	}

	QJsonObject r = m_client->mapDb()->updateData(uuid, m_binaryData);
	if (r.contains("error")) {
		(*jsonResponse)["error"] = r.value("error").toString();
		m_client->db()->db().rollback();
		m_client->mapDb()->db()->db().rollback();
		return;
	}

	QVariantMap params;
	if (r.contains("timeCreated"))  params["timeCreated"] = r.value("timeCreated").toString();
	if (r.contains("timeModified"))  params["timeModified"] = r.value("timeModified").toString();

	params["objectives"] = r.value("objectives").toInt();
	params["md5"] = r.value("md5").toString();

	updateMapInfo(mapid, params, true);

	if (r.contains("uuidList")) {
		QJsonArray list = r.value("uuidList").toArray();

		foreach (QJsonValue v, list) {
			QString u = v.toString();
			QVariantMap params;
			params["uuid"] = u;
			params["mapid"] = mapid;

			if (m_client->db()->execInsertQuery("INSERT OR REPLACE INTO mission (?k?) VALUES (?)", params) == -1) {
				(*jsonResponse)["error"] = "update mission uuid error";
				m_client->db()->db().rollback();
				m_client->mapDb()->db()->db().rollback();
				return;
			}
		}
	}

	(*jsonResponse)["updated"] = mapid;

	m_client->db()->db().commit();
	m_client->mapDb()->db()->db().commit();
}


/**
 * @brief TeacherMaps::removeMap
 * @param jsonResponse
 */

void TeacherMaps::removeMap(QJsonObject *jsonResponse, QByteArray *)
{
	int mapid = m_jsonData.value("id").toInt(-1);

	m_client->db()->db().transaction();
	m_client->mapDb()->db()->db().transaction();

	if (mapid != -1) {
		QVariantList params;
		params << mapid;
		params << m_client->clientUserName();

		if (!m_client->db()->execSimpleQuery("DELETE FROM map WHERE id=? AND owner=?", params)) {
			(*jsonResponse)["error"] = "internal db error";
			m_client->db()->db().rollback();
			m_client->mapDb()->db()->db().rollback();
			return;
		}

		m_client->db()->db().commit();
		m_client->mapDb()->db()->db().commit();

		(*jsonResponse)["removed"] = mapid;
		return;
	}

	QVariantList list = m_jsonData.value("list").toArray().toVariantList();

	if (!list.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		m_client->db()->db().rollback();
		m_client->mapDb()->db()->db().rollback();
		return;
	}



	for (int i=0; i<list.count(); ++i) {
		int id = list.at(i).toInt();
		QVariantList l;
		l << id;
		l << m_client->clientUserName();

		QVariantMap p;
		m_client->db()->execSelectQueryOneRow("SELECT uuid FROM MAP WHERE id=? AND owner=?", l, &p);

		QString uuid = p.value("uuid").toString();

		if (uuid.isEmpty()) {
			(*jsonResponse)["error"] = "invalid id";
			m_client->db()->db().rollback();
			m_client->mapDb()->db()->db().rollback();
			return;
		}

		QJsonObject r = m_client->mapDb()->remove(uuid);
		if (r.contains("error")) {
			(*jsonResponse)["error"] = r.value("error").toString();
			m_client->db()->db().rollback();
			m_client->mapDb()->db()->db().rollback();
			return;
		}

		m_client->db()->execSimpleQuery("DELETE FROM MAP WHERE id=? AND owner=?", l);
	}


	m_client->db()->db().commit();
	m_client->mapDb()->db()->db().commit();

	(*jsonResponse)["removed"] = QJsonArray::fromVariantList(list);
}





/**
 * @brief TeacherMaps::updateMapInfo
 */

void TeacherMaps::updateMapInfo(const int &id, const QVariantMap &params, const bool &increaseVersion)
{
	QVariantMap binds;
	binds[":id"] = id;
	binds[":owner"] = m_client->clientUserName();

	m_client->db()->db().transaction();

	m_client->db()->execUpdateQuery("UPDATE map SET ? WHERE id=:id AND owner=:owner", params, binds);

	if (increaseVersion)
		m_client->db()->execUpdateQuery("UPDATE map SET version=version+1 WHERE id=:id AND owner=:owner", params, binds);

	m_client->db()->db().commit();

}
