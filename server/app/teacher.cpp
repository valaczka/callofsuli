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
			Grading g(m.value("id", -1).toInt(), Grading::TypeGrade, m.value("value", 0).toInt(),
					  criteriaObject, ref, m.value("success", false).toBool());
			ret.append(g);
		} else if (!m.value("xp").isNull()) {
			Grading g(m.value("id", -1).toInt(), Grading::TypeXP, m.value("xp", 0).toInt(),
					  criteriaObject, -1, m.value("success", false).toBool());
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
 * @brief Teacher::autoFinishCampaigns
 * @return
 */

int Teacher::startAndFinishCampaigns(CosDb *db)
{
	int ret = 0;

	Q_ASSERT(db);

	qDebug() << "Start and finish campaigns";

	// Finish

	QVariantList list = db->execSelectQuery("SELECT id FROM campaign WHERE finished=false AND endtime<=datetime('now')");

	foreach (QVariant v, list) {
		QVariantMap m = v.toMap();
		int id = m.value("id").toInt();

		if (!finishCampaign(db, id)) {
			qCritical() << tr("Campaign finish error") << id;
			return ret;
		}

		++ret;
	}



	// Start

	QVariantList list2 = db->execSelectQuery("SELECT id FROM campaign WHERE started=false AND starttime<=datetime('now')");

	foreach (QVariant v, list2) {
		QVariantMap m = v.toMap();
		int id = m.value("id").toInt();

		if (!startCampaign(db, id)) {
			qCritical() << tr("Campaign start error") << id;
			return ret;
		}

		++ret;
	}

	return ret;
}


/**
 * @brief Teacher::finishCampaign
 * @param db
 * @param campaignId
 * @param groupId
 * @return
 */

bool Teacher::finishCampaign(CosDb *db, const int &campaignId)
{
	Q_ASSERT (db);

	const QVariantMap m = db->execSelectQueryOneRow("SELECT groupid, mapclose FROM campaign WHERE id=?", {campaignId});
	const int groupid = m.value("groupid").toInt();

	qInfo() << tr("Hadjárat (%1) automatikus zárása").arg(campaignId);

	if (!db->execSimpleQuery("UPDATE campaign SET finished=true WHERE id=?", {campaignId}))
		return false;


	// BindGroupMap close


	QVariantList mapList;
	foreach (QString s, m.value("mapclose").toString().split('|', Qt::SkipEmptyParts))
		mapList.append(s);

	if (!mapList.isEmpty()) {
		QVariantMap p;
		p[":groupid"] = groupid;
		if (!db->execListQuery("UPDATE bindGroupMap SET active=false WHERE groupid=:groupid AND mapid IN(?l?)", mapList, p)) {
			qCritical() << tr("Map close error");
			return false;
		}
	}


	// Grading

	const QVariantList userList = db->execSelectQuery("SELECT username FROM studentGroupInfo WHERE id=?", {groupid});
	QVariantList assList = db->execSelectQuery("SELECT id FROM assignment WHERE campaignid=?", {campaignId});

	foreach (QVariant v, userList) {
		const QString username = v.toMap().value("username").toString();

		foreach (const QVariant &av, assList) {
			const int assignmentId = av.toMap().value("id").toInt();

			const QVariantList gradingList = db->execSelectQuery("SELECT grading.id as id, gradeid, value, xp, criteria "
																 "FROM grading LEFT JOIN grade ON (grade.id=grading.gradeid) "
																 "WHERE assignmentid=? ORDER BY grading.id", {assignmentId});

			QVector<Grading> grading = gradingFromVariantList(gradingList);

			evaluate(db, grading, campaignId, username);


			// Értékelések rögzítése

			foreach (Grading g, grading) {
				if (!g.isValid() || g.id == -1)
					continue;

				QVariantMap p;
				p["username"] = username;
				p["gradingid"] = g.id;
				p["success"] = g.success;
				db->execInsertQuery("INSERT INTO gradingResult(?k?) VALUES (?)", p);
			}


			Grading g1 = gradingResult(grading, Teacher::Grading::TypeGrade);
			Grading g2 = gradingResult(grading, Teacher::Grading::TypeXP);

			if (g1.isValid()) {
				QVariantMap p;
				p["username"] = username;
				p["groupid"] = groupid;
				p["assignmentid"] = assignmentId;
				p["gradeid"] = g1.ref;
				if (db->execInsertQuery("INSERT INTO gradeBook(?k?) VALUES (?)", p) == -1) {
					qCritical() << tr("Grading error");
					return false;
				}

				qInfo() << tr("Értékelés: %1 -> JEGY (id: %2)").arg(username).arg(g1.ref);
			}

			if (g2.isValid()) {
				QVariantMap p;
				p["username"] = username;
				p["xp"] = g2.value;
				p["assignmentid"] = assignmentId;
				if (db->execInsertQuery("INSERT INTO score(?k?) VALUES (?)", p) == -1) {
					qCritical() << tr("Grading error");
					return false;
				}

				qInfo() << tr("Értékelés: %1 -> %2 XP").arg(username).arg(g2.value);
			}
		}

	}

	return true;

}



/**
 * @brief Teacher::startCampaign
 * @param db
 * @param campaignId
 * @return
 */

bool Teacher::startCampaign(CosDb *db, const int &campaignId)
{
	Q_ASSERT (db);

	qInfo() << tr("Hadjárat (%1) automatikus megnyitása").arg(campaignId);

	if (!db->execSimpleQuery("UPDATE campaign SET started=true WHERE id=?", {campaignId}))
		return false;

	QVariantMap m = db->execSelectQueryOneRow("SELECT groupid, mapopen FROM campaign WHERE id=?", {campaignId});
	const int groupid = m.value("groupid").toInt();

	foreach (QString s, m.value("mapopen").toString().split('|', Qt::SkipEmptyParts)) {
		QVariantMap p;
		p["groupid"] = groupid;
		p["mapid"] = s;
		p["active"] = true;

		if (db->execInsertQuery("INSERT OR REPLACE INTO bindGroupMap (?k?) VALUES (?)", p) == -1) {
			qCritical() << tr("Map open error");
			return false;
		}
	}


	return true;
}



/**
 * @brief Teacher::gradingGet
 * @param assignmentId
 * @param campaignId
 * @param username
 * @return
 */

QVector<Teacher::Grading> Teacher::gradingGet(const int &assignmentId, const int &campaignId, const QString &username, const bool &isFinished)
{
	QVariantList gradingList;

	if (isFinished)
		gradingList = m_client->db()->execSelectQuery("SELECT grading.id as id, gradeid, value, xp, criteria, success "
													  "FROM grading LEFT JOIN grade ON (grade.id=grading.gradeid) "
													  "LEFT JOIN gradingResult ON (gradingResult.gradingid=grading.id AND gradingResult.username=?) "
													  "WHERE assignmentid=? ORDER BY grading.id", {username, assignmentId});

	else
		gradingList = m_client->db()->execSelectQuery("SELECT grading.id as id, gradeid, value, xp, criteria "
													  "FROM grading LEFT JOIN grade ON (grade.id=grading.gradeid) "
													  "WHERE assignmentid=? ORDER BY grading.id", {assignmentId});


	QVector<Grading> grading = gradingFromVariantList(gradingList);

	if (!isFinished && !username.isEmpty())
		evaluate(m_client->db(), grading, campaignId, username);

	return grading;
}





/**
 * @brief Teacher::evaluate
 * @param list
 * @param campaignId
 * @param username
 * @return
 */

QVector<Teacher::Grading> &Teacher::evaluate(CosDb *db, QVector<Grading> &list, const int &campaignId, const QString &username)
{
	Q_ASSERT(db);

	for (int i=0; i<list.size(); ++i) {
		Grading &g = list[i];
		if (!g.success)
			evaluate(db, g, campaignId, username);
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

Teacher::Grading &Teacher::evaluate(CosDb *db, Grading &grading, const int &campaignId, const QString &username)
{
	Q_ASSERT(db);

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

		QVariantMap m = db->execSelectQueryOneRow("SELECT SUM(xp) as xp FROM campaignTrophy "
												  "WHERE id=? AND username=?",
												  {campaignId, username});

		int xp = m.value("xp", 0).toInt();

		if (xp>=targetXP)
			grading.success = true;
	} else if (module == "trophy") {
		int target = criteria.value("value").toInt(0);
		if (target < 1) {
			grading.success = true;
			return grading;
		}

		QVariantMap m = db->execSelectQueryOneRow("SELECT SUM(num) as trophy FROM campaignTrophy "
												  "WHERE id=? AND username=? AND success=true",
												  {campaignId, username});

		int trophy = m.value("trophy", 0).toInt();

		if (trophy >= target)
			grading.success = true;
	} else if (module == "sumlevel") {
		int target = criteria.value("value").toInt(0);
		if (target < 1) {
			grading.success = true;
			return grading;
		}

		int level = criteria.value("level").toInt(0);
		bool deathmatch = criteria.value("deathmatch").toBool(false);

		QVariantMap m = db->execSelectQueryOneRow("SELECT COUNT(*) as cnt FROM campaignTrophy "
												  "WHERE id=? AND username=? AND success=true AND level=? AND deathmatch=?",
												  {campaignId, username, level, deathmatch});

		int cnt = m.value("cnt", 0).toInt();

		if (cnt >= target)
			grading.success = true;
	} else if (module == "missionlevel") {
		int level = criteria.value("level").toInt(0);
		bool deathmatch = criteria.value("deathmatch").toBool(false);
		QString map = criteria.value("map").toString();
		QString mission = criteria.value("mission").toString();

		QVariantMap m = db->execSelectQueryOneRow("SELECT SUM(num) as trophy FROM campaignTrophy "
												  "WHERE id=? AND username=? AND mapid=? AND missionid=? AND success=true "
												  "AND level=? AND deathmatch=?",
												  {campaignId, username, map, mission, level, deathmatch});

		int trophy = m.value("trophy", 0).toInt();

		if (trophy > 0)
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

	QJsonArray cList = m_client->db()->execSelectQueryJson("SELECT classid, name FROM bindGroupClass "
														   "LEFT JOIN class ON (class.id=bindGroupClass.classid) WHERE groupid=?",
														   {id});

	QJsonArray uList = m_client->db()->execSelectQueryJson("SELECT userInfo.username as username, firstname, lastname, nickname, "
														   "rankid, COALESCE(ranklevel, -1) as ranklevel, rankimage, picture, xp, "
														   "active, COALESCE(classid, -1) as classid, classname "
														   "FROM bindGroupStudent LEFT JOIN userInfo ON (userInfo.username=bindGroupStudent.username) WHERE groupid=?",
														   {id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["name"] = group.value("name").toString();
	(*jsonResponse)["classList"] = cList;
	(*jsonResponse)["userList"] = uList;

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

	QJsonArray list = m_client->db()->execSelectQueryJson("SELECT username, firstname, lastname, "
														  "active, COALESCE(classid, -1) as classid, classname "
														  "FROM userInfo "
														  "WHERE isTeacher=false AND username NOT IN "
														  "(SELECT username FROM bindGroupStudent WHERE groupid=?) ",
														  {id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["list"] = list;

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

	QJsonArray list = m_client->db()->execSelectQueryJson("SELECT id as classid, name FROM class WHERE id NOT IN "
														  "(SELECT classid FROM bindGroupClass WHERE groupid=?) ",
														  {id});

	(*jsonResponse)["id"] = id;
	(*jsonResponse)["list"] = list;

	return true;
}










/**
 * @brief TeacherMap::groupListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::groupListGet(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray mapList = m_client->db()->execSelectQueryJson("SELECT id, name, (SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
															 "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
															 "WHERE bindGroupClass.groupid=studentgroup.id) as readableClassList FROM studentgroup "
															 "WHERE owner=?",
															 {m_client->clientUserName()});


	(*jsonResponse)["list"] = mapList;

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

	QJsonArray cList = m_client->db()->execSelectQueryJson("SELECT classid, name FROM bindGroupClass "
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
	(*jsonResponse)["classList"] = cList;
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

	QJsonArray tList = m_client->db()->execSelectQueryJson(query, queryParams);

	(*jsonResponse)["list"] = tList;



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

		QJsonArray uList = m_client->db()->execSelectQueryJson(query, queryParams);
		(*jsonResponse)["users"] = uList;
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

		o["binded"] = m_client->db()->execSelectQueryJson("SELECT groupid, name, active, "
														  "(SELECT GROUP_CONCAT(name, ', ') FROM bindGroupClass "
														  "LEFT JOIN class ON (class.id=bindGroupClass.classid) "
														  "WHERE bindGroupClass.groupid=bindGroupMap.groupid) as readableClassList "
														  "FROM bindGroupMap "
														  "LEFT JOIN studentgroup ON (studentgroup.id=bindGroupMap.groupid) "
														  "WHERE mapid=? AND owner=?", {uuid, m_client->clientUserName()});
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
 * @brief Teacher::gameListCampaignGet
 * @param jsonReponse
 * @return
 */

bool Teacher::gameListCampaignGet(QJsonObject *jsonResponse, QByteArray *)
{
	QVariantMap params = m_message.jsonData().toVariantMap();
	int campaignid = params.value("campaignid", -1).toInt();

	if (campaignid == -1) {
		(*jsonResponse)["error"] = "missing campaignid";
		return false;
	}

	int limit = params.value("limit", 50).toInt();
	int offset = params.value("offset", 0).toInt();

	QVariantList list;

	if (params.contains("username")) {
		QString username = params.value("username").toString();
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, game.username, firstname, lastname, nickname, "
											   "mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, level, success, "
											   "deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN userInfo ON (userInfo.username=game.username) "
											   "LEFT JOIN score ON (score.gameid=game.id) "
											   "WHERE tmpScore is NULL AND "
											   "game.username=? AND "
											   "game.timestamp>=(SELECT starttime FROM campaign WHERE id=?) AND "
											   "game.timestamp<(SELECT endtime FROM campaign WHERE id=?) AND "
											   "game.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=(SELECT groupid FROM campaign WHERE id=?))) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {username, campaignid, campaignid, campaignid, offset, limit});
		(*jsonResponse)["username"] = username;

	} else {
		list = m_client->db()->execSelectQuery("WITH t AS (SELECT game.rowid, "
											   "mapid, missionid, datetime(game.timestamp, 'localtime') as timestamp, level, success, "
											   "deathmatch, lite, duration, score.xp FROM game "
											   "LEFT JOIN score ON (score.gameid=game.id) "
											   "WHERE tmpScore is NULL AND "
											   "game.timestamp>=(SELECT starttime FROM campaign WHERE id=?) AND "
											   "game.timestamp<(SELECT endtime FROM campaign WHERE id=?) AND "
											   "game.mapid IN (SELECT mapid FROM bindGroupMap WHERE groupid=(SELECT groupid FROM campaign WHERE id=?))) "
											   "SELECT * FROM t WHERE rowid NOT IN (SELECT rowid FROM t ORDER BY timestamp DESC LIMIT ?) "
											   "ORDER BY timestamp DESC LIMIT ?",
											   {campaignid, campaignid, campaignid, offset, limit});
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


	(*jsonResponse)["groupid"] = params.value("groupid", -1).toInt();
	(*jsonResponse)["offset"] = offset;
	(*jsonResponse)["list"] = ret;
	(*jsonResponse)["campaignid"] = campaignid;

	return true;
}



/**
 * @brief Teacher::campaignGet
 * @param jsonResponse
 * @return
 */

bool Teacher::campaignGet(QJsonObject *jsonResponse, QByteArray *)
{
	const QVariantMap params = m_message.jsonData().toVariantMap();
	const int id = params.value("id", -1).toInt();


	// Grades

	(*jsonResponse)["gradeList"] = m_client->db()->execSelectQueryJson("SELECT id, shortname, longname, value FROM grade");


	// Maps

	(*jsonResponse)["mapList"] = m_client->mapsDb()->execSelectQueryJson("SELECT uuid, name, version FROM maps WHERE owner=?",
																		 {m_client->clientUserName()});

	if (id == -1) {
		(*jsonResponse)["error"] = "missing campaign";
		return false;
	}

	QVariantMap d = m_client->db()->execSelectQueryOneRow("SELECT datetime(starttime, 'localtime') as starttime, "
														  "datetime(endtime, 'localtime') as endtime, "
														  "started, finished, description, mapopen, mapclose FROM campaign "
														  "INNER JOIN studentgroup ON (studentgroup.id=campaign.groupid) "
														  "WHERE campaign.id=? AND owner=?", {id, m_client->clientUserName()});

	(*jsonResponse)["id"] = id;

	if (d.isEmpty()) {
		(*jsonResponse)["error"] = "invalid id";
		return false;
	}

	(*jsonResponse)["starttime"] = d.value("starttime").toString();
	(*jsonResponse)["endtime"] = d.value("endtime").toString();
	(*jsonResponse)["started"] = d.value("started").toBool();
	(*jsonResponse)["finished"] = d.value("finished").toBool();
	(*jsonResponse)["description"] = d.value("description").toString();

	QJsonArray ao, ac;
	foreach (QString s, d.value("mapopen").toString().split('|', Qt::SkipEmptyParts))
		ao.append(s);
	foreach (QString s, d.value("mapclose").toString().split('|', Qt::SkipEmptyParts))
		ac.append(s);

	(*jsonResponse)["mapopen"] = ao;
	(*jsonResponse)["mapclose"] = ac;


	// Campaigns

	QJsonArray assList = m_client->db()->execSelectQueryJson("SELECT id, name FROM assignment WHERE campaignid=?", {id});
	QJsonArray assRetList;

	foreach (const QJsonValue &v, assList) {
		QJsonObject ass = v.toObject();
		const int aid = ass.value("id").toInt();
		ass["gradingList"] = Grading::toArray(gradingGet(aid, id));
		assRetList.append(ass);
	}

	(*jsonResponse)["assignmentList"] = assRetList;


	return true;
}




/**
 * @brief Teacher::campaignListGet
 * @param jsonResponse
 * @return
 */

bool Teacher::campaignListGet(QJsonObject *jsonResponse, QByteArray *)
{
	const QVariantMap params = m_message.jsonData().toVariantMap();
	const int groupid = params.value("groupid", -1).toInt();

	if (groupid == -1) {
		(*jsonResponse)["error"] = "missing group";
		return false;
	}

	QJsonArray list = m_client->db()->execSelectQueryJson("SELECT campaign.id, datetime(starttime, 'localtime') as starttime, "
														  "datetime(endtime, 'localtime') as endtime, "
														  "started, finished, description FROM campaign "
														  "INNER JOIN studentgroup ON (studentgroup.id=campaign.groupid) "
														  "WHERE groupid=? AND owner=?", {groupid, m_client->clientUserName()});

	(*jsonResponse)["list"] = list;



	return true;
}





/**
 * @brief Teacher::Grading::Grading
 * @param t
 * @param v
 * @param c
 * @param r
 */

Teacher::Grading::Grading(const int &i, const Type &t, const int &v, const QJsonObject &c, const int &r, const bool &s) :
	type(t), value(v), ref(r), mode(ModeInvalid), criteria(c), success(s), id(i)
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

		o["id"] = g.id;
		o["type"] = g.type == Grading::TypeXP ? "xp" : "grade";
		o["value"] = g.value;
		o["ref"] = g.ref;
		if (g.mode == Grading::ModeDefault)
			o["mode"] = "default";
		else if (g.mode == Grading::ModeRequired)
			o["mode"] = "required";
		o["criteria"] = g.criteria;
		//o["success"] = g.success;

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
										 { "criterion", g.criteria },
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
										 { "criterion", g.criteria },
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
