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
 * @brief Teacher::gradingFromVariantList
 * @param list
 * @return
 */

QVector<Teacher::Grading> Teacher::gradingFromVariantList(const QVariantList &list)
{
	QVector<Teacher::Grading> ret;

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		QByteArray cData = m.value("criteria").toString().toUtf8();
		QJsonDocument doc = QJsonDocument::fromJson(cData);
		QJsonObject criteriaObject;

		if (!doc.isNull() && doc.isObject())
			criteriaObject = doc.object();

		if (!m.value("gradeid").isNull()) {
			int ref = m.value("gradeid").toInt();
			Grading g(Grading::TypeGrade, m.value("value", 0).toInt(), criteriaObject, ref);
			ret.append(g);
		} else if (!m.value("xp").isNull()) {
			Grading g(Grading::TypeXP, m.value("xp", 0).toInt(), criteriaObject);
			ret.append(g);
		}
	}

	return ret;
}





/**
 * @brief Teacher::gradingResultOneType
 * @param list
 * @param dest
 * @return
 */

Teacher::Grading Teacher::gradingResult(const QVector<Grading> &list, const Grading::Type &type)
{
	Grading ret;

	// Search default grade

	for (int i=0; i<list.size(); ++i) {
		Grading g = list.at(i);
		if (g.mode == Grading::ModeDefault && g.type == type)
			ret = g;
	}

	// Check required criteria

	bool requiredSuccess = true;

	for (int i=0; i<list.size(); ++i) {
		Grading g = list.at(i);
		if (g.mode == Grading::ModeRequired && !g.success) {
			requiredSuccess = false;
		}
	}

	if (!requiredSuccess)
		return ret;

	// Check over values

	QMap<int, QVector<Grading>> map = Grading::toMap(list, type);

	int maxValue = -1;

	QMapIterator<int, QVector<Grading>> it(map);

	while (it.hasNext()) {
		it.next();

		const QVector<Grading> &list = it.value();

		bool allSuccess = true;

		foreach (const Grading &g, list) {
			if (!g.success)
				allSuccess = false;
		}

		if (allSuccess && !list.isEmpty()) {
			const Grading &g = list.at(0);

			if (g.value>maxValue) {
				ret = g;
				maxValue = g.value;
			}
		}
	}

	return ret;
}



/**
 * @brief Teacher::gradingGet
 * @param assignmentId
 * @param campaignId
 * @param username
 * @return
 */

QVector<Teacher::Grading> Teacher::gradingGet(const int &assignmentId, const int &campaignId, const QString &username)
{
	QVariantList gradingList = m_client->db()->execSelectQuery("SELECT grading.id, gradeid, value, xp, criteria "
															   "FROM grading LEFT JOIN grade ON (grade.id=grading.gradeid) "
															   "WHERE assignmentid=? ORDER BY grading.id", {assignmentId});


	QVector<Grading> grading = gradingFromVariantList(gradingList);

	evaluate(grading, campaignId, username);

	return grading;
}





/**
 * @brief Teacher::evaluate
 * @param list
 * @param campaignId
 * @param username
 * @return
 */

QVector<Teacher::Grading> &Teacher::evaluate(QVector<Grading> &list, const int &campaignId, const QString &username)
{
	for (int i=0; i<list.size(); ++i) {
		Grading &g = list[i];
		if (!g.success)
			evaluate(g, campaignId, username);
	}

	return list;
}







/**
 * @brief Teacher::evaluate
 * @param grading
 * @param campaignId
 * @param username
 * @return
 */

