/*
 * ---- Call of Suli ----
 *
 * teachergroups.cpp
 *
 * Created on: 2020. 05. 24.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherGroups
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

#include "teachergroups.h"


TeacherGroups::TeacherGroups(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: AbstractHandler(client, object, binaryData)
{

}


/**
 * @brief TeacherGroups::classInit
 * @return
 */

bool TeacherGroups::classInit()
{
	if (!m_client->clientRoles().testFlag(Client::RoleTeacher))
		return false;

	return true;
}



/**
 * @brief TeacherGroups::getAllGroup
 * @param jsonResponse
 */

void TeacherGroups::getAllGroup(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;
	params << m_client->clientUserName();

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT id, name FROM studentgroup WHERE owner=? ORDER BY name",
									params,
									&l);
	(*jsonResponse)["list"] = l;
}


/**
 * @brief TeacherGroups::getGroup
 * @param jsonResponse
 */

void TeacherGroups::getGroup(QJsonObject *jsonResponse, QByteArray *)
{
	int refid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << refid;
	params << m_client->clientUserName();

	if (!m_client->db()->execSelectQueryOneRow("SELECT name FROM studentgroup WHERE id=? AND owner=?", params, jsonResponse)) {
		(*jsonResponse)["error"] = "internal db error";
		return;
	}

	if (jsonResponse->isEmpty()) {
		(*jsonResponse)["error"] = "group not found";
		return;
	}


	QVariantList l;
	l << refid;

	QJsonArray users;

	m_client->db()->execSelectQuery("SELECT bindGroupStudent.username as username, firstname, lastname, isTeacher, active, classid, class.name as classname "
									"FROM bindGroupStudent "
									"LEFT JOIN user ON (user.username=bindGroupStudent.username) "
									"LEFT JOIN class ON (class.id=user.classid) "
									"WHERE groupid=? ORDER BY firstname, lastname", l, &users);

	(*jsonResponse)["users"] = users;



	QJsonArray classes;

	m_client->db()->execSelectQuery("SELECT id, name "
									"FROM bindGroupClass "
									"LEFT JOIN class ON (class.id=bindGroupClass.classid) "
									"WHERE groupid=? ORDER BY name", l, &classes);

	(*jsonResponse)["classes"] = classes;


	QJsonArray members;

	m_client->db()->execSelectQuery("SELECT studentGroupInfo.name as groupName, studentGroupInfo.username as username, firstname, lastname, "
									"isTeacher, class.id as classid, class.name as classname "
									"FROM studentGroupInfo "
									"LEFT JOIN user ON (studentGroupInfo.username=user.username) "
									"LEFT JOIN class ON (class.id=user.classid) "
									"WHERE studentGroupInfo.id=? AND studentGroupInfo.owner=? ORDER BY firstname, lastname",
									params,
									&members);

	(*jsonResponse)["members"] = members;



	QJsonArray maps;

	m_client->db()->execSelectQuery("SELECT id, name, version "
									"FROM bindGroupMap "
									"LEFT JOIN map ON (map.id=bindGroupMap.mapid) "
									"WHERE groupid=? ORDER BY name", l, &maps);

	(*jsonResponse)["maps"] = maps;

}



/**
 * @brief TeacherGroups::createGroup
 * @param jsonResponse
 */

void TeacherGroups::createGroup(QJsonObject *jsonResponse, QByteArray *)
{
	QString name = m_jsonData.value("name").toString();

	QVariantMap params;
	params["name"] = name;
	params["owner"] = m_client->clientUserName();
	int id = m_client->db()->execInsertQuery("INSERT INTO studentgroup (?k?) VALUES (?)", params);

	if (id == -1) {
		(*jsonResponse)["error"] = "map create error";
		return;
	}

	(*jsonResponse)["created"] = id;
}



/**
 * @brief TeacherGroups::updateGroup
 * @param jsonResponse
 */

