/*
 * ---- Call of Suli ----
 *
 * basemap.h
 *
 * Created on: 2023. 03. 31.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * BaseMap
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

#ifndef BASEMAP_H
#define BASEMAP_H

#include "qjsonobject.h"
#include <QObject>
#include <selectableobject.h>

class BaseMap : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString md5 READ md5 WRITE setMd5 NOTIFY md5Changed)
	Q_PROPERTY(int size READ size WRITE setSize NOTIFY sizeChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QJsonObject cache READ cache WRITE setCache NOTIFY cacheChanged)
	Q_PROPERTY(bool downloaded READ downloaded WRITE setDownloaded NOTIFY downloadedChanged)
	Q_PROPERTY(qreal downloadProgress READ downloadProgress WRITE setDownloadProgress NOTIFY downloadProgressChanged)

public:
	explicit BaseMap(QObject *parent = nullptr);

	virtual void loadFromJson(const QJsonObject &object, const bool &allField = true);

	bool downloaded() const;
	void setDownloaded(bool newDownloaded);

	qreal downloadProgress() const;
	void setDownloadProgress(qreal newDownloadProgress);

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &md5() const;
	void setMd5(const QString &newMd5);

	int size() const;
	void setSize(int newSize);

	const QString &name() const;
	void setName(const QString &newName);

	const QJsonObject &cache() const;
	void setCache(const QJsonObject &newCache);

signals:
	void downloadedChanged();
	void downloadProgressChanged();
	void uuidChanged();
	void md5Changed();
	void sizeChanged();
	void nameChanged();
	void cacheChanged();

protected:
	QString m_uuid;
	QString m_md5;
	int m_size = 0;
	QString m_name;
	QJsonObject m_cache;
	bool m_downloaded = false;
	qreal m_downloadProgress = -1.0;
};

#endif // BASEMAP_H
