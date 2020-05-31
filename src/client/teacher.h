/*
 * ---- Call of Suli ----
 *
 * teachermaps.h
 *
 * Created on: 2020. 03. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * TeacherMaps
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

#ifndef TEACHER_H
#define TEACHER_H

#include <QObject>
#include "abstractactivity.h"

class Teacher : public AbstractActivity
{
	Q_OBJECT

public:
	Teacher(QObject *parent=nullptr);

	void clientSetup() override;

signals:
	void mapListLoaded(const QJsonArray &list);
	void mapCreated(const QJsonObject &map);
	void mapReceived(const QJsonObject &jsonData);
	void mapDataReceived(const QJsonObject &jsonData, const QByteArray &mapData);
	void mapUpdated(const QJsonObject &mapData);
	void mapRemoved(const QJsonObject &mapData);

	void groupListLoaded(const QJsonArray &list);
	void groupCreated(const QJsonObject &groupData);
	void groupReceived(const QJsonObject &groupData);
	void groupUpdated(const QJsonObject &groupData);
	void groupRemoved(const QJsonObject &groupData);
	void groupExcludedMapsReceived(const QJsonObject &groupData);
	void groupExcludedClassesReceived(const QJsonObject &groupData);
	void groupExcludedUsersReceived(const QJsonObject &groupData);

private slots:
	void onJsonMapsReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId);
	void onJsonGroupsReceived(const QJsonObject &object, const QByteArray &, const int &clientMsgId);
};

#endif // TEACHERMAPS_H
