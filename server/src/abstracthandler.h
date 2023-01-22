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
#include "client.h"

#define HANDLER_DESTROY_TIMEOUT_MSEC	180*1000

class ServerService;

class AbstractHandler : public QObject
{
	Q_OBJECT

public:
	AbstractHandler(Client *client);
	virtual ~AbstractHandler();

	void handleMessage(const WebSocketMessage &message);

	bool validateCredential(const bool &autoResponseOnFail = true);
	bool validateCredential(const Credential::Role &role, const bool &autoResponseOnFail = true);

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
	QPointer<Client> m_client = nullptr;

	friend class AdminHandler;

private slots:
	void destroyTimeout();
};

#define HANDLER_LOG_TRACE() LOG_CTRACE("client") << m_client
#define HANDLER_LOG_DEBUG() LOG_CDEBUG("client") << m_client
#define HANDLER_LOG_INFO() LOG_CINFO("client") << m_client
#define HANDLER_LOG_WARNING() LOG_CWARNING("client") << m_client
#define HANDLER_LOG_ERROR() LOG_CERROR("client") << m_client

#endif // ABSTRACTHANDLER_H
