/*
 * ---- Call of Suli ----
 *
 * serverhandler.cpp
 *
 * Created on: 2023. 01. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerHandler
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

#include "serverhandler.h"
#include "serverservice.h"

ServerHandler::ServerHandler(Client *client)
	: AbstractHandler(client)
{

}


/**
 * @brief ServerHandler::getConfig
 */

void ServerHandler::getConfig()
{
	HANDLER_LOG_TRACE() << "Get config";
	send(m_message.createResponse(_getConfig(service())));
}


/**
 * @brief ServerHandler::_getConfig
 * @param service
 * @return
 */

QJsonObject ServerHandler::_getConfig(ServerService *service)
{
	QJsonObject c = service->config().get();

	if (!service->settings()->oauthGoogle().clientId.isEmpty() ||
			!service->settings()->oauthGoogle().clientKey.isEmpty())
		c.insert(QStringLiteral("oauthGoogle"), true);

	QJsonObject r;
	r.insert(QStringLiteral("name"), service->serverName());
	r.insert(QStringLiteral("versionMajor"), service->versionMajor());
	r.insert(QStringLiteral("versionMinor"), service->versionMinor());
	r.insert(QStringLiteral("config"), c);
	return r;
}
