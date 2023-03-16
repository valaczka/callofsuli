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
#include "server.h"
#include <QPointer>
#include <QNetworkReply>
#include <QAbstractSocket>

#define WEBSOCKETREPLY_DELETE_AFTER_MSEC	20000

class Client;
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

public slots:
	void connectToServer(Server *server = nullptr);
	void close();
	void abort();

	WebSocketReply *send(const WebSocket::API &api, const QString &path, const QJsonObject &data = {});

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
	void stateChanged();
	void serverChanged();

	void pendingChanged();

private:
	Client *const m_client;
	State m_state = Disconnected;
	Server *m_server = nullptr;
	int m_signalUnavailableNum = 0;

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


public slots:
	void abort();
	void close();

private slots:
	void onReplyFinished();

signals:
	void finished();
	void failed(WebSocketReply *reply);

private:
	QPointer<QNetworkReply> m_reply = nullptr;
	WebSocket *const m_socket = nullptr;
	bool m_pending = true;
	QVector<std::function<void (const QJsonObject &)>> m_funcs;
	QJSValueList m_jsvalues;
	QVector<std::function<void (const QString &)>> m_funcsFail;
	QJSValueList m_jsvaluesFail;
};


Q_DECLARE_METATYPE(WebSocket::API)


#endif // WEBSOCKET_H
