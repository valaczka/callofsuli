/*
 * ---- Call of Suli ----
 *
 * abstracthandler.cpp
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

#include "abstracthandler.h"
#include "Logger.h"
#include "client.h"

AbstractHandler::AbstractHandler(Client *client)
	: QObject(client)
	, m_client(client)
{
	Q_ASSERT(m_client);
}


/**
 * @brief AbstractHandler::handleMessage
 * @param message
 */

void AbstractHandler::handleMessage(const WebSocketMessage &message)
{
	m_message = message;

	switch (message.opCode()) {
	case WebSocketMessage::Request:
		handleRequest();
		break;

	case WebSocketMessage::RequestResponse:
		handleRequestResponse();
		break;

	case WebSocketMessage::Event:
		handleEvent();
		break;

	default:
		LOG_CWARNING("client") << "Invalid message:" << message;
		break;
	}

	m_message = WebSocketMessage();
}


/**
 * @brief AbstractHandler::send
 */

void AbstractHandler::send(const WebSocketMessage &message)
{
	if (m_client)
		m_client->send(message);
}

ServerService *AbstractHandler::service() const
{
	if (m_client)
		return m_client->service();
	else
		return nullptr;
}
