/*
 * ---- Call of Suli ----
 *
 * server.h
 *
 * Created on: 2020. 03. 21.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Server
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QtWebSockets/QtWebSockets>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QSslSocket>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QTimer>

#include "client.h"

class ExamEngine;

class Server : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString serverDir READ serverDir WRITE setServerDir NOTIFY serverDirChanged)
	Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)
	Q_PROPERTY(QString serverUuid READ serverUuid WRITE setServerUuid NOTIFY serverUuidChanged)
	Q_PROPERTY(CosDb * db READ db CONSTANT)
	Q_PROPERTY(CosDb * mapsDb READ mapsDb CONSTANT)
	Q_PROPERTY(CosDb * statDb READ statDb CONSTANT)
	Q_PROPERTY(QVariantMap resources READ resources WRITE setResources NOTIFY resourcesChanged)

public:
	struct VersionUpgrade {
		int major;
		int minor;
		QString file;

		VersionUpgrade(const int &maj, const int &min, const QString &f) : major(maj), minor(min), file(f) {}
	};

private:
	QWebSocketServer *m_socketServer;
	QUdpSocket *m_udpSocket;
	QNetworkAccessManager m_networkAccessManager;

	QString m_serverDir;
	QString m_host;
	int m_port;
	int m_pendingConnections;
	QString m_serverName;
	QString m_socketCert;
	QString m_socketKey;
	bool m_createDb;

	QList<Client *> m_clients;
	CosDb *m_db;
	CosDb *m_mapsDb;
	CosDb *m_statDb;
	CosDb *m_examDb;

	QVariantMap m_resourceMap;
	QVariantMap m_resources;
	QString m_serverUuid;

	static QList<VersionUpgrade> m_versionUpgrades;

	QTimer *m_timer;

	QList<ExamEngine*> m_examEngineList;

public:
	explicit Server(QObject *parent = nullptr);
	virtual ~Server();

	bool start();
	void stop();
	bool commandLineParse(QCoreApplication &app);
	bool serverDirCheck();
	bool databaseLoad();
	bool rankInit();
	bool resourcesInit();

	bool websocketServerStart();
	void reloadResources();

	QString serverDir() const { return m_serverDir; }
	QString serverName() const { return m_serverName; }
	QByteArray resourceContent(const QString &filename, QString *md5 = nullptr) const;

	QWebSocketServer *socketServer() const { return m_socketServer; }
	CosDb * db() const { return m_db; }
	CosDb * mapsDb() const { return m_mapsDb; }
	CosDb * statDb() const { return m_statDb; }
	CosDb * examDb() const { return m_examDb; }
	QList<Client *> clients() const { return m_clients; }
	QVariantMap resourceMap() const { return m_resourceMap; }
	QVariantMap resources() const { return m_resources; }

	bool isClientActive(const QString &username) const;

	QString host() const { return m_host; }
	int port() const { return m_port; }
	QString serverUuid() const { return m_serverUuid; }

	QNetworkReply *networkRequestGet(const QNetworkRequest &request);
	QNetworkReply *networkRequestPost(const QNetworkRequest &request, const QByteArray &data);
	void networkRequestReply(QNetworkReply *);

	ExamEngine *examEngineNew(const int &examId, const QString &code, const QString &owner);
	ExamEngine *examEngineGet(const QString &code) const;
	ExamEngine *examEngineGet(const int &examId) const;
	bool examEngineDelete(ExamEngine *engine);
	bool examEnginesHasMember(const QString &username) const;

private slots:
	void onSslErrors(const QList<QSslError> &errors);
	void onNewConnection();
	void onSocketDisconnected();
	void onDatagramReady();
	void onTimerTimeout();

public slots:
	void setServerDir(QString serverDir);
	void setServerName(QString serverName);
	void setResources(QVariantMap resources);
	void setServerUuid(QString serverUuid);

signals:
	void serverDirChanged(QString serverDir);
	void serverNameChanged(QString serverName);
	void resourcesChanged(QVariantMap resources);
	void serverInfoChanged();
	void serverUuidChanged(QString serverUuid);
};

#endif // SERVER_H
