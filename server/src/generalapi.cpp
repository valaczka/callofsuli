/*
 * ---- Call of Suli ----
 *
 * generalapi.cpp
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

#include "generalapi.h"
#include "Logger.h"

/**
 * @brief GeneralAPI::GeneralAPI
 * @param service
 */

GeneralAPI::GeneralAPI(ServerService *service)
	: AbstractAPI(service)
{
	addMap("GET", "ranks", this, &GeneralAPI::ranks);
}


/**
 * @brief GeneralAPI::getRankList
 * @return
 */

QDeferred<RankList> GeneralAPI::getRankList() const
{
	QDeferred<RankList> ret;

	databaseMainWorker()->execInThread([ret, this]() mutable {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT id, level, sublevel, xp, name FROM rank ORDER BY level, sublevel");

		if (!q.exec()) {
			LOG_CDEBUG("client") << "SQL error";
			return ret.reject(RankList());
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
 * @brief GeneralAPI::ranks
 * @param params
 * @param data
 * @param response
 */

void GeneralAPI::ranks(const QString &, const QJsonObject &, HttpResponse *response) const
{
	LOG_CTRACE("client") << "Get rank list";

	getRankList().fail([this, response](RankList){
		responseError(response, "sql error");
	})
	.done([this, response](RankList list) {
		responseAnswer(response, "list", list.toJson());
	});

}

