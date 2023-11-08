/*
 * ---- Call of Suli ----
 *
 * eventstream.cpp
 *
 * Created on: 2023. 03. 26.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * EventStream
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

#include "eventstream.h"
#include "Logger.h"
#include "qjsondocument.h"
#include "server.h"
#include "httpconnection.h"
#include "application.h"


/**
 * @brief EventStream::EventStream
 * @param socket
 */

EventStream::EventStream(HttpConnection *socket)
	: QObject{socket}
	, m_socket(socket)
{
	Q_ASSERT(m_socket);

	LOG_CDEBUG("http") << "Event stream created:" << this;

	QObject::connect(this, &EventStream::finished, this, &EventStream::deleteLater);
}


/**
 * @brief EventStream::~EventStream
 */

EventStream::~EventStream()
{
	if (m_reply) {
		m_reply->deleteLater();
		m_reply = nullptr;
	}

	LOG_CDEBUG("http") << "Event stream destroyed:" << this;
}


/**
 * @brief EventStream::request
 * @return
 */

const QNetworkRequest &EventStream::request() const
{
	return m_request;
}

void EventStream::setRequest(const QNetworkRequest &newRequest)
{
	if (m_request == newRequest)
		return;
	m_request = newRequest;
	emit requestChanged();
}

const QByteArray &EventStream::requestData() const
{
	return m_requestData;
}

void EventStream::setRequestData(const QByteArray &newRequestData)
{
	if (m_requestData == newRequestData)
		return;
	m_requestData = newRequestData;
	emit requestDataChanged();
}

HttpConnection *EventStream::socket() const
{
	return m_socket;
}

bool EventStream::reconnect() const
{
	return m_reconnect;
}

void EventStream::setReconnect(bool newReconnect)
{
	if (m_reconnect == newReconnect)
		return;
	m_reconnect = newReconnect;
	emit reconnectChanged();
}



/**
 * @brief EventStream::connect
 */

void EventStream::connect(Server *server)
{
	LOG_CDEBUG("http") << "Connect event stream:" << m_request.url().toString();

	if (m_reply) {
		if (m_reply->isOpen()) {
			LOG_CDEBUG("http") << "QNetworkReply is already present and opened";
			return;
		}
		m_reply->deleteLater();
		m_reply = nullptr;
	}

	m_alreadyConnected = false;

	m_reply = m_socket->networkManager()->post(m_request, m_requestData);

#ifndef QT_NO_SSL
	if (server && !server->certificate().isEmpty()) {
		QSslCertificate cert(server->certificate());
		if (cert.isNull()) {
			LOG_CERROR("http") << "Invalid server certificate stored";
		} else {
			QList<QSslError> expectedErrors;

			foreach (const QSslError::SslError &err, server->ignoredSslErrors()) {
				LOG_CDEBUG("http") << "Ignore SSL error:" << err;
				expectedErrors << QSslError(err, cert);
			}

			if (!expectedErrors.isEmpty())
				m_reply->ignoreSslErrors(expectedErrors);
		}
	}

	QObject::connect(m_reply, &QNetworkReply::sslErrors, m_socket, [this](const QList<QSslError> &e){
		LOG_CDEBUG("http") << "SSL error:" << e;

		QList<QSslError> list;

		foreach (const QSslError &err, e) {
#ifdef QT_DEBUG
			if (err.error() == QSslError::HostNameMismatch)
				list.append(err);
#endif
		}

		if (!list.isEmpty()) {
			m_reply->ignoreSslErrors(list);
			LOG_CWARNING("http") << "SSL errors ignored:" << list;
		}

		if (list.size() < e.size()) {
			emit finished();
			emit m_socket->socketSslErrors(e);
		}
	});
#endif

	QObject::connect(m_reply, &QNetworkReply::errorOccurred, m_socket, [this](QNetworkReply::NetworkError e){
		emit finished();
		emit m_socket->socketError(e);
	});

	QObject::connect(m_reply, &QNetworkReply::finished, this, &EventStream::onReplyFinished);
	QObject::connect(m_reply, &QNetworkReply::readyRead, this, &EventStream::onReplyReadyRead);
}


/**
 * @brief EventStream::disconnect
 */

void EventStream::disconnect()
{
	LOG_CDEBUG("http") << "Event stream disconnect:" << this;


	m_reconnect = false;

	if (m_reply) {
		m_reply->deleteLater();
		m_reply = nullptr;
	}

	emit disconnected();

	this->deleteLater();
}


/**
 * @brief EventStream::onReplyFinished
 */

void EventStream::onReplyFinished()
{
	LOG_CDEBUG("http") << "Event stream finished:" << m_request.url().toString();

	if (m_reply) {
		emit disconnected();

		m_reply->deleteLater();
		m_reply = nullptr;
	}

	if (!m_reconnect) {
		emit finished();
		return;
	}

	if (m_retries < MAX_RETRIES) {
		LOG_CDEBUG("http") << "Reconnecting:" << m_request.url().toString();
		m_retries++;
		connect();
	} else {
		LOG_CERROR("http") << "Unable to reconnect event stream, max retries reached:" << m_request.url().toString();
		emit finished();
	}
}



/**
 * @brief EventStream::onReplyReadyRead
 */

void EventStream::onReplyReadyRead()
{
	if (!m_reply)
		return;

	if (m_reply->header(QNetworkRequest::ContentTypeHeader).toString() != QLatin1String("text/event-stream")) {
		Application::instance()->messageError(tr("Érvénytelen adat érkezett"));
		emit disconnected();
		m_reply->abort();
		this->disconnect();
		return;
	}


	if (!m_alreadyConnected)
		emit connected();
	m_alreadyConnected = true;
	m_retries = 0;

	const QList<QByteArray> &lines = m_reply->readAll().split('\n');

	QByteArray event;
	QByteArray data;

	foreach (const QByteArray &l, lines) {
		QByteArray line = l.simplified();

		// If more events received
		if (line.isEmpty() && (!event.isEmpty() || !data.isEmpty())) {
			if (event == QByteArrayLiteral("hello") && data == QByteArrayLiteral("hello message"))
				emit eventHelloReceived();
			else
				emit eventReceived(event, data);

			QJsonDocument doc = QJsonDocument::fromJson(data);
			if (!doc.isEmpty() || doc.isObject())
				emit eventJsonReceived(QString::fromUtf8(event), doc.object());

			event.clear();
			data.clear();
		}

		if (line.startsWith(":"))
			continue;

		if (line.startsWith("event:"))
			event = line.replace("event: ", "");
		else if (line.startsWith("data:")) {
			if (!data.isEmpty())
				data += QByteArrayLiteral("\n");
			data += line.replace("data: ", "");
		}
	}

	if (!event.isEmpty() || !data.isEmpty()) {
		if (event == QByteArrayLiteral("hello") && data == QByteArrayLiteral("hello message"))
			emit eventHelloReceived();
		else
			emit eventReceived(event, data);
		QJsonDocument doc = QJsonDocument::fromJson(data);
		if (!doc.isEmpty() || doc.isObject())
			emit eventJsonReceived(QString::fromUtf8(event), doc.object());
	}
}
