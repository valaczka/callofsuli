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
#include "mapplayoffline.h"
#include "server.h"
#include "offlineclientengine.h"

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
	if (!map || m_offlineEngine)
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

	if (m_offlineEngine) {
		LOG_CDEBUG("client") << "Offline campaign ready" << campaign->campaignid();
		emit reloaded();
		emit campaign->taskListReloaded();
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


	MapPlay *mapPlay = nullptr;

	std::unique_ptr<MapPlayCampaign> mapPlayCmp;
	std::unique_ptr<MapPlayOffline> mapPlayOffl;


	if (m_offlineEngine) {
		mapPlayOffl.reset(new MapPlayOffline(this, m_offlineEngine));
		mapPlay = mapPlayOffl.get();

		if (!mapPlayOffl->load(campaign, map))
			return;

	} else {
		mapPlayCmp.reset(new MapPlayCampaign(this));
		mapPlay = mapPlayCmp.get();

		if (!mapPlayCmp->load(campaign, map))
			return;
	}



	// Daily rate

	if (m_client->server() && m_client->server()->user()) {
		if (m_client->server()->user()->dailyRate() >= 1.0)
			mapPlay->setReadOnly(true);
	}


	if (m_client->server()) {
		QDir dir = m_client->server()->directory();

		if (!dir.cd(m_subdirName))
			LOG_CERROR("client") << "Can't read directory" << dir.absoluteFilePath(m_subdirName);
		else {
			const QString fname = dir.absoluteFilePath(QStringLiteral("seed.dat"));
			LOG_CDEBUG("client") << "Set storage seed" << qPrintable(fname);
			mapPlay->setStorageSeed(fname);
		}
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
																   { QStringLiteral("map"), QVariant::fromValue(mapPlay) },
																   { QStringLiteral("mission"), QVariant::fromValue(mission) },
															   }));

		if (!page) {
			m_client->messageError(tr("Nem lehet betölteni az oldalt!"));
			return;
		}

		connect(page, &QQuickItem::destroyed, mapPlay, [mapPlay, this](){
			if (mapPlay)
				mapPlay->deleteLater();
			if (m_client && m_client->currentGame())
				emit m_client->currentGame()->gameDestroyRequest();
		});

		if (mapPlayCmp)
			mapPlayCmp.release();

		if (mapPlayOffl)
			mapPlayOffl.release();

		return;
	}


	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMapPlay.qml"), QVariantMap({
																								  { QStringLiteral("title"), map->name() },
																								  { QStringLiteral("map"), QVariant::fromValue(mapPlay) }
																							  }));

	if (!page) {
		m_client->messageError(tr("Nem lehet betölteni az oldalt!"));
		return;
	}

	connect(page, &QQuickItem::destroyed, mapPlay, [g = mapPlay, this](){
		if (g)
			g->deleteLater();
		if (m_client && m_client->currentGame())
			emit m_client->currentGame()->gameDestroyRequest();
	});


	if (mapPlayCmp)
		mapPlayCmp.release();

	if (mapPlayOffl)
		mapPlayOffl.release();
}


/**
 * @brief StudentMapHandler::reloadList
 */

void StudentMapHandler::reloadList()
{
	if (m_offlineEngine)
		return;

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
 * @brief StudentMapHandler::offlineEngine
 * @return
 */

OfflineClientEngine *StudentMapHandler::offlineEngine() const
{
	return m_offlineEngine.get();
}

void StudentMapHandler::setOfflineEngine(OfflineClientEngine *newOfflineEngine)
{
	if (m_offlineEngine == newOfflineEngine)
		return;
	m_offlineEngine = newOfflineEngine;
	emit offlineEngineChanged();
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
	if (m_offlineEngine) {
		LOG_CDEBUG("client") << "Reload offline freeplay map list";

		if (!m_offlineEngine->freeplay())
			return;

		const OfflineDb &db = m_offlineEngine->db();
		OfflinePermit permit;

		if (db.permitList().contains(0))
			permit = db.permitList().value(0);
		else if (db.permitList().contains(-1))
			permit = db.permitList().value(-1);
		else {
			m_client->messageError(tr("Hibás adatokat találtam"));
			return;
		}

		if (!permit.isValid()) {
			m_client->messageError(tr("A szabad játék zárolva van, szinkronizálj!"));
			return;
		}


		list->clear();

		for (const PermitMap &map : permit.permitContent().maps) {
			OfflineMap *m = m_offlineEngine->findMap(map.map);
			if (!m) {
				LOG_CWARNING("client") << "Invalid map" << map.map;
				continue;
			}

			TeacherGroupFreeMap *fm  = new TeacherGroupFreeMap();
			fm->setMap(m);
			fm->setMissionUuid(map.mission);
			list->append(fm);
		}

		return;
	}

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

