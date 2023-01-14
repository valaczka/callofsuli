/*
 * ---- Call of Suli ----
 *
 * oauth2replyhandler.cpp
 *
 * Created on: 2023. 01. 04.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * OAuth2ReplyHandler
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

#include "oauth2replyhandler.h"
#include "Logger.h"
#include "oauth2authenticator.h"



/**
 * @brief AbstractReplyHandler::~AbstractReplyHandler
 */

AbstractReplyHandler::AbstractReplyHandler(OAuth2Authenticator *authenticator)
	: m_authenticator(authenticator)
{
	LOG_CTRACE("oauth2") << "Abstract reply handler created" << this;
}


/**
 * @brief AbstractReplyHandler::~AbstractReplyHandler
 */

AbstractReplyHandler::~AbstractReplyHandler()
{
	LOG_CTRACE("oauth2") << "Abstract reply handler destroyed" << this;
}


/**
 * @brief AbstractReplyHandler::onCallbackReceived
 * @param data
 */

void AbstractReplyHandler::onCallbackReceived(const QVariantMap &data)
{
	LOG_CTRACE("oauth2") << "Callback received" << data;

	const QString error = data.value(QStringLiteral("error")).toString();
	const QString code = data.value(QStringLiteral("code")).toString();//QUrl::fromPercentEncoding(data.value(QStringLiteral("code")).toByteArray());
	const QString receivedState = data.value(QStringLiteral("state")).toString();
	if (error.size()) {
		const QString uri = data.value(QStringLiteral("error_uri")).toString();
		const QString description = data.value(QStringLiteral("error_description")).toString();

		LOG_CERROR("oauth2") << "AuthenticationError:" << qPrintable(error) << qPrintable(uri) << qPrintable(description);
		return;
	}

	if (code.isEmpty()) {
		LOG_CERROR("oauth2") << "AuthenticationError: Code not received";
		return;
	}

	if (receivedState.isEmpty()) {
		LOG_CERROR("oauth2") << "State not received";
		return;
	}

	if (!m_authenticator) {
		LOG_CERROR("oauth2") << "Missing authenticator";
		return;
	}

	OAuth2CodeFlow *flow = m_authenticator->getCodeFlowForState(receivedState);


	if (flow) {
		flow->requestAccesToken(code);
	} else {
		LOG_CTRACE("oauth2") << "Flow not found" << flow;
	}
}
















#ifndef QT_NO_SSL

OAuthHttpServerReplySslHandler::OAuthHttpServerReplySslHandler(OAuth2Authenticator *authenticator)
	: OAuthHttpServerReplySslHandler(QHostAddress::Any, 0, authenticator)
{}

OAuthHttpServerReplySslHandler::OAuthHttpServerReplySslHandler(OAuth2Authenticator *authenticator, quint16 port)
	: 	OAuthHttpServerReplySslHandler(QHostAddress::Any, port, authenticator)
{}


OAuthHttpServerReplySslHandler::OAuthHttpServerReplySslHandler(const QHostAddress &address, quint16 port, OAuth2Authenticator *authenticator)
	:	QOAuthOobReplyHandler(authenticator)
	, m_authenticator(authenticator)
	, d_ptr(new OAuthHttpServerReplySslHandlerPrivate(this))
{
	Q_ASSERT(authenticator);

	listen(address, port);
}


OAuthHttpServerReplySslHandler::~OAuthHttpServerReplySslHandler()
{

}



QString OAuthHttpServerReplySslHandler::callback() const
{
	Q_D(const OAuthHttpServerReplySslHandler);
	Q_ASSERT(d->httpServer.isListening());
	const QUrl url(QString::fromLatin1("https://%1:%2/%3")
				   .arg(m_authenticator->redirectHost()).arg(d->httpServer.serverPort()).arg(d->path));
	return url.toString(QUrl::EncodeDelimiters);
}

QString OAuthHttpServerReplySslHandler::callbackPath() const
{
	Q_D(const OAuthHttpServerReplySslHandler);
	return d->path;
}

void OAuthHttpServerReplySslHandler::setCallbackPath(const QString &path)
{
	Q_D(OAuthHttpServerReplySslHandler);
	QString copy = path;
	while (copy.startsWith(QLatin1Char('/')))
		copy = copy.mid(1);
	d->path = copy;
}

QString OAuthHttpServerReplySslHandler::callbackText() const
{
	Q_D(const OAuthHttpServerReplySslHandler);
	return d->text;
}

void OAuthHttpServerReplySslHandler::setCallbackText(const QString &text)
{
	Q_D(OAuthHttpServerReplySslHandler);
	d->text = text;
}

