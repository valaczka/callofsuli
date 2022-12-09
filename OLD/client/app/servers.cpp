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
#include <QNetworkDatagram>
#include "androidshareutils.h"
#include <iostream>

#include "gamemap.h"

Servers::Servers(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_dataFileName(Client::standardPath("servers.json"))
	, m_connectedServer(nullptr)
	, m_serverTryToConnect(nullptr)
	, m_udpSocket(new QUdpSocket(this))
	, m_urlsToProcess()
	, m_googleOAuth2(nullptr)
	, m_serversModel(new ObjectGenericListModel<ServerObject>(this))
	, m_urlString()
{
	connect(this, &Servers::resourceRegisterRequest, this, &Servers::registerResource);

	connect(m_udpSocket, &QUdpSocket::readyRead, this, &Servers::onUdpDatagramReceived);

	connect(Client::clientInstance(), &Client::connectionStateChanged, this, &Servers::onConnectionStateChanged);
	connect(Client::clientInstance(), &Client::sessionTokenChanged, this, &Servers::onSessionTokenChanged);
	connect(Client::clientInstance(), &Client::userNameChanged, this, &Servers::onUserNameChanged);
	connect(Client::clientInstance(), &Client::authInvalid, this, &Servers::onAuthInvalid);
	connect(Client::clientInstance(), &Client::userRolesChanged, this, &Servers::onUserRolesChanged);
	connect(Client::clientInstance(), &Client::urlsToProcessReady, this, &Servers::parseUrls);

	connect(AndroidShareUtils::instance(), &AndroidShareUtils::urlSelected, this, &Servers::parseUrl);

	connect(Client::clientInstance()->socket(), &QWebSocket::sslErrors, this, &Servers::onSocketSslErrors);
	Client::clientInstance()->connectSslErrorSignalHandler(this);
}


/**
 * @brief Servers::~Servers
 */

Servers::~Servers()
{
	unregisterResources();

	delete m_serversModel;

	if (m_downloader)
		delete m_downloader;

	delete m_udpSocket;

		Client::clientInstance()->connectSslErrorSignalHandler(nullptr);

	if (m_googleOAuth2)
		delete m_googleOAuth2;
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
		} else if (func == "getServerInfo") {
			setGoogleOAuth2(d.value("googleOAuth2id").toString(),
							d.value("googleOAuth2key").toString(),
							-1);

			quint32 appVersion = message.appVersion();
			if (m_connectedServer && appVersion > CosMessage::versionNumber() && !m_connectedServer->hasErrorAndNotified()) {
				Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/update.svg", tr("Frissítés szükséges"), tr("A szerverhez képest az alkalmazás elavult, elképzelhető, hogy nem minden funkció fog helyesen működni.\nFrissítsd az alkalmazást a legfrissebb verzióra!"));
				m_connectedServer->setHasErrorAndNotified(true);
			}

		} else if (func == "registrationRequest") {
			emit registrationRequest(d);
		}
	} else if (message.cosClass() == CosMessage::ClassTeacher) {
		if (func == "groupCreate") {
			send(CosMessage::ClassUserInfo, "getMyGroups");
		} else if (func == "groupRemove") {
			send(CosMessage::ClassUserInfo, "getMyGroups");
		}
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
	QJsonDocument doc = Client::readJsonDocument(m_dataFileName);

	if (!doc.isEmpty())
		m_serversModel->updateJsonArray(doc.array(), "id");

	emit serverListLoaded();
}


/**
 * @brief Servers::serverConnect
 * @param id
 */


