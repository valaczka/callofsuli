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

private:
	ServerService *const m_service;
	QString m_redirectHost;
	std::shared_ptr<QHttpServer> m_server;
	std::unique_ptr<Handler> m_handler;
};

#endif // WEBSERVER_H
