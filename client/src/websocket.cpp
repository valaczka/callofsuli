#include "websocket.h"
#include "Logger.h"
#include "client.h"
#include "server.h"
#include "httpconnection.h"
#include "utils_.h"
#include <QFile>


#define MAX_TRIES	5
#define INTERVAL_TRIES	3000

/**
 * @brief WebSocket::WebSocket
 * @param connection
 */

WebSocket::WebSocket(HttpConnection *connection)
	: QObject{}
	, m_connection(connection)
{
	LOG_CTRACE("http") << "WebSocket created" << this;

	m_timerConnect.setInterval(INTERVAL_TRIES);
	QObject::connect(&m_timerConnect, &QTimer::timeout, this, &WebSocket::reconnect);
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
	m_tries = 0;
	m_forceClose = false;

	reconnect();
}



/**
 * @brief WebSocket::close
 */

void WebSocket::close()
{
	m_forceClose = true;

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

	m_tries = 0;
	m_timerConnect.stop();

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

	if (!m_forceClose)
		m_timerConnect.start();
}


/**
 * @brief WebSocket::onError
 * @param error
 */

void WebSocket::onError(const QAbstractSocket::SocketError &error)
{
	LOG_CERROR("http") << "WebSocket error:" << error;
	m_connection->m_client->snack("WebSocket error");

	if (!m_forceClose)
		m_timerConnect.start();
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
	} else if (operation == QStringLiteral("error")) {
		LOG_CERROR("http") << "WebSocket error" << json;
		close();
		return;
	} else if (m_state == WebSocketListening) {
		const QJsonValue &data = json.value(QStringLiteral("d"));
		LOG_CTRACE("http") << "WebSocket message received" << operation << data;
		emit messageReceived(operation, data);
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


/**
 * @brief WebSocket::send
 * @param op
 * @param data
 */

void WebSocket::send(const QString &op, const QJsonValue &data)
{
	if (m_state != WebSocketListening) {
		LOG_CWARNING("http") << "WebSocket isn't listening";
		return;
	}

	send(QJsonObject{
			 { QStringLiteral("op"), op },
			 { QStringLiteral("d"), data },
		 });
}



/**
 * @brief WebSocket::reconnect
 */

void WebSocket::reconnect()
{
	Server *server = m_connection->server();

	if (!server)
		return;

	if (!m_socket) {
		LOG_CTRACE("http") << "Try to connect" << m_tries;

		if (++m_tries > MAX_TRIES) {
			LOG_CWARNING("http") << "Connection failed";
			m_connection->m_client->messageError(tr("Nem sikerült létrehozni a kapcsolatot"), tr("WebSocket hiba"));
			emit connectionFailed();
			return;
		}


		m_socket = std::make_unique<QWebSocket>();
		m_state = WebSocketReset;

		//QObject::connect(m_socket.get(), &QWebSocket::destroyed, this, [this](){ qDebug() << "******" << sender(); });

		QObject::connect(m_socket.get(), &QWebSocket::connected, this, &WebSocket::onConnected);
		QObject::connect(m_socket.get(), &QWebSocket::disconnected, this, &WebSocket::onDisconnected);
#if QT_VERSION >= 0x060000
		QObject::connect(m_socket.get(), &QWebSocket::errorOccurred, this, &WebSocket::onError);
#else
		QObject::connect(m_socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebSocket::onError);
#endif
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


