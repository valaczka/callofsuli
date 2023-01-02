/*
 * ---- Call of Suli ----
 *
 * websocket.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WebSocket
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

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "qloggingcategory.h"
#include <QWebSocket>
#include "server.h"
#include "websocketmessage.h"

class Client;

class WebSocket : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Server *server READ server WRITE setServer NOTIFY serverChanged)
	Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
	explicit WebSocket(Client *client);
	virtual ~WebSocket();

	enum State {
		Disconnected = 0,
		Hello,
		Connected,
		Terminated,
		Error
	};

	Q_ENUM(State);

	QWebSocket *socket() const;

	const State &state() const;
	void setState(const State &newState);

	Server *server() const;
	void setServer(Server *newServer);

public slots:
	void connectToServer(Server *server = nullptr);
	void close();
	void send(const WebSocketMessage &message);

private slots:
	void onConnected();
	void onDisconnected();
	void onError(const QAbstractSocket::SocketError &error);
	void onSslErrors(const QList<QSslError> &errors);
	void onBinaryFrameReceived(const QByteArray &message, const bool &isLastFrame);
	void onBinaryMessageReceived(const QByteArray &message);

signals:
	void serverUnavailable(int num);
	void serverConnected();
	void stateChanged();
	void serverChanged();

private:
	QWebSocket *const m_socket;
	Client *const m_client;
	State m_state = Disconnected;
	Server *m_server = nullptr;
	int m_signalUnavailableNum = 0;
};

Q_DECLARE_LOGGING_CATEGORY(lcWebSocket)

#endif // WEBSOCKET_H
