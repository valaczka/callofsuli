/*
 * ---- Call of Suli ----
 *
 * client.cpp
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

#include <QDebug>

#include "application.h"
#include "client.h"
#include "mapplay.h"
#include "mapplaydemo.h"
#include "qquickwindow.h"
#include "websocket.h"
#include "gameterrain.h"
#include "qquickwindow.h"
#include "qguiapplication.h"
#include <qpa/qplatformwindow.h>
#include "Logger.h"

#ifdef Q_OS_ANDROID
#include "qandroidfunctions.h"
#include "qscreen.h"
#endif

Client::Client(Application *app, QObject *parent)
	: QObject{parent}
	, m_application(app)
	, m_utils(new Utils(this))
	, m_webSocket(new WebSocket(this))
{
	Q_ASSERT(app);

	m_oauthData.timer.setInterval(3000);

	connect(m_webSocket, &WebSocket::socketError, this, &Client::onWebSocketError);
	connect(m_webSocket, &WebSocket::responseError, this, &Client::onWebSocketResponseError);
#ifndef QT_NO_SSL
	connect(m_webSocket, &WebSocket::socketSslErrors, this, &Client::onWebSocketSslError);
#endif
	connect(m_webSocket, &WebSocket::serverConnected, this, &Client::onServerConnected);
	connect(m_webSocket, &WebSocket::serverDisconnected, this, &Client::onServerDisconnected);
	connect(m_webSocket, &WebSocket::serverChanged, this, &Client::serverChanged);

	connect(&m_oauthData.timer, &QTimer::timeout, this, &Client::onOAuthPendingTimer);
}



/**
 * @brief Client::~Client
 */

Client::~Client()
{
	if (m_currentGame)
		delete m_currentGame;

	delete m_utils;
	delete m_webSocket;
}

/**
 * @brief Client::mainStack
 * @return
 */

QQuickItem *Client::mainStack() const
{
	return m_mainStack;
}

/**
 * @brief Client::setMainStack
 * @param newMainStack
 */

void Client::setMainStack(QQuickItem *newMainStack)
{
	if (m_mainStack == newMainStack)
		return;
	m_mainStack = newMainStack;
	emit mainStackChanged();
}




/**
 * @brief Client::addPage
 */

QQuickItem* Client::stackPushPage(QString qml, QVariantMap parameters) const
{
	if (!m_mainStack) {
		LOG_CERROR("client") << "mainStack nincsen beállítva!";
		return nullptr;
	}

	if (qml.isEmpty()) {
		LOG_CWARNING("client") << "Nincs megadva lap";
		return nullptr;
	}

	QQuickItem *o = nullptr;
	QMetaObject::invokeMethod(m_mainStack, "createPage", Qt::DirectConnection,
							  Q_RETURN_ARG(QQuickItem*, o),
							  Q_ARG(QString, qml),
							  Q_ARG(QVariant, parameters)
							  );

	if (!o) {
		LOG_CERROR("client") << "Nem lehet a lapot betölteni:" << qPrintable(qml);
		return nullptr;
	}

	LOG_CDEBUG("client") << "Lap betöltve:" << qPrintable(qml) << o;

	return o;
}






/**
 * @brief Client::stackPop
 * @param depth
 */

