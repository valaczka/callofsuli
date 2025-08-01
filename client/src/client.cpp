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
#include "classobject.h"
#include "client.h"
#include "exam.h"
#include "mapplay.h"
#include "mapplaydemo.h"
#include "pass.h"
#include "qquickwindow.h"
#include "rpguserwallet.h"
#include "studentgroup.h"
#include "teachergroup.h"
#include "httpconnection.h"
#include "qquickwindow.h"
#include "qapplication.h"
#include <qpa/qplatformwindow.h>
#include "Logger.h"
#include "mapgame.h"
#include "updater.h"
#include "server.h"
#include "rpgplayer.h"
#include "rpggame.h"
#include <QScreen>

#ifdef Q_OS_ANDROID
#include "qscreen.h"
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include "mobileutils.h"
#endif


Client::Client(Application *app)
	: QObject{}
	, m_application(app)
	, m_utils(new Utils(this))
	, m_httpConnection(new HttpConnection(this))
	, m_updater(new Updater(this))
	, m_downloader(new Downloader)
	, m_contextHelper(new ContextHelper(this))
	, m_sound(new Sound(nullptr))
{
	Q_ASSERT(app);

	m_oauthData.timer.setInterval(3000);

	connect(m_httpConnection.get(), &HttpConnection::socketError, this, &Client::onHttpConnectionError);
	connect(m_httpConnection.get(), &HttpConnection::responseError, this, &Client::onHttpConnectionResponseError);
#ifndef QT_NO_SSL
	connect(m_httpConnection.get(), &HttpConnection::socketSslErrors, this, &Client::onHttpConnectionSslError);
#endif
	connect(m_httpConnection.get(), &HttpConnection::serverConnected, this, &Client::onServerConnected);
	connect(m_httpConnection.get(), &HttpConnection::serverDisconnected, this, &Client::onServerDisconnected);
	connect(m_httpConnection.get(), &HttpConnection::serverChanged, this, &Client::serverChanged);

	connect(&m_oauthData.timer, &QTimer::timeout, this, &Client::onOAuthPendingTimer);

	connect(m_downloader.get(), &Downloader::contentDownloaded, this, &RpgGame::reloadTerrains);
	connect(m_downloader.get(), &Downloader::contentDownloaded, this, &RpgGame::reloadCharacters);
	connect(m_downloader.get(), &Downloader::contentDownloaded, this, &RpgGame::reloadWorld);

	startCache();

	retranslate(Utils::settingsGet(QStringLiteral("window/language"), QStringLiteral("hu")).toString());

	LOG_CTRACE("app") << "Client created" << this;
}



/**
 * @brief Client::~Client
 */

