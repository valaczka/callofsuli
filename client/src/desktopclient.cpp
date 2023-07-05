/*
 * ---- Call of Suli ----
 *
 * desktopclient.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * DesktopClient
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

#include "desktopclient.h"
#include "Logger.h"
#include "qdiriterator.h"
#include "qquickwindow.h"
#include "qscreen.h"
#include <QSettings>


/**
 * @brief DesktopClient::DesktopClient
 * @param app
 * @param parent
 */

DesktopClient::DesktopClient(Application *app, QObject *parent)
	: Client(app, parent)
	, m_sound(new Sound(this))
	, m_serverList(new ServerList(this))
{
	LOG_CTRACE("client") << "DesktopClient created:" << this;

	m_oauthData.isLocal = true;

	m_sound->init();

	connect(this, &Client::mainWindowChanged, this, &DesktopClient::onMainWindowChanged);
	connect(this, &Client::startPageLoaded, this, &DesktopClient::onStartPageLoaded);

	serverListLoad();

	m_soundEffectTimer.setInterval(1500);
	connect(&m_soundEffectTimer, &QTimer::timeout, this, &DesktopClient::onSoundEffectTimeout);

	connect(m_sound, &Sound::volumeSfxChanged, &m_soundEffectTimer, [this](int){ m_soundEffectTimer.start(); });
}



/**
 * @brief DesktopClient::~DesktopClient
 */

DesktopClient::~DesktopClient()
{
	serverDeleteTemporary();
	serverListSave();

	delete m_sound;
	m_sound = nullptr;

	delete m_serverList;
	m_serverList = nullptr;

	LOG_CTRACE("client") << "DesktopClient destroyed:" << this;
}




/**
 * @brief DesktopClient::newSoundEffect
 * @return
 */

QSoundEffect *DesktopClient::newSoundEffect(QObject *parent)
{
	QSoundEffect *e = new QSoundEffect(parent);

	m_soundEffectList.append(e);

	return e;
}







/**
 * @brief DesktopClient::playSound
 * @param source
 * @param soundType
 */

void DesktopClient::playSound(const QString &source, const Sound::SoundType &soundType)
{
	QMetaObject::invokeMethod(m_sound, "playSound", Qt::QueuedConnection,
							  Q_ARG(QString, source),
							  Q_ARG(Sound::SoundType, soundType)
							  );
}


/**
 * @brief DesktopClient::stopSound
 * @param source
 * @param soundType
 */

void DesktopClient::stopSound(const QString &source, const Sound::SoundType &soundType)
{
	QMetaObject::invokeMethod(m_sound, "stopSound", Qt::QueuedConnection,
							  Q_ARG(QString, source),
							  Q_ARG(Sound::SoundType, soundType)
							  );
}



/**
 * @brief DesktopClient::onMainWindowChanged
 */

void DesktopClient::onMainWindowChanged()
{
	if (!m_mainWindow)
		return;

#if defined(Q_OS_ANDROID) || !defined(QT_DEBUG)
	m_mainWindow->showFullScreen();
#endif

	if (!m_mainWindow->screen())
		return;

	connect(m_mainWindow->screen(), &QScreen::primaryOrientationChanged, this, &DesktopClient::onOrientationChanged);

}


/**
 * @brief DesktopClient::onOrientationChanged
 * @param orientation
 */

void DesktopClient::onOrientationChanged(Qt::ScreenOrientation orientation)
{
	LOG_CTRACE("client") << "Screen orientation changed:" << orientation;

	safeMarginsGet();
}





/**
 * @brief DesktopClient::onStartPageLoaded
 */

void DesktopClient::onStartPageLoaded()
{
	if (m_parseUrl.isValid()) {
		LOG_CTRACE("client") << "Try connect to command line URL:" << m_parseUrl;

		Server *s = serverAddWithUrl(m_parseUrl);

		if (s) {
			connectToServer(s);
			return;
		}

	}

	for (Server *s : *m_serverList)
		if (s->autoConnect()) {
			connectToServer(s);
			break;
		}
}


