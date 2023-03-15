/*
 * ---- Call of Suli ----
 *
 * generalhandler.h
 *
 * Created on: 2023. 01. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * GeneralHandler
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

#ifndef GENERALHANDLER_H
#define GENERALHANDLER_H

#include "abstracthandler.h"
#include "rank.h"

class GeneralHandler : public AbstractHandler
{
	Q_OBJECT

public:
	explicit GeneralHandler(Client *client);

	QDeferred<RankList> getRankList() const;
	QDeferred<QJsonArray> getUserListGroup(const int &id) const;
	QDeferred<QJsonArray> getUserListClass(const int &id = -1) const;

protected:
	virtual void handleRequestResponse() {};
	virtual void handleEvent() {};

private slots:
	void rankList();
	void userList();
	void userListGroup();
	void userListClass();

	void me();

	void classList();

private:
};



#define _SQL_get_user	\
	"SELECT user.username, familyName, givenName, active, classid, class.name as className, isTeacher, isAdmin, isPanel," \
	"nickname, character, picture, xp, rankid FROM user " \
	"LEFT JOIN class ON (class.id=user.classid) " \
	"LEFT JOIN userRank ON (userRank.username=user.username) "


#endif // GENERALHANDLER_H
