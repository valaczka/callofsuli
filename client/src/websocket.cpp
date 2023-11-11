#include "websocket.h"
#include "Logger.h"
#include "client.h"
#include "server.h"
#include "httpconnection.h"
#include "utils_.h"
#include <QFile>

/**
 * @brief WebSocket::WebSocket
 * @param connection
 */

WebSocket::WebSocket(HttpConnection *connection)
	: QObject{}
	, m_connection(connection)
{
	LOG_CTRACE("http") << "WebSocket created" << this;
}


/**
 * @brief WebSocket::~WebSocket
 */

WebSocket::~WebSocket()
{
	LOG_CTRACE("http") << "WebSocket destroyed" << this;
}


/**
 * @brief WebSocket::active
 * @return
 */

bool WebSocket::active() const
{
	return m_socket && m_socket->isValid() && m_state == WebSocketListening;
}


/**
 * @brief WebSocket::connect
 */

void WebSocket::connect()
{
	Server *server = m_connection->server();

	if (!server)
		return;

	if (!m_socket) {
		m_socket = std::make_unique<QWebSocket>();
		m_state = WebSocketReset;

		QObject::connect(m_socket.get(), &QWebSocket::destroyed, this, [this](){ qDebug() << "******" << sender(); });

		QObject::connect(m_socket.get(), &QWebSocket::connected, this, &WebSocket::onConnected);
		QObject::connect(m_socket.get(), &QWebSocket::disconnected, this, &WebSocket::onDisconnected);
		QObject::connect(m_socket.get(), &QWebSocket::errorOccurred, this, &WebSocket::onError);
		QObject::connect(m_socket.get(), &QWebSocket::textMessageReceived, this, &WebSocket::onTextReceived);

#ifndef QT_NO_SSL
		if (!QSslSocket::supportsSsl())
			LOG_CERROR("http") << "Platform doesn't support SSL";
		else {
			if (!m_connection->m_rootCertificate.isNull()) {
				QSslConfiguration config = QSslConfiguration::defaultConfiguration();
				config.addCaCertificate(m_connection->m_rootCertificate);
				m_socket->setSslConfiguration(config);
			}

			QObject::connect(m_socket.get(), &QWebSocket::sslErrors, this, [this](const QList<QSslError> &e){
				Server *server = m_connection->server();

				if (m_socket.get() && server && !server->certificate().isEmpty()) {
					QSslCertificate cert(server->certificate(), QSsl::Pem);

					if (cert.isNull()) {
						LOG_CERROR("http") << "Invalid server certificate stored";
					} else {
						QList<QSslError> ignoredErrors;

						for (auto it=e.constBegin(); it != e.constEnd(); ++it) {
							if (it->certificate() != cert)
								LOG_CWARNING("http") << "Server certificate mismatch";
							else if (server->ignoredSslErrors().contains(it->error()))
								ignoredErrors.append(*it);

						}

						if (ignoredErrors.size() == e.size()) {
							LOG_CDEBUG("http") << "Ignore SSL errors:" << ignoredErrors;
							m_socket.get()->ignoreSslErrors(ignoredErrors);
							return;
						}
					}
				}
			});
		}
#else
		LOG_CERROR("http") << "Qt built without SSL support";
#endif
	}

	if (m_state != WebSocketReset)
		return;

	QUrl url = server->url();
	if (url.scheme() == QStringLiteral("https"))
		url.setScheme(QStringLiteral("wss"));
	else
		url.setScheme(QStringLiteral("ws"));

	url.setPath(QStringLiteral("/ws"));

	LOG_CDEBUG("http") << "Open WebSocket:" << qPrintable(url.toString());

	emit activeChanged();

	m_socket->open(url);

}



/**
 * @brief WebSocket::close
 */

void WebSocket::close()
{
	if (m_socket) {
		m_socket->close();
		m_state = WebSocketReset;
	}
}



/**
 * @brief WebSocket::onConnected
 */

