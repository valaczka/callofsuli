/*
 * ---- Call of Suli ----
 *
 * serverservice.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerService
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

#ifndef SERVERSERVICE_H
#define SERVERSERVICE_H

#include "qloggingcategory.h"
#include <QtService/Service>
#include <QPointer>
#include "serversettings.h"
#include "databasemain.h"
#include "websocketserver.h"
#include "client.h"

class ServerService : public QtService::Service
{
	Q_OBJECT

public:
	explicit ServerService(int &argc, char **argv);
	virtual ~ServerService();

	static void initialize();

	static int versionMajor();
	static int versionMinor();
	static int versionBuild();
	const char *version() const;

	ServerSettings *settings() const;
	DatabaseMain *databaseMain() const;
	WebSocketServer *webSocketServer() const;

	void clientAdd(Client *client);
	void clientRemove(Client *client);

	const QVector<QPointer<Client>> &clients() const;

protected:
	bool preStart() override;

	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;
	CommandResult onReload() override;
	CommandResult onPause() override;
	CommandResult onResume() override;

private:
	static const int m_versionMajor;
	static const int m_versionMinor;
	static const int m_versionBuild;
	static const char* m_version;

	QStringList m_arguments;

	ServerSettings *const m_settings;

	QPointer<DatabaseMain> m_databaseMain = nullptr;
	QPointer<WebSocketServer> m_webSocketServer = nullptr;
	QVector<QPointer<Client>> m_clients;
};

Q_DECLARE_LOGGING_CATEGORY(lcService)


#endif // SERVERSERVICE_H
