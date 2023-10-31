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
	TeacherAPI(Handler *handler, ServerService *service);
	virtual ~TeacherAPI() {}

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



	QHttpServerResponse groups(const Credential &credential);
	QHttpServerResponse group(const Credential &credential, const int &id);
	QHttpServerResponse groupCreate(const Credential &credential, const QJsonObject &json);
	QHttpServerResponse groupUpdate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse groupDelete(const Credential &credential, const QJsonArray &list);

	QHttpServerResponse groupClassAdd(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse groupClassRemove(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse groupClassExclude(const Credential &credential, const int &id);

	QHttpServerResponse groupUserAdd(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse groupUserRemove(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse groupUserExclude(const Credential &credential, const int &id);

	QHttpServerResponse groupResult(const Credential &credential, const int &id);
	QHttpServerResponse groupUserResult(const Credential &credential, const int &id, const QString &username, const QJsonObject &json);
	QHttpServerResponse groupGameLog(const Credential &credential, const int &id, const QJsonObject &json);

	QHttpServerResponse map(const Credential &credential, const QString &uuid = QStringLiteral(""));
	QHttpServerResponse mapCreate(const Credential &credential, const QByteArray &body, const QString &name = QStringLiteral(""));
	QHttpServerResponse mapUpdate(const Credential &credential, const QString &uuid, const QJsonObject &json);
	QHttpServerResponse mapPublish(const Credential &credential, const QString &uuid, const int &version);
	QHttpServerResponse mapDelete(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse mapDeleteDraft(const Credential &credential, const QString &uuid, const int &version);
	QHttpServerResponse mapUpload(const Credential &credential, const QString &uuid, const int &version, const QByteArray &body);
	QHttpServerResponse mapContent(const Credential &credential, const QString &uuid, const int &draftVersion = -1);




	// Static members

	static QString mapMd5(GameMap *map);
	static QString mapMd5(const QByteArray &data);
	static QJsonObject mapCache(GameMap *map);
	static QString mapCacheString(GameMap *map);

	static std::optional<UserCampaignResult> _campaignUserResult(const AbstractAPI *api, const int &campaign, const bool &finished,
												  const QString &username, const bool &withCriterion = false);
	static std::optional<UserCampaignResult> _campaignUserResult(const DatabaseMain *dbMain, const int &campaign, const bool &finished,
												  const QString &username, const bool &withCriterion = false);

	static std::optional<QJsonArray> _campaignUserGameResult(const AbstractAPI *api, const int &campaign, const QString &username,
											  const int &limit = DEFAULT_LIMIT, const int &offset = 0);
	static std::optional<QJsonArray> _groupUserGameResult(const AbstractAPI *api, const int &group, const QString &username,
										   const int &limit = DEFAULT_LIMIT, const int &offset = 0);
	static std::optional<QJsonArray> _groupGameResult(const AbstractAPI *api, const int &group,
									   const int &limit = DEFAULT_LIMIT, const int &offset = 0);


private:
	QJsonObject _task(const int &id) const;
	QJsonArray _taskList(const int &campaign) const;



#if _COMPAT


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

	void campaignUser(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignUserClear(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const;
	void campaignUserAddOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		campaignUserAdd(match.captured(1).toInt(), {match.captured(2)}, response);
	}
	void campaignUserAdd(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		campaignUserAdd(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void campaignUserAdd(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;

	void campaignUserRemoveOne(const QRegularExpressionMatch &match, const QJsonObject &, QPointer<HttpResponse> response) const {
		campaignUserRemove(match.captured(1).toInt(), {match.captured(2)}, response);
	}
	void campaignUserRemove(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const {
		campaignUserRemove(match.captured(1).toInt(), data.value(QStringLiteral("list")).toArray(), response);
	}
	void campaignUserRemove(const int &id, const QJsonArray &list, const QPointer<HttpResponse> &response) const;
	void campaignUserCopy(const QRegularExpressionMatch &match, const QJsonObject &data, QPointer<HttpResponse> response) const;

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


	void userPeers(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;
	void userPeersLive(const QRegularExpressionMatch &, const QJsonObject &, QPointer<HttpResponse> response) const;


	// Run in database worker thread


	static bool _evaluateCampaign(const AbstractAPI *api, const int &campaign, const QString &username, bool *err = nullptr);
	static bool _evaluateCriterionXP(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &username, bool *err = nullptr);
	static bool _evaluateCriterionMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &map,
										  const QString &username, bool *err = nullptr);
	static bool _evaluateCriterionMapMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &map,
											 const QString &username, bool *err = nullptr);
#endif

};

#endif // TEACHERAPI_H
