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
	, m_serverList(new ServerList(this))
{
	LOG_CTRACE("client") << "Desktop DesktopClient created:" << this;

	m_sound = new Sound();
	m_sound->moveToThread(&m_soundThread);
	connect(&m_soundThread, &QThread::finished, m_sound, &QObject::deleteLater);
	connect(m_sound, &Sound::volumeSfxChanged, this, &DesktopClient::setSfxVolumeInt);

	m_soundThread.start();

	QMetaObject::invokeMethod(m_sound, "init", Qt::QueuedConnection);

	connect(this, &Client::mainWindowChanged, this, &DesktopClient::onMainWindowChanged);

	serverListLoad();
}



/**
 * @brief DesktopClient::~DesktopClient
 */

DesktopClient::~DesktopClient()
{
	serverListSave();

	m_soundThread.quit();
	m_soundThread.wait();

	delete m_serverList;

	LOG_CTRACE("client") << "Desktop DesktopClient destroyed:" << this;
}


/**
 * @brief DesktopClient::sfxVolume
 * @return
 */

qreal DesktopClient::sfxVolume() const
{
	return m_sfxVolume;
}

void DesktopClient::setSfxVolume(qreal newSfxVolume)
{
	if (qFuzzyCompare(m_sfxVolume, newSfxVolume))
		return;
	m_sfxVolume = newSfxVolume;
	emit sfxVolumeChanged(m_sfxVolume);
}



QSoundEffect *DesktopClient::newSoundEffect()
{
	QSoundEffect *e = nullptr;

	QMetaObject::invokeMethod(m_sound, "newSoundEffect",
							  Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(QSoundEffect*, e)
							  );

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
 * @brief DesktopClient::volume
 * @param channel
 * @return
 */


int DesktopClient::volume(const Sound::ChannelType &channel) const
{
	QByteArray func;

	switch (channel) {
		case Sound::MusicChannel:
			func = QByteArrayLiteral("volumeMusic");
			break;
		case Sound::SfxChannel:
			func = QByteArrayLiteral("volumeSfx");
			break;
		case Sound::VoiceoverChannel:
			func = QByteArrayLiteral("volumeVoiceOver");
			break;
	}

	int volume = 0;

	QMetaObject::invokeMethod(m_sound, func, Qt::BlockingQueuedConnection,
							  Q_RETURN_ARG(int, volume)
							  );

	return volume;
}


/**
 * @brief DesktopClient::setVolume
 * @param channel
 * @param volume
 */

void DesktopClient::setVolume(const Sound::ChannelType &channel, const int &volume) const
{
	QByteArray func;

	QSettings s;
	s.beginGroup(QStringLiteral("sound"));

	switch (channel) {
		case Sound::MusicChannel:
			func = QByteArrayLiteral("setVolumeMusic");
			s.setValue(QStringLiteral("volumeMusic"), volume);
			break;
		case Sound::SfxChannel:
			func = QByteArrayLiteral("setVolumeSfx");
			s.setValue(QStringLiteral("volumeSfx"), volume);
			break;
		case Sound::VoiceoverChannel:
			func = QByteArrayLiteral("setVolumeVoiceOver");
			s.setValue(QStringLiteral("volumeVoiceOver"), volume);
			break;
	}

	s.endGroup();

	QMetaObject::invokeMethod(m_sound, func, Qt::DirectConnection,
							  Q_ARG(int, volume)
							  );
}





/**
 * @brief DesktopClient::setSfxVolumeInt
 * @param sfxVolume
 */

void DesktopClient::setSfxVolumeInt(int sfxVolume)
{
	qreal r = qreal(sfxVolume)/100;

	setSfxVolume(r);
}


/**
 * @brief DesktopClient::onMainWindowChanged
 */

void DesktopClient::onMainWindowChanged()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)

#endif

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

		LOG_CWARNING("client") << "FOUND" << realname;

		QJsonObject o = Utils::fileToJsonObject(realname);

		LOG_CTRACE("obj") << o;

		Server *s = Server::fromJson(o);

		if (!s)
			continue;


		s->setName(realname.section('/', -2, -2));
		s->setDirectory(realname.section('/', 0, -2));

		LOG_CTRACE("client") << "Add server"<< s->name() << s->url() << s->directory();

		m_serverList->append(s);
	}
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
 * @brief DesktopClient::serverList
 * @return
 */

ServerList *DesktopClient::serverList() const
{
	return m_serverList;
}