void Servers::serverConnect(ServerObject *server)
{
	if (Client::clientInstance()->connectionState() != Client::Standby) {
		//Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/lan-pending.svg", tr("Csatlakoztatva"), tr("Már csatlakozol szerverhez, előbb azt be kell zárni!"));
		return;
	}

	if (!server) {
		Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Belső hiba"), tr("Érvénytelen szerver"));
		return;
	}

	m_serverTryToConnect = nullptr;

	if (server->broadcast()) {
		server->setBroadcast(false);
		saveServerList();
	}

	QUrl url;
	url.setHost(server->host());
	url.setPort(server->port());
	url.setScheme(server->ssl() ? "wss" : "ws");

	QString dir = QVariant(server->id()).toString();
	QString serverDir = Client::standardPath(dir);
	if (!QFileInfo::exists(serverDir)) {
		QDir d(Client::standardPath());
		if (!d.mkdir(dir)) {
			Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/folder-alert.svg",tr("Programhiba"), tr("Nem sikerült létrehozni a könyvtárt:"), serverDir);
			return;
		}
	}

	QString certFileName = serverDir+"/cert.pem";

	if (server->ssl() && QFileInfo::exists(certFileName)) {
		QFile f(certFileName);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray cert = f.readAll();
			f.close();
			if (!cert.isEmpty()) {
				QSslCertificate c(cert, QSsl::Pem);
				if (!c.isNull()) {
					QList<QSslError> eList;

					foreach (QVariant v, server->ignoredErrors()) {
						QSslError::SslError error = static_cast<QSslError::SslError>(v.toInt());
						QSslError certError = QSslError(error, c);
						eList.append(certError);
					}

					Client::clientInstance()->socket()->ignoreSslErrors(eList);
					//QMetaObject::invokeMethod(Client::clientInstance()->socket(), "ignoreSslErrors", Qt::QueuedConnection, Q_ARG(QList<QSslError>, eList));
				}
			}
		}
	}

	Client::clientInstance()->setServerDataDir(serverDir);
	Client::clientInstance()->clearSession();

	m_serverTryToConnect = server;

	qDebug() << "CONNECT" << Client::clientInstance()->socket() << url;

	//Client::clientInstance()->socket()->open(url);
	QMetaObject::invokeMethod(Client::clientInstance()->socket(), "open", Qt::QueuedConnection, Q_ARG(QUrl, url));
}


/**
 * @brief Servers::serverInsertOrUpdate
 * @param index
 * @param map
 */

ServerObject *Servers::serverCreate(const QJsonObject &json)
{
	ServerObject *newServer = ObjectListModelObject::fromJsonObject<ServerObject>(json);
	newServer->setId(m_serversModel->nextIntValue("id"));
	m_serversModel->addObject(newServer);
	saveServerList();
	emit serverCreated(newServer);
	return newServer;
}




/**
 * @brief Servers::serverInfoDelete
 * @param id
 */

void Servers::serverDelete(ServerObject *server)
{
	if (!server) {
		qWarning() << "Missing server";
		return;
	}

	int idx = m_serversModel->index(server);

	if (idx != -1) {
		removeServerDir(server->id());
		m_serversModel->removeObject(idx);
		saveServerList();
	}

	server->deleteLater();

}


/**
 * @brief Servers::serverDelete
 * @param list
 */

void Servers::serverDeleteList(const QList<ServerObject *> list)
{
	for (ServerObject *server : list)
		serverDelete(server);
}


/**
 * @brief Servers::serverDeleteList
 * @param list
 */

void Servers::serverDeleteList(const QObjectList &list)
{
	for (QObject *object : list) {
		ServerObject *server = qobject_cast<ServerObject*>(object);
		if (server)
			serverDelete(server);
	}
}




/**
 * @brief Servers::serverSetAutoConnect
 * @param serverId
 */

void Servers::serverSetAutoConnect(ServerObject *server)
{
	if (server && server->autoconnect())
		server->setAutoconnect(false);
	else {
		foreach (ServerObject *s, m_serversModel->objects())
			s->setAutoconnect(false);

		if (server)
			server->setAutoconnect(true);
	}

	saveServerList();
}


/**
 * @brief Servers::serverTryLogin
 * @param serverId
 */

bool Servers::serverTryLogin()
{
	if (!m_urlsToProcess.isEmpty()) {
		QStringList u = m_urlsToProcess;
		m_urlsToProcess.clear();
		if (parseUrls(u))
			return false;
	}

	if (!m_connectedServer) {
		qWarning() << "Missing server";
		return false;
	}

	QString username = m_connectedServer->username();
	QString session = m_connectedServer->session();

	if (!username.isEmpty() && !session.isEmpty()) {
		Client::clientInstance()->login(username, session);
		return true;
	}

	return false;
}


/**
 * @brief Servers::serverLogOut
 */

void Servers::serverLogOut()
{
	if (m_connectedServer) {
		m_connectedServer->setSession("");
		m_connectedServer->setUsername("");
		saveServerList();
	}
}


/**
 * @brief Servers::doAutoConnect
 */

