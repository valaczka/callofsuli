#include "websocketstream.h"
#include "serverservice.h"
#include "Logger.h"



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

	static const QJsonObject data{
		{ QStringLiteral("versionMajor"), ServerService::versionMajor() },
		{ QStringLiteral("versionMinor"), ServerService::versionMinor() },
	};

	sendTextMessage(QString::fromUtf8(QJsonDocument(data).toJson()));

}



/**
 * @brief WebSocketStream::sendBinaryMessage
 * @param message
 */

void WebSocketStream::sendUdpMessage(const std::vector<uint8_t> &message)
{
	sendBinaryMessage(QByteArray::fromRawData(reinterpret_cast<const char*>(message.data()), message.size()));
}





/**
 * @brief WebSocketStream::onTextReceived
 * @param text
 */

void WebSocketStream::onTextReceived(const QString &text)
{
	LOG_CDEBUG("service") << "WebSocketStream text received:" << this << text;

	sendHello();
}





/**
 * @brief WebSocketStream::onWebSocketDisconnected
 */

void WebSocketStream::onWebSocketDisconnected()
{
	QWebSocket *ws = qobject_cast<QWebSocket*>(sender());

	LOG_CDEBUG("service") << "WebSocket disconnected:" << ws << this;

	m_handler->websocketDisconnected(this);

	LOG_CTRACE("service") << "WebSocket disconnected finished:" << ws << this;
}



/**
 * @brief WebSocketStream::udpPeer
 * @return
 */

UdpServerPeer *WebSocketStream::udpPeer() const
{
	return m_udpPeer;
}

void WebSocketStream::setUdpPeer(UdpServerPeer *newUdpPeer)
{
	m_udpPeer = newUdpPeer;
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
 * @brief WebSocketStream::peerAddress
 * @return
 */

QHostAddress WebSocketStream::peerAddress() const
{
	if (m_socket)
		return m_socket->peerAddress();
	else
		return QHostAddress();
}






/**
 * @brief WebSocketStream::onBinaryDataReceived
 * @param data
 */

void WebSocketStream::onBinaryDataReceived(const QByteArray &data)
{
	LOG_CDEBUG("service") << "WebSocketStream binary data received:" << this << data.size();
}



