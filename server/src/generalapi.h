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
	Q_OBJECT

public:
	GeneralAPI(Handler *handler, ServerService *service);
	virtual ~GeneralAPI() {}

	QHttpServerResponse config();
	QHttpServerResponse rank(const int &id = -1);
	QHttpServerResponse grade();

	QHttpServerResponse class_(const int &id = -1);
	QHttpServerResponse classUsers(const int &id);

	QHttpServerResponse user(const QString &username = QStringLiteral(""), const Credential::Roles &roles = Credential::None);
	QHttpServerResponse userLog(const QString &username);
	QHttpServerResponse userXpLog(const QString &username, const QJsonObject &json);
	QHttpServerResponse userGameLog(const QString &username);

	QHttpServerResponse me(const std::optional<Credential> &credential);

	QHttpServerResponse time(const QJsonObject &json);

	static std::optional<QJsonArray> _user(const AbstractAPI *api, const QString &username,
										   const Credential::Roles &roles = Credential::None);
};



#define _SQL_get_user	\
	"SELECT user.username, familyName, givenName, active, classid, class.name as className, isTeacher, isAdmin, isPanel, oauth, " \
	"nickname, character, picture, xp, rankid, COALESCE(streak, 0) AS streak, COALESCE((ended_on = date('now')), false) AS streakToday, " \
	"(SELECT COUNT(*) FROM game WHERE game.username=user.username AND success=TRUE) AS trophy, " \
	"COALESCE((SELECT rate FROM dailyLimit WHERE dailyLimit.username=user.username), 0) AS dailyRate " \
	"FROM user " \
	"LEFT JOIN auth ON (auth.username=user.username) " \
	"LEFT JOIN class ON (class.id=user.classid) " \
	"LEFT JOIN userRank ON (userRank.username=user.username) " \
	"LEFT JOIN streak ON (streak.username=user.username AND ended_on >= date('now', '-1 day'))"


#endif // GENERALAPI_H
