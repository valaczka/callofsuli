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
#include "multiplayerengine.h"


/**
 * @brief WebSocketServer::WebSocketServer
 * @param ssl
 * @param parent
 */

WebServer::WebServer(ServerService *service)
	: QObject(nullptr)
	, m_service(service)
	, m_handler(new Handler(service, this))
	, m_webSocketHandler(this, m_service)
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

		WebSocketStream *stream = webSocketAdd(ws);

		if (!stream) {
			LOG_CERROR("service") << "WebSocketStream add failed";
			return;
		}

		stream->sendHello();
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

WebSocketStreamHandler::WebSocketStreamHandler(WebServer *server, ServerService *service)
	: m_server(server)
	, m_service(service)
{
	LOG_CTRACE("service") << "WebSocketStreamHandler created" << this;
}


/**
 * @brief WebSocketStreamHandler::~WebSocketStreamHandler
 */

WebSocketStreamHandler::~WebSocketStreamHandler()
{
	LOG_CTRACE("service") << "WebSocketStreamHandler destroying" << this;

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
	LOG_CTRACE("service") << "Add WebSocket" << ws;

	QMutexLocker locker(&m_mutex);

	const auto &it = std::find_if(m_streams.begin(), m_streams.end(), [ws](const std::unique_ptr<WebSocketStream> &s) {
		return (s->socket() == ws);
	});

	if (it != m_streams.end()) {
		LOG_CTRACE("service") << "WebSocketSteram already EXISTS!" << ws;
		return it->get();
	}

	m_streams.push_back(std::make_unique<WebSocketStream>(m_service, ws));

	WebSocketStream *stream = m_streams.back().get();

	LOG_CTRACE("service") << "WebSocketStream added" << &m_streams.back();

	return stream;
}



/**
 * @brief WebSocketStreamHandler::webSocketRemove
 * @param ws
 */

void WebSocketStreamHandler::webSocketRemove(WebSocketStream *ws)
{
	LOG_CTRACE("service") << "Remove WebSocketStream" << ws;

	if (!ws)
		return;

	QMutexLocker locker(&m_mutex);

	for (auto it = m_streams.begin(); it != m_streams.end(); ) {
		if (it->get() == ws) {
			it = m_streams.erase(it);
		} else
			++it;
	}
}





/**
 * @brief WebSocketStreamHandler::trigger
 * @param type
 */

void WebSocketStreamHandler::trigger(const WebSocketStream::StreamType &type)
{
	auto list = _triggerEvent(type);

	switch (type) {
	case WebSocketStream::StreamPeers:
		_trPeers(list);
		break;
	case WebSocketStream::StreamMultiPlayer:
		_trMultiPlayer(list, -1);
		break;
	default:
		LOG_CERROR("service") << "Trigger not defined" << type;
	}

}


/**
 * @brief WebSocketStreamHandler::trigger
 * @param type
 * @param data
 */

void WebSocketStreamHandler::trigger(const WebSocketStream::StreamType &type, const QVariant &data)
{
	auto list = _triggerEvent(type, data);

	switch (type) {
	case WebSocketStream::StreamPeers:
		_trPeers(list);
		break;
	case WebSocketStream::StreamMultiPlayer:
		_trMultiPlayer(list, data.canConvert<int>() ? data.toInt() : -1);
		break;
	default:
		LOG_CERROR("service") << "Trigger not defined" << type;
	}
}




/**
 * @brief WebSocketStreamHandler::trigger
 * @param stream
 */

void WebSocketStreamHandler::trigger(WebSocketStream *stream)
{
	if (!stream)
		return;

	QMutexLocker locker(&m_mutex);

	for (const auto &ob : stream->observers()) {
		switch (ob.type) {
		case WebSocketStream::StreamPeers:
			_trPeers({stream});
			break;
		case WebSocketStream::StreamMultiPlayer:
			_trMultiPlayer({stream}, -1);
			break;
		default:
			LOG_CERROR("service") << "Trigger not defined" << ob.type;
		}
	}
}



/**
 * @brief WebSocketStreamHandler::closeAll
 */

void WebSocketStreamHandler::closeAll()
{
	QMutexLocker locker(&m_mutex);

	for (const auto &stream : std::as_const(m_streams))
		stream->close();

	m_streams.clear();
}





/**
 * @brief WebSocketStreamHandler::triggerEvent
 * @param type
 * @return
 */

QVector<WebSocketStream*> WebSocketStreamHandler::_triggerEvent(const WebSocketStream::StreamType &type)
{
	QMutexLocker locker(&m_mutex);
	QVector<WebSocketStream*> list;

	for (const auto &stream : std::as_const(m_streams)) {
		if (stream->hasObserver(type))
			list.append(stream.get());
	}

	return list;
}



/**
 * @brief WebSocketStreamHandler::triggerEvent
 * @param type
 * @param data
 * @return
 */

QVector<WebSocketStream*> WebSocketStreamHandler::_triggerEvent(const WebSocketStream::StreamType &type, const QVariant &data)
{
	QMutexLocker locker(&m_mutex);
	QVector<WebSocketStream*> list;

	for (const auto &stream : std::as_const(m_streams)) {
		if (stream->hasObserver(type, data))
			list.append(stream.get());
	}

	return list;
}




/**
 * @brief WebSocketStreamHandler::_trPeers
 * @param list
 */

void WebSocketStreamHandler::_trPeers(const QVector<WebSocketStream*> &list)
{
	for (auto ws : list) {
		ws->sendJson("peers", PeerUser::toJson(&(m_service->peerUser())));
	}
}


/**
 * @brief WebSocketStreamHandler::_trMultiPlayer
 * @param list
 */

void WebSocketStreamHandler::_trMultiPlayer(const QVector<WebSocketStream *> &list, const int &engineId)
{
	MultiPlayerEngine::handleWebSocketTrigger(list, m_service, engineId);
}


