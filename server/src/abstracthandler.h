/*
 * ---- Call of Suli ----
 *
 * abstracthandler.h
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * AbstractHandler
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

#ifndef ABSTRACTHANDLER_H
#define ABSTRACTHANDLER_H

#include "websocketmessage.h"
#include <QObject>

class Client;
class ServerService;

class AbstractHandler : public QObject
{
	Q_OBJECT

public:
	AbstractHandler(Client *client);
	virtual ~AbstractHandler() {}

	void handleMessage(const WebSocketMessage &message);

protected:
	virtual void handleRequest() = 0;
	virtual void handleRequestResponse() = 0;
	virtual void handleEvent() = 0;

	void send(const WebSocketMessage &message);

	const QJsonObject &json() const { return m_message.data(); }
	ServerService *service() const;

	WebSocketMessage m_message;
	Client *const m_client;
};

#endif // ABSTRACTHANDLER_H
