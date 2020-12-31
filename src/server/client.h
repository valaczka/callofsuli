/*
 * ---- Call of Suli ----
 *
 * handler.h
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Handler
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

#include "../3rdparty/smtpclient/SmtpMime"
#include "../common/cosdb.h"
#include "../common/cosmessage.h"

class Server;

class Client : public QObject
{
	Q_OBJECT

public:

	enum ClientState { ClientInvalid, ClientUnauthorized, ClientAuthorized };
	Q_ENUM(ClientState)

	Q_PROPERTY(ClientState clientState READ clientState WRITE setClientState NOTIFY clientStateChanged)
	Q_PROPERTY(QString clientUserName READ clientUserName WRITE setClientUserName NOTIFY clientUserNameChanged)
	Q_PROPERTY(CosMessage::ClientRoles clientRoles READ clientRoles WRITE setClientRoles NOTIFY clientRolesChanged)

	explicit Client(QWebSocket *socket, Server *server, QObject *parent = nullptr);
	virtual ~Client();

	ClientState clientState() const { return m_clientState; }
	QString clientUserName() const { return m_clientUserName; }
	CosDb *db() const;
	CosDb *mapsDb() const;
	CosMessage::ClientRoles clientRoles() const { return m_clientRoles; }
	QWebSocket *socket() const { return m_socket; }
	Server *server() const { return m_server; }

	bool emailSmptClient(const QString &emailType, SmtpClient *smtpClient, QString *serverName = nullptr, QString *serverEmail = nullptr);
	bool emailPasswordReset(const QString &email, const QString &firstname, const QString &lastname, const QString &code);

	QString clientSession() const { return m_clientSession; }
	void setClientSession(const QString &clientSession);


public slots:
	void sendClientRoles();
	void sendServerInfo();
	void sendUserInfo();

	void setClientState(ClientState clientState);
	void setClientUserName(QString clientUserName);
	void setClientRoles(CosMessage::ClientRoles clientRoles);

	QStringList emailRegistrationDomainList() const;

private slots:
	void onDisconnected();
	void onBinaryMessageReceived(const QByteArray &message);
	void clientAuthorize(const CosMessage &message);
	void clientLogout(const CosMessage &message);
	bool clientPasswordRequest(const CosMessage &message);
	void updateRoles();
	void onSmtpError(SmtpClient::SmtpError e);

signals:
	void disconnected();

	void clientStateChanged(ClientState clientState);
	void clientUserNameChanged(QString clientUserName);
	void clientRolesChanged(CosMessage::ClientRoles clientRoles);

private:
	Server *m_server;
	QWebSocket *m_socket;

	ClientState m_clientState;
	QString m_clientSession;
	QString m_clientUserName;
	CosMessage::ClientRoles m_clientRoles;
};

#endif // CLIENT_H
