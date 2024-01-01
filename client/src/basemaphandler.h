/*
 * ---- Call of Suli ----
 *
 * basemaphandler.h
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

#ifndef BASEMAPHANDLER_H
#define BASEMAPHANDLER_H

#include "basemap.h"
#include "client.h"
#include <QObject>
#include <optional>

class BaseMapHandler : public QObject
{
	Q_OBJECT

public:
	explicit BaseMapHandler(const QString &subdirName, QObject *parent = nullptr);
	virtual ~BaseMapHandler() {}

	const QString &subdirName() const;
	void setSubdirName(const QString &newSubdirName);

	Q_INVOKABLE void reload();

	Q_INVOKABLE std::optional<QByteArray> read(const BaseMap *map) const;

	Client *client() const;

signals:
	void reloaded();

protected:
	virtual void reloadList() {}
	virtual void download(BaseMap *map, const HttpConnection::API &api, const QString &path);

	bool hasDownloaded(const BaseMap *map) const;
	bool checkDownload(const BaseMap *map, const QByteArray &data) const;
	bool check(BaseMap *map) const;
	bool checkAndSave(BaseMap *map, const QByteArray &data) const;
	std::optional<QByteArray> readFromMap(const BaseMap *map) const;
	std::optional<QByteArray> loadAndCheck(const BaseMap *map) const;

	Client *const m_client;
	QString m_subdirName;

};

#endif // BASEMAPHANDLER_H
