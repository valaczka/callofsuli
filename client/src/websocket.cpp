/*
 * ---- Call of Suli ----
 *
 * websocket.cpp
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

#include "websocket.h"
#include "client.h"
#include "websocketmessage.h"

Q_LOGGING_CATEGORY(lcWebSocket, "app.websocket")

WebSocket::WebSocket(Client *client)
	: QObject(client)
	, m_socket(new QWebSocket(QStringLiteral("callofsuli"), QWebSocketProtocol::VersionLatest, this))
	, m_client(client)
{
	qCDebug(lcWebSocket).noquote() << tr("WebSocket created");

	connect(m_socket, &QWebSocket::connected, this, &WebSocket::onConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &WebSocket::onDisconnected);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocket::onError);
	connect(m_socket, &QWebSocket::sslErrors, this, &WebSocket::onSslErrors);
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocket::onBinaryMessageReceived);
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, &WebSocket::onBinaryFrameReceived);

	connect(m_socket, &QWebSocket::stateChanged, this, [=](const QAbstractSocket::SocketState &state){
		qCDebug(lcWebSocket).noquote() << tr("----- STATE CHANGED") << state;
	});
}


/**
 * @brief WebSocket::~WebSocket
 */

WebSocket::~WebSocket()
{
	delete m_socket;

	qCDebug(lcWebSocket).noquote() << tr("WebSocket destroyed");
}


/**
 * @brief WebSocket::socket
 * @return
 */

QWebSocket *WebSocket::socket() const
{
	return m_socket;
}

/**
 * @brief WebSocket::state
 * @return
 */

const WebSocket::State &WebSocket::state() const
{
	return m_state;
}

void WebSocket::setState(const State &newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	emit stateChanged();
}


/**
 * @brief WebSocket::server
 * @return
 */

Server *WebSocket::server() const
{
	return m_server;
}

void WebSocket::setServer(Server *newServer)
{
	if (m_server == newServer)
		return;
	m_server = newServer;
	emit serverChanged();
}


/**
 * @brief WebSocket::connectToServer
 * @param server
 */

void WebSocket::connectToServer(Server *server)
{
	if (!server)
		server = m_server;
	else
		setServer(server);

	if (!server) {
		m_client->messageError(tr("Nincs megadva szerver"), tr("Belső hiba"));
		return;
	}

	qCDebug(lcWebSocket).noquote() << tr("Connect to server:") << server->url();

	if (m_socket->state() != QAbstractSocket::UnconnectedState) {
		qCInfo(lcWebSocket).noquote() << tr("Régi nyitott kapcsolat lezárása:") << m_socket->requestUrl();
		m_socket->close();
	}

	if (m_socket->state() == QAbstractSocket::ConnectedState && m_socket->requestUrl() == server->url()) {
		qCDebug(lcWebSocket).noquote() << tr("Server already connected:") << m_socket->requestUrl();
		return;
	}

	m_socket->open(server->url());
}




/**
 * @brief WebSocket::close
 */

void WebSocket::close()
{
	qCDebug(lcWebSocket).noquote() << tr("Close connection:") << m_socket->requestUrl();
	setState(Disconnected);
	m_socket->close();
}



/**
 * @brief WebSocket::send
 * @param message
 */

void WebSocket::send(const WebSocketMessage &message)
{
	if (!(m_state == Hello && message.opCode() == WebSocketMessage::Hello) && m_state != Connected) {
		qCWarning(lcWebSocket).noquote() << tr("Web socket server unavailable");
		emit serverUnavailable(++m_signalUnavailableNum);

		return;
	}

	qDebug() << "-->" << message;

	m_socket->sendBinaryMessage(message.toByteArray());
}



/**
 * @brief WebSocket::onConnected
 */

void WebSocket::onConnected()
{
	qCDebug(lcWebSocket).noquote() << tr("Connected:") << m_socket->requestUrl();
	setState(Hello);
	send(WebSocketMessage::createHello());
	m_signalUnavailableNum = 0;
}

void WebSocket::onDisconnected()
{
	qCDebug(lcWebSocket).noquote() << tr("Disonnected:") << m_socket->requestUrl();

	if (m_state != Disconnected)
		setState(Terminated);
}


/**
 * @brief WebSocket::onError
 * @param error
 */

void WebSocket::onError(const QAbstractSocket::SocketError &error)
{
	qCDebug(lcWebSocket).noquote() << tr("Socket error:") << error << m_socket->requestUrl();
	setState(Error);
}


/**
 * @brief WebSocket::onSslErrors
 * @param errors
 */

void WebSocket::onSslErrors(const QList<QSslError> &errors)
{
	qCDebug(lcWebSocket).noquote() << tr("Socket ssl errors:") << errors << m_socket->requestUrl();
	setState(Error);
}


/**
 * @brief WebSocket::onBinaryFrameReceived
 * @param message
 * @param isLastFrame
 */

void WebSocket::onBinaryFrameReceived(const QByteArray &message, const bool &isLastFrame)
{
	///qCDebug(lcWebSocket).noquote() << "BINARY FRAME RECEIVED";
}


/**
 * @brief WebSocket::onBinaryMessageReceived
 * @param message
 */

void WebSocket::onBinaryMessageReceived(const QByteArray &message)
{
	WebSocketMessage m = WebSocketMessage::fromByteArray(message);

	if (m_state == Hello) {
		if (m.opCode() == WebSocketMessage::Hello) {
			/// TODO: CHECK VERSION
			qCDebug(lcWebSocket).noquote() << tr("Hello successfull");
			setState(Connected);
			emit serverConnected();
			return;
		}
	}

	if (m_state != Connected) {
		qCCritical(lcWebSocket).noquote() << tr("Invalid state");
		return;
	}

	if (!m.isValid()) {
		qCCritical(lcWebSocket).noquote() << tr("Invalid web socket message received");
		return;
	}

	qCDebug(lcWebSocket).noquote() << tr("RECEIVED:") << m;

	emit m_client->webSocketMessageReceived(m);
}

