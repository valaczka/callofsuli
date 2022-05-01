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
																	"classname, isTeacher, isAdmin FROM userInfo",
																	params));
	(*jsonResponse)["list"] = l;

	return true;
}



/**
 * @brief Admin::userListGet
 * @param jsonResponse
 * @return
 */

bool Admin::userListGet(QJsonObject *jsonResponse, QByteArray *)
{
	const QJsonObject &args = m_message.jsonData();
	QJsonArray l;
	QVariantList params;

	QString q = "SELECT username, firstname, lastname, active, COALESCE(classid, -1) as classid, classname, "
				"isTeacher, isAdmin, nickname, character, picture, "
				"rankid, rankname, ranklevel, rankimage FROM userInfo ";
	QString w;


	if (args.contains("classid")) {
		w += (w.isEmpty() ? "" : " AND ");
		w += "COALESCE(classid, -1)=?";
		params << args.value("classid").toInt(-1);
	}


	if (args.contains("isTeacher")) {
		w += (w.isEmpty() ? "" : " AND ");
		w += "isTeacher=?";
		params << args.value("isTeacher").toBool();
	}

	if (args.contains("isAdmin")) {
		w += (w.isEmpty() ? "" : " AND ");
		w += "isAdmin=?";
		params << args.value("isAdmin").toBool();
	}


	if (!w.isEmpty())
		q += " WHERE "+w;

	l = QJsonArray::fromVariantList(m_client->db()->execSelectQuery(q, params));
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
																						"COALESCE(classid, -1) as classid, classname, "
																						"isTeacher, isAdmin FROM userInfo "
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
	QString oauthToken;

	if (params.value("classid", -1) == -1) {
		int defaultClassid = m_client->db()->execSelectQueryOneRow("SELECT id FROM class WHERE id="
																   "(SELECT value FROM settings WHERE key='registration.defaultClass')")
							 .value("id", -1).toInt();

		if (defaultClassid != -1)
			params["classid"] = defaultClassid;
		else
			params["classid"] = QVariant::Invalid;

	}

	if (params.contains("oauthToken")) {
		oauthToken = params.value("oauthToken").toString();
		params.remove("oauthToken");
	}

	if (!params.contains("character"))
		params["character"] = "default";

	qDebug() << "CREATE USER" << params;

	int id = m_client->db()->execInsertQuery("INSERT INTO user (?k?) VALUES (?)", params);

	if (id == -1)
	{
		(*jsonResponse)["error"] = "create";
		return false;
	}

	QVariantMap m;
	m["username"] = username;

	if (!oauthToken.isEmpty()) {
		m["oauthToken"] = oauthToken;
		m["password"] = "*";
	}

	id = m_client->db()->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", m);

	if (id == -1)
	{
		(*jsonResponse)["error"] = "create auth";
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

	if (params.contains("classid") && params.value("classid", -1) == -1)
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
 * @brief Admin::userPasswordChange
 * @param jsonResponse
 * @return
 */

bool Admin::userPasswordChange(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	QString username = params.value("username").toString();
	QString password = params.value("password").toString();

	if (username.isEmpty()) {
		(*jsonResponse)["error"] = "missing username";
		return false;
	}


	if (params.contains("oldPassword")) {
		QString oldPassword = params.value("oldPassword").toString();
		QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT password, salt FROM auth WHERE username=?", {username});

		if (!m.isEmpty()) {
			QString storedPassword = m.value("password").toString();
			QString salt = m.value("salt").toString();
			QString hashedPassword = CosDb::hashPassword(oldPassword, &salt);

			if (QString::compare(storedPassword, hashedPassword, Qt::CaseInsensitive) != 0) {
				(*jsonResponse)["error"] = "invalid password";
				return false;
			}
		} else {
			(*jsonResponse)["error"] = "invalid password";
			return false;
		}
	}


	if (!userPasswordChangeReal(username, password)) {
		setServerError();
		return false;
	}

	(*jsonResponse)["username"] = username;
	(*jsonResponse)["passwordChanged"] = true;
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
	int classid = m_message.jsonData().value("id").toInt();
	bind[":id"] = classid;
	QVariantMap params = m_message.jsonData().toVariantMap();
	params.remove("id");

	if (!m_client->db()->execUpdateQuery("UPDATE class SET ? WHERE id=:id", params, bind)) {
		setServerError();
		return false;
	}


	(*jsonResponse)["updated"] = classid;
	(*jsonResponse)["name"] = m_client->db()->execSelectQueryOneRow("SELECT name FROM class WHERE id=?", { classid }).value("name").toString();

	return true;
}



/**
 * @brief User::classRemove
 * @param jsonResponse
 */

bool Admin::classRemove(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();
	int id = params.value("id").toInt(-1);

	QVariantList list;

	if (id != -1)
		list.append(id);

	foreach (QJsonValue v, params.value("list").toArray())
		list.append(v.toInt());


	if (list.size()) {
		if (!m_client->db()->execListQuery("DELETE FROM class WHERE id IN (?l?)", list)) {
			(*jsonResponse)["error"] = "sql error";
			setServerError();
			return false;
		}
	} else{
		(*jsonResponse)["error"] = "invalid parameters";
		setServerError();
		return false;
	}

	(*jsonResponse)["removed"] = true;

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

	(*jsonResponse)["classList"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id, name FROM class"));

	(*jsonResponse)["codeList"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT classid, code, name FROM classRegistration "
																							  "LEFT JOIN class ON (classRegistration.classid=class.id) "
																							  "ORDER BY name"));

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
 * @brief Admin::classRegistration
 * @param jsonResponse
 * @return
 */

bool Admin::classRegistration(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();

	if (params.value("refresh").toBool()) {
		const QString characters = "ABCDEFGHJKLMNPRSTUVWXYZ123456789";
		QVariantList list = m_client->db()->execSelectQuery("SELECT id FROM class");

		QVariantList codes;

		while (codes.size() < list.size()+1) {
			QString code;

			for (int i=0; i<6; i++)
				code.append(characters.at(QRandomGenerator::global()->bounded(characters.size())));

			if (!codes.contains(code))
				codes.append(code);
		}

		QVariantList classids;

		foreach (QVariant v, list)
			classids.append(v.toMap().value("id").toInt());

		classids.append(QVariant::Invalid);

		if (!m_client->db()->execSimpleQuery("DELETE FROM classRegistration") ||
			!m_client->db()->execBatchQuery("INSERT INTO classRegistration (classid, code) VALUES (?,?)", {classids, codes})) {
			(*jsonResponse)["error"] = "sql";
			return false;
		}

		(*jsonResponse)["success"] = true;
		return true;
	}

	m_client->db()->transaction();


	m_client->db()->commit();


	return false;
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





/**
 * @brief Admin::userCreateReal
 * @param username
 * @param classCode
 * @param oauthToken
 * @param character
 * @param isTeacher
 */


QString Admin::userCreateReal(const QString &username,
							  const QString &firstname,
							  const QString &lastname,
							  const bool &isTeacher,
							  const QString &classCode,
							  const QString &oauthToken,
							  const QString &picture,
							  const QString &character)
{
	if (username.isEmpty())
		return "missing username";

	if (m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM user WHERE username=?) as v", {username})
		.value("v", false).toBool()) {
		return "username exists";
	}


	QVariantMap params;


	int forcedClassid = m_client->db()->execSelectQueryOneRow("SELECT id FROM class WHERE id="
															  "(SELECT value FROM settings WHERE key='registration.forced')")
						.value("id", -1).toInt();

	if (forcedClassid != -1) {
		params["classid"] = forcedClassid;
	} else {
		QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT code, classid FROM classRegistration WHERE code=?", {classCode});

		if (m.isEmpty())
			return "invalid code";

		int classId = m.value("classid", -1).toInt();

		if (classId > 0)
			params["classid"] = classId;
		else
			params["classid"] = QVariant::Invalid;
	}


	params["username"] = username;
	params["firstname"] = firstname;
	params["lastname"] = lastname;
	params["isTeacher"] = isTeacher;
	params["picture"] = picture;
	params["character"] = character;
	params["active"] = true;

	qDebug() << "CREATE USER" << params;

	int id = m_client->db()->execInsertQuery("INSERT INTO user (?k?) VALUES (?)", params);

	if (id == -1) {
		qWarning() << "Error create user" << username;
		return "create error";
	}

	QVariantMap m;
	m["username"] = username;

	if (!oauthToken.isEmpty()) {
		m["oauthToken"] = oauthToken;
		m["password"] = "*";
	}

	id = m_client->db()->execInsertQuery("INSERT INTO auth (?k?) VALUES (?)", m);

	if (id == -1)
		qWarning() << "Error create auth for" << username;

	return "";
}





/**
 * @brief Admin::userPasswordChangeReal
 * @param username
 * @param password
 * @return
 */

bool Admin::userPasswordChangeReal(const QString &username, const QString &password)
{
	QString salt;
	QString hashedPassword = CosDb::hashPassword(password, &salt);

	return m_client->db()->execSimpleQuery("INSERT OR REPLACE INTO auth (username, password, salt, oauthToken) VALUES (?, ?, ?, NULL)",
										   {username, hashedPassword, salt});
}
