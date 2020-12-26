/*
 * ---- Call of Suli ----
 *
 * teachermap.cpp
 *
 * Created on: 2020. 12. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMap
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

#include "teachermap.h"
#include "../common/gamemap.h"

/**
 * @brief TeacherMap::TeacherMap
 * @param client
 * @param message
 */

TeacherMap::TeacherMap(Client *client, const CosMessage &message)
	: AbstractHandler(client, message, CosMessage::ClassTeacherMap)
{

}


/**
 * @brief TeacherMap::classInit
 * @return
 */

bool TeacherMap::classInit()
{
	if (!m_client->clientRoles().testFlag(CosMessage::RoleTeacher))
		return false;

	return true;
}


/**
 * @brief TeacherMap::uploadTest
 * @param jsonResponse
 * @return
 */

bool TeacherMap::uploadTest(QJsonObject *jsonResponse, QByteArray *)
{
	/*QVariantMap params = m_message.jsonData().toVariantMap();
	QString username = params.value("username").toString();

	if (params.value("classid", -1) == -1)
		params["classid"] = QVariant::Invalid;

	int id = m_client->db()->execInsertQuery("INSERT INTO user (?k?) VALUES (?)", params);

	if (id == -1)
	{
		setServerError();
		return false;
	}

	QVariantMap m;
	m["username"] = username;

	id = m_client->db()->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", m);

	if (id == -1)
	{
		setServerError();
		return false;
	}

	(*jsonResponse)["created"] = id;
	(*jsonResponse)["createdUserName"] = username;

	return true;*/

	(*jsonResponse)["uploaded"] = true;

	return true;
}


/**
 * @brief TeacherMap::getMapList
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	QVariantList params;

	params.append(m_client->clientUserName());

	l = QJsonArray::fromVariantList(m_client->mapsDb()->execSelectQuery("SELECT rowid, uuid, name, version, draft FROM maps WHERE owner=?",
																		params));
	(*jsonResponse)["list"] = l;

	return true;
}






/**
 * @brief TeacherMap::mapCreate
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString name = params.value("name").toString();

	QUuid uuid = QUuid::createUuid();

	GameMap map(uuid.toByteArray());

	QVariantMap m;
	m["uuid"] = uuid.toString();
	m["name"] = name;
	m["version"] = 1;
	m["draft"] = true;
	m["owner"] = m_client->clientUserName();
	m["data"] = map.toBinaryData();

	int id = m_client->mapsDb()->execInsertQuery("INSERT INTO maps (?k?) VALUES (?)", m);

	bool ret = (id != -1);

	(*jsonResponse)["created"] = ret;

	return ret;
}



/**
 * @brief TeacherMap::mapGet
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapGet(QJsonObject *jsonResponse, QByteArray *byteArrayResponse)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();
	int version = params.value("version", -1).toInt();

	QVariantList m;

	m.append(uuid);
	m.append(version);
	m.append(m_client->clientUserName());


	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, version, name, draft, data FROM maps WHERE uuid=? AND version=? AND owner=?",
															  m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	(*jsonResponse)["uuid"] = r.value("uuid").toString();
	(*jsonResponse)["version"] = r.value("version").toInt();
	(*jsonResponse)["name"] = r.value("name").toString();
	(*jsonResponse)["draft"] = r.value("draft").toBool();

	(*byteArrayResponse) = r.value("data").toByteArray();

	return true;
}