Teacher::Grading &Teacher::evaluate(Grading &grading, const int &campaignId, const QString &username)
{
	const QJsonObject criteria = grading.criteria;
	const QString module = criteria.value("module").toString();

	if (grading.mode == Grading::ModeDefault) {
		grading.success = true;
		return grading;
	}

	if (module == "xp") {
		int targetXP = criteria.value("value").toInt(0);
		if (targetXP < 1) {
			grading.success = true;
			return grading;
		}

		QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT SUM(xp) as xp FROM game LEFT JOIN score ON (score.gameid=game.id) "
															  "WHERE game.username=? AND "
															  "game.timestamp>=(SELECT starttime FROM campaign WHERE id=?) AND "
															  "game.timestamp<(SELECT endtime FROM campaign WHERE id=?) AND "
															  "game.mapid IN (SELECT mapid FROM bindGroupMap "
															  "WHERE groupid=(SELECT groupid FROM campaign WHERE id=?))",
															  {username, campaignId, campaignId, campaignId});

		int xp = m.value("xp", 0).toInt();

		if (xp>=targetXP)
			grading.success = true;
	} else if (module == "trophy") {
		int target = criteria.value("value").toInt(0);
		if (target < 1) {
			grading.success = true;
			return grading;
		}

		QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT COUNT(*) as trophy FROM game "
															  "WHERE game.username=? AND success=true AND "
															  "game.timestamp>=(SELECT starttime FROM campaign WHERE id=?) AND "
															  "game.timestamp<(SELECT endtime FROM campaign WHERE id=?) AND "
															  "game.mapid IN (SELECT mapid FROM bindGroupMap "
															  "WHERE groupid=(SELECT groupid FROM campaign WHERE id=?))",
															  {username, campaignId, campaignId, campaignId});

		int trophy = m.value("trophy", 0).toInt();

		if (trophy >= target)
			grading.success = true;
	} else {
		qWarning() << "Invalid grading module" << module;
	}

	return grading;
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

	bool oauth2Account = m_client->db()->execSelectQueryOneRow("SELECT (password='*') as v FROM auth WHERE username=?", {m_client->clientUserName()})
						 .value("v", false).toBool();

	QJsonObject o;

	QStringList p;
	p.append("nickname");
	p.append("character");

	if (!oauth2Account) {
		p.append("picture");
		p.append("firstname");
		p.append("lastname");
	}

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

	if (m_client->db()->execSelectQueryOneRow("SELECT (password='*') as v FROM auth WHERE username=?", {m_client->clientUserName()})
		.value("v", false).toBool()) {
		(*jsonResponse)["error"] = "oauth2 account";
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
 * @brief Teacher::groupModify
 * @param jsonResponse
 * @return
 */

bool Teacher::groupModify(QJsonObject *jsonResponse, QByteArray *)
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

	params.remove("id");
	params.remove("owner");

	if (!m_client->db()->execUpdateQuery("UPDATE studentGroup SET ? WHERE id=:id", params, {{":id", id}})) {
		(*jsonResponse)["error"] = "sql error";
		return false;
	}

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["modified"] = true;
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
														 "rankid, COALESCE(ranklevel, -1) as ranklevel, rankimage, picture, xp, "
														 "active, COALESCE(classid, -1) as classid, classname "
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
														"active, COALESCE(classid, -1) as classid, classname "
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

	QVariantList list = m_client->db()->execSelectQuery("SELECT id as classid, name FROM class WHERE id NOT IN "
														"(SELECT classid FROM bindGroupClass WHERE groupid=?) ",
														{id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);

	return true;
}










/**
 * @brief TeacherMap::groupListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList mapList = m_client->db()->execSelectQuery("SELECT id, name, (SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
														   "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
														   "WHERE bindGroupClass.groupid=studentgroup.id) as readableClassList FROM studentgroup "
														   "WHERE owner=?",
														   {m_client->clientUserName()});


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
														 "rankid, COALESCE(ranklevel, -1) as ranklevel, rankimage, picture, xp, "
														 "active, COALESCE(classid, -1) as classid, classname, "
														 "t1, t2, t3, d1, d2, d3, sumxp "
														 "FROM studentGroupInfo LEFT JOIN userInfo ON (userInfo.username=studentGroupInfo.username) "
														 "LEFT JOIN groupTrophy ON (groupTrophy.username=studentGroupInfo.username "
														 "AND groupTrophy.id=studentGroupInfo.id) "
														 "WHERE studentGroupInfo.id=?",
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
						"rankid, COALESCE(ranklevel, -1) as ranklevel, rankimage, picture, xp, "
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
 * @brief Teacher::mapListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::mapListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList mapList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name, version, datetime(lastModified, 'localtime') as lastModified, "
															   "md5, COALESCE(LENGTH(data),0) as dataSize "
															   "FROM maps WHERE owner=?",
															   {m_client->clientUserName()});

	QJsonArray retList;

	foreach (QVariant v, mapList) {
		QVariantMap m = v.toMap();
		QString uuid = m.value("uuid").toString();

		QJsonObject o = QJsonObject::fromVariantMap(m);
		o["used"] = m_client->db()->execSelectQueryOneRow("SELECT COALESCE(COUNT(*),0) AS used FROM game WHERE mapid=:a",
														  {uuid}).value("used").toInt();

		o["binded"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT groupid, name, active, "
																				  "(SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
																				  "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
																				  "WHERE bindGroupClass.groupid=bindGroupMap.groupid) as readableClassList "
																				  "FROM bindGroupMap "
																				  "LEFT JOIN studentgroup ON (studentgroup.id=bindGroupMap.groupid) "
																				  "WHERE mapid=? AND owner=?", {uuid, m_client->clientUserName()}));
		retList.append(o);
	}

	(*jsonResponse)["list"] = retList;

	return true;
}


/**
 * @brief Teacher::mapAdd
 * @param jsonResponse
 * @return
 */

bool Teacher::mapAdd(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QByteArray d = m_message.binaryData();


	if (d.isEmpty()) {
		(*jsonResponse)["error"] = "missing map data";
		return false;
	}

	QVariantMap u;

	GameMap *map = GameMap::fromBinaryData(d);
	QString uuid = map ? QString(map->uuid()) : "";
	if (map)
		delete map;

	if (uuid.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map data";
		return false;
	}

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT * FROM maps WHERE uuid=?", { uuid });

	if (!r.isEmpty()) {
		(*jsonResponse)["error"] = "map exists";
		return false;
	}


	QString md5 = QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex());
	u["data"] = d;
	u["md5"] = md5;

	if (params.contains("name"))
		u["name"] = params.value("name").toString();

	u["version"] = 1;
	u["uuid"] = uuid;
	u["owner"] = m_client->clientUserName();

	if (m_client->mapsDb()->execInsertQuery("INSERT INTO maps (?k?) VALUES (?)", u) != -1) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["md5"] = md5;
		(*jsonResponse)["created"] = true;
		return true;
	}

	(*jsonResponse)["uuid"]	= uuid;
	(*jsonResponse)["error"] = "sql error";

	return false;
}


