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
#include <QQueue>

/**
 * @brief The Sound class
 */

class Sound : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int volumeSfx READ volumeSfx WRITE setVolumeSfx NOTIFY volumeSfxChanged)
	Q_PROPERTY(int volumeVoiceOver READ volumeVoiceOver WRITE setVolumeVoiceOver NOTIFY volumeVoiceOverChanged)
	Q_PROPERTY(int volumeMusic READ volumeMusic WRITE setVolumeMusic NOTIFY volumeMusicChanged)

public:
	enum ChannelType { MusicChannel, SfxChannel, VoiceoverChannel };
	Q_ENUM(ChannelType)

	enum SoundType { Music, VoiceOver, PlayerSfx, PlayerVoice, GameSound };
	Q_ENUM(SoundType)

	explicit Sound(QObject *parent = nullptr);
	virtual ~Sound();

	void init();

	Q_INVOKABLE void playSound(const QString &source, const Sound::SoundType &soundType);
	Q_INVOKABLE void stopSound(const QString &source, const Sound::SoundType &soundType);

	int volume(const Sound::ChannelType &channel) const;
	void setVolume(const Sound::ChannelType &channel, int newVolume);

	bool isMuted(const Sound::ChannelType &channel) const;
	void setMuted(const Sound::ChannelType &channel, bool newMuted);

	Q_INVOKABLE bool isPlayingMusic() const;

	int volumeSfx() const { return volume(SfxChannel); }
	int volumeMusic() const { return volume(MusicChannel); }
	int volumeVoiceOver() const { return volume(VoiceoverChannel); }

	void setVolumeSfx(int volume) { setVolume(SfxChannel, volume); }
	void setVolumeMusic(int volume) { setVolume(MusicChannel, volume); }
	void setVolumeVoiceOver(int volume) { setVolume(VoiceoverChannel, volume); }

signals:
	void volumeMusicChanged(int volume);
	void volumeSfxChanged(int volume);
	void volumeVoiceOverChanged(int volume);

private slots:
	void musicPlay(const QString &source);
	void musicLoadNextSource();

private:
	std::unique_ptr<QMediaPlayer> m_mediaPlayerMusic;
	std::unique_ptr<QMediaPlayer> m_mediaPlayerSfx;
	std::unique_ptr<QMediaPlayer> m_mediaPlayerVoiceOver;
	SoundType m_soundTypeSfx;
	QString m_musicNextSource;

#if QT_VERSION >= 0x060000
	std::unique_ptr<QAudioOutput> m_audioOutputMusic;
	std::unique_ptr<QAudioOutput> m_audioOutputSfx;
	std::unique_ptr<QAudioOutput> m_audioOutputVoiceOver;
#endif

	QQueue<QString> m_playlist;
};



#endif // SOUND_H
