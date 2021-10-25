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

#include "teacher.h"
#include "gamemap.h"
#include "server.h"
#include "admin.h"
#include "userinfo.h"

/**
 * @brief TeacherMap::TeacherMap
 * @param client
 * @param message
 */

Teacher::Teacher(Client *client, const CosMessage &message)
	: AbstractHandler(client, message, CosMessage::ClassTeacher)
{

}


/**
 * @brief TeacherMap::classInit
 * @return
 */

bool Teacher::classInit()
{
	if (!m_client->clientRoles().testFlag(CosMessage::RoleTeacher))
		return false;

	return true;
}




/**
 * @brief Teacher::userGet
 * @param jsonResponse
 * @return
 */

bool Teacher::userGet(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject o = m_message.jsonData();

	if (!o.contains("withTrophy"))
		o["withTrophy"] = true;

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	UserInfo u(m_client, m2);
	return u.getUser(jsonResponse, nullptr);
}



/**
 * @brief Teacher::userModify
 * @param jsonResponse
 * @return
 */

bool Teacher::userModify(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();

	QJsonObject o;

	QStringList p;
	p.append("nickname");
	p.append("character");
	p.append("picture");
	p.append("firstname");
	p.append("lastname");

	foreach (QString s, p) {
		if (params.contains(s))
			o[s] = params.value(s);
	}

	if (o.isEmpty()) {
		(*jsonResponse)["error"] = "missing parameter";
		return false;
	}

	o["username"] = m_client->clientUserName();

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Admin u(m_client, m2);
	return u.userModify(jsonResponse, nullptr);
}


/**
 * @brief Teacher::userPasswordChange
 * @param jsonResponse
 * @return
 */

bool Teacher::userPasswordChange(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();
	QString password = params.value("password").toString();

	if (password.isEmpty()) {
		(*jsonResponse)["error"] = "missing password";
		return false;
	}

	QJsonObject o;

	o["username"] = m_client->clientUserName();
	o["password"] = password;
	o["oldPassword"] = params.value("oldPassword").toString();

	CosMessage m2(o, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Admin u(m_client, m2);
	return u.userPasswordChange(jsonResponse, nullptr);
}



/**
 * @brief Teacher::groupCreate
 * @param jsonResponse
 * @return
 */

bool Teacher::groupCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();

	if (!params.contains("name")) {
		(*jsonResponse)["error"] = "name empty";
		return false;
	}

	params["owner"] = m_client->clientUserName();

	int id = m_client->db()->execInsertQuery("INSERT INTO studentgroup (?k?) VALUES (?)", params);

	if (id == -1)
	{
		setServerError();
		return false;
	}

	(*jsonResponse)["created"] = id;

	return true;
}


/**
 * @brief Teacher::groupRemove
 * @param jsonResponse
 * @return
 */

bool Teacher::groupRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	if (id != -1) {
		QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id FROM studentgroup WHERE id=? AND owner=?",
																  {id, m_client->clientUserName()});
		if (group.isEmpty()) {
			(*jsonResponse)["error"] = "invalid id";
			return false;
		}
	}

	QVariantList list;

	if (id != -1)
		list.append(id);

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toInt());


	if (list.size()) {
		QVariantMap p;
		p[":owner"] = m_client->clientUserName();

		if (!m_client->db()->execListQuery("DELETE FROM studentgroup WHERE owner=:owner AND id IN (?l?)", list, p)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}

		(*jsonResponse)["removed"] = true;

	} else {
		(*jsonResponse)["error"] = "missing id";
		return false;
	}


	return true;
}