void Servers::doAutoConnect(const QStringList &arguments)
{
	if (parseUrls(arguments))
		return;

	if (AndroidShareUtils::instance()->checkPendingIntents())
		return;

	if (m_serversModel->count() == 1) {
		serverConnect(m_serversModel->object(0));
		return;
	}

	QList<ServerObject*> list = m_serversModel->find("autoconnect", true);

	if (list.size())
		serverConnect(list.at(0));
}




/**
 * @brief Servers::parseUrls
 * @param urls
 */

bool Servers::parseUrls(const QStringList &urls)
{
	foreach (QString u, urls) {
		QUrl url(u);

		qInfo() << "URL" << url;

		if (url.scheme() != "callofsuli") {
			continue;
		}


		QString host = url.host();
		int port = url.port(10101);
		QString username = url.userName();

		QString path = url.path();
		bool ssl = false;
		int _section = 1;

		if (path.section('/', _section, _section) == "ssl") {
			_section = 2;
			ssl = true;
		}

		QString func = path.section('/', _section, _section);

		ServerObject *server = username.isEmpty() ? findServer(host, port, ssl) : findServer(username, host, port, ssl);

		QUrlQuery q(url);
		QString serverUuid = q.queryItemValue("server", QUrl::FullyDecoded);

		if (func == "connect" && Client::clientInstance()->connectionState() != Client::Standby) {
			Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/account-arrow-right.svg", tr("Hiba"), tr("Nem lehet új szerverhez csatlakozni, előbb jelentkezz ki!"));
			return true;
		}

		if (func == "connect" || Client::clientInstance()->connectionState() == Client::Standby) {
			QString name = q.queryItemValue("name", QUrl::FullyDecoded);

			qDebug() << "Connect to " << name;

			if (func != "connect")
				m_urlsToProcess = QStringList({u});

			if (server) {
				server->setHost(host);
				server->setPort(port);
				server->setSsl(ssl);
				saveServerList();
			} else {
				QJsonObject map;
				map["host"] = host;
				map["port"] = port;
				map["ssl"] = ssl;
				server = serverCreate(map);
			}
			serverConnect(server);

			return true;
		}

		if (Client::clientInstance()->connectionState() != Client::Connected && Client::clientInstance()->connectionState() != Client::Reconnected) {
			Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/lan-disconnect.svg", tr("Hiba"), tr("A szerver nem elérhető, ismételd meg a kérést!"));
			return true;
		}

		if (serverUuid != Client::clientInstance()->serverUuid()) {
			Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", tr("Hiba"), tr("A kérés nem az aktuális kapcsolatra vonatkozik!"));
			return true;
		}

		if (func == "register") {
			QString code = q.queryItemValue("code", QUrl::FullyDecoded);
			QString oauth2 = q.queryItemValue("oauth2", QUrl::FullyDecoded);

			qDebug() << "REGISTER" << oauth2 << code;

			if (!Client::clientInstance()->userName().isEmpty()) {
				Client::clientInstance()->sendMessageWarningImage("qrc:/internal/icon/account-arrow-right.svg", tr("Hiba"), tr("A regisztrációhoz először ki kell jelentkezni!"));
				return true;
			}

			Client::clientInstance()->sendRegistrationRequest((oauth2 == "1"), code);
		}

	}

	return false;
}


/**
 * @brief Servers::parseUrl
 * @param url
 * @return
 */

bool Servers::parseUrl(const QString &url)
{
	return parseUrls({url});
}






/**
 * @brief Servers::sendBroadcast
 */

void Servers::sendBroadcast()
{
	if (m_udpSocket->state() != QAbstractSocket::BoundState) {
		qDebug() << "Start udp socket";

		if (!m_udpSocket->bind(SERVER_UDP_PORT)) {
			qInfo() << tr("UDP port nem elérhető: %1").arg(SERVER_UDP_PORT);

			if (!m_udpSocket->bind()) {
				Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("UDP socket error"));
				return;
			}
		}
	}

	qInfo() << tr("UPD socket listening on port %1").arg(m_udpSocket->localPort());

	CosMessage m(QJsonObject(), CosMessage::ClassServerInfo, "broadcast");

	QByteArray s;
	QDataStream writeStream(&s, QIODevice::WriteOnly);
	writeStream << m;

	m_udpSocket->writeDatagram(s, QHostAddress::Broadcast, SERVER_UDP_PORT);
}



