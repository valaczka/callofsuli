/*
 * ---- Call of Suli ----
 *
 * websocketclient.h
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

#ifndef CLIENT_H
#define CLIENT_H

#include <QWebSocket>
#include "oauth2codeflow.h"
#include "websocketmessage.h"
#include "credential.h"

class ServerService;
class AbstractHandler;

class Client : public QObject
{
	Q_OBJECT

	Q_PROPERTY(ClientState clientState READ clientState WRITE setClientState NOTIFY clientStateChanged)
	Q_PROPERTY(Credential credential READ credential WRITE setCredential NOTIFY credentialChanged)
	Q_PROPERTY(OAuth2CodeFlow *oauth2CodeFlow READ oauth2CodeFlow WRITE setOauth2CodeFlow NOTIFY oauth2CodeFlowChanged)

public:
	explicit Client(QWebSocket *webSocket, ServerService *service);
	virtual ~Client();

	enum ClientState {
		Invalid = 0,
		Hello,
		Connected,
		Error
	};

	Q_ENUM(ClientState);

	void handleMessage(const WebSocketMessage &message);

	Q_INVOKABLE void send(const WebSocketMessage &message);

	const ClientState &clientState() const;
	void setClientState(const ClientState &newClientState);

	const Credential &credential() const;
	void setCredential(const Credential &newCredential);

	ServerService *service() const;

	OAuth2CodeFlow *oauth2CodeFlow() const;
	void setOauth2CodeFlow(OAuth2CodeFlow *newOauth2CodeFlow);

	void removeRunningHandler(AbstractHandler *handler);
	void deleteAfterHandlers();

signals:
	void clientStateChanged();
	void credentialChanged();
	void oauth2CodeFlowChanged();

private:
	AbstractHandler *createAuthHandler();
	AbstractHandler *createGeneralHandler();
	AbstractHandler *createServerHandler();

private slots:
	void onClientStateChanged();
	void onDisconnected();
	void onBinaryMessageReceived(const QByteArray &message);
	void onTextMessageReceived(const QString &message);

private:
	QWebSocket *const m_webSocket;
	ServerService *const m_service;
	ClientState m_clientState = Invalid;
	Credential m_credential;

	typedef AbstractHandler* (Client::*HandlerFunc)();

	QHash<WebSocketMessage::ClassHandler, HandlerFunc> m_handlers;

	QPointer<OAuth2CodeFlow> m_oauth2CodeFlow = nullptr;
	QVector<AbstractHandler *> m_runningHandlers;

	bool m_destruction = false;
};

#endif // CLIENT_H
