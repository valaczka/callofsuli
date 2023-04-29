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

/**
 * @brief StudentMapHandler::StudentMapHandler
 * @param parent
 */

StudentMapHandler::StudentMapHandler(QObject *parent)
	: BaseMapHandler{QStringLiteral("studentmaps"), parent}
	, m_mapList(new StudentMapList(this))
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

	download(map, WebSocket::ApiUser, QStringLiteral("map/%1").arg(map->uuid()));
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

	m_client->webSocket()->send(WebSocket::ApiUser, QStringLiteral("campaign/%1").arg(campaign->campaignid()))
			->fail([this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done([this, campaign](const QJsonObject &data){
		campaign->loadFromJson(data, false);
		reloadList();
	});
}



/**
 * @brief StudentMapHandler::playCampaignMap
 * @param campaign
 * @param map
 */

void StudentMapHandler::playCampaignMap(Campaign *campaign, StudentMap *map)
{
	if (!campaign || !map)
		return;

	if (m_client->currentGame()) {
		m_client->messageError(tr("Még folyamatban van egy játék!"));
		return;
	}

	MapPlayCampaign *mapPlay = new MapPlayCampaign(this);

	if (!mapPlay->load(campaign, map)) {
		delete mapPlay;
		return;
	}


	QQuickItem *page = m_client->stackPushPage(QStringLiteral("PageMapPlay.qml"), QVariantMap({
																						{ QStringLiteral("title"), map->name() },
																						{ QStringLiteral("map"), QVariant::fromValue(mapPlay) }
																					}));

	if (!page) {
		m_client->messageError(tr("Nem lehet betölteni az oldalt!"));
		delete mapPlay;
		return;
	}

	connect(page, &QQuickItem::destroyed, mapPlay, &MapPlay::deleteLater);
}


/**
 * @brief StudentMapHandler::reloadList
 */

void StudentMapHandler::reloadList()
{
	m_client->webSocket()->send(WebSocket::ApiUser, QStringLiteral("map"))
			->fail([this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done([this](const QJsonObject &data){
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		OlmLoader::loadFromJsonArray<StudentMap>(m_mapList, list, "uuid", "uuid", true);
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
	return m_mapList;
}