/*
 * ---- Call of Suli ----
 *
 * gamemapmodel.cpp
 *
 * Created on: 2021. 09. 25.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GameMapModel
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QRandomGenerator>
#include "gamemapmodel.h"
#include "variantmapdata.h"


GameMapModel::GameMapModel(QObject *parent)
	: VariantMapModel({
					  "username",
					  "firstname",
					  "lastname",
					  "active",
					  "xp",
					  "displayData",
					  "userInfo"
					  }, parent)
	, m_missions()
	, m_users()
	, m_levelCount(0)
	, m_missionData()
{
	m_missionData = {
		{ "missions", {} },
		{ "levels", {} }
	};
}


/**
 * @brief GameMapModel::~GameMapModel
 */

GameMapModel::~GameMapModel()
{

}

/**
 * @brief GameMapModel::setGameMap
 * @param map
 */

void GameMapModel::setGameMap(GameMap *map)
{
	m_missions.clear();
	m_missionData.clear();

	if (!map) {
		setlevelCount(0);
		setMissionData({
						   { "missions", {} },
						   { "levels", {} }
					   });
		refreshUsers();
		return;
	}

	QVariantList mdata;
	QVariantList ldata;
	int cols = 0;

	foreach(GameMap::Mission *m, map->missions()) {
		GameMapModel::Mission mis;
		mis.uuid = QString::fromLatin1(m->uuid());
		mis.name = m->name();
		mis.medalImage = m->medalImage();

		int n = 0;

		foreach(GameMap::MissionLevel *ml, m->levels()) {
			GameMapModel::MissionLevel misLevel;
			misLevel.uuid = mis.uuid;
			misLevel.level = ml->level();
			misLevel.deathmatch = false;

			mis.levels.append(misLevel);
			n++;

			if (ml->canDeathmatch()) {
				misLevel.uuid = mis.uuid;
				misLevel.deathmatch = true;
				mis.levels.append(misLevel);
				n++;
			}
		}

		if (n) {
			m_missions.append(mis);
			mdata.append(mis.toMap());

			foreach(GameMapModel::MissionLevel ml, mis.levels)
				ldata.append(ml.toMap());

			cols += n;
		}
	}

	setMissionData({
					   { "missions", mdata },
					   { "levels", ldata }
				   });
	setlevelCount(cols);
	refreshUsers();
}


/**
 * @brief GameMapModel::appendUser
 * @param user
 */


void GameMapModel::appendUser(const QString &username, const GameMapModel::User &user)
{
	m_users[username] = user;
	updateUser(username);
}


/**
 * @brief GameMapModel::appendUser
 * @param username
 * @param firstname
 * @param lastname
 */

void GameMapModel::appendUser(const QString &username, const QString &firstname, const QString &lastname, const bool &active, const int &xp)
{
	GameMapModel::User u;

	u.firstname = firstname;
	u.lastname = lastname;
	u.xp = xp;
	u.active = active;

	appendUser(username, u);
}




/**
 * @brief GameMapModel::setUser
 * @param username
 * @param firstname
 * @param lastname
 * @param xp
 */

void GameMapModel::setUser(const QString &username, const QString &firstname, const QString &lastname, const bool &active, const int &xp)
{
	if (!m_users.contains(username))
		appendUser(username, firstname, lastname, active, xp);
	else {
		m_users[username].firstname = firstname;
		m_users[username].lastname = lastname;
		m_users[username].xp = xp;
		updateUser(username);
	}
}


/**
 * @brief GameMapModel::setUser
 * @param username
 * @param xp
 */

void GameMapModel::setUser(const QString &username, const int &xp)
{
	if (!m_users.contains(username)) {
		qWarning() << "Invalid user" << username;
		return;
	}

	m_users[username].xp = xp;
	updateUser(username);
}



/**
 * @brief GameMapModel::setUser
 * @param data
 */

void GameMapModel::setUser(const QJsonObject &data)
{
	QString username = data.value("username").toString();

	if (username.isEmpty())
		return;

	if (!m_users.contains(username))
		appendUser(username,
				   data.value("firstname").toString(),
				   data.value("lastname").toString(),
				   data.value("active").toBool(),
				   data.value("xp").toInt());


	if (data.contains("xp"))
		m_users[username].xp = data.value("xp").toInt();

	if (data.contains("active"))
		m_users[username].active = data.value("active").toBool();

	if (data.contains("firstname"))
		m_users[username].firstname = data.value("firstname").toString();

	if (data.contains("lastname"))
		m_users[username].lastname = data.value("lastname").toString();


	QVariantMap info = m_users.value(username).userInfo;

	if (data.contains("nickname"))
		info["nickname"] = data.value("nickname").toString();

	if (data.contains("rankid"))
		info["rankid"] = data.value("rankid").toInt();

	if (data.contains("ranklevel"))
		info["ranklevel"] = data.value("ranklevel").toInt();

	if (data.contains("rankimage"))
		info["rankimage"] = data.value("rankimage").toString();

	if (data.contains("picture"))
		info["picture"] = data.value("picture").toString();

	if (data.contains("classname"))
		info["classname"] = data.value("classname").toString();

	if (data.contains("classid"))
		info["classid"] = data.value("classid").toInt();

	if (data.contains("mapid"))
		info["mapid"] = data.value("mapid").toString();

	if (data.contains("missionid"))
		info["missionid"] = data.value("missionid").toString();

	if (data.contains("level"))
		info["level"] = data.value("level").toInt();

	if (data.contains("deathmatch"))
		info["deathmatch"] = data.value("deathmatch").toInt();


	m_users[username].userInfo = info;

	updateUser(username);
}


/**
 * @brief GameMapModel::setUserLevelData
 * @param data
 */