/**
 * @brief DesktopClient::onOAuthFinished
 */

void DesktopClient::onOAuthFinished()
{
	LOG_CTRACE("client") << "Desktop OAuth finished";

	m_oauthData.status = OAuthData::Invalid;
	m_oauthData.state = "";
	m_oauthData.path = "";
	m_oauthData.type = OAuthData::Login;
	m_oauthData.timer.stop();
	if (m_oauthData.webPage)
		stackPop(m_oauthData.webPage);
}


/**
 * @brief DesktopClient::onOAuthStarted
 * @param url
 */

void DesktopClient::onOAuthStarted(const QUrl &url)
{
	LOG_CTRACE("client") << "Desktop OAuth started:" << url;

	QString title;

	switch (m_oauthData.type) {
	case Client::OAuthData::Login:
		title = tr("Bejelentkezés");
		break;
	case Client::OAuthData::Registration:
		title = tr("Regisztráció");
		break;
	}

	QQuickItem *page = stackPushPage(QStringLiteral("PageWebView.qml"),
									 QVariantMap({
													 { QStringLiteral("url"), url },
													 { QStringLiteral("title"), title }
												 }));

	connect(page, &QQuickItem::destroyed, this, [this](){m_oauthData.webPage = nullptr;});

	m_oauthData.webPage = page;
	m_oauthData.status = OAuthData::Pending;
	m_oauthData.timer.start();
}


/**
 * @brief DesktopClient::prepareOAuth
 * @param json
 */

void DesktopClient::prepareOAuth(const QJsonObject &json)
{
	LOG_CTRACE("client") << "Local OAuth" << json;

	if (!m_replyHandler) {
		LOG_CTRACE("client") << "Create local reply handler";
		m_replyHandler = new QOAuthHttpServerReplyHandler(this);
		if (!m_replyHandler->isListening()) {
			messageError(tr("OAuth2 hitelesítés nem működik!"), tr("Belső hiba"));
			return;
		}
	}

	if (m_codeFlow)
		m_codeFlow->deleteLater();

	m_codeFlow = new QOAuth2AuthorizationCodeFlow(this);
	LOG_CTRACE("client") << "Create local code flow" << m_codeFlow->state();

	m_codeFlow->setReplyHandler(m_replyHandler);
	m_codeFlow->setAccessTokenUrl(json.value(QStringLiteral("access_token_url")).toString());
	m_codeFlow->setAuthorizationUrl(json.value(QStringLiteral("authorization_url")).toString());
	m_codeFlow->setClientIdentifier(json.value(QStringLiteral("client_id")).toString());
	m_codeFlow->setClientIdentifierSharedKey(json.value(QStringLiteral("client_key")).toString());
	m_codeFlow->setScope(json.value(QStringLiteral("scope")).toString());

	m_codeFlow->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QVariantMap* parameters) {
		if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
			const QString &code = QString::fromUtf8(QByteArray::fromPercentEncoding(parameters->value(QStringLiteral("code")).toByteArray()));
			parameters->insert(QStringLiteral("code"), code);
		}

		if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
			parameters->insert(QStringLiteral("access_type"), QStringLiteral("offline"));
			parameters->insert(QStringLiteral("prompt"), QStringLiteral("consent"));
		}
	});


	connect(m_codeFlow, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &DesktopClient::onOAuthStarted);
	connect(m_codeFlow, &QOAuth2AuthorizationCodeFlow::granted, this, [this]{
		QJsonObject d;

		d.insert(QStringLiteral("access_token"), m_codeFlow->token());
		d.insert(QStringLiteral("refresh_token"), m_codeFlow->refreshToken());
		d.insert(QStringLiteral("id_token"), m_codeFlow->extraTokens().value(QStringLiteral("id_token")).toString());
		d.insert(QStringLiteral("expiration"), m_codeFlow->expirationAt().toSecsSinceEpoch());
		d.insert(QStringLiteral("local"), true);
		d.insert(QStringLiteral("state"), m_oauthData.state);

		send(WebSocket::ApiAuth, m_oauthData.path, d)
				->done([this](const QJsonObject &data){this->onLoginSuccess(data);})
				->fail([this](const QString &err){this->onLoginFailed(err);});

		if (m_oauthData.webPage)
			stackPop(m_oauthData.webPage);

	});

	m_codeFlow->grant();


}


