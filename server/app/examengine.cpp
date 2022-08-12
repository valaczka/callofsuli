/*
 * ---- Call of Suli ----
 *
 * examengine.cpp
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

#include "examengine.h"
#include "server.h"
#include "client.h"

/**
 * @brief ExamEngine::ExamEngine
 * @param client
 * @param engineId
 */

ExamEngine::ExamEngine(Server *server, const int &engineId, const QString &owner)
	: QObject(server)
	, m_server(server)
	, m_engineId(engineId)
	, m_owner(owner)
{
	Q_ASSERT(server);
}


/**
 * @brief ExamEngine::~ExamEngine
 */

ExamEngine::~ExamEngine()
{

}


/**
 * @brief ExamEngine::run
 * @param message
 */

void ExamEngine::run(Client *client, const CosMessage &message)
{
	Q_ASSERT(client);

	QJsonObject jsonObject;

	const QString &func = message.cosFunc();

	/*if (!QMetaObject::invokeMethod(this, func.toStdString().data(), Qt::DirectConnection,
								   Q_RETURN_ARG(bool, returnArg),
								   Q_ARG(QJsonObject*, &jsonObject),
								   Q_ARG(QByteArray*, &binaryData))) {
		CosMessage r(CosMessage::InvalidFunction, m_message);
		r.setCosClass(CosMessage::ClassExamEngine);
		r.setCosFunc(func);
		r.send(m_client->socket());
		return;
	}*/

	CosMessage r(jsonObject, CosMessage::ClassExamEngine, func, message);

	/*if (!returnArg || m_serverError != CosMessage::ServerNoError)
		r.setMessageError(CosMessage::OtherError);

	r.setServerError(m_serverError);*/

	r.send(client->socket());
}


/**
 * @brief ExamEngine::engineId
 * @return
 */

int ExamEngine::engineId() const
{
	return m_engineId;
}


/**
 * @brief ExamEngine::owner
 * @return
 */

const QString &ExamEngine::owner() const
{
	return m_owner;
}


/**
 * @brief ExamEngine::clientAdd
 * @param client
 */

void ExamEngine::clientAdd(Client *client)
{
	Q_ASSERT(client);

	if (!m_clients.contains(client))
		m_clients.append(client);
}



/**
 * @brief ExamEngine::clientRemove
 * @param client
 */

void ExamEngine::clientRemove(Client *client)
{
	m_clients.removeAll(client);
}


/**
 * @brief ExamEngine::server
 * @return
 */

Server *ExamEngine::server() const
{
	return m_server;
}
