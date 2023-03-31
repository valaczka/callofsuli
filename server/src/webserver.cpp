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
	//configuration.verbosity = HttpServerConfig::Verbose::All;

	configuration.errorDocumentMap[HttpStatus::NotFound] = QStringLiteral(":/html/html_error.html");

	if (settings->ssl()) {
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