void GameMapModel::setUserLevelData(const QJsonObject &data)
{
	QString username = data.value("username").toString();
	QString uuid = data.value("missionid").toString();
	int level = data.value("level").toInt(-1);
	bool deathmatch = data.value("deathmatch").toInt(0) > 0;
	bool success = data.value("success").toInt(0) > 0;
	int num = data.value("num").toInt();

	if (username.isEmpty() || !m_users.contains(username) || uuid.isEmpty() || level == -1)
		return;

	QVector<MissionLevel> levelData = m_users.value(username).levelData;

	int index = -1;


	for (int i=0; i<levelData.size(); ++i) {
		MissionLevel l = levelData.value(i);
		if (l.uuid == uuid && l.level == level && l.deathmatch == deathmatch) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		MissionLevel d;
		d.success = success;
		d.num = num;
		d.uuid = uuid;
		d.level = level;
		d.deathmatch = deathmatch;

		levelData.append(d);
	} else {
		MissionLevel *d = levelData.data();

		if (d[index].success) {
			if (success)
				d[index].num = num;
		} else {
			d[index].success = success;
			d[index].num = num;
		}
	}

	m_users[username].levelData = levelData;
	updateUser(username);
}



/**
 * @brief GameMapModel::refreshUsers
 */

void GameMapModel::refreshUsers()
{
	foreach(QString u, m_users.keys())
		updateUser(u);
}


/**
 * @brief GameMapModel::loadFromServer
 * @param jsonData
 */

void GameMapModel::loadFromServer(const QJsonObject &jsonData, const QByteArray &)
{
	QJsonArray uList = jsonData.value("users").toArray();

	foreach (QJsonValue v, uList) {
		QJsonObject o = v.toObject();

		if (!o.contains("username"))
			continue;

		setUser(o);
	}



	QJsonArray list = jsonData.value("list").toArray();

	foreach (QJsonValue v, list) {
		QJsonObject o = v.toObject();

		if (!o.contains("username"))
			continue;

		setUserLevelData(o);
	}
}




/**
 * @brief GameMapModel::setlevelCount
 * @param levelCount
 */

void GameMapModel::setlevelCount(int levelCount)
{
	if (m_levelCount == levelCount)
		return;

	m_levelCount = levelCount;
	emit levelCountChanged(m_levelCount);
}


void GameMapModel::setMissionData(QVariantMap missionData)
{
	if (m_missionData == missionData)
		return;

	m_missionData = missionData;
	emit missionDataChanged(m_missionData);
}



/**
 * @brief GameMapModel::updateUser
 * @param username
 */

void GameMapModel::updateUser(const QString &username)
{
	if (!m_users.contains(username)) {
		qWarning() << "Invalid user" << username;
		return;
	}

	GameMapModel::User user = m_users.value(username);

	VariantMapData *mapdata = variantMapData();
	int index = mapdata->find("username", username);

	QVariantMap m;
	m["username"] = username;
	m["firstname"] = user.firstname;
	m["lastname"] = user.lastname;
	m["xp"] = user.xp;
	m["active"] = user.active;
	m["displayData"] = getUserData(user);
	m["userInfo"] = user.userInfo;

	if (index == -1) {
		mapdata->append(m);
	} else {
		mapdata->update(index, m);
	}
}


/**
 * @brief GameMapModel::getUserData
 * @param username
 * @return
 */

QVariantList GameMapModel::getUserData(const User &user) const
{
	QVariantList l;

	foreach(GameMapModel::Mission m, m_missions)
		foreach(GameMapModel::MissionLevel ml, m.levels) {
			QVariantMap r = user.getLevelData(ml).toMap();
			QVariantMap u = user.userInfo;

			if (u.contains("missionid") && u.value("missionid").toString() == r.value("uuid").toString() &&
				u.contains("level") && u.value("level").toInt() == r.value("level").toInt() &&
				u.contains("deathmatch") && u.value("deathmatch").toInt() == r.value("deathmatch").toInt())
				r["isCurrent"] = true;
			else
				r["isCurrent"] = false;

			l.append(r);
		}

	return l;
}




/**
 * @brief GameMapModel::User::getLevelData
 * @param level
 * @param deathmatch
 * @return
 */

GameMapModel::MissionLevel GameMapModel::User::getLevelData(const QString &uuid, const int &level, const bool &deathmatch) const
{
	foreach(GameMapModel::MissionLevel l, levelData) {
		if (l.uuid == uuid && l.level == level && l.deathmatch == deathmatch)
			return l;
	}

	return GameMapModel::MissionLevel();
}


/**
 * @brief GameMapModel::User::getLevelData
 * @param missionLevel
 * @return
 */

GameMapModel::MissionLevel GameMapModel::User::getLevelData(const GameMapModel::MissionLevel &missionLevel) const
{
	foreach(GameMapModel::MissionLevel l, levelData) {
		if (l.uuid == missionLevel.uuid && l.level == missionLevel.level && l.deathmatch == missionLevel.deathmatch)
			return l;
	}

	return GameMapModel::MissionLevel();
}




/**
 * @brief GameMapModel::MissionLevel::toMap
 * @return
 */

QVariantMap GameMapModel::MissionLevel::toMap() const
{
	QVariantMap m;
	m["uuid"] = uuid;
	m["level"] = level;
	m["deathmatch"] = deathmatch;
	m["success"] = success;
	m["num"] = num;

	return m;
}


/**
 * @brief GameMapModel::Mission::toMap
 * @return
 */

QVariantMap GameMapModel::Mission::toMap() const
{
	QVariantMap m;
	m["uuid"] = uuid;
	m["name"] = name;
	m["levels"] = levels.size();
	m["medalImage"] = medalImage;

	return m;
}