void TeacherGroups::updateGroup(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt();

	QVariantMap params;

	m_client->db()->db().transaction();

	if (m_jsonData.contains("name"))
		params["name"] = m_jsonData.value("name").toString();

	if (!params.isEmpty()) {
		QVariantMap binds;
		binds[":id"] = groupid;

		if (!m_client->db()->execUpdateQuery("UPDATE studentgroup set ? where id=:id", params, binds)) {
			(*jsonResponse)["error"] = "update error";
			m_client->db()->db().rollback();
			return;
		}
	}


	if (m_jsonData.contains("users")) {
		QJsonArray userlist = m_jsonData.value("users").toArray();

		QVariantMap p1;
		p1[":id"] = groupid;

		QVariantList ul;

		foreach (QJsonValue v, userlist) {
			ul << v.toString();
		}

		if (userlist.isEmpty()) {
			QVariantList p;
			p << groupid;
			if (!m_client->db()->execSimpleQuery("DELETE FROM bindGroupStudent WHERE groupid=?", p)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}
		} else {
			if (!m_client->db()->execListQuery("DELETE FROM bindGroupStudent WHERE groupid=:id AND username NOT IN (?l?)", ul, p1)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}



			p1[":id2"] = groupid;

			if (!m_client->db()->execListQuery("INSERT INTO bindGroupStudent(groupid, username) "
											   "SELECT :id as groupid, T.username as username "
											   "FROM (SELECT column1 as username FROM (values ?l? ) EXCEPT SELECT username from bindGroupStudent WHERE groupid=:id2) T",
											   ul, p1, true)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}
		}
	}



	if (m_jsonData.contains("classes")) {
		QJsonArray classlist = m_jsonData.value("classes").toArray();

		QVariantMap p1;
		p1[":id"] = groupid;

		QVariantList ul;

		foreach (QJsonValue v, classlist) {
			ul << v.toInt();
		}

		if (classlist.isEmpty()) {
			QVariantList p;
			p << groupid;
			if (!m_client->db()->execSimpleQuery("DELETE FROM bindGroupClass WHERE groupid=?", p)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}
		} else {
			if (!m_client->db()->execListQuery("DELETE FROM bindGroupClass WHERE groupid=:id AND classid NOT IN (?l?)", ul, p1)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}


			p1[":id2"] = groupid;

			if (!m_client->db()->execListQuery("INSERT INTO bindGroupClass(groupid, classid) "
											   "SELECT :id as groupid, T.classid as classid "
											   "FROM (SELECT column1 as classid FROM (values ?l? ) EXCEPT SELECT classid from bindGroupClass WHERE groupid=:id2) T",
											   ul, p1, true)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}
		}
	}




	if (m_jsonData.contains("maps")) {
		QJsonArray maplist = m_jsonData.value("maps").toArray();

		QVariantMap p1;
		p1[":id"] = groupid;

		QVariantList ul;

		foreach (QJsonValue v, maplist) {
			ul << v.toInt();
		}

		if (maplist.isEmpty()) {
			QVariantList p;
			p << groupid;
			if (!m_client->db()->execSimpleQuery("DELETE FROM bindGroupMap WHERE groupid=?", p)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}
		} else {
			if (!m_client->db()->execListQuery("DELETE FROM bindGroupMap WHERE groupid=:id AND mapid NOT IN (?l?)", ul, p1)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}


			p1[":id2"] = groupid;

			if (!m_client->db()->execListQuery("INSERT INTO bindGroupMap(groupid, mapid) "
											   "SELECT :id as groupid, T.mapid as mapid "
											   "FROM (SELECT column1 as mapid FROM (values ?l? ) EXCEPT SELECT mapid from bindGroupMap WHERE groupid=:id2) T",
											   ul, p1, true)) {
				(*jsonResponse)["error"] = "update error";
				m_client->db()->db().rollback();
				return;
			}
		}
	}



	(*jsonResponse)["updated"] = groupid;

	m_client->db()->db().commit();
}



/**
 * @brief TeacherGroups::removeGroup
 * @param jsonResponse
 */

void TeacherGroups::removeGroup(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt(-1);

	if (groupid != -1) {
		QVariantList params;
		params << groupid;
		params << m_client->clientUserName();

		if (!m_client->db()->execSimpleQuery("DELETE FROM studentgroup WHERE id=? AND owner=?", params)) {
			(*jsonResponse)["error"] = "internal db error";
			return;
		}

		(*jsonResponse)["removed"] = groupid;
		return;
	}

	QVariantList list = m_jsonData.value("list").toArray().toVariantList();

	if (!list.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return;
	}

	QVariantList ulist;

	for (int i=0; i<list.count(); ++i) {
		ulist << m_client->clientUserName();
	}

	QVariantList data;
	data << QVariant(list);
	data << QVariant(ulist);

	if (m_client->db()->execBatchQuery("DELETE FROM studentgroup WHERE id=? AND owner=?", data))
		(*jsonResponse)["removed"] = QJsonArray::fromVariantList(list);
	else
		(*jsonResponse)["error"] = "sql error";
}




/**
 * @brief TeacherGroups::getExcludedUsers
 * @param jsonResponse
 */

