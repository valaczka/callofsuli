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
	QHttpServerResponse classDelete(const QJsonArray &idList);


	QHttpServerResponse user(const QString &username);
	QHttpServerResponse userCreate(const QJsonObject &json);



	static std::optional<int> _classCreate(const AbstractAPI *api, const Class &_class);
	static QString generateClassCode();

	static bool userAdd(const AbstractAPI *api, const User &user);
	static bool userAdd(const DatabaseMain *dbMain, const User &user);
	static bool authAddPlain(const AbstractAPI *api, const QString &username, const QString &password);
	static bool authAddPlain(const DatabaseMain *dbMain, const QString &username, const QString &password);
	static bool authAddOAuth2(const AbstractAPI *api, const QString &username, const QString &type);

#ifdef _COMPAT


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

	void userImport(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;


	void userPeers(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void userPeersLive(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;



	/// CONFIG FUNCTIONS
	void configUpdate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void usersProfileUpdate(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;


	static QDeferred<bool, int> getClassIdFromCode(const AbstractAPI *api, const QString &code);

	static QDefer userExists(const AbstractAPI *api, const QString &username, const bool &inverse = false);
	static QDefer userNotExists(const AbstractAPI *api, const QString &username) {
		return userExists(api, username, true);
	}


	static QDefer authPlainPasswordChange(const AbstractAPI *api, const QString &username,
										  const QString &oldPassword, const QString &password, const bool &check);



	static QDefer campaignStart(const AbstractAPI *api, const int &campaign);
	static QDefer campaignStart(const DatabaseMain *dbMain, const int &campaign);
	static QDefer campaignFinish(const AbstractAPI *api, const int &campaign);
	static QDefer campaignFinish(const DatabaseMain *dbMain, const int &campaign);
};

#endif

};

#endif // ADMINAPI_H
