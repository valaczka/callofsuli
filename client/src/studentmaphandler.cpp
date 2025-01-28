/*
 * ---- Call of Suli ----
 *
 * studentmaphandler.cpp
 *
 * Created on: 2023. 04. 16.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StudentMapHandler
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

#include "studentmaphandler.h"
#include "campaign.h"
#include "mapplaycampaign.h"
#include "server.h"

/**
 * @brief StudentMapHandler::StudentMapHandler
 * @param parent
 */

StudentMapHandler::StudentMapHandler(QObject *parent)
	: BaseMapHandler{QStringLiteral("studentmaps"), parent}
	, m_mapList(new StudentMapList())
{
	LOG_CTRACE("client") << "StudentMapHandler created" << this;
}

/**
 * @brief StudentMapHandler::~StudentMapHandler
 */

StudentMapHandler::~StudentMapHandler()
{
	m_mapList->deleteLater();
	LOG_CTRACE("client") << "StudentMapHandler destroyed" << this;
}



/**
 * @brief StudentMapHandler::mapDownload
 * @param map
 */

void StudentMapHandler::mapDownload(StudentMap *map)
{
	if (!map)
		return;

	download(map, HttpConnection::ApiUser, QStringLiteral("map/%1").arg(map->uuid()));
}


/**
 * @brief StudentMapHandler::checkDownloads
 */

void StudentMapHandler::checkDownloads()
{
	for (StudentMap *map : *m_mapList)
		BaseMapHandler::check(map);
}



/**
 * @brief StudentMapHandler::getUserCampaign
 * @param campaign
 * @param user
 */

void StudentMapHandler::getUserCampaign(Campaign *campaign)
{
	if (!campaign) {
		LOG_CERROR("client") << "Invalid campaign";
		return;
	}

	m_client->httpConnection()->send(HttpConnection::ApiUser, QStringLiteral("campaign/%1").arg(campaign->campaignid()))
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, [this, campaign](const QJsonObject &data){
		campaign->loadFromJson(data, false);
		reloadList();
	});
}



/**
 * @brief StudentMapHandler::playCampaignMap
 * @param campaign
 * @param map
 */

void StudentMapHandler::playCampaignMap(Campaign *campaign, StudentMap *map, const QString &missionUuid)
{
	if (!map)
		return;

	if (m_client->currentGame()) {
		m_client->messageError(tr("Még folyamatban van egy játék!"));
		return;
	}

	auto mapPlay = std::make_unique<MapPlayCampaign>(this);

	if (!mapPlay->load(campaign, map))
		return;

	// Daily rate

	if (m_client->server() && m_client->server()->user()) {
		if (m_client->server()->user()->dailyRate() >= 1.0)
			mapPlay->setReadOnly(true);
	}




	// Auto load mission levels page

	MapPlayMission *mission = nullptr;

	if (campaign) {
		for (Task *t : *campaign->taskList()) {
			if (t->criterion().value(QStringLiteral("module")).toString() == QStringLiteral("levels") &&
					t->mapUuid() == map->uuid()) {
				if (const QString mUuid = t->criterion().value(QStringLiteral("mission")).toString(); !mUuid.isEmpty()) {
					mission = mapPlay->getMission(mapPlay->gameMap()->mission(mUuid));
					break;
				}
			}
		}
	} else if (!missionUuid.isEmpty()) {
		mission = mapPlay->getMission(mapPlay->gameMap()->mission(missionUuid));
	}


	if (mission && !mission->missionLevelList()->empty()) {
		mapPlay->solver()->setForceUnlockMissionList({mission->uuid()});

		QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMapPlayMissionLevel.qml"),
												   QVariantMap({
																   { QStringLiteral("map"), QVariant::fromValue(mapPlay.get()) },
																   { QStringLiteral("mission"), QVariant::fromValue(mission) },
															   }));

		if (!page) {
			m_client->messageError(tr("Nem lehet betölteni az oldalt!"));
			return;
		}

		connect(page, &QQuickItem::destroyed, mapPlay.get(), [g = mapPlay.get(), this](){
			if (g)
				g->deleteLater();
			if (m_client && m_client->currentGame())
				emit m_client->currentGame()->gameDestroyRequest();
		});
		mapPlay.release();

		return;
	}


	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMapPlay.qml"), QVariantMap({
																								  { QStringLiteral("title"), map->name() },
																								  { QStringLiteral("map"), QVariant::fromValue(mapPlay.get()) }
																							  }));

	if (!page) {
		m_client->messageError(tr("Nem lehet betölteni az oldalt!"));
		return;
	}

	connect(page, &QQuickItem::destroyed, mapPlay.get(), [g = mapPlay.get(), this](){
		if (g)
			g->deleteLater();
		if (m_client && m_client->currentGame())
			emit m_client->currentGame()->gameDestroyRequest();
	});
	mapPlay.release();
}


/**
 * @brief StudentMapHandler::reloadList
 */

void StudentMapHandler::reloadList()
{
	m_client->httpConnection()->send(HttpConnection::ApiUser, QStringLiteral("map"))
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, [this](const QJsonObject &data){
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		OlmLoader::loadFromJsonArray<StudentMap>(m_mapList.get(), list, "uuid", "uuid", true);
		checkDownloads();
		emit reloaded();
	});
}


/**
 * @brief StudentMapHandler::mapList
 * @return
 */

StudentMapList *StudentMapHandler::mapList() const
{
	return m_mapList.get();
}


/**
 * @brief StudentMapHandler::reloadFreePlayMapList
 * @param list
 */

void StudentMapHandler::reloadFreePlayMapList(TeacherGroupFreeMapList *list)
{
	m_client->httpConnection()->send(HttpConnection::ApiUser, QStringLiteral("freeplay"))
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Szabad játékok letöltése sikertelen"));})
			->done(this, [this, mapList = QPointer<TeacherGroupFreeMapList>(list)](const QJsonObject &data){
		const QJsonArray &l = data.value(QStringLiteral("list")).toArray();

		if (!mapList)
			return;

		mapList->clear();

		for (const QJsonValue &v : l) {
			const QJsonObject obj = v.toObject();
			const QString uuid = obj.value(QStringLiteral("mapuuid")).toString();

			StudentMap *map = nullptr;

			for (StudentMap *m : *m_mapList.get()) {
				if (m && m->uuid() == uuid) {
					map = m;
					break;
				}
			}

			if (map) {
				TeacherGroupFreeMap *fm  = new TeacherGroupFreeMap();
				fm->setMap(map);
				fm->setMissionUuid(obj.value(QStringLiteral("mission")).toString());
				mapList->append(fm);
			}
		}
	});
}

