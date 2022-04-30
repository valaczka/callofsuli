/*
 * ---- Call of Suli ----
 *
 * serverobject.cpp
 *
 * Created on: 2021. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerObject
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

#include "serverobject.h"

ServerObject::ServerObject(QObject *parent)
	: ObjectListModelObject{parent}
	, m_id(0)
	, m_port(0)
	, m_ssl(false)
	, m_autoconnect(false)
	, m_broadcast(false)
	, m_ignoredErrors()
	, m_hasErrorAndNotified(false)
{

}

int ServerObject::id() const
{
	return m_id;
}

void ServerObject::setId(int newId)
{
	if (m_id == newId)
		return;
	m_id = newId;
	emit idChanged();
}


const QString &ServerObject::host() const
{
	return m_host;
}

void ServerObject::setHost(const QString &newHost)
{
	if (m_host == newHost)
		return;
	m_host = newHost;
	emit hostChanged();
}

int ServerObject::port() const
{
	return m_port;
}

void ServerObject::setPort(int newPort)
{
	if (m_port == newPort)
		return;
	m_port = newPort;
	emit portChanged();
}

bool ServerObject::ssl() const
{
	return m_ssl;
}

void ServerObject::setSsl(bool newSsl)
{
	if (m_ssl == newSsl)
		return;
	m_ssl = newSsl;
	emit sslChanged();
}

const QString &ServerObject::username() const
{
	return m_username;
}

void ServerObject::setUsername(const QString &newUsername)
{
	if (m_username == newUsername)
		return;
	m_username = newUsername;
	emit usernameChanged();
}

const QString &ServerObject::session() const
{
	return m_session;
}

void ServerObject::setSession(const QString &newSession)
{
	if (m_session == newSession)
		return;
	m_session = newSession;
	emit sessionChanged();
}

bool ServerObject::autoconnect() const
{
	return m_autoconnect;
}

void ServerObject::setAutoconnect(bool newAutoconnect)
{
	if (m_autoconnect == newAutoconnect)
		return;
	m_autoconnect = newAutoconnect;
	emit autoconnectChanged();
}

bool ServerObject::broadcast() const
{
	return m_broadcast;
}

void ServerObject::setBroadcast(bool newBroadcast)
{
	if (m_broadcast == newBroadcast)
		return;
	m_broadcast = newBroadcast;
	emit broadcastChanged();
}

const QVariantList &ServerObject::ignoredErrors() const
{
	return m_ignoredErrors;
}

void ServerObject::setIgnoredErrors(const QVariantList &newIgnoredErrors)
{
	if (m_ignoredErrors == newIgnoredErrors)
		return;
	m_ignoredErrors = newIgnoredErrors;
	emit ignoredErrorsChanged();
}

bool ServerObject::hasErrorAndNotified() const
{
	return m_hasErrorAndNotified;
}

void ServerObject::setHasErrorAndNotified(bool newHasErrorAndNotified)
{
	m_hasErrorAndNotified = newHasErrorAndNotified;
}
