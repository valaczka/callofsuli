/*
 * ---- Call of Suli ----
 *
 * userinfo.cpp
 *
 * Created on: 2020. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserInfo
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

#include "userinfo.h"

UserInfo::UserInfo(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: AbstractHandler(client, object, binaryData)
{

}


/**
 * @brief UserInfo::getServer
 * @return
 */

void UserInfo::getServerInfo(QJsonObject *jsonResponse, QByteArray *)
{
	m_client->db()->execSelectQueryOneRow("SELECT serverName from system", QVariantList(), jsonResponse);
	m_client->db()->execSelectQueryOneRow("SELECT COALESCE(value, false) as passwordResetEnabled FROM settings WHERE key='email.passwordReset'", QVariantList(), jsonResponse);
	m_client->db()->execSelectQueryOneRow("SELECT COALESCE(value, false) as registrationEnabled FROM settings WHERE key='email.registration'", QVariantList(), jsonResponse);

	(*jsonResponse)["registrationDomains"] = QJsonArray::fromStringList(m_client->emailRegistrationDomainList());
}




/**
 * @brief UserInfo::getUser
 */

void UserInfo::getUser(QJsonObject *jsonResponse, QByteArray *)
{
	QString username = m_jsonData["username"].toString();
	if (username.isEmpty())
		username = m_client->clientUserName();

	if (username.isEmpty())
		return;

	QVariantList l;
	l << username;

	m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, email, active, "
										  "isTeacher, isAdmin, classid, classname, xp, rankid, rankname "
										  "FROM userInfo where username=?", l, jsonResponse);

}




/**
 * @brief UserInfo::getAllUser
 * @return
 */

void UserInfo::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT username, firstname, lastname, email, active, "
									"isTeacher, isAdmin, classid, classname, xp, rankid, rankname "
									"FROM userInfo ORDER BY firstname, lastname",
									QVariantList(),
									&l);
	(*jsonResponse)["list"] = l;
}


/**
 * @brief UserInfo::registrationRequest
 * @param jsonResponse
 */

void UserInfo::registrationRequest(QJsonObject *jsonResponse, QByteArray *)
{
	QString email = m_jsonData["email"].toString();
	QString firstname = m_jsonData["firstname"].toString();
	QString lastname = m_jsonData["lastname"].toString();

	if (email.isEmpty()) {
		(*jsonResponse)["error"] = true;
		(*jsonResponse)["errorString"] = "email empty";
		return;
	}

	QVariantList l;
	l << email;

	QVariantMap m;
	m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM user WHERE email=?) as e", l, &m);

	if (m.value("e", false).toBool()) {
		(*jsonResponse)["error"] = true;
		(*jsonResponse)["errorString"] = "email exists";
		return;
	}

	QStringList domainList = m_client->emailRegistrationDomainList();
	bool match = domainList.count() ? false : true;

	foreach (QString s, domainList) {
		if (email.endsWith(s)) {
			match = true;
			break;
		}
	}


	if (!match) {
		(*jsonResponse)["error"] = true;
		(*jsonResponse)["errorString"] = "email invalid";
		return;
	}

	QVariantMap ins;
	ins["email"] = email;
	ins["firstname"] = firstname;
	ins["lastname"] = lastname;

	int rowId = m_client->db()->execInsertQuery("INSERT OR REPLACE INTO registration (?k?) VALUES (?)", ins);

	if (rowId == -1) {
		(*jsonResponse)["error"] = true;
		(*jsonResponse)["errorString"] = "internal error";
		return;
	}

	QVariantList ll;
	ll << rowId;
	m_client->db()->execSelectQueryOneRow("SELECT code FROM registration WHERE id=?", ll, &m);

	QString code = m["code"].toString();

	if (code.isEmpty()) {
		(*jsonResponse)["error"] = true;
		(*jsonResponse)["errorString"] = "internal code error";
		return;
	}

	if (m_client->emailRegistration(email, firstname, lastname, code)) {
		(*jsonResponse)["error"] = false;
		(*jsonResponse)["success"] = true;
		return;
	} else {
		(*jsonResponse)["error"] = true;
		(*jsonResponse)["errorString"] = "smtp error";
		return;
	}
}



/**
 * @brief UserInfo::getSettings
 * @param jsonResponse
 */

void UserInfo::getSettings(QJsonObject *jsonResponse, QByteArray *)
{
	if (!m_client->clientRoles().testFlag(Client::RoleAdmin)) {
		m_client->sendError("permission denied");
		return;
	}

	m_client->db()->execSelectQueryOneRow("SELECT serverName from system", QVariantList(), jsonResponse);

	QJsonArray list;
	m_client->db()->execSelectQuery("SELECT key, value FROM settings", QVariantList(), &list);

	foreach (QJsonValue d, list) {
		QString key = d["key"].toString();
		(*jsonResponse)[key] = d["value"];
	}
}


/**
 * @brief UserInfo::setSettings
 * @param jsonResponse
 */

void UserInfo::setSettings(QJsonObject *jsonResponse, QByteArray *)
{
	if (!m_client->clientRoles().testFlag(Client::RoleAdmin)) {
		m_client->sendError("permission denied");
		return;
	}

	QStringList keys = m_jsonData.keys();

	m_client->db()->db().transaction();
	foreach (QString k, keys) {
		bool success = false;
		if (k == "serverName") {
			QVariantList l;
			l << m_jsonData.value(k).toString();
			success = m_client->db()->execSimpleQuery("UPDATE system SET serverName=?", l);
		} else {
			QVariantList l;
			l << k;
			l << m_jsonData.value(k).toString();
			success = m_client->db()->execSimpleQuery("INSERT OR REPLACE INTO settings (key, value) VALUES(?, ?)", l);
		}

		if (!success) {
			m_client->db()->db().rollback();
			(*jsonResponse)["error"] = true;
			return;
		}
	}

	m_client->db()->db().commit();
	(*jsonResponse)["error"] = false;
}



