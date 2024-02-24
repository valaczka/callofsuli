/*
 * ---- Call of Suli ----
 *
 * httpconnection.cpp
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

#include "application.h"
#include "httpconnection.h"
#include "client.h"
#include "qjsengine.h"
#include "qnetworkreply.h"
#include "Logger.h"
#include "qtimer.h"
#include "utils_.h"
#include "server.h"

HttpConnection::HttpConnection(Client *client)
	: QObject()
	, m_client(client)
	, m_networkManager(new QNetworkAccessManager())
	, m_webSocket(new WebSocket(this))
{
	LOG_CTRACE("http") << "HttpConnection created";


#ifndef QT_NO_SSL
	if (!QSslSocket::supportsSsl())
		LOG_CERROR("http") << "Platform doesn't support SSL";
	/*
	QFile certFile(QStringLiteral(":/root_CallOfSuli_CA.crt"));

	LOG_CTRACE("http") << "Cert file exists:" << certFile.exists();

	if (certFile.exists()) {
		certFile.open(QIODevice::ReadOnly);
		QSslCertificate cert(&certFile, QSsl::Pem);
		certFile.close();

		if (cert.isNull()) {
			LOG_CDEBUG("http") << "Invalid certificate";
		} else {
			m_rootCertificate = cert;
			LOG_CTRACE("http") << "Root certificate added";
		}
	} */
#else
	LOG_CERROR("http") << "Qt built without SSL support";
#endif


}


/**
 * @brief HttpConnection::~HttpConnection
 */

HttpConnection::~HttpConnection()
{
	LOG_CTRACE("http") << "HttpConnection destroyed";
}


/**
 * @brief HttpConnection::state
 * @return
 */

const HttpConnection::State &HttpConnection::state() const
{
	return m_state;
}

void HttpConnection::setState(const State &newState)
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
 * @brief HttpConnection::server
 * @return
 */

Server *HttpConnection::server() const
{
	return m_server;
}

void HttpConnection::setServer(Server *newServer)
{
	if (m_server == newServer)
		return;
	m_server = newServer;
	emit serverChanged();

	if (!m_server) {
		m_webSocket->close();
	}
}


/**
 * @brief HttpConnection::connectToServer
 * @param server
 */

void HttpConnection::connectToServer(Server *server)
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

	LOG_CDEBUG("http") << "Connect to server:" << server->url();

	setState(Connecting);

	HttpReply *wr = send(ApiGeneral, "config")->done(this, [this](const QJsonObject &json){

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
			LOG_CINFO("http") << "Connected to server:" << m_server->url();
		setState(Connected);
	});

#ifndef QT_NO_SSL
	wr->setSslErrorCallback([this](const QList<QSslError> &list){
		LOG_CDEBUG("http") << "SslErrorCallback:" << list;
		m_pendingSslErrors = list;

		QStringList errorList;
		QSslCertificate cert;
		foreach (const QSslError &e, list) {
			if (cert.isNull())
				cert = e.certificate();

			errorList << e.errorString();
		}

		if (cert.isNull())
			errorList.prepend(tr("Hiányzó tanúsítvány!"));
		else {
			if (!cert.subjectInfo(QSslCertificate::LocalityName).isEmpty() || !cert.subjectInfo(QSslCertificate::CountryName).isEmpty())
				errorList.prepend(tr("Helység: %1 (%2)").arg(cert.subjectInfo(QSslCertificate::LocalityName).join(QStringLiteral(", ")),
															 cert.subjectInfo(QSslCertificate::CountryName).join(QStringLiteral(", "))));
			if (!cert.subjectInfo(QSslCertificate::Organization).isEmpty())
				errorList.prepend(tr("Szervezet: %1").arg(cert.subjectInfo(QSslCertificate::Organization).join(QStringLiteral(", "))));
			if (!cert.subjectInfo(QSslCertificate::CommonName).isEmpty())
				errorList.prepend(tr("Név: %1").arg(cert.subjectInfo(QSslCertificate::CommonName).join(QStringLiteral(", "))));
		}

		emit pendingSslErrors(errorList);
	});
