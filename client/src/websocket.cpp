/*
 * ---- Call of Suli ----
 *
 * websocket.cpp
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

#include "websocket.h"
#include "client.h"
#include "qnetworkreply.h"
#include "Logger.h"
#include "qtimer.h"

WebSocket::WebSocket(Client *client)
	: QObject(client)
	, m_client(client)
	, m_networkManager(new QNetworkAccessManager(this))
{
	LOG_CTRACE("websocket") << "WebSocket created";

#ifndef QT_NO_SSL
	QFile certFile(QStringLiteral(":/root_CallOfSuli_CA.crt"));

	LOG_CTRACE("websocket") << "Cert file exists:" << certFile.exists();

	if (certFile.exists()) {
		certFile.open(QIODevice::ReadOnly);
		QSslCertificate cert(&certFile, QSsl::Pem);
		certFile.close();

		if (cert.isNull()) {
			LOG_CDEBUG("websocket") << "Invalid certificate";
		} else {
			QSslConfiguration config = QSslConfiguration::defaultConfiguration();
			config.addCaCertificate(cert);
			QSslConfiguration::setDefaultConfiguration(config);
			LOG_CTRACE("websocket") << "Root certificate added";
		}
	}
#endif


}


/**
 * @brief WebSocket::~WebSocket
 */

WebSocket::~WebSocket()
{
	delete m_networkManager;

	LOG_CTRACE("websocket") << "WebSocket destroyed";
}


/**
 * @brief WebSocket::state
 * @return
 */

const WebSocket::State &WebSocket::state() const
{
	return m_state;
}

void WebSocket::setState(const State &newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	emit stateChanged();

	if (m_state == Connected)
		emit serverConnected();
	else if (m_state == Disconnected)
		emit serverDisconnected();
}


/**
 * @brief WebSocket::server
 * @return
 */

Server *WebSocket::server() const
{
	return m_server;
}

void WebSocket::setServer(Server *newServer)
{
	if (m_server == newServer)
		return;
	m_server = newServer;
	emit serverChanged();
}


/**
 * @brief WebSocket::connectToServer
 * @param server
 */

void WebSocket::connectToServer(Server *server)
{
	if (!server)
		server = m_server;
	else
		setServer(server);

	if (!server || server->url().isEmpty()) {
		m_client->messageError(tr("Nincs megadva szerver"), tr("Belső hiba"));
		return;
	}

	LOG_CDEBUG("websocket") << "Connect to server:" << server->url();

	setState(Connecting);

	WebSocketReply *reply = send(ApiGeneral, "config");

	connect(reply, &WebSocketReply::success, reply, [this](WebSocketReply *r){
		if (m_server)
			LOG_CINFO("websocket") << "Connected to server:" << m_server->url();
		setState(Connected);

		LOG_CTRACE("websocket") << "INFO" << r->getContentJson();
	});

}




/**
 * @brief WebSocket::close
 */

void WebSocket::close()
{
	if (m_state != Disconnected) {
		LOG_CTRACE("websocket") << "Close connection";
		setState(Disconnected);
		abortAllReplies();
		setServer(nullptr);
		emit serverDisconnected();
	}
}



/**
 * @brief WebSocket::abort
 */

void WebSocket::abort()
{
	if (m_state != Disconnected) {
		LOG_CTRACE("websocket") << "Abort connection";
		m_client->stackPopToStartPage();
		setState(Disconnected);
		abortAllReplies();
		setServer(nullptr);
		emit serverDisconnected();
	}
}







/**
 * @brief WebSocket::abortAllReplies
 */

void WebSocket::abortAllReplies()
{
	LOG_CTRACE("websocket") << "Abort all replies";
	foreach (WebSocketReply *r, m_replies)
		r->abort();
}


/**
 * @brief WebSocket::pending
 * @return
 */

bool WebSocket::pending() const
{
	return m_pending;
}

void WebSocket::setPending(bool newPending)
{
	if (m_pending == newPending)
		return;
	m_pending = newPending;
	emit pendingChanged();
}


/**
 * @brief WebSocket::networkManager
 * @return
 */

QNetworkAccessManager *WebSocket::networkManager() const
{
	return m_networkManager;
}


/**
 * @brief WebSocket::send
 * @param method
 * @param api
 * @param path
 * @param data
 * @return
 */

WebSocketReply *WebSocket::send(const Method &method, const API &api, const QString &path, const QJsonObject &data)
{
	Q_ASSERT (m_networkManager);

	/// TODO: send (..., mime-type, bytearray)

	if (!m_server) {
		m_client->messageError(tr("Nincs szerver beállítva!"), tr("Hálózati hiba"));
		return nullptr;
	}

	QHash<API, const char*> apis;
	apis[ApiServer] = "server";
	apis[ApiGeneral] = "general";
	apis[ApiClient] = "client";
	apis[ApiAuth] = "auth";
	apis[ApiUser] = "user";
	apis[ApiTeacher] = "teacher";
	apis[ApiPanel] = "panel";
	apis[ApiAdmin] = "admin";

	if (!apis.contains(api)) {
		m_client->messageError(tr("Invalid api"));
		return nullptr;
	}

	QHash<Method, const char*> methods;
	methods[Get] = "GET";
	methods[Post] = "POST";
	methods[Put] = "PUT";
	methods[Delete] = "DELETE";

	if (!methods.contains(method)) {
		m_client->messageError(tr("Invalid method"));
		return nullptr;
	}

	QUrl url = m_server->url();
	url.setPath(QStringLiteral("/api/%1/%2").arg(apis.value(api), path));
	QNetworkRequest r(url);

	r.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	QNetworkReply *reply = m_networkManager->sendCustomRequest(r, methods.value(method), QJsonDocument(data).toJson());

	WebSocketReply *wr = new WebSocketReply(reply, this);
	connect(wr, &WebSocketReply::finished, this, &WebSocket::checkPending);
	connect(wr, &WebSocketReply::aborted, this, &WebSocket::checkPending);
	return wr;
}


