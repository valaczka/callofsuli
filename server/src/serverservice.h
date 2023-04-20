/*
 * ---- Call of Suli ----
 *
 * serverservice.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerService
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

#ifndef SERVERSERVICE_H
#define SERVERSERVICE_H

#include <QtService/Service>
#include <QPointer>
#include "ColorConsoleAppender.h"
#include "panel.h"
#include "serversettings.h"
#include "databasemain.h"
#include "webserver.h"
#include "oauth2authenticator.h"


class ServerService;

/**
 * @brief The ServerConfig class
 */

class ServerConfig
{
public:
	ServerConfig() {}

	QJsonValue get(const char *key) const { return m_data.value(key); }
	void set(const char *key, const QJsonValue &value);
	void set(const QJsonObject &object);
	const QJsonObject &get() const { return m_data; }


	bool registrationEnabled() const
	{ return get("registrationEnabled").toBool(false); }

	void setRegistrationEnabled(const bool &on = true)
	{ set("registrationEnabled", on); }


	bool oAuth2RegistrationForced() const
	{ return get("oauth2RegistrationForced").toBool(false); }

	void setOAuth2RegistrationForced(const bool &on = true)
	{ set("oauth2RegistrationForced", on); }


private:
	void loadFromDb(DatabaseMain *db);
	QPointer<DatabaseMain> m_db = nullptr;
	QJsonObject m_data;
	ServerService *m_service = nullptr;
	friend class ServerService;
};



/**
 * @brief The ServerService class
 */

class ServerService : public QtService::Service
{
	Q_OBJECT

	Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)

public:
	explicit ServerService(int &argc, char **argv);
	virtual ~ServerService();

	static void initialize();

	static int versionMajor();
	static int versionMinor();
	static int versionBuild();
	const char *version() const;

	ServerSettings *settings() const;
	DatabaseMain *databaseMain() const;
	WebServer *webServer() const;

	OAuth2Authenticator *oauth2Authenticator(const char *type) const;

	const QString &serverName() const;
	void setServerName(const QString &newServerName);

	ServerConfig &config();

	QVector<OAuth2Authenticator *> authenticators() const;

	QNetworkAccessManager *networkManager() const;

	QVector<HttpEventStream *> eventStreams() const;
	void addEventStream(HttpEventStream *stream);

	QVector<Panel *> panels() const;
	void addPanel(Panel *panel);
	void removePanel(Panel *panel, const bool &_delete = true);
	Panel *panel(const int &id) const;

	const QString &importDb() const;

signals:
	void configChanged();
	void serverNameChanged();

protected:
	bool preStart() override;

	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;
	CommandResult onReload() override;
	CommandResult onPause() override;
	CommandResult onResume() override;

	bool wasmLoad();
	bool wasmUnload();

protected slots:
	void terminalConnected(QtService::Terminal *terminal) override;

private:
	static const int m_versionMajor;
	static const int m_versionMinor;
	static const int m_versionBuild;
	static const char* m_version;

	QString m_serverName;

	QStringList m_arguments;

	ServerSettings *const m_settings;
	ServerConfig m_config;

	QPointer<DatabaseMain> m_databaseMain = nullptr;
	QPointer<WebServer> m_webSocketServer = nullptr;
	QVector<QPointer<OAuth2Authenticator>> m_authenticators;
	QVector<QPointer<HttpEventStream>> m_eventStreams;
	QVector<QPointer<Panel>> m_panels;

	ColorConsoleAppender *m_consoleAppender = nullptr;

	QNetworkAccessManager *const m_networkManager = nullptr;

	QString m_loadedWasmResource;
	QString m_importDb;
};



#endif // SERVERSERVICE_H
