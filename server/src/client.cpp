/*
 * ---- Call of Suli ----
 *
 * websocketclient.cpp
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WebSocketClient
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

#include "client.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "websocketserver.h"
#include "serverservice.h"


/**
 * @brief Client::Client
 * @param webSocket
 * @param parent
 */

Client::Client(QWebSocket *webSocket, ServerService *service)
	: QObject{service}
	, m_webSocket(webSocket)
	, m_service(service)
{
	qCDebug(lcWebSocket).noquote() << tr("Client created:") << this;
	qCInfo(lcWebSocket).noquote() << tr("New connection:") << webSocket->peerAddress().toString() << webSocket->peerPort();

	setClientState(Hello);

	connect(webSocket, &QWebSocket::disconnected, this, &Client::onDisconnected);
	connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Client::onBinaryMessageReceived);
	connect(webSocket, &QWebSocket::textMessageReceived, this, &Client::onTextMessageReceived);
}


/**
 * @brief Client::~Client
 */

Client::~Client()
{
	qCDebug(lcWebSocket).noquote() << tr("Client destroyed:") << this;
}


/**
 * @brief Client::handleMessage
 * @param message
 */

void Client::handleMessage(const WebSocketMessage &message)
{
	if (m_clientState == Hello) {
		if (message.opCode() != WebSocketMessage::Hello) {
			send(message.createErrorResponse(QStringLiteral("Hello message expected")));
		} else {
			/// IF CHECK VERSION
			setClientState(Connected);
			send(message.createHello());
		}

		return;
	}


	if (m_clientState != Connected) {
		qCWarning(lcWebSocket).noquote() << tr("Handle message error:") << m_clientState << this;
		send(message.createErrorResponse(QStringLiteral("Invalid handler state")));
		return;
	}


	if (message.opCode() == WebSocketMessage::Hello) {
		send(message.createHello());
	} else if (message.opCode() == WebSocketMessage::Request) {
		send(message.createResponse(QJsonObject({
													{ QStringLiteral("status"), QStringLiteral("success") }
												})));
	} else if (message.opCode() == WebSocketMessage::RequestResponse) {
		/// TODO
	} else if (message.opCode() == WebSocketMessage::Event) {
		/// TODO
	}
}


/**
 * @brief Client::send
 * @param message
 */

void Client::send(const WebSocketMessage &message)
{
	if (!m_webSocket) {
		qCWarning(lcWebSocket).noquote() << tr("Missing web socket");
		return;
	}

	qCDebug(lcMessage).noquote() << tr("Message sent:") << message << this;

	m_webSocket->sendBinaryMessage(message.toByteArray());
}




/**
 * @brief Client::onDisconnected
 */

void Client::onDisconnected()
{
	setClientState(Invalid);
	qCInfo(lcWebSocket).noquote() << tr("Disconnected:") << m_webSocket->peerAddress().toString() << m_webSocket->peerPort();
	m_service->clientRemove(this);
}


/**
 * @brief Client::onBinaryMessageReceived
 * @param message
 */

void Client::onBinaryMessageReceived(const QByteArray &message)
{
	WebSocketMessage m = WebSocketMessage::fromByteArray(message);

	if (m.isValid()) {
		qCDebug(lcMessage).noquote() << tr("Message received:") << m << this;
		handleMessage(m);
	} else {
		qCInfo(lcWebSocket).noquote() << tr("Invalid message received:") << this;

		QJsonObject r;
		r.insert(QStringLiteral("server"), QStringLiteral("Call of Suli server"));
		r.insert(QStringLiteral("version"), m_service->version());
		r.insert(QStringLiteral("error"), QStringLiteral("invalid message"));

		send(WebSocketMessage::createEvent(r));
		setClientState(Error);
	}
}


/**
 * @brief Client::onTextMessageReceived
 * @param message
 */

void Client::onTextMessageReceived(const QString &message)
{
	qCInfo(lcWebSocket).noquote() << tr("Text message received:") << message << this;

	QJsonObject r;
	r.insert(QStringLiteral("server"), QStringLiteral("Call of Suli server"));
	r.insert(QStringLiteral("version"), m_service->version());
	r.insert(QStringLiteral("error"), QStringLiteral("Text message not supported"));

	m_webSocket->sendTextMessage(QJsonDocument(r).toJson(QJsonDocument::Indented));
}


/**
 * @brief Client::clientState
 * @return
 */

const Client::ClientState &Client::clientState() const
{
	return m_clientState;
}

void Client::setClientState(const ClientState &newClientState)
{
	if (m_clientState == newClientState)
		return;
	m_clientState = newClientState;
	emit clientStateChanged();
	onClientStateChanged();
}


/**
 * @brief Client::onClientStateChanged
 */

void Client::onClientStateChanged()
{
	if (m_clientState == Error) {
		qCInfo(lcWebSocket).noquote() << tr("Client state error:") << this;
		m_webSocket->close(QWebSocketProtocol::CloseCodeBadOperation);
	}
}
