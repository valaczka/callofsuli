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
	UserAPI(ServerService *service);

	static QDeferred<QMap<QString, GameMap::SolverInfo> > solverInfo(const AbstractAPI *api, const QString &username, const QString &map);
	static QDeferred<GameMap::SolverInfo> solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission);
	static QDeferred<int> solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission,
									 const int &level);
	static int _solverInfo(const AbstractAPI *api, const QString &username, const QString &map, const QString &mission,
						   const int &level, const bool &deathmatch);

	static QDeferred<QJsonArray> getGroupScore(const DatabaseMain *database, const int &id);



	void groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groupScore(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groupScoreLive(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;

	void update(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void password(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;

	void campaigns(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignResult(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

	void maps(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapSolver(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;

	void gameInfo(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void gameCreate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void gameUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void gameFinish(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

	void inventory(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;

	void _addStatistics(const QJsonArray &list) const;
};

#endif // USERAPI_H
