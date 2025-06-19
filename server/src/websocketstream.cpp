#include "websocketstream.h"
#include "serverservice.h"
#include "Logger.h"


/// Static maps

const QHash<AbstractEngine::Type, Credential::Roles> WebSocketStream::m_observerRoles = {
	/*{ AbstractEngine::EnginePeer, Credential::Teacher|Credential::Admin },
	{ AbstractEngine::EngineExam, Credential::Teacher|Credential::Student|Credential::Panel },*/
};


const QHash<QString, AbstractEngine::Type> WebSocketStream::m_observerMap = {
	/*{ QStringLiteral("peers"), AbstractEngine::EnginePeer },
	{ QStringLiteral("exam"), AbstractEngine::EngineExam },*/
};


/**
 * @brief WebSocketStream::WebSocketStream
 * @param socket
 */

WebSocketStream::WebSocketStream(EngineHandler *handler, QWebSocket *socket)
	: QObject()
	, m_handler(handler)
	, m_service(handler ? handler->m_service : nullptr)
	, m_socket(std::move(socket))
{
	Q_ASSERT(m_service);

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

bool WebSocketStream::observerAdd(const AbstractEngine::Type &type)
{
	if (auto roles = m_observerRoles.value(type, Credential::None); roles != Credential::None && !(m_credential.roles() & roles)) {
		sendJson("warning", QStringLiteral("permission denied"));
	}

	LOG_CTRACE("service") << "Observer added" << m_credential.username() << type;

	if (!m_observers.contains(type)) {
		m_observers.append(type);
		return true;
	}

	return false;
}


/**
 * @brief WebSocketStream::observerRemove
 * @param type
 * @param data
 */

void WebSocketStream::observerRemove(const AbstractEngine::Type &type)
{
	m_observers.removeAll(type);

	LOG_CTRACE("service") << "Observer removed" << m_credential.username() << type;
}




/**
 * @brief WebSocketStream::close
 */

void WebSocketStream::close()
{
	if (m_socket) {
		m_socket->close();
		m_socket.reset();
	}
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

const QVector<AbstractEngine::Type> &WebSocketStream::observers() const
{
	return m_observers;
}


/**
 * @brief WebSocketStream::setObservers
 * @param newObservers
 */

void WebSocketStream::setObservers(const QVector<AbstractEngine::Type> &newObservers)
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
		const QByteArray &token = data.value(QStringLiteral("token")).toString().toUtf8();

		if (!Credential::verify(token, m_service->settings()->jwtSecret(), m_service->config().get("tokenFirstIat").toInteger(0))) {
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
	else if (operation == QStringLiteral("timeSync"))
		timeSync(d.toObject());
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

	QList<AbstractEngine::Type> added;

	for (const QJsonObject &obj : std::as_const(list)) {
		const QString &type = obj.value(QStringLiteral("type")).toString();

		const AbstractEngine::Type &t = m_observerMap.value(type, AbstractEngine::EngineInvalid);

		if (t == AbstractEngine::EngineInvalid)
			LOG_CWARNING("service") << "Invalid observer:" << type;
		else {
			if (observerAdd(t))
				added.append(t);
		}
	}

	for (const auto &t : added) {
		m_handler->websocketObserverAdded(this, t);
	}
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

	QList<AbstractEngine::Type> removed;

	for (const QJsonObject &obj : std::as_const(list)) {
		const QString &type = obj.value(QStringLiteral("type")).toString();

		const AbstractEngine::Type &t = m_observerMap.value(type, AbstractEngine::EngineInvalid);

		if (t == AbstractEngine::EngineInvalid)
			LOG_CWARNING("service") << "Invalid observer:" << type;
		else {
			observerRemove(t);
			removed.append(t);
		}
	}

	for (const auto &t : removed) {
		m_handler->websocketObserverRemoved(this, t);
	}
}




/**
 * @brief WebSocketStream::onWebSocketDisconnected
 */

void WebSocketStream::onWebSocketDisconnected()
{
	QWebSocket *ws = qobject_cast<QWebSocket*>(sender());

	LOG_CDEBUG("service") << "WebSocket disconnected:" << ws << m_credential.username() << this;

	m_handler->websocketDisconnected(this);

	LOG_CTRACE("service") << "WebSocket disconnected finished:" << ws << m_credential.username() << this;
}



/**
 * @brief WebSocketStream::timeSync
 * @param data
 */

void WebSocketStream::timeSync(QJsonObject data)
{
	data[QStringLiteral("serverTime")] = QDateTime::currentMSecsSinceEpoch();
	sendJson("timeSync", data);
}




/**
 * @brief WebSocketStream::engines
 * @return
 */

const QVector<std::shared_ptr<AbstractEngine> > &WebSocketStream::engines() const
{
	return m_engines;
}


/**
 * @brief WebSocketStream::engineAdd
 * @param engine
 */

void WebSocketStream::engineAdd(const std::shared_ptr<AbstractEngine> &engine)
{
	LOG_CTRACE("service") << "WebSocket add engine" << this << engine.get() << engine->type() << engine->id();

	std::shared_ptr<AbstractEngine> ptr = engine;
	m_engines.append(std::move(ptr));
}


/**
 * @brief WebSocketStream::engineRemove
 * @param engine
 */

void WebSocketStream::engineRemove(AbstractEngine *engine)
{
	LOG_CTRACE("service") << "WebSocket remove engine" << this << engine << engine->type();

	for (auto it = m_engines.begin(); it != m_engines.end(); ) {
		if (it->get() == engine)
			it = m_engines.erase(it);
		else
			++it;
	}
}


/**
 * @brief WebSocketStream::hasEngine
 * @param type
 * @return
 */

bool WebSocketStream::hasEngine(const AbstractEngine::Type &type)
{
	for (const auto &e : m_engines) {
		if (e && e->type() == type)
			return true;
	}

	return false;
}


/**
 * @brief WebSocketStream::hasEngine
 * @param type
 * @param id
 * @return
 */

bool WebSocketStream::hasEngine(const AbstractEngine::Type &type, const int &id)
{
	for (const auto &e : m_engines) {
		if (e && e->type() == type && e->id() == id)
			return true;
	}

	return false;
}



/**
 * @brief WebSocketStream::engineGet
 * @param type
 * @param id
 * @return
 */

std::weak_ptr<AbstractEngine> WebSocketStream::engineGet(const AbstractEngine::Type &type, const int &id)
{
	for (const auto &e : m_engines) {
		if (e && e->type() == type && e->id() == id)
			return e;
	}

	return std::weak_ptr<AbstractEngine>();
}




/**
 * @brief WebSocketStream::credential
 * @return
 */

const Credential &WebSocketStream::credential() const
{
	return m_credential;
}



/**
 * @brief WebSocketStream::onBinaryDataReceived
 * @param data
 */

void WebSocketStream::onBinaryDataReceived(const QByteArray &data)
{
	LOG_CTRACE("service") << "WebSocketStream binary data received:" << this << data.size();

	if (m_state == StateError) {
		sendJson("error", QStringLiteral("invalid stream"));
		return;
	}
}



