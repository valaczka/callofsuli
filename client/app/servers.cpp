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

Servers::Servers(QQuickItem *parent)
	: AbstractActivity(CosMessage::ClassInvalid, parent)
	, m_serversModel(nullptr)
	, m_dataFileName(Client::standardPath("servers.json"))
	, m_connectedServerKey(-1)
	, m_serverTryConnectKey(-1)
	, m_udpSocket(new QUdpSocket(this))
	, m_urlsToProcess()
	, m_googleOAuth2(nullptr)
{
	m_serversModel = new VariantMapModel({
											 "id",
											 "name",
											 "host",
											 "port",
											 "ssl",
											 "username",
											 "session",
											 "autoconnect",
											 "broadcast"
										 }, this);

	connect(this, &Servers::resourceRegisterRequest, this, &Servers::registerResource);

	connect(m_udpSocket, &QUdpSocket::readyRead, this, &Servers::onUdpDatagramReceived);
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

	delete m_udpSocket;

	if (m_client)
		m_client->connectSslErrorSignalHandler(nullptr);

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
			qDebug() << "GET SERVER INFO";
			setGoogleOAuth2(d.value("googleOAuth2id").toString(),
							d.value("googleOAuth2key").toString(),
							d.value("googleOAuth2port").toInt(-1));
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


void Servers::serverConnect(const int &id)
{
	if (m_client->socket()->state() != QAbstractSocket::UnconnectedState) {
		m_client->sendMessageWarning(tr("Csatlakoztatva"), tr("Már csatlakozol szerverhez, előbb azt be kell zárni!"));
		return;
	}

	int key =  m_serversModel->variantMapData()->key("id", id);

	if (key == -1) {
		m_client->sendMessageWarning(tr("Belső hiba"), tr("Érvénytelen szerverkulcs"), "key == -1");
		return;
	}

	m_serverTryConnectKey = -1;

	QVariantMap d = m_serversModel->variantMapData()->valueKey(key);

	if (d.value("broadcast", false).toBool()) {
		d.remove("broadcast");
		m_serversModel->variantMapData()->updateKey(key, d);
		saveServerList();
	}

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
			f.close();
			if (!cert.isEmpty()) {
				QSslCertificate c(cert, QSsl::Pem);
				if (!c.isNull()) {
					QList<QSslError> eList;

					foreach (QVariant v, d.value("ignoredErrors").toList()) {
						QSslError::SslError error = static_cast<QSslError::SslError>(v.toInt());
						QSslError certError = QSslError(error, c);
						eList.append(certError);
					}

					m_client->socket()->ignoreSslErrors(eList);
				}
			}
		}
	}

	m_client->setServerDataDir(serverDir);
	m_client->clearSession();

	m_serverTryConnectKey = key;

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
		return n.value("id").toInt();
	} else {
		QVariantMap n = createFullMap(map);
		m_serversModel->variantMapData()->append(n);
		saveServerList();
		return n.value("id").toInt();
	}
}




/**
 * @brief Servers::serverInfoDelete
 * @param id
 */

void Servers::serverDelete(const QVariantMap &params)
{
	int id = params.value("id", -1).toInt();

	QList<int> list;

	if (id != -1)
		list.append(id);

	foreach (QVariant v, params.value("list").toList())
		list.append(v.toInt());

	if (list.isEmpty())
		return;



	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;
		int key = m_serversModel->variantMapData()->at(i).first;

		if (list.contains(d.value("id").toInt())) {

			if (m_connectedServerKey == key)
				setConnectedServerKey(-1);

			if (m_serverTryConnectKey == key)
				m_serverTryConnectKey = -1;

			removeServerDir(id);

			m_serversModel->variantMapData()->removeAt(i);
			i = -1;
		}
	}

	saveServerList();

}




/**
 * @brief Servers::serverSetAutoConnect
 * @param serverId
 */