/**
 * @brief Teacher::mapRemove
 * @param jsonResponse
 * @return
 */

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




/**
 * @brief Teacher::mapModify
 * @param jsonResponse
 * @return
 */

bool Teacher::mapModify(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString uuid = params.value("uuid").toString();
	QByteArray d = m_message.binaryData();


	if (uuid.isEmpty()) {
		(*jsonResponse)["error"] = "missing uuid";
		return false;
	}


	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT owner FROM maps WHERE uuid=?", { uuid });

	if (!r.isEmpty() && r.value("owner") != m_client->clientUserName()) {
		(*jsonResponse)["error"] = "owner mismatch";
		return false;
	}


	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid uuid";
		return false;
	}


	QVariantMap u;

	if (!d.isEmpty()) {
		GameMap *map = GameMap::fromBinaryData(d);
		QString newUuid = map ? QString(map->uuid()) : "";
		if (map)
			delete map;

		if (newUuid.isEmpty() || newUuid != uuid) {
			(*jsonResponse)["error"] = "invalid map data";
			return false;
		}

		QString md5 = QString(QCryptographicHash::hash(d, QCryptographicHash::Md5).toHex());
		(*jsonResponse)["md5"] = md5;
		u["data"] = d;
		u["md5"] = md5;
	}


	if (params.contains("name")) {
		u["name"] = params.value("name").toString();
	}


	if (u.isEmpty()) {
		(*jsonResponse)["error"] = "missing parameter";
		return false;
	}

	QString query = "UPDATE maps SET ";

	if (u.contains("data"))
		query += "version=version+1, lastModified=datetime('now'),";

	query += "? WHERE uuid=:uuid AND owner=:owner";

	if (m_client->mapsDb()->execUpdateQuery(query, u, {	{":uuid", uuid}, {":owner", m_client->clientUserName() } })) {
		(*jsonResponse)["uuid"]	= uuid;
		(*jsonResponse)["updated"] = true;
		return true;
	}

	(*jsonResponse)["uuid"]	= uuid;
	(*jsonResponse)["error"] = "sql error";

	return false;
}


