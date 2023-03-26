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

#include "application.h"
#include "websocket.h"
#include "client.h"
#include "qjsengine.h"
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
			m_rootCertificate = cert;
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

#ifdef Q_OS_WASM
	if (!server) {
		m_client->messageError(tr("Nincs megadva szerver"), tr("Belső hiba"));
		return;
	}
#else
	if (!server || server->url().isEmpty()) {
		m_client->messageError(tr("Nincs megadva szerver"), tr("Belső hiba"));
		return;
	}
#endif

	LOG_CDEBUG("websocket") << "Connect to server:" << server->url();

	setState(Connecting);

	send(ApiGeneral, "config")->done([this](const QJsonObject &json){

		if (json.isEmpty() || json.value(QStringLiteral("server")).toString() != QStringLiteral("Call of Suli server")) {
			m_client->messageError(tr("A megadott címen nem található Call of Suli szerver"), tr("Sikertelen csatlakozás"));
			close();
			return;
		}

		int vMajor = json.value(QStringLiteral("versionMajor")).toInt();
		int vMinor = json.value(QStringLiteral("versionMinor")).toInt();

		if (vMajor < 1) {
			m_client->messageError(tr("A megadott címen nem található Call of Suli szerver"), tr("Sikertelen csatlakozás"));
			close();
			return;
		}

		if (Utils::versionCode(vMajor, vMinor) > Utils::versionCode()) {
			m_client->messageWarning(tr("A szerver verziója újabb az alkamazásnál, frissítsd az applikációt!"), tr("Szoftverfrissítés szükséges"));
		}

		if (m_server)
			LOG_CINFO("websocket") << "Connected to server:" << m_server->url();
		setState(Connected);
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
 * @brief WebSocket::getUrl
 * @param api
 * @param path
 * @param data
 * @return
 */

QUrl WebSocket::getUrl(const API &api, const QString &path) const
{
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
		return QUrl();
	}

	QUrl url = m_server->url();
	url.setPath(QStringLiteral("/api/%1/%2").arg(apis.value(api), path));
	return url;
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

WebSocketReply *WebSocket::send(const API &api, const QString &path, const QJsonObject &data)
{
	Q_ASSERT (m_networkManager);

	if (!m_server) {
		m_client->messageError(tr("Nincs szerver beállítva!"), tr("Hálózati hiba"));
		return nullptr;
	}

	QNetworkRequest r(getUrl(api, path));

#ifndef QT_NO_SSL
	if (!m_rootCertificate.isNull()) {
		QSslConfiguration config = r.sslConfiguration();
		config.addCaCertificate(m_rootCertificate);
		r.setSslConfiguration(config);
	}
#endif

	if (!m_server->token().isEmpty()) {
		r.setRawHeader(QByteArrayLiteral("Authorization"), QByteArrayLiteral("Bearer ")+m_server->token().toLocal8Bit());
	}

	r.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));

	QNetworkReply *reply = m_networkManager->post(r, QJsonDocument(data).toJson());

	LOG_CTRACE("websocket") << "SEND:" << qPrintable(Utils::enumToQString<API>(api)) << qPrintable(path) << this << data;

	WebSocketReply *wr = new WebSocketReply(reply, this);
	connect(wr, &WebSocketReply::finished, this, &WebSocket::checkPending);
	return wr;
}





/**
 * @brief WebSocket::get
 * @param url
 * @return
 */

QNetworkReply *WebSocket::get(const QUrl &url)
{
	Q_ASSERT (m_networkManager);

	if (!m_server) {
		m_client->messageError(tr("Nincs szerver beállítva!"), tr("Hálózati hiba"));
		return nullptr;
	}

	QNetworkRequest r(url);

#ifndef QT_NO_SSL
	if (!m_rootCertificate.isNull()) {
		QSslConfiguration config = r.sslConfiguration();
		config.addCaCertificate(m_rootCertificate);
		r.setSslConfiguration(config);
	}
#endif

	LOG_CTRACE("websocket") << "GET:" << url.toString();

	return m_networkManager->get(r);
}



/**
 * @brief WebSocket::getEventStream
 * @param api
 * @param path
 * @param data
 * @return
 */

EventStream *WebSocket::getEventStream(const API &api, const QString &path, const QJsonObject &data)
{
	Q_ASSERT (m_networkManager);

	if (!m_server) {
		m_client->messageError(tr("Nincs szerver beállítva!"), tr("Hálózati hiba"));
		return nullptr;
	}

	QNetworkRequest r(getUrl(api, path));

#ifndef QT_NO_SSL
	if (!m_rootCertificate.isNull()) {
		QSslConfiguration config = r.sslConfiguration();
		config.addCaCertificate(m_rootCertificate);
		r.setSslConfiguration(config);
	}
#endif

	if (!m_server->token().isEmpty()) {
		r.setRawHeader(QByteArrayLiteral("Authorization"), QByteArrayLiteral("Bearer ")+m_server->token().toLocal8Bit());
	}

	r.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("text/event-stream"));
	//r.setHeader(QNetworkRequest::UserAgentHeader, USER_AGENT);
	r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
	r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);

	r.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));

	EventStream *stream = new EventStream(this);
	stream->setRequest(r);
	stream->setRequestData(QJsonDocument(data).toJson());
	stream->connect();

	LOG_CTRACE("websocket") << "Get EventStream:" << qPrintable(Utils::enumToQString<API>(api)) << qPrintable(path) << this << data;

	return stream;
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
	m_socket->checkPending();