void Servers::serverSetAutoConnect(const int &id)
{
	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;
		if (d.value("id", -1).toInt() == id) {
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
	if (!m_urlsToProcess.isEmpty()) {
		QStringList u = m_urlsToProcess;
		m_urlsToProcess.clear();
		if (parseUrls(u))
			return;
	}


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

void Servers::doAutoConnect(const QStringList &arguments)
{
	if (parseUrls(arguments))
		return;

	if (AndroidShareUtils::instance()->checkPendingIntents())
		return;

	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;
		if (d.value("autoconnect").toBool()) {
			serverConnect(d.value("id").toInt());
			return;
		}
	}
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

		int serverKey = username.isEmpty() ? findServer(host, port, ssl) : findServer(username, host, port, ssl);

		QUrlQuery q(url);
		QString serverUuid = q.queryItemValue("server", QUrl::FullyDecoded);

		if (func == "connect" && m_client->connectionState() != Client::Standby) {
			m_client->sendMessageWarning(tr("Hiba"), tr("Nem lehet új szerverhez csatlakozni, előbb jelentkezz ki!"));
			return true;
		}

		if (func == "connect" || m_client->connectionState() == Client::Standby) {
			QString name = q.queryItemValue("name", QUrl::FullyDecoded);
			QVariantMap map;

			qDebug() << "Connect to " << name;

			if (func != "connect")
				m_urlsToProcess = QStringList({u});

			if (serverKey == -1)
				map["name"] = (name.isEmpty() ? QString("%1:%2").arg(host).arg(port) : name);

			map["host"] = host;
			map["port"] = port;
			map["ssl"] = ssl;
			int serverId = serverInsertOrUpdate(serverKey, map);
			serverConnect(serverId);

			return true;
		}

		if (m_client->connectionState() != Client::Connected && m_client->connectionState() != Client::Reconnected) {
			m_client->sendMessageWarning(tr("Hiba"), tr("A szerver nem elérhető, ismételd meg a kérést!"));
			return true;
		}

		if (serverUuid != m_client->serverUuid()) {
			m_client->sendMessageWarning(tr("Hiba"), tr("A kérés nem az aktuális kapcsolatra vonatkozik!"));
			return true;
		}

		if (func == "register") {
			QString user = q.queryItemValue("user", QUrl::FullyDecoded);
			QString code = q.queryItemValue("code", QUrl::FullyDecoded);

			qDebug() << "REGISTER" << user << code;

			if (user.isEmpty() || code.isEmpty())
				return true;

			m_client->login(user, "", code);
		} else if (func == "reset") {
			QString user = q.queryItemValue("user", QUrl::FullyDecoded);
			QString code = q.queryItemValue("code", QUrl::FullyDecoded);

			qDebug() << "RESET" << user << code;

			if (user.isEmpty() || code.isEmpty())
				return true;

			m_client->passwordRequest(user, code);
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
				m_client->sendMessageError(tr("Belső hiba"), tr("UDP socket error"));
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
 * @brief Servers::acceptCertificate
 * @param cert
 * @param errorList
 */

void Servers::acceptCertificate(const int &serverKey, const QSslCertificate &cert, const QVariantList &errorList)
{
	if (cert.isNull()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Érvénytelen tanúsítvány"));
		return;
	}

	if (serverKey < 0) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Érvénytelen szerverkulcs"), "serverKey < 0");
		return;
	}

	QVariantMap data = m_serversModel->variantMapData()->valueKey(serverKey);

	if (data.isEmpty()) {
		m_client->sendMessageError(tr("Belső hiba"), tr("Érvénytelen szerverkulcs"), "empty data");
		return;
	}


	int id = data.value("id").toInt();
	QString dir = data.value("id").toString();
	QString serverDir = Client::standardPath(dir);
	QString certFileName = serverDir+"/cert.pem";

	QFile f(certFileName);
	f.open(QIODevice::WriteOnly);
	f.write(cert.toPem());
	f.close();

	QVariantList list = data.value("ignoredErrors").toList();

	list.append(errorList);

	QVariantList _toList;
	foreach (QVariant v, list)
		if (!_toList.contains(v))
			_toList.append(v);

	data["ignoredErrors"] = _toList;

	m_serversModel->variantMapData()->updateKey(serverKey, data);
	saveServerList();

	serverConnect(id);
}





/**
 * @brief Servers::onSocketSslErrors
 * @param errors
 */

void Servers::onSocketSslErrors(QList<QSslError> errors)
{
	qDebug() << "SSL socket errors" << errors << m_serverTryConnectKey;

	QVariantMap data = m_serversModel->variantMapData()->valueKey(m_serverTryConnectKey);

	if (data.isEmpty()) {
		qWarning() << "Invalid server key";
		return;
	}

	QString dir = data.value("id").toString();
	QString serverDir = Client::standardPath(dir);
	QString certFileName = serverDir+"/cert.pem";

	if (data.value("ssl").toBool() && QFileInfo::exists(certFileName)) {
		QFile f(certFileName);
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray cert = f.readAll();
			f.close();

			if (!cert.isEmpty()) {
				QSslCertificate c(cert, QSsl::Pem);
				if (!c.isNull()) {
					foreach (QVariant v, data.value("ignoredErrors").toList()) {
						QSslError::SslError error = static_cast<QSslError::SslError>(v.toInt());
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
				m_client->sendMessageError(tr("SSL hiba"), tr("Többféle tanúsítvány található!"));
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
	d["serverKey"] = m_serverTryConnectKey;
	d["info"] = cert.toText();

	emit certificateError(cert, d);
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
	connect(m_client, &Client::urlsToProcessReady, this, &Servers::parseUrls);

	connect(AndroidShareUtils::instance(), &AndroidShareUtils::urlSelected, this, &Servers::parseUrl);

	connect(m_client->socket(), &QWebSocket::sslErrors, this, &Servers::onSocketSslErrors);
	m_client->connectSslErrorSignalHandler(this);
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
	if (m_connectedServerKey != -1 && !username.isEmpty()) {
		m_serversModel->variantMapData()->updateValueByKey(m_connectedServerKey, "username", username);
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

			VariantMapData *md = m_serversModel->variantMapData();

			_MapList::const_iterator it;

			bool found = false;

			for (it = md->constBegin(); it != md->constEnd(); ++it) {
				_MapPair p = *it;
				QVariantMap m = p.second;

				if (m.value("ssl", true).toBool() == serverData.value("ssl").toBool(false) &&
					m.value("host", "").toString() == serverData.value("host").toString("-") &&
					m.value("port", -1).toInt() == serverData.value("port").toInt(0)) {
					found = true;
					break;
				}
			}

			if (!found) {
				QVariantMap map;
				map["name"] = serverData.value("name").toString();
				map["host"] = serverData.value("host").toString();
				map["port"] = serverData.value("port").toInt();
				map["ssl"] = serverData.value("ssl").toBool();
				map["broadcast"] = true;

				m_serversModel->variantMapData()->append(createFullMap(map));
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

	foreach (_MapPair p, *(m_serversModel->variantMapData())) {
		QJsonObject o = QJsonObject::fromVariantMap(p.second);
		if (o.contains("broadcast"))
			continue;
		list.append(QJsonObject::fromVariantMap(p.second));
	}

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
	if (!m.contains("ignoredErrors")) m["ignoredErrors"] = QVariantList();

	return m;
}



/**
 * @brief Servers::findServer
 * @param host
 * @param port
 * @param ssl
 * @param username
 * @return
 */

int Servers::findServer(const QString &host, const int &port, const bool &ssl)
{
	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;

		if (d.value("host").toString() == host &&
			d.value("port").toInt() == port &&
			d.value("ssl").toBool() == ssl)
		{
			return m_serversModel->variantMapData()->at(i).first;
		}
	}

	return -1;
}



/**
 * @brief Servers::findServer
 * @param username
 * @param host
 * @param port
 * @param ssl
 * @return
 */

int Servers::findServer(const QString &username, const QString &host, const int &port, const bool &ssl)
{
	for (int i=0; i<m_serversModel->variantMapData()->size(); i++) {
		QVariantMap d = m_serversModel->variantMapData()->at(i).second;

		if (d.value("host").toString() == host &&
			d.value("port").toInt() == port &&
			d.value("ssl").toBool() == ssl &&
			d.value("username").toString() == username)
		{
			return m_serversModel->variantMapData()->at(i).first;
		}
	}

	return -1;
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
		if (m_googleOAuth2->id() == id && m_googleOAuth2->key() == key && m_googleOAuth2->handlerPort() == port)
			return;

		setGoogleOAuth2(nullptr);
	}

	if (!id.isEmpty() && !key.isEmpty() && port > 0) {
		setGoogleOAuth2(new GoogleOAuth2(this));
		m_googleOAuth2->setClient(id, key);
		m_googleOAuth2->setHandlerPort(port);
	}
}
