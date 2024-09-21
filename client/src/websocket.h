#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "qjsonobject.h"
#include "qjsonvalue.h"
#include "qtimer.h"
#include <QObject>
#include <QWebSocket>

class HttpConnection;

#ifndef OPAQUE_PTR_HttpConnection
#define OPAQUE_PTR_HttpConnection
Q_DECLARE_OPAQUE_POINTER(HttpConnection*)
#endif


/**
 * @brief The WebSocket class
 */

class WebSocket : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool active READ active NOTIFY activeChanged)

public:
	explicit WebSocket(HttpConnection *connection);
	virtual ~WebSocket();

	bool active() const;

	Q_INVOKABLE void connect();
	Q_INVOKABLE void close();

	Q_INVOKABLE void observerAdd(const QString &type) { observerAddValue(type, QJsonValue::Null); }
	Q_INVOKABLE void observerAddValue(const QString &type, const QJsonValue &value);
	Q_INVOKABLE void observerRemove(const QString &type) { observerRemoveValue(type, QJsonValue::Null); }
	Q_INVOKABLE void observerRemoveValue(const QString &type, const QJsonValue &value);

	Q_INVOKABLE void send(const QString &op, const QJsonValue &data = {});
	Q_INVOKABLE void send(const QByteArray &data);

	QWebSocket* socket() const;

signals:
	void activeChanged();
	void connectionError();
	void connectionFailed();
	void messageReceived(QString operation, QJsonValue data);

private:
	void onConnected();
	void onDisconnected();
	void onError(const QAbstractSocket::SocketError &error);
	void onTextReceived(const QString &text);
	void onJsonReceived(const QJsonObject &json);
	void send(const QJsonObject &json);
	void reconnect();

	enum State {
		WebSocketReset = 0,
		WebSocketConnected,
		WebSocketHelloReceived,
		WebSocketAuthenticated,
		WebSocketListening
	};


	struct Observer {
		QString type;
		QJsonValue data;

		friend bool operator==(const Observer &ob1, const Observer &ob2) {
			return ob1.type == ob2.type && ob1.data == ob2.data;
		}
	};

	HttpConnection *m_connection = nullptr;
	std::unique_ptr<QWebSocket> m_socket;
	State m_state = WebSocketReset;
	QVector<Observer> m_observers;
	QTimer m_timerConnect;
	int m_tries = 0;
	bool m_forceClose = false;
};

#endif // WEBSOCKET_H
