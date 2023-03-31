/*
 * ---- Call of Suli ----
 *
 * teachermaphandler.cpp
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMapHandler
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

#include "teachermaphandler.h"
#include "gamemap.h"


/**
 * @brief TeacherMapHandler::TeacherMapHandler
 * @param parent
 */

TeacherMapHandler::TeacherMapHandler(QObject *parent)
	: BaseMapHandler{QStringLiteral("teachermaps"), parent}
	, m_mapList(new TeacherMapList(this))
{
	LOG_CTRACE("client") << "TeacherMapHandler created" << this;
}


/**
 * @brief TeacherMapHandler::~TeacherMapHandler
 */

TeacherMapHandler::~TeacherMapHandler()
{
	delete m_mapList;
	LOG_CTRACE("client") << "TeacherMapHandler destroyed" << this;
}





/**
 * @brief TeacherMapHandler::check
 * @param list
 */

void TeacherMapHandler::checkDownloads()
{
	for (TeacherMap *map : *m_mapList)
		BaseMapHandler::check(map);
}




/**
 * @brief TeacherMapHandler::reloadList
 */

void TeacherMapHandler::reloadList()
{
	m_client->webSocket()->send(WebSocket::ApiTeacher, QStringLiteral("map"))
			->fail([this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done([this](const QJsonObject &data){
		const QJsonArray &list = data.value(QStringLiteral("list")).toArray();
		OlmLoader::loadFromJsonArray<TeacherMap>(m_mapList, list, "uuid", "uuid", true);
		checkDownloads();
		emit reloaded();
	});
}


/**
 * @brief TeacherMapHandler::mapList
 * @return
 */

TeacherMapList *TeacherMapHandler::mapList() const
{
	return m_mapList;
}




/**
 * @brief TeacherMapHandler::mapImport
 * @param file
 */

void TeacherMapHandler::mapImport(const QUrl &file)
{
	LOG_CDEBUG("client") << "Import map:" << file;

	if (file.isEmpty() || !file.isLocalFile()) {
		LOG_CERROR("client") << "Invalid URL:" << file;
		return m_client->messageError(tr("A fájl nem importálható"));
	}

	bool err = false;
	const QByteArray &b = Utils::fileContent(file.toLocalFile(), &err);

	if (err)
		return m_client->messageError(tr("A fájl nem importálható"));

	GameMap *map = GameMap::fromBinaryData(b);

	if (!map)
		return m_client->messageError(tr("A fájl nem Call of Suli pályát tartalmaz!"), tr("Érvénytelen fájl"));

	delete map;
	map = nullptr;

	m_client->webSocket()->send(WebSocket::ApiTeacher, QStringLiteral("map/create"), b, {
									{ QStringLiteral("name"), Utils::fileBaseName(file.toLocalFile()) }
								})
			->fail([this](const QString &err){m_client->messageWarning(err, tr("Importálási hiba"));})
			->done([this](const QJsonObject &){
		m_client->snack(tr("Az importálás sikerült"));
		reload();
	});
}





/**
 * @brief TeacherMapHandler::mapDownload
 * @param map
 */

void TeacherMapHandler::mapDownload(TeacherMap *map)
{
	if (!map)
		return;

	download(map, WebSocket::ApiTeacher, QStringLiteral("map/%1/content").arg(map->uuid()));
}



