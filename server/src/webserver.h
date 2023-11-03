/*
 * ---- Call of Suli ----
 *
 * websocketserver.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WebSocketServer
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

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "serversettings.h"
#include <QSslConfiguration>
#include <QHttpServer>
#include "handler.h"

class ServerService;
class WebServer;

/**
 * @brief The WebSocketStream class
 */

class WebSocketStream
{
public:
	WebSocketStream(QWebSocket *socket);
	~WebSocketStream();

	enum StreamType {
		StreamInvalid = 0,
		StreamPeers,
		StreamGroupScore
	};

	struct StreamObserver {
		StreamType type = StreamInvalid;
		QVariant data;

		friend inline bool operator== (const StreamObserver &o1, const StreamObserver &o2) {
			return o1.type == o2.type && o1.data == o2.data;
		}
	};



	void observerAdd(const StreamType &type, const QVariant &data = QVariant());
	void observerRemove(const StreamType &type, const QVariant &data = QVariant());
	void observerRemoveAll(const WebSocketStream::StreamType &type);

	bool hasObserver(const StreamType &type);
	bool hasObserver(const StreamType &type, const QVariant &data);

	QWebSocket *socket() const { return m_socket; }

	void sendTextMessage(const QString &message) { if (m_socket) m_socket->sendTextMessage(message); }

	const QVector<StreamObserver> &observers() const;
	void setObservers(const QVector<StreamObserver> &newObservers);

private:
	QPointer<QWebSocket> m_socket;
	QVector<StreamObserver> m_observers;
	QMutex m_mutex;
};



/**
 * @brief The WebSocketStreamHandler class
 */

class WebSocketStreamHandler
{
public:
	WebSocketStreamHandler(WebServer *server);
	~WebSocketStreamHandler();


	void webSocketAdd(QWebSocket *ws);
	void webSocketRemove(QWebSocket *ws);

	void runTest();

	QList<QPointer<QWebSocket>> triggerEvent(const WebSocketStream::StreamType &type);
	QList<QPointer<QWebSocket>> triggerEvent(const WebSocketStream::StreamType &type, const QVariant &data);

private:
	QVector<std::shared_ptr<WebSocketStream>> m_streams;
	QMutex m_mutex;
	WebServer *m_server = nullptr;

};



/**
 * @brief The WebServer class
 */

class WebServer : public QObject
{
	Q_OBJECT

public:
	explicit WebServer(ServerService *service);
	virtual ~WebServer();

	bool start();

	std::optional<QSslConfiguration> loadSslConfiguration(const ServerSettings &settings);

	const QString &redirectHost() const;
	void setRedirectHost(const QString &newRedirectHost);

	std::weak_ptr<QHttpServer> server() const;

	Handler* handler() const;

	void webSocketAdd(QWebSocket *ws) { m_webSocketHandler.webSocketAdd(ws); }
	void webSocketRemove(QWebSocket *ws) { m_webSocketHandler.webSocketRemove(ws); }

	WebSocketStreamHandler &webSocketHandler();

private slots:
	void onWebSocketConnection();

private:
	ServerService *const m_service;
	QString m_redirectHost;
	std::shared_ptr<QHttpServer> m_server;
	std::unique_ptr<Handler> m_handler;
	WebSocketStreamHandler m_webSocketHandler;
};

#endif // WEBSERVER_H
