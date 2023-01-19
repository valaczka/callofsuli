/*
 * ---- Call of Suli ----
 *
 * asyncmessagehandler.h
 *
 * Created on: 2023. 01. 14.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AsyncMessageHandler
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

#ifndef ASYNCMESSAGEHANDLER_H
#define ASYNCMESSAGEHANDLER_H

#include <QObject>
#include <QCache>
#include "websocketmessage.h"

class Client;

class AsyncMessageHandler : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Client *client READ client WRITE setClient NOTIFY clientChanged)
	Q_PROPERTY(bool pending READ pending WRITE setPending NOTIFY pendingChanged)

public:
	explicit AsyncMessageHandler(QObject *parent = nullptr);
	explicit AsyncMessageHandler(Client *client);
	virtual ~AsyncMessageHandler();

	Client *client() const;
	void setClient(Client *newClient);

	Q_INVOKABLE void sendRequest(const WebSocketMessage::ClassHandler &classHandler, const QJsonObject &json);
	Q_INVOKABLE void sendRequestFunc(const WebSocketMessage::ClassHandler &classHandler, const QString &func,
									 const QJsonObject &json = QJsonObject());
	void sendRequest(const WebSocketMessage::ClassHandler &classHandler, const char *func, QJsonObject json = QJsonObject());

	void handleMessage(const WebSocketMessage &message);

	bool pending() const;
	void setPending(bool newPending);

	static bool checkStatus(const QJsonObject &json, const QString &status = QStringLiteral("ok"));

protected:
	virtual bool prehandleMessage(const WebSocketMessage &/*message*/) { return true; }

signals:
	void clientChanged();
	void pendingChanged();
	void invalidMessageReceived(const WebSocketMessage &message, const WebSocketMessage &orig);

protected:
	Client *m_client = nullptr;

private:
	QHash<int, WebSocketMessage> m_messages;
	QCache<int, WebSocketMessage> m_repliedMessages;
	bool m_pending = false;

};

#endif // ASYNCMESSAGEHANDLER_H
