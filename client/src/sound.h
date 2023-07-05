/*
 * ---- Call of Suli ----
 *
 * cosclientsound.h
 *
 * Created on: 2021. 01. 07.
 *     Author: Valaczka János Pál <valaczka.janos@piarista.hu>
 *
 * CosClientSound
 *
 *  This file is part of Call of Suli.
 *
 *  Call of Suli is free software: you can redistribute it and/or modify
 *  it under the terms of the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SOUND_H
#define SOUND_H

#include <QObject>
#include <QMediaPlayer>
#include <QVariantAnimation>

/**
 * @brief The Sound class
 */

class Sound : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int volumeMusic READ volumeMusic WRITE setVolumeMusic NOTIFY volumeMusicChanged)
	Q_PROPERTY(int volumeSfx READ volumeSfx WRITE setVolumeSfx NOTIFY volumeSfxChanged)
	Q_PROPERTY(int volumeVoiceOver READ volumeVoiceOver WRITE setVolumeVoiceOver NOTIFY volumeVoiceOverChanged)
	Q_PROPERTY(bool vibrate READ vibrate WRITE setVibrate NOTIFY vibrateChanged)

public:
	enum ChannelType { MusicChannel, SfxChannel, VoiceoverChannel };
	Q_ENUM(ChannelType)

	enum SoundType { Music, VoiceOver, PlayerSfx, PlayerVoice, GameSound };
	Q_ENUM(SoundType)

	explicit Sound(QObject *parent = nullptr);
	virtual ~Sound();

	Q_INVOKABLE void performVibrate() const;

	void init();

	Q_INVOKABLE void playSound(const QString &source, const Sound::SoundType &soundType);
	Q_INVOKABLE void stopSound(const QString &source, const Sound::SoundType &soundType);

	Q_INVOKABLE bool isPlayingMusic() const;

	int volumeMusic() const { return m_mediaPlayerMusic ? m_mediaPlayerMusic->volume() : 0;  }
	int volumeSfx() const { return m_mediaPlayerSfx ? m_mediaPlayerSfx->volume() : 0;  }
	int volumeVoiceOver() const { return m_mediaPlayerVoiceOver ? m_mediaPlayerVoiceOver->volume() : 0; }

	void setVolumeSfx(int volume);
	void setVolumeMusic(int volume);
	void setVolumeVoiceOver(int volume);

	bool vibrate() const;
	void setVibrate(bool newVibrate);

private slots:
	void musicPlay(const QString &source);
	void musicLoadNextSource();

signals:
	void volumeSfxChanged(int volumeSfx);
	void volumeMusicChanged(int volumeMusic);
	void volumeVoiceOverChanged(int volumeVoiceOver);
	void vibrateChanged();

private:
	QMediaPlayer *m_mediaPlayerMusic;
	QMediaPlayer *m_mediaPlayerSfx;
	QMediaPlayer *m_mediaPlayerVoiceOver;
	SoundType m_soundTypeSfx;
	QString m_musicNextSource;
	QVariantAnimation *m_fadeAnimation;
	int m_musicVolume;
	bool m_vibrate = true;
};



#endif // SOUND_H
