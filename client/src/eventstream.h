/*
 * ---- Call of Suli ----
 *
 * eventstream.h
 *
 * Created on: 2023. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * EventStream
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EVENTSTREAM_H
#define EVENTSTREAM_H

#include "qnetworkreply.h"
#include "qnetworkrequest.h"
#include <QObject>

class WebSocket;

#define MAX_RETRIES	5

/**
 * @brief The EventStream class
 */

class EventStream : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QNetworkRequest request READ request WRITE setRequest NOTIFY requestChanged)
	Q_PROPERTY(QByteArray requestData READ requestData WRITE setRequestData NOTIFY requestDataChanged)
	Q_PROPERTY(bool reconnect READ reconnect WRITE setReconnect NOTIFY reconnectChanged)

public:
	explicit EventStream(WebSocket *socket);
	virtual ~EventStream();

	const QNetworkRequest &request() const;
	void setRequest(const QNetworkRequest &newRequest);

	const QByteArray &requestData() const;
	void setRequestData(const QByteArray &newRequestData);

	WebSocket *socket() const;

	bool reconnect() const;
	void setReconnect(bool newReconnect);

public slots:
	void connect();
	void disconnect();

private slots:
	void onReplyFinished();
	void onReplyReadyRead();

signals:
	void connected();
	void disconnected();
	void finished();
	void eventReceived(const QByteArray &event, const QByteArray &data);
	void eventJsonReceived(const QString &event, const QJsonObject &json);

	void requestChanged();
	void requestDataChanged();
	void reconnectChanged();

private:
	WebSocket *const m_socket;
	QNetworkRequest m_request;
	QByteArray m_requestData;
	bool m_reconnect = true;
	int m_retries = 0;
	bool m_alreadyConnected = false;
	QNetworkReply *m_reply = nullptr;

};

#endif // EVENTSTREAM_H
