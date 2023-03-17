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
	};


	/// FUNCTIONS

	//void loginOAuth2(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

	static QDeferred<bool, int> getClassIdFromCode(const AbstractAPI *api, const QString &code);

	static QDefer userExists(const AbstractAPI *api, const QString &username, const bool &inverse = false);
	static QDefer userNotExists(const AbstractAPI *api, const QString &username) {
		return userExists(api, username, true);
	}
	static QDefer userAdd(const AbstractAPI *api, const User &user);
	static QDefer authAddPlain(const AbstractAPI *api, const QString &username, const QString &password);
	static QDefer authAddOAuth2(const AbstractAPI *api, const QString &username, const QString &type);
};

#endif // ADMINAPI_H