bool Client::stackPop(int index, const bool &forced) const
{
	if (!m_mainStack) {
		LOG_CERROR("client") << "mainStack nincsen beállítva!";
		return false;
	}

	QQuickItem *currentItem = qvariant_cast<QQuickItem*>(m_mainStack->property("currentItem"));

	if (!currentItem) {
		LOG_CERROR("client") << "mainStack currentItem unavailable";
		return false;
	}

	const int &depth = m_mainStack->property("depth").toInt();

	if (index == -1) {
		index = depth-2;
	}


	bool canPop = true;

	QMetaObject::invokeMethod(m_mainStack, "callStackPop", Qt::DirectConnection,
							  Q_RETURN_ARG(bool, canPop)
							  );

	if (!canPop)
		return false;


	if (index >= depth) {
		LOG_CWARNING("client") << "Nem lehet a lapra visszalépni:" << index << "mélység:" << depth;
		return false;
	}

	if (depth <= 2) {
#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
		LOG_CDEBUG("client") << "Nem lehet visszalépni, mélység:" << depth;
		//Application::instance()->messageInfo("Nem lehet visszalépni!");
		return false;
#endif

		m_mainWindow->close();
		return true;
	}




	QString closeDisabled = currentItem->property("closeDisabled").toString();
	QString question = currentItem->property("closeQuestion").toString();

	if (!closeDisabled.isEmpty()) {
		messageWarning(closeDisabled);
		return false;
	}

	if (forced || question.isEmpty()) {
		QMetaObject::invokeMethod(m_mainStack, "popPage", Qt::DirectConnection,
								  Q_ARG(int, index)
								  );

		return true;
	}

	LOG_CDEBUG("client") << "Kérdés a visszalépés előtt" << currentItem;

	QMetaObject::invokeMethod(m_mainWindow, "closeQuestion", Qt::DirectConnection,
							  Q_ARG(QString, question),
							  Q_ARG(bool, true),						// _pop
							  Q_ARG(int, index)
							  );

	return false;
}


/**
 * @brief Client::stackPop
 * @param page
 * @param forced
 * @return
 */

bool Client::stackPop(QQuickItem *page, const bool &forced) const
{
	QQmlProperty property(page, "StackView.index", qmlContext(page));
	return stackPop(property.read().toInt()-1, forced);
}


/**
 * @brief Client::stackPopToPage
 * @param page
 * @param forced
 * @return
 */

bool Client::stackPopToPage(QQuickItem *page) const
{
	if (!m_mainStack) {
		LOG_CERROR("client") << "mainStack nincsen beállítva!";
		return false;
	}

	QQuickItem *currentItem = qvariant_cast<QQuickItem*>(m_mainStack->property("currentItem"));

	if (!currentItem) {
		LOG_CERROR("client") << "mainStack currentItem unavailable";
		return false;
	}

	if (currentItem == page)
		return false;

	QQmlProperty propertyCurrent(currentItem, "StackView.index", qmlContext(currentItem));
	QQmlProperty propertyPage(page, "StackView.index", qmlContext(page));

	const int &index = propertyPage.read().toInt();

	bool canPop = true;

	QMetaObject::invokeMethod(m_mainStack, "callStackPop", Qt::DirectConnection,
							  Q_RETURN_ARG(bool, canPop)
							  );

	if (!canPop)
		return false;


	QString closeDisabled = currentItem->property("closeDisabled").toString();

	if (!closeDisabled.isEmpty()) {
		messageWarning(closeDisabled);
		return false;
	}

	QMetaObject::invokeMethod(m_mainStack, "popPage", Qt::DirectConnection,
							  Q_ARG(int, index)
							  );

	return true;
}






/**
 * @brief Client::closeWindow
 * @return
 */

bool Client::closeWindow(const bool &forced)
{
	if (m_mainWindowClosable)
		return true;


	QQuickItem *currentItem = qvariant_cast<QQuickItem*>(m_mainStack->property("currentItem"));

	if (!currentItem) {
		LOG_CERROR("client") << "mainStack currentItem unavailable";
		return false;
	}

	QString closeDisabled = currentItem->property("closeDisabled").toString();
	QString question = currentItem->property("closeQuestion").toString();

	if (!closeDisabled.isEmpty()) {
		messageWarning(closeDisabled);
		return false;
	}


	if (forced || question.isEmpty()) {
		LOG_CDEBUG("client") << "Ablak bezárása";
		m_mainWindowClosable = true;
		m_mainWindow->close();
		return true;
	}

	LOG_CDEBUG("client") << "Kérdés a bezárás előtt" << currentItem;

	QMetaObject::invokeMethod(m_mainWindow, "closeQuestion", Qt::DirectConnection,
							  Q_ARG(QString, question),
							  Q_ARG(bool, false),					// _pop
							  Q_ARG(int, -1)						// _index
							  );

	return false;
}





/**
 * @brief Client::onApplicationStarted
 */

