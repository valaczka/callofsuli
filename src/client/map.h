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
#include "abstractactivity.h"

class Map : public AbstractActivity
{
	Q_OBJECT

	Q_PROPERTY(QString mapTitle READ mapTitle WRITE setMapTitle NOTIFY mapTitleChanged)
	Q_PROPERTY(QString mapUuid READ mapUuid WRITE setMapUuid NOTIFY mapUuidChanged)


public:
	Map(QObject *parent = nullptr);
	virtual ~Map() override;

	QString mapTitle() const { return m_mapTitle; }
	bool dbExists() const { return (QFile::exists(m_databaseFile)); }
	QString mapUuid() const { return m_mapUuid; }

public slots:
	bool create();
	bool loadFromJson(const QByteArray &data);
	bool loadFromFile(const QString &filename);
	QByteArray saveToJson();
	bool saveToFile(const QString &filename);

	void updateMapTitle(const QString &name);


protected slots:
	bool databaseInit() override;


private slots:
	void setMapTitle(QString mapTitle);
	void setMapUuid(QString mapUuid);

signals:
	void mapTitleChanged(QString mapTitle);
	void mapUuidChanged(QString mapUuid);

private:
	QJsonArray tableToJson(const QString &table, const bool &convertData = false);
	bool JsonToTable(const QJsonArray &array, const QString &table, const bool &convertData = false);

	QStringList m_tableNames;
	QString m_mapTitle;
	QString m_mapUuid;
};

#endif // MAP_H
