/*
 * ---- Call of Suli ----
 *
 * serversettings.cpp
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerSettings
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

#include "serversettings.h"
#include "Logger.h"
#include <QByteArray>
#include <QTextStream>

ServerSettings::ServerSettings()
{

}


/**
 * @brief ServerSettings::printConfig
 */

void ServerSettings::printConfig() const
{
	LOG_CINFO("service") << "Configuration:";
	LOG_CINFO("service") << "-----------------------------------------------------";
	LOG_CINFO("service") << "Directory:" << m_dataDir.canonicalPath();
	LOG_CINFO("service") << "Host address:" << m_listenAddress;
	LOG_CINFO("service") << "Port:" << m_listenPort;
	LOG_CINFO("service") << "SSL:" << m_ssl;
	LOG_CDEBUG("service") << "JWT secret:" << m_jwtSecret;
	LOG_CINFO("service") << "-----------------------------------------------------";
}


/**
 * @brief ServerSettings::dataDir
 * @return
 */

const QDir &ServerSettings::dataDir() const
{
	return m_dataDir;
}

void ServerSettings::setDataDir(const QDir &newDataDir)
{
	m_dataDir = newDataDir;
}

bool ServerSettings::ssl() const
{
	return m_ssl;
}

void ServerSettings::setSsl(bool newSsl)
{
	m_ssl = newSsl;
}

const QHostAddress &ServerSettings::listenAddress() const
{
	return m_listenAddress;
}

void ServerSettings::setListenAddress(const QHostAddress &newListenAddress)
{
	m_listenAddress = newListenAddress;
}

quint16 ServerSettings::listenPort() const
{
	return m_listenPort;
}

void ServerSettings::setListenPort(quint16 newListenPort)
{
	m_listenPort = newListenPort;
}

const QString &ServerSettings::jwtSecret() const
{
	return m_jwtSecret;
}

void ServerSettings::setJwtSecret(const QString &newJwtSecret)
{
	m_jwtSecret = newJwtSecret;
}

