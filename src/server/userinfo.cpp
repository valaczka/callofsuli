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

#include "admin.h"
#include "userinfo.h"
#include "server.h"

UserInfo::UserInfo(Client *client, const CosMessage &message)
	: AbstractHandler(client, message, CosMessage::ClassUserInfo)
{

}


/**
 * @brief UserInfo::getServer
 * @return
 */

bool UserInfo::getServerInfo(QJsonObject *jsonResponse, QByteArray *)
{
	m_client->db()->execSelectQueryOneRow("SELECT serverName from system", QVariantList(), jsonResponse);
	m_client->db()->execSelectQueryOneRow("SELECT COALESCE(value, false) as passwordResetEnabled FROM settings WHERE key='email.passwordReset'", QVariantList(), jsonResponse);
	m_client->db()->execSelectQueryOneRow("SELECT COALESCE(value, false) as registrationEnabled FROM settings WHERE key='email.registration'", QVariantList(), jsonResponse);

	(*jsonResponse)["registrationDomains"] = QJsonArray::fromStringList(m_client->emailRegistrationDomainList());

	return true;
}




/**
 * @brief UserInfo::getUser
 */

bool UserInfo::getUser(QJsonObject *jsonResponse, QByteArray *)
{
	QString username = m_message.jsonData().value("username").toString();
	if (username.isEmpty())
		username = m_client->clientUserName();

	if (username.isEmpty()) {
		return true;
	}

	QVariantList l;
	l << username;

	m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, active, "
										  "isTeacher, isAdmin, classid, classname, xp, rankid, rankname "
										  "FROM userInfo where username=?", l, jsonResponse);

	return true;

}




/**
 * @brief UserInfo::getAllUser
 * @return
 */

bool UserInfo::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT username, firstname, lastname, email, active, "
									"isTeacher, isAdmin, classid, classname, xp, rankid, rankname "
									"FROM userInfo ORDER BY firstname, lastname",
									QVariantList(),
									&l);
	(*jsonResponse)["list"] = l;

	return true;
}


/**
 * @brief UserInfo::registrationRequest
 * @param jsonResponse
 */

bool UserInfo::registrationRequest(QJsonObject *jsonResponse, QByteArray *)
{
	QString email = m_message.jsonData().value("email").toString();
	QString firstname = m_message.jsonData().value("firstname").toString();
	QString lastname = m_message.jsonData().value("lastname").toString();

	if (email.isEmpty()) {
		(*jsonResponse)["error"] = "email empty";
		return false;
	}

	QVariantList l;
	l << email;

	QVariantMap m;
	m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM user WHERE username=?) as e", l, &m);

	if (m.value("e", false).toBool()) {
		(*jsonResponse)["error"] = "email exists";
		return false;
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
		(*jsonResponse)["error"] = "email invalid";
		return false;
	}

	QVariantMap ins;
	ins["email"] = email;
	ins["firstname"] = firstname;
	ins["lastname"] = lastname;

	int rowId = m_client->db()->execInsertQuery("INSERT OR REPLACE INTO registration (?k?) VALUES (?)", ins);

	if (rowId == -1) {
		setServerError();
		return false;
	}

	QVariantList ll;
	ll << rowId;
	m_client->db()->execSelectQueryOneRow("SELECT code FROM registration WHERE id=?", ll, &m);

	QString code = m.value("code").toString();

	if (code.isEmpty()) {
		setServerError();
		return false;
	}

	if (emailRegistration(email, firstname, lastname, code)) {
		(*jsonResponse)["success"] = true;
		return true;
	} else {
		(*jsonResponse)["error"] = "smtp error";
		return false;
	}
}


/**
 * @brief UserInfo::registerUser
 * @param jsonResponse
 * @return
 */

bool UserInfo::registerUser(QJsonObject *jsonResponse, QByteArray *)
{
	QString email = m_message.jsonData().value("email").toString();
	QString password = m_message.jsonData().value("password").toString();

	QVariantMap m;

	QVariantList l;
	l << email;
	l << password;
	if (!m_client->db()->execSelectQueryOneRow("SELECT firstname, lastname FROM registration WHERE email=? and code=?", l, &m)) {
		(*jsonResponse)["error"] = "invalid email or code";
		return false;
	}

	QVariantList ll;
	ll << email;


	QJsonObject obj;
	obj["username"] = email;
	obj["firstname"] = m.value("firstname").toString();
	obj["lastname"] = m.value("lastname").toString();
	obj["active"] = true;

	CosMessage m2(obj, CosMessage::ClassInvalid, "");

	QJsonObject ret;
	Admin u(m_client, m2);
	bool isSuccess = u.userCreate(&ret, nullptr);

	if (isSuccess) {
		m_client->db()->execSimpleQuery("DELETE FROM registration WHERE email=? AND code=?", l);
		(*jsonResponse)["createdUserName"] = ret.value("createdUserName").toString();
		return true;
	} else {
		setServerError();
		return false;
	}
}


/**
 * @brief UserInfo::getResources
 * @param jsonResponse
 * @return
 */

bool UserInfo::getResources(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap resources = m_client->server()->resources();

	(*jsonResponse) = QJsonObject::fromVariantMap(resources);

	return true;
}


/**
 * @brief UserInfo::downloadFile
 * @param jsonResponse
 * @param binaryResponse
 * @return
 */

bool UserInfo::downloadFile(QJsonObject *jsonResponse, QByteArray *binaryResponse)
{
	QJsonObject data = m_message.jsonData();
	QVariantMap resources = m_client->server()->resources();

	QString filename = data.value("filename").toString();

	if (filename.isEmpty() || !resources.contains(filename)) {
		(*jsonResponse)["error"] = "invalid filename";
		return false;
	}

	QString md5;
	(*binaryResponse) = m_client->server()->resourceContent(filename, &md5);

	(*jsonResponse)["filename"] = filename;
	(*jsonResponse)["md5"] = md5;

	return true;
}




/**
 * @brief UserInfo::emailRegistration
 * @param email
 * @param firstname
 * @param lastname
 * @param code
 * @return
 */


bool UserInfo::emailRegistration(const QString &email, const QString &firstname, const QString &lastname, const QString &code)
{
	SmtpClient smtp;
	QString serverName;
	QString serverEmail;

	if (!m_client->emailSmptClient("registration", &smtp, &serverName, &serverEmail))
		return false;


	MimeMessage message;

	message.setSender(new EmailAddress(serverEmail, serverName));
	message.addRecipient(new EmailAddress(email, firstname+" "+lastname));
	message.setSubject(tr("Call of Suli regisztráció"));

	MimeText text;

	text.setText(QString("Kedves %1!\n\n"
						 "A(z) %2 szerverre a(z) %3 címmel regisztráltál.\n"
						 "A regisztráció aktiválásához jelentkezz be a következő ideiglenes jelszóval:\n\n"
						 "%4\n\n"
						 "Call of Suli")
				 .arg(lastname)
				 .arg(serverName)
				 .arg(email)
				 .arg(code)
				 );

	message.addPart(&text);

	smtp.sendMail(message);
	smtp.quit();

	qInfo().noquote() << tr("Regisztrációs kód elküldve: ") << email;

	return true;
}
