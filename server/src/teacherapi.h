/*
 * ---- Call of Suli ----
 *
 * teacherapi.h
 *
 * Created on: 2023. 03. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherAPI
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

#ifndef TEACHERAPI_H
#define TEACHERAPI_H

#include "abstractapi.h"
#include "gamemap.h"
#include "qjsonarray.h"

class TeacherAPI : public AbstractAPI
{
public:
	TeacherAPI(ServerService *service);


	/**
	 * @brief The UserCampaignResult class
	 */

	struct UserCampaignResult {
		int grade = -1;
		int gradeValue = -1;
		int xp = 0;
		QJsonArray tasks;
	};



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



	static QString mapMd5(GameMap *map);
	static QString mapMd5(const QByteArray &data);
	static QJsonObject mapCache(GameMap *map);
	static QString mapCacheString(GameMap *map);

	void groupOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groups(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groupCreate(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void groupUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void groupDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupDelete({match.captured(1).toInt()}, response);
	}
	void groupDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;



	void groupClassAddOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupClassAdd(match.captured(1).toInt(), {match.captured(2).toInt()}, response);
	}
	void groupClassAdd(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupClassAdd(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupClassAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void groupClassRemoveOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupClassRemove(match.captured(1).toInt(), {match.captured(2).toInt()}, response);
	}
	void groupClassRemove(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupClassRemove(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupClassRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;
	void groupClassExclude(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;



	void groupUserAddOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupUserAdd(match.captured(1).toInt(), {match.captured(2)}, response);
	}
	void groupUserAdd(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupUserAdd(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupUserAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void groupUserRemoveOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		groupUserRemove(match.captured(1).toInt(), {match.captured(2)}, response);
	}
	void groupUserRemove(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		groupUserRemove(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void groupUserRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;
	void groupUserExclude(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;

	void groupResult(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void groupUserResult(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;


	void panelOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panels(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelGrab(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelRelease(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void panelUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;



	void mapOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapOneContent(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		mapContent(match.captured(1), response, -1);
	};
	void mapOneDraft(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		mapContent(match.captured(1), response, match.captured(2).toInt());
	};
	void mapContent(const QString &uuid, const QPointer<HttpResponse> &response, const int &draftVersion) const;
	void maps(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void mapPublish(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapDeleteDraft(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void mapDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		mapDelete({match.captured(1)}, response);
	}
	void mapDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		mapDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void mapDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void mapCreate(const QRegularExpressionMatch &match, HttpRequest *request, QPointer<HttpResponse> response) const;
	void mapUpload(const QRegularExpressionMatch &match, HttpRequest *request, QPointer<HttpResponse> response) const;



	void campaignOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignOneResult(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignUserResult(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void campaignCreate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void campaignUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void campaignDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		campaignDelete({match.captured(1).toInt()}, response);
	}
	void campaignDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		campaignDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void campaignDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;
	void campaignRun(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void campaignFinish(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void campaignDuplicate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

	void taskOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void task(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void taskCreate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void taskUpdate(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;
	void taskDeleteOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		taskDelete({match.captured(1).toInt()}, response);
	}
	void taskDelete(const QRegularExpressionMatch &, const QJsonObject &data, QPointer<HttpResponse> response) const {
		taskDelete(data.value(QStringLiteral("list")).toArray(), response);
	}
	void taskDelete(const QJsonArray &list, const QPointer<HttpResponse> &response) const;


	// Run in database worker thread
	QJsonObject _task(const int &id) const;
	QJsonArray _taskList(const int &campaign) const;
	static UserCampaignResult _campaignUserResult(const AbstractAPI *api, const int &campaign, const bool &finished,
												  const QString &username, const bool &withCriterion = false, bool *err = nullptr);
	static UserCampaignResult _campaignUserResult(const DatabaseMain *dbMain, const int &campaign, const bool &finished,
												  const QString &username, const bool &withCriterion = false, bool *err = nullptr);
	static QJsonArray _campaignUserGameResult(const AbstractAPI *api, const int &campaign, const QString &username,
											  const int &limit = DEFAULT_LIMIT, const int &offset = 0, bool *err = nullptr);
	static QJsonArray _groupUserGameResult(const AbstractAPI *api, const int &group, const QString &username,
											  const int &limit = DEFAULT_LIMIT, const int &offset = 0, bool *err = nullptr);

	static bool _evaluateCampaign(const AbstractAPI *api, const int &campaign, const QString &username, bool *err = nullptr);
	static bool _evaluateCriterionXP(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &username, bool *err = nullptr);
	static bool _evaluateCriterionMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &map,
										  const QString &username, bool *err = nullptr);
	static bool _evaluateCriterionMapMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &map,
										  const QString &username, bool *err = nullptr);

};

#endif // TEACHERAPI_H