/**
 * @brief Servers::checkQR
 * @param url
 */

bool Servers::isValidUrl(const QString &url)
{
	QUrl u(url);

	return (u.scheme() == "callofsuli");
}









/**
 * @brief Servers::acceptCertificate
 * @param cert
 * @param errorList
 */

void Servers::acceptCertificate(ServerObject *server, const QSslCertificate &cert, const QList<int> &errorList)
{
	if (cert.isNull()) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen tanúsítvány"));
		return;
	}

	if (!server) {
		Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("Belső hiba"), tr("Érvénytelen szerver"));
		return;
	}

	QString dir = QVariant(server->id()).toString();
	QString serverDir = Client::standardPath(dir);
	QString certFileName = serverDir+"/cert.pem";

	QFile f(certFileName);
	f.open(QIODevice::WriteOnly);
	f.write(cert.toPem());
	f.close();

	QVariantList list = server->ignoredErrors();
	list.reserve(errorList.size());

	foreach (int i, errorList)
	list.append(i);

	QVariantList _toList;
	foreach (QVariant v, list)
		if (!_toList.contains(v))
			_toList.append(v);

	server->setIgnoredErrors(_toList);
	saveServerList();
	serverConnect(server);
}





/**
 * @brief Servers::onSocketSslErrors
 * @param errors
 */

void Servers::onSocketSslErrors(QList<QSslError> errors)
{
	qDebug() << "SSL socket errors" << errors << m_serverTryToConnect;

	if (!m_serverTryToConnect) {
		qWarning() << "Invalid server try to connect";
		return;
	}

	QString dir = QVariant(m_serverTryToConnect->id()).toString();
	QString serverDir = Client::standardPath(dir);
	QString certFileName = serverDir+"/cert.pem";

	if (m_serverTryToConnect->ssl() && QFileInfo::exists(certFileName)) {
		QFile f(certFileName);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray cert = f.readAll();
			f.close();

			if (!cert.isEmpty()) {
				QSslCertificate c(cert, QSsl::Pem);
				if (!c.isNull()) {
					foreach (QVariant v, m_serverTryToConnect->ignoredErrors()) {
						int i = v.toInt();
						QSslError::SslError error = static_cast<QSslError::SslError>(i);
						QSslError certError = QSslError(error, c);
						errors.removeAll(certError);
					}
				}
			}
		}
	}

	if (errors.isEmpty())
		return;


	QVariantList eList;
	QStringList errorStringList;
	QSslCertificate cert;

	foreach(QSslError e, errors) {
		if (!e.certificate().isNull()) {
			if (!cert.isNull() && e.certificate() != cert) {
				Client::clientInstance()->sendMessageErrorImage("qrc:/internal/icon/alert-octagon.svg",tr("SSL hiba"), tr("Többféle tanúsítvány található!"));
				return;
			}

			cert = e.certificate();
		}
		eList.append(e.error());
		errorStringList.append(e.errorString());
	}

	QVariantMap d;
	d["errorCodes"] = eList;
	d["errorStrings"] = errorStringList;
	d["info"] = cert.toText();

	emit certificateError(m_serverTryToConnect, cert, d);
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
		m_connectedServer = m_serverTryToConnect;
		m_serverTryToConnect = nullptr;
		send(CosMessage::ClassUserInfo, "getServerInfo");
		send(CosMessage::ClassUserInfo, "getResources");
	} else if (state == Client::Standby) {
		m_connectedServer = nullptr;
		Client::clientInstance()->setServerDataDir("");
		unregisterResources();
		Client::reloadGameResources();
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
	if (m_connectedServer  && !username.isEmpty()) {
		m_connectedServer->setUsername(username);
		saveServerList();
	}
}



/**
 * @brief Servers::onUdpDatagramReceived
 */

