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
class InternalHandler;


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
	Q_PROPERTY(Server *server READ server NOTIFY serverChanged)



public:
	explicit Client(Application *app, QObject *parent = nullptr);
	virtual ~Client();

	QQuickItem *mainStack() const;
	void setMainStack(QQuickItem *newMainStack);

	Q_INVOKABLE QQuickItem *stackPushPage(QString qml, QVariantMap parameters = {}) const;
	Q_INVOKABLE bool stackPop(int index = -1, const bool &forced = false) const;
	Q_INVOKABLE bool stackPop(QQuickItem *page, const bool &forced = false) const;
	Q_INVOKABLE bool stackPopToPage(QQuickItem *page) const;

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


	// Cache

	Q_INVOKABLE qolm::QOlmBase *cache(const QString &key) const
	{
		if (server())
			return server()->cache()->get(key.toUtf8().constData());
		else
			return nullptr;
	}
	Q_INVOKABLE qolm::QOlmBase *cache(const QString &key, const QString &id) const
	{
		if (server())
			return server()->cache()->get(key.toUtf8().constData(), id.toUtf8().constData());
		else
			return nullptr;
	}
	Q_INVOKABLE qolm::QOlmBase *cache(const QString &key, const int &id) const
	{
		if (server())
			return server()->cache()->get(key.toUtf8().constData(), id);
		else
			return nullptr;
	}

	Q_INVOKABLE void setCache(const QString &key, const QJsonArray &list)
	{
		if (server())
			server()->cache()->set(key.toUtf8().constData(), list);
	}
	Q_INVOKABLE void setCache(const QString &key, const QString &id, const QJsonArray &list)
	{
		if (server())
			server()->cache()->set(key.toUtf8().constData(), id.toUtf8().constData(), list);
	}
	Q_INVOKABLE void setCache(const QString &key, const int &id, const QJsonArray &list)
	{
		if (server())
			server()->cache()->set(key.toUtf8().constData(), id, list);
	}


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


	// WebSocket

	WebSocket *webSocket() const;
	Q_INVOKABLE void sendRequest(const WebSocketMessage::ClassHandler &classHandler, const QJsonObject &json);

	Server *server() const;

	Q_INVOKABLE void connectToServer(Server *server);

	void stackPopToStartPage();

	void addMessageHandler(AsyncMessageHandler *handler);
	void removeMessageHandler(AsyncMessageHandler *handler);
	void handleMessage(const WebSocketMessage &message);


	// Login

	Q_INVOKABLE virtual void loginGoogle();
	Q_INVOKABLE virtual void registrationGoogle(const QString &code);

	Q_INVOKABLE virtual void loginPlain(const QString &username, const QString &password);
	Q_INVOKABLE virtual void registrationPlain(const QJsonObject &data);

	Q_INVOKABLE virtual bool loginToken();

	Q_INVOKABLE virtual void logout();

	// Cached lists

	Q_INVOKABLE void loadClassListFromArray(QJsonArray list);

protected slots:
	virtual void onApplicationStarted();
	friend class Application;

	virtual void onWebSocketError(const QAbstractSocket::SocketError &error);
	virtual void onServerConnected();
	virtual void onServerDisconnected();
	virtual void onServerTerminated();
	virtual void onServerReconnected();

	virtual void onUserLoggedIn();
	virtual void onUserLoggedOut();

protected:
	void _message(const QString &text, const QString &title, const QString &type) const;
	virtual bool handleMessageInternal(const WebSocketMessage &message);
	void _userAuthTokenReceived(const QString &token);

signals:
	void startPageLoaded();
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

	qreal m_safeMarginLeft = 0;
	qreal m_safeMarginRight = 0;
	qreal m_safeMarginTop = 0;
	qreal m_safeMarginBottom = 0;

	QVector<QPointer<AsyncMessageHandler>> m_messageHandlers;

	QQuickItem *m_startPage = nullptr;
	QQuickItem *m_mainPage = nullptr;

	InternalHandler *m_internalHandler = nullptr;
	friend class InternalHandler;

};



/**
 * @brief The InternalHandler class
 */

class InternalHandler : public AsyncMessageHandler
{
	Q_OBJECT

public:
	explicit InternalHandler(Client *client) : AsyncMessageHandler(client) {}
	virtual ~InternalHandler() {}

public slots:
	void getConfig(const QJsonObject &json) const;
	void rankList(const QJsonObject &json) const;
	void loginPlain(const QJsonObject &json) const;
	void loginGoogle(const QJsonObject &json) const;
	void loginToken(const QJsonObject &json) const { loginPlain(json); }
	void logout(const QJsonObject &json) const;

	void registrationPlain(const QJsonObject &json) const;
	void registrationGoogle(const QJsonObject &json) const;

	void me(const QJsonObject &json) const;

};

#endif // CLIENT_H
