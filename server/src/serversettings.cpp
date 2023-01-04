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
#include "qsettings.h"
#include <QByteArray>
#include <QTextStream>
#include "utils.h"

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
	LOG_CINFO("service") << "SSL certificate:" << m_certFile;
	LOG_CINFO("service") << "SSL certificate key:" << m_certKeyFile;
	LOG_CINFO("service") << "Google OAuth2 listening address:" << m_googleListenAddress;
	LOG_CINFO("service") << "Google OAuth2 listening port:" << m_googleListenPort;
	LOG_CINFO("service") << "-----------------------------------------------------";
}



/**
 * @brief ServerSettings::loadFromFile
 * @param filename
 */

void ServerSettings::loadFromFile(const QString &filename)
{
	const QString &f = m_dataDir.absoluteFilePath(filename.isEmpty() ? QStringLiteral("config.ini") : filename);

	if (!QFile::exists(f)) {
		LOG_CWARNING("service") << "Config file doesn't exists: " << f.toUtf8().constData();
		return;
	}

	QSettings s(f, QSettings::IniFormat);

	if (s.contains(QStringLiteral("server/host")))
		setListenAddress(QHostAddress(s.value(QStringLiteral("server/host")).toString()));

	if (s.contains(QStringLiteral("server/port")))
		setListenPort(s.value(QStringLiteral("server/port")).toInt());

	if (s.contains(QStringLiteral("server/jwtSecret")))
		setJwtSecret(s.value(QStringLiteral("server/jwtSecret")).toString());


	if (s.contains(QStringLiteral("ssl/enabled")))
		setSsl(s.value(QStringLiteral("ssl/enabled")).toBool());

	if (s.contains(QStringLiteral("ssl/certificate")))
		setCertFile(s.value(QStringLiteral("ssl/certificate")).toString());

	if (s.contains(QStringLiteral("ssl/key")))
		setCertKeyFile(s.value(QStringLiteral("ssl/key")).toString());


	if (s.contains(QStringLiteral("google/host")))
		setGoogleListenAddress(QHostAddress(s.value(QStringLiteral("google/host")).toString()));


	if (s.contains(QStringLiteral("google/port")))
		setGoogleListenPort(s.value(QStringLiteral("google/port")).toInt());

	if (s.contains(QStringLiteral("google/client")))
		setGoogleClientId(s.value(QStringLiteral("google/client")).toString());

	if (s.contains(QStringLiteral("google/secret")))
		setGoogleClientKey(s.value(QStringLiteral("google/secret")).toString());


	LOG_CINFO("service") << "Configuration loaded from:" << f;
}



/**
 * @brief ServerSettings::saveToFile
 * @param filename
 */

void ServerSettings::saveToFile(const QString &filename) const
{
	saveToFile(false, filename);
}



/**
 * @brief ServerSettings::saveToFile
 * @param filename
 */

void ServerSettings::saveToFile(const bool &forced, const QString &filename) const
{
	const QString &f = m_dataDir.absoluteFilePath(filename.isEmpty() ? QStringLiteral("config.ini") : filename);

	if (QFile::exists(f) && !forced) {
		LOG_CTRACE("service") << "Configuration file exists, save skipped:" << f;
		return;
	}

	QSettings s(f, QSettings::IniFormat);

	s.setValue(QStringLiteral("server/host"), m_listenAddress.toString());
	s.setValue(QStringLiteral("server/port"), m_listenPort);
	s.setValue(QStringLiteral("server/jwtSecret"), m_jwtSecret);

	s.setValue(QStringLiteral("ssl/enabled"), m_ssl);
	s.setValue(QStringLiteral("ssl/certificate"), m_certFile);
	s.setValue(QStringLiteral("ssl/key"), m_certKeyFile);


	s.setValue(QStringLiteral("google/host"), m_googleListenAddress.toString());
	s.setValue(QStringLiteral("google/port"), m_googleListenPort);
	s.setValue(QStringLiteral("google/client"), m_googleClientId);
	s.setValue(QStringLiteral("google/secret"), m_googleClientKey);

	LOG_CINFO("service") << "Configuration saved to:" << f;
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


/**
 * @brief ServerSettings::generateJwtSecret
 * @param forced
 */

bool ServerSettings::generateJwtSecret(const bool &forced)
{
	if (m_jwtSecret.isEmpty() || forced) {
		LOG_CTRACE("service") << "Generate new JWT secret";
		setJwtSecret(QString::fromUtf8(Utils::generateRandomString(24)));
		return true;
	}

	return false;
}

const QString &ServerSettings::googleClientId() const
{
	return m_googleClientId;
}

void ServerSettings::setGoogleClientId(const QString &newGoogleClientId)
{
	m_googleClientId = newGoogleClientId;
}

const QString &ServerSettings::googleClientKey() const
{
	return m_googleClientKey;
}

void ServerSettings::setGoogleClientKey(const QString &newGoogleClientKey)
{
	m_googleClientKey = newGoogleClientKey;
}

const QHostAddress &ServerSettings::googleListenAddress() const
{
	return m_googleListenAddress;
}

void ServerSettings::setGoogleListenAddress(const QHostAddress &newGoogleListenAddress)
{
	m_googleListenAddress = newGoogleListenAddress;
}

quint16 ServerSettings::googleListenPort() const
{
	return m_googleListenPort;
}

void ServerSettings::setGoogleListenPort(quint16 newGoogleListenPort)
{
	m_googleListenPort = newGoogleListenPort;
}

const QString &ServerSettings::certFile() const
{
	return m_certFile;
}

void ServerSettings::setCertFile(const QString &newCertFile)
{
	m_certFile = newCertFile;
}

const QString &ServerSettings::certKeyFile() const
{
	return m_certKeyFile;
}

void ServerSettings::setCertKeyFile(const QString &newCertKeyFile)
{
	m_certKeyFile = newCertKeyFile;
}

