/*
 * ---- Call of Suli ----
 *
 * adminapi.h
 *
 * Created on: 2023. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AdminAPI
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

#ifndef ADMINAPI_H
#define ADMINAPI_H

#include "abstractapi.h"

class AdminAPI : public AbstractAPI
{
	Q_OBJECT

public:

	AdminAPI(Handler *handler, ServerService *service);
	virtual ~AdminAPI() {}

	/**
	 * @brief The User class
	 */

	struct User {
		QString username;
		QString familyName;
		QString givenName;
		bool active = false;
		int classid = -1;
		bool isTeacher = false;
		bool isAdmin = false;
		bool isPanel = false;
		QString nickname;
		QString picture;
		QString character;

		Credential toCredential() const {
			Credential c;
			c.setUsername(username);
			c.setRole(Credential::Student);
			c.setRole(Credential::Teacher, isTeacher);
			c.setRole(Credential::Admin, isAdmin);
			c.setRole(Credential::Panel, isPanel);
			return c;
		}

	};


	/**
	 * @brief The Class class
	 */

	struct Class {
		int id = -1;
		QString name;

		Class(const int &i, const QString &n) : id(i), name(n) {}
		Class(const QString &n) : name(n) {}
	};




	QHttpServerResponse classUsers(const int &id = 0);
	QHttpServerResponse classCreate(const QJsonObject &json);
	QHttpServerResponse classUpdate(const int &id, const QJsonObject &json);
	QHttpServerResponse classCode(const int &id = 0);
	QHttpServerResponse classUpdateCode(const int &id, const QJsonObject &json);
	QHttpServerResponse classUpdateLimit(const int &id, const int &limit);
	QHttpServerResponse classDelete(const QJsonArray &idList);


	QHttpServerResponse user(const QString &username);
	QHttpServerResponse userCreate(const QJsonObject &json);
	QHttpServerResponse userCreateClass(QJsonObject json, const int &classid);
	QHttpServerResponse userUpdate(const QString &username, const QJsonObject &json);
	QHttpServerResponse userUpdateLimit(const QString &username, const int &limit);
	QHttpServerResponse userDelete(const QJsonArray &userList);
	QHttpServerResponse userPassword(const QString &username, const QJsonObject &json);
	QHttpServerResponse userActivate(const QJsonArray &userList, const bool &active);
	QHttpServerResponse userMove(const QJsonArray &userList, const int &classid);
	QHttpServerResponse userImport(const QJsonObject &json);

	QHttpServerResponse userPeers();

	QHttpServerResponse configUpdate(const QJsonObject &json);
	QHttpServerResponse usersProfileUpdate();




	// Static functions

	static std::optional<int> _classCreate(const AbstractAPI *api, const Class &_class);
	static QString generateClassCode();

	static bool userAdd(const AbstractAPI *api, const User &user);
	static bool userAdd(const DatabaseMain *dbMain, const User &user);

	static bool authAddPlain(const AbstractAPI *api, const QString &username, const QString &password);
	static bool authAddPlain(const DatabaseMain *dbMain, const QString &username, const QString &password);
	static bool authAddOAuth2(const AbstractAPI *api, const QString &username, const QString &type);
	static bool authPlainPasswordChange(const AbstractAPI *api, const QString &username, const QString &oldPassword, const QString &password, const bool &check);

	static bool campaignStart(const AbstractAPI *api, const int &campaign);
	static bool campaignStart(const DatabaseMain *dbMain, const int &campaign);
	static bool campaignFinish(const AbstractAPI *api, const int &campaign);
	static bool campaignFinish(const DatabaseMain *dbMain, const int &campaign);

	static std::optional<int> getClassIdFromCode(const AbstractAPI *api, const QString &code);

	static bool userExists(const AbstractAPI *api, const QString &username, const bool &inverse = false);
	static bool userNotExists(const AbstractAPI *api, const QString &username) {
		return userExists(api, username, true);
	}

};

#endif // ADMINAPI_H