#endif
}




/**
 * @brief HttpConnection::close
 */

void HttpConnection::close()
{
	if (m_state != Disconnected) {
		LOG_CTRACE("http") << "Close connection";
		setState(Disconnected);
		abortAllReplies();
		if (m_server) {
			m_server->dynamicContentReset();
			m_server->setDynamicContentReady(false);
		}
		setServer(nullptr);
		emit serverDisconnected();
	}
}



/**
 * @brief HttpConnection::abort
 */

void HttpConnection::abort()
{
	if (m_state != Disconnected) {
		LOG_CTRACE("http") << "Abort connection";
		m_client->stackPopToStartPage();
		setState(Disconnected);
		abortAllReplies();
		if (m_server) {
			m_server->dynamicContentReset();
			m_server->setDynamicContentReady(false);
		}
		setServer(nullptr);
		emit serverDisconnected();
	}
}







/**
 * @brief HttpConnection::abortAllReplies
 */

void HttpConnection::abortAllReplies()
{
	LOG_CTRACE("http") << "Abort all replies";
	foreach (HttpReply *r, m_replies)
		r->abort();
}


/**
 * @brief HttpConnection::webSocket
 * @return
 */

WebSocket* HttpConnection::webSocket() const
{
	return m_webSocket.get();
}




/**
 * @brief HttpConnection::pending
 * @return
 */

bool HttpConnection::pending() const
{
	return m_pending;
}

void HttpConnection::setPending(bool newPending)
{
	if (m_pending == newPending)
		return;
	m_pending = newPending;
	emit pendingChanged();
}



/**
 * @brief HttpConnection::getUrl
 * @param api
 * @param path
 * @param data
 * @return
 */

QUrl HttpConnection::getUrl(const API &api, const QString &path) const
{
	static const QHash<API, const char*> apis = {
		{ ApiServer, "server" },
		{ ApiGeneral , "general" },
		{ ApiClient , "client" },
		{ ApiAuth , "auth" },
		{ ApiUser , "user" },
		{ ApiTeacher , "teacher" },
		{ ApiPanel , "panel" },
		{ ApiAdmin , "admin" },
	};

	if (!apis.contains(api)) {
		m_client->messageError(tr("Invalid api"));
		return QUrl();
	}

	QUrl url = m_server->url();
	url.setPath(QStringLiteral("/api/%1/%2").arg(apis.value(api), path));
	return url;
}


/**
 * @brief HttpConnection::networkManager
 * @return
 */

QNetworkAccessManager *HttpConnection::networkManager() const
{
	return m_networkManager.get();
}


/**
 * @brief HttpConnection::send
 * @param method
 * @param api
 * @param path
 * @param data
 * @return
 */

HttpReply *HttpConnection::send(const API &api, const QString &path, const QJsonObject &data)
{
	if (!m_server) {
		m_client->messageError(tr("Nincs szerver beállítva!"), tr("Hálózati hiba"));
		return new HttpReply(QNetworkReply::InternalServerError);
	}

	const QByteArray &content = QJsonDocument(data).toJson();

	if (m_server->maxUploadSize() > 0 && content.size() >= m_server->maxUploadSize()) {
		m_client->messageError(tr("Az üzenet túl nagy méretű"), tr("Hálózati hiba"));
		return new HttpReply(QNetworkReply::InternalServerError);
	}

	QNetworkRequest r(getUrl(api, path));

#ifndef QT_NO_SSL
	if (!m_rootCertificate.isNull()) {
		QSslConfiguration config = QSslConfiguration::defaultConfiguration();
		config.addCaCertificate(m_rootCertificate);
		r.setSslConfiguration(config);
	}
#endif

	if (!m_server->token().isEmpty()) {
		r.setRawHeader(QByteArrayLiteral("Authorization"), QByteArrayLiteral("Bearer ")+m_server->token().toLocal8Bit());
	}

	r.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
	r.setHeader(QNetworkRequest::UserAgentHeader, m_client->application()->userAgent());

	QNetworkReply *reply = m_networkManager->post(r, content);

	LOG_CTRACE("http") << "SEND:" << qPrintable(Utils::enumToQString<API>(api)) << qPrintable(path) << this << data;

	HttpReply *wr = new HttpReply(reply, this);
	connect(wr, &HttpReply::finished, this, &HttpConnection::checkPending);
	return wr;
}