void Client::onApplicationStarted()
{
	LOG_CINFO("client") << "A kliens alkalmazás sikeresen elindult.";

	QCoreApplication::processEvents();

	GameTerrain::reloadAvailableTerrains();
	AbstractLevelGame::reloadAvailableMusic();
	AbstractLevelGame::reloadAvailableMedal();

	m_startPage = stackPushPage(QStringLiteral("PageStart.qml"));

	emit startPageLoaded();
}


/**
 * @brief Client::onWebSocketError
 * @param error
 */

void Client::onWebSocketError(QNetworkReply::NetworkError code)
{
	LOG_CWARNING("client") << "Websocket error:" << code;

	if (m_webSocket->state() == WebSocket::Connected)
		snack(tr("ERROR: %1").arg(code));
	else
		messageError(tr("ERROR: %1").arg(code), tr("Sikertelen csatlakozás"));
	//m_webSocket->close();
}



/**
 * @brief Client::onWebSocketResponseError
 * @param error
 */

void Client::onWebSocketResponseError(const QString &error)
{
	if (error == QLatin1String("unauthorized"))
		messageWarning(tr("A kérés végrehajátáshoz be kell jelentkezni"), tr("Hitelesítés szükséges"));
	else if (error == QLatin1String("permission denied"))
		messageWarning(tr("Hozzáférés megtagadva"), tr("Hitelesítés szükséges"));
}


/**
 * @brief Client::onWebSocketSslError
 * @param errors
 */

#ifndef QT_NO_SSL
void Client::onWebSocketSslError(const QList<QSslError> &errors)
{
	LOG_CWARNING("client") << "Websocket SSL error:" << errors;

	if (m_webSocket->state() == WebSocket::Connected)
		snack(tr("SSL ERROR"));
	else
		messageError(tr("SSL ERROR"), tr("Sikertelen csatlakozás"));
	//m_webSocket->close();
}
#endif


/**
 * @brief Client::onServerConnected
 */

void Client::onServerConnected()
{
	LOG_CINFO("client") << "Server connected:" << m_webSocket->server()->url();

	server()->user()->setLoginState(User::LoggedOut);

	m_mainPage = stackPushPage(QStringLiteral("PageMain.qml"));

	loginToken();


	send(WebSocket::ApiGeneral, QStringLiteral("config"))
			->done([this](const QJsonObject &json)
	{
		if (json.contains(QStringLiteral("name")) && server())
			server()->setServerName(json.value(QStringLiteral("name")).toString());

		if (json.contains(QStringLiteral("config")) && server())
			server()->setConfig(json.value(QStringLiteral("config")).toObject());
	});


	send(WebSocket::ApiGeneral, QStringLiteral("rank"))
			->done([this](const QJsonObject &json)
	{
		server()->setRankList(RankList::fromJson(json.value(QStringLiteral("list")).toArray()));
	});
}


/**
 * @brief Client::onServerDisconnected
 */

void Client::onServerDisconnected()
{
	LOG_CINFO("client") << "Server disconnected";

	m_oauthData.timer.stop();

	if (server())
		server()->user()->setLoginState(User::LoggedOut);

	stackPopToStartPage();
}





/**
 * @brief Client::onOAuthStateChanged
 */

void Client::onOAuthLoginStateChanged(const QJsonObject &json)
{
	LOG_CTRACE("client") << "OAuth login state changed" << json;

	if (json.contains(QStringLiteral("auth_token")) && json.value(QStringLiteral("status")).toString() == QStringLiteral("ok")) {
		m_oauthData.status = OAuthData::Success;
		m_oauthData.timer.stop();
		onOAuthFinished();
		_userAuthTokenReceived(json.value(QStringLiteral("auth_token")).toString());
		return;
	}

	if (json.value(QStringLiteral("pending")).toBool()) {
		if (m_oauthData.status == OAuthData::UrlReceived || m_oauthData.status == OAuthData::Pending) {
			m_oauthData.status = OAuthData::Pending;
		} else {
			LOG_CWARNING("client") << "OAuth login protocol warning" << m_oauthData.status;
		}
	} else if (json.contains(QStringLiteral("state")) && (m_oauthData.isLocal || json.contains(QStringLiteral("url")))) {
		if (m_oauthData.status == OAuthData::Invalid) {
			m_oauthData.state = json.value(QStringLiteral("state")).toString();
			m_oauthData.status = OAuthData::UrlReceived;

			if (m_oauthData.isLocal)
				return prepareOAuth(json);

			onOAuthStarted(QUrl::fromEncoded(json.value(QStringLiteral("url")).toString().toUtf8()));
		} else {
			LOG_CWARNING("client") << "OAuth login protocol warning" << m_oauthData.status;
		}
	}

}



