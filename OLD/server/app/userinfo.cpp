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
 * @brief UserInfo::startHttpReply
 * @param reply
 */

void UserInfo::startHttpReply(QNetworkReply *reply, void *data)
{
	if (m_message.cosFunc() == "registrationRequest")
		onOAuth2UserinfoReply(reply, data);

}


/**
 * @brief UserInfo::getServer
 * @return
 */

bool UserInfo::getServerInfo(QJsonObject *jsonResponse, QByteArray *)
{
	(*jsonResponse)["serverName"] = QJsonValue::fromVariant(m_client->db()->execSelectQueryOneRow("SELECT serverName from system").value("serverName"));
	(*jsonResponse)["serverUuid"] = QJsonValue::fromVariant(m_client->db()->execSelectQueryOneRow("SELECT serverUuid from system").value("serverUuid"));

	(*jsonResponse)["registrationEnabled"] = QJsonValue::fromVariant(m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='registration.enabled'").value("v", false).toBool());


	(*jsonResponse)["ranklist"] = m_client->db()->execSelectQueryJson("SELECT id as rankid, name as rankname,"
																	  "COALESCE(level, -1) as ranklevel, image as rankimage, xp "
																	  "FROM rank ORDER BY id");



	QString googleId = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.googleID'")
					   .value("v").toString();
	QString googleKey = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.googleKey'")
						.value("v").toString();

	if (!googleId.isEmpty() && !googleKey.isEmpty()) {
		(*jsonResponse)["googleOAuth2id"] = googleId;
		(*jsonResponse)["googleOAuth2key"] = googleKey;
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

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, "
														  "isTeacher, isAdmin, COALESCE(classid, -1) as classid, classname, xp, rankid, "
														  "rankname, rankimage, COALESCE(ranklevel, -1) as ranklevel, "
														  "nickname, character, picture "
														  "FROM userInfo WHERE active IS TRUE AND username=?", {username});


	// Van-e dolgozat

	m["examEngineExists"] = m_client->server()->examEnginesHasMember(username);


	if (!m_message.jsonData().value("ping").toBool(false)) {

		m["maxStreak"] = m_client->db()->execSelectQueryOneRow("SELECT COALESCE(MAX(maxStreak), 0) as maxStreak "
															   "FROM score WHERE username=?", {username}).value("maxStreak").toInt();

		int currentStreak = m_client->db()->execSelectQueryOneRow("SELECT streak FROM "
																  "(SELECT MAX(dt) as dt, COUNT(*) as streak FROM "
																  "(SELECT t1.dt as dt, date(t1.dt,-(select count(*) FROM "
																  "(SELECT DISTINCT date(timestamp) AS dt "
																  "FROM game WHERE username=? AND success=true) t2 "
																  "WHERE t2.dt<=t1.dt)||' day', 'localtime') as grp FROM "
																  "(SELECT DISTINCT date(timestamp, 'localtime') AS dt FROM game "
																  "WHERE username=? AND success=true) t1) t GROUP BY grp) "
																  "WHERE dt=date('now', 'localtime')", {username, username})
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
																  "WHERE dt=date('now', '-1 day', 'localtime')", {username, username})
							.value("streak", 0).toInt();

		m["currentStreak"] = currentStreak;

		if (m_message.jsonData().contains("withTrophy")) {
			m.insert(m_client->db()->execSelectQueryOneRow("SELECT (SELECT COALESCE(MAX(maxStreak), 0) FROM score WHERE score.username=?) as longestStreak, "
														   "t1, t2, t3, d1, d2, d3 FROM fullTrophy WHERE username=?", {username, username}));
		}




		if (m_message.jsonData().contains("withRanklog")) {
			QJsonArray rList = m_client->db()->execSelectQueryJson("SELECT rankid, datetime(timestamp, 'localtime') as timestamp, ranklog.xp, name, level, image,"
																   "-1 as maxStreak "
																   "FROM ranklog LEFT JOIN rank ON (rank.id=ranklog.rankid) "
																   "WHERE username=? "
																   "UNION "
																   "SELECT -1 as rankid, datetime(timestamp, 'localtime') as timestamp, -1 as xp, '' as name, "
																   "-1 as level, '' as image, maxStreak "
																   "FROM score WHERE username=? AND maxStreak IS NOT NULL", {username, username});
			m["ranklog"] = rList;
		}


		m["nameModificationDisabled"] = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='user.disableNameModification'")
										.value("v", false).toBool();

		m["oauth2Account"] = m_client->db()->execSelectQueryOneRow("SELECT (password='*') as v FROM auth WHERE username=?", {username})
							 .value("v", false).toBool();

	}

	(*jsonResponse) = QJsonObject::fromVariantMap(m);

	return true;

}




