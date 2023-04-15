/*
 * ---- Call of Suli ----
 *
 * generalapi.h
 *
 * Created on: 2023. 03. 13.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GeneralAPI
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

#ifndef GENERALAPI_H
#define GENERALAPI_H

#include "abstractapi.h"

class GeneralAPI : public AbstractAPI
{
public:
	GeneralAPI(ServerService *service);
	virtual ~GeneralAPI() {}

	void config(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;

	void rank(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		ranks(match.captured(1).toInt(), response);
	}
	void ranks(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		ranks(-1, response);
	}
	void ranks(const int &id, const QPointer<HttpResponse> &response) const;

	void classOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		classes(match.captured(1).toInt(), response);
	}
	void classes(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		classes(-1, response);
	}
	void classes(const int &id, const QPointer<HttpResponse> &response) const;
	void classUsers(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;

	void user(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		user(match.captured(1), response);
	}
	void users(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		user(QLatin1String(""), response);
	}
	void userStudent(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const {
		user(QLatin1String(""), response, Credential::Student);
	}
	void userMe(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void user(const QString &username, const QPointer<HttpResponse> &response, const Credential::Roles &roles = Credential::None) const;

	void grade(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;


	void testEvents(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;

};



#define _SQL_get_user	\
	"SELECT user.username, familyName, givenName, active, classid, class.name as className, isTeacher, isAdmin, isPanel," \
	"nickname, character, picture, xp, rankid FROM user " \
	"LEFT JOIN class ON (class.id=user.classid) " \
	"LEFT JOIN userRank ON (userRank.username=user.username) "



#endif // GENERALAPI_H
