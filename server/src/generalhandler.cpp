/*
 * ---- Call of Suli ----
 *
 * generalhandler.cpp
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

#include "generalhandler.h"

GeneralHandler::GeneralHandler(Client *client)
	: AbstractHandler(client)
{

}


/**
 * @brief GeneralHandler::getRankList
 * @return
 */

QDeferred<RankList> GeneralHandler::getRankList() const
{
	LOG_CTRACE("client") << "Get rank list";

	QDeferred<RankList> ret;

	databaseMain()->worker()->execInThread([ret, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT id, level, sublevel, xp, name FROM rank ORDER BY level, sublevel");

		if (!q.exec()) {
			LOG_CDEBUG("client") << "SQL error";
			ret.reject(RankList());
			return;
		}

		RankList list;

		list.reserve(q.sqlQuery().size());

		while (q.sqlQuery().next()) {
			Rank r(
						q.value("id").toInt(),
						q.value("level").toInt(),
						q.value("sublevel", -1).toInt(),
						q.value("xp", -1).toInt(),
						q.value("name").toString()
						);
			list.append(r);

		}

		ret.resolve(list);
	});

	return ret;
}


/**
 * @brief GeneralHandler::rankList
 */

void GeneralHandler::rankList()
{
	getRankList().fail([this](RankList){
		send(m_message.createErrorResponse(QStringLiteral("sql error")));
	})
	.done([this](RankList list) {
		send(m_message.createResponse("list", list.toJson()));
	});
}
