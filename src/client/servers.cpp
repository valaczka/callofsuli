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
#include <QtConcurrent/QtConcurrent>

Servers::Servers(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_serversModel(nullptr)
	, m_dataFileName(Client::standardPath("servers.json"))
	, m_connectedServerKey(-1)
	, m_serverTryConnectKey(-1)
{
	m_serversModel = new VariantMapModel({
											 "id",
											 "name",
											 "host",
											 "port",
											 "ssl",
											 "username",
											 "session",
											 "autoconnect"
										 }, this);

	connect(this, &Servers::resourceRegisterRequest, this, &Servers::registerResource);
}


/**
 * @brief Servers::~Servers
 */

Servers::~Servers()
{
	unregisterResources();
	if (m_serversModel)
		delete m_serversModel;

	if (m_downloader)
		delete m_downloader;
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
 * @brief Servers::onOneResourceDownloaded
 * @param item
 * @param data
 */

void Servers::onOneResourceDownloaded(const CosDownloaderItem &item, const QByteArray &, const QJsonObject &)
{
	registerResource(item.localFile);
}





/**
 * @brief Servers::serverListReload
 */

void Servers::serverListReload()
{
	qDebug() << "Server list reload";

	if (m_serversModel->variantMapData()->size()) {
		m_serversModel->variantMapData()->clear();
	}

	QJsonDocument doc = Client::readJsonDocument(m_dataFileName);

	if (!doc.isEmpty())
		m_serversModel->variantMapData()->fromJsonArray(doc.array(), "id");

	emit serverListLoaded();

	if (m_serversModel->variantMapData()->keyIndex(m_connectedServerKey) == -1)
		setConnectedServerKey(-1);

	if (m_serversModel->variantMapData()->keyIndex(m_serverTryConnectKey) == -1)
		m_serverTryConnectKey = -1;

}


/**
 * @brief Servers::serverConnect
 * @param id
 */


void Servers::serverConnect(const int &index)
{
	if (m_client->socket()->state() != QAbstractSocket::UnconnectedState) {
		m_client->sendMessageWarning(tr("Csatlakoztatva"), tr("Már csatlakozol szerverhez, előbb azt be kell zárni!"));
		return;
	}


	m_serverTryConnectKey = -1;

	QVariantMap d = m_serversModel->variantMapData()->value(index).second;

	QUrl url;
	url.setHost(d.value("host").toString());
	url.setPort(d.value("port").toInt());
	url.setScheme(d.value("ssl").toBool() ? "wss" : "ws");


	QString dir = d.value("id").toString();
	QString serverDir = Client::standardPath(dir);
	if (!QFileInfo::exists(serverDir)) {
		QDir d(Client::standardPath());
		if (!d.mkdir(dir)) {
			m_client->sendMessageError(tr("Programhiba"), tr("Nem sikerült létrehozni a könyvtárt:"), serverDir);
			return;
		}
	}

	QString certFileName = serverDir+"/cert.pem";

	if (d.value("ssl").toBool() && QFileInfo::exists(certFileName)) {
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
	m_client->clearSession();

	m_serverTryConnectKey = m_serversModel->variantMapData()->value(index).first;

	qDebug() << "CONNECT" << m_client->socket() << url;

	m_client->socket()->open(url);
}


/**
 * @brief Servers::serverInsertOrUpdate
 * @param index
 * @param map
 */

int Servers::serverInsertOrUpdate(const int &key, const QVariantMap &map)
{
	int index = m_serversModel->variantMapData()->keyIndex(key);
	if (index != -1) {
		QVariantMap n = createFullMap(map, m_serversModel->variantMapData()->at(index).second);
		m_serversModel->variantMapData()->update(index, n);
		saveServerList();
		return key;
	} else {
		int nextKey = m_serversModel->variantMapData()->append(createFullMap(map));
		saveServerList();
		return nextKey;
	}
}




/**
 * @brief Servers::serverInfoDelete
 * @param id
 */

void Servers::serverDelete(const int &index)
{
	if (index<0 || index>=m_serversModel->variantMapData()->size())
		return;

	QVariantMap m = m_serversModel->variantMapData()->at(index).second;
	int key = m_serversModel->variantMapData()->at(index).first;
	int id = m.value("id", -1).toInt();

	if (m_connectedServerKey == key)
		setConnectedServerKey(-1);

	if (m_serverTryConnectKey == key)
		m_serverTryConnectKey = -1;

	removeServerDir(id);

	m_serversModel->variantMapData()->removeAt(index);

	saveServerList();

}


/**
 * @brief Servers::serverDeleteKey
 * @param key
 */

void Servers::serverDeleteKey(const int &key)
{
	QVariantMap m = m_serversModel->variantMapData()->valueKey(key);
	int id = m.value("id", -1).toInt();

	if (m_connectedServerKey == key)
		setConnectedServerKey(-1);

	if (m_serverTryConnectKey == key)
		m_serverTryConnectKey = -1;

	removeServerDir(id);

	m_serversModel->variantMapData()->removeKey(key);

	saveServerList();
}


/**
 * @brief Servers::serverDeleteSelected
 * @param model
 */

void Servers::serverDeleteSelected(VariantMapModel *model)
{
	Q_ASSERT(model);
	QList<int> list = model->getSelected();

	foreach (int i, list) {
		if (m_serversModel->variantMapData()->keyIndex(i) == -1)
			continue;

		if (m_connectedServerKey == i)
			setConnectedServerKey(-1);

		if (m_serverTryConnectKey == i)
			m_serverTryConnectKey = -1;

		serverDeleteKey(i);
	}

	model->unselectAll();

	saveServerList();
}







/**
 * @brief Servers::serverSetAutoConnect
 * @param serverId
 */

void Servers::serverSetAutoConnect(const int &index)
{
	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;
		if (i==index) {
			bool old = d.value("autoconnect").toBool();
			m_serversModel->variantMapData()->updateValue(i, "autoconnect", !old);
		} else {
			if (d.value("autoconnect").toBool()) {
				m_serversModel->variantMapData()->updateValue(i, "autoconnect", false);
			}
		}
	}

	saveServerList();
}


/**
 * @brief Servers::serverTryLogin
 * @param serverId
 */

void Servers::serverTryLogin(const int &key)
{
	QVariantMap d = m_serversModel->variantMapData()->valueKey(key);
	if (d.isEmpty())
		return;

	QString username = d.value("username").toString();
	QString session = d.value("session").toString();

	if (!username.isEmpty() && !session.isEmpty())
		m_client->login(username, session);
}


/**
 * @brief Servers::serverLogOut
 */

void Servers::serverLogOut()
{
	if (m_connectedServerKey != -1) {
		m_serversModel->variantMapData()->updateValueByKey(m_connectedServerKey, "session", "");
		m_serversModel->variantMapData()->updateValueByKey(m_connectedServerKey, "username", "");
		saveServerList();
	}

}


/**
 * @brief Servers::doAutoConnect
 */

void Servers::doAutoConnect()
{
	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;
		if (d.value("autoconnect").toBool()) {
			serverConnect(i);
			return;
		}
	}
}



/**
 * @brief Servers::playTestMap
 * @param data
 */

void Servers::playTestMap(QVariantMap data)
{
	QFile f(data.value("filename").toString());
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning() << "*** Can't open" << data.value("filename").toString();
		return;
	}

	QByteArray b = f.readAll();
	f.close();

	GameMap *map = GameMap::fromBinaryData(b);

	if (!map) {
		qWarning() << "*** Invalid file" << data.value("filename").toString();
		return;
	}

	GameMap::MissionLevel *ml = nullptr;

	GameMap::Campaign *c = map->campaigns().at(0);
	if (c) {
		GameMap::Mission *m = c->missions().at(0);

		if (m)
			ml = m->levels().at(0);
	}

	if (!ml) {
		qWarning() << "*** Invalid file" << data.value("filename").toString();
		return;
		delete map;
	}

	GameMatch *m_gameMatch = new GameMatch(ml, this);

	m_gameMatch->setDeleteGameMap(true);
	m_gameMatch->setBgImage("");
	m_gameMatch->setLevel(2);

	emit playTestMapReady(m_gameMatch);
}








