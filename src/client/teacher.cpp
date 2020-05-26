/*
 * ---- Call of Suli ----
 *
 * teachermaps.cpp
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

#include "teacher.h"

Teacher::Teacher(QObject *parent)
	: AbstractActivity(parent)
{

}


/**
 * @brief TeacherMaps::clientSetup
 */

void Teacher::clientSetup()
{
	connect(m_client, &Client::jsonTeacherMapsReceived, this, &Teacher::onJsonMapsReceived);
	connect(m_client, &Client::jsonTeacherGroupsReceived, this, &Teacher::onJsonGroupsReceived);
}




/**
 * @brief TeacherMaps::onJsonTeacherMapsReceived
 * @param object
 */

void Teacher::onJsonMapsReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId)
{
	QString func = object.value("func").toString();
	QJsonObject data = object.value("data").toObject();

	if (!func.isEmpty())
		busyStackRemove(func, clientMsgId);

	if (func == "getAllMap")
		emit mapListLoaded(data.value("list").toArray());
	else if (func == "getMap")
		emit mapReceived(data, binaryData);
	else if (func == "createMap")
		emit mapCreated(data);
	else if (func == "updateMap")
		emit mapUpdated(data);
}



/**
 * @brief Teacher::onJsonGroupsReceived
 * @param object
 * @param binaryData
 * @param clientMsgId
 */

void Teacher::onJsonGroupsReceived(const QJsonObject &object, const QByteArray &, const int &clientMsgId)
{
	QString func = object.value("func").toString();
	QJsonObject data = object.value("data").toObject();

	if (!func.isEmpty())
		busyStackRemove(func, clientMsgId);

	if (func == "getAllGroup")
		emit groupListLoaded(data.value("list").toArray());
	else if (func == "getGroup")
		emit groupReceived(data);
	else if (func == "createGroup")
		emit groupCreated(data);
	else if (func == "updateGroup")
		emit groupUpdated(data);
}