void WebSocket::onConnected()
{
	LOG_CDEBUG("http") << "WebSocket connected";
	m_state = WebSocketConnected;

	emit activeChanged();
}



/**
 * @brief WebSocket::onDisconnected
 */

void WebSocket::onDisconnected()
{
	LOG_CDEBUG("http") << "WebSocket disconnected";

	auto ws = m_socket.release();
	m_state = WebSocketReset;

	ws->deleteLater();

	emit activeChanged();
}


/**
 * @brief WebSocket::onError
 * @param error
 */

void WebSocket::onError(const QAbstractSocket::SocketError &error)
{
	LOG_CERROR("http") << "WebSocket error:" << error;
	m_connection->m_client->snack("WebSocket error");
}



/**
 * @brief WebSocket::onTextReceived
 * @param text
 */

void WebSocket::onTextReceived(const QString &text)
{
	LOG_CDEBUG("http") << "WebSocket received:" << text;

	auto json = Utils::byteArrayToJsonObject(text.toUtf8());

	if (!json) {
		LOG_CWARNING("service") << "Invalid WebSocket JSON received:" << text;
		return;
	}

	onJsonReceived(*json);
}


/**
 * @brief WebSocket::onJsonReceived
 * @param json
 */

void WebSocket::onJsonReceived(const QJsonObject &json)
{
	const QString &operation = json.value(QStringLiteral("op")).toString();

	if ((m_state == WebSocketReset || m_state == WebSocketConnected) && operation == QStringLiteral("hello")) {
		m_state = WebSocketHelloReceived;

		LOG_CDEBUG("http") << "Authenticate WebSocket stream";
		Server *server = m_connection->server();
		if (!server)
			return;

		send(QJsonObject{
				 { QStringLiteral("token"), server->token()}
			 });

	} else if (m_state == WebSocketHelloReceived && operation == QStringLiteral("authenticated")) {
		LOG_CDEBUG("http") << "WebSocket authenticated";
		m_state = WebSocketAuthenticated;

		emit activeChanged();

		QJsonArray list;

		for (const auto &ob : std::as_const(m_observers)) {
			QJsonObject obj;
			obj.insert(QStringLiteral("type"), ob.type);

			if (!ob.data.isNull())
				obj.insert(QStringLiteral("data"), ob.data);

			list.append(obj);
		}

		if (!list.isEmpty()) {
			send(QJsonObject{
					 { QStringLiteral("op"), QStringLiteral("add") },
					 { QStringLiteral("d"), list },
				 });

			m_state = WebSocketListening;
			emit activeChanged();
		}
	}
}


/**
 * @brief WebSocket::send
 * @param json
 */

void WebSocket::send(const QJsonObject &json)
{
	if (m_socket && m_socket->isValid()) {
		m_socket->sendTextMessage(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
	}
}


/**
 * @brief WebSocket::observerAddValue
 * @param type
 * @param value
 */

void WebSocket::observerAddValue(const QString &type, const QJsonValue &value)
{
	Observer ob{type, value};

	if (!m_observers.contains(ob)) {
		m_observers.append(ob);

		if (m_socket && (m_state == WebSocketConnected || m_state == WebSocketListening)) {
			QJsonObject obj;
			obj.insert(QStringLiteral("type"), type);

			if (!value.isNull())
				obj.insert(QStringLiteral("data"), value);

			send(QJsonObject{
					 { QStringLiteral("op"), QStringLiteral("add") },
					 { QStringLiteral("d"), obj },
				 });

			m_state = WebSocketListening;
			emit activeChanged();
		}
	}
}



/**
 * @brief WebSocket::observerRemoveValue
 * @param type
 * @param value
 */

void WebSocket::observerRemoveValue(const QString &type, const QJsonValue &value)
{
	Observer ob{type, value};

	m_observers.removeAll(ob);

	if (m_observers.isEmpty())
		close();
}


