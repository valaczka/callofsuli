/*
 * ---- Call of Suli ----
 *
 * client.cpp
 *
 * Created on: 2020. 03. 22.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Client
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  Call of Suli is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QResource>
#include <QQuickItem>
#include <QJsonDocument>

#include "client.h"
#include "servers.h"
#include "../version/buildnumber.h"

Client::Client(QObject *parent) : QObject(parent)
{
	m_connectionState = ConnectionState::Standby;
	m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
	m_timer = new QTimer(this);

	connect(m_socket, &QWebSocket::connected, this, &Client::onSocketConnected);
	connect(m_socket, &QWebSocket::disconnected, this, &Client::onSocketDisconnected);
	connect(m_socket, &QWebSocket::stateChanged, this, &Client::onSocketStateChanged);
	connect(m_socket, &QWebSocket::sslErrors, this, &Client::onSocketSslErrors);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
			[=](QAbstractSocket::SocketError error){
		qDebug() << "error" << error;
		if (m_connectionState == ConnectionState::Standby || m_connectionState == ConnectionState::Connecting)
		sendMessageError(m_socket->errorString(), m_socket->requestUrl().toString());
	});

	connect(m_timer, &QTimer::timeout, this, &Client::socketPing);
	m_timer->start(5000);
}

/**
 * @brief Client::~Client
 */

Client::~Client()
{
	delete m_timer;
	delete m_socket;
}


/**
 * @brief Client::initialize
 */

void Client::initialize()
{
#ifndef QT_NO_DEBUG_OUTPUT
	qSetMessagePattern("%{time hh:mm:ss} [%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{message}");
#endif

	QCoreApplication::setApplicationName("callofsuli");
	QCoreApplication::setOrganizationDomain("client.callofsuli.vjp.piarista.hu");
	//QCoreApplication::setOrganizationName("Call of Suli");
	QCoreApplication::setApplicationVersion(_VERSION_FULL);

}



/**
 * @brief Client::registerResource
 * @param filename
 * @return
 */

bool Client::registerResource(const QString &filename)
{
#ifdef Q_OS_ANDROID
	if (QResource::registerResource("assets:/"+filename)) {
		qDebug().noquote() << tr("Registered resource from assets") + filename;
		return true;
	}
#endif

	QString binDir = QCoreApplication::applicationDirPath();

	QStringList searchList;

	searchList.append(binDir);
	searchList.append(binDir+"/share");
	searchList.append(binDir+"/../share");
	searchList.append(binDir+"/../../share");
	searchList.append(binDir+"/../../../share");
	searchList.append(binDir+"/../../../../share");
	searchList.append(binDir+"/callofsuli/share");
	searchList.append(binDir+"/../callofsuli/share");
	searchList.append(binDir+"/../../callofsuli/share");
	searchList.append(binDir+"/../../../callofsuli/share");
	searchList.append(binDir+"/../../../../callofsuli/share");
	searchList.append(QDir::rootPath()+"usr/local/share/callofsuli");
	searchList.append(QDir::rootPath()+"usr/share/callofsuli");
	searchList.append(QDir::rootPath()+"share/callofsuli");

	searchList.append(QStandardPaths::displayName(QStandardPaths::HomeLocation));

	foreach (QString dir, searchList)
	{
		QFile file(dir+"/"+filename);

		if (file.exists())
		{
			QString realname=QDir(file.fileName()).canonicalPath();
			qInfo().noquote() << tr("Registered resource: ")+realname;
			QResource::registerResource(realname);

			return true;
		}
	}

	return false;
}



/**
 * @brief Client::registerTypes
 */

void Client::registerTypes()
{
	qmlRegisterType<Client>("COS.Client", 1, 0, "Client");
	qmlRegisterType<Servers>("COS.Client", 1, 0, "Servers");
}



/**
 * @brief Client::windowSaveGeometry
 * @param window
 */

void Client::windowSaveGeometry(QQuickWindow *window)
{
#ifndef Q_OS_ANDROID
	QSettings s;
	s.beginGroup("window");

	s.setValue("size", window->size());
	s.setValue("position", window->position());
	s.setValue("visibility", window->visibility());

	s.endGroup();
#else
	Q_UNUSED(window)
#endif
}



/**
 * @brief Client::restoreWindowGeometry
 * @param window
 * @param forceFullscreen
 */

void Client::windowRestoreGeometry(QQuickWindow *window, const bool &forceFullscreen)
{
#ifndef Q_OS_ANDROID
	QSettings s;
	s.beginGroup("window");

	window->resize(s.value("size", QSize(600, 600)).toSize());
	window->setPosition(s.value("position", QPoint(0, 0)).toPoint());

	int v = s.value("visibility", QWindow::AutomaticVisibility).toInt();

	if (forceFullscreen)
		v = QWindow::FullScreen;

	switch (v) {
		case QWindow::Windowed:
		case QWindow::Maximized:
		case QWindow::FullScreen:
		case QWindow::AutomaticVisibility:
			window->setVisibility(static_cast<QWindow::Visibility>(v));
			break;
	}

	s.endGroup();

#else
	Q_UNUSED(window);
	Q_UNUSED(forceFullscreen);
#endif
}