/**
 * @brief Client::onUserLoggedIn
 */

void Client::onUserLoggedIn()
{
	LOG_CINFO("client") << "User logged in:" << qPrintable(server()->user()->username());

	send(WebSocket::ApiGeneral, "me")->done([this](const QJsonObject &json){
		LOG_CINFO("client") << "User logged in:" << json;
		server()->user()->loadFromJson(json);
		stackPushPage(QStringLiteral("PageDashboard.qml"));
	});

}


/**
 * @brief Client::onUserLoggedOut
 */

void Client::onUserLoggedOut()
{
	LOG_CINFO("client") << "User logged out:" << qPrintable(server()->user()->username());

	server()->setToken(QLatin1String(""));
	server()->user()->clear();

	if (m_mainPage)
		stackPopToPage(m_mainPage);
}


/**
 * @brief Client::onOAuthFinished
 */

void Client::onOAuthFinished()
{
	LOG_CTRACE("client") << "OAuth finished";

	m_oauthData.status = OAuthData::Invalid;
	m_oauthData.state = "";
	m_oauthData.path = "";
	m_oauthData.type = OAuthData::Login;
	m_oauthData.timer.stop();
}


/**
 * @brief Client::onOAuthStarted
 */

void Client::onOAuthStarted(const QUrl &url)
{
	LOG_CTRACE("client") << "OAuth started:" << url;
	Utils::openUrl(url);
	m_oauthData.status = OAuthData::Pending;
	m_oauthData.timer.start();
}


/**
 * @brief Client::onOAuthPendingTimer
 */

void Client::onOAuthPendingTimer()
{
	LOG_CTRACE("client") << "OAuth pending timer timeout";

	if (m_oauthData.state.isEmpty() || m_oauthData.status != OAuthData::Pending) {
		LOG_CWARNING("client") << "OAuth pending timer timeout invalid state" << m_oauthData.status;
		return;
	}

	if (m_oauthData.path.isEmpty()) {
		LOG_CWARNING("client") << "OAuth pending timer empty path";
		return;
	}


	QJsonObject j;
	j.insert(QStringLiteral("state"), m_oauthData.state);
	if (m_oauthData.isLocal)
		j.insert(QStringLiteral("local"), true);

	if (m_oauthData.isLocal)
		LOG_CTRACE("client") << "LOCAL";


	send(WebSocket::ApiAuth, m_oauthData.path, j)
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);

}



/**
 * @brief Client::onLoginResponse
 */

void Client::onLoginSuccess(const QJsonObject &json)
{
	LOG_CTRACE("client") << "On login success" << json;

	if (json.contains(QStringLiteral("auth_token")) && json.value(QStringLiteral("status")).toString() == QStringLiteral("ok")) {
		if (m_oauthData.status != OAuthData::Invalid)
			onOAuthLoginStateChanged(json);
		else
			_userAuthTokenReceived(json.value(QStringLiteral("auth_token")).toString());
		return;
	}

	if (json.contains(QStringLiteral("state")) || json.contains(QStringLiteral("url")) ||
			json.contains(QStringLiteral("pending"))) {
		onOAuthLoginStateChanged(json);
		return;
	}

}


/**
 * @brief Client::onLoginFailed
 * @param error
 */

void Client::onLoginFailed(const QString &error)
{
	m_oauthData.timer.stop();
	if (m_oauthData.status != OAuthData::Invalid) {
		m_oauthData.status = OAuthData::Success;
		onOAuthFinished();
	}

	QString errorStr = error;
	if (error == QLatin1String("invalid user"))
		errorStr = tr("Érvénytelen felhasználó");
	else if (error == QLatin1String("authorization failed"))
		errorStr = tr("Hibás felhasználónév/jelszó");
	else if (error == QLatin1String("invalid token"))
		errorStr = tr("Érvénytelen token");
	else if (error == QLatin1String("invalid state"))
		errorStr = tr("Hibás OAuth2 kérés");
	else if (error == QLatin1String("authentication failed"))
		errorStr = tr("A felhasználó nem azonosítható");

	messageWarning(errorStr, tr("Sikertelen bejelentkezés"));
	server()->user()->setLoginState(User::LoggedOut);
}






