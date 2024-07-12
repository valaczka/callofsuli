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

#include "clientcache.h"
#include <QObject>
#include <QQuickItem>
#include <QLoggingCategory>
#include "downloader.h"
#include "qtimer.h"
#include "httpconnection.h"
#include <QAbstractSocket>
#include <QNetworkReply>
#include "QQuickWindow"
#include "sound.h"


class Application;
class AbstractGame;
class HttpConnection;
class Updater;
class Utils;


#if QT_VERSION >= 0x060000

#ifndef OPAQUE_PTR_AbstractGame
#define OPAQUE_PTR_AbstractGame
  Q_DECLARE_OPAQUE_POINTER(AbstractGame*)
#endif

#ifndef OPAQUE_PTR_Updater
#define OPAQUE_PTR_Updater
  Q_DECLARE_OPAQUE_POINTER(Updater*)
#endif

#ifndef OPAQUE_PTR_HttpConnection
#define OPAQUE_PTR_HttpConnection
  Q_DECLARE_OPAQUE_POINTER(HttpConnection*)
#endif


#ifndef OPAQUE_PTR_Utils
#define OPAQUE_PTR_Utils
  Q_DECLARE_OPAQUE_POINTER(Utils*)
#endif

#endif

/**
 * @brief The Client class
 */

class Client : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem* mainStack READ mainStack WRITE setMainStack NOTIFY mainStackChanged)
	Q_PROPERTY(QQuickWindow* mainWindow READ mainWindow WRITE setMainWindow NOTIFY mainWindowChanged)
	Q_PROPERTY(bool fullScreenHelper READ fullScreenHelper WRITE setFullScreenHelper NOTIFY fullScreenHelperChanged)

	Q_PROPERTY(Utils* Utils READ utils CONSTANT)
	Q_PROPERTY(bool debug READ debug CONSTANT)
	Q_PROPERTY(Updater *updater READ updater CONSTANT)

	Q_PROPERTY(AbstractGame* currentGame READ currentGame NOTIFY currentGameChanged)

	Q_PROPERTY(qreal safeMarginLeft READ safeMarginLeft NOTIFY safeMarginLeftChanged)
	Q_PROPERTY(qreal safeMarginRight READ safeMarginRight NOTIFY safeMarginRightChanged)
	Q_PROPERTY(qreal safeMarginTop READ safeMarginTop NOTIFY safeMarginTopChanged)
	Q_PROPERTY(qreal safeMarginBottom READ safeMarginBottom NOTIFY safeMarginBottomChanged)

	Q_PROPERTY(HttpConnection *httpConnection READ httpConnection CONSTANT)
	Q_PROPERTY(Server *server READ server NOTIFY serverChanged)
	Q_PROPERTY(Sound *sound READ sound NOTIFY soundChanged)
	Q_PROPERTY(Downloader *downloader READ downloader CONSTANT FINAL)




