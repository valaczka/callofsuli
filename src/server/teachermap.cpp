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
 * @brief TeacherMap::getMapList
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;

	params.append(m_client->clientUserName());

	QVariantList mapList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name, version, lastModified, md5, COALESCE(LENGTH(data),0) as dataSize "
"FROM maps WHERE owner=?",
															   params);

	QJsonArray retList;

	foreach (QVariant v, mapList) {
		QVariantMap m = v.toMap();
		QVariantList l;
		QString uuid = m.value("uuid").toString();
		l.append(uuid);
		l.append(uuid);
		QVariantMap mm = m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM bindGroupMap WHERE mapid=?) AS binded,"
															   "EXISTS(SELECT * FROM game WHERE mapid=?) AS used", l);

		QJsonObject o = QJsonObject::fromVariantMap(m);
		o["binded"] = mm.value("binded").toBool();
		o["used"] = mm.value("used").toBool();
		retList.append(o);
	}

	(*jsonResponse)["list"] = retList;

	return true;
}








/**
 * @brief TeacherMap::mapUpdate
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();
	QByteArray d = m_message.binaryData();


	if (uuid.isEmpty()) {
		(*jsonResponse)["error"] = "missing uuid";
		return false;
	}

	QVariantList m;

	m.append(uuid);

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT owner FROM maps WHERE uuid=?", m);

	if (!r.isEmpty() && r.value("owner") != m_client->clientUserName()) {
		(*jsonResponse)["error"] = "map exists";
		return false;
	}


	if (r.isEmpty() && d.isEmpty()) {
		(*jsonResponse)["error"] = "insufficient parameters";
		return false;
	}


	QVariantMap u;

	QString md5;

	if (!d.isEmpty()) {
		md5 = QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex());
		(*jsonResponse)["md5"] = md5;
		u["data"] = d;
		u["md5"] = md5;
	}

	if (params.contains("name")) {
		u["name"] = params.value("name").toString();
	}



	if (r.isEmpty()) {
		u["version"] = 1;
		u["uuid"] = uuid;
		u["owner"] = m_client->clientUserName();

		if (m_client->mapsDb()->execInsertQuery("INSERT INTO maps (?k?) VALUES (?)", u) != -1) {
			(*jsonResponse)["uuid"]	= uuid;
			(*jsonResponse)["created"] = true;
			return true;
		}
	} else {
		QVariantMap uu;
		uu[":uuid"] = uuid;
		uu[":owner"] = m_client->clientUserName();

		if (m_client->mapsDb()->execUpdateQuery("UPDATE maps SET version=version+1, lastModified=datetime('now'),? "
												"WHERE uuid=:uuid AND owner=:owner", u, uu)) {
			(*jsonResponse)["uuid"]	= uuid;
			(*jsonResponse)["updated"] = true;
			return true;
		}
	}

	(*jsonResponse)["uuid"]	= uuid;
	(*jsonResponse)["error"] = "sql error";

	return false;
}






/**
 * @brief TeacherMap::mapRemove
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();

	QVariantList m;

	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid FROM maps WHERE uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}


	if (m_client->mapsDb()->execSimpleQuery("DELETE FROM maps WHERE uuid=? AND owner=?", m)) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["removed"] = true;
		return true;
	}

	(*jsonResponse)["error"] = "sql error";
	return false;
}