/**
 * @brief Client::_message
 * @param text
 * @param title
 * @param icon
 */

void Client::_message(const QString &text, const QString &title, const QString &type) const
{
	if (m_mainWindow) {
		QMetaObject::invokeMethod(m_mainWindow, "messageDialog", Qt::DirectConnection,
								  Q_ARG(QString, text),
								  Q_ARG(QString, title),
								  Q_ARG(QString, type)
								  );
	}
}




/**
 * @brief Client::_userAuthTokenReceived
 * @param token
 */

void Client::_userAuthTokenReceived(const QString &token)
{
	const Credential &c = Credential::fromJWT(token);

	server()->setToken(token);
	server()->user()->setUsername(c.username());
	server()->user()->setRoles(c.roles());
	server()->user()->setLoginState(User::LoggedIn);

	onUserLoggedIn();
}






/**
 * @brief Client::connectToServer
 * @param server
 */

void Client::connectToServer(Server *server)
{
	if (!server) {
		messageError(tr("Nincs megadva szerver"));
		return;
	}

	if (server->url().isEmpty()) {
		messageError(tr("A szerver URL címe nincs beállítva!"));
		return;
	}

	m_webSocket->connectToServer(server);
}


/**
 * @brief Client::stackPopToStartPage
 */

void Client::stackPopToStartPage()
{
	if (m_startPage)
		stackPopToPage(m_startPage);
}





/**
 * @brief Client::webSocket
 * @return
 */

WebSocket *Client::webSocket() const
{
	return m_webSocket;
}



/**
 * @brief Client::send
 * @return
 */

WebSocketReply *Client::send(const WebSocket::API &api, const QString &path, const QJsonObject &data) const
{
	return m_webSocket->send(api, path, data);
}




/**
 * @brief Client::server
 * @return
 */

Server *Client::server() const
{
	return m_webSocket->server();
}




/**
 * @brief Client::loginGoogle
 */

void Client::loginGoogle()
{
	server()->user()->setLoginState(User::LoggingIn);

	m_oauthData.status = OAuthData::Invalid;
	if (m_oauthData.timer.isActive())
		m_oauthData.timer.stop();
	m_oauthData.type = OAuthData::Login;
	m_oauthData.state = "";
	m_oauthData.path = QStringLiteral("login/google");

	QJsonObject j;
	if (m_oauthData.isLocal)
		j.insert(QStringLiteral("local"), true);

	send(WebSocket::ApiAuth, m_oauthData.path, j)
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);
}


/**
 * @brief Client::registrationGoogle
 */

void Client::registrationGoogle(const QString &code)
{
	server()->user()->setLoginState(User::LoggingIn);

	m_oauthData.status = OAuthData::Invalid;
	if (m_oauthData.timer.isActive())
		m_oauthData.timer.stop();
	m_oauthData.type = OAuthData::Registration;
	m_oauthData.state = "";
	m_oauthData.path = QStringLiteral("registration/google");

	QJsonObject j;
	if (m_oauthData.isLocal)
		j.insert(QStringLiteral("local"), true);
	j.insert(QStringLiteral("code"), code);

	send(WebSocket::ApiAuth, m_oauthData.path, j)
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);
}


/**
 * @brief Client::loginPlain
 * @param username
 * @param password
 */

void Client::loginPlain(const QString &username, const QString &password)
{
	server()->user()->setLoginState(User::LoggingIn);

	send(WebSocket::ApiAuth, QStringLiteral("login"),
		 QJsonObject{
			 { QStringLiteral("username"), username },
			 { QStringLiteral("password"), password }
		 })
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);

}





/**
 * @brief Client::registrationPlain
 */

void Client::registrationPlain(const QJsonObject &data)
{
	send(WebSocket::ApiAuth, QStringLiteral("registration"), data)
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);
}




