/*
 * ---- Call of Suli ----
 *
 * standaloneclient.cpp
 *
 * Created on: 2022. 12. 17.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * StandaloneClient
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

#include "standaloneclient.h"
#include "Logger.h"
#include "qdiriterator.h"
#include "qquickwindow.h"
#include "qscreen.h"
#include "application.h"
#include <QSettings>

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include "mobileutils.h"
#endif



/**
 * @brief StandaloneClient::StandaloneClient
 * @param app
 * @param parent
 */

StandaloneClient::StandaloneClient(Application *app, QObject *parent)
	: Client(app, parent)
	, m_serverList(new ServerList(this))
	#ifdef NO_SOUND_THREAD
	, m_sound(new Sound(this))
	#else
	, m_worker(new QLambdaThreadWorker())
	#endif
{
	LOG_CTRACE("client") << "StandaloneClient created:" << this;

#ifdef NO_SOUND_THREAD
	m_sound->init();
#else
	QDefer ret;

	m_worker->execInThread([this, &ret](){
		m_sound = new Sound();
		m_sound->init();

		ret.resolve();
	});

	QDefer::await(ret);
#endif

	QSettings s;
	s.beginGroup(QStringLiteral("sound"));
	setVolumeMusic(s.value(QStringLiteral("volumeMusic"), 50).toInt());
	setVolumeSfx(s.value(QStringLiteral("volumeSfx"), 50).toInt());
	setVolumeVoiceOver(s.value(QStringLiteral("volumeVoiceOver"), 50).toInt());
	setVibrate(s.value(QStringLiteral("vibrate"), true).toBool());
	s.endGroup();


	connect(this, &Client::mainWindowChanged, this, &StandaloneClient::onMainWindowChanged);
	connect(this, &Client::startPageLoaded, this, &StandaloneClient::onStartPageLoaded);

	serverListLoad();

	m_soundEffectTimer.setInterval(1500);
	connect(&m_soundEffectTimer, &QTimer::timeout, this, &StandaloneClient::onSoundEffectTimeout);

	connect(this, &StandaloneClient::volumeSfxChanged, &m_soundEffectTimer, [this](){ m_soundEffectTimer.start(); });
}



/**
 * @brief StandaloneClient::~StandaloneClient
 */

StandaloneClient::~StandaloneClient()
{
	serverDeleteTemporary();
	serverListSave();

	QSettings s;
	s.beginGroup(QStringLiteral("sound"));
	s.setValue(QStringLiteral("volumeMusic"), m_volumeMusic);
	s.setValue(QStringLiteral("volumeSfx"), m_volumeSfx);
	s.setValue(QStringLiteral("volumeVoiceOver"), m_volumeVoiceOver);
	s.setValue(QStringLiteral("vibrate"), m_vibrate);
	s.endGroup();

#ifdef NO_SOUND_THREAD
	m_sound->deleteLater();
	m_sound = nullptr;
#else
	QDefer ret;

	m_worker->execInThread([this, &ret](){
		m_sound->deleteLater();
		m_sound = nullptr;
		ret.resolve();
	});

	QDefer::await(ret);

	delete m_worker;
	m_worker = nullptr;
#endif

	delete m_serverList;
	m_serverList = nullptr;

	LOG_CTRACE("client") << "StandaloneClient destroyed:" << this;
}




/**
 * @brief StandaloneClient::newSoundEffect
 * @return
 */

QSoundEffect *StandaloneClient::newSoundEffect()
{
	QSoundEffect *e = nullptr;

#ifdef NO_SOUND_THREAD
	e = new QSoundEffect(m_sound);
	const qreal vol = (qreal) volumeSfx() / 100.0;
	e->setVolume(vol);
#else
	QDefer ret;

	m_worker->execInThread([&e, this, &ret](){
		e = new QSoundEffect(m_sound);
		const qreal vol = (qreal) volumeSfx() / 100.0;
		e->setVolume(vol);
		ret.resolve();
	});

	QDefer::await(ret);
#endif

	m_soundEffectList.append(e);

	return e;
}


/**
 * @brief StandaloneClient::removeSoundEffect
 * @param effect
 */

void StandaloneClient::removeSoundEffect(QSoundEffect *effect)
{
	if (effect)
		m_soundEffectList.removeAll(effect);
}







/**
 * @brief StandaloneClient::playSound
 * @param source
 * @param soundType
 */

void StandaloneClient::playSound(const QString &source, const Sound::SoundType &soundType)
{
#ifdef NO_SOUND_THREAD
	m_sound->playSound(source, soundType);
#else
	m_worker->execInThread([this, soundType, source](){
		m_sound->playSound(source, soundType);
	});
#endif
}


/**
 * @brief StandaloneClient::stopSound
 * @param source
 * @param soundType
 */

void StandaloneClient::stopSound(const QString &source, const Sound::SoundType &soundType)
{
#ifdef NO_SOUND_THREAD
	m_sound->stopSound(source, soundType);
#else
	m_worker->execInThread([this, soundType, source](){
		m_sound->stopSound(source, soundType);
	});
#endif
}


/**
 * @brief StandaloneClient::performVibrate
 */

void StandaloneClient::performVibrate() const
{
	if (m_vibrate)
		Utils::vibrate();
}





/**
 * @brief StandaloneClient::onMainWindowChanged
 */

