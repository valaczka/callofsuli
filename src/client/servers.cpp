/*
 * ---- Call of Suli ----
 *
 * servers.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Servers
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "servers.h"

Servers::Servers(QQuickItem *parent)
	: AbstractActivity(parent)
	, m_resources()
	, m_readyResources(false)
	, m_serverList()
	, m_serversModel(nullptr)
	, m_dataFileName(Client::standardPath("servers.json"))
	, m_connectedServer(nullptr)
	, m_serverTryConnect(nullptr)
{
	m_serversModel = new QObjectModel(&m_serverList, QStringList({"name", "host", "port", "ssl", "autoconnect", "username", "session"}), this);
}


/**
 * @brief Servers::~Servers
 */

Servers::~Servers()
{
	unregisterResources();
	if (m_serversModel)
		delete m_serversModel;
}


/**
 * @brief Servers::onMessageReceived
 * @param message
 */

void Servers::onMessageReceived(const CosMessage &message)
{
	QString func = message.cosFunc();
	QJsonObject d = message.jsonData();

	if (message.cosClass() == CosMessage::ClassUserInfo) {
		if (func == "getResources") {
			reloadResources(d.toVariantMap());
		} else if (func == "downloadFile") {
			getDownloadedResource(message);
		}
		/*if (func == "registrationRequest") {
			bool error = d.value("error").toBool(false);

			if (error) {
				emit registrationRequestFailed();
				QString errorString = d.value("errorString").toString();

				if (errorString == "email empty")
					sendMessageWarning(tr("Regisztráció"), tr("Nincs megadva email cím!"));
				else if (errorString == "email exists")
					sendMessageWarning(tr("Regisztráció"), tr("A megadott email cím már regisztrálva van!"));
				else
					sendMessageWarning(tr("Regisztráció"), tr("Internal error"), errorString);
			} else {
				emit registrationRequestSuccess();
			}
		} else if (func == "getSettings") {
			if (d.contains("serverName"))
				setServerName(d.value("serverName").toString());

			emit settingsLoaded(d);
		} else if (func == "setSettings") {
			bool error = d.value("error").toBool(true);

			if (error)
				emit settingsError();
			else
				emit settingsSuccess();
		} else */
	}
}


/**
 * @brief Servers::onMessageFrameReceived
 * @param message
 */

void Servers::onMessageFrameReceived(const CosMessage &message)
{
	QString filename = message.jsonData().value("filename").toString();

	if (filename.isEmpty())
		return;

	QString newFile = m_client->serverDataDir()+"/"+filename;

	if (m_resources.contains(newFile)) {
		qreal ratio = message.receivedDataRatio();
		if (ratio >= 1.0)
			ratio = 0.999999999;

		m_resources[newFile] = ratio;

		emit resourcesChanged(m_resources);
	}
}





/**
 * @brief Servers::serverListReload
 */

void Servers::serverListReload()
{
	qDebug() << "Server list reload";

	m_connectedServer = nullptr;
	m_serverTryConnect = nullptr;

	if (m_serverList.size()) {
		qDeleteAll(m_serverList.begin(), m_serverList.end());
		m_serverList.clear();
	}

	QJsonDocument doc = Client::readJsonDocument(m_dataFileName);

	int lastid = 0;

	if (!doc.isEmpty()) {
		QJsonArray list = doc.array();

		foreach (QJsonValue v, list) {
			ServerData *s = new ServerData(v.toObject(), this);
			int id = s->id();
			if (id<=0)
				s->setId(++lastid);
			else
				lastid = id;

			m_serverList.append(s);
		}
	}

	emit serverListLoaded();

}


/**
 * @brief Servers::serverConnect
 * @param id
 */


void Servers::serverConnect(const int &index)
{
	m_serverTryConnect = nullptr;

	ServerData *d = qobject_cast<ServerData *>(m_serverList.at(index));

	QUrl url;
	url.setHost(d->host());
	url.setPort(d->port());
	url.setScheme(d->ssl() ? "wss" : "ws");


	QString dir = QString("%1").arg(d->id());
	QString serverDir = Client::standardPath(dir);
	if (!QFileInfo::exists(serverDir)) {
		QDir d(Client::standardPath());
		if (!d.mkdir(dir)) {
			m_client->sendMessageError(tr("Programhiba"), tr("Nem sikerült létrehozni a könyvtárt:"), serverDir);
			return;
		}
	}

	QString certFileName = serverDir+"/cert.pem";

	if (d->ssl() && QFileInfo::exists(certFileName)) {
		QFile f(certFileName);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray cert = f.readAll();
			if (!cert.isEmpty()) {
				QSslCertificate c(cert, QSsl::Pem);
				if (!c.isNull()) {
					QList<QSslError> eList;
					eList.append(QSslError(QSslError::SelfSignedCertificate, c));
					eList.append(QSslError(QSslError::HostNameMismatch, c));

					m_client->socket()->ignoreSslErrors(eList);
				}
			}
			f.close();
		}
	}

	m_client->setServerDataDir(serverDir);

	m_serverTryConnect = d;

	m_client->socket()->open(url);
}


