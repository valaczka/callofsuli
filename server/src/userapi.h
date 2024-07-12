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
	Q_OBJECT

public:
	UserAPI(Handler *handler, ServerService *service);
	virtual ~UserAPI() {}

	/**
	 * @brief The UserGame class
	 */

	struct UserGame {
		QString map;
		QString mission;
		int level = -1;
		bool deathmatch = false;
		GameMap::GameMode mode = GameMap::Invalid;
		int campaign = -1;
	};

	QHttpServerResponse update(const Credential &credential, const QJsonObject &json);
	QHttpServerResponse password(const Credential &credential, const QJsonObject &json);

	QHttpServerResponse group(const Credential &credential);
	QHttpServerResponse groupScore(const int &id);

	QHttpServerResponse campaigns(const Credential &credential);
	QHttpServerResponse campaign(const Credential &credential, const int &id);
	QHttpServerResponse campaignResult(const Credential &credential, const int &id, const QJsonObject &json);

	QHttpServerResponse freePlay(const Credential &credential);

	QHttpServerResponse map(const Credential &credential);
	QHttpServerResponse mapContent(const Credential &credential, const QString &uuid);
	QHttpServerResponse mapSolver(const Credential &credential, const QString &uuid);

	QHttpServerResponse gameInfo(const Credential &credential, const QJsonObject &json);
	QHttpServerResponse gameCreate(const Credential &credential, const int &campaign, const QJsonObject &json);
	QHttpServerResponse gameCreate(const QString &username, const int &campaign,
								   const UserGame &game, const QJsonObject &inventory,
								   int *gameIdPtr = nullptr);
	QHttpServerResponse gameUpdate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse gameUpdateStatistics(const QString &username, const QJsonArray &statistics);
	QHttpServerResponse gameFinish(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse gameFinish(const QString &username, const int &id, const UserGame &game,
								   const QJsonObject &inventory, const QJsonArray &statistics, const bool &success, const int &xp, const int &duration,
								   bool *okPtr = nullptr);

	QHttpServerResponse inventory(const Credential &credential);

	QHttpServerResponse exam(const Credential &credential, const int &id);

	QHttpServerResponse wallet(const Credential &credential);


	static std::optional<QMap<QString, GameMap::SolverInfo> > solverInfo(const AbstractAPI *api, const QString &username, const QString &map);
	static std::optional<GameMap::SolverInfo> solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission);
	static std::optional<int> solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission,
									 const int &level);

	static std::optional<QJsonArray> getGroupScore(const DatabaseMain *database, const int &id);


	static std::optional<int> _solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission,
						   const int &level, const bool &deathmatch);

private:
	void _addStatistics(const QString &username, const QJsonArray &list) const;

};

#endif // USERAPI_H
