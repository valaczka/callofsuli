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
#include "websocketstream.h"


class ServerService;
class WebServer;



/**
 * @brief The WebSocketStreamHandler class
 */

class WebSocketStreamHandler
{
public:
	WebSocketStreamHandler(WebServer *server, ServerService *service);
	~WebSocketStreamHandler();

	WebSocketStream *webSocketAdd(QWebSocket *ws);
	void webSocketRemove(WebSocketStream *ws);

	void trigger(const WebSocketStream::StreamType &type);
	void trigger(const WebSocketStream::StreamType &type, const QVariant &data);
	void trigger(WebSocketStream *stream);

	void closeAll();

private:
	QVector<WebSocketStream*> _triggerEvent(const WebSocketStream::StreamType &type);
	QVector<WebSocketStream*> _triggerEvent(const WebSocketStream::StreamType &type, const QVariant &data);

	void _trPeers(const QVector<WebSocketStream*> &list);
	void _trMultiPlayer(const QVector<WebSocketStream*> &list, const int &engineId);

	std::vector<std::unique_ptr<WebSocketStream>> m_streams;
	QMutex m_mutex;
	WebServer *m_server = nullptr;
	ServerService *m_service = nullptr;

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

	WebSocketStreamHandler &webSocketHandler();

	WebSocketStream *webSocketAdd(QWebSocket *ws) { return m_webSocketHandler.webSocketAdd(ws); }

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