void TeacherGroups::getExcludedUsers(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << groupid;

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT username, firstname, lastname, email, active, COALESCE(classid, -1) as classid, class.name as classname, "
									"isTeacher, isAdmin FROM user "
									"LEFT JOIN class ON (class.id=user.classid) "
									"WHERE user.username NOT IN (SELECT username FROM bindGroupStudent WHERE groupid=?) "
									"ORDER BY firstname, lastname",
									params, &l);

	(*jsonResponse)["list"] = l;
}




/**
 * @brief TeacherGroups::getExcludedClasses
 * @param jsonResponse
 */

void TeacherGroups::getExcludedClasses(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << groupid;

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT id, name FROM class WHERE id NOT IN (SELECT classid FROM bindGroupClass WHERE groupid=?) "
									"ORDER BY name", params, &l);

	(*jsonResponse)["list"] = l;
}





/**
 * @brief TeacherGroups::getExcludedMaps
 * @param jsonResponse
 */

void TeacherGroups::getExcludedMaps(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << groupid;
	params << m_client->clientUserName();

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT id, name, version FROM map WHERE id NOT IN (SELECT mapid FROM bindGroupMap WHERE groupid=?) AND owner=? "
									"ORDER BY name", params, &l);

	(*jsonResponse)["list"] = l;
}



/**
 * @brief TeacherGroups::addMaps
 * @param jsonResponse
 */

void TeacherGroups::addMaps(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt(-1);
	QJsonArray maplist = m_jsonData.value("list").toArray();

	if (groupid == -1 || !maplist.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return;
	}

	QVariantMap params;

	m_client->db()->db().transaction();

	QVariantMap p1;
	p1[":id"] = groupid;
	p1[":id2"] = groupid;

	QVariantList ul;

	foreach (QJsonValue v, maplist) {
		ul << v.toInt();
	}

	if (!m_client->db()->execListQuery("INSERT INTO bindGroupMap(groupid, mapid) "
									   "SELECT :id as groupid, T.mapid as mapid "
									   "FROM (SELECT column1 as mapid FROM (values ?l? ) EXCEPT SELECT mapid from bindGroupMap WHERE groupid=:id2) T",
									   ul, p1, true)) {
		(*jsonResponse)["error"] = "update error";
		m_client->db()->db().rollback();
		return;
	}


	(*jsonResponse)["updated"] = groupid;

	m_client->db()->db().commit();
}


/**
 * @brief TeacherGroups::addClasses
 * @param jsonResponse
 */

void TeacherGroups::addClasses(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt(-1);
	QJsonArray maplist = m_jsonData.value("list").toArray();

	if (groupid == -1 || !maplist.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return;
	}

	QVariantMap params;

	m_client->db()->db().transaction();

	QVariantMap p1;
	p1[":id"] = groupid;
	p1[":id2"] = groupid;

	QVariantList ul;

	foreach (QJsonValue v, maplist) {
		ul << v.toInt();
	}

	if (!m_client->db()->execListQuery("INSERT INTO bindGroupClass(groupid, classid) "
									   "SELECT :id as groupid, T.classid as classid "
									   "FROM (SELECT column1 as classid FROM (values ?l? ) EXCEPT SELECT classid from bindGroupClass WHERE groupid=:id2) T",
									   ul, p1, true)) {
		(*jsonResponse)["error"] = "update error";
		m_client->db()->db().rollback();
		return;
	}


	(*jsonResponse)["updated"] = groupid;

	m_client->db()->db().commit();
}




/**
 * @brief TeacherGroups::addUsers
 * @param jsonResponse
 */

void TeacherGroups::addUsers(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt(-1);
	QJsonArray maplist = m_jsonData.value("list").toArray();

	if (groupid == -1 || !maplist.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return;
	}

	QVariantMap params;

	m_client->db()->db().transaction();

	QVariantMap p1;
	p1[":id"] = groupid;
	p1[":id2"] = groupid;

	QVariantList ul;

	foreach (QJsonValue v, maplist) {
		ul << v.toString();
	}

	if (!m_client->db()->execListQuery("INSERT INTO bindGroupStudent(groupid, username) "
									   "SELECT :id as groupid, T.username as username "
									   "FROM (SELECT column1 as username FROM (values ?l? ) EXCEPT SELECT username from bindGroupStudent WHERE groupid=:id2) T",
									   ul, p1, true)) {
		(*jsonResponse)["error"] = "update error";
		m_client->db()->db().rollback();
		return;
	}


	(*jsonResponse)["updated"] = groupid;

	m_client->db()->db().commit();
}
