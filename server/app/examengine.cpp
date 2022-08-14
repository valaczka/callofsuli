/*
 * ---- Call of Suli ----
 *
 * examengine.cpp
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

#include "examengine.h"
#include "server.h"
#include "client.h"

/**
 * @brief ExamEngine::ExamEngine
 * @param client
 * @param engineId
 */

ExamEngine::ExamEngine(Server *server, const int &examId, const QString &code, const QString &owner)
	: QObject(server)
	, m_server(server)
	, m_code(code)
	, m_owner(owner)
	, m_members()
	, m_examId(examId)
{
	Q_ASSERT(server);

	qDebug() << "EXAM ENGINE CREATED" << this << m_code << m_owner;
}


/**
 * @brief ExamEngine::~ExamEngine
 */

ExamEngine::~ExamEngine()
{
	qDebug() << "EXAM ENGINE DESTROYED" << this << m_code << m_owner << m_mapUuid;
}


/**
 * @brief ExamEngine::run
 * @param message
 */

void ExamEngine::run(Client *client, const CosMessage &message)
{
	Q_ASSERT(client);

	QJsonObject retObject;

	const QString &func = message.cosFunc();
	const QJsonObject &jsonData = message.jsonData();
	const int &memberIdx = clientMember(client);

	QJsonObject ret = QJsonObject();

	if (isOwnerClient(client)) {
		if (func == "stop")
			ret = stop();
		else if (func == "getMembers")
			ret = getMembers(m_members.at(memberIdx).client);
	} else {
		if (func == "prepared")
			ret = memberPrepared(memberIdx, jsonData);
	}


	if (!ret.isEmpty()) {
		CosMessage r(ret, CosMessage::ClassExamEngine, func, message);
		r.send(client->socket());
	} else {
		CosMessage r(CosMessage::InvalidFunction, message);
		r.setCosClass(CosMessage::ClassExamEngine);
		r.setCosFunc(func);
		r.send(client->socket());
	}
}


/** * @brief ExamEngine::memberAdd
 * @param username
 * @param client
 * @return
 */

bool ExamEngine::memberAdd(const QString &username, Client *client)
{
	if ((username != m_owner && hasMember(username)) || username.isEmpty())
		return false;

	m_members.append(Member(username, client));

	return true;
}


/**
 * @brief ExamEngine::memberListAdd
 * @param userList
 * @return
 */

int ExamEngine::memberListAdd(const QStringList &userList)
{
	int ret = 0;

	foreach (QString s, userList) {
		if (!hasMember(s)) {
			memberAdd(s, nullptr);
			++ret;
		}
	}

	return ret;
}



/**
 * @brief ExamEngine::memberClient
 * @param username
 * @return
 */

Client *ExamEngine::memberClient(const QString &username) const
{
	if (username == m_owner) {
		qWarning().noquote() << username << "is owner. Use instead: ownerClients()";
		return nullptr;
	}

	foreach (Member m, m_members) {
		if (m.username == username)
			return m.client;
	}

	return nullptr;
}


/**
 * @brief ExamEngine::setMemberClient
 * @param username
 * @param client
 * @return
 */

bool ExamEngine::setMemberClient(const QString &username, Client *client)
{
	if (username == m_owner) {
		qWarning().noquote() << username << "is owner. Use instead: addOwnerClient()";
		return false;
	}

	QList<Member>::iterator it;

	for (it = m_members.begin(); it != m_members.end(); ++it) {
		if (it->username == username) {
			it->client = client;

			sendToOtherMembers(*it, "clientChanged", {
								   { "username", username },
								   { "connected", client ? true : false }
							   });
			return true;
		}
	}

	return false;
}


/**
 * @brief ExamEngine::findMember
 * @param username
 * @param start
 * @return
 */

int ExamEngine::findMember(const QString &username, const int &start)
{
	for (int i=start; i<m_members.size(); ++i) {
		if (m_members.at(i).username == username)
			return i;
	}

	return -1;
}




/**
 * @brief ExamEngine::clientAdd
 * @param client
 * @param username
 * @return
 */