/**
 * @brief HttpConnection::send
 * @param api
 * @param path
 * @param content
 * @return
 */

HttpReply *HttpConnection::send(const API &api, const QString &path, const QByteArray &content)
{
	if (!m_server) {
		m_client->messageError(tr("Nincs szerver beállítva!"), tr("Hálózati hiba"));
		return new HttpReply(QNetworkReply::InternalServerError);
	}

	if (m_server->maxUploadSize() > 0 && content.size() >= m_server->maxUploadSize()) {
		m_client->messageError(tr("Az üzenet túl nagy méretű"), tr("Hálózati hiba"));
		return new HttpReply(QNetworkReply::InternalServerError);
	}

	QNetworkRequest r(getUrl(api, path));

#ifndef QT_NO_SSL
	if (!m_rootCertificate.isNull()) {
		QSslConfiguration config = QSslConfiguration::defaultConfiguration();
		config.addCaCertificate(m_rootCertificate);
		r.setSslConfiguration(config);
	}
#endif

	if (!m_server->token().isEmpty()) {
		r.setRawHeader(QByteArrayLiteral("Authorization"), QByteArrayLiteral("Bearer ")+m_server->token().toLocal8Bit());
	}

	r.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/octet-stream"));
	r.setHeader(QNetworkRequest::UserAgentHeader, m_client->application()->userAgent());

	QNetworkReply *reply = m_networkManager->post(r, content);


	LOG_CDEBUG("http") << "Send content:" << qPrintable(Utils::enumToQString<API>(api)) << qPrintable(path) << content.size();

	HttpReply *wr = new HttpReply(reply, this);
	connect(wr, &HttpReply::finished, this, &HttpConnection::checkPending);
	return wr;
}







/**
 * @brief HttpConnection::checkPending
 */

void HttpConnection::checkPending()
{
	bool pending = false;

	foreach (HttpReply *r, m_replies)
		if (r && r->pending()) {
			pending = true;
			break;
		}

	setPending(pending);
}



/**
 * @brief HttpConnection::acceptPendingSslErrors
 */

void HttpConnection::acceptPendingSslErrors()
{
#ifndef QT_NO_SSL
	if (m_server && !m_pendingSslErrors.isEmpty()) {
		QSslCertificate cert;
		QList<QSslError::SslError> ignoredErrors = m_server->ignoredSslErrors();
		foreach (const QSslError &error, m_pendingSslErrors) {
			if (cert.isNull())
				cert = error.certificate();

			if (!ignoredErrors.contains(error.error()))
				ignoredErrors << error.error();
		}
		m_server->setCertificate(cert.toPem());
		m_server->setIgnoredSslErrors(ignoredErrors);

		m_pendingSslErrors.clear();

		connectToServer();
	}
#endif
}



/**
 * @brief HttpConnectionReply::HttpConnectionReply
 * @param reply
 * @param socket
 */

