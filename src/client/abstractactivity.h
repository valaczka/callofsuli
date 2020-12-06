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
#include <QtConcurrent/QtConcurrent>
#include "cosclient.h"
#include "../common/cosdb.h"
#include "../common/cosmessage.h"
#include "cosdownloader.h"

class Client;


class AbstractActivity : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(Client* client READ client WRITE setClient NOTIFY clientChanged)
	Q_PROPERTY(bool isBusy READ isBusy WRITE setIsBusy NOTIFY isBusyChanged)
	Q_PROPERTY(CosDownloader * downloader READ downloader WRITE setDownloader NOTIFY downloaderChanged)
	Q_PROPERTY(int canUndo READ canUndo NOTIFY canUndoChanged)
	Q_PROPERTY(QString canUndoString READ canUndoString WRITE setCanUndoString NOTIFY canUndoStringChanged)
	Q_PROPERTY(CosDb* db MEMBER m_db)


public:
	struct busyData;

	explicit AbstractActivity(QQuickItem *parent = nullptr);
	~AbstractActivity();

	Client* client() const { return m_client; }
	bool isBusy() const { return m_isBusy; }
	CosDb* db() const { return m_db; }
	CosDownloader * downloader() const { return m_downloader; }


	Q_INVOKABLE
	template <typename Class>
	void run(const QHash<QString, void (Class::*)(QVariantMap)> &hash, const QString &func, QVariantMap data) {
		if (!hash.contains(func)) {
			qWarning() << "INVALID FUNCTION" << func;
			return;
		}

		int id = ++m_runId;
		busyStackAdd(CosMessage::ClassInvalid, "_run", id);

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(watcher, &QFutureWatcher<void>::finished, this, [=](){
			busyStackRemove(CosMessage::ClassInvalid, "_run", id);
			watcher->deleteLater();
		});
		QFuture<void> future = QtConcurrent::run((Class *)(this), hash.value(func), data);
		watcher->setFuture(future);
	}

	Q_INVOKABLE
	template <typename T, typename Class>
	T run(T (Class::*func)(QVariantMap), QVariantMap data) {
		int id = ++m_runId;
		busyStackAdd(CosMessage::ClassInvalid, "_run", id);

		QFutureWatcher<T> *watcher = new QFutureWatcher<T>(this);
		connect(watcher, &QFutureWatcher<T>::finished, this, [=](){
			busyStackRemove(CosMessage::ClassInvalid, "_run", id);
			watcher->deleteLater();
		});
		QFuture<T> future = QtConcurrent::run((Class *)(this), func, data);
		watcher->setFuture(future);
	}

	Q_INVOKABLE virtual void run(const QString &, QVariantMap) {};

public slots:
	void send(const CosMessage::CosClass &cosClass, const QString &cosFunc,
			  const QJsonObject &jsonData = QJsonObject(), const QByteArray &binaryData = QByteArray());
	void setClient(Client* client);
	void setIsBusy(bool isBusy);
	void busyStackAdd(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId, QObject *otherObject = nullptr);
	void busyStackRemove(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId, QObject *otherObject = nullptr);
	void setDownloader(CosDownloader * downloader);
	void setCanUndoString(QString canUndoString);

	int canUndo() const;

	void addDb(CosDb *db);


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
	void socketDisconnected();
	void downloaderChanged(CosDownloader * downloader);
	void canUndoChanged(int canUndo);
	void canUndoStringChanged(QString canUndoString);

protected:
	Client* m_client;
	bool m_isBusy;
	QList<busyData> m_busyStack;
	CosDownloader *m_downloader;

private:
	CosDb* m_db;
	int m_runId;
	QString m_canUndoString;

public:

	/**
	 * @brief The busyData struct
	 */

	struct busyData {
		CosMessage::CosClass m_cosClass;
		QString m_cosFunc;
		int m_msgId;
		QObject *m_otherObject;

		busyData(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgid, QObject *other)
			: m_cosClass(cosClass)
			, m_cosFunc(cosFunc)
			, m_msgId(msgid)
			, m_otherObject(other)
		{ }

		friend inline bool operator== (const busyData &b1, const busyData &b2) {
			return b1.m_cosClass == b2.m_cosClass
					&& b1.m_cosFunc == b2.m_cosFunc
					&& b1.m_msgId == b2.m_msgId
					&& b1.m_otherObject == b2.m_otherObject;
		}
	};
	QString canUndoString() const
	{
		return m_canUndoString;
	}
};


#endif // ABSTRACTACTIVITY_H
