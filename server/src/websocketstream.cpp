#include "websocketstream.h"
#include "serverservice.h"
#include "Logger.h"




/**
 * @brief WebSocketStream::WebSocketStream
 * @param socket
 */

WebSocketStream::WebSocketStream(ServerService *service, QWebSocket *socket)
	: QObject()
	, m_service(service)
	, m_socket(std::move(socket))
{
	LOG_CTRACE("service") << "WebSocketStream created" << this;


	/*connect(m_socket.get(), &QWebSocket::destroyed, this, [this](){
		LOG_CINFO("service") << "WebSocketDestroyed" << m_socket.get();
	});*/

	connect(m_socket.get(), &QWebSocket::disconnected, this, &WebSocketStream::onWebSocketDisconnected);
	connect(m_socket.get(), &QWebSocket::textMessageReceived, this, &WebSocketStream::onTextReceived);
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

	StreamObserver ob{type, data};

	if (!m_observers.contains(ob))
		m_observers.append(ob);

	LOG_CTRACE("service") << "Observer added" << type << data;
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

	LOG_CTRACE("service") << "Observer removed" << type << data;
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

bool WebSocketStream::hasObserver(const StreamType &type)
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

bool WebSocketStream::hasObserver(const StreamType &type, const QVariant &data)
{
	QMutexLocker locker(&m_mutex);
	return m_observers.contains(StreamObserver{type, data});
}



/**
 * @brief WebSocketStream::close
 */

void WebSocketStream::close()
{
	if (!m_socket)
		return;

	LOG_CTRACE("service") << "DISCONNECT" << m_socket.get();
	disconnect(m_socket.get(), &QWebSocket::disconnected, this, &WebSocketStream::onWebSocketDisconnected);
	disconnect(m_socket.get(), &QWebSocket::textMessageReceived, this, &WebSocketStream::onTextReceived);

	auto ws = m_socket.release();

	ws->close();

	delete ws;
}


/**
 * @brief WebSocketStream::sendHello
 */

void WebSocketStream::sendHello()
{
	if (!m_socket)
		return;

	if (m_state != StateInvalid) {
		LOG_CWARNING("service") << "Invalid state" << m_state << __PRETTY_FUNCTION__;
		return;
	}

	sendJson("hello", QJsonObject{
				 { QStringLiteral("versionMajor"), ServerService::versionMajor() },
				 { QStringLiteral("versionMinor"), ServerService::versionMinor() },
			 });

	m_state = StateHelloSent;

}


/**
 * @brief WebSocketStream::sendJson
 * @param operation
 * @param data
 */

void WebSocketStream::sendJson(const char *operation, const QJsonValue &data)
{
	QJsonObject obj;

	obj.insert(QStringLiteral("op"), operation);
	if (!data.isNull())
		obj.insert(QStringLiteral("d"), data);

	QJsonDocument doc(obj);

	sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
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


/**
 * @brief WebSocketStream::state
 * @return
 */

WebSocketStream::StreamState WebSocketStream::state() const
{
	return m_state;
}



/**
 * @brief WebSocketStream::onTextReceived
 * @param text
 */

void WebSocketStream::onTextReceived(const QString &text)
{
	LOG_CTRACE("service") << "WebSocketStream text received:" << this;

	if (m_state == StateError) {
		sendJson("error", QStringLiteral("invalid stream"));
		return;
	}

	auto json = Utils::byteArrayToJsonObject(text.toUtf8());

	if (!json) {
		LOG_CWARNING("service") << "Invalid WebSocketStream JSON received:" << text;
		sendJson("error", QStringLiteral("invalid json"));
		return;
	}

	onJsonReceived(*json);
}



/**
 * @brief WebSocketStream::onJsonReceived
 * @param data
 */

void WebSocketStream::onJsonReceived(const QJsonObject &data)
{
	LOG_CTRACE("service") << "WebSocketStream data received:" << this << data;

	if (m_state != StateAuthenticated && !data.contains(QStringLiteral("token"))) {
		sendJson("error", QStringLiteral("invalid token"));
		m_state = StateError;
		return;
	}

	if (m_state == StateHelloSent) {
		const QString &token = data.value(QStringLiteral("token")).toString();

		if (!Credential::verify(token, m_service->settings()->jwtSecret(), m_service->config().get("tokenFirstIat").toInt(0))) {
			LOG_CDEBUG("service") << "Token verification failed" << this;
			sendJson("error", QStringLiteral("unauthorized"));
			m_state = StateError;
			return;
		}

		Credential c = Credential::fromJWT(token);

		if (!c.isValid()) {
			LOG_CDEBUG("service") << "Invalid token" << this;
			sendJson("error", QStringLiteral("unauthorized"));
			m_state = StateError;
			return;
		}

		m_credential = c;
		m_state = StateAuthenticated;
		sendJson("authenticated", m_credential.username());

		LOG_CDEBUG("service") << "WebSocketStream authenticated" << this << qPrintable(m_credential.username());
	}

	if (m_state != StateAuthenticated || !m_credential.isValid()) {
		LOG_CDEBUG("service") << "Unauthorized" << this;
		sendJson("error", QStringLiteral("unauthorized"));
		m_state = StateError;
		return;
	}

	const QString &operation = data.value(QStringLiteral("op")).toString();
	const QJsonValue &d = data.value(QStringLiteral("d"));

	if (operation.isEmpty())
		return;

	if (operation == QStringLiteral("add"))
		observerAdd(d);
	else if (operation == QStringLiteral("remove"))
		observerRemove(d);
	else {
		LOG_CDEBUG("service") << "Invalid operation:" << operation << qPrintable(m_credential.username());
		sendJson("error", QStringLiteral("invalid operation"));
		return;
	}
}



/**
 * @brief WebSocketStream::observerAdd
 * @param data
 */

void WebSocketStream::observerAdd(const QJsonValue &data)
{
	QVector<QJsonObject> list;

	if (data.isArray()) {
		const QJsonArray &a = data.toArray();
		for (const QJsonValue &v : std::as_const(a)) {
			list.append(v.toObject());
		}
	} else {
		list.append(data.toObject());
	}

	for (const QJsonObject &obj : std::as_const(list)) {
		const QString &type = obj.value(QStringLiteral("type")).toString();

		if (type == QStringLiteral("peers")) {
			observerAdd(StreamPeers);
		} else {
			LOG_CWARNING("service") << "Invalid observer:" << type;
		}
	}

	m_service->webServer().lock().get()->webSocketHandler().trigger(this);
}


/**
 * @brief WebSocketStream::observerRemove
 * @param data
 */

void WebSocketStream::observerRemove(const QJsonValue &data)
{
	QVector<QJsonObject> list;

	if (data.isArray()) {
		const QJsonArray &a = data.toArray();
		for (const QJsonValue &v : std::as_const(a)) {
			list.append(v.toObject());
		}
	} else {
		list.append(data.toObject());
	}

	for (const QJsonObject &obj : std::as_const(list)) {
		const QString &type = obj.value(QStringLiteral("type")).toString();

		if (type == QStringLiteral("peers")) {
			observerRemove(StreamPeers);
		} else {
			LOG_CWARNING("service") << "Invalid observer:" << type;
		}
	}
}


/**
 * @brief WebSocketStream::onWebSocketDisconnected
 */

void WebSocketStream::onWebSocketDisconnected()
{
	QWebSocket *ws = qobject_cast<QWebSocket*>(sender());

	LOG_CDEBUG("service") << "WebSocket disconnected:" << ws;

	m_service->webServer().lock().get()->webSocketHandler().webSocketRemove(this);
}


/**
 * @brief WebSocketStream::credential
 * @return
 */

const Credential &WebSocketStream::credential() const
{
	return m_credential;
}
