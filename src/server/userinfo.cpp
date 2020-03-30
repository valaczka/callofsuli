/*
 * ---- Call of Suli ----
 *
 * userinfo.cpp
 *
 * Created on: 2020. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * UserInfo
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

#include "userinfo.h"

UserInfo::UserInfo(Client *client, const QJsonObject &object, const QByteArray &binaryData)
	: AbstractHandler(client, object, binaryData)
{

}


/**
 * @brief UserInfo::getServer
 * @return
 */

void UserInfo::getServerName(QJsonObject *jsonResponse, QByteArray *)
{
	m_client->db()->execSelectQueryOneRow("SELECT serverName from system", QVariantList(), jsonResponse);
}




/**
 * @brief UserInfo::getUser
 */

void UserInfo::getUser(QJsonObject *jsonResponse, QByteArray *)
{
	QString username = m_jsonData["username"].toString();
	if (username.isEmpty())
		username = m_client->clientUserName();

	if (username.isEmpty())
		return;

	QVariantList l;
	l << username;

	m_client->db()->execSelectQueryOneRow("SELECT username, firstname, lastname, email, active, "
										  "isTeacher, isAdmin, classid, classname, xp, rankid, rankname "
										  "FROM userInfo where username=?", l, jsonResponse);

}




/**
 * @brief UserInfo::getAllUser
 * @return
 */

void UserInfo::getAllUser(QJsonObject *jsonResponse, QByteArray *)
{
	QJsonArray l;
	m_client->db()->execSelectQuery("SELECT username, firstname, lastname, email, active, "
									"isTeacher, isAdmin, classid, classname, xp, rankid, rankname "
									"FROM userInfo ORDER BY firstname, lastname",
									QVariantList(),
									&l);
	(*jsonResponse)["list"] = l;
}
