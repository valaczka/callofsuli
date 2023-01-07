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

#include "credential.h"
#include "databasemain.h"
#include "websocketmessage.h"
#include <QObject>

#define HANDLER_DESTROY_TIMEOUT_MSEC	180*1000

class Client;
class ServerService;

class AbstractHandler : public QObject
{
	Q_OBJECT

public:
	AbstractHandler(Client *client);
	virtual ~AbstractHandler();

	void handleMessage(const WebSocketMessage &message);

	bool validateJwtToken(const bool &autoResponseOnFail = true);
	bool validateJwtToken(const Credential::Role &role, const bool &autoResponseOnFail = true);

	const Credential &credential() const;
	void setCredential(const Credential &newCredential);

	DatabaseMain *databaseMain() const;
	ServerService *service() const;

protected:
	virtual void handleRequest();
	virtual void handleRequestResponse() = 0;
	virtual void handleEvent() = 0;

	virtual void clear();

	void send(const WebSocketMessage &message, const bool &deleteHandler = true);

	const QJsonObject &json() const { return m_message.data(); }

	WebSocketMessage m_message;
	Client *const m_client;

private slots:
	void destroyTimeout();

private:
	bool m_jwtTokenValidated = false;
	Credential m_credential;
};

#endif // ABSTRACTHANDLER_H