quint16 OAuthHttpServerReplySslHandler::port() const
{
	Q_D(const OAuthHttpServerReplySslHandler);
	return d->httpServer.serverPort();
}

bool OAuthHttpServerReplySslHandler::listen(const QHostAddress &address, quint16 port)
{
	Q_D(OAuthHttpServerReplySslHandler);
	return d->httpServer.listen(address, port);
}

void OAuthHttpServerReplySslHandler::close()
{
	Q_D(OAuthHttpServerReplySslHandler);
	return d->httpServer.close();
}

bool OAuthHttpServerReplySslHandler::isListening() const
{
	Q_D(const OAuthHttpServerReplySslHandler);
	return d->httpServer.isListening();
}

const QSslConfiguration &OAuthHttpServerReplySslHandler::configuration() const
{
	Q_D(const OAuthHttpServerReplySslHandler);
	return d->httpServer.configuration();
}

void OAuthHttpServerReplySslHandler::setConfiguration(const QSslConfiguration &newConfiguration)
{
	Q_D(OAuthHttpServerReplySslHandler);
	d->httpServer.setConfiguration(newConfiguration);
}












/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::OAuthHttpServerReplySslHandlerPrivate
 * @param p
 */

OAuthHttpServerReplySslHandlerPrivate::OAuthHttpServerReplySslHandlerPrivate(OAuthHttpServerReplySslHandler *p)
	: text(QObject::tr("Callback received. Feel free to close this page.")), q_ptr(p)
{
	QObject::connect(&httpServer, &QTcpServer::newConnection,
					 [this]() { _q_clientConnected(); });
}

/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::~OAuthHttpServerReplySslHandlerPrivate
 */

OAuthHttpServerReplySslHandlerPrivate::~OAuthHttpServerReplySslHandlerPrivate()
{
	if (httpServer.isListening())
		httpServer.close();
}


/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::_q_clientConnected
 */

void OAuthHttpServerReplySslHandlerPrivate::_q_clientConnected()
{
	QTcpSocket *socket = httpServer.nextPendingConnection();
	LOG_CTRACE("oauth2") << "Client connected" << socket->peerAddress() << socket->peerPort();
	QObject::connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
	QObject::connect(socket, &QTcpSocket::readyRead,
					 [this, socket]() { _q_readData(socket); });
}


/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::_q_readData
 * @param socket
 */

void OAuthHttpServerReplySslHandlerPrivate::_q_readData(QTcpSocket *socket)
{
	if (!clients.contains(socket))
		clients[socket].port = httpServer.serverPort();
	QHttpRequest *request = &clients[socket];
	bool error = false;
	if (Q_LIKELY(request->state == QHttpRequest::State::ReadingMethod))
		if (Q_UNLIKELY(error = !request->readMethod(socket)))
			LOG_CWARNING("oauth2") << "Invalid Method";
	if (Q_LIKELY(!error && request->state == QHttpRequest::State::ReadingUrl))
		if (Q_UNLIKELY(error = !request->readUrl(socket)))
			LOG_CWARNING("oauth2") << "Invalid URL";
	if (Q_LIKELY(!error && request->state == QHttpRequest::State::ReadingStatus))
		if (Q_UNLIKELY(error = !request->readStatus(socket)))
			LOG_CWARNING("oauth2") << "Invalid Status";
	if (Q_LIKELY(!error && request->state == QHttpRequest::State::ReadingHeader))
		if (Q_UNLIKELY(error = !request->readHeader(socket)))
			LOG_CWARNING("oauth2") << "Invalid Header";
	if (error) {
		socket->disconnectFromHost();
		clients.remove(socket);
	} else if (!request->url.isEmpty()) {
		Q_ASSERT(request->state != QHttpRequest::State::ReadingUrl);
		_q_answerClient(socket, request->url);
		clients.remove(socket);
	}
}




/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::_q_answerClient
 * @param socket
 * @param url
 */

