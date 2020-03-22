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


#include "client.h"
#include "servers.h"
#include "../version/buildnumber.h"

Client::Client(QObject *parent) : QObject(parent)
{
	m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
}

/**
 * @brief Client::~Client
 */

Client::~Client()
{
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

	QCoreApplication::setApplicationName(QString::fromUtf8("Call of Suli"));
	QCoreApplication::setOrganizationDomain("client.callofsuli.vjp.piarista.hu");
	QCoreApplication::setOrganizationName("Call of Suli");
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

	d.mkdir("db");
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




/**
 * @brief Client::socketOpen
 */

void Client::socketOpen()
{
	m_socket->open(m_socketUrl);
}


/**
 * @brief Client::socketLoadSslCerts
 */

void Client::socketLoadSslCerts()
{
	QList<QSslCertificate> cert = QSslCertificate::fromPath(QLatin1String("server-certificate.pem"));
	QSslError error(QSslError::SelfSignedCertificate, cert.at(0));
	QList<QSslError> expectedSslErrors;
	expectedSslErrors.append(error);

	m_socket->ignoreSslErrors(expectedSslErrors);
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

void Client::setSocketUrl(QUrl socketUrl)
{
	if (m_socketUrl == socketUrl)
		return;

	m_socketUrl = socketUrl;
	emit socketUrlChanged(m_socketUrl);
}

void Client::onSocketConnected()
{

}

void Client::onSocketDisconnected()
{

}

void Client::onSocketBinaryFrameReceived(const QByteArray &frame, bool isLastFrame)
{

}

void Client::onSocketBinaryMessageReceived(const QByteArray &message)
{

}

void Client::onSocketBytesWritten(qint64 bytes)
{

}

void Client::onSocketError(QAbstractSocket::SocketError error)
{

}

