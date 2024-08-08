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

#include <QPointer>
#include "ColorConsoleAppender.h"
#include "qnetworkaccessmanager.h"
#include "server.h"
#include "serversettings.h"
#include "databasemain.h"
#include "webserver.h"
#include "oauth2authenticator.h"
#include "enginehandler.h"
#include "rpgconfig.h"

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


	bool nameUpdateEnabled() const
	{ return get("nameUpdateEnabled").toBool(false); }

	void setNameUpdateEnabled(const bool &on = true)
	{ set("nameUpdateEnabled", on); }

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

class ServerService : public QObject
{
	Q_OBJECT

public:
	explicit ServerService(int &argc, char **argv);
	virtual ~ServerService();

	enum State {
		ServerInit,
		ServerPrepared,
		ServerRunning,
		ServerPaused,
		ServerFinished
	};

	static void initialize();

	static int versionMajor();
	static int versionMinor();
	static int versionBuild();
	const char *version() const;

	int exec();

	ServerSettings *settings() const;
	DatabaseMain *databaseMain() const;
	QLambdaThreadWorker *databaseMainWorker() const;
	std::weak_ptr<WebServer> webServer() const;
	EngineHandler *engineHandler() const { return m_engineHandler.get(); }
	SimpleMail::Server *smtpServer() const { return m_smtpServer.get(); }

	ServerConfig &config();

	QNetworkAccessManager *networkManager() const;

	const QString &importDb() const;

	int imitateLatency() const;
	void setImitateLatency(int newImitateLatency);

	const QString &serverName() const;
	void setServerName(const QString &newServerName);

	std::optional<int> preStart();

	const QVector<std::shared_ptr<OAuth2Authenticator> > &authenticators() const;
	std::optional<std::weak_ptr<OAuth2Authenticator>> oauth2Authenticator(const char *type) const;

	int mainTimerInterval() const;

	void reloadDynamicContent();

	void stop();
	void pause();
	void reload();

	const QJsonArray &dynamicContent() const;
	const QJsonObject &dynamicContentDict() const;
	const QJsonArray &loadableDynamicContent() const;

	const RpgMarketList &market() const;
	void setMarket(const RpgMarketList &newMarket);

signals:
	void configChanged();
	void serverNameChanged();

protected:
	bool wasmLoad();
	bool wasmUnload();
	virtual void timerEvent(QTimerEvent *event) override;

private:
	void loadDynamicDictFromRcc(const QString &filename, const QString &path);
	RpgMarketList loadMarket() const;
	RpgMarketList loadMarket(const QString &filename) const;
	static void processSignal(int sig);
	void loadSmtpServer();

	bool start();
	void resume();

	static const int m_versionMajor;
	static const int m_versionMinor;
	static const int m_versionBuild;
	static const char* m_version;

	QString m_serverName;
	QStringList m_arguments;
	ServerConfig m_config;

	State m_state = ServerInit;

	ColorConsoleAppender *const m_consoleAppender;

	std::unique_ptr<QCoreApplication> m_application;
	std::unique_ptr<ServerSettings> m_settings;
	std::unique_ptr<DatabaseMain> m_databaseMain;
	QVector<std::shared_ptr<OAuth2Authenticator> > m_authenticators;
	std::unique_ptr<QNetworkAccessManager> m_networkManager;
	std::shared_ptr<WebServer> m_webServer;
	std::unique_ptr<EngineHandler> m_engineHandler;
	std::unique_ptr<SimpleMail::Server> m_smtpServer;

	QString m_loadedWasmResource;
	QString m_importDb;
	QString m_forceUpgrade;
	int m_imitateLatency = 0;
	QString m_createToken;
	bool m_zap = false;

	QBasicTimer m_mainTimer;
	QDateTime m_mainTimerLastTick;
	int m_mainTimerInterval = 0;

	QJsonArray m_dynamicContent;
	QJsonArray m_loadableDynamicContent;
	QJsonObject m_dynamicContentDict;

	RpgMarketList m_market;

	static ServerService *m_instance;
};



#endif // SERVERSERVICE_H
