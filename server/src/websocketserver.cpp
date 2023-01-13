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

#include "websocketserver.h"
#include "Logger.h"
#include "serversettings.h"
#include "serverservice.h"


/**
 * @brief WebSocketServer::WebSocketServer
 * @param ssl
 * @param parent
 */

WebSocketServer::WebSocketServer(const QWebSocketServer::SslMode &ssl, ServerService *service)
	: QWebSocketServer{QStringLiteral("callofsuli-websocket-server"), ssl, service}
	, m_service(service)
{
	Q_ASSERT(m_service);

	LOG_CINFO("websocket") << "Web socket server created:" << ssl;

	connect(this, &QWebSocketServer::acceptError, this, &WebSocketServer::onAcceptError);
	connect(this, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
	connect(this, &QWebSocketServer::peerVerifyError, this, &WebSocketServer::onPeerVerifyError);
	connect(this, &QWebSocketServer::serverError, this, &WebSocketServer::onServerError);
	connect(this, &QWebSocketServer::sslErrors, this, &WebSocketServer::onSslErrors);

}


/**
 * @brief WebSocketServer::~WebSocketServer
 */

WebSocketServer::~WebSocketServer()
{
	LOG_CINFO("websocket") << "Web socket server destroyed";
}




bool WebSocketServer::start()
{
	ServerSettings *settings = m_service->settings();

	if (settings->ssl()) {
		QSslConfiguration config = loadSslConfiguration(*m_service->settings());

		if (!config.isNull())
			setSslConfiguration(config);
	}


	if (!listen(settings->listenAddress(), settings->listenPort()))	{
		LOG_CERROR("websocket") << "Can't listening on host " << settings->listenAddress() << " port " << settings->listenPort();
		return false;
	}

	LOG_CINFO("websocket") << "Listening on:" << serverUrl();

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

QSslConfiguration WebSocketServer::loadSslConfiguration(const ServerSettings &settings)
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
 * @brief WebSocketServer::onNewConnection
 */

void WebSocketServer::onNewConnection()
{
	while (hasPendingConnections()) {
		QWebSocket *socket = nextPendingConnection();
		Client *client = new Client(socket, m_service);
		m_service->clientAdd(client);
	}
}



/**
 * @brief WebSocketServer::onAcceptError
 * @param socketError
 */

void WebSocketServer::onAcceptError(const QAbstractSocket::SocketError &socketError)
{
	LOG_CWARNING("websocket") << "acceptError:" << socketError;
}


/**
 * @brief WebSocketServer::onPeerVerifyError
 * @param error
 */

void WebSocketServer::onPeerVerifyError(const QSslError &error)
{
	LOG_CWARNING("websocket") << "peerVerifyError:" << error;
}


/**
 * @brief WebSocketServer::onServerError
 * @param closeCode
 */

void WebSocketServer::onServerError(const QWebSocketProtocol::CloseCode &closeCode)
{
	LOG_CWARNING("websocket") << "serverError:" << closeCode;
}


/**
 * @brief WebSocketServer::onSslErrors
 * @param errors
 */

void WebSocketServer::onSslErrors(const QList<QSslError> &errors)
{
	LOG_CWARNING("websocket") << "sslErrors:" << errors;
}
