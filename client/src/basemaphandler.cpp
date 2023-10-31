/*
 * ---- Call of Suli ----
 *
 * basemaphandler.cpp
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * BaseMapHandler
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

#include "basemaphandler.h"
#include "application.h"
#include "utils_.h"
#include "server.h"

BaseMapHandler::BaseMapHandler(const QString &subdirName, QObject *parent)
	: QObject{parent}
	, m_client(Application::instance()->client())
	, m_subdirName(subdirName)
{

}



/**
 * @brief BaseMapHandler::hasDownloaded
 * @param subdir
 * @param map
 * @return
 */

bool BaseMapHandler::hasDownloaded(const BaseMap *map) const
{
	const auto &c = readFromMap(map);
	if (c)
		return checkDownload(map, *c);
	else
		return false;
}





/**
 * @brief BaseMapHandler::checkDownload
 * @param map
 * @param data
 * @return
 */

bool BaseMapHandler::checkDownload(const BaseMap *map, const QByteArray &data) const
{
	Q_ASSERT(map);

	return (map->md5() == QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex()) &&
			map->size() == data.size());
}





/**
 * @brief BaseMapHandler::check
 * @param map
 * @return
 */

bool BaseMapHandler::check(BaseMap *map) const
{
	if (!map)
		return false;

	bool dl = hasDownloaded(map);
	map->setDownloaded(dl);
	return dl;
}





const QString &BaseMapHandler::subdirName() const
{
	return m_subdirName;
}

void BaseMapHandler::setSubdirName(const QString &newSubdirName)
{
	m_subdirName = newSubdirName;
}


/**
 * @brief BaseMapHandler::reload
 */

void BaseMapHandler::reload()
{
	reloadList();
}



/**
 * @brief BaseMapHandler::read
 * @param map
 * @return
 */

std::optional<QByteArray> BaseMapHandler::read(BaseMap *map) const
{
	if (!map) {
		LOG_CERROR("client") << "Invalid map";
		return std::nullopt;
	}

	return loadAndCheck(map);
}



/**
 * @brief BaseMapHandler::mapDownload
 * @param map
 * @param api
 * @param path
 */

void BaseMapHandler::download(BaseMap *map, const WebSocket::API &api, const QString &path)
{
	WebSocketReply *r = m_client->webSocket()->send(api, path)
			->fail(this, [this](const QString &err){m_client->messageWarning(err, tr("Letöltési hiba"));})
			->done(this, [this, map](const QByteArray &data){ checkAndSave(map, data); });

	connect(r, &WebSocketReply::downloadProgress, map, &BaseMap::setDownloadProgress);
}




/**
 * @brief BaseMapHandler::checkAndSave
 * @param map
 * @param data
 * @return
 */

bool BaseMapHandler::checkAndSave(BaseMap *map, const QByteArray &data) const
{
	if (!map || !m_client->server()) {
		m_client->messageWarning(tr("Belső hiba"), tr("Letöltési hiba"));
		return false;
	}

	if (!checkDownload(map, data)) {
		m_client->messageWarning(tr("Hibás adat érkezett"), tr("Letöltési hiba"));
		return false;
	}

	/*const QByteArray &uncomp = qUncompress(data);

	if (uncomp.isEmpty()) {
		m_client->messageError(tr("Hibás adat érkezett"), tr("Tömörítési hiba"));
		return false;
	}*/

	QDir dir = m_client->server()->directory();

	if ((!dir.exists(m_subdirName) && !dir.mkdir(m_subdirName)) || !dir.cd(m_subdirName)) {
		m_client->messageError(tr("Belső hiba"));
		return false;
	}

	const QString filename = QStringLiteral("%1.map").arg(map->uuid());

	LOG_CTRACE("client") << "Save map:" << dir.absoluteFilePath(filename);

	QFile f(dir.absoluteFilePath(filename));

	if (!f.open(QIODevice::WriteOnly)) {
		m_client->messageError(tr("Nem lehet menteni az adatokat"), tr("Belső hiba"));
		return false;
	}

	f.write(data);

	f.close();

	check(map);

	return true;
}



/**
 * @brief BaseMapHandler::readFromMap
 * @param map
 * @return
 */

std::optional<QByteArray> BaseMapHandler::readFromMap(const BaseMap *map) const
{
	if (!map || !m_client->server())
		return std::nullopt;

	QDir dir = m_client->server()->directory();

	if (!dir.cd(m_subdirName))
		return std::nullopt;


	const QString filename = QStringLiteral("%1.map").arg(map->uuid());

	if (!dir.exists(filename))
		return std::nullopt;

	const auto &b = Utils::fileContent(dir.absoluteFilePath(filename));

	if (!b) {
		m_client->messageError(tr("Nem olvasható fájl"), tr("Belső hiba"));
		return std::nullopt;
	}

	return b;
}





/**
 * @brief BaseMapHandler::loadAndCheck
 * @param map
 * @return
 */

std::optional<QByteArray> BaseMapHandler::loadAndCheck(const BaseMap *map) const
{
	Q_ASSERT(map);

	const auto &b = readFromMap(map);

	if (b && checkDownload(map, *b))
		return qUncompress(*b);
	else {
		LOG_CWARNING("client") << "Map check failed";
		return std::nullopt;
	}
}



/**
 * @brief BaseMapHandler::client
 * @return
 */

Client *BaseMapHandler::client() const
{
	return m_client;
}