void OAuthHttpServerReplySslHandlerPrivate::_q_answerClient(QTcpSocket *socket, const QUrl &url)
{
	Q_Q(OAuthHttpServerReplySslHandler);
	if (!url.path().startsWith(QLatin1String("/") + path)) {
		LOG_CWARNING("oauth2") << "Invalid request: %s", qPrintable(url.toString());
	} else {
		QVariantMap receivedData;
		const QUrlQuery query(url.query());
		const auto items = query.queryItems();
		for (auto it = items.begin(), end = items.end(); it != end; ++it)
			receivedData.insert(it->first, it->second);
		Q_EMIT q->callbackReceived(receivedData);
		const QByteArray html = QByteArrayLiteral("<html><head><title>") +
				qApp->applicationName().toUtf8() +
				QByteArrayLiteral("</title></head><body>") +
				text.toUtf8() +
				QByteArrayLiteral("</body></html>");
		const QByteArray htmlSize = QByteArray::number(html.size());
		const QByteArray replyMessage = QByteArrayLiteral("HTTP/1.0 200 OK \r\n"
														  "Content-Type: text/html; "
														  "charset=\"utf-8\"\r\n"
														  "Content-Length: ") + htmlSize +
				QByteArrayLiteral("\r\n\r\n") +
				html;
		socket->write(replyMessage);
	}
	socket->disconnectFromHost();
}



/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readMethod
 * @param socket
 * @return
 */

bool OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readMethod(QTcpSocket *socket)
{
	bool finished = false;
	while (socket->bytesAvailable() && !finished) {
		char c;
		socket->getChar(&c);
		if (std::isupper(c) && fragment.size() < 6)
			fragment += c;
		else
			finished = true;
	}
	if (finished) {
		if (fragment == "HEAD")
			method = Method::Head;
		else if (fragment == "GET")
			method = Method::Get;
		else if (fragment == "PUT")
			method = Method::Put;
		else if (fragment == "POST")
			method = Method::Post;
		else if (fragment == "DELETE")
			method = Method::Delete;
		else
			LOG_CWARNING("oauth2") << "Invalid operation %s", fragment.data();
		state = State::ReadingUrl;
		fragment.clear();
		return method != Method::Unknown;
	}
	return true;
}



/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readUrl
 * @param socket
 * @return
 */

bool OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readUrl(QTcpSocket *socket)
{
	bool finished = false;
	while (socket->bytesAvailable() && !finished) {
		char c;
		socket->getChar(&c);
		if (std::isspace(c))
			finished = true;
		else
			fragment += c;
	}
	if (finished) {
		if (!fragment.startsWith("/")) {
			LOG_CWARNING("oauth2") << "Invalid URL path %s", fragment.constData();
			return false;
		}
		url.setUrl(QStringLiteral("http://127.0.0.1:") + QString::number(port) +
				   QString::fromUtf8(fragment));
		state = State::ReadingStatus;
		if (!url.isValid()) {
			LOG_CWARNING("oauth2") << "Invalid URL %s", fragment.constData();
			return false;
		}
		fragment.clear();
		return true;
	}
	return true;
}



/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readStatus
 * @param socket
 * @return
 */

bool OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readStatus(QTcpSocket *socket)
{
	bool finished = false;
	while (socket->bytesAvailable() && !finished) {
		char c;
		socket->getChar(&c);
		fragment += c;
		if (fragment.endsWith("\r\n")) {
			finished = true;
			fragment.resize(fragment.size() - 2);
		}
	}
	if (finished) {
		if (!std::isdigit(fragment.at(fragment.size() - 3)) ||
				!std::isdigit(fragment.at(fragment.size() - 1))) {
			LOG_CWARNING("oauth2") << "Invalid version";
			return false;
		}
		version = qMakePair(fragment.at(fragment.size() - 3) - '0',
							fragment.at(fragment.size() - 1) - '0');
		state = State::ReadingHeader;
		fragment.clear();
	}
	return true;
}




/**
 * @brief OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readHeader
 * @param socket
 * @return
 */

bool OAuthHttpServerReplySslHandlerPrivate::QHttpRequest::readHeader(QTcpSocket *socket)
{
	while (socket->bytesAvailable()) {
		char c;
		socket->getChar(&c);
		fragment += c;
		if (fragment.endsWith("\r\n")) {
			if (fragment == "\r\n") {
				state = State::ReadingBody;
				fragment.clear();
				return true;
			} else {
				fragment.chop(2);
				const int index = fragment.indexOf(':');
				if (index == -1)
					return false;
				const QByteArray key = fragment.mid(0, index).trimmed();
				const QByteArray value = fragment.mid(index + 1).trimmed();
				headers.insert(key, value);
				fragment.clear();
			}
		}
	}
	return false;
}





/**
 * @brief SslServer::incomingConnection
 * @param socketDescriptor
 */

void SslServer::incomingConnection(qintptr socketDescriptor)
{
	QSslSocket *sslSocket = new QSslSocket(this);
	sslSocket->setSocketDescriptor(socketDescriptor);
	sslSocket->setSslConfiguration(m_configuration);
	sslSocket->startServerEncryption();

	this->addPendingConnection(sslSocket);
}


#endif




