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
#include "rank.h"
#include "Logger.h"
#include "serverservice.h"

/**
 * @brief GeneralAPI::GeneralAPI
 * @param service
 */

GeneralAPI::GeneralAPI(ServerService *service)
	: AbstractAPI(service)
{
	addMap("^config/*$", this, &GeneralAPI::config);

	addMap("^rank/*$", this, &GeneralAPI::ranks);
	addMap("^rank/(\\d+)/*$", this, &GeneralAPI::rank);

	addMap("^class/*$", this, &GeneralAPI::classes);
	addMap("^class/(\\d+)/*$", this, &GeneralAPI::classOne);
	addMap("^class/(\\d+)/users/*$", this, &GeneralAPI::classUsers);

	addMap("^user/*$", this, &GeneralAPI::users);
	addMap("^user/(.+)/*$", this, &GeneralAPI::user);

	addMap("^me/*$", this, &GeneralAPI::userMe);
}





/**
 * @brief GeneralAPI::info
 * @param response
 */

void GeneralAPI::config(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	LOG_CTRACE("client") << "Get config";

	QJsonObject c = m_service->config().get();

	if (!m_service->settings()->oauthGoogle().clientId.isEmpty() &&
			!m_service->settings()->oauthGoogle().clientKey.isEmpty())
		c.insert(QStringLiteral("oauthGoogle"), true);

	QJsonObject r;
	r.insert(QStringLiteral("name"), m_service->serverName());
	r.insert(QStringLiteral("versionMajor"), m_service->versionMajor());
	r.insert(QStringLiteral("versionMinor"), m_service->versionMinor());
	r.insert(QStringLiteral("config"), c);

	responseAnswer(response, r);
}





/**
 * @brief GeneralAPI::ranks
 * @param id
 * @param response
 */

void GeneralAPI::ranks(const int &id, const QPointer<HttpResponse> &response) const
{
	LOG_CTRACE("client") << "Get rank list:" << id;

	databaseMainWorker()->execInThread([response, id, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery("SELECT id, level, sublevel, xp, name FROM rank");

		if (id != -1)
			q.addQuery(" WHERE id=").addValue(id);

		q.addQuery(" ORDER BY level, sublevel");

		if (!q.exec())
			return responseErrorSql(response);

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

		if (id == -1)
			responseAnswer(response, "list", list.toJson());
		else if (list.size() != 1)
			responseError(response, "not found");
		else {
			responseAnswer(response, list.at(0).toJson());
		}
	});
}





/**
 * @brief GeneralAPI::classes
 * @param id
 * @param response
 */

void GeneralAPI::classes(const int &id, const QPointer<HttpResponse> &response) const
{
	LOG_CTRACE("client") << "Get class list:" << id;


	databaseMainWorker()->execInThread([response, id, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		bool err = false;

		QueryBuilder q(db);
		q.addQuery("SELECT id, name FROM class");

		if (id != -1)
			q.addQuery(" WHERE id=").addValue(id);

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		if (id == -1)
			responseAnswer(response, "list", list);
		else if (list.size() != 1)
			responseError(response, "not found");
		else
			responseAnswer(response, list.at(0).toObject());
	});
}





/**
 * @brief GeneralAPI::classUsers
 * @param match
 * @param response
 */

void GeneralAPI::classUsers(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &classid = match.captured(1).toInt();

	LOG_CTRACE("client") << "Get user list in class:" << classid;

	databaseMainWorker()->execInThread([response, classid, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery(_SQL_get_user)
				.addQuery("WHERE isPanel=false AND active=true AND user.classid=")
				.addValue(classid);

		bool err = false;

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});

}





/**
 * @brief GeneralAPI::userMe
 * @param match
 * @param response
 */

void GeneralAPI::userMe(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	LOG_CTRACE("client") << "Get me";

	if (!m_credential.isValid()) {
		LOG_CWARNING("client") << "Unauthorized request";
		return responseError(response, "unauthorized");
	}

	user(m_credential.username(), response);
}







/**
 * @brief GeneralAPI::user
 * @param username
 * @param response
 */

void GeneralAPI::user(const QString &username, const QPointer<HttpResponse> &response) const
{
	LOG_CTRACE("client") << "Get user list:" << username;

	databaseMainWorker()->execInThread([response, username, this]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);
		q.addQuery(_SQL_get_user)
				.addQuery("WHERE isPanel=false AND active=true");

		if (!username.isEmpty()) {
			q.addQuery(" AND user.username=")
					.addValue(username);
		}

		bool err = false;

		const QJsonArray &list = q.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		if (username.isEmpty())
			responseAnswer(response, "list", list);
		else if (list.size() != 1)
			responseError(response, "not found");
		else {
			responseAnswer(response, list.at(0).toObject());
		}
	});

}