/**
 * @brief Servers::serverInsertOrUpdate
 * @param index
 * @param map
 */

int Servers::serverInsertOrUpdate(const int &index, const QVariantMap &map)
{
	if (index < 0 || index >= m_serverList.count()) {
		ServerData *d = new ServerData(map, this);
		d->setId(nextId());
		m_serverList.append(d);
		saveServerList();
		return d->id();
	} else {
		ServerData *d = qobject_cast<ServerData *>(m_serverList.at(index));
		d->set(map);
		m_serverList.update(d);
		saveServerList();
		return d->id();
	}
	return -1;
}




/**
 * @brief Servers::serverInfoDelete
 * @param id
 */

void Servers::serverDelete(const int &index)
{
	ServerData *d = qobject_cast<ServerData *>(m_serverList.at(index));

	if (!d)
		return;

	if (m_connectedServer == d)
		m_connectedServer = nullptr;

	if (m_serverTryConnect == d)
		m_serverTryConnect = nullptr;

	removeServerDir(d->id());

	if (m_serverList.removeOne(d))
		d->deleteLater();

	saveServerList();

}


/**
 * @brief Servers::serverDeleteSelected
 * @param model
 */

void Servers::serverDeleteSelected(QObjectModel *model)
{
	Q_ASSERT(model);
	QObjectList list = model->getSelected();

	foreach (QObject *o, list) {
		if (m_connectedServer == o)
			m_connectedServer = nullptr;

		if (m_serverTryConnect == o)
			m_serverTryConnect = nullptr;

		if (m_serverList.removeOne(o))
			o->deleteLater();
	}

	saveServerList();
}







/**
 * @brief Servers::serverSetAutoConnect
 * @param serverId
 */

void Servers::serverSetAutoConnect(const int &index)
{
	for (int i=0; i<m_serverList.count(); i++) {
		ServerData *d = qobject_cast<ServerData *>(m_serverList.at(i));
		if (i==index) {
			bool old = d->autoconnect();
			d->setAutoconnect(!old);
			m_serverList.update(d);
		} else {
			if (d->autoconnect()) {
				d->setAutoconnect(false);
				m_serverList.update(d);
			}
		}
	}

	saveServerList();
}


/**
 * @brief Servers::serverTryLogin
 * @param serverId
 */

void Servers::serverTryLogin(ServerData *d)
{
	if (!d)
		return;

	if (!d->username().isEmpty() && !d->session().isEmpty())
		m_client->login(d->username(), d->session());
}


/**
 * @brief Servers::serverLogOut
 */

void Servers::serverLogOut()
{
	if (m_connectedServer) {
		m_connectedServer->setSession("");
		m_connectedServer->setUsername("");
		m_serverList.update(m_connectedServer);
		saveServerList();
	}

}


/**
 * @brief Servers::doAutoConnect
 */

void Servers::doAutoConnect()
{
	for (int i=0; i<m_serverList.count(); i++) {
		ServerData *d = qobject_cast<ServerData *>(m_serverList.at(i));
		if (d->autoconnect()) {
			serverConnect(i);
			return;
		}
	}
}






void Servers::setReadyResources(bool readyResources)
{
	if (m_readyResources == readyResources)
		return;

	m_readyResources = readyResources;
	emit readyResourcesChanged(m_readyResources);
}



void Servers::setConnectedServer(ServerData *connectedServer)
{
	if (m_connectedServer == connectedServer)
		return;

	m_connectedServer = connectedServer;
	emit connectedServerChanged(m_connectedServer);
}




/**
 * @brief Servers::clientSetup
 */

void Servers::clientSetup()
{
	connect(m_client, &Client::connectionStateChanged, this, &Servers::onConnectionStateChanged);
	connect(m_client, &Client::sessionTokenChanged, this, &Servers::onSessionTokenChanged);
	connect(m_client, &Client::userNameChanged, this, &Servers::onUserNameChanged);
	connect(m_client, &Client::authInvalid, this, &Servers::onAuthInvalid);
	connect(m_client, &Client::userRolesChanged, this, &Servers::onUserRolesChanged);
}


/**
 * @brief Servers::removeServerDir
 */

void Servers::removeServerDir(const int &serverId)
{
	QString dir = QString("%1").arg(serverId);
	QString serverDir = Client::standardPath(dir);

	QDir d(serverDir);
	if (d.exists())
		qDebug() << tr("Remove server files") << serverDir << d.removeRecursively();
}


