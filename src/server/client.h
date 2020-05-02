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
#include "../common/cossql.h"
#include "../common/maprepository.h"

class Client : public QObject
{
	Q_OBJECT

public:

	enum ClientState { ClientInvalid, ClientUnauthorized, ClientAuthorized };
	Q_ENUM(ClientState)

	enum ClientRole {
		RoleGuest = 0x01,
		RoleStudent = 0x02,
		RoleTeacher = 0x04,
		RoleAdmin = 0x08
	};
	Q_DECLARE_FLAGS(ClientRoles, ClientRole)


	Q_PROPERTY(ClientState clientState READ clientState WRITE setClientState NOTIFY clientStateChanged)
	Q_PROPERTY(QString clientUserName READ clientUserName WRITE setClientUserName NOTIFY clientUserNameChanged)
	Q_PROPERTY(ClientRoles clientRoles READ clientRoles WRITE setClientRoles NOTIFY clientRolesChanged)

	explicit Client(CosSql *database,
					MapRepository *mapDb,
					 QWebSocket *socket,
					 QObject *parent = nullptr);
	virtual ~Client();

	int nextServerMsgId();


	ClientState clientState() const { return m_clientState; }
	QString clientUserName() const { return m_clientUserName; }
	ClientRoles clientRoles() const { return m_clientRoles; }
	CosSql *db() const { return m_db; }
	MapRepository *mapDb() const { return m_mapDb; }

public slots:
	void sendError(const QString &error, const int &clientMsgId = -1);
	void sendJson(const QJsonObject &object, const int &clientMsgId = -1);
	void sendBinary(const QJsonObject &object, const QByteArray &binaryData, const int &clientMsgId = -1);
	void sendClientRoles(const ClientRoles &clientRoles);

	void setClientState(ClientState clientState);
	void setClientUserName(QString clientUserName);
	void setClientRoles(ClientRoles clientRoles);

private slots:
	void onDisconnected();
	void onBinaryMessageReceived(const QByteArray &message);
	void clientAuthorize(const QJsonObject &data, const int &clientMsgId = -1);
	void clientLogout(const QJsonObject &data);
	bool clientPasswordRequest(const QJsonObject &data, const int &clientMsgId);
	void updateRoles();

	void onSmtpError(SmtpClient::SmtpError e);

signals:
	void disconnected();

	void clientStateChanged(ClientState clientState);
	void clientUserNameChanged(QString clientUserName);
	void clientRolesChanged(ClientRoles clientRoles);

private:
	void parseJson(const QByteArray &jsonData, const int &clientMsgId, const int &serverMsgId, const QByteArray &binaryData);
	bool emailPasswordReset(const QString &email, const QString &firstname, const QString &lastname, const QString &code);

private:
	CosSql *m_db;
	MapRepository* m_mapDb;
	QWebSocket *m_socket;
	int m_serverMsgId;

	ClientState m_clientState;
	QString m_clientSession;
	QString m_clientUserName;
	ClientRoles m_clientRoles;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Client::ClientRoles);

#endif // CLIENT_H
