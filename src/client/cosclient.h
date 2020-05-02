/*
 * ---- Call of Suli ----
 *
 * client.h
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Client
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


#ifndef CLIENT_H
#define CLIENT_H

#include <QWebSocket>
#include <QObject>
#include <QQuickWindow>
#include <QSettings>
#include <QJsonObject>
#include "../version/buildnumber.h"

class Client : public QObject
{
	Q_OBJECT


public:
	enum ConnectionState { Standby, Connecting, Connected, Disconnected, Reconnecting, Reconnected, Closing };
	Q_ENUM(ConnectionState)

	enum RoleFlag {
		RoleGuest = 0x01,
		RoleStudent = 0x02,
		RoleTeacher = 0x04,
		RoleAdmin = 0x08
	};
	Q_ENUM (RoleFlag)
	Q_DECLARE_FLAGS(Roles, RoleFlag)
	Q_FLAGS(Roles)


	Q_PROPERTY(QWebSocket * socket READ socket WRITE setSocket NOTIFY socketChanged)
	Q_PROPERTY(ConnectionState connectionState READ connectionState WRITE setConnectionState NOTIFY connectionStateChanged)
	Q_PROPERTY(int clientVersionMajor READ clientVersionMajor NOTIFY clientVersionMajorChanged)
	Q_PROPERTY(int clientVersionMinor READ clientVersionMinor NOTIFY clientVersionMinorChanged)

	Q_PROPERTY(QString serverDataDir READ serverDataDir WRITE setServerDataDir NOTIFY serverDataDirChanged)
	Q_PROPERTY(QString sessionToken READ sessionToken WRITE setSessionToken NOTIFY sessionTokenChanged)
	Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)
	Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
	Q_PROPERTY(Roles userRoles READ userRoles WRITE setUserRoles NOTIFY userRolesChanged)
	Q_PROPERTY(int userXP READ userXP WRITE setUserXP NOTIFY userXPChanged)
	Q_PROPERTY(int userRank READ userRank WRITE setUserRank NOTIFY userRankChanged)
	Q_PROPERTY(QString userRankName READ userRankName WRITE setUserRankName NOTIFY userRankNameChanged)
	Q_PROPERTY(QString userFirstName READ userFirstName WRITE setUserFirstName NOTIFY userFirstNameChanged)
	Q_PROPERTY(QString userLastName READ userLastName WRITE setUserLastName NOTIFY userLastNameChanged)


	explicit Client(QObject *parent = nullptr);
	virtual ~Client();


	static bool registerResource(const QString &filename = "callofsuli.rcc");
	static void registerTypes();
	static void initialize();
	static void standardPathCreate();

	Q_INVOKABLE void windowSaveGeometry(QQuickWindow *window);
	Q_INVOKABLE void windowRestoreGeometry(QQuickWindow *window, const bool &forceFullscreen = false);
	Q_INVOKABLE void windowSetIcon(QQuickWindow *window);

	Q_INVOKABLE static QString standardPath(const QString &path = QString());
	Q_INVOKABLE static void setSetting(const QString &key, const QVariant &value);
	Q_INVOKABLE static QVariant getSetting(const QString &key);

	QWebSocket * socket() const { return m_socket; }
	ConnectionState connectionState() const { return m_connectionState; }
	QString userName() const { return m_userName; }
	Roles userRoles() const { return m_userRoles; }
	QString sessionToken() const { return m_sessionToken; }
	int userXP() const { return m_userXP; }
	int userRank() const { return m_userRank; }
	QString userRankName() const { return m_userRankName; }
	QString userFirstName() const { return m_userFirstName; }
	QString userLastName() const { return m_userLastName; }
	QString serverName() const { return m_serverName; }
	static int clientVersionMajor() { return _VERSION_MAJOR; }
	static int clientVersionMinor() { return _VERSION_MINOR; }
	QString serverDataDir() const { return m_serverDataDir; }

public slots:
	void sendMessageWarning(const QString &title, const QString &informativeText, const QString &detailedText = "") {
		emit messageSent("warning", title, informativeText, detailedText);
	}
	void sendMessageError(const QString &title, const QString &informativeText, const QString &detailedText = "") {
		emit messageSent("error", title, informativeText, detailedText);
	}
	void sendMessageInfo(const QString &title, const QString &informativeText, const QString &detailedText = "") {
		emit messageSent("info", title, informativeText, detailedText);
	}
	void sendDatabaseError(const QString &informativeText) {
		emit messageSent("error", tr("Adatbázis hiba"), informativeText, "");
	}

	void setConnectionState(ConnectionState connectionState);
	void closeConnection();
	void login(const QString &username, const QString &session, const QString &password = "", const bool &isPasswordReset = false);
	void logout();
	void passwordRequest(const QString &email, const QString &code = "");

	int socketNextClientMsgId();
	int socketSend(const QJsonObject &jsonObject, const QByteArray &binaryData = QByteArray(), const int &serverMsgId = -1);
	void setServerDataDir(QString serverDataDir);





private slots:
	void setSocket(QWebSocket * socket);
	void socketPing();

	void parseJson(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId);
	QByteArray prepareJson(const QJsonObject &jsonObject);

	void onSocketConnected();
	void onSocketDisconnected();
	void onSocketBinaryMessageReceived(const QByteArray &message);
	void onSocketSslErrors(const QList<QSslError> &errors);
	void onSocketStateChanged(QAbstractSocket::SocketState state);
	void onSocketServerError(const QString &error);

	void onJsonUserInfoReceived(const QJsonObject &object, const QByteArray &, const int &);


	void setUserName(QString userName);
	void setUserRoles(Roles userRoles);
	void setSessionToken(QString sessionToken);
	void setUserXP(int userXP);
	void setUserRank(int userRank);
	void setUserRankName(QString userRankName);
	void setUserFirstName(QString userFirstName);
	void setUserLastName(QString userLastName);
	void setServerName(QString serverName);

signals:
	void messageSent(const QString &type,
					 const QString &title,
					 const QString &informativeText,
					 const QString &detailedText);
	void reconnecting();

	void authInvalid();
	void authRequirePasswordReset();
	void authPasswordResetSuccess();

	void jsonUserInfoReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId);
	void jsonTeacherMapsReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId);
	void jsonUserReceived(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId);

	void socketChanged(QWebSocket * socket);
	void connectionStateChanged(ConnectionState connectionState);
	void userNameChanged(QString userName);
	void userRolesChanged(Roles userRoles);
	void sessionTokenChanged(QString sessionToken);
	void userXPChanged(int userXP);
	void userRankChanged(int userRank);
	void userRankNameChanged(QString userRankName);
	void userFirstNameChanged(QString userFirstName);
	void userLastNameChanged(QString userLastName);
	void serverNameChanged(QString serverName);
	void clientVersionMajorChanged(int clientVersionMajor);
	void clientVersionMinorChanged(int clientVersionMinor);
	void serverDataDirChanged(QString serverDataDir);

private:
	QWebSocket* m_socket;
	QTimer* m_timer;
	QUrl m_connectedUrl;
	int m_clientMsgId;
	QHash<QString, QString> m_signalList;

	ConnectionState m_connectionState;
	QString m_userName;
	Roles m_userRoles;
	QString m_sessionToken;
	int m_userXP;
	int m_userRank;
	QString m_userFirstName;
	QString m_userLastName;
	QString m_serverName;
	int m_clientVersionMajor;
	int m_clientVersionMinor;
	QString m_serverDataDir;
	QString m_userRankName;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(Client::Roles);

#endif // CLIENT_H
