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
#include "Logger.h"

WebSocket::WebSocket(Client *client)
	: QObject(client)
	, m_socket(new QWebSocket(QStringLiteral("callofsuli"), QWebSocketProtocol::VersionLatest, this))
	, m_client(client)
	, m_timerPing(new QTimer(this))
{
	LOG_CTRACE("websocket") << "WebSocket created";

#ifndef QT_NO_SSL
	QFile certFile(QStringLiteral(":/root_CallOfSuli_CA.crt"));

	LOG_CTRACE("websocket") << "Cert file exists:" << certFile.exists();


	if (certFile.exists()) {
		certFile.open(QIODevice::ReadOnly);
		QSslCertificate cert(&certFile, QSsl::Pem);
		certFile.close();

		if (cert.isNull()) {
			LOG_CDEBUG("websocket") << "Invalid certificate";
		} else {
			QSslConfiguration config = m_socket->sslConfiguration();
			config.addCaCertificate(cert);
			m_socket->setSslConfiguration(config);
			LOG_CTRACE("websocket") << "Root certificate added";
		}
	}
#endif

	connect(m_socket, &QWebSocket::connected, this, &WebSocket::onConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &WebSocket::onDisconnected);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocket::onError);
#ifndef QT_NO_SSL
	connect(m_socket, &QWebSocket::sslErrors, this, &WebSocket::onSslErrors);
#endif
	connect(m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocket::onBinaryMessageReceived);
	connect(m_socket, &QWebSocket::binaryFrameReceived, this, &WebSocket::onBinaryFrameReceived);


	m_timerPing->setInterval(5000);
	connect(m_timerPing, &QTimer::timeout, this, &WebSocket::onTimerPingTimeout);


}


/**
 * @brief WebSocket::~WebSocket
 */

WebSocket::~WebSocket()
{
	delete m_timerPing;
	delete m_socket;

	LOG_CTRACE("websocket") << "WebSocket destroyed";
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

	LOG_CTRACE("websocket") << "State changed:" << m_state;

	if (m_state == Terminated) {
		emit serverTerminated();
		pingEnabled(true);
	} else
		pingEnabled(false);

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

	if (!server || server->url().isEmpty()) {
		m_client->messageError(tr("Nincs megadva szerver"), tr("Belső hiba"));
		return;
	}

	LOG_CDEBUG("websocket") << "Connect to server:" << server->url();

	if (m_socket->state() != QAbstractSocket::UnconnectedState) {
		LOG_CINFO("websocket") << "Régi nyitott kapcsolat lezárása:" << m_socket->requestUrl();
		m_socket->close();
	}

	if (m_socket->state() == QAbstractSocket::ConnectedState && m_socket->requestUrl() == server->url()) {
		LOG_CTRACE("websocket") << "Server already connected:" << m_socket->requestUrl();
		return;
	}

	setState(Connecting);
	m_socket->open(server->url());
}




/**
 * @brief WebSocket::close
 */

void WebSocket::close()
{
	if (m_state != Disconnected) {
		LOG_CTRACE("websocket") << "Close connection:" << m_socket->requestUrl();
		setState(Disconnected);
		m_socket->close();
		setServer(nullptr);
		emit serverDisconnected();
	}
}



/**
 * @brief WebSocket::abort
 */

void WebSocket::abort()
{
	if (m_state != Disconnected) {
		LOG_CTRACE("websocket") << "Abort connection:" << m_socket->requestUrl();
		m_client->stackPopToStartPage();
		setState(Disconnected);
		m_socket->abort();
		setServer(nullptr);
		emit serverDisconnected();
	}
}



/**
 * @brief WebSocket::send
 * @param message
 */

void WebSocket::send(const WebSocketMessage &message)
{
	if (!((m_state == Hello || m_state == Terminated) && message.opCode() == WebSocketMessage::Hello) && m_state != Connected) {
		LOG_CWARNING("websocket") << "Web socket server unavailable";
		emit serverUnavailable(++m_signalUnavailableNum);

		return;
	}

	m_socket->sendBinaryMessage(message.toByteArray());
}



/**
 * @brief WebSocket::onConnected
 */

void WebSocket::onConnected()
{
	LOG_CTRACE("websocket") << "Connected:" << m_socket->requestUrl();

	if (m_state != Terminated)
		setState(Hello);

	send(WebSocketMessage::createHello());
	m_signalUnavailableNum = 0;
}



/**
 * @brief WebSocket::onDisconnected
 */

void WebSocket::onDisconnected()
{
	LOG_CTRACE("websocket") << "Disonnected:" << m_socket->requestUrl();

	if (m_state == Connecting || m_state == Error)
		setState(Disconnected);
	else if (m_state != Disconnected)
		setState(Terminated);

}


/**
 * @brief WebSocket::onError
 * @param error
 */

void WebSocket::onError(const QAbstractSocket::SocketError &error)
{
	if (m_state == Connected && error == QAbstractSocket::RemoteHostClosedError)
		return;

	if (m_state == Terminated) {
		LOG_CTRACE("websocket") << "Socket ping error:" << error << m_socket->requestUrl();
		return;
	}

	LOG_CTRACE("websocket") << "Socket error:" << error << m_socket->requestUrl();

	emit socketError(error);

	if (m_state != Disconnected)
		setState(Error);
}


/**
 * @brief WebSocket::onSslErrors
 * @param errors
 */

#ifndef QT_NO_SSL
void WebSocket::onSslErrors(const QList<QSslError> &errors)
{
	LOG_CTRACE("websocket") << "Socket ssl errors:" << errors << m_socket->requestUrl();
	if (m_state != Disconnected)
		setState(Error);

	m_socket->ignoreSslErrors(errors);
}
#endif

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

	if (m_state == Hello || m_state == Terminated) {
		if (m.opCode() == WebSocketMessage::Hello) {
			/// TODO: CHECK VERSION
			LOG_CTRACE("websocket") << "Hello successfull";
			if (m_state == Hello) {
				setState(Connected);
				emit serverConnected();
			} else {
				setState(Connected);
				emit serverReconnected();
			}
			return;
		}
	}

	if (m_state != Connected) {
		LOG_CERROR("websocket") << "Invalid state";
		return;
	}

	if (!m.isValid()) {
		LOG_CERROR("websocket") << "Invalid web socket message received";
		return;
	}

	LOG_CTRACE("websocket") << "RECEIVED:" << m;

	if (m.data().contains("error"))
		m_client->snack(m.data().value("error").toString());

	m_client->handleMessage(m);
}


/**
 * @brief WebSocket::pingEnabled
 * @param on
 */

void WebSocket::pingEnabled(const bool &on)
{
	LOG_CDEBUG("websocket") << "Ping enabled:" << on;

	if (on && m_timerPing->isActive())
		return;

	if (!on && !m_timerPing->isActive())
		return;

	if (on)
		m_timerPing->start();
	else
		m_timerPing->stop();
}



/**
 * @brief WebSocket::onTimerPingTimeout
 */

void WebSocket::onTimerPingTimeout()
{
	LOG_CTRACE("websocket") << "Ping";

	if (m_state == Terminated && m_server)
		m_socket->open(m_server->url());
}

