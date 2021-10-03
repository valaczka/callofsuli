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

#include "admin.h"
#include "server.h"
#include "cosmessage.h"


Admin::Admin(Client *client, const CosMessage &message)
	: AbstractHandler(client, message, CosMessage::ClassAdmin)
{

}


/**
 * @brief User::classInit
 * @return
 */

bool Admin::classInit()
{
	if (!m_client->clientRoles().testFlag(CosMessage::RoleAdmin))
		return false;

	return true;
}


/**
 * @brief User::getAllUser
 * @param jsonResponse
 */

bool Admin::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	QVariantList params;

	l = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT username, firstname, lastname, active, COALESCE(classid, -1) as classid, "
																	"class.name as classname, "
																	"isTeacher, isAdmin FROM user "
																	"LEFT JOIN class ON (class.id=user.classid)",
																	params));
	(*jsonResponse)["list"] = l;

	return true;
}


/**
 * @brief User::userGet
 * @param jsonResponse
 */

bool Admin::userGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList params;

	params << m_message.jsonData().value("username").toString();

	(*jsonResponse) = QJsonObject::fromVariantMap(m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, active, "
																						"classid, class.name as classname, "
																						"isTeacher, isAdmin FROM user "
																						"LEFT JOIN class ON (class.id=user.classid) "
																						"WHERE username=?",
																						params));

	return true;
}




/**
 * @brief User::userAdd
 * @param jsonResponse
 */

bool Admin::userCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
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

	return true;
}


/**
 * @brief User::userUpdate
 * @param jsonResponse
 */

bool Admin::userModify(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap bind;
	bind[":username"] = m_message.jsonData().value("username").toString();

	QVariantMap params = m_message.jsonData().toVariantMap();
	params.remove("username");

	if (params.value("classid", -1) == -1)
		params["classid"] = QVariant::Invalid;

	if (!m_client->db()->execUpdateQuery("UPDATE user SET ? WHERE username=:username", params, bind)) {
		setServerError();
		return false;
	}

	(*jsonResponse)["modified"] = bind.value(":username").toString();

	return true;
}


/**
 * @brief User::userBatchUpdate
 * @param jsonResponse
 */

bool Admin::userBatchUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList users = m_message.jsonData().value("users").toArray().toVariantList();

	if (!users.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return false;
	}

	QVariantMap params = m_message.jsonData().toVariantMap();
	params.remove("users");


	QStringList paramList;
	QVariantList data;


	QMapIterator<QString, QVariant> i(params);
	while (i.hasNext()) {
		i.next();
		paramList << i.key()+"=?";

		QVariantList d;
		for (int j=0; j<users.count(); ++j)
			d << i.value();
		data << QVariant(d);
	}

	QString cmd = "UPDATE USER SET "+paramList.join(", ")+" WHERE username=?";
	data << QVariant(users);

	if (m_client->db()->execBatchQuery(cmd, data))
		(*jsonResponse)["updated"] = true;
	else {
		setServerError();
		return false;
	}

	return true;
}


/**
 * @brief User::userBatchRemove
 * @param jsonResponse
 */

bool Admin::userBatchRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList list = m_message.jsonData().value("list").toArray().toVariantList();

	if (!list.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return false;
	}

	QVariantList data;
	data << QVariant(list);

	if (m_client->db()->execBatchQuery("DELETE FROM user WHERE username=?", data))
		(*jsonResponse)["removed"] = true;
	else {
		setServerError();
		return false;
	}

	return true;

}



/**
 * @brief User::getAllClass
 * @param jsonResponse
 */

bool Admin::getAllClass(QJsonObject *jsonResponse, QByteArray *)
{
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id, name FROM class ORDER BY name"));

	return true;
}


/**
 * @brief User::classAdd
 * @param jsonResponse
 */

bool Admin::classCreate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap m;

	m["name"] = m_message.jsonData().value("name").toString();

	int id = m_client->db()->execInsertQuery("INSERT INTO class (?k?) VALUES (?)", m);

	if (id == -1) {
		setServerError();
		return false;
	}

	(*jsonResponse)["created"] = id;

	return true;
}



/**
 * @brief User::classUpdate
 * @param jsonResponse
 */


bool Admin::classUpdate(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap bind;
	bind[":id"] = m_message.jsonData().value("id").toInt();
	QVariantMap params = m_message.jsonData().toVariantMap();
	params.remove("id");

	if (!m_client->db()->execUpdateQuery("UPDATE class SET ? WHERE id=:id", params, bind)) {
		setServerError();
		return false;
	}

	(*jsonResponse)["updated"] = bind.value(":id").toInt();

	return true;
}



/**
 * @brief User::classRemove
 * @param jsonResponse
 */

bool Admin::classBatchRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantList list = m_message.jsonData().value("list").toArray().toVariantList();

	if (!list.count()) {
		(*jsonResponse)["error"] = "invalid parameters";
		return false;
	}

	QVariantList data;
	data << QVariant(list);

	if (m_client->db()->execBatchQuery("DELETE FROM class WHERE id=?", data))
		(*jsonResponse)["removed"] = true;
	else {
		setServerError();
		return false;
	}

	return true;
}


/**
 * @brief Admin::getSettings
 * @param jsonResponse
 * @return
 */

bool Admin::getSettings(QJsonObject *jsonResponse, QByteArray *)
{
	(*jsonResponse)["serverName"] = QJsonValue::fromVariant(m_client->db()->execSelectQueryOneRow("SELECT serverName from system").value("serverName"));

	QVariantList list = m_client->db()->execSelectQuery("SELECT key, value FROM settings");

	foreach (QVariant d, list) {
		QString key = d.toMap().value("key").toString();
		(*jsonResponse)[key] = QJsonValue::fromVariant(d.toMap().value("value"));
	}

	return true;
}


/**
 * @brief Admin::setSettings
 * @param jsonResponse
 * @return
 */

bool Admin::setSettings(QJsonObject *jsonResponse, QByteArray *)
{
	QStringList keys = m_message.jsonData().keys();

	m_client->db()->transaction();
	foreach (QString k, keys) {
		bool success = false;
		if (k == "serverName") {
			QVariantList l;
			l << m_message.jsonData().value(k).toString();
			success = m_client->db()->execSimpleQuery("UPDATE system SET serverName=?", l);
		} else {
			QVariantList l;
			l << k;
			l << m_message.jsonData().value(k).toString();
			success = m_client->db()->execSimpleQuery("INSERT OR REPLACE INTO settings (key, value) VALUES(?, ?)", l);
		}

		if (!success) {
			m_client->db()->rollback();
			setServerError();
			return false;
		}
	}

	m_client->db()->commit();

	(*jsonResponse)["success"] = true;

	emit m_client->server()->serverInfoChanged();

	return true;
}


/**
 * @brief Admin::getAllClients
 * @param jsonResponse
 * @return
 */

bool Admin::getAllClients(QJsonObject *jsonResponse, QByteArray *)
{
	QList<Client *> clients = m_client->server()->clients();

	QJsonArray list;

	foreach (Client *c, clients) {
		QWebSocket *socket = c->socket();
		QJsonObject o;
		o["state"] = c->clientState();
		o["username"] = c->clientUserName();
		o["roles"] = (int)(c->clientRoles());
		o["host"] = socket->peerAddress().toString();
		o["port"] = socket->peerPort();
		list.append(o);
	}

	(*jsonResponse)["list"] = list;
	return true;
}




