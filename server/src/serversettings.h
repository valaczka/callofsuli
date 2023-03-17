/*
 * ---- Call of Suli ----
 *
 * serversettings.h
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

#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H

#include "qdir.h"
#include "qhostaddress.h"
#include "qsettings.h"
#include <QString>

class ServerSettings
{
public:
	ServerSettings();

	/**
	 * @brief The OAuth class
	 */

	struct OAuth {
		QString clientId;
		QString clientKey;
		QString path;
		QString localClientId;
		QString localClientKey;

		static OAuth fromSettings(QSettings *settings, const QString &group = "");
		void toSettings(QSettings *settings, const QString &group = "") const;
	};

	void printConfig() const;
	void loadFromFile(const QString &filename = "");
	void saveToFile(const QString &filename = "") const;
	void saveToFile(const bool &forced, const QString &filename = "") const;


	const QDir &dataDir() const;
	void setDataDir(const QDir &newDataDir);

	bool ssl() const;
	void setSsl(bool newSsl);

	const QHostAddress &listenAddress() const;
	void setListenAddress(const QHostAddress &newListenAddress);

	quint16 listenPort() const;
	void setListenPort(quint16 newListenPort);

	const QString &jwtSecret() const;
	void setJwtSecret(const QString &newJwtSecret);

	bool generateJwtSecret(const bool &forced = false);

	const QString &certFile() const;
	void setCertFile(const QString &newCertFile);

	const QString &certKeyFile() const;
	void setCertKeyFile(const QString &newCertKeyFile);

	const OAuth &oauthGoogle() const;
	void setOauthGoogle(const OAuth &newOauthGoogle);

	const QString &redirectHost() const;
	void setRedirectHost(const QString &newRedirectHost);

private:
	QDir m_dataDir;

	QString m_jwtSecret;

	QHostAddress m_listenAddress = QHostAddress::Any;
	quint16 m_listenPort = 10101;
	QString m_redirectHost;

	bool m_ssl = false;
	QString m_certFile;
	QString m_certKeyFile;

	OAuth m_oauthGoogle;

};

#endif // SERVERSETTINGS_H
