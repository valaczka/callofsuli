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

	LOG_CDEBUG("service") << "Web server created:" << this;
}


/**
 * @brief WebSocketServer::~WebSocketServer
 */

WebServer::~WebServer()
{
	LOG_CDEBUG("service") << "Web server destroyed";
}




bool WebServer::start()
{
	ServerSettings *settings = m_service->settings();

#ifdef _COMPAT
	m_configuration.errorDocumentMap[HttpStatus::NotFound] = QStringLiteral(":/html/html_error.html");
#endif


	m_server = std::make_shared<QHttpServer>(this);

	m_handler->loadRoutes();

	LOG_CTRACE("service") << "HttpServer created";

	if (settings->ssl()) {
		const auto &configuration = loadSslConfiguration(*settings);

		if (!configuration) {
			LOG_CERROR("service") << "Invalid SSL configuration";
			return false;
		}

		m_server->sslSetup(configuration.value());
	}

	if (!m_server->listen(settings->listenAddress(), settings->listenPort()))	{
		LOG_CERROR("service") << "Can't listening on host " << settings->listenAddress() << " port " << settings->listenPort();
		return false;
	}

	LOG_CINFO("service") << qPrintable(tr("A szerver elindult, elérhető a következő címeken:"));
	LOG_CINFO("service") << tr("====================================================");

	foreach (QHostAddress h, QNetworkInterface::allAddresses()) {
		if (!h.isGlobal())
			continue;

		if (settings->listenAddress() == QHostAddress::Any || settings->listenAddress() == QHostAddress::AnyIPv4 ||
				settings->listenAddress() == QHostAddress::AnyIPv6 || settings->listenAddress() == h) {

			QUrl u;
			u.setScheme(settings->ssl() ? QStringLiteral("https") : QStringLiteral("http"));
			u.setHost(h.toString());
			u.setPort(settings->listenPort());

			LOG_CINFO("service") << u.toString();

			if (m_redirectHost.isEmpty())
				setRedirectHost(h.toString());
		}

	}

	LOG_CINFO("service") << tr("====================================================");

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

std::weak_ptr<QHttpServer> WebServer::server() const
{
	return m_server;
}

Handler* WebServer::handler() const
{
	return m_handler.get();
}



