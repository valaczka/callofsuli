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
{
	qCDebug(lcClient).noquote() << tr("Desktop DesktopClient created") << this;

	m_sound = new Sound();
	m_sound->moveToThread(&m_soundThread);
	connect(&m_soundThread, &QThread::finished, m_sound, &QObject::deleteLater);
	connect(m_sound, &Sound::volumeSfxChanged, this, &DesktopClient::setSfxVolumeInt);

	m_soundThread.start();

	QMetaObject::invokeMethod(m_sound, "init", Qt::QueuedConnection);

	connect(this, &Client::mainWindowChanged, this, &DesktopClient::onMainWindowChanged);
}



/**
 * @brief DesktopClient::~DesktopClient
 */

DesktopClient::~DesktopClient()
{
	m_soundThread.quit();
	m_soundThread.wait();

	qCDebug(lcClient).noquote() << tr("Desktop DesktopClient destroyed") << this;
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
			func = "volumeMusic";
			break;
		case Sound::SfxChannel:
			func = "volumeSfx";
			break;
		case Sound::VoiceoverChannel:
			func = "volumeVoiceOver";
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
	s.beginGroup("sound");

	switch (channel) {
		case Sound::MusicChannel:
			func = "setVolumeMusic";
			s.setValue("volumeMusic", volume);
			break;
		case Sound::SfxChannel:
			func = "setVolumeSfx";
			s.setValue("volumeSfx", volume);
			break;
		case Sound::VoiceoverChannel:
			func = "setVolumeVoiceOver";
			s.setValue("volumeVoiceOver", volume);
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

	if (!m_mainWindow->screen())
		return;

	connect(m_mainWindow->screen(), &QScreen::orientationChanged, this, &DesktopClient::onOrientationChanged);

}


/**
 * @brief DesktopClient::onOrientationChanged
 * @param orientation
 */

void DesktopClient::onOrientationChanged(Qt::ScreenOrientation orientation)
{

/*	Timer {
		id: timerOrientation
		interval: 20
		running: false
		repeat: false

		onTriggered: {
			var m = cosClient.getWindowSafeMargins(mainWindow)
			safeMarginBottom = m.bottom
			safeMarginLeft = m.left
			safeMarginTop = m.top
			safeMarginRight = m.right*/

	qCDebug(lcClient).noquote() << tr("Screen orientation changed:") << orientation;
}
