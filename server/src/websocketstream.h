#ifndef WEBSOCKETSTREAM_H
#define WEBSOCKETSTREAM_H

#include <QWebSocket>
#include "abstractengine.h"
#include "credential.h"
#include <QJsonObject>
#include <QPointer>
#include <QMutex>

class UdpServerPeer;

/**
 * @brief The WebSocketStream class
 */

class WebSocketStream : public QObject
{
	Q_OBJECT

public:
	WebSocketStream(EngineHandler *handler, QWebSocket *socket);
	~WebSocketStream();

	void close();

	void sendHello();
	void sendTextMessage(const QString &message) { if (m_socket) m_socket->sendTextMessage(message); }
	void sendBinaryMessage(const QByteArray &message) { if (m_socket) m_socket->sendBinaryMessage(message); }
	void sendUdpMessage(const std::vector<std::uint8_t> &message);

	const Credential &credential() const;

	bool hasEngine(const AbstractEngine::Type &type);
	bool hasEngine(const AbstractEngine::Type &type, const int &id);

	template <typename T>
	T* engineGet(const AbstractEngine::Type &type, const int &id);

	std::weak_ptr<AbstractEngine> engineGet(const AbstractEngine::Type &type, const int &id);

	QHostAddress peerAddress() const;

	UdpServerPeer *udpPeer() const;
	void setUdpPeer(UdpServerPeer *newUdpPeer);

private:
	const QVector<std::shared_ptr<AbstractEngine> > &engines() const;
	void engineAdd(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(AbstractEngine *engine);

	void onBinaryDataReceived(const QByteArray &data);
	void onTextReceived(const QString &text);
	void onWebSocketDisconnected();

	EngineHandler *m_handler = nullptr;
	ServerService *m_service = nullptr;
	std::unique_ptr<QWebSocket> m_socket;
	UdpServerPeer *m_udpPeer = nullptr;

	QVector<std::shared_ptr<AbstractEngine>> m_engines;

	friend class EngineHandler;
	friend class EngineHandlerPrivate;
};


/**
 * @brief WebSocketStream::engineGet
 * @param type
 * @param id
 * @return
 */

template<typename T>
T *WebSocketStream::engineGet(const AbstractEngine::Type &type, const int &id)
{
	for (const auto &e : m_engines) {
		if (e && e->type() == type && e->id() == id) {
			return qobject_cast<T*>(e.get());
		}
	}
	return nullptr;
}


#endif // WEBSOCKETSTREAM_H
