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
	Q_OBJECT

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

	QHttpServerResponse campaign(const Credential &credential, const int &id);
	QHttpServerResponse campaignCreate(const Credential &credential, const int &group, const QJsonObject &json);
	QHttpServerResponse campaignUpdate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse campaignRun(const Credential &credential, const int &id);
	QHttpServerResponse campaignFinish(const Credential &credential, const int &id);
	QHttpServerResponse campaignDelete(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse campaignDuplicate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse campaignResult(const Credential &credential, const int &id);
	QHttpServerResponse campaignResultUser(const Credential &credential, const int &id, const QString &username, const QJsonObject &json);

	QHttpServerResponse campaignUser(const Credential &credential, const int &id);
	QHttpServerResponse campaignUserClear(const Credential &credential, const int &id);
	QHttpServerResponse campaignUserAdd(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse campaignUserRemove(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse campaignUserCopy(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse campaignTask(const Credential &credential, const int &id);
	QHttpServerResponse campaignTaskCreate(const Credential &credential, const int &id, const QJsonObject &json);

	QHttpServerResponse task(const Credential &credential, const int &id);
	QHttpServerResponse taskUpdate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse taskDelete(const Credential &credential, const QJsonArray &list);

	QHttpServerResponse freePlayAdd(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse freePlayRemove(const Credential &credential, const int &id, const QJsonArray &list);

	QHttpServerResponse exam(const Credential &credential, const int &id, const int &groupId);
	QHttpServerResponse examCreate(const Credential &credential, const int &group, const QJsonObject &json);
	QHttpServerResponse examUpdate(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse examDelete(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse examResult(const Credential &credential, const int &id, const int &groupId);
	QHttpServerResponse examCreateContent(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse examRemoveContent(const Credential &credential, const int &id, const QJsonArray &list);
	QHttpServerResponse examContent(const Credential &credential, const int &id, const QString &user);
	QHttpServerResponse examContent(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse examAnswer(const Credential &credential, const int &id, const QJsonObject &json);
	QHttpServerResponse examGrading(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse examActivate(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse examInactivate(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse examFinish(const Credential &credential, const QJsonArray &list);
	QHttpServerResponse examReclaim(const Credential &credential, const QJsonArray &list);

	QHttpServerResponse userPeers() const;


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

	static bool _evaluateCampaign(const AbstractAPI *api, const int &campaign, const QString &username);
	static std::optional<bool> _evaluateCriterionXP(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &username);
	static std::optional<bool> _evaluateCriterionMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &map,
										  const QString &username);
	static std::optional<bool> _evaluateCriterionMapMission(const AbstractAPI *api, const int &campaign, const QJsonObject &criterion, const QString &map,
											 const QString &username);


private:
	QJsonObject _task(const int &id) const;
	QJsonArray _taskList(const int &campaign) const;

};

#endif // TEACHERAPI_H
