/*
 * ---- Call of Suli ----
 *
 * websocket.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * WebSocket
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "qnetworkaccessmanager.h"
#include "eventstream.h"
#include <QPointer>
#include <QNetworkReply>
#include <QAbstractSocket>
#include <QJSValue>
#include <QJsonObject>

#define WEBSOCKETREPLY_DELETE_AFTER_MSEC	20000

class Client;
class Server;
class WebSocketReply;

/**
 * @brief The WebSocket class
 */

class WebSocket : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Server *server READ server WRITE setServer NOTIFY serverChanged)
	Q_PROPERTY(State state READ state NOTIFY stateChanged)
	Q_PROPERTY(bool pending READ pending NOTIFY pendingChanged)

public:
	explicit WebSocket(Client *client);
	virtual ~WebSocket();

	///
	/// \brief The State enum
	///

	enum State {
		Disconnected = 0,
		Connecting,
		Connected
	};

	Q_ENUM(State);


	///
	/// \brief The API enum
	///

	enum API {
		ApiInvalid,
		ApiServer,
		ApiClient,
		ApiAuth,
		ApiGeneral,
		ApiUser,
		ApiTeacher,
		ApiPanel,
		ApiAdmin
	};

	Q_ENUM(API);

	const State &state() const;
	void setState(const State &newState);

	Server *server() const;
	void setServer(Server *newServer);

	QNetworkAccessManager *networkManager() const;

	bool pending() const;
	void setPending(bool newPending);

	QUrl getUrl(const WebSocket::API &api, const QString &path) const;

public slots:
	void connectToServer(Server *server = nullptr);
	void close();
	void abort();

	WebSocketReply *send(const WebSocket::API &api, const QString &path, const QJsonObject &data = {});
	WebSocketReply *send(const WebSocket::API &api, const QString &path, const QByteArray &content);
#ifndef Q_OS_WASM
	WebSocketReply *send(const WebSocket::API &api, const QString &path, const QByteArray &content, const QJsonObject &data);
#endif

	QNetworkReply *get(const QUrl &url);
	EventStream *getEventStream(const WebSocket::API &api, const QString &path, const QJsonObject &data = {});

private slots:
	void checkPending();

private:
	void abortAllReplies();

signals:
	void serverConnected();
	void serverDisconnected();
#ifndef QT_NO_SSL
	void socketSslErrors(const QList<QSslError> &errors);
#endif
	void socketError(QNetworkReply::NetworkError code);
	void responseError(const QString &error);
	void stateChanged();
	void serverChanged();

	void pendingChanged();

private:
	Client *const m_client;
	State m_state = Disconnected;
	Server *m_server = nullptr;
	int m_signalUnavailableNum = 0;

#ifndef QT_NO_SSL
	QSslCertificate m_rootCertificate;
#endif
	QNetworkAccessManager *const m_networkManager = nullptr;
	QVector<WebSocketReply *> m_replies;
	bool m_pending = false;

	friend class WebSocketReply;
};



/**
 * @brief The WebSocketReply class
 */

class WebSocketReply : public QObject
{
	Q_OBJECT

public:
	explicit WebSocketReply(QNetworkReply *reply, WebSocket *socket);
	virtual ~WebSocketReply();

	bool pending() const;

	// Done

	WebSocketReply *done(const std::function<void (const QJsonObject &)> &func)
	{
		m_funcs.append(func);
		return this;
	}

	template <typename T>
	WebSocketReply *done(T *inst, void (T::*func)(const QJsonObject &)) {
		m_funcs.append(std::bind(func, inst, std::placeholders::_1));
		return this;
	}

	WebSocketReply *done(const std::function<void (const QByteArray &)> &func)
	{
		m_funcsByteArray.append(func);
		return this;
	}

	template <typename T>
	WebSocketReply *done(T *inst, void (T::*func)(const QByteArray &)) {
		m_funcsByteArray.append(std::bind(func, inst, std::placeholders::_1));
		return this;
	}

	Q_INVOKABLE WebSocketReply *done(const QJSValue &v);


	// Fail

	WebSocketReply *fail(const std::function<void (const QString &)> &func)
	{
		m_funcsFail.append(func);
		return this;
	}

	template <typename T>
	WebSocketReply *fail(T *inst, void (T::*func)(const QString &)) {
		m_funcsFail.append(std::bind(func, inst, std::placeholders::_1));
		return this;
	}

	Q_INVOKABLE WebSocketReply *fail(const QJSValue &v);

	// Network error

	WebSocketReply *error(const std::function<void (const QNetworkReply::NetworkError &)> &func)
	{
		m_funcsError.append(func);
		return this;
	}

	template <typename T>
	WebSocketReply *error(T *inst, void (T::*func)(const QNetworkReply::NetworkError &)) {
		m_funcsError.append(std::bind(func, inst, std::placeholders::_1));
		return this;
	}

	Q_INVOKABLE WebSocketReply *error(const QJSValue &v);


public slots:
	void abort();
	void close();

private slots:
	void onReplyFinished();

signals:
	void finished();
	void failed(WebSocketReply *reply);
	void downloadProgress(qreal percent);
	void uploadProgress(qreal percent);

private:
	QPointer<QNetworkReply> m_reply = nullptr;
	WebSocket *const m_socket = nullptr;
	bool m_pending = true;
	QVector<std::function<void (const QJsonObject &)>> m_funcs;
	QVector<std::function<void (const QByteArray &)>> m_funcsByteArray;
	QJSValueList m_jsvalues;
	QVector<std::function<void (const QString &)>> m_funcsFail;
	QJSValueList m_jsvaluesFail;
	QVector<std::function<void (const QNetworkReply::NetworkError &)>> m_funcsError;
	QJSValueList m_jsvaluesError;
};


Q_DECLARE_METATYPE(WebSocket::API)


#endif // WEBSOCKET_H
