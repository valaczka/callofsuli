/*
 * ---- Call of Suli ----
 *
 * serverhandler.h
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

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include "abstracthandler.h"

class ServerHandler : public AbstractHandler
{
	Q_OBJECT

public:
	explicit ServerHandler(Client *client);

protected:
	virtual void handleRequestResponse() {};
	virtual void handleEvent() {};

private slots:
	void getConfig();

private:
	static QJsonObject _getConfig(ServerService *service);

	friend class ServerService;
};

#endif // SERVERHANDLER_H
