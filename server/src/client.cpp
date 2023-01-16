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
#include "authhandler.h"
#include "generalhandler.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "serverservice.h"
#include <Logger.h>


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
	LOG_CINFO("client") << this << "New connection:" << qPrintable(webSocket->peerAddress().toString()) << webSocket->peerPort();

	setClientState(Hello);

	connect(webSocket, &QWebSocket::disconnected, this, &Client::onDisconnected);
	connect(webSocket, &QWebSocket::binaryMessageReceived, this, &Client::onBinaryMessageReceived);
	connect(webSocket, &QWebSocket::textMessageReceived, this, &Client::onTextMessageReceived);

	m_handlers.insert(WebSocketMessage::ClassAuth, &Client::createAuthHandler);
	m_handlers.insert(WebSocketMessage::ClassGeneral, &Client::createGeneralHandler);
}


/**
 * @brief Client::~Client
 */

Client::~Client()
{
	LOG_CTRACE("client") << this << "Client destroyed";
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
		LOG_CWARNING("client") << this << "Handle message error:" << m_clientState;
		send(message.createErrorResponse(QStringLiteral("Invalid handler state")));
		return;
	}


	if (message.opCode() == WebSocketMessage::Hello) {
		send(message.createHello());
	} else {
		HandlerFunc f = m_handlers.value(message.classHandler());

		if (f) {
			std::invoke(f, this)->handleMessage(message);
		} else {
			LOG_CERROR("client") << this << "Missing handler for class:" << message.classHandler();
		}
	}

}


/**
 * @brief Client::send
 * @param message
 */

void Client::send(const WebSocketMessage &message)
{
	if (!m_webSocket) {
		LOG_CWARNING("client") << "Missing web socket";
		return;
	}

	LOG_CTRACE("client") << this << "Message sent:" << message;

	m_webSocket->sendBinaryMessage(message.toByteArray());
}




/**
 * @brief Client::onDisconnected
 */

void Client::onDisconnected()
{
	setClientState(Invalid);
	LOG_CINFO("client") << this << "Disconnected:" << qPrintable(m_webSocket->peerAddress().toString()) << m_webSocket->peerPort()
						<< qPrintable(m_credential.username());
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
		LOG_CTRACE("client") << this << "Message received:" << m;
		handleMessage(m);
	} else {
		LOG_CINFO("client") << this << "Invalid message received:";

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
	LOG_CINFO("client") << this << "Text message received:" << message;

	QJsonObject r;
	r.insert(QStringLiteral("server"), QStringLiteral("Call of Suli server"));
	r.insert(QStringLiteral("version"), m_service->version());
	r.insert(QStringLiteral("error"), QStringLiteral("Text message not supported"));

	m_webSocket->sendTextMessage(QJsonDocument(r).toJson(QJsonDocument::Indented));
}


/**
 * @brief Client::oauth2CodeFlow
 * @return
 */

OAuth2CodeFlow *Client::oauth2CodeFlow() const
{
	return m_oauth2CodeFlow.data();
}


/**
 * @brief Client::setOauth2CodeFlow
 * @param newOauth2CodeFlow
 */

void Client::setOauth2CodeFlow(OAuth2CodeFlow *newOauth2CodeFlow)
{
	if (m_oauth2CodeFlow == newOauth2CodeFlow)
		return;

	if (m_oauth2CodeFlow)
		m_oauth2CodeFlow->deleteLater();

	m_oauth2CodeFlow = newOauth2CodeFlow;
	emit oauth2CodeFlowChanged();
}




ServerService *Client::service() const
{
	return m_service;
}


/**
 * @brief Client::credential
 * @return
 */

const Credential &Client::credential() const
{
	return m_credential;
}

void Client::setCredential(const Credential &newCredential)
{
	if (m_credential == newCredential)
		return;
	m_credential = newCredential;
	emit credentialChanged();
}




/**
 * @brief Client::createAuthHandler
 * @return
 */

AbstractHandler *Client::createAuthHandler()
{
	return new AuthHandler(this);
}


/**
 * @brief Client::createGeneralHandler
 * @return
 */

AbstractHandler *Client::createGeneralHandler()
{
	return new GeneralHandler(this);
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
		LOG_CINFO("client") << this << "Client state error";
		m_webSocket->close(QWebSocketProtocol::CloseCodeBadOperation);
	}
}