/**
 * @brief WebSocket::checkPending
 */

void WebSocket::checkPending()
{
	bool pending = false;

	foreach (WebSocketReply *r, m_replies)
		if (r && r->pending()) {
			pending = true;
			break;
		}

	setPending(pending);
}



/**
 * @brief WebSocketReply::WebSocketReply
 * @param reply
 * @param socket
 */

WebSocketReply::WebSocketReply(QNetworkReply *reply, WebSocket *socket)
	: QObject(socket)
	, m_reply(reply)
	, m_socket(socket)
{
	Q_ASSERT(m_socket);
	Q_ASSERT(m_reply);

	LOG_CTRACE("websocket") << "WebSocketReply created" << this;

	m_socket->m_replies.append(this);

#ifndef QT_NO_SSL
	connect(m_reply, &QNetworkReply::sslErrors, m_socket, [this](const QList<QSslError> &e){
		m_pending = false;
		emit failed(this);
		emit finished(this);
		emit m_socket->socketSslErrors(e);
	});
#endif

	connect(m_reply, &QNetworkReply::errorOccurred, m_socket, [this](QNetworkReply::NetworkError e){
		m_pending = false;
		emit failed(this);
		emit finished(this);
		emit m_socket->socketError(e);
	});

	connect(m_reply, &QNetworkReply::finished, this, &WebSocketReply::onReplyFinished);
}


/**
 * @brief WebSocketReply::~WebSocketReply
 */

WebSocketReply::~WebSocketReply()
{
	if (m_socket)
		m_socket->m_replies.removeAll(this);

	if (m_reply)
		m_reply->deleteLater();

	LOG_CTRACE("websocket") << "WebSocketReply destroyed" << this;
}


/**
 * @brief WebSocketReply::networkReply
 * @return
 */

QNetworkReply *WebSocketReply::networkReply() const
{
	return m_reply;
}


/**
 * @brief WebSocketReply::abort
 */

void WebSocketReply::abort()
{
	m_pending = false;

	emit aborted(this);
	emit failed(this);

	LOG_CTRACE("websocket") << "Abort" << this;

	if (m_reply && !m_reply->isFinished() && m_reply->error() == QNetworkReply::NoError)
		m_reply->abort();

	done();
}


/**
 * @brief WebSocketReply::done
 */

void WebSocketReply::done()
{
	LOG_CTRACE("websocket") << "WebSocketReply done" << this;
	deleteLater();
}


/**
 * @brief WebSocketReply::onReplyFinished
 */

void WebSocketReply::onReplyFinished()
{
	Q_ASSERT(m_socket);

	m_pending = false;

	const QNetworkReply::NetworkError &error = m_reply->error();

	LOG_CTRACE("websocket") << "WebSocketReply finished" << error << this;

	if (m_reply->isReadable()) {
		m_content = m_reply->readAll();
		m_contentJson = QJsonDocument::fromJson(m_content).object();

		if (!m_contentJson.isEmpty() && m_contentJson.contains(QStringLiteral("error")))
			m_errorString = m_contentJson.value(QStringLiteral("error")).toString();
	}

	if (error == QNetworkReply::NoError)
		emit success(this);
	else
		emit failed(this);

	emit finished(this);

	QTimer::singleShot(WEBSOCKETREPLY_DELETE_AFTER_MSEC, this, &WebSocketReply::done);
}

bool WebSocketReply::pending() const
{
	return m_pending;
}



/**
 * @brief WebSocketReply::hasNetworkError
 * @return
 */

bool WebSocketReply::hasNetworkError() const
{
	return m_reply && m_reply->error();
}


/**
 * @brief WebSocketReply::networkError
 * @return
 */

QNetworkReply::NetworkError WebSocketReply::networkError() const
{
	return m_reply ? m_reply->error() : QNetworkReply::UnknownServerError;
}


/**
 * @brief WebSocketReply::getErrorString
 * @return
 */

QString WebSocketReply::getErrorString()
{
	done();
	return m_errorString;
}


/**
 * @brief WebSocketReply::getContent
 * @return
 */

QByteArray WebSocketReply::getContent()
{
	done();
	return m_content;
}


/**
 * @brief WebSocketReply::getContentJson
 * @return
 */

QJsonObject WebSocketReply::getContentJson()
{
	done();
	return m_contentJson;
}

