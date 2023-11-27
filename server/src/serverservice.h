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
//#include "panel.h"
#include "abstractengine.h"
#include "qnetworkaccessmanager.h"
#include "serversettings.h"
#include "databasemain.h"
#include "webserver.h"
#include "oauth2authenticator.h"
//#include "eventstream.h"


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
 * @brief The PeerUser class
 */

class PeerUser
{
public:
	PeerUser(const QString &username, const QHostAddress &host, const QString &agent)
		: m_username(username)
		, m_host(host)
		, m_timestamp(QDateTime::currentDateTime())
		, m_agent(agent)
	{}
	PeerUser(const QString &username, const QHostAddress &host)
		: PeerUser(username, host, QStringLiteral("")) {}
	PeerUser(const QString &username, const QString &agent)
		: PeerUser(username, QHostAddress::Any, agent) {}
	PeerUser(const QString &username)
		: PeerUser(username, QHostAddress::Any) {}
	PeerUser()
		: PeerUser(QString()) {}

	static QVector<PeerUser>::iterator find(QVector<PeerUser> *list, const PeerUser &user);
	static bool addOrUpdate(QVector<PeerUser> *list, const PeerUser &user);
	static bool remove(QVector<PeerUser> *list, const PeerUser &user);
	static bool get(QVector<PeerUser> *list, const QString &username, PeerUser *user);
	static bool clear(QVector<PeerUser> *list, const qint64 &sec = 120);
	static QJsonArray toJson(const QVector<PeerUser> *list);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &familyName() const;
	void setFamilyName(const QString &newFamilyName);

	const QString &givenName() const;
	void setGivenName(const QString &newGivenName);

	const QHostAddress &host() const;
	void setHost(const QHostAddress &newHost);

	const QDateTime &timestamp() const;
	void setTimestamp(const QDateTime &newTimestamp);

	const QString &agent() const;
	void setAgent(const QString &newAgent);

	friend bool operator==(const PeerUser &u1, const PeerUser &u2) {
		return (u1.m_username == u2.m_username && u1.m_host == u2.m_host && u1.m_agent == u2.m_agent);
	}

	friend QDebug operator<<(QDebug debug, const PeerUser &user) {
		QDebugStateSaver saver(debug);
		debug.nospace() << qPrintable(QByteArrayLiteral("PeerUser("))
						<< qPrintable(user.m_username)
						<< qPrintable(QByteArrayLiteral(", "))
						<< qPrintable(user.m_host.toString())
						<< qPrintable(QByteArrayLiteral(", "))
						<< qPrintable(user.m_agent)
						<< qPrintable(QByteArrayLiteral(" @"))
						<< qPrintable(user.m_timestamp.toString())
						<< ')';
		return debug;
	}


private:
	QString m_username;
	QString m_familyName;
	QString m_givenName;
	QHostAddress m_host;
	QDateTime m_timestamp;
	QString m_agent;

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

	ServerConfig &config();

	QNetworkAccessManager *networkManager() const;

	const QString &importDb() const;

	int imitateLatency() const;
	void setImitateLatency(int newImitateLatency);

	bool logPeerUser(const PeerUser &user);
	const QVector<PeerUser> &peerUser() const;

	const QString &serverName() const;
	void setServerName(const QString &newServerName);

	std::optional<int> preStart();

	const QVector<std::shared_ptr<OAuth2Authenticator> > &authenticators() const;
	std::optional<std::weak_ptr<OAuth2Authenticator>> oauth2Authenticator(const char *type) const;

	void stop();
	void pause();
	void reload();

	const QVector<std::shared_ptr<AbstractEngine> > &engines() const;
	void engineAdd(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(AbstractEngine *engine);

	const QRecursiveMutex &mutex() const;

signals:
	void configChanged();
	void serverNameChanged();

protected:
	bool wasmLoad();
	bool wasmUnload();

private:
	static void processSignal(int sig);

	void onMainTimerTimeout();
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
	QVector<std::shared_ptr<AbstractEngine>> m_engines;

	/*QVector<QPointer<EventStream>> m_eventStreams;
	QVector<QPointer<Panel>> m_panels;*/

	std::unique_ptr<QNetworkAccessManager> m_networkManager;
	std::shared_ptr<WebServer> m_webServer;

	QString m_loadedWasmResource;
	QString m_importDb;
	int m_imitateLatency = 0;

	QTimer m_mainTimer;
	QDateTime m_mainTimerLastTick;

	QVector<PeerUser> m_peerUser;

	static ServerService *m_instance;

	QRecursiveMutex m_mutex;
};



#endif // SERVERSERVICE_H
