/*
 * ---- Call of Suli ----
 *
 * serversettings.cpp
 *
 * Created on: 2020. 11. 20.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerSettings
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

#include "serversettings.h"
#include <QRandomGenerator>

ServerSettings::ServerSettings(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassAdmin, parent)
	, m_modelUserList(new ObjectGenericListModel<UserListObject>(this))
{
	connect(this, &ServerSettings::userListGet, this, &ServerSettings::onUserListGet);
}

/**
 * @brief ServerSettings::~ServerSettings
 */

ServerSettings::~ServerSettings()
{
	delete m_modelUserList;
}



ObjectGenericListModel<UserListObject> *ServerSettings::modelUserList() const
{
	return m_modelUserList;
}


/**
 * @brief ServerSettings::extraServerInfo
 * @return
 */

QJsonObject ServerSettings::extraServerInfo()
{
	QJsonDocument doc;

	const QString filename(":/serverinfo/info.json");

	QFile f(filename);

	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		return QJsonObject();
	}

	QByteArray b = f.readAll();

	f.close();

	QJsonParseError error;
	doc = QJsonDocument::fromJson(b, &error);
	if (error.error != QJsonParseError::NoError)
		qWarning().noquote() << tr("invalid JSON file '%1' at offset %2").arg(error.errorString()).arg(error.offset);

	return doc.object();
}




/**
 * @brief ServerSettings::onUserListGet
 * @param jsonData
 */

void ServerSettings::onUserListGet(QJsonObject jsonData, QByteArray)
{
	m_modelUserList->unselectAll();
	m_modelUserList->resetJsonArray(jsonData.value("list").toArray());
}