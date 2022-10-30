/*
 * ---- Call of Suli ----
 *
 * examengine.h
 *
 * Created on: 2022. 08. 12.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * ExamEngine
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

#ifndef EXAMENGINE_H
#define EXAMENGINE_H

#include <QObject>
#include "cosmessage.h"

class Server;
class Client;

class ExamEngine : public QObject
{
	Q_OBJECT

public:
	struct Member;

	explicit ExamEngine(Server *server, const int &examId, const QString &code, const QString &owner);
	virtual ~ExamEngine();

	void run(Client *client, const CosMessage &message);

	bool memberAdd(const QString &username, Client *client = nullptr);
	int memberListAdd(const QStringList &userList);
	bool hasMember(const QString &username) const;
	int clientMember(Client *client) const;
	Client *memberClient(const QString &username) const;
	bool setMemberClient(const QString &username, Client *client);
	int findMember(const QString &username, const int &start = 0);

	bool clientAdd(Client *client, QString username = "");
	bool clientRemove(Client *client);

	bool addOwnerClient(Client *client);
	QList<Client *> ownerClients() const;
	bool isOwnerClient(Client *client) const;

	Server *server() const;
	const QString &owner() const;
	const QString &code() const;
	const QList<Member> &members() const;

	const QString &examUuid() const;
	void setExamUuid(const QString &newExamUuid);

	const QString &mapUuid() const;
	void setMapUuid(const QString &newMapUuid);

	const QString &title() const;
	void setTitle(const QString &newTitle);

	const QString &description() const;
	void setDescription(const QString &newDescription);

	const QJsonObject &config() const;
	void setConfig(const QJsonObject &newConfig);
	void setConfig(const QString &newConfig);

	int examId() const;
	void setExamId(int newExamId);

	void sendToOwners(const QString &func, const QJsonObject &data) const;
	void sendToStudents(const QString &func, const QJsonObject &data) const;
	void sendToAllMembers(const QString &func, const QJsonObject &data) const;
	void sendToOtherMembers(const Member &member, const QString &func, const QJsonObject &data) const;


public slots:
	//bool getServerInfo(QJsonObject *jsonResponse, QByteArray *);

private:
	void notifyClientChanged(const Member &member) const;
	QJsonObject memberPrepared(const int &memberIdx, const QJsonObject &);
	QJsonObject stop();
	QJsonObject getMembers(Client *client);

private:
	Server *m_server = nullptr;
	QString m_code = "";
	QString m_owner = "";
	QList<Member> m_members;

	int m_examId = -1;
	QString m_examUuid = "";
	QString m_mapUuid = "";
	QString m_title = "";
	QString m_description = "";
	QJsonObject m_config = QJsonObject();

};



// ExamEngine::Member

#include "client.h"

struct ExamEngine::Member {
	enum State {
		Invalid,
		Prepared,
		Writing,
		Finished
	};

	QString username = "";
	QPointer<Client> client;
	bool isOwner = false;
	State state = Invalid;

	Member(const QString &u) :
		username(u), client(nullptr), isOwner(false)
	{}

	Member(const QString &u, Client *c) :
		username(u), client(c), isOwner(false)
	{}

	Member(const bool &t, const QString &u, Client *c) :
		username(u), client(c), isOwner(t)
	{}

	Member() :
		username(), client(nullptr), isOwner(false)
	{}

	void send(const QString &func, const QJsonObject &json) const;

	bool operator== (const Member &m) const;
	bool operator!= (const Member &m) const;
};

#endif // EXAMENGINE_H
