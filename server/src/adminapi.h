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
#include "qjsonarray.h"

class AdminAPI : public AbstractAPI
{
public:
	AdminAPI(ServerService *service);

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


	/// CLASS FUNCTIONS

	void classUsers(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		classUsers(0, response);
	}
	void classUsersNone(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		classUsers(-1, response);
	}
	void classUsersOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		classUsers(match.captured(1).toInt(), response);
	}
	void classUsers(const int &id, const QPointer<HttpResponse> &response) const;


	void classCreate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void classUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void classCodeOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		classCode(match.captured(1).toInt(), response);
	}
	void classCode(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		classCode(-2, response);
	}
	void classCode(const int &code, QPointer<HttpResponse> response) const;
	void classUpdateCode(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void classDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		classDelete({match.captured(1).toInt()}, response);
	}
	void classDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		classDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void classDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;



	/// USER FUNCTIONS

	void user(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void userCreateClass(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		QJsonObject d = data;
		d.insert(QStringLiteral("classid"), match.captured(1).toInt());
		userCreate(match, d, response);
	}
	void userCreate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void userUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void userDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		userDelete({match.captured(1)}, response);
	}
	void userDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		userDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void userDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void userPassword(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

	void userActivateOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		userActivate({match.captured(1)}, true, response);
	}
	void userActivate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		userActivate(data.value(QStringLiteral("list")).toArray(), true, response);
	}
	void userInactivateOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		userActivate({match.captured(1)}, false, response);
	}
	void userInactivate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		userActivate(data.value(QStringLiteral("list")).toArray(), false, response);
	}
	void userActivate(const QJsonArray &list, const bool &active, const QPointer<HttpResponse> &response) const;

	void userMoveOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		userMove({match.captured(1)}, match.captured(2).toInt(), response);
	}
	void userMove(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		userMove(data.value(QStringLiteral("list")).toArray(), match.captured(1).toInt(), response);
	}
	void userMoveOneNoClass(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		userMove({match.captured(1)}, -1, response);
	}
	void userMoveNoClass(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		userMove(data.value(QStringLiteral("list")).toArray(), -1, response);
	}
	void userMove(const QJsonArray &list, const int &classid, const QPointer<HttpResponse> &response) const;



	/// CONFIG FUNCTIONS
	void configUpdate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;


	static QDeferred<bool, int> getClassIdFromCode(const AbstractAPI *api, const QString &code);

	static QDefer userExists(const AbstractAPI *api, const QString &username, const bool &inverse = false);
	static QDefer userNotExists(const AbstractAPI *api, const QString &username) {
		return userExists(api, username, true);
	}
	static QDefer userAdd(const AbstractAPI *api, const User &user);
	static QDefer userAdd(const DatabaseMain *dbMain, const User &user);
	static QDefer authAddPlain(const AbstractAPI *api, const QString &username, const QString &password);
	static QDefer authAddPlain(const DatabaseMain *dbMain, const QString &username, const QString &password);
	static QDefer authAddOAuth2(const AbstractAPI *api, const QString &username, const QString &type);
	static QDefer authPlainPasswordChange(const AbstractAPI *api, const QString &username,
										  const QString &oldPassword, const QString &password, const bool &check);

	static int _classCreate(const AbstractAPI *api, const Class &_class);
	static QString generateClassCode();

	static QDefer campaignStart(const AbstractAPI *api, const int &campaign);
	static QDefer campaignStart(const DatabaseMain *dbMain, const int &campaign);
	static QDefer campaignFinish(const AbstractAPI *api, const int &campaign);
};

#endif // ADMINAPI_H