HttpReply::HttpReply(QNetworkReply *reply, HttpConnection *socket)
	: QObject(socket)
	, m_reply(reply)
	, m_socket(socket)
{
	Q_ASSERT(m_socket);
	Q_ASSERT(m_reply);

	LOG_CTRACE("http") << "HttpConnectionReply created" << this;

	m_socket->m_replies.append(this);
	m_socket->checkPending();

#ifndef QT_NO_SSL
	connect(m_reply, &QNetworkReply::sslErrors, this, [this](const QList<QSslError> &e){
		LOG_CDEBUG("http") << "SSL error:" << e;

		if (!m_socket) return;

		Server *server = m_socket->server();

		if (server && !server->certificate().isEmpty()) {
			QSslCertificate cert(server->certificate(), QSsl::Pem);

			if (cert.isNull()) {
				LOG_CERROR("http") << "Invalid server certificate stored";
			} else {
				QList<QSslError> ignoredErrors;

				for (auto it=e.constBegin(); it != e.constEnd(); ++it) {
					if (it->certificate() != cert)
						LOG_CWARNING("http") << "Server certificate mismatch";
					else if (server->ignoredSslErrors().contains(it->error()))
						ignoredErrors.append(*it);

				}

				if (ignoredErrors.size() == e.size()) {
					LOG_CDEBUG("http") << "Ignore SSL errors:" << ignoredErrors;
					m_reply->ignoreSslErrors(ignoredErrors);
					return;
				}
			}
		}

		m_pending = false;
		emit failed(this);
		emit finished();

		if (m_sslErrorCallback)
			m_sslErrorCallback(e);
		else
			emit m_socket->socketSslErrors(e);
	});
#endif

	connect(m_reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError e){
		m_pending = false;
		if (m_socket)
			emit m_socket->socketError(e);
		this->abort();
	});

	connect(m_reply, &QNetworkReply::finished, this, &HttpReply::onReplyFinished);

	connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 rec, qint64 total){
		if (total <= 0)
			emit downloadProgress(0);
		else
			emit downloadProgress((qreal)rec/(qreal)total);
	});

	connect(m_reply, &QNetworkReply::uploadProgress, this, [this](qint64 rec, qint64 total){
		if (total <= 0)
			emit uploadProgress(0);
		else
			emit uploadProgress((qreal)rec/(qreal)total);
	});
}



/**
 * @brief HttpConnectionReply::HttpConnectionReply
 * @param error
 */

HttpReply::HttpReply(const QNetworkReply::NetworkError &error, QObject *parent)
	: QObject(parent)
{
	LOG_CTRACE("http") << "HttpConnectionReply created" << error << this;

	QMetaObject::invokeMethod(this, std::bind(&HttpReply::onErrorPresent, this, error), Qt::QueuedConnection);
}


/**
 * @brief HttpConnectionReply::~HttpConnectionReply
 */

HttpReply::~HttpReply()
{
	if (m_socket) {
		m_socket->m_replies.removeAll(this);
		m_socket->checkPending();
	}

	LOG_CTRACE("http") << "HttpConnectionReply destroyed" << this;
}



/**
 * @brief HttpConnectionReply::abort
 */

void HttpReply::abort()
{
	m_pending = false;

	emit failed(this);

	LOG_CTRACE("http") << "Abort" << this;

	emit finished();

	QTimer::singleShot(HTTPREPLY_DELETE_AFTER_MSEC, this, &HttpReply::close);

	//close();
}


/**
 * @brief HttpConnectionReply::done
 */

void HttpReply::close()
{
	LOG_CTRACE("http") << "HttpConnectionReply done" << this << parent();
	this->deleteLater();
}


/**
 * @brief HttpConnectionReply::done
 * @param func
 * @return
 */




/**
 * @brief HttpConnectionReply::onReplyFinished
 */