/**
 * @brief Teacher::studentGameListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::gameListUserGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString username = params.value("username").toString();

	if (username.isEmpty()) {
		(*jsonResponse)["error"] = "missing username";
		return false;
	}

	int limit = params.value("limit", 50).toInt();
	int offset = params.value("offset", 0).toInt();


	QVariantList list;

	if (params.contains("groupid")) {
		int groupid = params.value("groupid", -1).toInt();
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, "
											   "level, success, deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND game.username=? "
											   "AND mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=?)) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {username, groupid, offset, limit});
		(*jsonResponse)["groupid"] = groupid;

	} else if (params.contains("myGroups")) {
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, "
											   "level, success, deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND game.username=? "
											   "AND mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid IN (SELECT id FROM studentGroup WHERE owner=?))) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {username, m_client->clientUserName(), offset, limit});

	} else if (params.contains("allGroups")) {
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, "
											   "level, success, deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND game.username=?) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {username, offset, limit});
	}


	QVariantList mapDataList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name FROM maps");

	QHash<QString, QString> mapHash;

	foreach (QVariant v, mapDataList) {
		QVariantMap m = v.toMap();
		mapHash.insert(m.value("uuid").toString(), m.value("name").toString());
	}

	QJsonArray ret;

	foreach (QVariant v, list) {
		QJsonObject m = v.toJsonObject();
		m["mapname"] = mapHash.value(m.value("mapid").toString());
		ret.append(m);
	}


	(*jsonResponse)["offset"] = offset;
	(*jsonResponse)["list"] = ret;
	(*jsonResponse)["username"] = username;

	return true;
}




/**
 * @brief Teacher::gameListGroupGet
 * @param jsonResponse
 * @return
 */

bool Teacher::gameListGroupGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int groupid = params.value("groupid", -1).toInt();

	if (groupid == -1) {
		(*jsonResponse)["error"] = "missing groupid";
		return false;
	}

	int limit = params.value("limit", 50).toInt();
	int offset = params.value("offset", 0).toInt();

	QVariantList list;

	if (params.contains("username")) {
		QString username = params.value("username").toString();
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, "
											   "level, success, deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND game.username=? "
											   "AND mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=?)) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {username, groupid, offset, limit});
		(*jsonResponse)["username"] = username;

	} else {
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, game.username, firstname, lastname, nickname, "
											   "mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, level, success, "
											   "deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN userInfo ON (userInfo.username=game.username) "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=?)) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {groupid, offset, limit});
	}


	QVariantList mapDataList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name FROM maps");

	QHash<QString, QString> mapHash;

	foreach (QVariant v, mapDataList) {
		QVariantMap m = v.toMap();
		mapHash.insert(m.value("uuid").toString(), m.value("name").toString());
	}

	QJsonArray ret;

	foreach (QVariant v, list) {
		QJsonObject m = v.toJsonObject();
		m["mapname"] = mapHash.value(m.value("mapid").toString());
		ret.append(m);
	}


	(*jsonResponse)["offset"] = offset;
	(*jsonResponse)["list"] = ret;
	(*jsonResponse)["groupid"] = groupid;

	return true;
}



/**
 * @brief Teacher::gameListMapGet
 * @param jsonResponse
 * @return
 */

bool Teacher::gameListMapGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString mapid = params.value("mapid").toString();

	qWarning() << "TODO: OFFSET IMPLEMENTATION REQUIRED";

	if (mapid.isEmpty()) {
		(*jsonResponse)["error"] = "missing mapid";
		return false;
	}

	QVariantList list;

	if (params.contains("username")) {
		QString username = params.value("username").toString();
		list = m_client->db()->execSelectQuery("SELECT missionid, datetime(game.timestamp, 'localtime') as timestamp, "
											   "level, success, deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND game.username=? "
											   "AND mapid=?",
											   {username, mapid});
		(*jsonResponse)["username"] = username;

	} else {
		list = m_client->db()->execSelectQuery("SELECT game.username, firstname, lastname, nickname, "
											   "missionid, datetime(game.timestamp, 'localtime') as timestamp, level, success, "
											   "deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN userInfo ON (userInfo.username=game.username) "
											   "LEFT JOIN score ON (score.gameid=game.id) WHERE tmpScore is NULL "
											   "AND mapid=?",
											   {mapid});
	}


	/*QVariantList mapDataList = m_client->mapsDb()->execSelectQuery("SELECT uuid, name FROM maps");

	QHash<QString, QString> mapHash;

	foreach (QVariant v, mapDataList) {
		QVariantMap m = v.toMap();
		mapHash.insert(m.value("uuid").toString(), m.value("name").toString());
	}

	QJsonArray ret;

	foreach (QVariant v, list) {
		QJsonObject m = v.toJsonObject();
		m["mapname"] = mapHash.value(m.value("mapid").toString());
		ret.append(m);
	}*/


	(*jsonResponse)["list"] = QJsonArray::fromVariantList(list);
	(*jsonResponse)["mapid"] = mapid;

	return true;
}