/**
 * @brief UserInfo::getAllUser
 * @return
 */

bool UserInfo::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	(*jsonResponse)["list"] = m_client->db()->execSelectQueryJson("SELECT username, firstname, lastname, active, "
																  "isTeacher, isAdmin, COALESCE(classid, -1) as classid, classname, xp, rankid, rankname, COALESCE(ranklevel, -1) as ranklevel, rankimage, nickname, picture "
																  "FROM userInfo WHERE active=true");

	(*jsonResponse)["classlist"] = m_client->db()->execSelectQueryJson("SELECT id as classid, name as classname "
																	   "FROM class");

	return true;
}



/**
 * @brief UserInfo::getUserScore
 * @param jsonResponse
 * @return
 */

bool UserInfo::getUserScore(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();

	if (!params.contains("withTrophy")) {
		params["withTrophy"] = true;
		m_message.setJsonData(params);
	}

	if (!getUser(jsonResponse, nullptr))
		return false;

	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT "
														  "(SELECT COALESCE(MAX(maxStreak), 0) FROM score) as maxStreak, "
														  "(SELECT COALESCE(MAX(xp), 0) FROM userInfo) as maxXP, "
														  "(SELECT COALESCE(MAX(t1), 0) FROM fullTrophy) as maxT1, "
														  "(SELECT COALESCE(MAX(t2), 0) FROM fullTrophy) as maxT2, "
														  "(SELECT COALESCE(MAX(t3), 0) FROM fullTrophy) as maxT3, "
														  "(SELECT COALESCE(MAX(d1), 0) FROM fullTrophy) as maxD1, "
														  "(SELECT COALESCE(MAX(d2), 0) FROM fullTrophy) as maxD2, "
														  "(SELECT COALESCE(MAX(d3), 0) FROM fullTrophy) as maxD3");

	(*jsonResponse)["maxStreak"] = m.value("maxStreak").toInt();
	(*jsonResponse)["maxXP"] = m.value("maxXP").toInt();
	(*jsonResponse)["maxT1"] = m.value("maxT1").toInt();
	(*jsonResponse)["maxT2"] = m.value("maxT2").toInt();
	(*jsonResponse)["maxT3"] = m.value("maxT3").toInt();
	(*jsonResponse)["maxD1"] = m.value("maxD1").toInt();
	(*jsonResponse)["maxD2"] = m.value("maxD2").toInt();
	(*jsonResponse)["maxD3"] = m.value("maxD3").toInt();

	return true;
}


/**
 * @brief UserInfo::registrationRequest
 * @param jsonResponse
 */

