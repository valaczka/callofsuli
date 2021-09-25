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
#include "teacher.h"
#include "student.h"

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
	(*jsonResponse)["serverName"] = QJsonValue::fromVariant(m_client->db()->execSelectQueryOneRow("SELECT serverName from system").value("serverName"));
	(*jsonResponse)["passwordResetEnabled"] = QJsonValue::fromVariant(m_client->db()->execSelectQueryOneRow("SELECT COALESCE(value, false) as v "
																											"FROM settings WHERE key='email.passwordReset'")
																	  .value("v"));


	bool autoRegistration = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='registration.auto'")
							.value("v", false).toBool();

	bool emailRegistration = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='email.registration'")
							 .value("v", false).toBool();

	(*jsonResponse)["registrationEnabled"] = (autoRegistration || emailRegistration);


	(*jsonResponse)["registrationDomains"] = QJsonArray::fromStringList(m_client->emailRegistrationDomainList());

	(*jsonResponse)["ranklist"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id as rankid, name as rankname,"
"level as ranklevel, image as rankimage, xp "
									"FROM rank ORDER BY id"));


	if (m_client->db()->execSelectQueryOneRow("SELECT COALESCE(value, false) as v "
											  "FROM settings WHERE key='registration.class'").value("v").toBool()) {
		(*jsonResponse)["classlist"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id as classid, name as classname FROM class ORDER BY name"));
	}

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

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, active, "
																						"isTeacher, isAdmin, classid, classname, xp, rankid, "
																						"rankname, rankimage, ranklevel, "
																						"nickname, character, picture "
																						"FROM userInfo where username=?", l);

	(*jsonResponse) = QJsonObject::fromVariantMap(m);

	return true;

}




/**
 * @brief UserInfo::getAllUser
 * @return
 */

bool UserInfo::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	(*jsonResponse)["list"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT username, firstname, lastname, active, "
									"isTeacher, isAdmin, classid, classname, xp, rankid, rankname, ranklevel, rankimage, nickname "
									"FROM userInfo WHERE active=true"));

	(*jsonResponse)["classlist"] = QJsonArray::fromVariantList(m_client->db()->execSelectQuery("SELECT id as classid, name as classname "
									"FROM class"));

	return true;
}



/**
 * @brief UserInfo::getUserScore
 * @param jsonResponse
 * @return
 */