/**
 * @brief Teacher::groupUserGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupUserGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();


	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList cList = m_client->db()->execSelectQuery("SELECT classid, name FROM bindGroupClass "
"LEFT JOIN class ON (class.id=bindGroupClass.classid) WHERE groupid=?",
														 {id});

	QVariantList uList = m_client->db()->execSelectQuery("SELECT userInfo.username as username, firstname, lastname, nickname, "
"rankid, ranklevel, rankimage, picture, xp, "
"active, classid, classname "
"FROM bindGroupStudent LEFT JOIN userInfo ON (userInfo.username=bindGroupStudent.username) WHERE groupid=?",
														 {id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["name"] = group.value("name").toString();
	(*jsonResponse)["classList"] = QJsonArray::fromVariantList(cList);
	(*jsonResponse)["userList"] = QJsonArray::fromVariantList(uList);

	return true;
}




/**
 * @brief Teacher::groupUserAdd
 * @param jsonResponse
 * @return
 */

bool Teacher::groupUserAdd(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toString());

	if (params.contains("username"))
		list.append(params.value("username").toString());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("INSERT INTO bindGroupStudent (groupid, username) SELECT :id, T.id FROM "
										   "(SELECT column1 as id FROM (values ?l? )) T", list, p, true)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["added"] = list.size();

	return true;
}



/**
 * @brief Teacher::groupUserRemove
 * @param jsonResponse
 * @return
 */

bool Teacher::groupUserRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toString());

	if (params.contains("username"))
		list.append(params.value("username").toString());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("DELETE FROM bindGroupStudent WHERE groupid=:id AND username IN (?l?)", list, p, false)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["removed"] = list.size();

	return true;
}



/**
 * @brief Teacher::groupExcludedUserListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupExcludedUserListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantList list = m_client->db()->execSelectQuery("SELECT username, firstname, lastname, "
																		"active, classid, classname "
																		"FROM userInfo "
																		"WHERE isTeacher=false AND username NOT IN "
																		"(SELECT username FROM bindGroupStudent WHERE groupid=?) ",
														{id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);

	return true;
}



/**
 * @brief Teacher::groupClassAdd
 * @param jsonResponse
 * @return
 */

bool Teacher::groupClassAdd(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toInt());

	if (params.contains("classid"))
		list.append(params.value("classid", -1).toInt());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("INSERT INTO bindGroupClass (groupid, classid) SELECT :id, T.id FROM "
										   "(SELECT column1 as id FROM (values ?l? )) T", list, p, true)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["added"] = list.size();

	return true;
}




/**
 * @brief Teacher::groupClassRemove
 * @param jsonResponse
 * @return
 */

bool Teacher::groupClassRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toInt());

	if (params.contains("classid"))
		list.append(params.value("classid", -1).toInt());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("DELETE FROM bindGroupClass WHERE groupid=:id AND classid IN (?l?)", list, p, false)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["removed"] = list.size();

	return true;
}



/**
 * @brief Teacher::groupExcludedClassListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupExcludedClassListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantList list = m_client->db()->execSelectQuery("SELECT id, name FROM class WHERE id NOT IN "
														"(SELECT classid FROM bindGroupClass WHERE groupid=?) ",
														{id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);

	return true;
}




/**
 * @brief TeacherMap::getMapList
 * @param jsonResponse
 * @return


bool Teacher::mapListGet(QJsonObject *jsonResponse, QByteArray *)
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
*/







/**
 * @brief TeacherMap::mapUpdate
 * @param jsonResponse
 * @return

bool Teacher::mapUpdate(QJsonObject *jsonResponse, QByteArray *)
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


*/



/**
 * @brief TeacherMap::mapRemove
 * @param jsonResponse
 * @return

bool Teacher::mapRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toString());

	if (params.contains("uuid"))
		list.append(params.value("uuid").toString());


	if (list.size()) {
		QVariantMap p;
		p[":owner"] = m_client->clientUserName();

		if (!m_client->mapsDb()->execListQuery("DELETE FROM maps WHERE owner=:owner AND uuid IN (?l?)", list, p, false)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}

		if (!m_client->db()->execListQuery("DELETE FROM bindGroupMap WHERE id IN "
										   "(SELECT bindGroupMap.id FROM bindGroupMap "
										   "LEFT JOIN studentgroup ON (studentgroup.id=bindGroupMap.groupid) "
										   "WHERE mapid IN (?l?) AND owner=:owner)",
										   list, p, false)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}

		(*jsonResponse)["removed"] = true;
	} else {
		(*jsonResponse)["removed"] = false;
	}

	return true;
}

*/



