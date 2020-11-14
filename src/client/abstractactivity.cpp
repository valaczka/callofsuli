/*
 * ---- Call of Suli ----
 *
 * abstractactivity.cpp
 *
 * Created on: 2020. 03. 29.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractActivity
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

#include "abstractactivity.h"
#include "QQuickItem"

AbstractActivity::AbstractActivity(QQuickItem *parent)
	: QQuickItem(parent)
	, m_client(nullptr)
	, m_isBusy(false)
{

}

/**
 * @brief AbstractActivity::~AbstractActivity
 */

AbstractActivity::~AbstractActivity()
{

}


/**
 * @brief AbstractActivity::send
 * @param cosClass
 * @param cosFunc
 * @param jsonData
 * @param binaryData
 */

void AbstractActivity::send(const CosMessage::CosClass &cosClass, const QString &cosFunc, const QJsonObject &jsonData, const QByteArray &binaryData)
{
	if (!m_client) {
		qWarning() << tr("Missing client") << this;
		return;
	}

	int msgid = m_client->socketSend(cosClass, cosFunc, jsonData, binaryData);

	busyStackAdd(cosClass, cosFunc, msgid);
}




void AbstractActivity::setClient(Client *client)
{
	if (m_db)
		m_db->setClient(client);

	if (m_client == client)
		return;

	m_client = client;
	emit clientChanged(m_client);

	if (m_client) {
		connect(m_client, &Client::messageReceived, this, &AbstractActivity::onMessageReceivedPrivate);
		connect(m_client, &Client::messageReceived, this, &AbstractActivity::onMessageReceived);
		connect(m_client, &Client::messageFrameReceived, this, &AbstractActivity::onMessageFrameReceived);
		clientSetup();
		if (m_db) {
			connect(m_db, &COSdb::databaseError, m_client, &Client::sendDatabaseError);
		}
	}
}

void AbstractActivity::setIsBusy(bool isBusy)
{
	if (m_isBusy == isBusy)
		return;

	m_isBusy = isBusy;
	emit isBusyChanged(m_isBusy);
}


/**
 * @brief AbstractActivityPrivate::busyStackAdd
 * @param func
 */

void AbstractActivity::busyStackAdd(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId)
{
	m_busyStack.append(busyData(cosClass, cosFunc, msgId));
	setIsBusy(true);
}


/**
 * @brief AbstractActivityPrivate::busyStackRemove
 * @param func
 */

void AbstractActivity::busyStackRemove(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId)
{
	m_busyStack.removeAll(busyData(cosClass, cosFunc, msgId));
	setIsBusy(m_busyStack.count());
}


/**
 * @brief AbstractActivity::setDb
 * @param db
 */


void AbstractActivity::setDb(ActivityDB *db)
{
	if (m_db == db)
		return;

	m_db = db;
	emit dbChanged(m_db);

	if (m_db) {
		m_db->setClient(m_client);
		if (m_client)
			connect(m_db, &COSdb::databaseError, m_client, &Client::sendDatabaseError);
	}
}


/**
 * @brief AbstractActivity::onMessageReceivedPrivate
 * @param message
 */

void AbstractActivity::onMessageReceivedPrivate(const CosMessage &message)
{
	busyStackRemove(message.cosClass(), message.cosFunc(), message.responsedMsgId());
}




