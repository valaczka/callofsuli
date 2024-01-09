/*
 * ---- Call of Suli ----
 *
 * studentapi.h
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

#ifndef USERAPI_H
#define USERAPI_H

#include "abstractapi.h"
#include "gamemap.h"

class UserAPI : public AbstractAPI
{
public:
	UserAPI(Handler *handler, ServerService *service);
	virtual ~UserAPI() {}

	QHttpServerResponse update(const Credential &credential, const QJsonObject &json);
	QHttpServerResponse password(const Credential &credential, const QJsonObject &json);

	QHttpServerResponse group(const Credential &credential);
	QHttpServerResponse groupScore(const int &id);

	QHttpServerResponse campaigns(const Credential &credential);
	QHttpServerResponse campaign(const Credential &credential, const int &id);
	QHttpServerResponse campaignResult(const Credential &credential, const int &id, const QJsonObject &json);

	QHttpServerResponse map(const Credential &credential);
	QHttpServerResponse mapContent(const Credential &credential, const QString &uuid);
	QHttpServerResponse mapSolver(const Credential &credential, const QString &uuid);

	QHttpServerResponse gameInfo(const Credential &credential, const QJsonObject &json);
	QHttpServerResponse gameCreate(const Credential &credential, const int &campaign, const QJsonObject &json);
	QHttpServerResponse gameUpdate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse gameFinish(const Credential &credential, const int &id, const QJsonObject &json);

	QHttpServerResponse inventory(const Credential &credential);

	QHttpServerResponse exam(const Credential &credential, const int &id);


	// Static members

	static std::optional<QMap<QString, GameMap::SolverInfo> > solverInfo(const AbstractAPI *api, const QString &username, const QString &map);
	static std::optional<GameMap::SolverInfo> solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission);
	static std::optional<int> solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission,
									 const int &level);

	static std::optional<QJsonArray> getGroupScore(const DatabaseMain *database, const int &id);


	static std::optional<int> _solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission,
						   const int &level, const bool &deathmatch);

private:
	void _addStatistics(const Credential &credential, const QJsonArray &list) const;

};

#endif // USERAPI_H