/**
 * @brief Servers::onSessionTokenChanged
 * @param sessionToken
 */

void Servers::onSessionTokenChanged(QString sessionToken)
{
	if (m_connectedServer) {
		m_connectedServer->setSession(sessionToken);
		m_serverList.update(m_connectedServer);
		saveServerList();
	}
}


/**
 * @brief Servers::onConnectionStateChanged
 * @param state
 */

void Servers::onConnectionStateChanged(Client::ConnectionState state)
{
	if (state == Client::Connected) {
		m_connectedServer = m_serverTryConnect;
		m_serverTryConnect = nullptr;
		serverTryLogin(m_connectedServer);
	} else if (state == Client::Standby) {
		setConnectedServer(nullptr);
		unregisterResources();
	}
}


/**
 * @brief Servers::onInvalidAuth
 */

void Servers::onAuthInvalid()
{
	serverLogOut();
}


/**
 * @brief Servers::onUserRolesChanged
 * @param userRoles
 */

void Servers::onUserRolesChanged(CosMessage::ClientRoles userRoles)
{
	if (!userRoles.testFlag(CosMessage::RoleStudent) &&
		!userRoles.testFlag(CosMessage::RoleTeacher) &&
		!userRoles.testFlag(CosMessage::RoleAdmin))
	{
		serverLogOut();
	}
}


/**
 * @brief Servers::onUserNameChanged
 * @param username
 */


void Servers::onUserNameChanged(QString username)
{
	if (m_connectedServer && !username.isEmpty()) {
		m_connectedServer->setUsername(username);
		m_serverList.update(m_connectedServer);
		saveServerList();
	}
}




/**
 * @brief Servers::reloadResources
 * @param resources
 */

void Servers::reloadResources(QVariantMap resources)
{
	qDebug() << "Reload server resources" << resources;

	unregisterResources();

	QMapIterator<QString, QVariant> i(resources);

	while (i.hasNext()) {
		i.next();

		QString filename = i.key();
		QString md5 = i.value().toString();
		QString localFile = m_client->serverDataDir()+"/"+filename;


		QFile f(localFile);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray filecontent = f.readAll();
			f.close();

			QString localmd5 = QCryptographicHash::hash(filecontent, QCryptographicHash::Md5).toHex();

			if (localmd5 == md5) {
				qDebug() << localFile << "success";
				m_resources[localFile] = 1.0;
				emit resourcesChanged(m_resources);
				registerResource(localFile);
				continue;
			}
		}

		qDebug() << "Download" << localFile;

		m_resources[localFile] = 0.0;
		emit resourcesChanged(m_resources);

		QJsonObject o;
		o["filename"] = filename;
		send(CosMessage::ClassUserInfo, "downloadFile", o);
	}

	checkResources();
}


/**
 * @brief Client::getDownloadedResource
 * @param message
 */


void Servers::getDownloadedResource(const CosMessage &message)
{
	QString filename = message.jsonData().value("filename").toString();
	QString md5 = message.jsonData().value("md5").toString();

	if (message.messageType() != CosMessage::MessageBinaryData) {
		qWarning() << tr("Érvénytelen fájl érkezett") << message;
	}

	if (filename.isEmpty()) {
		qWarning() << tr("Érvénytelen fájl érkezett") << message;
		return;
	}

	QByteArray data = message.binaryData();

	QString localmd5 = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

	if (localmd5 != md5) {
		qWarning() << tr("MD5 hash hiba") << message;
		return;
	}

	QString newFile = m_client->serverDataDir()+"/"+filename;

	QFile f(newFile);

	if (!f.open(QIODevice::WriteOnly)) {
		m_client->sendMessageError(tr("Fájl létrehozási hiba"), newFile, f.errorString());
		return;
	}

	if (f.write(data) == -1) {
		m_client->sendMessageError(tr("Fájl írási hiba"), newFile, f.errorString());
		f.close();
		return;
	}

	f.close();

	qDebug() << tr("Fájl létrehozva") << newFile;

	m_resources[newFile] = 1.0;
	registerResource(newFile);
	checkResources();
}


/**
 * @brief Servers::registerResource
 * @param filename
 */

void Servers::registerResource(const QString &filename)
{
	if (!filename.endsWith(".cres"))
		return;

	qInfo() << tr("Register server resource:") << filename;
	QResource::registerResource(filename);
}



/**
 * @brief Servers::unregisterResources
 */

void Servers::unregisterResources()
{
	foreach (QString f, m_resources.keys()) {
		qInfo() << tr("Unregister server resource:") << f;
		QResource::unregisterResource(f);
	}
	m_resources.clear();
	emit resourcesChanged(m_resources);
	checkResources();
}


/**
 * @brief Servers::checkResources
 */

