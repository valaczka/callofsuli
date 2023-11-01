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
#include "utils_.h"
#include "oauth2authenticator.h"



const QStringList ServerSettings::m_supportedProviders = {
	QStringLiteral("google"),
	QStringLiteral("microsoft")
};



/**
 * @brief ServerSettings::printConfig
 */

void ServerSettings::printConfig() const
{
	LOG_CINFO("service") << "Configuration:";
	LOG_CINFO("service") << "-----------------------------------------------------";
	LOG_CINFO("service") << "Directory:" << qPrintable(m_dataDir.canonicalPath());
	LOG_CINFO("service") << "Host address:" << qPrintable(m_listenAddress.toString());
	LOG_CINFO("service") << "Port:" << m_listenPort;
	LOG_CINFO("service") << "SSL:" << m_ssl;

	if (m_ssl) {
		LOG_CINFO("service") << "SSL certificate:" << qPrintable(m_certFile);
		LOG_CINFO("service") << "SSL certificate key:" << qPrintable(m_certKeyFile);
	}

	for (auto it=m_oauthMap.constBegin(); it != m_oauthMap.constEnd(); ++it) {
		if (it->clientId.isEmpty())
			continue;

		LOG_CINFO("service") << "OAuth2" << qPrintable(it.key()) << "listening:"
							 << qPrintable(QStringLiteral("/%1/").arg(OAuth2Authenticator::callbackPath())+it->path);
	}


	if (m_logLimit > 0)
		LOG_CINFO("service") << "Log limit:" << m_logLimit;

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

	if (s.contains(QStringLiteral("server/redirectHost")))
		setRedirectHost(s.value(QStringLiteral("server/redirectHost")).toString());


	if (s.contains(QStringLiteral("ssl/enabled")))
		setSsl(s.value(QStringLiteral("ssl/enabled")).toBool());

	if (s.contains(QStringLiteral("ssl/certificate")))
		setCertFile(s.value(QStringLiteral("ssl/certificate")).toString());

	if (s.contains(QStringLiteral("ssl/key")))
		setCertKeyFile(s.value(QStringLiteral("ssl/key")).toString());


	foreach (const QString &p, m_supportedProviders) {
		if (s.contains(p+QStringLiteral("/clientId")))
			m_oauthMap.insert(p, OAuth::fromSettings(&s, p));
	}


	if (s.contains(QStringLiteral("log/limit")))
		setLogLimit(s.value(QStringLiteral("log/limit")).toInt());

	LOG_CINFO("service") << "Configuration loaded from:" << qPrintable(f);
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
	s.setValue(QStringLiteral("server/redirectHost"), m_redirectHost);

	s.setValue(QStringLiteral("ssl/enabled"), m_ssl);
	s.setValue(QStringLiteral("ssl/certificate"), m_certFile);
	s.setValue(QStringLiteral("ssl/key"), m_certKeyFile);

	s.setValue(QStringLiteral("log/limit"), m_logLimit);

	for (auto it=m_oauthMap.constBegin(); it != m_oauthMap.constEnd(); ++it)
		it->toSettings(&s, it.key());

	LOG_CINFO("service") << "Configuration saved to:" << qPrintable(f);
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
	m_dataDir.setPath(newDataDir.absolutePath());
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

const QString &ServerSettings::redirectHost() const
{
	return m_redirectHost;
}

void ServerSettings::setRedirectHost(const QString &newRedirectHost)
{
	m_redirectHost = newRedirectHost;
}

int ServerSettings::logLimit() const
{
	return m_logLimit;
}

void ServerSettings::setLogLimit(int newLogLimit)
{
	m_logLimit = newLogLimit;
}




const QStringList &ServerSettings::supportedProviders()
{
	return m_supportedProviders;
}

const QHash<QString, ServerSettings::OAuth> &ServerSettings::oauthMap() const
{
	return m_oauthMap;
}

void ServerSettings::setOauthMap(const QHash<QString, OAuth> &newOauthMap)
{
	m_oauthMap = newOauthMap;
}




/**
 * @brief ServerSettings::OAuth::fromSettings
 * @param settings
 * @param group
 * @return
 */

ServerSettings::OAuth ServerSettings::OAuth::fromSettings(QSettings *settings, const QString &group)
{
	Q_ASSERT (settings);
	settings->beginGroup(group);

	ServerSettings::OAuth r;

	r.clientId = settings->value(QStringLiteral("clientId")).toString();
	r.clientKey = settings->value(QStringLiteral("clientKey")).toString();
	r.path = settings->value(QStringLiteral("path")).toString();
	r.localClientId = settings->value(QStringLiteral("localClientId")).toString();
	r.localClientKey = settings->value(QStringLiteral("localClientKey")).toString();
	r.tenant = settings->value(QStringLiteral("tenant")).toString();

	settings->endGroup();

	return r;
}


/**
 * @brief ServerSettings::OAuth::toSettings
 * @param settings
 * @param group
 */

void ServerSettings::OAuth::toSettings(QSettings *settings, const QString &group) const
{
	Q_ASSERT (settings);
	settings->beginGroup(group);

	settings->setValue(QStringLiteral("clientId"), clientId);
	settings->setValue(QStringLiteral("clientKey"), clientKey);
	settings->setValue(QStringLiteral("path"), path);
	settings->setValue(QStringLiteral("localClientId"), localClientId);
	settings->setValue(QStringLiteral("localClientKey"), localClientKey);
	settings->setValue(QStringLiteral("tenant"), tenant);

	settings->endGroup();
}

