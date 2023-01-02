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

#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include "qloggingcategory.h"
#include <QWebSocketServer>

class ServerService;

class WebSocketServer : public QWebSocketServer
{
	Q_OBJECT

public:
	explicit WebSocketServer(const SslMode &ssl, ServerService *service);
	virtual ~WebSocketServer();

	bool start();

private slots:
	void onNewConnection();

	void onAcceptError(const QAbstractSocket::SocketError &socketError);
	void onPeerVerifyError(const QSslError &error);
	void onServerError(const QWebSocketProtocol::CloseCode &closeCode);
	void onSslErrors(const QList<QSslError> &errors);

private:
	ServerService *const m_service;
};

Q_DECLARE_LOGGING_CATEGORY(lcWebSocket)
Q_DECLARE_LOGGING_CATEGORY(lcMessage)

#endif // WEBSOCKETSERVER_H
