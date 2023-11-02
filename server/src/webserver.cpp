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
	, m_webSocketHandler(this)
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




/**
 * @brief WebServer::start
 * @return
 */

bool WebServer::start()
{
	ServerSettings *settings = m_service->settings();

	m_server = std::make_shared<QHttpServer>(this);

	m_handler->loadRoutes();

	// WebSocket connection

	m_server.get()->route("/ws", [](QHttpServerResponder &&) {});
	connect(m_server.get(), &QAbstractHttpServer::newWebSocketConnection, this, &WebServer::onWebSocketConnection);

	LOG_CTRACE("service") << "HttpServer created";

	// SSL

	if (settings->ssl()) {
		const auto &configuration = loadSslConfiguration(*settings);

		if (!configuration) {
			LOG_CERROR("service") << "Invalid SSL configuration";
			return false;
		}

		m_server->sslSetup(configuration.value());
	}

	// Listen

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

			LOG_CINFO("service") << qPrintable(u.toString());

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

		auto stream = webSocketAdd(ws);

		if (!stream) {
			LOG_CERROR("service") << "Error";
			return;
		}

		LOG_CDEBUG("service") << "Added" << stream << stream->socket();
	}
}



/**
 * @brief WebServer::webSocketHandler
 * @return
 */

WebSocketStreamHandler &WebServer::webSocketHandler()
{
	return m_webSocketHandler;
}




/**
 * @brief WebSocketStreamHandler::webSocketAdd
 * @param ws
 */

WebSocketStreamHandler::WebSocketStreamHandler(WebServer *server)
	: m_server(server)
{
	LOG_CTRACE("service") << "WebSocketStreamHandler created" << this;
}


/**
 * @brief WebSocketStreamHandler::~WebSocketStreamHandler
 */

WebSocketStreamHandler::~WebSocketStreamHandler()
{
	LOG_CTRACE("service") << "WebSocketStreamHandler destroying" << this;

	QMutexLocker locker(&m_mutex);

	m_streams.clear();

	LOG_CTRACE("service") << "WebSocketStreamHandler destroyed" << this;
}



/**
 * @brief WebSocketStreamHandler::webSocketAdd
 * @param ws
 * @return
 */

WebSocketStream *WebSocketStreamHandler::webSocketAdd(QWebSocket *ws)
{
	LOG_CTRACE("service") << "Add WebSocketStream" << ws;

	if (!ws)
		return nullptr;

	QMutexLocker locker(&m_mutex);

	const auto &it = std::find_if(m_streams.begin(), m_streams.end(), [ws](const WebSocketStream &s) {
		return (s.socket() == ws);
	});

	if (it != m_streams.end()) {
		LOG_CTRACE("service") << "WebSocketSteram already EXISTS!" << ws;
		return &(*it);
	}


	QObject::connect(ws, &QWebSocket::disconnected, m_server, [ws, this]() {
		LOG_CTRACE("service") << "WebSocket disconnected";
		LOG_CTRACE("service") << "WebSocket disconnected:" << ws;
		m_server->webSocketRemove(ws);
	});


	m_streams.append(ws);

	LOG_CTRACE("service") << "WebSocketStream added" << &m_streams.last();

	return &m_streams.last();
}



/**
 * @brief WebSocketStreamHandler::webSocketRemove
 * @param ws
 */

void WebSocketStreamHandler::webSocketRemove(QWebSocket *ws)
{
	LOG_CTRACE("service") << "Remove WebSocket" << ws;

	if (!ws)
		return;

	QMutexLocker locker(&m_mutex);

	for (auto it = m_streams.constBegin(); it != m_streams.constEnd(); ) {
		if (it->socket() == ws) {
			it = m_streams.erase(it);
		} else
			++it;
	}

	LOG_CTRACE("service") << "WebServer removed" << ws;
}


/**
 * @brief WebSocketStreamHandler::runTest
 */

void WebSocketStreamHandler::runTest()
{
	QMutexLocker locker(&m_mutex);

	for (auto it = m_streams.begin(); it != m_streams.end(); ++it)
		it->sendTextMessage("szia, ezt üzenem neked!");

}






/**
 * @brief WebSocketStreamHandler::triggerEvent
 * @param type
 * @return
 */

QList<QPointer<QWebSocket> > WebSocketStreamHandler::triggerEvent(const WebSocketStream::StreamType &type)
{
	QMutexLocker locker(&m_mutex);
	QList<QPointer<QWebSocket> > list;

	for (const WebSocketStream &stream : m_streams) {
		if (stream.hasObserver(type))
			list.append(stream.socket());
	}

	return list;
}



/**
 * @brief WebSocketStreamHandler::triggerEvent
 * @param type
 * @param data
 * @return
 */

QList<QPointer<QWebSocket> > WebSocketStreamHandler::triggerEvent(const WebSocketStream::StreamType &type, const QVariant &data)
{
	QMutexLocker locker(&m_mutex);
	QList<QPointer<QWebSocket> > list;

	for (const WebSocketStream &stream : m_streams) {
		if (stream.hasObserver(type, data))
			list.append(stream.socket());
	}

	return list;
}





/**
 * @brief WebSocketStream::WebSocketStream
 * @param socket
 */

WebSocketStream::WebSocketStream(QWebSocket *socket)
	: m_socket(socket)
{
	LOG_CTRACE("service") << "WebSocketStream created" << this;
}


/**
 * @brief WebSocketStream::~WebSocketStream
 */

WebSocketStream::~WebSocketStream()
{
	LOG_CTRACE("service") << "WebSocketStream destroyed" << this;
}


/**
 * @brief WebSocketStream::observerAdd
 * @param type
 * @param data
 */

void WebSocketStream::observerAdd(const StreamType &type, const QVariant &data)
{
	QMutexLocker locker(&m_mutex);
	m_observers.append(StreamObserver{type, data});
}


/**
 * @brief WebSocketStream::observerRemove
 * @param type
 * @param data
 */

void WebSocketStream::observerRemove(const StreamType &type, const QVariant &data)
{
	QMutexLocker locker(&m_mutex);
	m_observers.removeAll(StreamObserver{type, data});
}


/**
 * @brief WebSocketStream::observerRemoveAll
 * @param type
 */

void WebSocketStream::observerRemoveAll(const StreamType &type)
{
	QMutexLocker locker(&m_mutex);
	for (auto it=m_observers.constBegin(); it != m_observers.constEnd();) {
		if (it->type == type)
			it = m_observers.erase(it);
		else
			++it;
	}
}


/**
 * @brief WebSocketStream::hasObserver
 * @param type
 * @return
 */

bool WebSocketStream::hasObserver(const StreamType &type) const
{
	QMutexLocker locker(&m_mutex);
	for (auto it=m_observers.constBegin(); it != m_observers.constEnd();) {
		if (it->type == type)
			return true;
	}

	return false;
}

/**
 * @brief WebSocketStream::hasObserver
 * @param type
 * @param data
 * @return
 */

bool WebSocketStream::hasObserver(const StreamType &type, const QVariant &data) const
{
	QMutexLocker locker(&m_mutex);
	return m_observers.contains(StreamObserver{type, data});
}


/**
 * @brief WebSocketStream::observers
 * @return
 */

const QVector<WebSocketStream::StreamObserver> &WebSocketStream::observers() const
{
	return m_observers;
}


/**
 * @brief WebSocketStream::setObservers
 * @param newObservers
 */

void WebSocketStream::setObservers(const QVector<StreamObserver> &newObservers)
{
	m_observers = newObservers;
}