/**
 * @brief TeacherMap::mapGet
 * @param jsonResponse
 * @return

bool Teacher::mapGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();

	QVariantList m;

	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, name FROM maps WHERE uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}



	QVariantList list = m_client->db()->execSelectQuery("SELECT groupid, name, active, "
"(SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
"WHERE bindGroupClass.groupid=bindGroupMap.groupid) as readableClassList "
"FROM bindGroupMap "
"LEFT JOIN studentgroup ON (studentgroup.id=bindGroupMap.groupid) "
"WHERE mapid=? AND owner=?", m);


	QVariantList l;
	l.append(uuid);
	l.append(uuid);
	QVariantMap mm = m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM bindGroupMap WHERE mapid=?) AS binded,"
														   "EXISTS(SELECT * FROM game WHERE mapid=?) AS used", l);

	(*jsonResponse)["uuid"] = uuid;
	(*jsonResponse)["name"] = r.value("name").toString();
	(*jsonResponse)["groupList"] = QJsonArray::fromVariantList(list);
	(*jsonResponse)["binded"] = mm.value("binded").toBool();
	(*jsonResponse)["used"] = mm.value("used").toBool();

	return true;
}

*/


/**
 * @brief TeacherMap::mapGroupAdd
 * @param jsonResponse
 * @return

bool Teacher::mapGroupAdd(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();

	QVariantList m;

	m.append(uuid);
	m.append(m_client->clientUserName());

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT uuid, name FROM maps WHERE uuid=? AND owner=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("groupList").toList())
		list.append(v.toInt());

	if (params.contains("groupid"))
		list.append(params.value("groupid").toInt());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = uuid;
		p[":owner"] = m_client->clientUserName();

		if (!m_client->db()->execListQuery("INSERT INTO bindGroupMap (mapid, groupid) SELECT :id, T.column1 FROM "
										   "(values ?l?) T LEFT JOIN studentgroup ON (studentgroup.id=T.column1) WHERE owner=:owner",
										   list, p, true)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["uuid"] = uuid;
	(*jsonResponse)["added"] = list.size();

	return true;
}

*/



/**
 * @brief TeacherMap::mapGroupRemove
 * @param jsonResponse
 * @return

bool Teacher::mapGroupRemove(QJsonObject *jsonResponse, QByteArray *)
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

	QVariantList list;

	foreach (QVariant v, params.value("groupList").toList())
		list.append(v.toInt());

	if (params.contains("groupid"))
		list.append(params.value("groupid").toInt());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = uuid;

		if (!m_client->db()->execListQuery("DELETE FROM bindGroupMap WHERE id IN "
										   "(SELECT bindGroupMap.id FROM bindGroupMap "
										   "LEFT JOIN studentgroup ON (studentgroup.id=bindGroupMap.groupid) "
										   "WHERE groupid IN (?l?) AND mapid=:id)",
										   list, p, false)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["uuid"] = uuid;
	(*jsonResponse)["removed"] = true;

	return true;
}

*/


/**
 * @brief TeacherMap::mapExcludedGroupListGet
 * @param jsonResponse
 * @return

bool Teacher::mapExcludedGroupListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();

	QVariantList l;
	l.append(m_client->clientUserName());
	l.append(uuid);

	QVariantList list = m_client->db()->execSelectQuery("SELECT id, name, (SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
"WHERE bindGroupClass.groupid=studentgroup.id) as readableClassList FROM studentgroup "
"WHERE owner=? AND id NOT IN (SELECT groupid FROM bindGroupMap WHERE mapid=?) ",
														l);

	(*jsonResponse)["uuid"] = uuid;
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);

	return true;
}

*/


