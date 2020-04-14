/*
 * ---- Call of Suli ----
 *
 * adminusers.cpp
 *
 * Created on: 2020. 04. 11.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AdminUsers
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

#include "adminusers.h"

AdminUsers::AdminUsers(QObject *parent)
: AbstractActivity(parent)
{

}


/**
 * @brief AdminUsers::clientSetup
 */

void AdminUsers::clientSetup()
{
	connect(m_client, &Client::jsonUserReceived, this, &AdminUsers::onJsonReceived);
}


/**
 * @brief AdminUsers::onJsonReceived
 * @param object
 * @param binaryData
 * @param clientMsgId
 */

void AdminUsers::onJsonReceived(const QJsonObject &object, const QByteArray &, const int &clientMsgId)
{
	QString func = object["func"].toString();
	QJsonObject data = object["data"].toObject();

	if (!func.isEmpty())
		busyStackRemove(func, clientMsgId);

	if (func == "getAllUser")
		emit userListLoaded(data["list"].toArray());
	else if (func == "userGet")
		emit userLoaded(data);
	else if (func == "userCreate")
		emit userCreated(data);
	else if (func == "userUpdate")
		emit userUpdated(data);
	else if (func == "getAllClass")
		emit classListLoaded(data["list"].toArray());
	else if (func == "classCreate")
		emit classCreated(data);
	else if (func == "classUpdate")
		emit classUpdated(data);
}