bool UserInfo::registrationRequest(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonObject params = m_message.jsonData();

	QString username = params.value("username").toString();
	QString firstname = params.value("firstname").toString();
	QString lastname = params.value("lastname").toString();
	QString token = params.value("oauthToken").toString();
	QString code = params.value("code").toString();
	QString password = params.value("password").toString();
	QString refreshToken = params.value("oauthRefreshToken").toString();
	QDateTime expiration;

	const QString dateTimeFormat = "yyyy-MM-dd HH:mm:ss";

	if (params.contains("oauthExpiration"))
		expiration = QDateTime::fromString(params.value("oauthExpiration").toString(), dateTimeFormat);

	qDebug() << "*****" << expiration << refreshToken;


	bool registrationEnabled = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='registration.enabled'")
							   .value("v", false).toBool();
	bool oauth2Enabled = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.registration'")
						 .value("v", false).toBool();
	bool oauth2Forced = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='oauth2.forced'")
						.value("v", false).toBool();



	QVariantMap m = m_client->db()->execSelectQueryOneRow("SELECT value as list FROM settings WHERE key='oauth2.domains'");

	QString s = m.value("list", "").toString();

	QStringList domainList = s.isEmpty() ? QStringList() : s.split(",");
	domainList.replaceInStrings(QRegExp("\\s"), "");


	// Csak regisztrációs információk

	if (username.isEmpty() && token.isEmpty()) {
		(*jsonResponse)["status"] = true;
		(*jsonResponse)["registrationEnabled"] = QJsonValue::fromVariant(registrationEnabled);
		(*jsonResponse)["oauth2Enabled"] = QJsonValue::fromVariant(oauth2Enabled);
		(*jsonResponse)["oauth2Forced"] = QJsonValue::fromVariant(oauth2Forced);

		if (oauth2Enabled)
			(*jsonResponse)["domains"] = QJsonArray::fromStringList(domainList);

		return true;
	}


	// Ha a regisztráció le van tiltva

	if (!registrationEnabled) {
		(*jsonResponse)["error"] = "registration disabled";
		return false;
	}


	// Ha az oauth2 le van tiltva

	if (!oauth2Enabled && !token.isEmpty()) {
		(*jsonResponse)["error"] = "oauth2 registration disabled";
		return false;
	}



	// Ha nincs kényszerített osztály és nincs osztálykód

	int forcedClass = m_client->db()->execSelectQueryOneRow("SELECT value as v FROM settings WHERE key='registration.forced'").value("v", -1).toInt();

	if (forcedClass <= 0 && code.isEmpty()) {
		(*jsonResponse)["error"] = "missing code";
		return false;
	}



	if (!token.isEmpty()) {
		getOAuth2Userinfo(token, refreshToken, expiration, domainList, code);
		(*jsonResponse)["oauth2"] = true;
		return true;
	} else if (!oauth2Forced) {
		Admin a(m_client, CosMessage());

		QString err = a.userCreateReal(username,
									   firstname,
									   lastname,
									   false,
									   code);

		if (!err.isEmpty()) {
			(*jsonResponse)["error"] = err;
			return false;
		}

		if (!a.userPasswordChangeReal(username, password)) {
			(*jsonResponse)["error"] = "auth error";
			return false;
		} else {
			(*jsonResponse)["created"] = username;
		}
	} else {
		(*jsonResponse)["error"] = "invalid method";
		return false;
	}

	return true;
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

	QVariantMap r = m_client->mapsDb()->execSelectQueryOneRow("SELECT data, md5, name, version, datetime(lastModified, 'localtime') as lastModified "
															  "FROM maps WHERE uuid=?", m);

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
 * @brief UserInfo::getOAuth2Userinfo
 * @param token
 */


void UserInfo::getOAuth2Userinfo(const QString &token, const QString &refreshToken, const QDateTime &expiration,
								 const QStringList &domainList, const QString &classCode)
{
	QUrl url("https://www.googleapis.com/oauth2/v1/userinfo");
	QUrlQuery q;
	q.addQueryItem("alt", "json");
	q.addQueryItem("access_token", token);
	url.setQuery(q);

	OAuth2Data *d = new OAuth2Data;
	d->classCode = classCode;
	d->domainList = domainList;
	d->token = token;
	d->refreshToken = refreshToken;
	d->expiration = expiration;

	m_client->httpGet(QNetworkRequest(url), m_message, d);
}



/**
 * @brief UserInfo::OAuth2UserinfoReply
 * @param reply
 */

void UserInfo::onOAuth2UserinfoReply(QNetworkReply *reply, void *data)
{
	QString classCode;
	QStringList domainList;
	QString token;
	QString refreshToken;
	QDateTime expiration;

	if (data) {
		OAuth2Data *d = static_cast<OAuth2Data*>(data);

		if (d) {
			classCode = d->classCode;
			domainList = d->domainList;
			token = d->token;
			refreshToken = d->refreshToken;
			expiration = d->expiration;

			delete d;
		}
	}


	QByteArray content = reply->readAll();
	QJsonDocument doc = QJsonDocument::fromJson(content);

	qDebug() << "OAUTH2 RESPONSE" << doc;

	if (doc.isObject()) {
		QJsonObject data = doc.object();

		if (data.contains("error")) {
			qWarning() << "Invalid oauth2 response" << data.value("error");
			CosMessage cm(QJsonObject({
										  {"error", data.value("error").toObject().value("message").toString()}
									  }),
						  CosMessage::ClassUserInfo, "registrationRequest");
			cm.send(m_client->socket());
			return;
		}



		QString username = data.value("email").toString();
		QString familyname = data.value("family_name").toString();
		QString givenname = data.value("given_name").toString();
		QString picture = data.value("picture").toString();

		bool match = domainList.count() ? false : true;

		foreach (QString s, domainList) {
			if (username.endsWith(s)) {
				match = true;
				break;
			}
		}


		if (!match) {
			CosMessage cm(QJsonObject({
										  {"error", "invalid domain"}
									  }),
						  CosMessage::ClassUserInfo, "registrationRequest");
			cm.send(m_client->socket());
			return;
		}



		Admin a(m_client, CosMessage());

		QString err = a.userCreateReal(username,
									   familyname,
									   givenname,
									   false,
									   classCode,
									   token,
									   refreshToken,
									   expiration,
									   picture
									   );

		if (!err.isEmpty()) {
			CosMessage cm(QJsonObject({
										  {"error", err}
									  }),
						  CosMessage::ClassUserInfo, "registrationRequest");
			cm.send(m_client->socket());
			return;
		} else {
			CosMessage cm(QJsonObject({
										  {"created", username},
										  {"token", token}
									  }),
						  CosMessage::ClassUserInfo, "registrationRequest");
			cm.send(m_client->socket());
			return;
		}
	}
}
