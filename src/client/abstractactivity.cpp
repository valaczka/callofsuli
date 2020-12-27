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
#include <QtConcurrent/QtConcurrent>

AbstractActivity::AbstractActivity(const CosMessage::CosClass &defaultClass, QQuickItem *parent)
	: QQuickItem(parent)
	, m_client(nullptr)
	, m_isBusy(false)
	, m_busyStack()
	, m_downloader(nullptr)
	, m_db(nullptr)
	, m_runId(0)
	, m_canUndoString()
	, m_removeDatabase(false)
	, m_defaultClass(defaultClass)
{
}

/**
 * @brief AbstractActivity::~AbstractActivity
 */

AbstractActivity::~AbstractActivity()
{
	if (m_db) {
		if (m_removeDatabase) {
			QString dbName = m_db->databaseName();

			if (!m_db->isOpen())
				m_db->close();

			QFile::remove(dbName);
		}
		delete m_db;
	}
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


/**
 * @brief AbstractActivity::send
 * @param cosFunc
 * @param jsonData
 * @param binaryData
 */




/**
 * @brief AbstractActivity::setClient
 * @param client
 */


void AbstractActivity::setClient(Client *client)
{
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
			connect(m_db->worker(), &CosDbWorker::databaseError, m_client, &Client::sendDatabaseError);
		}

		QWebSocket *socket = m_client->socket();
		connect(socket, &QWebSocket::disconnected, this, &AbstractActivity::onSocketDisconnected);
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

void AbstractActivity::busyStackAdd(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId, QObject *otherObject)
{
	m_busyStack.append(busyData(cosClass, cosFunc, msgId, otherObject));
	setIsBusy(true);
}


/**
 * @brief AbstractActivityPrivate::busyStackRemove
 * @param func
 */

void AbstractActivity::busyStackRemove(const CosMessage::CosClass &cosClass, const QString &cosFunc, const int &msgId, QObject *otherObject)
{
	m_busyStack.removeAll(busyData(cosClass, cosFunc, msgId, otherObject));
	setIsBusy(m_busyStack.count());
}

/**
 * @brief AbstractActivity::setDownloader
 * @param downloader
 */

void AbstractActivity::setDownloader(CosDownloader *downloader)
{
	if (m_downloader == downloader)
		return;

	m_downloader = downloader;
	emit downloaderChanged(m_downloader);
}


/**
 * @brief AbstractActivity::canUndo
 * @return
 */

int AbstractActivity::canUndo() const
{
	if (m_db)
		return m_db->canUndo();
	else
		return -1;
}


/**
 * @brief AbstractActivity::addDb
 * @param db
 */

void AbstractActivity::addDb(CosDb *db, const bool &removeDatabase)
{
	if (m_db) {
		qWarning() << "COSdb already exists";
		return;
	}

	m_db = db;

	setRemoveDatabase(removeDatabase);

	emit dbChanged(m_db);

	if (m_client)
		connect(m_db->worker(), &CosDbWorker::databaseError, m_client, &Client::sendDatabaseError);

	connect(m_db, &CosDb::canUndoChanged, this, [=](int canUndo, const QString &canUndoString) {
		emit canUndoChanged(canUndo);
		setCanUndoString(canUndoString);
	});
}

void AbstractActivity::setRemoveDatabase(bool removeDatabase)
{
	if (m_removeDatabase == removeDatabase)
		return;

	m_removeDatabase = removeDatabase;
	emit removeDatabaseChanged(m_removeDatabase);
}



/**
 * @brief AbstractActivity::setCanUndoString
 * @param canUndoString
 */

void AbstractActivity::setCanUndoString(QString canUndoString)
{
	if (m_canUndoString == canUndoString)
		return;

	m_canUndoString = canUndoString;
	emit canUndoStringChanged(m_canUndoString);
}




/**
 * @brief AbstractActivity::onMessageReceivedPrivate
 * @param message
 */

void AbstractActivity::onMessageReceivedPrivate(const CosMessage &message)
{
	busyStackRemove(message.cosClass(), message.cosFunc(), message.responsedMsgId());
}


/**
 * @brief AbstractActivity::onSocketDisconnected
 */

void AbstractActivity::onSocketDisconnected()
{
	qDebug() << "Socket disconnected";
	m_busyStack.clear();
	setIsBusy(false);
	emit socketDisconnected();
}


/**
 * @brief AbstractActivity::autoSignalEmit
 * @param message
 */

void AbstractActivity::autoSignalEmit(const CosMessage &message)
{
	if (m_defaultClass == CosMessage::ClassInvalid)
		return;

	QString func = message.cosFunc();
	QJsonObject d = message.jsonData();
	QByteArray b = message.binaryData();

	if (message.cosClass() == m_defaultClass) {
		QByteArray normalizedSignature = QMetaObject::normalizedSignature(func.toLatin1()+"(QJsonObject, QByteArray)");

		int methodIndex = this->metaObject()->indexOfSignal(normalizedSignature);

		if (methodIndex == -1) {
			qWarning() << func+" not found";
			return;
		}

		QMetaObject::invokeMethod(this, func.toLatin1(), Qt::AutoConnection,
								  Q_ARG(QJsonObject, d),
								  Q_ARG(QByteArray, b)
								  );
	}
}


/**
 * @brief AbstractActivity::waitForFutureFinished
 * @param future
 * @param id
 */

void AbstractActivity::waitForFutureFinished(QFuture<void> &future, int id)
{
	if (future.isFinished())
		return;

	busyStackAdd(CosMessage::ClassInvalid, "_run", id);
	qDebug() << "WAIT FOR FINISH" << id;

	future.waitForFinished();

	qDebug() << "FINISHED" << id;

	busyStackRemove(CosMessage::ClassInvalid, "_run", id);
}




