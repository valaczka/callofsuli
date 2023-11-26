#ifndef WEBSOCKETSTREAM_H
#define WEBSOCKETSTREAM_H

#include <QWebSocket>
#include "credential.h"
#include <QJsonObject>
#include <QPointer>
#include <QMutex>

class ServerService;

/**
 * @brief The WebSocketStream class
 */

class WebSocketStream : public QObject
{
	Q_OBJECT

public:
	WebSocketStream(ServerService *service, QWebSocket *socket);
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


	/**
	 * @brief The StreamType enum
	 */

	enum StreamType {
		StreamInvalid = 0,
		StreamPeers,
		StreamGroupScore,
		StreamMultiPlayer
	};


	/**
	 * @brief The StreamObserver class
	 */

	struct StreamObserver {
		StreamType type = StreamInvalid;
		QVariant data;

		friend inline bool operator== (const StreamObserver &o1, const StreamObserver &o2) {
			return o1.type == o2.type && o1.data == o2.data;
		}
	};


	void observerAdd(const StreamType &type, const QVariant &data = QVariant());
	void observerRemove(const StreamType &type, const QVariant &data = QVariant());
	void observerRemoveAll(const WebSocketStream::StreamType &type);

	bool hasObserver(const StreamType &type);
	bool hasObserver(const StreamType &type, const QVariant &data);

	QWebSocket *socket() const { return m_socket.get(); }

	void close();

	void sendHello();
	void sendJson(const char *operation, const QJsonValue &data = QJsonValue::Null);
	void sendTextMessage(const QString &message) { if (m_socket) m_socket->sendTextMessage(message); }

	const QVector<StreamObserver> &observers() const;
	void setObservers(const QVector<StreamObserver> &newObservers);

	StreamState state() const;
	const Credential &credential() const;

private:
	void onBinaryDataReceived(const QByteArray &data);
	void onTextReceived(const QString &text);
	void onJsonReceived(const QJsonObject &data);
	void observerAdd(const QJsonValue &data);
	void observerRemove(const QJsonValue &data);
	void onWebSocketDisconnected();
	void timeSync(QJsonObject data);

	ServerService *m_service = nullptr;
	std::unique_ptr<QWebSocket> m_socket;
	QVector<StreamObserver> m_observers;
	QMutex m_mutex;
	StreamState m_state = StateInvalid;
	Credential m_credential;
	static const QHash<StreamType, Credential::Roles> m_observerRoles;

	friend class WebSocketStreamHandler;
};


#endif // WEBSOCKETSTREAM_H
