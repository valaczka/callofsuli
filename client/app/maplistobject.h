/*
 * ---- Call of Suli ----
 *
 * mapobject.h
 *
 * Created on: 2021. 12. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapObject
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

#ifndef MAPLISTOBJECT_H
#define MAPLISTOBJECT_H

#include "objectlistmodel.h"
#include "objectlistmodelobject.h"


class MapListObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(QString uuid READ uuid WRITE setUuid NOTIFY uuidChanged)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
	Q_PROPERTY(int dataSize READ dataSize WRITE setDataSize NOTIFY dataSizeChanged)
	Q_PROPERTY(bool downloaded READ downloaded WRITE setDownloaded NOTIFY downloadedChanged)
	Q_PROPERTY(QString md5 READ md5 WRITE setMd5 NOTIFY md5Changed)

	Q_PROPERTY(int version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(QVariantList binded READ binded WRITE setBinded NOTIFY bindedChanged)
	Q_PROPERTY(bool used READ used WRITE setUsed NOTIFY usedChanged)
	Q_PROPERTY(QDateTime lastModified READ lastModified WRITE setLastModified NOTIFY lastModifiedChanged)

public:
	Q_INVOKABLE explicit MapListObject(QObject *parent = nullptr);

	const QString &uuid() const;
	void setUuid(const QString &newUuid);

	const QString &name() const;
	void setName(const QString &newName);

	bool active() const;
	void setActive(bool newActive);

	int dataSize() const;
	void setDataSize(int newDataSize);

	bool downloaded() const;
	void setDownloaded(bool newDownloaded);

	const QString &md5() const;
	void setMd5(const QString &newMd5);

	int version() const;
	void setVersion(int newVersion);

	const QVariantList &binded() const;
	void setBinded(const QVariantList &newBinded);

	bool used() const;
	void setUsed(bool newUsed);

	const QDateTime &lastModified() const;
	void setLastModified(const QDateTime &newLastModified);

signals:
	void uuidChanged();
	void nameChanged();
	void activeChanged();
	void dataSizeChanged();
	void downloadedChanged();
	void md5Changed();
	void versionChanged();
	void bindedChanged();
	void usedChanged();
	void lastModifiedChanged();

private:
	QString m_uuid;
	QString m_name;
	bool m_active;
	int m_dataSize;
	bool m_downloaded;
	QString m_md5;
	int m_version;
	QVariantList m_binded;
	bool m_used;
	QDateTime m_lastModified;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<MapListObject>*);

#endif // MAPOBJECT_H
