/*
 * ---- Call of Suli ----
 *
 * websocketserver.cpp
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WebSocketServer
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

#include "webserver.h"
#include "Logger.h"
#include "serversettings.h"
#include "serverservice.h"


/**
 * @brief WebSocketServer::WebSocketServer
 * @param ssl
 * @param parent
 */

WebServer::WebServer(ServerService *service)
	: QObject(service)
	, m_service(service)
	, m_handler(new Handler(service, this))
{
	Q_ASSERT(m_service);

	LOG_CDEBUG("client") << "Socket server created:" << this;

	/*connect(this, &QWebSocketServer::acceptError, this, &WebSocketServer::onAcceptError);
	connect(this, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
	connect(this, &QWebSocketServer::peerVerifyError, this, &WebSocketServer::onPeerVerifyError);
	connect(this, &QWebSocketServer::serverError, this, &WebSocketServer::onServerError);
	connect(this, &QWebSocketServer::sslErrors, this, &WebSocketServer::onSslErrors);*/

}


/**
 * @brief WebSocketServer::~WebSocketServer
 */

WebServer::~WebServer()
{
	delete m_handler;
	LOG_CDEBUG("client") << "Web socket server destroyed";
}




bool WebServer::start()
{
	ServerSettings *settings = m_service->settings();

	HttpServerConfig configuration;
	configuration.host = settings->listenAddress();
	configuration.port = settings->listenPort();

	configuration.errorDocumentMap[HttpStatus::NotFound] = QStringLiteral(":/html/html_error.html");

	if (settings->ssl()) {
		/*QSslConfiguration config = loadSslConfiguration(*m_service->settings());

		if (!config.isNull())
			setSslConfiguration(config);*/

		configuration.sslCertPath = settings->dataDir().absoluteFilePath(settings->certFile());
		configuration.sslKeyPath = settings->dataDir().absoluteFilePath(settings->certKeyFile());
	}

	m_server = new HttpServer(configuration, m_handler, this);

	if (!m_server->listen())	{
		LOG_CERROR("client") << "Can't listening on host " << settings->listenAddress() << " port " << settings->listenPort();
		return false;
	}


	LOG_CINFO("client") << tr("A szerver elindult, elérhető a következő címeken:");
	LOG_CINFO("client") << tr("====================================================");

	foreach (QHostAddress h, QNetworkInterface::allAddresses()) {
		if (!h.isGlobal())
			continue;

		if (settings->listenAddress() == QHostAddress::Any || settings->listenAddress() == QHostAddress::AnyIPv4 ||
				settings->listenAddress() == QHostAddress::AnyIPv6 || settings->listenAddress() == h) {

			QUrl u;
			u.setScheme(m_server->getSslConfig() ? QStringLiteral("https") : QStringLiteral("http"));
			u.setHost(h.toString());
			u.setPort(settings->listenPort());

			LOG_CINFO("client") << u.toString();

			if (m_redirectHost.isEmpty())
				setRedirectHost(h.toString());
		}

	}

	LOG_CINFO("client") << tr("====================================================");


	/*QFile certFile(base+m_socketCert);
		QFile keyFile(base+m_socketKey);

		if (!certFile.exists()) {
			qCritical().noquote() << tr("Server certificate doesn't exists: %1").arg(certFile.fileName());
			return false;
		}

		if (!keyFile.exists()) {
			qCritical().noquote() << tr("Server key doesn't exists: %1").arg(keyFile.fileName());
			return false;
		}

		certFile.open(QIODevice::ReadOnly);
		keyFile.open(QIODevice::ReadOnly);

		QSslCertificate cert(&certFile, QSsl::Pem);
		QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem);

		certFile.close();
		keyFile.close();

		if (cert.isNull()) {
			qCritical().noquote() << tr("Invalid server certificate: %1").arg(certFile.fileName());
			return false;
		}

		if (key.isNull()) {
			qCritical().noquote() << tr("Invalid key: %1").arg(keyFile.fileName());
			return false;
		}

		qInfo().noquote() << tr("Szerver tanúsítvány: %1").arg(certFile.fileName());
		qInfo().noquote() << tr("Szerver kulcs: %1").arg(keyFile.fileName());


		QSslConfiguration config;
		config.setLocalCertificate(cert);
		config.setPrivateKey(key);
		config.setPeerVerifyMode(QSslSocket::VerifyNone);
		m_socketServer->setSslConfiguration(config);*/


	/*connect(m_socketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	//connect(m_socketServer, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)));
	connect(m_socketServer, &QWebSocketServer::sslErrors, this, &Server::onSslErrors);

	if (m_port <= 0)
		m_port = 10101;

	if (!m_socketServer->listen(QHostAddress::Any, m_port))	{
		qCritical().noquote() << QString(tr("Cannot listen on port %1")).arg(m_port);
		return false;
	}

	qDebug("Maximum pending connections: %d", m_socketServer->maxPendingConnections());


	qInfo().noquote() << tr("A szerver elindult, elérhető a következő címeken:");
	qInfo().noquote() << tr("====================================================");

	foreach (QHostAddress h, QNetworkInterface::allAddresses()) {
		if (!h.isGlobal())
			continue;

		qInfo().noquote() << QString("%1://%2:%3")
							 .arg(m_socketServer->secureMode() == QWebSocketServer::SecureMode ? "wss" : "ws")
							 .arg(h.toString())
							 .arg(m_port);

		if (m_host.isEmpty()) {
			m_host = h.toString();
		}

	}

	qInfo().noquote() << tr("====================================================");

	qInfo().noquote() << tr("Szerver hoszt: %1").arg(m_host);

	quint32 udpPort = SERVER_UDP_PORT;

	if (m_udpSocket->bind(udpPort, QUdpSocket::ShareAddress)) {
		qInfo().noquote() << tr("Figyelt UDP port: %1").arg(udpPort);
	}

*/
	return true;
}




