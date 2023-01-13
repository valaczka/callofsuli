/*
 * ---- Call of Suli ----
 *
 * server.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Server
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

#ifndef SERVER_H
#define SERVER_H

#include "QOlm/QOlm.hpp"
#include "qdir.h"
#ifndef QT_NO_SSL
#include "qsslerror.h"
#endif
#include "qurl.h"
#include <QObject>

class Server;

using ServerList = qolm::QOlm<Server>;

class Server : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QDir directory READ directory WRITE setDirectory NOTIFY directoryChanged)
	Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
	Q_PROPERTY(QByteArray certificate READ certificate WRITE setCertificate NOTIFY certificateChanged)

#ifndef QT_NO_SSL
	Q_PROPERTY(QList<QSslError::SslError> ignoredSslErrors READ ignoredSslErrors WRITE setIgnoredSslErrors NOTIFY ignoredSslErrorsChanged)
#endif

public:
	explicit Server(QObject *parent = nullptr);

	static Server *fromJson(const QJsonObject &data, QObject *parent = nullptr);
	QJsonObject toJson() const;

	const QUrl &url() const;
	void setUrl(const QUrl &newUrl);

	const QDir &directory() const;
	void setDirectory(const QDir &newDirectory);

	bool autoConnect() const;
	void setAutoConnect(bool newAutoConnect);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &token() const;
	void setToken(const QString &newToken);

	const QByteArray &certificate() const;
	void setCertificate(const QByteArray &newCertificate);

#ifndef QT_NO_SSL
	const QList<QSslError::SslError> &ignoredSslErrors() const;
	void setIgnoredSslErrors(const QList<QSslError::SslError> &newIgnoredSslErrors);
#endif

	const QString &name() const;
	void setName(const QString &newName);

signals:
	void urlChanged();
	void directoryChanged();
	void autoConnectChanged();
	void usernameChanged();
	void tokenChanged();
	void certificateChanged();
	void ignoredSslErrorsChanged();
	void nameChanged();

private:
	QString m_name;
	QUrl m_url;
	QDir m_directory;
	bool m_autoConnect = false;
	QString m_username;
	QString m_token;
#ifndef QT_NO_SSL
	QList<QSslError::SslError> m_ignoredSslErrors;
#endif
	QByteArray m_certificate;
};


Q_DECLARE_METATYPE(ServerList*)

#endif // SERVER_H
