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
#include "qlambdathreadworker.h"
#include "qsoundeffect.h"
#include "sound.h"


/**
 * @brief The DesktopClient class
 */

class DesktopClient : public Client
{
	Q_OBJECT

	Q_PROPERTY(ServerList *serverList READ serverList CONSTANT)
	Q_PROPERTY(int serverListSelectedCount READ serverListSelectedCount NOTIFY serverListSelectedCountChanged)

	Q_PROPERTY(int volumeMusic READ volumeMusic WRITE setVolumeMusic NOTIFY volumeMusicChanged)
	Q_PROPERTY(int volumeSfx READ volumeSfx WRITE setVolumeSfx NOTIFY volumeSfxChanged)
	Q_PROPERTY(int volumeVoiceOver READ volumeVoiceOver WRITE setVolumeVoiceOver NOTIFY volumeVoiceOverChanged)
	Q_PROPERTY(bool vibrate READ vibrate WRITE setVibrate NOTIFY vibrateChanged)


public:
	explicit DesktopClient(Application *app, QObject *parent = nullptr);
	virtual ~DesktopClient();

	QSoundEffect *newSoundEffect();
	void removeSoundEffect(QSoundEffect *effect);

	ServerList *serverList() const;

	Q_INVOKABLE void serverSetAutoConnect(Server *server) const;
	Q_INVOKABLE Server *serverAdd();
	Q_INVOKABLE bool serverDelete(Server *server);
	Q_INVOKABLE void serverDeleteTemporary();
	Q_INVOKABLE bool serverDeleteSelected();
	Q_INVOKABLE Server *serverAddWithUrl(const QUrl &url) override;

	int serverListSelectedCount() const;

	int volumeMusic() const;
	void setVolumeMusic(int newVolumeMusic);

	int volumeSfx() const;
	void setVolumeSfx(int newVolumeSfx);

	int volumeVoiceOver() const;
	void setVolumeVoiceOver(int newVolumeVoiceOver);

	bool vibrate() const;
	void setVibrate(bool newVibrate);

	Q_INVOKABLE bool isPlayingMusic() const { return m_sound && m_sound->isPlayingMusic(); }

public slots:
	void playSound(const QString &source, const Sound::SoundType &soundType);
	void stopSound(const QString &source, const Sound::SoundType &soundType);
	void performVibrate() const;

protected slots:
	void onStartPageLoaded();
	void onOAuthFinished() override;
	void onOAuthStarted(const QUrl &url) override;

private slots:
	void onMainWindowChanged();
	void onOrientationChanged(Qt::ScreenOrientation orientation);
	void onSoundEffectTimeout();

private:
	void serverListLoad(const QDir &dir = Utils::standardPath(QStringLiteral("servers")));
	void serverListSave(const QDir &dir = Utils::standardPath(QStringLiteral("servers")));
	void _setVolume(const Sound::ChannelType &channel, int newVolume);

signals:
	void serverListSelectedCountChanged();
	void volumeMusicChanged();
	void volumeSfxChanged();
	void volumeVoiceOverChanged();
	void vibrateChanged();

private:
	Sound *m_sound = nullptr;
	ServerList *m_serverList = nullptr;
	QVector<QPointer<QSoundEffect>> m_soundEffectList;

	QLambdaThreadWorker *m_worker = nullptr;
	QTimer m_soundEffectTimer;
	bool m_vibrate = true;

	int m_volumeSfx = 0;
	int m_volumeMusic = 0;
	int m_volumeVoiceOver = 0;
};




#endif