bool ExamEngine::clientAdd(Client *client, QString username)
{
	if (!client)
		return false;

	if (username.isEmpty())
		username = client->clientUserName();

	if (username.isEmpty())
		return false;

	if (username == m_owner) {
		qWarning().noquote() << username << "is owner. Use instead: addOwnerClient()";
		return false;
	}

	return setMemberClient(username, client);

}


/**
 * @brief ExamEngine::clientRemove
 * @param client
 * @param username
 * @return
 */

bool ExamEngine::clientRemove(Client *client)
{
	if (!client)
		return false;

	bool ret = false;

	QList<Member>::iterator it;

	for (it = m_members.begin(); it != m_members.end(); ++it) {
		if (it->client == client) {
			it->client = nullptr;
			ret = true;

			if (!it->isOwner)
				sendToOtherMembers(*it, "clientChanged", {
									   { "username", it->username },
									   { "connected", false }
								   });
		}
	}

	return ret;
}


/**
 * @brief ExamEngine::addOwnerClient
 * @param client
 * @return
 */

bool ExamEngine::addOwnerClient(Client *client)
{
	if (!client || client->clientUserName() != m_owner)
		return false;


	if (clientMember(client) == -1)
		m_members.append(Member(true, m_owner, client));

	return true;
}


/**
 * @brief ExamEngine::ownerClients
 * @return
 */

QList<Client *> ExamEngine::ownerClients() const
{
	QList<Client *> list;

	foreach (Member m, m_members) {
		if (m.isOwner)
			list.append(m.client);
	}

	return list;
}


/**
 * @brief ExamEngine::isOwnerClient
 * @param client
 * @return
 */

bool ExamEngine::isOwnerClient(Client *client) const
{
	foreach (Member m, m_members) {
		if (m.client == client && m.isOwner)
			return true;
	}

	return false;
}



/**
 * @brief ExamEngine::hasMember
 * @param username
 * @return
 */

bool ExamEngine::hasMember(const QString &username) const
{
	foreach (Member m, m_members) {
		if (m.username == username)
			return true;
	}

	return false;
}




/**
 * @brief ExamEngine::clientMember
 * @param client
 * @return
 */

int ExamEngine::clientMember(Client *client) const
{
	for (int i=0; i<m_members.size(); ++i) {
		if (m_members.at(i).client == client)
			return i;
	}

	return -1;
}




/**
 * @brief ExamEngine::owner
 * @return
 */

const QString &ExamEngine::owner() const
{
	return m_owner;
}


const QString &ExamEngine::code() const
{
	return m_code;
}

const QList<ExamEngine::Member> &ExamEngine::members() const
{
	return m_members;
}




/**
 * @brief ExamEngine::examEnginePrepared
 * @param data
 */

QJsonObject ExamEngine::memberPrepared(const int &memberIdx, const QJsonObject &)
{
	if (memberIdx == -1)
		return QJsonObject();

	if (m_members.at(memberIdx).state == Member::Invalid) {
		m_members[memberIdx].state = Member::Prepared;

		sendToOwners("studentPrepared", {
						 { "username", m_members.at(memberIdx).username }
					 });
	}

	return QJsonObject({{"status", true}});
}


/**
 * @brief ExamEngine::stop
 * @param memberIdx
 */

QJsonObject ExamEngine::stop()
{
	sendToAllMembers("engineStopped", {
						 { "code", m_code }
					 });


	QList<Member>::iterator it;

	for (it = m_members.begin(); it != m_members.end(); ++it) {
		if (it->client) {
			it->client->setExamEngine(nullptr);
		}
	}

	m_server->db()->execSimpleQuery("UPDATE session SET examEngineId=null WHERE examEngineId=?", {m_code});

	m_server->examEngineDelete(this);

	return QJsonObject({{"status", true}});
}



/**
 * @brief ExamEngine::getMembers
 * @return
 */