void Servers::checkResources()
{
	bool r = !m_resources.isEmpty();

	foreach (QVariant v, m_resources) {
		if (v.toReal() < 1.0)
			r = false;
	}

	setReadyResources(r);
}


/**
 * @brief Servers::saveServerList
 */

void Servers::saveServerList()
{
	QJsonArray list;

	foreach (QObject *o, m_serverList) {
		ServerData *d = qobject_cast<ServerData *>(o);
		if (!d)
			continue;

		list.append(d->asJsonObject());
	}

	QJsonDocument doc(list);

	Client::saveJsonDocument(doc, m_dataFileName);
}


/**
 * @brief Servers::serverGet
 * @param index
 */




/**
 * @brief Servers::nextId
 * @return
 */

int Servers::nextId()
{
	int nextId = 1;

	foreach (QObject *o, m_serverList) {
		ServerData *d = qobject_cast<ServerData *>(o);
		if (!d)
			continue;

		if (d->id() >= nextId)
			nextId = d->id()+1;
	}

	return nextId;
}



/**
 * @brief ServerData::ServerData
 * @param parent
 */

ServerData::ServerData(QObject *parent)
	: QObject(parent)
{
	qDebug() << "CREATE" << this;
}


/**
 * @brief ServerData::ServerData
 * @param object
 * @param parent
 */

ServerData::ServerData(const QJsonObject &object, QObject *parent)
	: QObject(parent)
	, m_name(object.value("name").toString())
	, m_host(object.value("host").toString())
	, m_port(object.value("port").toInt())
	, m_ssl(object.value("ssl").toBool())
	, m_username(object.value("username").toString())
	, m_session(object.value("session").toString())
	, m_autoconnect(object.value("autoconnect").toBool())
	, m_id(object.value("id").toInt())
{
	qDebug() << "CREATE" << this;
}

/**
 * @brief ServerData::ServerData
 * @param map
 * @param parent
 */

ServerData::ServerData(const QVariantMap &map, QObject *parent)
	: QObject(parent)
	, m_name(map.value("name").toString())
	, m_host(map.value("host").toString())
	, m_port(map.value("port").toInt())
	, m_ssl(map.value("ssl").toBool())
	, m_username(map.value("username").toString())
	, m_session(map.value("session").toString())
	, m_autoconnect(map.value("autoconnect").toBool())
	, m_id(map.value("id").toInt())
{

	qDebug() << "CREATE" << this;
}


/**
 * @brief ServerData::~ServerData
 */

ServerData::~ServerData()
{
	qDebug() << "DELETE" << this;
}


/**
 * @brief ServerData::asJsonObject
 * @return
 */

QJsonObject ServerData::asJsonObject() const
{
	QJsonObject root;
	root["name"] = m_name;
	root["host"] = m_host;
	root["port"] = m_port;
	root["ssl"] = m_ssl;
	root["username"] = m_username;
	root["session"] = m_session;
	root["autoconnect"] = m_autoconnect;
	root["id"] = m_id;
	return root;
}


/**
 * @brief ServerData::set
 * @param map
 */

void ServerData::set(const QVariantMap &map)
{
	if (map.contains("name"))	setName(map.value("name").toString());
	if (map.contains("host"))	setHost(map.value("host").toString());
	if (map.contains("port"))	setPort(map.value("port").toInt());
	if (map.contains("ssl"))	setSsl(map.value("ssl").toBool());
	if (map.contains("username"))	setUsername(map.value("username").toString());
	if (map.contains("session"))	setSession(map.value("session").toString());
	if (map.contains("autoconnect"))	setAutoconnect(map.value("autoconnect").toBool());
	if (map.contains("id"))	setId(map.value("id").toInt());
}





void ServerData::setName(QString name)
{
	if (m_name == name)
		return;

	m_name = name;
	emit nameChanged(m_name);
}

void ServerData::setHost(QString host)
{
	if (m_host == host)
		return;

	m_host = host;
	emit hostChanged(m_host);
}

void ServerData::setPort(int port)
{
	if (m_port == port)
		return;

	m_port = port;
	emit portChanged(m_port);
}

void ServerData::setSsl(bool ssl)
{
	if (m_ssl == ssl)
		return;

	m_ssl = ssl;
	emit sslChanged(m_ssl);
}

void ServerData::setUsername(QString username)
{
	if (m_username == username)
		return;

	m_username = username;
	emit usernameChanged(m_username);
}

void ServerData::setSession(QString session)
{
	if (m_session == session)
		return;

	m_session = session;
	emit sessionChanged(m_session);
}


void ServerData::setAutoconnect(bool autoconnect)
{
	if (m_autoconnect == autoconnect)
		return;

	m_autoconnect = autoconnect;
	emit autoconnectChanged(m_autoconnect);
}

void ServerData::setId(int id)
{
	if (m_id == id)
		return;

	m_id = id;
	emit idChanged(m_id);
}