/**
 * @brief Servers::setConnectedServerKey
 * @param connectedServerKey
 */

void Servers::setConnectedServerKey(int connectedServerKey)
{
	if (m_connectedServerKey == connectedServerKey)
		return;

	m_connectedServerKey = connectedServerKey;
	emit connectedServerKeyChanged(m_connectedServerKey);
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
	if (m_connectedServerKey != -1) {
		m_serversModel->variantMapData()->updateValueByKey(m_connectedServerKey, "session", sessionToken);
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
		m_connectedServerKey = m_serverTryConnectKey;
		m_serverTryConnectKey = -1;
		send(CosMessage::ClassUserInfo, "getServerInfo");
		send(CosMessage::ClassUserInfo, "getResources");
	} else if (state == Client::Standby) {
		setConnectedServerKey(-1);
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
	if (m_connectedServerKey != -1 && !username.isEmpty()) {
		m_serversModel->variantMapData()->updateValueByKey(m_connectedServerKey, "username", username);
		saveServerList();
	}
}


/**
 * @brief Servers::_reloadResources
 * @param resources
 */

void Servers::_reloadResources(QVariantMap resources)
{
	unregisterResources();

	QMapIterator<QString, QVariant> i(resources);

	while (i.hasNext()) {
		i.next();

		QVariantMap data = i.value().toMap();

		QString filename = i.key();
		QString md5 = data.value("md5").toString();
		QString localFile = m_client->serverDataDir()+"/"+filename;


		QFile f(localFile);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray filecontent = f.readAll();
			f.close();

			QString localmd5 = QCryptographicHash::hash(filecontent, QCryptographicHash::Md5).toHex();

			if (localmd5 == md5) {
				qDebug() << localFile << "success";
				data["progress"] = 1.0;
				data["downloaded"] = true;
				data["remoteFile"] = filename;
				data["localFile"] = localFile;

				m_downloader->append(data);

				//registerResource(localFile);
				emit resourceRegisterRequest(localFile);
				continue;
			}
		}

		data["progress"] = 0.0;
		data["downloaded"] = false;
		data["remoteFile"] = filename;
		data["localFile"] = localFile;

		m_downloader->append(data);
	}

	if (m_downloader->hasDownloadable()) {
		emit resourceDownloadRequest(Client::formattedDataSize(m_downloader->fullSize()));
	} else {
		emit resourceReady();
	}
}




