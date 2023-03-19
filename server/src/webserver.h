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
#include "httpServer.h"
#include "handler.h"

class ServerService;

class WebServer : public QObject
{
	Q_OBJECT

public:
	explicit WebServer(ServerService *service);
	virtual ~WebServer();

	bool start();

	HttpServer *server() const;

	static QSslConfiguration loadSslConfiguration(const ServerSettings &settings);

	const QString &redirectHost() const;
	void setRedirectHost(const QString &newRedirectHost);

	Handler *handler() const;

private:
	ServerService *const m_service;
	HttpServer *m_server = nullptr;
	Handler *const m_handler = nullptr;
	QString m_redirectHost;
};

#endif // WEBSERVER_H