/**
 * @brief Teacher::Grading::Grading
 * @param t
 * @param v
 * @param c
 * @param r
 */

Teacher::Grading::Grading(const Type &t, const int &v, const QJsonObject &c, const int &r) :
	type(t), value(v), ref(r), mode(ModeInvalid), criteria(c), success(false)
{
	QString m = c.value("mode").toString();
	if (m == "default")
		mode = ModeDefault;
	else if (m == "required")
		mode = ModeRequired;
	else
		mode = ModeInvalid;
}





/**
 * @brief Teacher::Grading::toArray
 * @param list
 * @return
 */

QJsonArray Teacher::Grading::toArray(const QVector<Grading> &list)
{
	QJsonArray ret;

	foreach (Grading g, list) {
		if (!g.isValid())
			continue;

		QJsonObject o;

		o["type"] = g.type == Grading::TypeXP ? "xp" : "grade";
		o["value"] = g.value;
		o["ref"] = g.ref;
		/*if (g.mode == Grading::ModeDefault)
			o["mode"] = "default";
		else if (g.mode == Grading::ModeRequired)
			o["mode"] = "required";
		else
			o["mode"] = "";*/
		o["criteria"] = g.criteria;
		o["success"] = g.success;

		ret.append(o);
	}

	return ret;
}



/**
 * @brief Teacher::Grading::toNestedArray
 * @param list
 * @return
 */

QJsonObject Teacher::Grading::toNestedArray(const QVector<Grading> &list)
{
	QJsonObject ret;
	QMap<int, QVector<Grading>> gradeMap = toMap(list, Grading::TypeGrade);
	QMap<int, QVector<Grading>> xpMap = toMap(list, Grading::TypeXP);

	QJsonArray gradeArray;
	QJsonArray xpArray;

	QMapIterator<int, QVector<Grading>> it(gradeMap);
	while (it.hasNext()) {
		it.next();

		const QVector<Grading> &list = it.value();

		QJsonArray cList;

		foreach (const Grading &g, list) {
			cList.append(QJsonObject({
										 { "data", g.criteria },
										 { "success", g.success }
									 }));
		}

		QJsonObject o;

		if (!list.isEmpty()) {
			const Grading &g = list.at(0);
			o["ref"] = g.ref;
			o["value"] = g.value;
		} else {
			o["ref"] = it.key();
		}

		o["criteria"] = cList;

		gradeArray.append(o);
	}


	QMapIterator<int, QVector<Grading>> it2(xpMap);
	while (it2.hasNext()) {
		it2.next();

		const QVector<Grading> &list = it2.value();

		QJsonArray cList;

		foreach (const Grading &g, list) {
			cList.append(QJsonObject({
										 { "data", g.criteria },
										 { "success", g.success }
									 }));
		}

		QJsonObject o;

		o["value"] = it2.key();
		o["criteria"] = cList;

		xpArray.append(o);
	}

	ret["xp"] = xpArray;
	ret["grade"] = gradeArray;

	//QJsonDocument doc(ret);
	//qDebug() << doc.toJson(QJsonDocument::Indented).data();

	return ret;
}



/**
 * @brief Teacher::Grading::toMap
 * @param list
 * @param type
 * @return
 */

QMap<int, QVector<Teacher::Grading>> Teacher::Grading::toMap(const QVector<Grading> &list, const Type &type)
{
	QMap<int, QVector<Grading>> map;

	foreach (Grading g, list) {
		if (!g.isValid())
			continue;

		if (type == Grading::TypeGrade && g.type == Grading::TypeGrade) {
			map[g.ref].append(g);
		} else if (type == Grading::TypeXP && g.type == Grading::TypeXP) {
			map[g.value].append(g);
		}
	}

	return map;
}
