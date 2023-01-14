/*
 * ---- Call of Suli ----
 *
 * client.h
 *
 * Created on: 2022. 12. 09.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * Client
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

#ifndef CLIENT_H
#define CLIENT_H

#include "qnetworkaccessmanager.h"
#include <QObject>
#include <QQuickItem>
#include <QLoggingCategory>
#include "server.h"
#include "utils.h"
#include "websocketmessage.h"
#include <QAbstractSocket>
#include "asyncmessagehandler.h"

class Application;
class AbstractGame;
class WebSocket;


/**
 * @brief The Client class
 */

class Client : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem* mainStack READ mainStack WRITE setMainStack NOTIFY mainStackChanged)
	Q_PROPERTY(QQuickWindow* mainWindow READ mainWindow WRITE setMainWindow NOTIFY mainWindowChanged)

	Q_PROPERTY(Utils* Utils READ utils CONSTANT)
	Q_PROPERTY(bool debug READ debug CONSTANT)

	Q_PROPERTY(AbstractGame* currentGame READ currentGame NOTIFY currentGameChanged)

	Q_PROPERTY(qreal safeMarginLeft READ safeMarginLeft NOTIFY safeMarginLeftChanged)
	Q_PROPERTY(qreal safeMarginRight READ safeMarginRight NOTIFY safeMarginRightChanged)
	Q_PROPERTY(qreal safeMarginTop READ safeMarginTop NOTIFY safeMarginTopChanged)
	Q_PROPERTY(qreal safeMarginBottom READ safeMarginBottom NOTIFY safeMarginBottomChanged)

	Q_PROPERTY(WebSocket *webSocket READ webSocket CONSTANT)
	Q_PROPERTY(Server *server READ server WRITE setServer NOTIFY serverChanged)



public:
	explicit Client(Application *app, QObject *parent = nullptr);
	virtual ~Client();

	QQuickItem *mainStack() const;
	void setMainStack(QQuickItem *newMainStack);

	Q_INVOKABLE QQuickItem *stackPushPage(const QString &qml, const QVariantMap &parameters = {}) const;
	Q_INVOKABLE bool stackPop(const int &index = -1, const bool &forced = false) const;
	Q_INVOKABLE bool stackPop(QQuickItem *page, const bool &forced = false) const;

	QQuickWindow *mainWindow() const;
	void setMainWindow(QQuickWindow *newMainWindow);

	Q_INVOKABLE bool closeWindow(const bool &forced = false);

	QNetworkAccessManager *networkManager() const;
	Utils *utils() const;
	Application *application() const;

	bool debug() const;

	AbstractGame *currentGame() const;
	void setCurrentGame(AbstractGame *newCurrentGame);

	Q_INVOKABLE void loadDemoMap();

	Q_INVOKABLE void messageInfo(const QString &text, QString title = "") const;
	Q_INVOKABLE void messageWarning(const QString &text, QString title = "") const;
	Q_INVOKABLE void messageError(const QString &text, QString title = "") const;

	Q_INVOKABLE void snack(const QString &text) const;


	// Safe margins

	Q_INVOKABLE void safeMarginsGet();

	qreal safeMarginLeft() const;
	void setSafeMarginLeft(qreal newSafeMarginLeft);

	qreal safeMarginRight() const;
	void setSafeMarginRight(qreal newSafeMarginRight);

	qreal safeMarginTop() const;
	void setSafeMarginTop(qreal newSafeMarginTop);

	qreal safeMarginBottom() const;
	void setSafeMarginBottom(qreal newSafeMarginBottom);

	void setSafeMargins(const QMarginsF &margins);


	// Server

	Server *server() const;
	void setServer(Server *newServer);

	Q_INVOKABLE void connectToServer(Server *server);

	// WebSocket

	WebSocket *webSocket() const;
	Q_INVOKABLE void sendRequest(const WebSocketMessage::ClassHandler &classHandler, const QJsonObject &json);

	void addMessageHandler(AsyncMessageHandler *handler);
	void removeMessageHandler(AsyncMessageHandler *handler);
	void handleMessage(const WebSocketMessage &message);


	Q_INVOKABLE void testConnect();
	Q_INVOKABLE void testHello();
	Q_INVOKABLE void testRequest();
	Q_INVOKABLE void testClose();
	Q_INVOKABLE void testText(const QString &username, const QString &password);
	Q_INVOKABLE void testToken(const QString &token);


protected slots:
	virtual void onApplicationStarted();
	friend class Application;

	virtual void onWebSocketError(const QAbstractSocket::SocketError &error);

protected:
	void _message(const QString &text, const QString &title, const QString &type) const;
	virtual bool handleMessageInternal(const WebSocketMessage &) { return false; }

signals:
	void pixelSizeChanged();
	void pixelSizeRatioChanged();
	void mainStackChanged();
	void mainWindowChanged();
	void currentGameChanged();
	void safeMarginLeftChanged();
	void safeMarginRightChanged();
	void safeMarginTopChanged();
	void safeMarginBottomChanged();
	void serverChanged();


protected:
	Application *const m_application;
	QQuickItem *m_mainStack = nullptr;
	QQuickWindow *m_mainWindow = nullptr;
	bool m_mainWindowClosable = false;

	QNetworkAccessManager *const m_networkManager;
	Utils *const m_utils;
	AbstractGame *m_currentGame = nullptr;
	WebSocket *const m_webSocket;
	Server *m_server = nullptr;

	qreal m_safeMarginLeft = 0;
	qreal m_safeMarginRight = 0;
	qreal m_safeMarginTop = 0;
	qreal m_safeMarginBottom = 0;

	QVector<QPointer<AsyncMessageHandler>> m_messageHandlers;

private:

};

#endif // CLIENT_H