QJsonObject ExamEngine::getMembers(Client *client)
{
	QJsonArray uList = client->db()->execSelectQueryJson("SELECT studentGroupInfo.username, firstname, lastname, nickname, picture FROM studentGroupInfo "
														 "LEFT JOIN user ON (studentGroupInfo.username=user.username) "
														 "WHERE id=(SELECT groupid FROM exam WHERE exam.id=?) AND user.active=true",
														 {m_examId});

	QJsonArray ret;

	foreach (QJsonValue v, uList) {
		QJsonObject o = v.toObject();
		QString status = "invalid";

		int idx = findMember(o.value("username").toString());

		if (idx != -1) {
			const Member &m = m_members.at(idx);

			switch (m.state) {
				case Member::Prepared: status = "prepared"; break;
				case Member::Writing: status = "writing"; break;
				case Member::Finished: status = "finished"; break;
				case Member::Invalid: default: status = "invalid"; break;
			}
		}

		o["status"] = status;

		ret.append(o);
	}

	return QJsonObject({
						   { "status", true },
						   { "list", ret }
					   });
}



int ExamEngine::examId() const
{
	return m_examId;
}

void ExamEngine::setExamId(int newExamId)
{
	m_examId = newExamId;
}



const QJsonObject &ExamEngine::config() const
{
	return m_config;
}

void ExamEngine::setConfig(const QJsonObject &newConfig)
{
	m_config = newConfig;
}


/**
 * @brief ExamEngine::setConfig
 * @param newConfig
 */

void ExamEngine::setConfig(const QString &newConfig)
{
	QJsonDocument doc = QJsonDocument::fromJson(newConfig.toUtf8());
	setConfig(doc.object());
}



/**
 * @brief ExamEngine::sendToOwnerClients
 * @param func
 * @param data
 */

void ExamEngine::sendToOwners(const QString &func, const QJsonObject &data) const
{
	foreach (Member m, m_members) {
		if (m.isOwner)
			m.send(func, data);
	}
}


/**
 * @brief ExamEngine::sendToStudentClients
 * @param func
 * @param data
 */

void ExamEngine::sendToStudents(const QString &func, const QJsonObject &data) const
{
	foreach (Member m, m_members) {
		if (!m.isOwner)
			m.send(func, data);
	}
}


/**
 * @brief ExamEngine::sendToAllClients
 * @param func
 * @param data
 */

void ExamEngine::sendToAllMembers(const QString &func, const QJsonObject &data) const
{
	foreach (Member m, m_members)
		m.send(func, data);
}


/**
 * @brief ExamEngine::sendToOtherMembers
 * @param member
 * @param func
 * @param data
 */

void ExamEngine::sendToOtherMembers(const Member &member, const QString &func, const QJsonObject &data) const
{
	foreach (Member m, m_members) {
		if (m != member)
			m.send(func, data);
	}
}





const QString &ExamEngine::description() const
{
	return m_description;
}

void ExamEngine::setDescription(const QString &newDescription)
{
	m_description = newDescription;
}

const QString &ExamEngine::title() const
{
	return m_title;
}

void ExamEngine::setTitle(const QString &newTitle)
{
	m_title = newTitle;
}

const QString &ExamEngine::mapUuid() const
{
	return m_mapUuid;
}

void ExamEngine::setMapUuid(const QString &newMapUuid)
{
	m_mapUuid = newMapUuid;
}

const QString &ExamEngine::examUuid() const
{
	return m_examUuid;
}

void ExamEngine::setExamUuid(const QString &newExamUuid)
{
	m_examUuid = newExamUuid;
}



Server *ExamEngine::server() const
{
	return m_server;
}



/**
 * @brief ExamEngine::Member::send
 * @param json
 */

void ExamEngine::Member::send(const QString &func, const QJsonObject &json) const
{
	if (!client)
		return;

	CosMessage m(json, CosMessage::ClassExamEngine, func);
	m.send(client->socket());
}



/**
 * @brief ExamEngine::Member::operator ==
 * @param m
 * @return
 */

bool ExamEngine::Member::operator==(const Member &m) const
{
	return (client == m.client && username == m.username && isOwner == m.isOwner);
}


/**
 * @brief ExamEngine::Member::operator !=
 * @param m
 * @return
 */

bool ExamEngine::Member::operator!=(const Member &m) const
{
	return !(client == m.client && username == m.username && isOwner == m.isOwner);
}
