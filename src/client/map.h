/*
 * ---- Call of Suli ----
 *
 * map.h
 *
 * Created on: 2020. 03. 27.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Map
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MAP_H
#define MAP_H

#include <QObject>
#include "abstractdbactivity.h"


class Map : public AbstractDbActivity
{
	Q_OBJECT

public:

	enum MapType { MapInvalid, MapEditor, MapGame, MapCustom };
	Q_ENUM(MapType)

	Q_PROPERTY(QString mapUuid READ mapUuid WRITE setMapUuid NOTIFY mapUuidChanged)
	Q_PROPERTY(QString mapTimeCreated READ mapTimeCreated WRITE setMapTimeCreated NOTIFY mapTimeCreatedChanged)
	Q_PROPERTY(QString mapOriginalFile READ mapOriginalFile WRITE setMapOriginalFile NOTIFY mapOriginalFileChanged)
	Q_PROPERTY(MapType mapType READ mapType WRITE setMapType NOTIFY mapTypeChanged)


	Map(QObject *parent = nullptr);
	virtual ~Map() override;


	QString mapUuid() const { return m_mapUuid; }
	QString mapTimeCreated() const { return m_mapTimeCreated; }
	QString mapOriginalFile() const { return m_mapOriginalFile; }
	MapType mapType() const { return m_mapType; }

public slots:
	void save(const bool &binaryFormat = true);
	bool loadFromJson(const QByteArray &data, const bool &binaryFormat = true);
	bool loadFromFile(const QString &filename, const bool &binaryFormat = true);
	bool loadFromBackup();
	QByteArray saveToJson(const bool &binaryFormat = true);
	static QByteArray create(const bool &binaryFormat = true);
	bool saveToFile(const QString &filename, const QByteArray &data = QByteArray());
	void updateMapOriginalFile(const QString &filename);
	void updateMapServerId(const int &serverId, const int &mapId);
	bool databaseCheck();
	bool databasePrepare();

	void setMapType(MapType mapType);
	void setMapUuid(QString mapUuid);
	void setMapTimeCreated(QString mapTimeCreated);
	void setMapOriginalFile(QString mapOriginalFile);



signals:
	void mapBackupExists(const QString &originalFile, const QString &uuid, const int &serverid, const int &mapid);
	void mapLoaded();
	void mapLoadedFromBackup();
	void mapSaved(const QByteArray &data, const QString &uuid);

#ifdef QT_DEBUG
	void mapRefreshed(const QByteArray &data);
#endif

	void mapTitleChanged(QString mapTitle);
	void mapUuidChanged(QString mapUuid);
	void mapTimeCreatedChanged(QString mapTimeCreated);
	void mapOriginalFileChanged(QString mapOriginalFile);

	void mapTypeChanged(MapType mapType);

private:
	QJsonArray tableToJson(const QString &table, const bool &convertData = false);
	bool JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData = false);

	QStringList m_tableNames;
	QString m_mapUuid;
	QString m_mapTimeCreated;
	QString m_mapOriginalFile;
	MapType m_mapType;
};

#endif // MAP_H