/**
 * @brief Servers::reloadResources
 * @param resources
 */

void Servers::reloadResources(QVariantMap resources)
{
	qDebug() << "Reload server resources" << resources;

	if (!m_downloader) {
		setDownloader(new CosDownloader(this, CosMessage::ClassUserInfo, "downloadFile", this));
		connect(m_downloader, &CosDownloader::oneDownloadFinished, this, &Servers::onOneResourceDownloaded);
		connect(m_downloader, &CosDownloader::downloadFinished, this, &Servers::resourceReady);
		connect(m_downloader, &CosDownloader::downloadFailed, m_client, &Client::closeConnection);
	}

	run(&Servers::_reloadResources, resources);

	/*setIsBusy(true);

	QFutureWatcher<void> www;
	connect(&www, &QFutureWatcher<void>::finished, this, [=](){setIsBusy(false);});
	QFuture<void> future = QtConcurrent::run(this, &Servers::_reloadResources, resources);
	www.setFuture(future);*/

}




/**
 * @brief Servers::registerResource
 * @param filename
 */

void Servers::registerResource(const QString &filename)
{
	if (filename.endsWith(".cres")) {
		qInfo() << tr("Register server resource:") << filename;
		QResource::registerResource(filename);
	} else if (filename.endsWith("/images.db")) {
		QQmlEngine *engine = qmlEngine(this);
		SqlImage *sqlImage = new SqlImage(m_client, "sqlimageprovider", filename);
		engine->addImageProvider("sql", sqlImage);
		m_sqlImageProviders.append("sql");
	}
}



/**
 * @brief Servers::unregisterResources
 */

void Servers::unregisterResources()
{
	if (!m_downloader)
		return;

	foreach (CosDownloaderItem m, m_downloader->list()) {
		QString f = m.localFile;

		if (f.endsWith(".cres")) {
			qInfo() << tr("Unregister server resource:") << f;
			QResource::unregisterResource(f);
		}
	}

	QQmlEngine *engine = qmlEngine(this);

	qDebug() << "Remove image providers from engine" << engine << m_sqlImageProviders;

	if (engine)
		foreach (QString s, m_sqlImageProviders)
			engine->removeImageProvider(s);

	m_sqlImageProviders.clear();

	m_downloader->clear();
}



/**
 * @brief Servers::saveServerList
 */

void Servers::saveServerList()
{
	QJsonArray list;

	foreach (_MapPair p, *(m_serversModel->variantMapData()))
		list.append(QJsonObject::fromVariantMap(p.second));

	QJsonDocument doc(list);

	Client::saveJsonDocument(doc, m_dataFileName);
}



/**
 * @brief Servers::createFullMap
 * @param from
 * @return
 */

QVariantMap Servers::createFullMap(const QVariantMap &newData, const QVariantMap &from)
{
	QVariantMap m = from;

	foreach (QString k, newData.keys())
		m[k] = newData.value(k);


	if (!m.contains("id")) m["id"] = m_serversModel->variantMapData()->getNextId("id");
	if (!m.contains("name")) m["name"] = "";
	if (!m.contains("host")) m["host"] = "";
	if (!m.contains("port")) m["port"] = 10101;
	if (!m.contains("ssl")) m["ssl"] = false;
	if (!m.contains("username")) m["username"] = "";
	if (!m.contains("session")) m["session"] = "";
	if (!m.contains("autoconnect")) m["autoconnect"] = false;

	return m;
}

