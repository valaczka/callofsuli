/*
 * ---- Call of Suli ----
 *
 * examengine.h
 *
 * Created on: 2022. 08. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ExamEngine
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

#ifndef EXAMENGINE_H
#define EXAMENGINE_H

#include <QObject>
#include "cosmessage.h"

class Server;
class Client;

class ExamEngine : public QObject
{
	Q_OBJECT

public:
	explicit ExamEngine(Server *server, const int &engineId, const QString &owner);
	virtual ~ExamEngine();

	void run(Client *client, const CosMessage &message);

	Server *server() const;
	int engineId() const;

	const QString &owner() const;

	void clientAdd(Client *client);
	void clientRemove(Client *client);

public slots:
	//bool getServerInfo(QJsonObject *jsonResponse, QByteArray *);

private:
	Server *m_server = nullptr;
	int m_engineId = -1;
	QString m_owner = "";
	QList<QPointer<Client>> m_clients;

};


#endif // EXAMENGINE_H
