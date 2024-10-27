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
#include <QSslKey>
#include "serversettings.h"
#include "serverservice.h"
#include <QNetworkInterface>


/**
 * @brief WebSocketServer::WebSocketServer
 * @param ssl
 * @param parent
 */

WebServer::WebServer(ServerService *service)
	: QObject(nullptr)
	, m_service(service)
	, m_handler(new Handler(service, this))
{
	Q_ASSERT(m_service);

	LOG_CDEBUG("service") << "Web server created";
}


/**
 * @brief WebSocketServer::~WebSocketServer
 */

WebServer::~WebServer()
{
	LOG_CDEBUG("service") << "Web server destroyed";
}




/**
 * @brief WebServer::start
 * @return
 */

bool WebServer::start()
{
	ServerSettings *settings = m_service->settings();

	m_server.reset(new QHttpServer);

	m_handler->loadRoutes();

	// WebSocket connection

	m_server.get()->route("/ws", [](const QHttpServerRequest &) {
		return QFuture<QHttpServerResponse>();
	});

	connect(m_server.get(), &QAbstractHttpServer::newWebSocketConnection, this, &WebServer::onWebSocketConnection);

	LOG_CTRACE("service") << "HttpServer created";

	// SSL

	if (settings->ssl()) {
		const auto &configuration = loadSslConfiguration(*settings);

		if (!configuration) {
			LOG_CERROR("service") << "Invalid SSL configuration";
			return false;
		}


		QSslServer *s = new QSslServer;
		s->setSslConfiguration(configuration.value());
		m_tcpServer.reset(s);

	} else {
		m_tcpServer.reset(new QTcpServer);
	}


	Q_ASSERT(m_tcpServer);

	// Listen

	if (!m_tcpServer->listen(settings->listenAddress(), settings->listenPort()))	{
		LOG_CERROR("service") << "Can't listening on host " << settings->listenAddress() << " port " << settings->listenPort();
		return false;
	}

	if (!m_server->bind(m_tcpServer.get())) {
		LOG_CERROR("service") << "Can't bind QTcpServer";
		return false;
	}


	LOG_CINFO("service") << qPrintable(tr("A szerver elindult, elérhető a következő címeken:"));
	LOG_CINFO("service") << qPrintable("====================================================");

	foreach (QHostAddress h, QNetworkInterface::allAddresses()) {
		if (!h.isGlobal())
			continue;

		if (settings->listenAddress() == QHostAddress::Any || settings->listenAddress() == QHostAddress::AnyIPv4 ||
				settings->listenAddress() == QHostAddress::AnyIPv6 || settings->listenAddress() == h) {

			QUrl u;
			u.setScheme(settings->ssl() ? QStringLiteral("https") : QStringLiteral("http"));
			u.setHost(h.toString());
			u.setPort(settings->listenPort());

			LOG_CINFO("service") << qPrintable(u.toString());

			if (m_redirectHost.isEmpty())
				setRedirectHost(h.toString());
		}

	}

	LOG_CINFO("service") << qPrintable("====================================================");

	return true;
}




/**
 * @brief WebSocketServer::loadSslConfiguration
 * @param certFilePath
 * @param keyFilePath
 * @return
 */

std::optional<QSslConfiguration> WebServer::loadSslConfiguration(const ServerSettings &settings)
{
	if (!QSslSocket::supportsSsl()) {
		LOG_CERROR("websocket") << "Platform doesn't support SSL";
		return std::nullopt;
	}

	const QString &certFilePath = settings.dataDir().absoluteFilePath(settings.certFile());
	const QString &keyFilePath = settings.dataDir().absoluteFilePath(settings.certKeyFile());

	QFile certFile(certFilePath);
	QFile keyFile(keyFilePath);

	if (!certFile.exists()) {
		LOG_CERROR("websocket") << "Server certificate doesn't exists:" << qPrintable(certFile.fileName());
		return std::nullopt;
	}

	if (!keyFile.exists()) {
		LOG_CERROR("websocket") << "Server certificate key doesn't exists:" << qPrintable(keyFile.fileName());
		return std::nullopt;
	}

	certFile.open(QIODevice::ReadOnly);
	keyFile.open(QIODevice::ReadOnly);

	QSslCertificate cert(&certFile, QSsl::Pem);
	QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem);

	certFile.close();
	keyFile.close();

	if (cert.isNull()) {
		LOG_CERROR("websocket") << "Invalid certificate:" << qPrintable(certFile.fileName());
		return std::nullopt;
	}

	if (key.isNull()) {
		LOG_CERROR("websocket") << "Invalid key:" << qPrintable(keyFile.fileName());
		return std::nullopt;
	}

	QSslConfiguration c;

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


/**
 * @brief WebServer::server
 * @return
 */

QHttpServer *WebServer::server() const
{
	return m_server.get();
}

Handler* WebServer::handler() const
{
	return m_handler.get();
}




/**
 * @brief WebServer::onNewWebsSocketConnection
 */

void WebServer::onWebSocketConnection()
{
	LOG_CDEBUG("service") << "New WebSocket connection";

	while (m_server.get()->hasPendingWebSocketConnections()) {
		auto ptr = m_server.get()->nextPendingWebSocketConnection();
		QWebSocket *ws = ptr.get();
		ptr.release();

		m_service->engineHandler()->websocketAdd(ws);
	}
}