/**
 * @brief Client::loginToken
 * @param token
 */

bool Client::loginToken()
{
	const QString &token = server()->token();

	if (token.isEmpty()) {
		LOG_CTRACE("client") << "No auth token";
		return false;
	}

	QJsonWebToken jwt;
	if (!jwt.setToken(token)) {
		LOG_CWARNING("credential") << "Invalid token:" << token;
		server()->setToken(QLatin1String(""));
		return false;
	}


	if (jwt.getPayloadJDoc().object().value(QStringLiteral("exp")).toInt() <= QDateTime::currentSecsSinceEpoch()) {
		LOG_CINFO("client") << "Token expired";
		server()->setToken(QLatin1String(""));
		return false;
	}

	send(WebSocket::ApiAuth, QStringLiteral("login"),
		 QJsonObject{
			 { QStringLiteral("token"), token },
		 })
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);

	return true;
}




/**
 * @brief Client::logout
 */

void Client::logout()
{
	LOG_CDEBUG("client") << "Logout";
	onUserLoggedOut();
}


/**
 * @brief Client::loadClassListFromArray
 * @param list
 */

void Client::loadClassListFromArray(QJsonArray list)
{
	LOG_CTRACE("client") << "Refresh class list";

	list.append(QJsonObject({
								{ QStringLiteral("id"), -1 },
								{ QStringLiteral("name"), tr("Mind")}
							}));

	list.append(QJsonObject({
								{ QStringLiteral("id"), -10 },
								{ QStringLiteral("name"), tr("Osztály nélkül")}
							}));

	setCache(QStringLiteral("classList"), list);
}





qreal Client::safeMarginBottom() const
{
	return m_safeMarginBottom;
}

void Client::setSafeMarginBottom(qreal newSafeMarginBottom)
{
	if (qFuzzyCompare(m_safeMarginBottom, newSafeMarginBottom))
		return;
	m_safeMarginBottom = newSafeMarginBottom;
	emit safeMarginBottomChanged();
}


/**
 * @brief Client::setSafeMargins
 * @param margins
 */

void Client::setSafeMargins(const QMarginsF &margins)
{
	setSafeMarginLeft(margins.left());
	setSafeMarginRight(margins.right());
	setSafeMarginTop(margins.top());
	setSafeMarginBottom(margins.bottom());
}



qreal Client::safeMarginTop() const
{
	return m_safeMarginTop;
}

void Client::setSafeMarginTop(qreal newSafeMarginTop)
{
	if (qFuzzyCompare(m_safeMarginTop, newSafeMarginTop))
		return;
	m_safeMarginTop = newSafeMarginTop;
	emit safeMarginTopChanged();
}

qreal Client::safeMarginRight() const
{
	return m_safeMarginRight;
}

void Client::setSafeMarginRight(qreal newSafeMarginRight)
{
	if (qFuzzyCompare(m_safeMarginRight, newSafeMarginRight))
		return;
	m_safeMarginRight = newSafeMarginRight;
	emit safeMarginRightChanged();
}

qreal Client::safeMarginLeft() const
{
	return m_safeMarginLeft;
}

void Client::setSafeMarginLeft(qreal newSafeMarginLeft)
{
	if (qFuzzyCompare(m_safeMarginLeft, newSafeMarginLeft))
		return;
	m_safeMarginLeft = newSafeMarginLeft;
	emit safeMarginLeftChanged();
}



/**
 * @brief Client::safeMarginsGet
 */

void Client::safeMarginsGet()
{
	QMarginsF margins;

#if !defined (Q_OS_ANDROID)
	QPlatformWindow *platformWindow = m_mainWindow->handle();
	if(!platformWindow) {
		qCWarning(lcUtils).noquote() << tr("Invalid QPlatformWindow");
		return;
	}
	margins = platformWindow->safeAreaMargins();
#else
	static const double devicePixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();

	QAndroidJniObject rect = QtAndroid::androidActivity().callObjectMethod<jobject>("getSafeArea");

	const double left = static_cast<double>(rect.getField<jint>("left"));
	const double top = static_cast<double>(rect.getField<jint>("top"));
	const double right = static_cast<double>(rect.getField<jint>("right"));
	const double bottom = static_cast<double>(rect.getField<jint>("bottom"));

	margins.setTop(top/devicePixelRatio);
	margins.setBottom(bottom/devicePixelRatio);
	margins.setLeft(left/devicePixelRatio);
	margins.setRight(right/devicePixelRatio);
#endif

	LOG_CDEBUG("client") << "New safe margins:" << margins;

	setSafeMargins(margins);
}