void HttpReply::onReplyFinished()
{
	Q_ASSERT(m_socket);

	m_pending = false;
	m_socket->checkPending();

	const QNetworkReply::NetworkError &error = m_reply->error();


	if (error != QNetworkReply::NoError) {
		LOG_CWARNING("http") << "HttpConnectionReply error" << error << this;
		emit finished();

		foreach (const auto &func, m_funcsError) {
			if (func.first)
				func.second(error);
		}

		foreach (auto func, m_jsvaluesError) {
			if (func.first)
				func.second.call({error});
		}

		QTimer::singleShot(HTTPREPLY_DELETE_AFTER_MSEC, this, &HttpReply::close);

		return;
	}

	QJsonObject contentJson;
	QString errorString;

	QByteArray content;

	if (m_reply->isReadable()) {
		content = m_reply->readAll();
		contentJson = QJsonDocument::fromJson(content).object();

		if (!contentJson.isEmpty() && contentJson.contains(QStringLiteral("error")))
			errorString = contentJson.value(QStringLiteral("error")).toString();
	}



	if (errorString.isEmpty()) {
		LOG_CTRACE("http") << "RECEIVED:" << content.size() << contentJson;

		foreach (const auto &func, m_funcsByteArray) {
			if (func.first)
				func.second(content);
		}

		foreach (const auto &func, m_funcs) {
			if (func.first)
				func.second(contentJson);
		}

		QJSValueList list;

		QJSEngine *engine = qjsEngine(Application::instance()->client());

		if (engine)
			list = {engine->toScriptValue<QJsonObject>(contentJson)};
		else
			LOG_CERROR("http") << "Invalid JSEngine";

		foreach (auto v, m_jsvalues) {
			if (v.first)
				v.second.call(list);
		}
	} else {
		LOG_CWARNING("http") << "Response error:" << errorString << this;

		emit m_socket->responseError(errorString);

		foreach (const auto &func, m_funcsFail) {
			if (func.first)
				func.second(errorString);
		}

		foreach (auto v, m_jsvaluesFail) {
			if (v.first)
				v.second.call({errorString});
		}
	}

	emit finished();

	QTimer::singleShot(HTTPREPLY_DELETE_AFTER_MSEC, this, &HttpReply::close);
}



/**
 * @brief HttpConnectionReply::onErrorPresent
 * @param error
 */

void HttpReply::onErrorPresent(const QNetworkReply::NetworkError &error)
{
	LOG_CWARNING("http") << "HttpConnectionReply error" << error << this;

	emit finished();

	foreach (const auto &func, m_funcsError) {
		if (func.first)
			func.second(error);
	}

	foreach (auto v, m_jsvaluesError) {
		if (v.first)
			v.second.call({error});
	}

	QTimer::singleShot(HTTPREPLY_DELETE_AFTER_MSEC, this, &HttpReply::close);
}


/**
 * @brief HttpConnectionReply::sslErrorCallback
 * @return
 */

const std::function<void (const QList<QSslError> &)> &HttpReply::sslErrorCallback() const
{
	return m_sslErrorCallback;
}

void HttpReply::setSslErrorCallback(const std::function<void (const QList<QSslError> &)> &newSslErrorCallback)
{
	m_sslErrorCallback = newSslErrorCallback;
}


/**
 * @brief HttpConnectionReply::pending
 * @return
 */

bool HttpReply::pending() const
{
	return m_pending;
}


/**
 * @brief HttpConnectionReply::done
 * @param v
 * @return
 */

HttpReply *HttpReply::done(QObject *inst, const QJSValue &v)
{
	if (!v.isCallable())
		LOG_CERROR("http") << "QJSValue isn't callable";
	else
		m_jsvalues.append(qMakePair(inst, v));
	return this;
}


/**
 * @brief HttpConnectionReply::fail
 * @param v
 * @return
 */

HttpReply *HttpReply::fail(QObject *inst, const QJSValue &v)
{
	if (!v.isCallable())
		LOG_CERROR("http") << "QJSValue isn't callable";
	else
		m_jsvaluesFail.append(qMakePair(inst, v));
	return this;
}


/**
 * @brief HttpConnectionReply::error
 * @param v
 * @return
 */

HttpReply *HttpReply::error(QObject *inst, const QJSValue &v)
{
	if (!v.isCallable())
		LOG_CERROR("http") << "QJSValue isn't callable";
	else
		m_jsvaluesError.append(qMakePair(inst, v));
	return this;
}