/**
 * @brief Client::windowSetIcon
 * @param window
 */

void Client::windowSetIcon(QQuickWindow *window)
{
	window->setIcon(QIcon(":/img/cos96.png"));
}


/**
 * @brief Client::standardPathCreate
 */

void Client::standardPathCreate()
{
	QDir d(QStandardPaths::standardLocations(QStandardPaths::DataLocation).first());
	if (!d.exists()) {
		qInfo().noquote() << tr("Create directory ") + d.absolutePath();
		d.mkpath(d.absolutePath());
	}
}


/**
 * @brief Client::standardPath
 * @param path
 * @return
 */

QString Client::standardPath(const QString &path)
{
	return QStandardPaths::standardLocations(QStandardPaths::DataLocation).first()+"/"+path;
}



/**
 * @brief Client::setSetting
 * @param key
 * @param value
 */

void Client::setSetting(const QString &key, const QVariant &value)
{
	QSettings s;
	s.setValue(key, value);
}


/**
 * @brief Client::getSetting
 * @param key
 * @return
 */

QVariant Client::getSetting(const QString &key)
{
	QSettings s;
	return s.value(key);
}

void Client::setConnectionState(Client::ConnectionState connectionState)
{
	if (m_connectionState == connectionState)
		return;

	qDebug() << tr("set connection state") << connectionState;

	m_connectionState = connectionState;
	emit connectionStateChanged(m_connectionState);
}


/**
 * @brief Client::closeConnection
 */

void Client::closeConnection()
{
	if (m_socket->state() == QAbstractSocket::ConnectedState) {
		setConnectionState(ConnectionState::Closing);
		m_socket->close();
	} else {
		setConnectionState(ConnectionState::Standby);
		m_socket->close();
	}
}


/**
 * @brief Client::sendData
 */

void Client::sendData()
{
	QJsonObject o;

	o["name"] = "Valaczka János";
	o["size"] = 234;
	o["bool"] = true;

	QJsonDocument d(o);

	QByteArray data;
	QDataStream ds(&data, QIODevice::ReadWrite);
	ds.setVersion(QDataStream::Qt_5_14);

	ds << quint64(1234);
	ds << QString("--ez a hash--");
	ds << true;
	ds << d.toJson(QJsonDocument::Compact);

	QFile f(standardPath("servers.db"));
	f.open(QIODevice::ReadOnly);
	QByteArray fff = f.readAll();

	QByteArray hash = QCryptographicHash::hash(fff, QCryptographicHash::Sha1);

	ds << hash << fff;

	f.close();


	m_socket->sendBinaryMessage(data);

	qDebug() << "data sent" << hash.toHex();
}





/**
 * @brief Client::setSocket
 * @param socket
 */

void Client::setSocket(QWebSocket *socket)
{
	if (m_socket == socket)
		return;

	m_socket = socket;
	emit socketChanged(m_socket);
}


/**
 * @brief Client::socketPing
 */

void Client::socketPing()
{
	if (m_connectionState == ConnectionState::Connected || m_connectionState == ConnectionState::Reconnected) {
		qDebug() << "ping";
		m_socket->ping();
	} else if (m_connectionState == ConnectionState::Disconnected) {
		qDebug() << "reconnect";
		m_socket->open(m_connectedUrl);
	}
}



/**
 * @brief Client::onSocketConnected
 */

void Client::onSocketConnected()
{
	m_connectedUrl = m_socket->requestUrl();
	if (m_connectionState == ConnectionState::Connecting || m_connectionState == ConnectionState::Standby)
		setConnectionState(ConnectionState::Connected);
	else if (m_connectionState == ConnectionState::Reconnecting || m_connectionState == ConnectionState::Disconnected)
		setConnectionState(ConnectionState::Reconnected);
}


void Client::onSocketDisconnected()
{
	if (m_connectionState == ConnectionState::Connected ||
		m_connectionState == ConnectionState::Reconnecting ||
		m_connectionState == ConnectionState::Reconnected)
		setConnectionState(ConnectionState::Disconnected);
	else
		setConnectionState(ConnectionState::Standby);

}

void Client::onSocketBinaryFrameReceived(const QByteArray &frame, bool isLastFrame)
{
	Q_UNUSED(frame)
	Q_UNUSED(isLastFrame)
}

void Client::onSocketBinaryMessageReceived(const QByteArray &message)
{
	Q_UNUSED(message)
}

void Client::onSocketBytesWritten(qint64 bytes)
{
	Q_UNUSED(bytes)
}

void Client::onSocketError(QAbstractSocket::SocketError error)
{
	qDebug() << "error" << error;
}

void Client::onSocketSslErrors(const QList<QSslError> &errors)
{
	qDebug() << "ssl error" << errors;
}


/**
 * @brief Client::onSocketStateChanged
 * @param state
 */

void Client::onSocketStateChanged(QAbstractSocket::SocketState state)
{
	if (m_connectionState == ConnectionState::Standby && state == QAbstractSocket::ConnectingState)
		setConnectionState(ConnectionState::Connecting);
	else if (m_connectionState == ConnectionState::Disconnected && state == QAbstractSocket::ConnectingState)
		setConnectionState(ConnectionState::Reconnecting);

}

