/*
 * ---- Call of Suli ----
 *
 * abstractactivity.h
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

#ifndef ABSTRACTACTIVITY_H
#define ABSTRACTACTIVITY_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "cosclient.h"

class Client;

class AbstractActivity : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Client* client READ client WRITE setClient NOTIFY clientChanged)
	Q_PROPERTY(bool isBusy READ isBusy WRITE setIsBusy NOTIFY isBusyChanged)
	Q_PROPERTY(QStringList busyStack READ busyStack WRITE setBusyStack NOTIFY busyStackChanged)


public:
	explicit AbstractActivity(QObject *parent = nullptr);

	Client* client() const { return m_client; }
	bool isBusy() const { return m_isBusy; }
	QStringList busyStack() const { return m_busyStack; }

public slots:
	void send(const QJsonObject &query, const QByteArray &binaryData = QByteArray());
	void setClient(Client* client);
	void setIsBusy(bool isBusy);
	void setBusyStack(QStringList busyStack);
	void busyStackAdd(const QString &func, const int &msgId);
	void busyStackRemove(const QString &func, const int &msgId);

protected slots:
	virtual void clientSetup() {}

signals:
	void clientChanged(Client* client);
	void isBusyChanged(bool isBusy);
	void busyStackChanged(QStringList busyStack);

protected:
	Client* m_client;
	bool m_isBusy;
	QStringList m_busyStack;

};

#endif // ABSTRACTACTIVITY_H
