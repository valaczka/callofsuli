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

	params.append(m_client->clientSession());
	params.append(m_client->clientUserName());

	QVariantList mapList = m_client->mapsDb()->execSelectQuery("SELECT rowid, uuid, name, version, lastModified, LENGTH(data) as dataSize, "
"(editSession IS NOT NULL AND editSession<>?) AS editLocked "
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

	QVariantList m;

	m.append(m_client->clientSession());
	m.append(uuid);
	m.append(m_client->clientUserName());


	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, version, name, data, "
"(editSession IS NOT NULL AND editSession<>?) AS editLocked "
"FROM maps WHERE uuid=? AND owner=?",
															  m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	QVariantList l;
	l.append(uuid);
	l.append(uuid);
	QVariantMap rr = m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM bindGroupMap WHERE mapid=?) AS binded,"
														   "EXISTS(SELECT * FROM game WHERE mapid=?) AS used", l);


	(*jsonResponse)["uuid"] = r.value("uuid").toString();
	(*jsonResponse)["version"] = r.value("version").toInt();
	(*jsonResponse)["name"] = r.value("name").toString();
	(*jsonResponse)["editLocked"] = r.value("editLocked").toBool();
	(*jsonResponse)["binded"] = rr.value("binded").toBool();
	(*jsonResponse)["used"] = rr.value("used").toBool();


	(*byteArrayResponse) = r.value("data").toByteArray();

	return true;
}


/**
 * @brief TeacherMap::mapEditLock
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapEditLock(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	bool forced = params.value("forced").toBool();
	QString uuid = params.value("uuid").toString();

	QVariantList m;

	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT editSession FROM maps WHERE uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	QString editSession = r.value("editSession").toString();

	if (!editSession.isEmpty() && editSession != m_client->clientSession()) {
		(*jsonResponse)["forced"] = forced;

		if (!forced)
			return false;
	}

	m.prepend(m_client->clientSession());

	if (m_client->mapsDb()->execSimpleQuery("UPDATE maps SET editSession=? WHERE uuid=? AND owner=?", m)) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["locked"] = true;
		return true;
	}

	return false;
}


/**
 * @brief TeacherMap::mapEditUnlock
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapEditUnlock(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();

	QVariantList m;

	m.append(m_client->clientSession());
	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT rowid FROM maps WHERE editSession=? AND uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	QVariantList ll;
	ll.append(r.value("rowid").toInt());

	if (m_client->mapsDb()->execSimpleQuery("UPDATE maps SET editSession=NULL WHERE rowid=?", ll)) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["unlocked"] = true;
		return true;
	}

	return false;
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

	QVariantList m;

	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT editSession FROM maps WHERE uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	QString editSession = r.value("editSession").toString();

	if (!editSession.isEmpty() && editSession != m_client->clientSession()) {
		(*jsonResponse)["error"] = "edit locked";
		return false;
	}

	QVariantMap u;

	QByteArray d = m_message.binaryData();

	u["editSession"] = m_client->clientSession();
	u["data"] = d;
	u["md5"] = QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex();

	QVariantMap uu;
	uu[":uuid"] = uuid;
	uu[":owner"] = m_client->clientUserName();

	if (m_client->mapsDb()->execUpdateQuery("UPDATE maps SET version=version+1,? WHERE uuid=:uuid AND owner=:owner", u, uu)) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["updated"] = true;
		return true;
	}

	return false;
}


/**
 * @brief TeacherMap::mapRename
 * @param jsonResponse
 * @return
 */

bool TeacherMap::mapRename(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();
	QString name = params.value("name").toString();

	QVariantList m;

	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT rowid FROM maps WHERE uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	if (name.isEmpty()) {
		(*jsonResponse)["error"] = "invalid name";
		return false;
	}

	QVariantList ll;
	ll.append(name);
	ll.append(r.value("rowid").toInt());

	if (m_client->mapsDb()->execSimpleQuery("UPDATE maps SET name=? WHERE rowid=?", ll)) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["name"] = name;
		return true;
	}

	return false;
}
