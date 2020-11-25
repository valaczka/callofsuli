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
#include <QQuickItem>
#include <QJsonObject>
#include <QJsonArray>
#include "cosclient.h"
#include "../common/cosdb.h"
#include "../common/cosmessage.h"

class Client;

class ActivityDB : public CosDb
{
	Q_OBJECT

public:
	explicit ActivityDB(const QString &connectionName, const QString &databaseFile, QObject *parent = nullptr)
		: CosDb(connectionName, parent)
		, m_client(nullptr)
	{
		setDatabaseName(databaseFile);
	}

public slots:
	void setClient(Client *client) { m_client = client; }

protected:
	Client *m_client;
};


class AbstractActivity : public QQuickItem
{
	Q_OBJECT


	Q_PROPERTY(Client* client READ client WRITE setClient NOTIFY clientChanged)
	Q_PROPERTY(ActivityDB* db READ db WRITE setDb NOTIFY dbChanged)
	Q_PROPERTY(bool isBusy READ isBusy WRITE setIsBusy NOTIFY isBusyChanged)

public:
	struct busyData;

	explicit AbstractActivity(QQuickItem *parent = nullptr);
	~AbstractActivity();

	Client* client() const { return m_client; }
	bool isBusy() const { return m_isBusy; }
	ActivityDB* db() const { return m_db; }

public slots:
	void send(const CosMessage::CosClass &cosClass, const QString &cosFunc,
			  const QJsonObject &jsonData = QJsonObject(), const QByteArray &binaryData = QByteArray());
	void setClient(Client* client);
	void setIsBusy(bool isBusy);
	void busyStackAdd(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId);
	void busyStackRemove(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId);
	void setDb(ActivityDB* db);

protected slots:
	virtual void onMessageReceived(const CosMessage &) {}
	virtual void onMessageFrameReceived(const CosMessage &) {}
	virtual void clientSetup() {}

private slots:
	void onMessageReceivedPrivate(const CosMessage &message);
	void onSocketDisconnected();

signals:
	void clientChanged(Client* client);
	void isBusyChanged(bool isBusy);
	void dbChanged(ActivityDB* db);
	void socketDisconnected();

protected:
	Client* m_client;
	ActivityDB* m_db;
	bool m_isBusy;
	QList<busyData> m_busyStack;


public:

	/**
	 * @brief The busyData struct
	 */

	struct busyData {
		CosMessage::CosClass m_cosClass;
		QString m_cosFunc;
		int m_msgId;

		busyData(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgid)
			: m_cosClass(cosClass)
			, m_cosFunc(cosFunc)
			, m_msgId(msgid)
		{ }

		friend inline bool operator== (const busyData &b1, const busyData &b2) {
			return b1.m_cosClass == b2.m_cosClass
					&& b1.m_cosFunc == b2.m_cosFunc
					&& b1.m_msgId == b2.m_msgId;
		}
	};
};


#endif // ABSTRACTACTIVITY_H