#ifndef QT_NO_SSL
	connect(m_reply, &QNetworkReply::sslErrors, m_socket, [this](const QList<QSslError> &e){
		LOG_CDEBUG("websocket") << "SSL error:" << e;

		QList<QSslError> list;

		foreach (const QSslError &err, e) {
#ifdef QT_DEBUG
			if (err.error() == QSslError::HostNameMismatch)
				list.append(err);
#endif
		}

		if (!list.isEmpty()) {
			m_reply->ignoreSslErrors(list);
			LOG_CWARNING("websocket") << "SSL errors ignored:" << list;
		}

		if (list.size() < e.size()) {
			m_pending = false;
			emit failed(this);
			emit finished();
			emit m_socket->socketSslErrors(e);
		}
	});
#endif

	connect(m_reply, &QNetworkReply::errorOccurred, m_socket, [this](QNetworkReply::NetworkError e){
		m_pending = false;
		emit m_socket->socketError(e);
		this->abort();
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
 * @brief WebSocketReply::abort
 */

void WebSocketReply::abort()
{
	m_pending = false;

	emit failed(this);

	LOG_CTRACE("websocket") << "Abort" << this;

	if (m_reply && !m_reply->isFinished() && m_reply->error() == QNetworkReply::NoError)
		m_reply->abort();

	emit finished();

	close();
}


/**
 * @brief WebSocketReply::done
 */

void WebSocketReply::close()
{
	LOG_CTRACE("websocket") << "WebSocketReply done" << this << parent();
	this->deleteLater();
}


/**
 * @brief WebSocketReply::done
 * @param func
 * @return
 */




/**
 * @brief WebSocketReply::onReplyFinished
 */

void WebSocketReply::onReplyFinished()
{
	Q_ASSERT(m_socket);

	m_pending = false;

	const QNetworkReply::NetworkError &error = m_reply->error();


	if (error != QNetworkReply::NoError) {
		LOG_CWARNING("websocket") << "WebSocketReply error" << error << this;
		emit finished();
		return;
	}

	//LOG_CTRACE("websocket") << "WebSocketReply finished successful" << this;

	QJsonObject contentJson;
	QString errorString;

	if (m_reply->isReadable()) {
		contentJson = QJsonDocument::fromJson(m_reply->readAll()).object();

		if (!contentJson.isEmpty() && contentJson.contains(QStringLiteral("error")))
			errorString = contentJson.value(QStringLiteral("error")).toString();
	}




	if (errorString.isEmpty()) {
		LOG_CTRACE("websocket") << "RECEIVED:" << contentJson;

		foreach (const std::function<void (const QJsonObject &)> &func, m_funcs)
			func(contentJson);

		QJSValueList list;

		QJSEngine *engine = qjsEngine(Application::instance()->client());

		if (engine)
			list = {engine->toScriptValue<QJsonObject>(contentJson)};
		else
			LOG_CERROR("websocket") << "Invalid JSEngine";

		foreach (QJSValue v, m_jsvalues)
			v.call(list);
	} else {
		LOG_CWARNING("websocket") << "Response error:" << errorString << this;

		emit m_socket->responseError(errorString);

		foreach (const std::function<void (const QString &)> &func, m_funcsFail)
			func(errorString);

		foreach (QJSValue v, m_jsvaluesFail)
			v.call({errorString});
	}

	emit finished();

	QTimer::singleShot(WEBSOCKETREPLY_DELETE_AFTER_MSEC, this, &WebSocketReply::close);
}


/**
 * @brief WebSocketReply::pending
 * @return
 */

bool WebSocketReply::pending() const
{
	return m_pending;
}


/**
 * @brief WebSocketReply::done
 * @param v
 * @return
 */

WebSocketReply *WebSocketReply::done(const QJSValue &v)
{
	if (!v.isCallable())
		LOG_CERROR("websocket") << "QJSValue isn't callable";
	else
		m_jsvalues.append(v);
	return this;
}


/**
 * @brief WebSocketReply::fail
 * @param v
 * @return
 */

WebSocketReply *WebSocketReply::fail(const QJSValue &v)
{
	if (!v.isCallable())
		LOG_CERROR("websocket") << "QJSValue isn't callable";
	else
		m_jsvaluesFail.append(v);
	return this;
}



