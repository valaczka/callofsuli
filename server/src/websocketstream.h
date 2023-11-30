#ifndef WEBSOCKETSTREAM_H
#define WEBSOCKETSTREAM_H

#include <QWebSocket>
#include "Logger.h"
#include "abstractengine.h"
#include "credential.h"
#include <QJsonObject>
#include <QPointer>
#include <QMutex>


/**
 * @brief The WebSocketStream class
 */

class WebSocketStream : public QObject
{
	Q_OBJECT

public:
	WebSocketStream(EngineHandler *handler, QWebSocket *socket);
	~WebSocketStream();

	/**
	 * @brief The StreamState enum
	 */
	enum StreamState {
		StateInvalid = 0,
		StateHelloSent,
		StateAuthenticated,
		StateError
	};


	bool observerAdd(const AbstractEngine::Type &type);
	void observerRemove(const AbstractEngine::Type &type);
	bool hasObserver(const AbstractEngine::Type &type) { return m_observers.contains(type); }

	void close();

	void sendHello();
	void sendJson(const char *operation, const QJsonValue &data = QJsonValue::Null);
	void sendTextMessage(const QString &message) { if (m_socket) m_socket->sendTextMessage(message); }
	void sendBinaryMessage(const QByteArray &message) { if (m_socket) m_socket->sendBinaryMessage(message); }

	const QVector<AbstractEngine::Type> &observers() const;
	void setObservers(const QVector<AbstractEngine::Type> &newObservers);

	StreamState state() const;
	const Credential &credential() const;

	bool hasEngine(const AbstractEngine::Type &type);
	bool hasEngine(const AbstractEngine::Type &type, const int &id);

	template <typename T>
	T* engineGet(const AbstractEngine::Type &type, const int &id);

	std::weak_ptr<AbstractEngine> engineGet(const AbstractEngine::Type &type, const int &id);

private:
	const QVector<std::shared_ptr<AbstractEngine> > &engines() const;
	void engineAdd(const std::shared_ptr<AbstractEngine> &engine);
	void engineRemove(AbstractEngine *engine);

	void onBinaryDataReceived(const QByteArray &data);
	void onTextReceived(const QString &text);
	void onWebSocketDisconnected();

	void onJsonReceived(const QJsonObject &data);
	void observerAdd(const QJsonValue &data);
	void observerRemove(const QJsonValue &data);
	void timeSync(QJsonObject data);

	EngineHandler *m_handler = nullptr;
	ServerService *m_service = nullptr;
	std::unique_ptr<QWebSocket> m_socket;
	QVector<AbstractEngine::Type> m_observers;
	StreamState m_state = StateInvalid;
	Credential m_credential;

	static const QHash<AbstractEngine::Type, Credential::Roles> m_observerRoles;
	static const QHash<QString, AbstractEngine::Type> m_observerMap;

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
		LOG_CTRACE("engine") << "--CHECK" << type << id << e.get() << e->type() << e->id();
		if (e && e->type() == type && e->id() == id) {
			return qobject_cast<T*>(e.get());
		}
	}
	return nullptr;
}


#endif // WEBSOCKETSTREAM_H
