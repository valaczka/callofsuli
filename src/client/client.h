/*
 * ---- Call of Suli ----
 *
 * client.h
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


#ifndef CLIENT_H
#define CLIENT_H

#include <QWebSocket>
#include <QObject>
#include <QQuickWindow>
#include <QSettings>


class Client : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QWebSocket * socket READ socket WRITE setSocket NOTIFY socketChanged)
	Q_PROPERTY(QUrl socketUrl READ socketUrl WRITE setSocketUrl NOTIFY socketUrlChanged)

public:
	explicit Client(QObject *parent = nullptr);
	virtual ~Client();

	QWebSocket * socket() const { return m_socket; }
	QUrl socketUrl() const { return m_socketUrl; }

	static bool registerResource(const QString &filename = "callofsuli.rcc");
	static void registerTypes();
	static void initialize();
	static void standardPathCreate();

	Q_INVOKABLE void windowSaveGeometry(QQuickWindow *window);
	Q_INVOKABLE void windowRestoreGeometry(QQuickWindow *window, const bool &forceFullscreen = false);
	Q_INVOKABLE void windowSetIcon(QQuickWindow *window);

	Q_INVOKABLE static QString standardPath(const QString &path = QString());
	Q_INVOKABLE static void setSetting(const QString &key, const QVariant &value);
	Q_INVOKABLE static QVariant getSetting(const QString &key);

public slots:
	void socketOpen();
	void socketLoadSslCerts();

	void setSocketUrl(QUrl socketUrl);

	void sendMessageWarning(const QString &title, const QString &informativeText, const QString &detailedText) {
		emit messageSent("warning", title, informativeText, detailedText);
	}

private slots:
	void setSocket(QWebSocket * socket);

	void onSocketConnected();
	void onSocketDisconnected();
	void onSocketBinaryFrameReceived(const QByteArray &frame, bool isLastFrame);
	void onSocketBinaryMessageReceived(const QByteArray &message);
	void onSocketBytesWritten(qint64 bytes);
	void onSocketError(QAbstractSocket::SocketError error);


signals:
	void socketChanged(QWebSocket * socket);
	void socketUrlChanged(QUrl socketUrl);

	void messageSent(const QString &type,
					 const QString &title,
					 const QString &informativeText,
					 const QString &detailedText);

private:
	QWebSocket* m_socket;
	QUrl m_socketUrl;


};

#endif // CLIENT_H
