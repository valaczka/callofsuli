#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "qjsonobject.h"
#include "qjsonvalue.h"
#include "qtimer.h"
#include <QObject>
#include <QWebSocket>

class HttpConnection;

#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_HttpConnection
#define OPAQUE_PTR_HttpConnection
Q_DECLARE_OPAQUE_POINTER(HttpConnection*)
#endif

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
	/*Q_INVOKABLE void observerAddInt(const QString &type, const int &num) { observerAddValue(type, num); }
	Q_INVOKABLE void observerAddString(const QString &type, const QString &string) { observerAddValue(type, string); }
	Q_INVOKABLE void observerAddObject(const QString &type, const QJsonObject &object) { observerAddValue(type, object); }
	Q_INVOKABLE void observerAddArray(const QString &type, const QJsonArray &array) { observerAddValue(type, array); }*/

	Q_INVOKABLE void observerRemove(const QString &type) { observerRemoveValue(type, QJsonValue::Null); }
	/*Q_INVOKABLE void observerRemoveInt(const QString &type, const int &num) { observerRemoveValue(type, num); }
	Q_INVOKABLE void observerRemoveString(const QString &type, const QString &string) { observerRemoveValue(type, string); }
	Q_INVOKABLE void observerRemoveObject(const QString &type, const QJsonObject &object) { observerRemoveValue(type, object); }
	Q_INVOKABLE void observerRemoveArray(const QString &type, const QJsonArray &array) { observerRemoveValue(type, array); }*/

	Q_INVOKABLE void observerAddValue(const QString &type, const QJsonValue &value);
	Q_INVOKABLE void observerRemoveValue(const QString &type, const QJsonValue &value);

	Q_INVOKABLE void send(const QString &op, const QJsonValue &data = {});

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
