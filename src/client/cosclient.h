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

class Client : public QObject
{
	Q_OBJECT


public:
	enum ConnectionState { Standby, Connecting, Connected, Disconnected, Reconnecting, Reconnected, Closing };
	Q_ENUM(ConnectionState)

	enum Role {
		Guest = 0x01,
		Student = 0x02,
		Teacher = 0x04,
		Admin = 0x08
	};
	Q_DECLARE_FLAGS(Roles, Role)

	Q_PROPERTY(QWebSocket * socket READ socket WRITE setSocket NOTIFY socketChanged)
	Q_PROPERTY(ConnectionState connectionState READ connectionState WRITE setConnectionState NOTIFY connectionStateChanged)

	Q_PROPERTY(QString sessionToken READ sessionToken WRITE setSessionToken NOTIFY sessionTokenChanged)
	Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
	Q_PROPERTY(Roles userRoles READ userRoles WRITE setUserRoles NOTIFY userRolesChanged)


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
	void login(const QString &username, const QString &session, const QString &password = "");

	int socketNextClientMsgId();
	int socketSend(const QString &msgType, const QByteArray &data, const int &serverMsgId = -1);
	int socketSendJson(const QJsonObject &jsonObject);
	void setUserName(QString userName);
	void setUserRoles(Roles userRoles);
	void setSessionToken(QString sessionToken);

private slots:
	void setSocket(QWebSocket * socket);
	void socketPing();

	bool parseJson(const QJsonObject &object);

	void onSocketConnected();
	void onSocketDisconnected();
	void onSocketBinaryMessageReceived(const QByteArray &message);
	void onSocketSslErrors(const QList<QSslError> &errors);
	void onSocketStateChanged(QAbstractSocket::SocketState state);
	void onSocketServerError(const QString &error);


signals:
	void messageSent(const QString &type,
					 const QString &title,
					 const QString &informativeText,
					 const QString &detailedText);
	void jsonReceived(const QJsonObject &json);

	void socketChanged(QWebSocket * socket);
	void connectionStateChanged(ConnectionState connectionState);
	void userNameChanged(QString userName);
	void userRolesChanged(Roles userRoles);
	void sessionTokenChanged(QString sessionToken);

private:
	QWebSocket* m_socket;
	QTimer* m_timer;
	QUrl m_connectedUrl;
	int m_clientMsgId;

	ConnectionState m_connectionState;
	QString m_userName;
	Roles m_userRoles;
	QString m_sessionToken;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(Client::Roles);

#endif // CLIENT_H