/**
 * @brief Client::application
 * @return
 */

Application *Client::application() const
{
	return m_application;
}


/**
 * @brief Client::utils
 * @return
 */

Utils *Client::utils() const
{
	return m_utils;
}








/**
 * @brief Client::mainWindow
 * @return
 */

QQuickWindow *Client::mainWindow() const
{
	return m_mainWindow;
}


/**
 * @brief Client::setMainWindow
 * @param newMainWindow
 */

void Client::setMainWindow(QQuickWindow *newMainWindow)
{
	if (m_mainWindow == newMainWindow)
		return;

	m_mainWindow = newMainWindow;
	emit mainWindowChanged();

	if (!m_mainWindow)
		return;

	m_mainWindow->setIcon(QIcon(QStringLiteral(":/internal/img/cos.png")));

	safeMarginsGet();
}


AbstractGame *Client::currentGame() const
{
	return m_currentGame;
}


void Client::setCurrentGame(AbstractGame *newCurrentGame)
{
	if (m_currentGame == newCurrentGame)
		return;
	m_currentGame = newCurrentGame;
	emit currentGameChanged();
}


/**
 * @brief Client::messageInfo
 * @param text
 * @param title
 */

void Client::messageInfo(const QString &text, QString title) const
{
	if (title.isEmpty())
		title = m_application->application()->applicationDisplayName();

	LOG_CINFO("client") << qPrintable(text) << title;
	_message(text, title, QStringLiteral("info"));
}


/**
 * @brief Client::messageWarning
 * @param text
 * @param title
 */

void Client::messageWarning(const QString &text, QString title) const
{
	if (title.isEmpty())
		title = m_application->application()->applicationDisplayName();

	LOG_CWARNING("client") << qPrintable(text) << title;
	_message(text, title, QStringLiteral("warning"));
}


/**
 * @brief Client::messageError
 * @param text
 * @param title
 */

void Client::messageError(const QString &text, QString title) const
{
	if (title.isEmpty())
		title = m_application->application()->applicationDisplayName();

	LOG_CERROR("client") << qPrintable(text) << title;
	_message(text, title, QStringLiteral("error"));
}


/**
 * @brief Client::snack
 * @param text
 */

void Client::snack(const QString &text) const
{
	if (m_mainWindow) {
		QMetaObject::invokeMethod(m_mainWindow, "snack", Qt::DirectConnection,
								  Q_ARG(QString, text)
								  );
	}
}





/**
 * @brief Client::loadDemoMap
 */

void Client::loadDemoMap()
{
	if (m_currentGame) {
		LOG_CERROR("client") << "Game already exists";
		return;
	}

	MapPlayDemo *mapPlay = new MapPlayDemo(this);

	if (!mapPlay->load()) {
		delete mapPlay;
		return;
	}


	QQuickItem *page = stackPushPage(QStringLiteral("PageMapPlay.qml"), QVariantMap({
																						{ QStringLiteral("title"), tr("Demó pálya") },
																						{ QStringLiteral("map"), QVariant::fromValue(mapPlay) }
																					}));

	if (!page) {
		messageError(tr("Nem lehet betölteni a demó oldalt!"));
		delete mapPlay;
		return;
	}

	connect(page, &QQuickItem::destroyed, mapPlay, &MapPlay::deleteLater);
}




/**
 * @brief Client::debug
 * @return
 */

bool Client::debug() const
{
	return m_application->debug();
}




/*



void InternalHandler::registrationGoogle(const QJsonObject &json) const
{
	if (json.contains(QStringLiteral("url"))) {
		Utils::openUrl(QUrl::fromEncoded(json.value(QStringLiteral("url")).toString().toUtf8()));
	} else {
		registrationPlain(json);
	}
}




*/