/**
 * @brief TeacherMap::groupListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;

	params.append(m_client->clientUserName());

	QVariantList mapList = m_client->db()->execSelectQuery("SELECT id, name, (SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
"WHERE bindGroupClass.groupid=studentgroup.id) as readableClassList FROM studentgroup "
"WHERE owner=?",
														   params);


	(*jsonResponse)["list"] = QJsonArray::fromVariantList(mapList);

	return true;
}





/**
 * @brief TeacherMap::groupGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();


	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}



	QVariantList mapList = m_client->db()->execSelectQuery("SELECT mapid, active FROM bindGroupMap WHERE groupid=?", {id});

	QVariantList cList = m_client->db()->execSelectQuery("SELECT classid, name FROM bindGroupClass "
"LEFT JOIN class ON (class.id=bindGroupClass.classid) WHERE groupid=?",
														 {id});

	QVariantList uList = m_client->db()->execSelectQuery("SELECT userInfo.username as username, firstname, lastname, nickname, "
"rankid, ranklevel, rankimage, picture, xp, "
"active, classid, classname "
"FROM studentGroupInfo LEFT JOIN userInfo ON (userInfo.username=studentGroupInfo.username) WHERE id=?",
														 {id});



	QVariantList mapDataList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name, md5, "
																"COALESCE(LENGTH(data),0) as dataSize FROM maps WHERE owner=?",
																   {m_client->clientUserName()});

	QJsonArray mList;

	foreach (QVariant v, mapList) {
		QVariantMap lMap = v.toMap();

		QJsonObject o;
		o["uuid"] = lMap.value("mapid").toString();
		o["name"] = lMap.value("mapid").toString();
		o["active"] = lMap.value("active").toBool();

		foreach (QVariant vv, mapDataList) {
			QVariantMap mMap = vv.toMap();
			if (mMap.value("uuid").toString() == lMap.value("mapid").toString()) {
				o["name"] = mMap.value("name").toString();
				o["md5"] = mMap.value("md5").toString();
				o["dataSize"] = mMap.value("dataSize").toInt();
				break;
			}
		}

		mList.append(o);
	}



	QJsonArray userList;

	foreach (QVariant v, uList) {
		QVariantMap m = v.toMap();
		m["activeClient"] = m_client->server()->isClientActive(m.value("username").toString());
		QJsonObject o = QJsonObject::fromVariantMap(m);
		userList.append(o);
	}


	(*jsonResponse)["id"] = id;
	(*jsonResponse)["name"] = group.value("name").toString();
	(*jsonResponse)["classList"] = QJsonArray::fromVariantList(cList);
	(*jsonResponse)["userList"] = userList;
	(*jsonResponse)["mapList"] = mList;

	return true;
}


/**
 * @brief Teacher::groupMapAdd
 * @param jsonResponse
 * @return
 */

bool Teacher::groupMapAdd(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toString());

	if (params.contains("uuid"))
		list.append(params.value("uuid").toString());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("INSERT INTO bindGroupMap (groupid, mapid) SELECT :id, T.id FROM "
										   "(SELECT column1 as id FROM (values ?l? )) T", list, p, true)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["added"] = true;

	return true;
}



/**
 * @brief Teacher::groupMapActivate
 * @param jsonResponse
 * @return
 */

bool Teacher::groupMapActivate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();
	bool active = params.value("active", false).toBool();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toString());

	if (params.contains("uuid"))
		list.append(params.value("uuid").toString());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;
		p[":active"] = active;

		if (!m_client->db()->execListQuery("UPDATE bindGroupMap SET active=:active WHERE groupid=:id AND mapid IN (?l?)", list, p)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["updated"] = true;

	return true;
}