void Servers::onUdpDatagramReceived()
{
	while (m_udpSocket->hasPendingDatagrams()) {
		QNetworkDatagram d = m_udpSocket->receiveDatagram(MAX_UDP_DATAGRAM_SIZE);

		if (!d.isValid())
			continue;

		CosMessage m(d.data());

		if (!m.valid())
			continue;

		if (m.cosClass() == CosMessage::ClassServerInfo && m.cosFunc() == "broadcastInfo") {
			QJsonObject serverData = m.jsonData();

			ServerObject *server = findServer(serverData.value("host").toString("-"), serverData.value("port").toInt(0), serverData.value("ssl").toBool(false));

			if (!server) {
				ServerObject *newServer = new ServerObject(this);
				newServer->setId(m_serversModel->nextIntValue("id"));
				newServer->setHost(serverData.value("host").toString());
				newServer->setPort(serverData.value("port").toInt());
				newServer->setSsl(serverData.value("ssl").toBool());
				newServer->setBroadcast(true);
				m_serversModel->addObject(newServer);
			}
		}
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
		QString localFile = Client::clientInstance()->serverDataDir()+"/"+filename;


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
		connect(m_downloader, &CosDownloader::downloadFailed, Client::clientInstance(), &Client::closeConnection);
	}

	run(&Servers::_reloadResources, resources);
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
		SqlImage *sqlImage = new SqlImage(Client::clientInstance(), "sqlimageprovider", filename);
		Client::clientInstance()->rootEngine()->addImageProvider("sql", sqlImage);
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

	qDebug() << "Remove image providers from engine" << m_sqlImageProviders;

		foreach (QString s, m_sqlImageProviders)
			Client::clientInstance()->rootEngine()->removeImageProvider(s);

	m_sqlImageProviders.clear();

	m_downloader->clear();
}




/**
 * @brief Servers::saveServerList
 */

void Servers::saveServerList()
{
	QList<ServerObject *> servers = m_serversModel->find("broadcast", false);
	QJsonArray list = ObjectGenericListModel<ServerObject>::toJsonArray(servers);

	QJsonDocument doc(list);

	Client::saveJsonDocument(doc, m_dataFileName);
}




/**
 * @brief Servers::findServer
 * @param host
 * @param port
 * @param ssl
 * @param username
 * @return
 */

ServerObject* Servers::findServer(const QString &host, const int &port, const bool &ssl)
{
	foreach (ServerObject *server, m_serversModel->objects()) {
		if (server->host() == host &&
			server->port() == port &&
			server->ssl() == ssl)
		{
			return server;
		}
	}

	return nullptr;
}



/**
 * @brief Servers::findServer
 * @param username
 * @param host
 * @param port
 * @param ssl
 * @return
 */

ServerObject* Servers::findServer(const QString &username, const QString &host, const int &port, const bool &ssl)
{
	foreach (ServerObject *server, m_serversModel->objects()) {
		if (server->host() == host &&
			server->port() == port &&
			server->ssl() == ssl &&
			server->username() == username)
		{
			return server;
		}
	}

	return nullptr;
}



/**
 * @brief Servers::googleOAuth2
 * @return
 */

GoogleOAuth2 *Servers::googleOAuth2() const
{
	return m_googleOAuth2;
}


/**
 * @brief Servers::setGoogleOAuth2
 * @param newGoogleOAuth2
 */

void Servers::setGoogleOAuth2(GoogleOAuth2 *newGoogleOAuth2)
{
	if (m_googleOAuth2 == newGoogleOAuth2)
		return;

	if (m_googleOAuth2 && !newGoogleOAuth2) {
		delete m_googleOAuth2;
	}

	m_googleOAuth2 = newGoogleOAuth2;
	emit googleOAuth2Changed();
}


/**
 * @brief Servers::setGoogleOAuth2
 * @param id
 * @param key
 * @param port
 */

void Servers::setGoogleOAuth2(const QString &id, const QString &key, const qint16 &port)
{
	if (m_googleOAuth2) {
		if (m_googleOAuth2->id() == id && m_googleOAuth2->key() == key && (port == -1 || m_googleOAuth2->handlerPort() == port))
			return;

		setGoogleOAuth2(nullptr);
	}

	if (!id.isEmpty() && !key.isEmpty()) {
		setGoogleOAuth2(new GoogleOAuth2(this));
		m_googleOAuth2->setClient(id, key, port);
	}
}

ObjectGenericListModel<ServerObject> *Servers::serversModel() const
{
	return m_serversModel;
}


/**
 * @brief Servers::setUrlString
 * @param url
 */

void Servers::setUrlString(const QString &url)
{
	m_urlString = url;
}



/**
 * @brief Servers::takeUrlString
 * @return
 */

QString Servers::takeUrlString()
{
	QString s = m_urlString;
	m_urlString = "";
	return s;
}