/**
 * @brief WebSocketServer::loadSslConfiguration
 * @param certFilePath
 * @param keyFilePath
 * @return
 */

QSslConfiguration WebServer::loadSslConfiguration(const ServerSettings &settings)
{
	QSslConfiguration c;
	if (!QSslSocket::supportsSsl()) {
		LOG_CERROR("websocket") << "Platform doesn't support SSL";
		return c;
	}

	const QString &certFilePath = settings.dataDir().absoluteFilePath(settings.certFile());
	const QString &keyFilePath = settings.dataDir().absoluteFilePath(settings.certKeyFile());

	QFile certFile(certFilePath);
	QFile keyFile(keyFilePath);

	if (!certFile.exists()) {
		LOG_CERROR("websocket") << "Server certificate doesn't exists:" << qPrintable(certFile.fileName());
		return c;
	}

	if (!keyFile.exists()) {
		LOG_CERROR("websocket") << "Server certificate key doesn't exists:" << qPrintable(keyFile.fileName());
		return c;
	}

	certFile.open(QIODevice::ReadOnly);
	keyFile.open(QIODevice::ReadOnly);

	QSslCertificate cert(&certFile, QSsl::Pem);
	QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem);

	certFile.close();
	keyFile.close();

	if (cert.isNull()) {
		LOG_CERROR("websocket") << "Invalid certificate:" << qPrintable(certFile.fileName());
		return c;
	}

	if (key.isNull()) {
		LOG_CERROR("websocket") << "Invalid key:" << qPrintable(keyFile.fileName());
		return c;
	}

	c.setLocalCertificate(cert);
	c.setPrivateKey(key);
	c.setPeerVerifyMode(QSslSocket::VerifyNone);

	return c;
}


/**
 * @brief WebServer::redirectUrl
 * @return
 */

const QString &WebServer::redirectHost() const
{
	return m_redirectHost;
}

void WebServer::setRedirectHost(const QString &newRedirectUrl)
{
	m_redirectHost = newRedirectUrl;
}

Handler *WebServer::handler() const
{
	return m_handler;
}



HttpServer *WebServer::server() const
{
	return m_server;
}


/**
 * @brief WebSocketServer::onNewConnection
 */
/*
void WebSocketServer::onNewConnection()
{
	while (hasPendingConnections()) {
		QWebSocket *socket = nextPendingConnection();
		Client *client = new Client(socket, m_service);
		m_service->clientAdd(client);
	}
}
*/

