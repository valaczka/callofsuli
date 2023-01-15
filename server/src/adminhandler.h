/*
 * ---- Call of Suli ----
 *
 * adminhandler.h
 *
 * Created on: 2023. 01. 06.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AdminHandler
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

#ifndef ADMINHANDLER_H
#define ADMINHANDLER_H

#include "abstracthandler.h"

class AdminHandler : public AbstractHandler
{
	Q_OBJECT

public:
	AdminHandler(Client *client);

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

	static QDefer userAdd(AbstractHandler *handler, const User &user);
	static QDefer authAddPlain(AbstractHandler *handler, const QString &username, const QString &password);
	static QDefer authAddOAuth2(AbstractHandler *handler, const QString &username, const QString &oauthType);

	static QString generateClassCode();

protected:
	virtual void handleRequestResponse() {};
	virtual void handleEvent() {};
};

#endif // ADMINHANDLER_H