public:
	explicit Client(Application *app);
	virtual ~Client();


	/**
	 * @brief The OAuthData class
	 */

	struct OAuthData {
		enum Type {
			Login,
			Registration
		};

		enum Status {
			Invalid,
			UrlReceived,
			Pending,
			Failed,
			Success
		};

		QString path;
		QString state;
		Type type = Login;
		Status status = Invalid;
		QTimer timer;
		QPointer<QQuickItem> webPage = nullptr;
	};

	QQuickItem *mainStack() const;
	void setMainStack(QQuickItem *newMainStack);

	Q_INVOKABLE QQuickItem *stackPushPage(QString qml, QVariantMap parameters = {}) const;
	Q_INVOKABLE bool stackPop(int index = -1, const bool &forced = false) const;
	Q_INVOKABLE bool stackPop(QQuickItem *page) const;
	Q_INVOKABLE bool stackPopToPage(QQuickItem *page) const;

	QQuickWindow *mainWindow() const;
	void setMainWindow(QQuickWindow *newMainWindow);

	Q_INVOKABLE bool closeWindow(const bool &forced = false);
	Q_INVOKABLE void notifyWindow();

	Utils *utils() const;
	Application *application() const;

	bool debug() const;


	AbstractGame *currentGame() const;
	void setCurrentGame(AbstractGame *newCurrentGame);

	Q_INVOKABLE QQuickItem *loadDemoMap(const QUrl &url = QString());
	Q_INVOKABLE QQuickItem *loadAdjacencySetup(const QString &world = QString());

	Q_INVOKABLE void messageInfo(const QString &text, QString title = "") const;
	Q_INVOKABLE void messageWarning(const QString &text, QString title = "") const;
	Q_INVOKABLE void messageError(const QString &text, QString title = "") const;

	Q_INVOKABLE void snack(const QString &text) const;


	// Cache

	Q_INVOKABLE qolm::QOlmBase *cache(const QString &key) const { return m_cache.get(key); }
	Q_INVOKABLE void setCache(const QString &key, const QJsonArray &list) { m_cache.set(key, list); }
	Q_INVOKABLE void reloadCache(const QString &key) { m_cache.reload(m_httpConnection.get(), key); }
	Q_INVOKABLE void reloadCache(const QString &key, QObject *inst, const QJSValue &func) { m_cache.reload(m_httpConnection.get(), key, inst, func); }
	Q_INVOKABLE QObject *findCacheObject(const QString &key, const QVariant &value) { return m_cache.find(key, value); }
	Q_INVOKABLE QObject *findOlmObject(qolm::QOlmBase *list, const QString &key, const QVariant &value) { return OlmLoader::find(list, key.toUtf8(), value); }

	Q_INVOKABLE void callReloadHandler(const QString &key, qolm::QOlmBase *list, const QJsonArray &array) {
		m_cache.callReloadHandler(key, list, array);
	}

	Q_INVOKABLE void callFinderHandler(const QString &key, qolm::QOlmBase *list, const QVariant &value) {
		m_cache.callFinderHandler(key, list, value);
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


	// HttpConnection

	Q_INVOKABLE HttpConnection *httpConnection() const;
	Q_INVOKABLE HttpReply *send(const HttpConnection::API &api, const QString &path, const QJsonObject &data = {}) const;

	Server *server() const;

	Q_INVOKABLE virtual Server *serverAddWithUrl(const QUrl &url);
	Q_INVOKABLE void connectToServer(Server *server);

	void stackPopToStartPage();


	// Login

	Q_INVOKABLE void loginOAuth2(const QString &provider);
	Q_INVOKABLE void registrationOAuth2(const QString &provider, const QString &code);

	Q_INVOKABLE void loginPlain(const QString &username, const QString &password);
	Q_INVOKABLE void registrationPlain(const QJsonObject &data);

	Q_INVOKABLE bool loginToken();

	Q_INVOKABLE void logout();

	Q_INVOKABLE void reloadUser(QJSValue func = QJSValue::UndefinedValue);



	// Scanned, readed,...stb. Url

	Q_INVOKABLE virtual void parseUrl();
	Q_INVOKABLE void setParseUrl(const QUrl &url);
	Q_INVOKABLE const QUrl &getParseUrl() const;

	static QUrl normalizeUrl(const QUrl &url);

	Q_INVOKABLE QString getSystemInfo() const;
	Q_INVOKABLE qreal getDevicePixelSizeCorrection() const;

	Q_INVOKABLE QVariantMap availableCharacters() const;

	Q_INVOKABLE QString generateRandomString(int length, const QString &characters = "") const;



	// Helpers

	Q_INVOKABLE QVariantMap userToMap(const QJsonObject &data) const;
	Q_INVOKABLE void retranslate(const QString &language = QStringLiteral("hu"));

	Downloader *downloader() const;

	Updater *updater() const;

	Sound* sound() const;
	void setSound(std::unique_ptr<Sound> newSound);

	virtual bool fullScreenHelper() const;
	virtual void setFullScreenHelper(bool newFullScreenHelper);

public slots:
	virtual void onHttpConnectionError(const QNetworkReply::NetworkError &code);

protected slots:
	virtual void onApplicationStarted();
	friend class Application;

	virtual void onHttpConnectionResponseError(const QString &error);
#ifndef QT_NO_SSL
	virtual void onHttpConnectionSslError(const QList<QSslError> &errors);
#endif
	virtual void onServerConnected();
	virtual void onServerDisconnected();

	virtual void onUserLoggedIn();
	virtual void onUserLoggedOut();

	virtual void onOAuthFinished();
	virtual void onOAuthStarted(const QUrl &url);
	virtual void onOAuthPendingTimer();

	void onLoginSuccess(const QJsonObject &json);
	void onLoginFailed(const QString &error);
	void onOAuthLoginStateChanged(const QJsonObject &json);

protected:
	void _message(const QString &text, const QString &title, const QString &type) const;
	void _userAuthTokenReceived(const QString &token);
	virtual void fullScreenHelperConnect(QQuickWindow *window);
	virtual void fullScreenHelperDisconnect(QQuickWindow *window);
	void initializeDynamicResources();

signals:
	void loadRequestRegistration(const QString &oauth, const QString &code);
	void serverSetAutoConnectRequest(Server *server);

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
	void soundChanged();
	void fullScreenHelperChanged();

private:
	void startCache();
	void onSoundEffectTimeout();
	void onGameDestroyRequest();


protected:
	Application *const m_application;
	QQuickItem *m_mainStack = nullptr;
	QQuickWindow *m_mainWindow = nullptr;
	bool m_mainWindowClosable = false;

	std::unique_ptr<Utils> m_utils;
	std::unique_ptr<AbstractGame> m_currentGame;
	std::unique_ptr<HttpConnection> m_httpConnection;

	qreal m_safeMarginLeft = 0;
	qreal m_safeMarginRight = 0;
	qreal m_safeMarginTop = 0;
	qreal m_safeMarginBottom = 0;

	QQuickItem *m_startPage = nullptr;
	QQuickItem *m_mainPage = nullptr;

	OAuthData m_oauthData;

	ClientCache m_cache;
	OlmLoader m_olmLoader;

	QUrl m_parseUrl;

	std::unique_ptr<Updater> m_updater;
	std::unique_ptr<QTranslator> m_translator;
	std::unique_ptr<Downloader> m_downloader;


private:
	std::unique_ptr<Sound> m_sound;
};


#endif // CLIENT_H
