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

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "QOlm/QOlm.hpp"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#include "qdir.h"
#include "qjsonobject.h"
#ifndef QT_NO_SSL
#include "qsslerror.h"
#endif
#include "qurl.h"
#include <selectableobject.h>
#include "user.h"

class Server;

using ServerList = qolm::QOlm<Server>;
Q_DECLARE_METATYPE(ServerList*)

class Server : public SelectableObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QDir directory READ directory WRITE setDirectory NOTIFY directoryChanged)
	Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged)
	Q_PROPERTY(QString token READ token WRITE setToken NOTIFY tokenChanged)
	Q_PROPERTY(QByteArray certificate READ certificate WRITE setCertificate NOTIFY certificateChanged)
	Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)
	Q_PROPERTY(QJsonObject config READ config NOTIFY configChanged)
	Q_PROPERTY(User *user READ user CONSTANT)
	Q_PROPERTY(QList<Rank> rankList READ rankList NOTIFY rankListChanged)
	Q_PROPERTY(bool temporary READ temporary WRITE setTemporary NOTIFY temporaryChanged)
	Q_PROPERTY(int maxUploadSize READ maxUploadSize WRITE setMaxUploadSize NOTIFY maxUploadSizeChanged)

#ifndef QT_NO_SSL
	Q_PROPERTY(QList<QSslError::SslError> ignoredSslErrors READ ignoredSslErrors WRITE setIgnoredSslErrors NOTIFY ignoredSslErrorsChanged)
#endif

public:
	explicit Server(QObject *parent = nullptr);
	virtual ~Server();

	static Server *fromJson(const QJsonObject &data, QObject *parent = nullptr);
	QJsonObject toJson() const;

	const QUrl &url() const;
	void setUrl(const QUrl &newUrl);

	const QDir &directory() const;
	void setDirectory(const QDir &newDirectory);

	bool autoConnect() const;
	void setAutoConnect(bool newAutoConnect);

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

	const QString &serverName() const;
	void setServerName(const QString &newServerName);

	Q_INVOKABLE QString host() const { return m_url.host(); }
	Q_INVOKABLE int port() const { return m_url.port(); }
	Q_INVOKABLE bool ssl() const { return m_url.scheme() == QLatin1String("https"); }

	const QJsonObject &config() const;
	void setConfig(const QJsonObject &newConfig);

	User *user() const;

	QList<Rank> rankList() const { return m_rankList.toList(); }
	void setRankList(const RankList &newRankList);

	static bool isTokenValid(const QString &jwt);
	bool isTokenValid() const { return isTokenValid(m_token); }

	Q_INVOKABLE Rank rank(const int &id) const;
	Q_INVOKABLE Rank nextRank(const Rank &rank) const { return m_rankList.next(rank); }

	bool temporary() const;
	void setTemporary(bool newTemporary);

	int maxUploadSize() const;
	void setMaxUploadSize(int newMaxUploadSize);

signals:
	void urlChanged();
	void directoryChanged();
	void autoConnectChanged();
	void tokenChanged();
	void certificateChanged();
	void ignoredSslErrorsChanged();
	void nameChanged();
	void serverNameChanged();
	void configChanged();
	void rankListChanged();
	void temporaryChanged();

	void maxUploadSizeChanged();

private:
	QString m_name;
	QString m_serverName;
	QUrl m_url;
	QDir m_directory;
	bool m_autoConnect = false;
	QString m_token;
	User *const m_user;
	RankList m_rankList;
	bool m_temporary = false;
	int m_maxUploadSize = 0;

#ifndef QT_NO_SSL
	QList<QSslError::SslError> m_ignoredSslErrors;
#endif
	QByteArray m_certificate;
	QJsonObject m_config;
};



#endif // SERVER_H