void StandaloneClient::onMainWindowChanged()
{
	if (!m_mainWindow)
		return;

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	m_mainWindow->showFullScreen();
#elif defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
	m_mainWindow->showMaximized();
#endif

	if (!m_mainWindow->screen())
		return;

	connect(m_mainWindow->screen(), &QScreen::primaryOrientationChanged, this, &StandaloneClient::onOrientationChanged);

}


/**
 * @brief StandaloneClient::onOrientationChanged
 * @param orientation
 */

void StandaloneClient::onOrientationChanged(Qt::ScreenOrientation orientation)
{
	LOG_CTRACE("client") << "Screen orientation changed:" << orientation;

	safeMarginsGet();
}





/**
 * @brief StandaloneClient::onStartPageLoaded
 */

void StandaloneClient::onStartPageLoaded()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
	const QString &uri = MobileUtils::checkPendingIntents();

	if (!uri.isEmpty()) {
		m_parseUrl = QUrl(uri);
		parseUrl();
		return;
	}
#endif

	if (m_parseUrl.isValid()) {
		m_parseUrl = normalizeUrl(m_parseUrl);

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
 * @brief StandaloneClient::onOAuthFinished
 */

void StandaloneClient::onOAuthFinished()
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
 * @brief StandaloneClient::onOAuthStarted
 * @param url
 */

void StandaloneClient::onOAuthStarted(const QUrl &url)
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
 * @brief StandaloneClient::serverListLoad
 * @param dir
 */

void StandaloneClient::serverListLoad(const QDir &dir)
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

		connect(s, &Server::selectedChanged, this, &StandaloneClient::serverListSelectedCountChanged);

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
 * @brief StandaloneClient::serverListSave
 * @param dir
 */

void StandaloneClient::serverListSave(const QDir &dir)
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
 * @brief StandaloneClient::_setVolume
 * @param channel
 * @param newVolume
 */

void StandaloneClient::_setVolume(const Sound::ChannelType &channel, int newVolume)
{
#ifndef NO_SOUND_THREAD
	if (!m_worker)
		return;

	m_worker->execInThread([this, channel, newVolume](){
#endif
		int v = m_sound->volume(channel);

		if (v == newVolume)
			return;

		m_sound->setVolume(channel, newVolume);
		switch (channel) {
		case Sound::MusicChannel:
			m_volumeMusic = newVolume;
			emit volumeMusicChanged();
			break;
		case Sound::SfxChannel:
			m_volumeSfx = newVolume;
			emit volumeSfxChanged();
			break;
		case Sound::VoiceoverChannel:
			m_volumeVoiceOver = newVolume;
			emit volumeVoiceOverChanged();
			break;
		}

#ifndef NO_SOUND_THREAD
	});
#endif
}



/**
 * @brief StandaloneClient::onSoundEffectTimeout
 */

void StandaloneClient::onSoundEffectTimeout()
{
	if (!m_sound)
		return;

	const qreal vol = (qreal) volumeSfx() / 100.0;

	foreach (QSoundEffect *e, m_soundEffectList)
		if (e)
			e->setVolume(vol);
}






/**
 * @brief StandaloneClient::serverList
 * @return
 */

ServerList *StandaloneClient::serverList() const
{
	return m_serverList;
}


/**
 * @brief StandaloneClient::serverSetAutoConnect
 * @param server
 */

void StandaloneClient::serverSetAutoConnect(Server *server) const
{
	for (Server *s : *m_serverList)
		s->setAutoConnect(s == server);
}


/**
 * @brief StandaloneClient::serverAdd
 * @param server
 */

Server *StandaloneClient::serverAdd()
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

	connect(server, &Server::selectedChanged, this, &StandaloneClient::serverListSelectedCountChanged);

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
 * @brief StandaloneClient::serverDelete
 * @param server
 */

bool StandaloneClient::serverDelete(Server *server)
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
 * @brief StandaloneClient::serverDeleteTemporary
 */

void StandaloneClient::serverDeleteTemporary()
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
 * @brief StandaloneClient::serverDeleteSelected
 * @return
 */

bool StandaloneClient::serverDeleteSelected()
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
 * @brief StandaloneClient::serverAddWithUrl
 * @param url
 * @return
 */

Server *StandaloneClient::serverAddWithUrl(const QUrl &url)
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
 * @brief StandaloneClient::serverListSelectedCount
 * @return
 */

int StandaloneClient::serverListSelectedCount() const
{
	return Utils::selectedCount(m_serverList);
}



/**
 * @brief StandaloneClient::volumeMusic
 * @return
 */


int StandaloneClient::volumeMusic() const
{
	return m_volumeMusic;
}

void StandaloneClient::setVolumeMusic(int newVolumeMusic)
{
	_setVolume(Sound::MusicChannel, newVolumeMusic);
}

int StandaloneClient::volumeSfx() const
{
	return m_volumeSfx;
}

void StandaloneClient::setVolumeSfx(int newVolumeSfx)
{
	_setVolume(Sound::SfxChannel, newVolumeSfx);
}

int StandaloneClient::volumeVoiceOver() const
{
	return m_volumeVoiceOver;
}

void StandaloneClient::setVolumeVoiceOver(int newVolumeVoiceOver)
{
	_setVolume(Sound::VoiceoverChannel, newVolumeVoiceOver);
}

bool StandaloneClient::vibrate() const
{
	return m_vibrate;
}

void StandaloneClient::setVibrate(bool newVibrate)
{
	if (m_vibrate == newVibrate)
		return;
	m_vibrate = newVibrate;
	emit vibrateChanged();
}

