/*
 * ---- Call of Suli ----
 *
 * serverobject.h
 *
 * Created on: 2021. 12. 23.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ServerObject
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

#ifndef SERVEROBJECT_H
#define SERVEROBJECT_H

#include "objectlistmodelobject.h"
#include "objectlistmodel.h"

class ServerObject : public ObjectListModelObject
{
	Q_OBJECT

	Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
	Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
	Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
	Q_PROPERTY(bool ssl READ ssl WRITE setSsl NOTIFY sslChanged)
	Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
	Q_PROPERTY(QString session READ session WRITE setSession NOTIFY sessionChanged)
	Q_PROPERTY(bool autoconnect READ autoconnect WRITE setAutoconnect NOTIFY autoconnectChanged)
	Q_PROPERTY(bool broadcast READ broadcast WRITE setBroadcast NOTIFY broadcastChanged STORED false)
	Q_PROPERTY(QList<int> ignoredErrors READ ignoredErrors WRITE setIgnoredErrors NOTIFY ignoredErrorsChanged)

public:
	Q_INVOKABLE explicit ServerObject(QObject *parent = nullptr);

	int id() const;
	void setId(int newId);

	const QString &host() const;
	void setHost(const QString &newHost);

	int port() const;
	void setPort(int newPort);

	bool ssl() const;
	void setSsl(bool newSsl);

	const QString &username() const;
	void setUsername(const QString &newUsername);

	const QString &session() const;
	void setSession(const QString &newSession);

	bool autoconnect() const;
	void setAutoconnect(bool newAutoconnect);

	bool broadcast() const;
	void setBroadcast(bool newBroadcast);

	const QList<int> &ignoredErrors() const;
	void setIgnoredErrors(const QList<int> &newIgnoredErrors);

signals:
	void idChanged();
	void hostChanged();
	void portChanged();
	void sslChanged();
	void usernameChanged();
	void sessionChanged();
	void autoconnectChanged();

	void broadcastChanged();

	void ignoredErrorsChanged();

private:
	int m_id;
	QString m_host;
	int m_port;
	bool m_ssl;
	QString m_username;
	QString m_session;
	bool m_autoconnect;
	bool m_broadcast;
	QList<int> m_ignoredErrors;
};

Q_DECLARE_METATYPE(ObjectGenericListModel<ServerObject>*);

#endif // SERVEROBJECT_H