/**
 * @brief DesktopClient::serverListLoad
 * @param dir
 */

void DesktopClient::serverListLoad(const QDir &dir)
{
	LOG_CDEBUG("client") << "Load servers from:" << qPrintable(dir.absolutePath());

	m_serverList->clear();

	QDirIterator it(dir.absolutePath(), {QStringLiteral("config.json")}, QDir::Files, QDirIterator::Subdirectories);

	while (it.hasNext()) {
		const QString &realname = it.next();

		QJsonObject o = Utils::fileToJsonObject(realname);

		Server *s = Server::fromJson(o);

		if (!s)
			continue;

		connect(s, &Server::selectedChanged, this, &DesktopClient::serverListSelectedCountChanged);

		s->setName(realname.section('/', -2, -2));
		s->setDirectory(realname.section('/', 0, -2));

		if (!s->token().isEmpty() && !s->isTokenValid())
			s->setToken(QLatin1String(""));


		LOG_CTRACE("client") << "Add server"<< s->name() << s->url() << s->directory();

		m_serverList->append(s);
	}

	LOG_CDEBUG("client") << "Servers loaded:" << m_serverList->size();
}




/**
 * @brief DesktopClient::serverListSave
 * @param dir
 */

void DesktopClient::serverListSave(const QDir &dir)
{
	LOG_CDEBUG("client") << "Save servers to:" << qPrintable(dir.absolutePath());

	for (const Server *s : *m_serverList) {
		LOG_CTRACE("client") << "Server dir" << dir;

		if (!dir.exists(s->name())) {
			if (!dir.mkpath(s->name())) {
				LOG_CERROR("client") << "Can't create server directory:" << qPrintable(dir.absoluteFilePath(s->name()));
				continue;
			}
		}

		if (!Utils::jsonObjectToFile(s->toJson(), dir.filePath(s->name()+QStringLiteral("/config.json")))) {
			LOG_CERROR("client") << "Can't save server data:" << qPrintable(dir.absoluteFilePath(s->name()));
		}
	}
}



/**
 * @brief DesktopClient::onSoundEffectTimeout
 */

void DesktopClient::onSoundEffectTimeout()
{
	if (!m_sound)
		return;

	const qreal vol = (qreal) m_sound->volumeSfx() / 100.0;

	foreach (QSoundEffect *e, m_soundEffectList)
		if (e)
			e->setVolume(vol);
}


/**
 * @brief DesktopClient::sound
 * @return
 */

Sound *DesktopClient::sound() const
{
	return m_sound;
}




/**
 * @brief DesktopClient::serverList
 * @return
 */

ServerList *DesktopClient::serverList() const
{
	return m_serverList;
}


/**
 * @brief DesktopClient::serverSetAutoConnect
 * @param server
 */

void DesktopClient::serverSetAutoConnect(Server *server) const
{
	for (Server *s : *m_serverList)
		s->setAutoConnect(s == server);
}


/**
 * @brief DesktopClient::serverAdd
 * @param server
 */