Client::~Client()
{
	LOG_CTRACE("app") << "Client destroying" << this;

	m_cache.removeAll();

	if (m_translator) {
		LOG_CTRACE("app") << "Remove translator";
		/// TODO: Utils::settingsSet(QStringLiteral("window/language"), m_translator->language());
		m_application->application()->removeTranslator(m_translator.get());
	}

	m_sound.reset();

	LOG_CTRACE("app") << "Client destroyed" << this;
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
		messageError(tr("Nem lehet a lapot betölteni!"), tr("Belső hiba"));
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
	/*QInputMethod *im = QApplication::inputMethod();

	if (im && im->isVisible()) {
		LOG_CDEBUG("client") << "Hide input method";
		im->hide();
		return false;
	}*/

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


	LOG_CTRACE("client") << "Stack pop" << index << depth;

	if (index >= depth-1) {
		LOG_CWARNING("client") << "Nem lehet a lapra visszalépni:" << index << "mélység:" << depth;
		return false;
	}

	bool canPop = true;

	QMetaObject::invokeMethod(m_mainStack, "callStackPop", Qt::DirectConnection,
							  Q_RETURN_ARG(bool, canPop)
							  );

	if (!canPop && !forced)
		return false;



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

	if (!closeDisabled.isEmpty() && !forced) {
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

bool Client::stackPop(QQuickItem *page) const
{
	QQmlProperty property(page, "StackView.index", qmlContext(page));
	return stackPop(property.read().toInt()-1, true);
}


/**
 * @brief Client::stackPopToPage
 * @param page
 * @param forced
 * @return
 */

bool Client::stackPopToPage(QQuickItem *page) const
{
	QQmlProperty property(page, "StackView.index", qmlContext(page));
	return stackPop(property.read().toInt(), true);
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
 * @brief Client::notifyWindow
 */

void Client::notifyWindow()
{
	LOG_CDEBUG("client") << "Notify window";

	if (!m_mainWindow)
		return;

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	if (m_mainWindow->visibility() == QWindow::Minimized)
		m_mainWindow->show();
#endif

	m_mainWindow->raise();
	m_mainWindow->alert(0);
	m_mainWindow->requestActivate();
}





/**
 * @brief Client::onApplicationStarted
 */

void Client::onApplicationStarted()
{
	LOG_CINFO("client") << "A kliens alkalmazás sikeresen elindult.";

	QCoreApplication::processEvents();

	AbstractLevelGame::reloadAvailableMusic();
	AbstractLevelGame::reloadAvailableMedal();

	switch (m_application->commandLine()) {
		case Application::Demo:
			loadDemoMap();
			break;
		case Application::Editor:
			stackPushPage(QStringLiteral("PageMapEditor.qml"));
			break;
		case Application::Map:
			stackPushPage(QStringLiteral("PageMapEditor.qml"), {
							  { QStringLiteral("loadFile"), QUrl::fromLocalFile(m_application->commandLineData()) }
						  });
			break;
		case Application::Play:
			loadDemoMap(QUrl::fromLocalFile(m_application->commandLineData()));
			break;
		case Application::DevPage:
			stackPushPage(QStringLiteral("_PageDev.qml"));
			break;
		default:
			m_startPage = stackPushPage(QStringLiteral("PageStart.qml"));
			emit startPageLoaded();
			break;
	}

	m_updater->checkAvailableUpdates(false);
	m_contextHelper->download();

}



/**
 * @brief Client::onHttpConnectionError
 * @param error
 */

void Client::onHttpConnectionError(const QNetworkReply::NetworkError &code)
{
	LOG_CWARNING("client") << "HttpConnection error:" << code;

	QString errStr;
	bool closeSocket = false;

	switch (code) {
		case QNetworkReply::ConnectionRefusedError:
			errStr = tr("A szerver nem elérhető");
			closeSocket = true;
			break;
		case QNetworkReply::RemoteHostClosedError:
			errStr = tr("A szerver lezárta a kapcsolatot");
			closeSocket = true;
			break;
		case QNetworkReply::HostNotFoundError:
			errStr = tr("A szerver nem található");
			closeSocket = true;
			break;
		case QNetworkReply::OperationCanceledError:
			//errStr = tr("A művelet megszakadt");
			return;
			break;
		case QNetworkReply::SslHandshakeFailedError:
			//errStr = tr("Az SSL tanúsítvány hibás");
			return;
			break;

		default:
			errStr = QString::fromStdString(Utils::enumToString<QNetworkReply::NetworkError>(code));
	}


	if (m_httpConnection->state() == HttpConnection::Connecting && closeSocket)
		m_httpConnection->abort();

	if (m_httpConnection->state() == HttpConnection::Connected)
		snack(errStr);
	else
		messageError(errStr, tr("Sikertelen csatlakozás"));
	//m_httpConnection->close();
}



/**
 * @brief Client::onHttpConnectionResponseError
 * @param error
 */

void Client::onHttpConnectionResponseError(const QString &error)
{
	if (error == QStringLiteral("unauthorized"))
		messageWarning(tr("A kérés végrehajátáshoz be kell jelentkezni"), tr("Hitelesítés szükséges"));
	else if (error == QStringLiteral("permission denied"))
		messageWarning(tr("Hozzáférés megtagadva"), tr("Hitelesítés szükséges"));
}


/**
 * @brief Client::onHttpConnectionSslError
 * @param errors
 */

#ifndef QT_NO_SSL
void Client::onHttpConnectionSslError(const QList<QSslError> &errors)
{
	LOG_CWARNING("client") << "Httpconnection SSL error:" << errors;

	QStringList errList;

	foreach (const QSslError &err, errors)
		errList.append(err.errorString());


	if (m_httpConnection->state() == HttpConnection::Connected)
		snack(errList.join(QStringLiteral(", ")));
	else
		messageError(errList.join(QStringLiteral(", ")), tr("Sikertelen csatlakozás"));
	//m_httpConnection->close();
}
#endif


/**
 * @brief Client::onServerConnected
 */

void Client::onServerConnected()
{
	if (!server()) {
		LOG_CWARNING("client") << "Server disconnected";
		return;
	}

	LOG_CINFO("client") << "Server connected:" << m_httpConnection->server()->url();

	if (server()->isStatic()) {
		initializeDynamicResources();
		return;
	}

	server()->user()->setLoginState(User::LoggedOut);

	m_mainPage = stackPushPage(QStringLiteral("PageMain.qml"));

	send(HttpConnection::ApiGeneral, QStringLiteral("config"))
			->done(this, [this](const QJsonObject &json)
	{
		if (!server())
			return;

		if (json.contains(QStringLiteral("name")) && server())
			server()->setServerName(json.value(QStringLiteral("name")).toString());

		if (json.contains(QStringLiteral("config")) && server())
			server()->setConfig(json.value(QStringLiteral("config")).toObject());

		if (json.contains(QStringLiteral("uploadLimit")) && server())
			server()->setMaxUploadSize(json.value(QStringLiteral("uploadLimit")).toInt());


		if (server()->temporary())
			emit serverSetAutoConnectRequest(server());

		server()->setTemporary(false);

		parseUrl();

		send(HttpConnection::ApiGeneral, QStringLiteral("rank"))
				->done(this, [this](const QJsonObject &json)
		{
			if (!server())
				return;

			server()->setRankList(RankList::fromJson(json.value(QStringLiteral("list")).toArray()));
			loginToken();
		});

		reloadCache(QStringLiteral("gradeList"));

		initializeDynamicResources();
	})
			->fail(this, [this](const QString &err){
		LOG_CWARNING("client") << "Server hello failed:" << qPrintable(err);
		m_httpConnection->close();
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

	m_cache.clear();

	if (server())
		server()->user()->wallet()->unloadWorld();

	m_downloader->contentClear();
	m_downloader->setServer(nullptr);
	RpgGame::reloadTerrains();
	RpgGame::reloadCharacters();
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
		_userAuthTokenReceived(json.value(QStringLiteral("auth_token")).toString().toUtf8());
		return;
	}

	if (json.value(QStringLiteral("pending")).toVariant().toBool()) {
		if (m_oauthData.status == OAuthData::UrlReceived || m_oauthData.status == OAuthData::Pending) {
			m_oauthData.status = OAuthData::Pending;
		} else {
			LOG_CWARNING("client") << "OAuth login protocol warning" << m_oauthData.status;
		}
	} else if (json.contains(QStringLiteral("state")) && json.contains(QStringLiteral("url"))) {
		if (m_oauthData.status == OAuthData::Invalid) {
			m_oauthData.state = json.value(QStringLiteral("state")).toString();
			m_oauthData.status = OAuthData::UrlReceived;

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

	send(HttpConnection::ApiGeneral, "me")->done(this, [this](const QJsonObject &json){
		if (!server())
			return;

		server()->user()->loadFromJson(json);
		server()->user()->wallet()->reload();

		if (server()->user()->roles().testFlag(Credential::Panel))
			stackPushPage(QStringLiteral("PagePanel.qml"));
		else if (server()->user()->roles().testFlag(Credential::Teacher) || server()->user()->roles().testFlag(Credential::Admin))
			stackPushPage(QStringLiteral("PageTeacherDashboard.qml"));
		else
			stackPushPage(QStringLiteral("PageStudentDashboard.qml"));
	});
}


/**
 * @brief Client::onUserLoggedOut
 */

void Client::onUserLoggedOut()
{
	LOG_CINFO("client") << "User logged out:" << qPrintable(server()->user()->username());

	server()->setToken(QString());
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


	send(HttpConnection::ApiAuth, m_oauthData.path, j)
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
			_userAuthTokenReceived(json.value(QStringLiteral("auth_token")).toString().toUtf8());
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
	if (error == QStringLiteral("invalid user"))
		errorStr = tr("Érvénytelen felhasználó");
	else if (error == QStringLiteral("authorization failed"))
		errorStr = tr("Hibás felhasználónév/jelszó");
	else if (error == QStringLiteral("invalid token"))
		errorStr = tr("Érvénytelen token");
	else if (error == QStringLiteral("invalid state"))
		errorStr = tr("Hibás OAuth2 kérés");
	else if (error == QStringLiteral("invalid code"))
		errorStr = tr("Érvénytelen hitelesítő kód");
	else if (error == QStringLiteral("invalid domain"))
		errorStr = tr("Érvénytelen email cím (domain)");
	else if (error == QStringLiteral("authentication failed"))
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

void Client::_userAuthTokenReceived(const QByteArray &token)
{
	const Credential &c = Credential::fromJWT(token);

	server()->setToken(token);
	server()->user()->setUsername(c.username());
	server()->user()->setRoles(c.roles());
	server()->user()->setLoginState(User::LoggedIn);

	onUserLoggedIn();
}



/**
 * @brief Client::fullScreenHelperConnect
 * @param window
 */

void Client::fullScreenHelperConnect(QQuickWindow *window)
{
	if (!window)
		return;

	connect(window, &QQuickWindow::visibilityChanged, this, &Client::fullScreenHelperChanged);
}


/**
 * @brief Client::fullScreenHelperDisconnect
 * @param window
 */

void Client::fullScreenHelperDisconnect(QQuickWindow *window)
{
	if (!window)
		return;

	disconnect(window, &QQuickWindow::visibilityChanged, this, &Client::fullScreenHelperChanged);
}


/**
 * @brief Client::initializeDynamicResources
 */

void Client::initializeDynamicResources()
{
	LOG_CDEBUG("client") << "Initialize dynamic resources";

	if (!server())
		return;

	m_downloader->contentClear();
	m_downloader->setServer(server());

	send(HttpConnection::ApiGeneral, QStringLiteral("content"))
			->done(this, [this](const QJsonObject &json)
	{
		if (!server())
			return;

		const QJsonArray &list = json.value(QStringLiteral("list")).toArray();

		for (const QJsonValue &v : list) {
			const QJsonObject &o = v.toObject();

			Server::DynamicContent content;
			content.name = o.value(QStringLiteral("file")).toString();
			content.md5 = o.value(QStringLiteral("md5")).toString();
			content.size = o.value(QStringLiteral("size")).toInteger();

			m_downloader->contentAdd(content);
		}

	})
			->fail(this, [this](const QString &err){
		messageError(tr("Dynamic content download failed:\n").append(err));
	});
}





/**
 * @brief Client::startCache
 */

void Client::startCache()
{
	m_cache.add<User>(QStringLiteral("scoreList"), std::move(new UserList(this)),
					  &OlmLoader::loadFromJsonArray<User>,
					  &OlmLoader::find<User>,
					  "username", "username", true,
					  HttpConnection::ApiGeneral, "score");

	m_cache.add<Grade>(QStringLiteral("gradeList"), std::move(new GradeList(this)),
					   &OlmLoader::loadFromJsonArray<Grade>,
					   &OlmLoader::find<Grade>,
					   "id", "gradeid", false,
					   HttpConnection::ApiGeneral, "grade");

	m_cache.add<ClassObject>(QStringLiteral("classList"), std::move(new ClassList(this)),
							 &OlmLoader::loadFromJsonArray<ClassObject>,
							 &OlmLoader::find<ClassObject>,
							 "id", "classid", true,
							 HttpConnection::ApiGeneral, "class");

	m_cache.add<StudentGroup>(QStringLiteral("studentGroupList"), std::move(new StudentGroupList(this)),
							  &OlmLoader::loadFromJsonArray<StudentGroup>,
							  &OlmLoader::find<StudentGroup>,
							  "id", "groupid", false,
							  HttpConnection::ApiUser, "group");

	m_cache.add<Campaign>(QStringLiteral("studentCampaignList"), std::move(new CampaignList(this)),
						  &OlmLoader::loadFromJsonArray<Campaign>,
						  &OlmLoader::find<Campaign>,
						  "id", "campaignid", false,
						  HttpConnection::ApiUser, "campaign");

	m_cache.add<Pass>(QStringLiteral("passList"), std::move(new PassList(this)),
						  &OlmLoader::loadFromJsonArray<Pass>,
						  &OlmLoader::find<Pass>,
						  "id", "passid", false,
						  HttpConnection::ApiUser, "pass");

	m_cache.add<TeacherGroup>(QStringLiteral("teacherGroupList"), std::move(new TeacherGroupList(this)),
							  &OlmLoader::loadFromJsonArray<TeacherGroup>,
							  &OlmLoader::find<TeacherGroup>,
							  "id", "groupid", false,
							  HttpConnection::ApiTeacher, "group");


	m_cache.addHandler<User>(QStringLiteral("user"), &OlmLoader::loadFromJsonArray<User>, &OlmLoader::find<User>);
	m_cache.addHandler<MapGame>(QStringLiteral("mapGame"), &OlmLoader::loadFromJsonArray<MapGame>, &OlmLoader::find<MapGame>);
	m_cache.addHandler<Exam>(QStringLiteral("exam"), &OlmLoader::loadFromJsonArray<Exam>, &OlmLoader::find<Exam>, "id", "examId");
	m_cache.addHandler<Pass>(QStringLiteral("pass"), &OlmLoader::loadFromJsonArray<Pass>, &OlmLoader::find<Pass>, "id", "passid");
}



/**
 * @brief Client::sound
 * @return
 */

Sound* Client::sound() const
{
	return m_sound.get();
}


/**
 * @brief Client::setSound
 * @param newSound
 */

void Client::setSound(std::unique_ptr<Sound> newSound)
{
	if (m_sound == newSound)
		return;
	m_sound = std::move(newSound);
	emit soundChanged();
}


/**
 * @brief Client::updater
 * @return
 */

Updater *Client::updater() const
{
	return m_updater.get();
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

	m_httpConnection->connectToServer(server);
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
 * @brief Client::HttpConnection
 * @return
 */

HttpConnection *Client::httpConnection() const
{
	return m_httpConnection.get();
}



/**
 * @brief Client::send
 * @return
 */

HttpReply *Client::send(const HttpConnection::API &api, const QString &path, const QJsonObject &data) const
{
	return m_httpConnection->send(api, path, data);
}




/**
 * @brief Client::server
 * @return
 */

Server *Client::server() const
{
	return m_httpConnection->server();
}


/**
 * @brief Client::serverAddWithUrl
 * @param url
 * @return
 */

Server *Client::serverAddWithUrl(const QUrl &url)
{
	Q_UNUSED(url);
	LOG_CWARNING("client") << "Missing implementation";
	return nullptr;
}




/**
 * @brief Client::loginGoogle
 */

void Client::loginOAuth2(const QString &provider)
{
	server()->user()->setLoginState(User::LoggingIn);

	m_oauthData.status = OAuthData::Invalid;
	if (m_oauthData.timer.isActive())
		m_oauthData.timer.stop();
	m_oauthData.type = OAuthData::Login;
	m_oauthData.state = "";
	m_oauthData.path = QStringLiteral("login/")+provider;


	send(HttpConnection::ApiAuth, m_oauthData.path, {
		 #ifdef Q_OS_WASM
			 { QStringLiteral("wasm"), true }
		 #endif
		 })
			->done(this, &Client::onLoginSuccess)
			->fail(this, &Client::onLoginFailed);
}


/**
 * @brief Client::registrationGoogle
 */

void Client::registrationOAuth2(const QString &provider, const QString &code)
{
	server()->user()->setLoginState(User::LoggingIn);

	m_oauthData.status = OAuthData::Invalid;
	if (m_oauthData.timer.isActive())
		m_oauthData.timer.stop();
	m_oauthData.type = OAuthData::Registration;
	m_oauthData.state = "";
	m_oauthData.path = QStringLiteral("registration/")+provider;


	send(HttpConnection::ApiAuth, m_oauthData.path, {
		 #ifdef Q_OS_WASM
			 { QStringLiteral("wasm"), true },
		 #endif
			 { QStringLiteral("code"), code }
		 })
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

	send(HttpConnection::ApiAuth, QStringLiteral("login"),
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
	send(HttpConnection::ApiAuth, QStringLiteral("registration"), data)
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

	Token jwt(token.toUtf8());
	if (jwt.payload().empty()) {
		LOG_CWARNING("credential") << "Invalid token:" << token;
		server()->setToken(QString());
		return false;
	}


	if (jwt.payload().value(QStringLiteral("exp")).toInteger() <= QDateTime::currentSecsSinceEpoch()) {
		LOG_CINFO("client") << "Token expired";
		server()->setToken(QString());
		return false;
	}

	server()->user()->setLoginState(User::LoggingIn);

	send(HttpConnection::ApiAuth, QStringLiteral("login"),
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
 * @brief Client::reloadUser
 */

void Client::reloadUser(QJSValue func)
{
	if (!server() || !server()->user() || server()->user()->username().isEmpty())
		return;

	LOG_CDEBUG("client") << "Reload user:" << qPrintable(server()->user()->username());

	send(HttpConnection::ApiGeneral, "me")->done(this, [this, func](const QJsonObject &json) mutable {
		if (!server())
			return;

		server()->user()->loadFromJson(json);

		if (func.isCallable())
			func.call();
	});
}




/**
 * @brief Client::parseUrl
 */

void Client::parseUrl()
{
	if (m_parseUrl.isEmpty()) {
		LOG_CTRACE("client") << "Parse URL empty";
		return;
	}

	LOG_CINFO("client") << "Parse URL:" << m_parseUrl;

	m_parseUrl = normalizeUrl(m_parseUrl);

	const QUrlQuery q(m_parseUrl);


	if (!server()) {
		Server *server = serverAddWithUrl(m_parseUrl);

		if (server) {
			connectToServer(server);
			return;
		} else {
			snack(tr("Érvénytelen URL"));
		}
	}


	if (server()->url() == m_parseUrl || server()->url().isParentOf(m_parseUrl)) {
		LOG_CDEBUG("client") << "URL parsed successfuly";

		const QString &page = q.queryItemValue(QStringLiteral("page"));

		if (page == QStringLiteral("registration")) {
			const QString &oauth = q.queryItemValue(QStringLiteral("oauth"));
			const QString &code = q.queryItemValue(QStringLiteral("code"));

			LOG_CDEBUG("client") << "Load registration with oauth" << oauth << "and code" << code;

			emit loadRequestRegistration(oauth, code);
		} else if (page == QStringLiteral("login")) {
			const QString &token = q.queryItemValue(QStringLiteral("token"));

			if (!token.isEmpty()) {
				LOG_CINFO("client") << "Login with token:" << qPrintable(token);
				server()->setToken(token);
				loginToken();
			}

		} else if (!q.isEmpty()) {
			LOG_CWARNING("client") << "Invalid page request:" << page;
		}
	} else {

		messageWarning(tr("A link másik szerverhez vezet. Előbb zárd le a kapcsolatot!"), tr("Csatlakozás"));
	}


	m_parseUrl = QUrl();
}


/**
 * @brief Client::setParseUrl
 * @param url
 */

void Client::setParseUrl(const QUrl &url)
{
	m_parseUrl = url;
}


/**
 * @brief Client::getParseUrl
 */

const QUrl &Client::getParseUrl() const
{
	return m_parseUrl;
}




/**
 * @brief Client::normalizeUrl
 * @param url
 * @return
 */

QUrl Client::normalizeUrl(const QUrl &url)
{
	QUrl ret = url;

	if (url.isEmpty())
		return ret;

	LOG_CDEBUG("client") << "Normalize URL:" << url;

	const QUrlQuery q(url);


	if (url.scheme() == QStringLiteral("callofsuli")) {
		if (q.hasQueryItem(QStringLiteral("ssl")))
			ret.setScheme(QStringLiteral("https"));
		else
			ret.setScheme(QStringLiteral("http"));

		LOG_CINFO("client") << "Normalized URL:" << ret;
	}

	return ret;
}




/**
 * @brief Client::getSystemInfo
 * @return
 */

QString Client::getSystemInfo() const
{
	QString text;

	QScreen *screen = QApplication::primaryScreen();

	text += tr("Qt: **%1**\n\n").arg(QT_VERSION_STR);
	text += tr("Képernyő mérete: **%1x%2**\n\n").arg(screen->geometry().width()).arg(screen->geometry().height());
	text += tr("Logical DPI: **%1**\n\n").arg(screen->logicalDotsPerInch());

	return text;
}



/**
 * @brief Client::getDevicePixelSizeCorrection
 * @return
 */

qreal Client::getDevicePixelSizeCorrection() const
{
	const qreal refDpi = 72.;
	const qreal refHeight = 700.;
	const qreal refWidth = 400.;
	const QRect &rect = QApplication::primaryScreen()->geometry();
	const qreal &height = qMax(rect.width(), rect.height());
	const qreal &width = qMin(rect.width(), rect.height());

	const qreal &dpi = QApplication::primaryScreen()->logicalDotsPerInch();

	qreal ratioFont = qMin(1., qMin(height*refDpi/(dpi*refHeight), width*refDpi/(dpi*refWidth)));

	LOG_CDEBUG("client") << "Device pixel size correction:" << ratioFont;

	return ratioFont;
}





/**
 * @brief Client::generateRandomString
 * @param length
 * @param characters
 * @return
 */

QString Client::generateRandomString(int length, const QString &characters) const
{
	if (characters.isEmpty())
		return Utils::generateRandomString(length);
	else
		return Utils::generateRandomString(length, characters.toUtf8());
}



/**
 * @brief Client::userToMap
 * @param data
 * @return
 */

QVariantMap Client::userToMap(const QJsonObject &data) const
{
	User u;
	u.loadFromJson(data, true);
	return u.toVariantMap();
}


/**
 * @brief Client::retranslate
 * @param language
 */

void Client::retranslate(const QString &language)
{
	QLocale locale(language);

	LOG_CINFO("client") << "Retranslate:" << qPrintable(language) << "-" << qPrintable(locale.name());

	std::unique_ptr<QTranslator> translator(new QTranslator());

	if (translator->load(locale, QStringLiteral("qt"), QStringLiteral("_"), QStringLiteral(":/"))) {
		LOG_CDEBUG("client") << "Translator file loaded:" << qPrintable(translator->filePath());

		if (m_translator)
			m_application->application()->removeTranslator(m_translator.get());

		m_application->application()->installTranslator(translator.get());
		m_translator = std::move(translator);
		m_application->engine()->retranslate();
	} else {
		LOG_CWARNING("client") << "Can't load translator language:" << qPrintable(locale.name());
	}
}



/**
 * @brief Client::downloader
 * @return
 */

Downloader *Client::downloader() const
{
	return m_downloader.get();
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

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	margins = MobileUtils::getSafeMargins();
#else
	const QString &str = QString::fromUtf8(qgetenv("SAFE_MARGINS"));

	if (!str.isEmpty()) {
		margins.setTop(str.section(',', 0, 0).toDouble());
		margins.setLeft(str.section(',', 1, 1).toDouble());
		margins.setBottom(str.section(',', 2, 2).toDouble());
		margins.setRight(str.section(',', 3, 3).toDouble());

		setSafeMargins(margins);
		return;
	}


	QPlatformWindow *platformWindow = m_mainWindow->handle();
	if(!platformWindow) {
		LOG_CERROR("client") << "Invalid QPlatformWindow";
		return;
	}
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
	return m_utils.get();
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

	if (m_mainWindow)
		fullScreenHelperDisconnect(m_mainWindow);

	m_mainWindow = newMainWindow;
	emit mainWindowChanged();

	if (!m_mainWindow)
		return;

	fullScreenHelperConnect(m_mainWindow);
	m_mainWindow->setIcon(QIcon(QStringLiteral(":/internal/img/cos.png")));

	safeMarginsGet();
}



/**
 * @brief Client::currentGame
 * @return
 */

AbstractGame *Client::currentGame() const
{
	return m_currentGame.get();
}


void Client::setCurrentGame(AbstractGame *newCurrentGame)
{
	if (m_currentGame.get() == newCurrentGame)
		return;

	if (m_currentGame)
		m_currentGame->disconnect(this);

	//disconnect(m_currentGame.get(), &AbstractGame::gameDestroyRequest, this, &Client::onGameDestroyRequest);

	m_currentGame.reset(newCurrentGame);
	emit currentGameChanged();

	if (m_currentGame)
		connect(m_currentGame.get(), &AbstractGame::gameDestroyRequest, this, &Client::onGameDestroyRequest);

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
		QMetaObject::invokeMethod(m_mainWindow, "snack", Qt::AutoConnection,
								  Q_ARG(QString, text)
								  );
	}
}







/**
 * @brief Client::loadDemoMap
 */

QQuickItem* Client::loadDemoMap(const QUrl &url)
{
	if (m_currentGame) {
		LOG_CERROR("client") << "Game already exists";
		return nullptr;
	}

	std::unique_ptr<MapPlayDemo> mapPlay(new MapPlayDemo(this));

	bool success = false;

	if (url.isEmpty())
		success = mapPlay->load();
	else
		success = mapPlay->load(url.toLocalFile());

	if (!success)
		return nullptr;


	if (!server()) {
		LOG_CINFO("client") << "Connect to static server";
		connectToServer(getStaticServer());
	}

	QQuickItem *page = stackPushPage(QStringLiteral("PageMapPlay.qml"), QVariantMap({
																						{ QStringLiteral("title"), tr("Demó pálya") },
																						{ QStringLiteral("map"), QVariant::fromValue(mapPlay.get()) }
																					}));

	if (!page) {
		messageError(tr("Nem lehet betölteni a demó oldalt!"));
		return nullptr;
	}

	connect(page, &QQuickItem::destroyed, mapPlay.get(), &MapPlay::deleteLater);
	connect(page, &QQuickItem::destroyed, this, &Client::onDemoMapDestroyed);

	mapPlay.release();

	return page;
}



/**
 * @brief Client::debug
 * @return
 */

bool Client::debug() const
{
	return m_application->debug();
}








/**
 * @brief Client::onGameDestroyRequest
 */

void Client::onGameDestroyRequest()
{
	LOG_CDEBUG("client") << "Destroy current game request";

	if (!m_currentGame)
		return;

	m_currentGame.reset();
}



/**
 * @brief Client::onDemoMapDestroyed
 */

void Client::onDemoMapDestroyed()
{
	LOG_CTRACE("client") << "Demo map destroyed";

	if (server() == getStaticServer() && m_httpConnection) {
		LOG_CINFO("client") << "Disconnect from static server";
		m_httpConnection->close();
	}
}


/**
 * @brief Client::getStaticServer
 * @return
 */

Server *Client::getStaticServer()
{
	if (m_staticServer)
		return m_staticServer.get();

	m_staticServer.reset(new Server);

	m_staticServer->setUrl(QStringLiteral("https://valaczka.github.io/callofsuli/demo"));
	m_staticServer->setIsStatic(true);
	m_staticServer->setTemporary(true);

	return m_staticServer.get();
}


/**
 * @brief Client::fullScreenHelper
 * @return
 */

bool Client::fullScreenHelper() const
{
	return m_mainWindow && m_mainWindow->visibility() == QWindow::FullScreen;
}


/**
 * @brief Client::setFullScreenHelper
 * @param newFullScreenHelper
 */

void Client::setFullScreenHelper(bool newFullScreenHelper)
{
	if (!m_mainWindow)
		return;

	m_mainWindow->setVisibility(newFullScreenHelper ? QWindow::FullScreen : QWindow::Maximized);
}


/**
 * @brief Client::contextHelper
 * @return
 */

ContextHelper *Client::contextHelper() const
{
	return m_contextHelper.get();
}
