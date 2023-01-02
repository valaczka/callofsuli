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

#include "qcoreapplication.h"
#include "qdir.h"
#include "qhostaddress.h"
#include <QString>

class ServerSettings
{
public:
	ServerSettings();

	QByteArray printConfig() const;

	const QDir &dataDir() const;
	void setDataDir(const QDir &newDataDir);

	bool ssl() const;
	void setSsl(bool newSsl);

	const QHostAddress &listenAddress() const;
	void setListenAddress(const QHostAddress &newListenAddress);

	quint16 listenPort() const;
	void setListenPort(quint16 newListenPort);

private:
	QDir m_dataDir;
	bool m_ssl = false;
	QHostAddress m_listenAddress = QHostAddress::Any;
	quint16 m_listenPort = 10101;

};

#endif // SERVERSETTINGS_H
