/*
 * ---- Call of Suli ----
 *
 * maprepository.h
 *
 * Created on: 2020. 03. 28.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * MapRepository
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

#ifndef MAPREPOSITORY_H
#define MAPREPOSITORY_H

#include <QObject>
#include "cosdb.h"

class MapRepository : public CosDb
{
	Q_OBJECT

public:
	MapRepository(const QString &connectionName = "mapRepository", QObject *parent = nullptr);

public slots:
	void listReload();
	QVariantMap getInfo(const int &id);
	QVariantMap getInfo(const QString &uuid);
	QByteArray getData(const int &id);
	QByteArray getData(const QString &uuid);
	int getId(const QString &uuid);
	QVariantMap create(const QString &uuid = "");
	int add(const QString &uuid, const QByteArray &data, const QString &md5 = "");
	QJsonObject remove(const int &id);
	QJsonObject remove(const QString &uuid);
	QJsonObject updateData(const QString &uuid, const QByteArray &data);

protected slots:
	bool databaseInit() override;

private:
	QByteArray getDataReal(QSqlQuery q);

signals:
	void listLoaded(QVariantList list);
	void uuidComapareError(const int &id);

};

#endif // MAPREPOSITORY_H