bool UserInfo::getUserScore(QJsonObject *jsonResponse, QByteArray *)
{
	QString username = m_message.jsonData().value("username").toString();
	if (username.isEmpty()) {
		(*jsonResponse)["error"] = "username empty";
		return false;
	}


	QVariantList l;
	l << username;

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, active, "
																						"isTeacher, isAdmin, classid, classname, xp, rankid, "
																						"rankname, rankimage, ranklevel, "
																						"nickname, character, picture "
																						"FROM userInfo where username=?", l);

	m.insert(m_client->db()->execSelectQueryOneRow("SELECT (SELECT COALESCE(MAX(maxStreak), 0) FROM score WHERE score.username=u.user) as longestStreak, "
			"(SELECT COALESCE(MAX(maxStreak), 0) FROM score) as maxStreak, "
			"(SELECT COALESCE(MAX(xp), 0) FROM userInfo) as maxXP, "
			"(SELECT COALESCE(MAX(t1), 0) FROM fullTrophy) as maxT1, "
			"(SELECT COALESCE(MAX(t2), 0) FROM fullTrophy) as maxT2, "
			"(SELECT COALESCE(MAX(t3), 0) FROM fullTrophy) as maxT3, "
			"(SELECT COALESCE(MAX(d1), 0) FROM fullTrophy) as maxD1, "
			"(SELECT COALESCE(MAX(d2), 0) FROM fullTrophy) as maxD2, "
			"(SELECT COALESCE(MAX(d3), 0) FROM fullTrophy) as maxD3, "
			"t1, t2, t3, d1, d2, d3, sumxp "
			"FROM (SELECT ? as user) u LEFT JOIN fullTrophy ON (fullTrophy.username=u.user)", l));

	l << username;

	int currentStreak = m_client->db()->execSelectQueryOneRow("SELECT streak FROM "
		"(SELECT MAX(dt) as dt, COUNT(*) as streak FROM "
		"(SELECT t1.dt as dt, date(t1.dt,-(select count(*) FROM "
		"(SELECT DISTINCT date(timestamp) AS dt "
		"FROM game WHERE username=? AND success=true) t2 "
		"WHERE t2.dt<=t1.dt)||' day', 'localtime') as grp FROM "
		"(SELECT DISTINCT date(timestamp, 'localtime') AS dt FROM game "
		"WHERE username=? AND success=true) t1) t GROUP BY grp) "
		"WHERE dt=date('now', 'localtime')", l)
						 .value("streak", 0).toInt();


	// Ha ma még nem volt megoldás, akkor a tegnapit számoljuk
	if (currentStreak == 0)
		currentStreak = m_client->db()->execSelectQueryOneRow("SELECT streak FROM "
				"(SELECT MAX(dt) as dt, COUNT(*) as streak FROM "
				"(SELECT t1.dt as dt, date(t1.dt,-(select count(*) FROM "
				"(SELECT DISTINCT date(timestamp) AS dt "
				"FROM game WHERE username=? AND success=true) t2 "
				"WHERE t2.dt<=t1.dt)||' day', 'localtime') as grp FROM "
				"(SELECT DISTINCT date(timestamp, 'localtime') AS dt FROM game "
				"WHERE username=? AND success=true) t1) t GROUP BY grp) "
				"WHERE dt=date('now', '-1 day', 'localtime')", l)
								 .value("streak", 0).toInt();

	m["currentStreak"] = currentStreak;

	(*jsonResponse) = QJsonObject::fromVariantMap(m);

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
	int classid = -1;

	if (email.isEmpty()) {
		(*jsonResponse)["error"] = "email empty";
		return false;
	}

	QVariantList l;
	l << email;

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT EXISTS(SELECT * FROM user WHERE username=?) as e", l);

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

	if (m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='registration.class'")
		.value("v", false).toBool()) {
		classid = m_message.jsonData().value("classid").toInt(-1);
	}


	bool autoRegistration = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='registration.auto'")
							.value("v", false).toBool();

	if (autoRegistration) {
		QJsonObject obj;
		obj["username"] = email;
		obj["firstname"] = firstname;
		obj["lastname"] = lastname;
		obj["active"] = true;
		obj["classid"] = classid;

		qInfo().noquote() << tr("Auto-register user") << email;

		CosMessage m2(obj, CosMessage::ClassInvalid, "");

		QJsonObject ret;
		Admin u(m_client, m2);
		bool isSuccess = u.userCreate(&ret, nullptr);

		if (isSuccess) {
			(*jsonResponse)["createdUserName"] = ret.value("createdUserName").toString();
			return true;
		} else {
			setServerError();
			return false;
		}

	} else {

		QVariantMap ins;
		ins["email"] = email;
		ins["firstname"] = firstname;
		ins["lastname"] = lastname;
		if (classid > 0)
			ins["classid"] = classid;
		else
			ins["classid"] = QVariant::Invalid;

		int rowId = m_client->db()->execInsertQuery("INSERT OR REPLACE INTO registration (?k?) VALUES (?)", ins);

		if (rowId == -1) {
			setServerError();
			return false;
		}

		QVariantList ll;
		ll << rowId;

		QString code = m_client->db()->execSelectQueryOneRow("SELECT code FROM registration WHERE id=?", ll).value("code").toString();

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
	m = m_client->db()->execSelectQueryOneRow("SELECT firstname, lastname, classid FROM registration WHERE email=? and code=?", l);
	if (m.isEmpty()) {
		(*jsonResponse)["error"] = "invalid email or code";
		return false;
	}


	QJsonObject obj;
	obj["username"] = email;
	obj["firstname"] = m.value("firstname").toString();
	obj["lastname"] = m.value("lastname").toString();
	obj["active"] = true;

	int classid = m.value("classid", -1).toInt();
	if (classid != -1) {
		obj["classid"] = classid;
	}

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
 * @brief UserInfo::downloadMap
 * @param jsonResponse
 * @param binaryResponse
 * @return
 */

bool UserInfo::downloadMap(QJsonObject *jsonResponse, QByteArray *binaryResponse)
{
	QJsonObject data = m_message.jsonData();
	QString uuid = data.value("uuid").toString();

	QVariantList m;
	m.append(uuid);

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT data, md5, name, version, lastModified FROM maps WHERE uuid=?", m);

	if (r.isEmpty()) {
		(*jsonResponse)["error"] = "invalid map";
		return false;
	}

	(*binaryResponse) = r.value("data").toByteArray();

	(*jsonResponse)["uuid"] = uuid;
	(*jsonResponse)["md5"] = r.value("md5").toString();
	(*jsonResponse)["name"] = r.value("name").toString();
	(*jsonResponse)["version"] = r.value("version").toInt();
	(*jsonResponse)["lastModified"] = r.value("lastModified").toString();

	return true;
}


/**
 * @brief UserInfo::getMyGroups
 * @param jsonResponse
 * @return
 */

bool UserInfo::getMyGroups(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray list;

	CosMessage empty(QJsonObject(), CosMessage::ClassInvalid, "");

	if (m_client->clientRoles().testFlag(CosMessage::RoleTeacher)) {
		QJsonObject ret;
		Teacher u(m_client, empty);
		if (u.groupListGet(&ret, nullptr)) {
			list = ret.value("list").toArray();
		} else {
			qWarning().noquote() << "Teacher group list get error";
		}
	} else if (m_client->clientRoles().testFlag(CosMessage::RoleStudent)) {
		QJsonObject ret;
		Student u(m_client, empty);
		if (u.groupListGet(&ret, nullptr)) {
			list = ret.value("list").toArray();
		} else {
			qWarning().noquote() << "Student group list get error";
		}
	}

	(*jsonResponse)["list"] = list;

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
