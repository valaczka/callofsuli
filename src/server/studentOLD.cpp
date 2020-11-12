/*
 * ---- Call of Suli ----
 *
 * student.cpp
 *
 * Created on: 2020. 05. 31.
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

Student::Student(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: AbstractHandler(client, object, binaryData)
{

}


/**
 * @brief Student::classInit
 * @return
 */

bool Student::classInit()
{
	if (!m_client->clientRoles().testFlag(Client::RoleStudent) && !m_client->clientRoles().testFlag(Client::RoleTeacher))
		return false;

	return true;
}


/**
 * @brief Student::getMaps
 * @param jsonResponse
 */

void Student::getMaps(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;
	params << m_client->clientUserName();

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT groupid, groupname, groupowner, mapid, mapname, version, uuid, md5 "
									"FROM mapGroupInfo WHERE username=?",
									params,
									&l);
	(*jsonResponse)["list"] = l;
}


/**
 * @brief Student::getMapData
 * @param jsonResponse
 * @param binaryResponse
 */


void Student::getMapData(QJsonObject *jsonResponse, QByteArray *binaryResponse)
{
	int mapid = m_jsonData.value("id").toInt(-1);
	QString uuid = m_jsonData.value("uuid").toString();

	if (mapid != -1) {
		QVariantList params;
		params << mapid;
		params << m_client->clientUserName();

		m_client->db()->execSelectQueryOneRow("SELECT uuid FROM mapGroupInfo WHERE mapid=? and username=? GROUP BY uuid",
											  params, jsonResponse);
	} else if (!uuid.isEmpty()) {
		QVariantList params;
		params << uuid;
		params << m_client->clientUserName();

		m_client->db()->execSelectQueryOneRow("SELECT uuid FROM mapGroupInfo WHERE uuid=? and username=? GROUP BY uuid",
											  params, jsonResponse);
	}

	if (jsonResponse->isEmpty()) {
		(*jsonResponse)["error"] = "map not found";
		return;
	}

	QString realuuid = (*jsonResponse).value("uuid").toString();

	QVariantList l;
	l << realuuid;
/*
	m_client->mapDb()->db()->execSelectQueryOneRow("SELECT md5 FROM mapdata WHERE uuid=?", l, jsonResponse);

	QVariantMap mdata = m_client->mapDb()->db()->runSimpleQuery("SELECT data FROM mapdata WHERE uuid=?", l);

	*binaryResponse = mdata.value("records").toList().value(0).toMap().value("data").toByteArray();
	if (binaryResponse->isEmpty()) {
		qWarning() << tr("Map data not found") << uuid;
		(*jsonResponse)["error"] = "map data not found";
		return;
	}
	*/
}


/**
 * @brief Student::getMapResult
 * @param jsonResponse
 */

void Student::getMapResult(QJsonObject *jsonResponse, QByteArray *)
{
	QString uuid = m_jsonData.value("uuid").toString();

	QVariantList params;
	params << uuid;
	params << m_client->clientUserName();

	QJsonArray list;
	m_client->db()->execSelectQuery("SELECT mission.id as missionid, mission.uuid as uuid, level, attempt, success "
									"FROM mission "
									"LEFT JOIN missionResultInfo ON (missionResultInfo.missionid=mission.id) "
									"WHERE mapid=(SELECT id FROM map WHERE uuid=?) and username=?",
									params, &list);

	(*jsonResponse)["list"] = list;
}
