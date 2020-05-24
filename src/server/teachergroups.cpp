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

	QJsonArray students;

	m_client->mapDb()->db()->execSelectQuery("SELECT bindGroupStudent.username as username, firstname, lastname, isTeacher, classid, classname "
											 "FROM bindGroupStudent "
											 "LEFT JOIN user ON (user.username=bindGroupStudent.username) "
											 "LEFT JOIN class ON (class.id=user.classid) "
											 "WHERE groupid=? AND user.active=true ORDER BY firstname, lastname", l, &students);

	(*jsonResponse)["students"] = students;



	QJsonArray classes;

	m_client->mapDb()->db()->execSelectQuery("SELECT id, name "
											 "FROM bindGroupClass "
											 "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
											 "WHERE groupid=? ORDER BY name", l, &classes);

	(*jsonResponse)["classes"] = classes;

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

	QVariantMap info = m_client->mapDb()->create(id);

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


	if (m_jsonData.contains("students")) {
		QJsonArray userlist = m_jsonData.value("students").toArray();

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


	(*jsonResponse)["updated"] = groupid;

	m_client->db()->db().commit();
}



/**
 * @brief TeacherGroups::removeGroup
 * @param jsonResponse
 */

void TeacherGroups::removeGroup(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << groupid;
	params << m_client->clientUserName();

	if (!m_client->db()->execSimpleQuery("DELETE FROM studentgroup WHERE id=? AND owner=?", params)) {
		(*jsonResponse)["error"] = "internal db error";
		return;
	}

	(*jsonResponse)["removed"] = groupid;
}



/**
 * @brief TeacherGroups::getMembers
 * @param jsonResponse
 */

void TeacherGroups::getMembers(QJsonObject *jsonResponse, QByteArray *)
{
	int groupid = m_jsonData.value("id").toInt();

	QVariantList params;
	params << groupid;
	params << m_client->clientUserName();

	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT studentGroupInfo.name as groupName, studentGroupInfo.username as username, firstname, lastname, "
									"isTeacher, class.id as classid, class.name as classname "
									"FROM studentGroupInfo "
									"LEFT JOIN user ON (studentGroupInfo.username=user.username) "
									"LEFT JOIN class ON (class.id=user.classid) "
									"WHERE studentGroupInfo.id=? AND studentGroupInfo.owner=? ORDER BY firstanem, lastname",
									params,
									&l);
	(*jsonResponse)["list"] = l;
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
