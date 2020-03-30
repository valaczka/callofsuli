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

AbstractActivity::AbstractActivity(QObject *parent) : QObject(parent)
{
	m_client = nullptr;
	m_isBusy = false;
}


/**
 * @brief AbstractActivity::send
 * @param query
 */

void AbstractActivity::send(const QJsonObject &query, const QByteArray &binaryData)
{
	int msgid = m_client->socketSend(query, binaryData);

	QString f = query["func"].toString();
	if (!f.isEmpty())
		busyStackAdd(f, msgid);

}



void AbstractActivity::setClient(Client *client)
{
	if (m_client == client)
		return;

	m_client = client;
	emit clientChanged(m_client);

	qDebug() << "setClient" << m_client;

	if (m_client) {
		clientSetup();
	}
}

void AbstractActivity::setIsBusy(bool isBusy)
{
	if (m_isBusy == isBusy)
		return;

	m_isBusy = isBusy;
	emit isBusyChanged(m_isBusy);
}

void AbstractActivity::setBusyStack(QStringList busyStack)
{
	if (m_busyStack == busyStack)
		return;

	m_busyStack = busyStack;
	emit busyStackChanged(m_busyStack);
}


/**
 * @brief AbstractActivity::busyStackAdd
 * @param func
 */

void AbstractActivity::busyStackAdd(const QString &func, const int &msgId)
{
	m_busyStack.append(QString(func).append(msgId));
	setIsBusy(true);
}


/**
 * @brief AbstractActivity::busyStackRemove
 * @param func
 */

void AbstractActivity::busyStackRemove(const QString &func, const int &msgId)
{
	m_busyStack.removeAll(QString(func).append(msgId));
	setIsBusy(m_busyStack.count());
}
