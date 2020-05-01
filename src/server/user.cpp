/*
 * ---- Call of Suli ----
 *
 * user.cpp
 *
 * Created on: 2020. 04. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * User
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

#include "user.h"


User::User(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: AbstractHandler(client, object, binaryData)
{

}


/**
 * @brief User::classInit
 * @return
 */

bool User::classInit()
{
	if (!m_client->clientRoles().testFlag(Client::RoleAdmin))
		return false;

	return true;
}


/**
 * @brief User::getAllUser
 * @param jsonResponse
 */

void User::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	QVariantList params;


	m_client->db()->execSelectQuery("SELECT username, firstname, lastname, email, active, COALESCE(classid, -1) as classid, class.name as classname, "
									"isTeacher, isAdmin FROM user "
									"LEFT JOIN class ON (class.id=user.classid)",
									params, &l);
	(*jsonResponse)["list"] = l;

}


/**
 * @brief User::userGet
 * @param jsonResponse
 */

void User::userGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;

	params << m_jsonData["username"].toString();

	m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, email, active, classid, class.name as classname, "
									"isTeacher, isAdmin FROM user "
									"LEFT JOIN class ON (class.id=user.classid) "
									"WHERE username=?",
									params, jsonResponse);
}




/**
 * @brief User::userAdd
 * @param jsonResponse
 */

void User::userCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_jsonData.toVariantMap();
	QString username = params["username"].toString();

	if (params.value("classid", -1) == -1)
		params["classid"] = QVariant::Invalid;

	int id = m_client->db()->execInsertQuery("INSERT INTO user (?k?) VALUES (?)", params);

	if (id == -1) {
		(*jsonResponse)["error"] = "user create error";
		return;
	}

	QVariantMap m;
	m["username"] = username;

	if (m_client->db()->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", m) == -1) {
		(*jsonResponse)["error"] = "user auth create error";
		return;
	}

	(*jsonResponse)["created"] = id;
	(*jsonResponse)["createdUserName"] = username;
}


/**
 * @brief User::userUpdate
 * @param jsonResponse
 */

void User::userUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap bind;
	bind[":username"] = m_jsonData["username"].toString();

	QVariantMap params = m_jsonData.toVariantMap();
	params.remove("username");

	if (params.value("classid", -1) == -1)
		params["classid"] = QVariant::Invalid;

	if (!m_client->db()->execUpdateQuery("UPDATE user SET ? WHERE username=:username", params, bind)) {
		(*jsonResponse)["error"] = "user update error";
		return;
	}

	(*jsonResponse)["updatedUserName"] = bind[":username"].toString();
}


/**
 * @brief User::userBatchUpdate
 * @param jsonResponse
 */

void User::userBatchUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList users = m_jsonData["users"].toArray().toVariantList();

	if (!users.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return;
	}

	QVariantMap params = m_jsonData.toVariantMap();
	params.remove("users");


	QStringList paramList;
	QVariantList dataList;


	QMapIterator<QString, QVariant> i(params);
	while (i.hasNext()) {
		i.next();
		paramList << i.key()+"=?";

		QVariantList d;
		for (int j=0; j<users.count(); ++j)
			d << i.value();
		QVariantMap m;
		m["list"] = d;
		dataList << m;
	}

	QString cmd = "UPDATE USER SET "+paramList.join(", ")+" WHERE username=?";
	QVariantMap m;
	m["list"] = users;
	dataList << m;

	if (m_client->db()->execBatchQuery(cmd, dataList))
		(*jsonResponse)["updated"] = true;
	else
		(*jsonResponse)["error"] = "sql error";
}



/**
 * @brief User::getAllClass
 * @param jsonResponse
 */

void User::getAllClass(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT id, name FROM class ORDER BY name", QVariantList(), &l);
	(*jsonResponse)["list"] = l;
}


/**
 * @brief User::classAdd
 * @param jsonResponse
 */

void User::classCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap m;

	m["name"] = m_jsonData["name"].toString();

	int id = m_client->db()->execInsertQuery("INSERT INTO class (?k?) VALUES (?)", m);

	if (id == -1) {
		(*jsonResponse)["error"] = "class create error";
		return;
	}

	(*jsonResponse)["created"] = id;
}



/**
 * @brief User::classUpdate
 * @param jsonResponse
 */


void User::classUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap bind;
	bind[":id"] = m_jsonData["id"].toInt();
	QVariantMap params = m_jsonData.toVariantMap();
	params.remove("id");

	if (!m_client->db()->execUpdateQuery("UPDATE class SET ? WHERE id=:id", params, bind)) {
		(*jsonResponse)["error"] = "class update error";
		return;
	}

	(*jsonResponse)["updated"] = bind[":id"].toInt();
}


/**
 * @brief User::classRemove
 * @param jsonResponse
 */

void User::classBatchRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList list = m_jsonData["list"].toArray().toVariantList();

	if (!list.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return;
	}

	QVariantList dataList;
	QVariantMap m;
	m["list"] = list;
	dataList << m;

	if (m_client->db()->execBatchQuery("DELETE FROM class WHERE id=?", dataList))
		(*jsonResponse)["removed"] = true;
	else
		(*jsonResponse)["error"] = "sql error";
}




