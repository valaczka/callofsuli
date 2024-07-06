/*
 * ---- Call of Suli ----
 *
 * httpconnection.h
 *
 * Created on: 2023. 01. 02.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * HttpConnection
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

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "qnetworkaccessmanager.h"
#include <QPointer>
#include <QNetworkReply>
#include <QAbstractSocket>
#include <QJSValue>
#include <QJsonObject>
#include "websocket.h"

#define HTTPREPLY_DELETE_AFTER_MSEC	20000

class Client;
class Server;
class HttpReply;


#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_Server
#define OPAQUE_PTR_Server
  Q_DECLARE_OPAQUE_POINTER(Server*)
#endif

#endif

/**
 * @brief The HttpConnection class
 */

class HttpConnection : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Server *server READ server WRITE setServer NOTIFY serverChanged)
	Q_PROPERTY(State state READ state NOTIFY stateChanged)
	Q_PROPERTY(bool pending READ pending NOTIFY pendingChanged)
	Q_PROPERTY(WebSocket* webSocket READ webSocket CONSTANT)

public:
	explicit HttpConnection(Client *client);
	virtual ~HttpConnection();

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

	QUrl getUrl(const HttpConnection::API &api, const QString &path) const;

	WebSocket* webSocket() const;

public slots:
	void connectToServer(Server *server = nullptr);
	void close();
	void abort();

	HttpReply *send(const HttpConnection::API &api, const QString &path, const QJsonObject &data = {});
	HttpReply *send(const HttpConnection::API &api, const QString &path, const QByteArray &content);
	HttpReply *get(const QString &path);

	void checkPending();
	void acceptPendingSslErrors();

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
	void pendingSslErrors(const QStringList &list);

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
	QList<QSslError> m_pendingSslErrors;
#endif
	std::unique_ptr<QNetworkAccessManager> m_networkManager;
	QVector<HttpReply *> m_replies;
	bool m_pending = false;

	std::unique_ptr<WebSocket> m_webSocket;

	friend class HttpReply;
	friend class WebSocket;
};



/**
 * @brief The HttpConnectionReply class
 */

class HttpReply : public QObject
{
	Q_OBJECT

public:
	explicit HttpReply(QNetworkReply *reply, HttpConnection *socket);
	explicit HttpReply(const QNetworkReply::NetworkError &error, QObject *parent = nullptr);
	virtual ~HttpReply();

	bool pending() const;

	// Done

	HttpReply *done(QObject *inst, const std::function<void (const QJsonObject &)> &func)
	{
		m_funcs.append(qMakePair(inst, func));
		return this;
	}

	template <typename T>
	HttpReply *done(T *inst, void (T::*func)(const QJsonObject &)) {
		m_funcs.append(qMakePair(inst, std::bind(func, inst, std::placeholders::_1)));
		return this;
	}

	HttpReply *done(QObject *inst, const std::function<void (const QByteArray &)> &func)
	{
		m_funcsByteArray.append(qMakePair(inst, func));
		return this;
	}

	template <typename T>
	HttpReply *done(T *inst, void (T::*func)(const QByteArray &)) {
		m_funcsByteArray.append(qMakePair(inst, std::bind(func, inst, std::placeholders::_1)));
		return this;
	}

	Q_INVOKABLE HttpReply *done(QObject *inst, const QJSValue &v);


	// Fail

	HttpReply *fail(QObject *inst, const std::function<void (const QString &)> &func)
	{
		m_funcsFail.append(qMakePair(inst, func));
		return this;
	}

	template <typename T>
	HttpReply *fail(T *inst, void (T::*func)(const QString &)) {
		m_funcsFail.append(qMakePair(inst, std::bind(func, inst, std::placeholders::_1)));
		return this;
	}

	Q_INVOKABLE HttpReply *fail(QObject *inst, const QJSValue &v);

	// Network error

	HttpReply *error(QObject *inst, const std::function<void (const QNetworkReply::NetworkError &)> &func)
	{
		m_funcsError.append(qMakePair(inst, func));
		return this;
	}

	template <typename T>
	HttpReply *error(T *inst, void (T::*func)(const QNetworkReply::NetworkError &)) {
		m_funcsError.append(qMakePair(inst, std::bind(func, inst, std::placeholders::_1)));
		return this;
	}

	Q_INVOKABLE HttpReply *error(QObject *inst, const QJSValue &v);

	const std::function<void (const QList<QSslError> &)> &sslErrorCallback() const;
	void setSslErrorCallback(const std::function<void (const QList<QSslError> &)> &newSslErrorCallback);

public slots:
	void abort();
	void close();

signals:
	void finished();
	void failed(HttpReply *reply);
	void downloadProgress(qreal percent);
	void uploadProgress(qreal percent);

private:
	void onReplyFinished();
	void onErrorPresent(const QNetworkReply::NetworkError &error);


	QPointer<QNetworkReply> m_reply = nullptr;
	QPointer<HttpConnection> m_socket = nullptr;
	bool m_pending = true;
	QVector<QPair<QPointer<QObject>, std::function<void (const QJsonObject &)>>> m_funcs;
	QVector<QPair<QPointer<QObject>, std::function<void (const QByteArray &)>>> m_funcsByteArray;
	QVector<QPair<QPointer<QObject>, QJSValue>> m_jsvalues;
	QVector<QPair<QPointer<QObject>, std::function<void (const QString &)>>> m_funcsFail;
	QVector<QPair<QPointer<QObject>, QJSValue>> m_jsvaluesFail;
	QVector<QPair<QPointer<QObject>, std::function<void (const QNetworkReply::NetworkError &)>>> m_funcsError;
	QVector<QPair<QPointer<QObject>, QJSValue>> m_jsvaluesError;
	std::function<void (const QList<QSslError> &)> m_sslErrorCallback;
};


Q_DECLARE_METATYPE(HttpConnection::API)


#endif // HTTPCONNECTION_H
