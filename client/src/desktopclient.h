/*
 * ---- Call of Suli ----
 *
 * desktopclient.h
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

#ifndef DESKTOPCLIENT_H
#define DESKTOPCLIENT_H

#include "client.h"
#include "qsoundeffect.h"
#include "qthread.h"
#include "sound.h"

class DesktopClient : public Client
{
	Q_OBJECT

	Q_PROPERTY(qreal sfxVolume READ sfxVolume WRITE setSfxVolume NOTIFY sfxVolumeChanged)

public:
	explicit DesktopClient(Application *app, QObject *parent = nullptr);
	virtual ~DesktopClient();

	qreal sfxVolume() const;
	void setSfxVolume(qreal newSfxVolume);

	QSoundEffect *newSoundEffect();

public slots:
	void playSound(const QString &source, const Sound::SoundType &soundType);
	void stopSound(const QString &source, const Sound::SoundType &soundType);
	int volume(const Sound::ChannelType &channel) const;
	void setVolume(const Sound::ChannelType &channel, const int &volume) const;
	void setSfxVolumeInt(int sfxVolume);

private slots:
	void onMainWindowChanged();
	void onOrientationChanged(Qt::ScreenOrientation orientation);

signals:
	void sfxVolumeChanged(const qreal &volume);

private:
	Sound *m_sound = nullptr;
	QThread m_soundThread;
	qreal m_sfxVolume = 1.0;
};

#endif // DESKTOPCLIENT_H