Server *DesktopClient::serverAdd()
{
	LOG_CTRACE("client") << "Add new server";

	QDir dir = Utils::standardPath(QStringLiteral("servers"));

	QString subdir;

	for (int i=1; i<INT_MAX; ++i) {
		subdir = QString::number(i);
		if (!dir.exists(subdir))
			break;
	}

	if (!dir.mkpath(subdir)) {
		LOG_CWARNING("client") << "Can't create directory:" << qPrintable(dir.absoluteFilePath(subdir));
		messageError(tr("Nem sikerült lérehozni a szerver könyvtárát!"));
		return nullptr;
	}

	Server *server = new Server();

	server->setName(subdir);
	server->setDirectory(dir.absoluteFilePath(subdir));

	connect(server, &Server::selectedChanged, this, &DesktopClient::serverListSelectedCountChanged);

	if (!Utils::jsonObjectToFile(server->toJson(), dir.filePath(server->name()+QStringLiteral("/config.json")))) {
		LOG_CERROR("client") << "Can't save server data:" << qPrintable(dir.absoluteFilePath(server->name()));
		messageError(tr("Nem sikerült menteni a szerver adatait!"));
		delete server;
		return nullptr;
	}

	m_serverList->append(server);

	LOG_CINFO("client") << "Server created:" << qPrintable(server->name()) << qPrintable(server->url().toString());

	return server;
}




/**
 * @brief DesktopClient::serverDelete
 * @param server
 */

bool DesktopClient::serverDelete(Server *server)
{
	Q_ASSERT(server);

	LOG_CTRACE("client") << "Delete server:" << qPrintable(server->name());

	QDir dir = server->directory();

	if (dir.removeRecursively()) {
		m_serverList->remove(server);
		LOG_CINFO("client") << "Server removed:" << qPrintable(server->directory().path());
		return true;
	} else {
		LOG_CERROR("client") << "Can't remove server data:" << qPrintable(server->directory().path());
		messageError(tr("Nem sikerült törölni a szerver adatait!"));
		return false;
	}
}


/**
 * @brief DesktopClient::serverDeleteTemporary
 */

void DesktopClient::serverDeleteTemporary()
{
	QVector<Server*> list;

	for (Server *s : *m_serverList) {
		if (s->temporary())
			list.append(s);
	}

	if (list.isEmpty())
		return;

	LOG_CTRACE("client") << "Delete temporary servers:" << list.size();

	foreach (Server *s, list)
		serverDelete(s);
}



/**
 * @brief DesktopClient::serverDeleteSelected
 * @return
 */

bool DesktopClient::serverDeleteSelected()
{
	LOG_CTRACE("client") << "Delete selected servers";

	QVector<Server*> list;

	list.reserve(m_serverList->size());

	for (Server *s : *m_serverList) {
		if (s->selected())
			list.append(s);
	}

	list.squeeze();

	foreach (Server *s, list) {
		if (!serverDelete(s))
			return false;
	}

	return true;
}



/**
 * @brief DesktopClient::serverAddWithUrl
 * @param url
 * @return
 */

Server *DesktopClient::serverAddWithUrl(const QUrl &url)
{
	LOG_CTRACE("client") << "Add new server:" << url;

	if (url.scheme() != QLatin1String("http") && url.scheme() != QLatin1String("https")) {
		LOG_CWARNING("client") << "Invalid scheme:" << url;
		return nullptr;
	}

	QUrl baseUrl;
	baseUrl.setScheme(url.scheme());
	baseUrl.setHost(url.host());
	baseUrl.setPort(url.port());

	Server *server = nullptr;

	for (Server *s : *m_serverList) {
		if (s->url().toString() == baseUrl.toString()) {
			server = s;
			break;
		}
	}

	if (!server) {
		server = serverAdd();
		server->setServerName(tr("-- automatikusan hozzáadott szerver --"));
		server->setUrl(baseUrl);
		server->setTemporary(true);
	}

	return server;
}





/**
 * @brief DesktopClient::serverListSelectedCount
 * @return
 */

int DesktopClient::serverListSelectedCount() const
{
	return Utils::selectedCount(m_serverList);
}