/**
 * @brief Teacher::groupMapRemove
 * @param jsonResponse
 * @return
 */

bool Teacher::groupMapRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toString());

	if (params.contains("uuid"))
		list.append(params.value("uuid").toString());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("DELETE FROM bindGroupMap WHERE groupid=:id AND mapid IN (?l?)", list, p)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["removed"] = true;

	return true;
}



/**
 * @brief Teacher::groupExcludedMapListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupExcludedMapListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantList mapList = m_client->db()->execSelectQuery("SELECT mapid FROM bindGroupMap WHERE groupid=?", {id});

	QStringList ml;
	foreach (QVariant v, mapList)
		ml.append(v.toMap().value("mapid").toString());

	QVariantList list = m_client->mapsDb()->execSelectQuery("SELECT uuid, name FROM maps WHERE owner=? ", {m_client->clientUserName()});

	QJsonArray r;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		if (ml.contains(m.value("uuid").toString()))
			continue;

		r.append(QJsonObject::fromVariantMap(m));
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["list"] = r;

	return true;
}



/**
 * @brief Teacher::groupTrophyGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupTrophyGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?",
															  {id, m_client->clientUserName()});
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList queryParams;

	QString query = "SELECT username, mapid, missionid, level, deathmatch, success, num FROM missionTrophy "
"WHERE username IN (SELECT username FROM studentGroupInfo where id=?)";
	queryParams.append(id);

	if (params.contains("map")) {
		query += " AND mapid=?";
		queryParams.append(params.value("map").toString());
	}

	if (params.contains("mission")) {
		query += " AND missionid=?";
		queryParams.append(params.value("mission").toString());
	}

	QVariantList tList = m_client->db()->execSelectQuery(query, queryParams);

	(*jsonResponse)["list"] = QJsonArray::fromVariantList(tList);



	if (params.value("withUsers", false).toBool()) {
		QVariantList queryParams;
		QString query = "SELECT userInfo.username as username, firstname, lastname, nickname, "
			"rankid, ranklevel, rankimage, picture, xp, "
			"active, classid, classname ";

		if (params.value("withCurrentGame", false).toBool()) {
			query += ", mapid, missionid, level, deathmatch ";
		}

		query += " FROM studentGroupInfo LEFT JOIN userInfo ON (userInfo.username=studentGroupInfo.username) ";

		if (params.value("withCurrentGame", false).toBool()) {
			query += " LEFT JOIN (SELECT username, mapid, missionid, level, deathmatch FROM game WHERE tmpScore IS NOT NULL GROUP BY username) g "
"ON (g.username = userInfo.username)";
		}

		query += " WHERE id=?";
		queryParams.append(id);

		QVariantList uList = m_client->db()->execSelectQuery(query, queryParams);
		(*jsonResponse)["users"] = QJsonArray::fromVariantList(uList);
	}

	return true;
}





/**
 * @brief TeacherMap::groupMapRemove
 * @param jsonResponse
 * @return

bool Teacher::groupMapRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int id = params.value("id", -1).toInt();

	QVariantList m;

	m.append(id);
	m.append(m_client->clientUserName());

	QVariantMap group = m_client->db()->execSelectQueryOneRow("SELECT id, name FROM studentgroup WHERE id=? AND owner=?", m);
	if (group.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	QVariantList list;

	foreach (QVariant v, params.value("mapList").toList())
		list.append(v.toString());

	if (params.contains("uuid"))
		list.append(params.value("uuid").toString());

	if (list.size()) {
		QVariantMap p;
		p[":id"] = id;

		if (!m_client->db()->execListQuery("DELETE FROM bindGroupMap WHERE groupid=:id AND mapid IN "
										   "(SELECT column1 FROM (values ?l? ))", list, p, true)) {
			(*jsonResponse)["error"] = "sql error";
			return false;
		}
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["removed"] = true;

	return true;
}

*/
