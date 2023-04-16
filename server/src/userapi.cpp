/*
 * ---- Call of Suli ----
 *
 * studentapi.cpp
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentAPI
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

#include "userapi.h"
#include "qjsonarray.h"


/**
 * @brief StudentAPI::StudentAPI
 * @param service
 */

UserAPI::UserAPI(ServerService *service)
	: AbstractAPI(service)
{
	m_validateRole = Credential::Student;

	addMap("^campaign/*$", this, &UserAPI::campaigns);
	addMap("^campaign/(\\d+)/*$", this, &UserAPI::campaignOne);
	addMap("^group/*$", this, &UserAPI::groups);

	addMap("^map/*$", this, &UserAPI::maps);
	addMap("^map/([^/]+)/*$", this, &UserAPI::mapOne);

}





/**
 * @brief StudentAPI::groups
 * @param response
 */

void UserAPI::groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT id, name, owner, familyName AS ownerFamilyName, givenName AS ownerGivenName FROM studentGroupInfo "
						  "LEFT JOIN user ON (user.username=studentGroupInfo.owner) "
						  "WHERE studentGroupInfo.active=true AND studentGroupInfo.username=").addValue(username)
				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}




/**
 * @brief StudentAPI::campaigns
 * @param response
 */

void UserAPI::campaigns(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
						  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
						  "description, finished, groupid "
						  "FROM campaign WHERE started=true AND groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")")
				.execToJsonArray(&err);

		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}



/**
 * @brief UserAPI::campaignOne
 * @param match
 * @param response
 */

void UserAPI::campaignOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const int &id = match.captured(1).toInt();

	databaseMainWorker()->execInThread([this, response, id]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		QJsonObject obj = QueryBuilder::q(db)
				.addQuery("SELECT id, CAST(strftime('%s', starttime) AS INTEGER) AS starttime, "
						  "CAST(strftime('%s', endtime) AS INTEGER) AS endtime, "
						  "description, finished, groupid "
						  "FROM campaign WHERE started=true "
						  "AND id=").addValue(id)
				.addQuery(" AND groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")")
				.execToJsonObject(&err);

		if (err)
			return responseErrorSql(response);

		if (obj.isEmpty())
			return responseError(response, "not found");


		/// TODO: solved?

		obj[QStringLiteral("taskList")] = QueryBuilder::q(db)
				.addQuery("SELECT id, gradeid, xp, required, mapuuid, criterion, false AS solved FROM task WHERE campaignid=").addValue(id)
				.execToJsonArray({
									 { QStringLiteral("criterion"), [](const QVariant &v) {
										   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
									   } }
								 });


		responseAnswer(response, obj);
	});

}




/**
 * @brief StudentAPI::maps
 * @param response
 */

void UserAPI::maps(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const
{
	databaseMainWorker()->execInThread([this, response]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		const QString &username = m_credential.username();

		bool err = false;

		const QJsonArray &list = QueryBuilder::q(db)
				.addQuery("SELECT mapdb.map.uuid, name, md5, "
						  "mapdb.cache.data AS cache, length(mapdb.map.data) as size "
						  "FROM mapdb.map LEFT JOIN mapdb.cache ON (mapdb.cache.uuid=mapdb.map.uuid) "
						  "WHERE mapdb.map.uuid IN "
						  "(SELECT mapuuid FROM task WHERE campaignid IN "
						  "(SELECT id FROM campaign WHERE groupid IN "
						  "(SELECT id FROM studentGroupInfo WHERE active=true AND username=").addValue(username)
				.addQuery(")))")
				.execToJsonArray({
									 { QStringLiteral("cache"), [](const QVariant &v) {
										   return QJsonDocument::fromJson(v.toString().toUtf8()).object();
									   } }
								 },
								 &err);


		if (err)
			return responseErrorSql(response);

		responseAnswer(response, "list", list);
	});
}




/**
 * @brief UserAPI::mapOne
 * @param match
 * @param response
 */

void UserAPI::mapOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const
{
	const QString &uuid = match.captured(1);

	databaseMainWorker()->execInThread([this, response, uuid]() {
		QSqlDatabase db = QSqlDatabase::database(databaseMain()->dbName());

		QMutexLocker(databaseMain()->mutex());

		QueryBuilder q(db);

		q.addQuery("SELECT data FROM mapdb.map WHERE uuid=").addValue(uuid);

		if (!q.exec())
			return responseErrorSql(response);

		if (q.sqlQuery().first()) {
			const QByteArray &b = q.sqlQuery().value(QStringLiteral("data")).toByteArray();
			if (response)
				response->setStatus(HttpStatus::Ok, b);
			return;
		} else
			return responseError(response, "not found");
	});
}



